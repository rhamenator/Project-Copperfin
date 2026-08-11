// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/polyglot_benchmark.h"
#include "copperfin/runtime/polyglot_runtime_host.h"
#include "copperfin/security/sha256.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

using namespace copperfin::platform;
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;

constexpr const char* capability_id = "samples.dotnet.add-v1";
constexpr const char* protocol_version = "1.0.0";
int failures = 0;

struct PublishedCandidate {
    fs::path executable;
    fs::path root;
};

void expect_local(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

unsigned long process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long>(::_getpid());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

fs::path unique_root() {
    return fs::temp_directory_path() /
        ("copperfin_polyglot_dotnet_candidate_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

std::string utf8_path(const fs::path& path) {
    return copperfin::platform::path_to_utf8_string(path);
}

PolyglotArtifactAdmissionResult admit_candidate(
    const PublishedCandidate& published) {
    const auto digest = copperfin::security::sha256_hex_for_file(
        utf8_path(published.executable));
    expect_local(digest.ok, "the published .NET candidate should be hashable");
    return admit_polyglot_artifact({
        .capability_id = capability_id,
        .process_policy = {
            .executable_name = utf8_path(published.executable),
            .allowed_path_roots = {utf8_path(published.root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

PolyglotRuntimeHostConfiguration configuration(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& publish_root) {
    auto routes = load_polyglot_route_registry({
        {capability_id, "on", 0U}});
    expect_local(routes.ok(), "the .NET sample route should load");

    PolyglotArtifactInvocationRequest candidate;
    candidate.invocation = {
        .capability_id = capability_id,
        .correlation_id = "dotnet-candidate-correlation",
        .protocol_version = protocol_version,
        .arguments_json = {}};
    candidate.policy = {
        .timeout_ms = 5000U,
        .latency_budget_ms = 4500U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = PolyglotFallbackPolicy::fail_fast,
        .max_attempts = 1U};
    candidate.working_directory = utf8_path(publish_root);
    candidate.poll_interval_ms = 2U;
    candidate.stdin_limit_bytes = 64U * 1024U;
    candidate.stdout_limit_bytes = 64U * 1024U;
    candidate.stderr_limit_bytes = 4U * 1024U;

    PolyglotRuntimeCapabilityBinding binding{
        .capability_id = capability_id,
        .artifact_admission = admission,
        .candidate_request_template = std::move(candidate),
        .invoke_native = {},
        .normalize_shadow_parity = {},
        .parity_policy = {}};
    return {std::move(routes.registry), {std::move(binding)}};
}

PolyglotArtifactInvocationRequest candidate_request(
    const PolyglotBenchmarkInvocation& invocation) {
    PolyglotArtifactInvocationRequest request;
    request.invocation = {
        .capability_id = capability_id,
        .correlation_id = std::string("benchmark-") +
            invocation.workload->workload_id + "-" +
            polyglot_route_implementation_name(invocation.implementation) + "-" +
            std::to_string(invocation.iteration) +
            (invocation.warmup ? "-warmup" : "-measured"),
        .protocol_version = protocol_version,
        .arguments_json = invocation.workload->arguments_json};
    request.policy = {
        .timeout_ms = 5000U,
        .latency_budget_ms = 5000U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = PolyglotFallbackPolicy::fail_fast,
        .max_attempts = 1U};
    request.poll_interval_ms = 1U;
    request.stdin_limit_bytes = 64U * 1024U;
    request.stdout_limit_bytes = 64U * 1024U;
    request.stderr_limit_bytes = 4U * 1024U;
    return request;
}

std::uint64_t elapsed_microseconds(
    const std::chrono::steady_clock::time_point started) {
    const auto value = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - started).count();
    return static_cast<std::uint64_t>(std::max<std::int64_t>(1, value));
}

std::optional<std::uint64_t> parse_self_reported_peak_memory(
    const std::string_view document) {
    constexpr std::string_view prefix = "COPPERFIN_PEAK_MEMORY_KIB=";
    if (!document.starts_with(prefix) || document.size() <= prefix.size() + 1U ||
        document.back() != '\n') {
        return std::nullopt;
    }
    const std::string_view digits = document.substr(
        prefix.size(), document.size() - prefix.size() - 1U);
    std::uint64_t value = 0U;
    const auto parsed = std::from_chars(
        digits.data(), digits.data() + digits.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != digits.data() + digits.size() ||
        value == 0U) {
        return std::nullopt;
    }
    return value;
}

void test_representative_benchmark(
    PolyglotArtifactAdmissionResult& admission,
    const PublishedCandidate& published) {
    PolyglotBenchmarkRequest request{
        .capability_id = capability_id,
        .warmup_iterations = 1U,
        .measured_iterations = 3U,
        .policy = {
            .minimum_sample_count = 9U,
            .maximum_p95_latency_us = 5'000'000U,
            .minimum_throughput_per_second = 1U,
            .maximum_peak_memory_kib = 1'048'576U,
            .maximum_p95_startup_ms = 5000U,
            .maximum_security_profile =
                PolyglotRouteSecurityProfile::admitted_process,
            .weights = {25U, 25U, 25U, 25U}},
        .workloads = {
            {"positive", R"({"left":20,"right":22})", R"({"sum":42})"},
            {"negative", R"({"left":-20,"right":-22})", R"({"sum":-42})"},
            {"zero", R"({"left":0,"right":0})", R"({"sum":0})"}},
        .routes = {
            {PolyglotRouteImplementation::direct_cpp,
             PolyglotRouteSecurityProfile::trusted_native, true, true, true},
            {PolyglotRouteImplementation::cpp_dotnet_wrapper,
             PolyglotRouteSecurityProfile::admitted_process, true, true, true},
            {PolyglotRouteImplementation::csharp_service,
             PolyglotRouteSecurityProfile::admitted_process, true, true, true}},
        .invoke = {}};

    request.invoke = [&](const PolyglotBenchmarkInvocation& invocation) {
        const auto started = std::chrono::steady_clock::now();
        if (invocation.implementation ==
            PolyglotRouteImplementation::direct_cpp) {
            return PolyglotBenchmarkObservation{
                true, true, elapsed_microseconds(started), 0U, 0U};
        }

        auto candidate = candidate_request(invocation);
        candidate.working_directory = utf8_path(published.root);
        candidate.environment = {{"COPPERFIN_BENCHMARK_SELF_METRICS", "1"}};
        if (invocation.implementation ==
            PolyglotRouteImplementation::cpp_dotnet_wrapper) {
            const auto result = invoke_polyglot_artifact(admission, candidate);
            const auto peak_memory = parse_self_reported_peak_memory(
                result.process.standard_error);
            return PolyglotBenchmarkObservation{
                result.ok() && peak_memory.has_value(),
                result.ok() && peak_memory.has_value() &&
                    result.response.envelope.payload_json ==
                    invocation.workload->expected_payload_json,
                elapsed_microseconds(started),
                peak_memory.value_or(0U),
                result.process.elapsed_ms};
        }

        const auto serialized = serialize_polyglot_invocation_request(
            candidate.invocation);
        if (!serialized.ok()) {
            return PolyglotBenchmarkObservation{
                false, false, elapsed_microseconds(started), 0U, 0U};
        }
        const auto process = run_bounded_process({
            .executable_path = utf8_path(published.executable),
            .arguments = {},
            .working_directory = utf8_path(published.root),
            .environment = candidate.environment,
            .standard_input = serialized.document,
            .timeout_ms = 5000U,
            .poll_interval_ms = 1U,
            .stdin_limit_bytes = 64U * 1024U,
            .stdout_limit_bytes = 64U * 1024U,
            .stderr_limit_bytes = 4U * 1024U,
            .cancellation_requested = {}});
        const auto parsed = parse_polyglot_interop_envelope(
            process.standard_output,
            {capability_id, candidate.invocation.correlation_id,
             protocol_version, 64U * 1024U, 32U});
        const bool succeeded = process.completed() && process.exit_code == 0 &&
            process.process_tree_closed && parsed.ok();
        const auto peak_memory = parse_self_reported_peak_memory(
            process.standard_error);
        return PolyglotBenchmarkObservation{
            succeeded && peak_memory.has_value(),
            succeeded && peak_memory.has_value() &&
                parsed.envelope.payload_json ==
                invocation.workload->expected_payload_json,
            elapsed_microseconds(started),
            peak_memory.value_or(0U),
            process.elapsed_ms};
    };

    const auto result = run_polyglot_benchmark(request);
    expect_local(result.ok() && result.impact.recommendation_ready,
                 "representative native/.NET measurements should be advisory-ready");
    expect_local(result.measurements.size() == 3U,
                 "the representative benchmark should retain all route layers");
    for (const auto& measurement : result.measurements) {
        expect_local(
            measurement.sample_count == 9U &&
                measurement.failure_count == 0U &&
                measurement.parity_mismatch_count == 0U &&
                measurement.p95_latency_us > 0U &&
                measurement.throughput_per_second > 0U,
            std::string("representative evidence should be complete for ") +
                polyglot_route_implementation_name(measurement.implementation));
        if (measurement.implementation !=
            PolyglotRouteImplementation::direct_cpp) {
            expect_local(
                measurement.peak_memory_kib > 0U,
                "each external process layer should report peak resident memory");
        }
    }
    std::cout << "COPPERFIN_POLYGLOT_BENCHMARK_V1={\"capability_id\":\""
              << capability_id
              << "\",\"warmup_iterations\":1,\"measured_iterations\":3,"
                 "\"workload_count\":3,\"measurements\":[";
    for (std::size_t index = 0U; index < result.measurements.size(); ++index) {
        const auto& measurement = result.measurements[index];
        if (index != 0U) {
            std::cout << ',';
        }
        std::cout << "{\"implementation\":\""
                  << polyglot_route_implementation_name(measurement.implementation)
                  << "\",\"security_profile\":\""
                  << polyglot_route_security_profile_name(measurement.security_profile)
                  << "\",\"sample_count\":" << measurement.sample_count
                  << ",\"failure_count\":" << measurement.failure_count
                  << ",\"parity_mismatch_count\":"
                  << measurement.parity_mismatch_count
                  << ",\"p95_latency_us\":" << measurement.p95_latency_us
                  << ",\"throughput_per_second\":"
                  << measurement.throughput_per_second
                  << ",\"peak_memory_kib\":" << measurement.peak_memory_kib
                  << ",\"p95_startup_ms\":" << measurement.p95_startup_ms
                  << '}';
    }
    std::cout << "],\"recommendation_ready\":"
              << (result.impact.recommendation_ready ? "true" : "false")
              << ",\"preferred\":\""
              << polyglot_route_implementation_name(result.impact.preferred)
              << "\",\"fallback_chain\":[";
    for (std::size_t index = 0U;
         index < result.impact.fallback_chain.size(); ++index) {
        if (index != 0U) {
            std::cout << ',';
        }
        std::cout << '\"'
                  << polyglot_route_implementation_name(
                         result.impact.fallback_chain[index])
                  << '\"';
    }
    std::cout << "]}\n";
}

void test_publish_is_one_runtime_artifact(
    const PublishedCandidate& published) {
    expect_local(fs::is_regular_file(published.executable),
                 "Native AOT publish should produce the configured executable");
    for (const auto& entry : fs::directory_iterator(published.root)) {
        if (!entry.is_regular_file() || entry.path() == published.executable) {
            continue;
        }
        const std::string extension = entry.path().extension().string();
        expect_local(
            extension == ".dbg" || extension == ".pdb",
            "publish output must not require a loose runtime or managed sidecar: " +
                entry.path().filename().string());
    }
}

void test_candidate_rejects_wrong_identity(
    const PublishedCandidate& published) {
    const auto expect_rejected = [&](std::string input, const std::string& message) {
        BoundedProcessRequest request;
        request.executable_path = utf8_path(published.executable);
        request.working_directory = utf8_path(published.root);
        request.standard_input = std::move(input);
        request.timeout_ms = 5000U;
        request.poll_interval_ms = 2U;
        request.stdin_limit_bytes = 2U * 1024U * 1024U;
        request.stdout_limit_bytes = 64U * 1024U;
        request.stderr_limit_bytes = 4U * 1024U;
        const auto result = run_bounded_process(request);
        expect_local(
            result.status == BoundedProcessStatus::exited &&
            result.exit_code == 2 && result.standard_output.empty() &&
            result.standard_error.empty() && result.process_tree_closed,
            message);
    };
    expect_rejected(
        R"json({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.dotnet.wrong-v1","correlation_id":"wrong-identity","protocol_version":"1.0.0","arguments":{"left":1,"right":2}})json",
        "the .NET candidate should fail closed without reflecting a wrong identity");
    expect_rejected(
        R"json({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.dotnet.add-v1","capability_id":"samples.dotnet.add-v1","correlation_id":"duplicate","protocol_version":"1.0.0","arguments":{"left":1,"right":2}})json",
        "the .NET candidate should reject duplicate identity fields");
    expect_rejected("{\"broken\"",
                    "the .NET candidate should reject malformed JSON quietly");
    expect_rejected(std::string(1024U * 1024U + 1U, 'x'),
                    "the .NET candidate should enforce its own one-MiB input bound");
}

void test_runtime_host_and_prg_dispatch(
    const PolyglotArtifactAdmissionResult& admission,
    const PublishedCandidate& published,
    const fs::path& fixture_root) {
    auto built = PolyglotRuntimeHost::create(
        configuration(admission, published.root));
    expect_local(built.ok(), "the .NET candidate should compose into the runtime host");
    if (!built.ok()) {
        return;
    }

    const auto callback = built.host->dispatch_callback();
    const auto success = callback({
        capability_id,
        R"json({"left":20,"right":22})json",
        0U,
        []() { return false; }});
    expect_local(
        success.status == RuntimePolyglotDispatchStatus::success &&
        success.authority == RuntimePolyglotDispatchAuthority::candidate &&
        success.selection == RuntimePolyglotDispatchSelection::candidate &&
        success.native_invocation_count == 0U &&
        success.candidate_invocation_count == 1U &&
        success.payload_json == R"json({"sum":42})json",
        "the admitted .NET candidate should return the authoritative sum once");

    const auto overflow = callback({
        capability_id,
        R"json({"left":9223372036854775807,"right":1})json",
        0U,
        []() { return false; }});
    expect_local(
        overflow.status == RuntimePolyglotDispatchStatus::candidate_failed &&
        overflow.error_code == "polyglot.adapter.candidate_error" &&
        overflow.authority == RuntimePolyglotDispatchAuthority::candidate &&
        overflow.native_invocation_count == 0U &&
        overflow.candidate_invocation_count == 1U &&
        overflow.payload_json.empty(),
        "a typed candidate error should remain bounded and non-authoritative");

    const fs::path program = fixture_root / "dotnet-candidate.prg";
    write_text(
        program,
        "cDispatch = CFPOLYGLOTDISPATCH('samples.dotnet.add-v1', "
        "'{\"left\":20,\"right\":22}', 0)\n"
        "cStatus = CFJSONGET(cDispatch, '/status')\n"
        "cAuthority = CFJSONGET(cDispatch, '/authority')\n"
        "nSum = CFJSONGET(cDispatch, '/payload/sum')\n"
        "nCandidateCalls = CFJSONGET(cDispatch, '/candidate_invocation_count')\n"
        "RETURN\n");
    auto options = make_runtime_session_options(program, fixture_root);
    options.polyglot_dispatch_callback = callback;
    auto session = PrgRuntimeSession::create(options);
    const auto state = session.run(DebugResumeAction::continue_run);
    const auto value = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        return found == state.globals.end()
            ? std::string{}
            : format_value(found->second);
    };
    expect_local(
        state.completed && value("cstatus") == "success" &&
        value("cauthority") == "candidate" && value("nsum") == "42" &&
        value("ncandidatecalls") == "1",
        "ordinary PRG should supervise and inspect the external C# leaf result");
    for (const auto& event : state.events) {
        if (event.category == "runtime.polyglot.dispatch") {
            expect_local(
                event.detail.find("\"left\"") == std::string::npos &&
                event.detail.find("dotnet-candidate-correlation") == std::string::npos,
                "PRG dispatch telemetry should redact .NET request and correlation bytes");
        }
    }
}

}  // namespace

int main() {
    PublishedCandidate published;
    published.executable = COPPERFIN_POLYGLOT_DOTNET_CANDIDATE_PATH;
    published.root = published.executable.parent_path();
    const fs::path fixture_root = unique_root();
    std::error_code error;
    fs::create_directories(fixture_root, error);
    expect_local(!error, "the .NET integration fixture root should be created");
    if (!error) {
        test_publish_is_one_runtime_artifact(published);
        test_candidate_rejects_wrong_identity(published);
        auto admission = admit_candidate(published);
        expect_local(admission.ok(), "the exact Native AOT candidate should be admitted");
        if (admission.ok()) {
            test_representative_benchmark(admission, published);
            test_runtime_host_and_prg_dispatch(
                admission, published, fixture_root);
        }
    }
    fs::remove_all(fixture_root, error);
    if (failures == 0) {
        std::cout << "All .NET polyglot candidate tests passed\n";
        return 0;
    }
    std::cerr << failures << " .NET polyglot candidate test(s) failed\n";
    return 1;
}
