// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_route_execution.h"
#include "copperfin/security/sha256.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

namespace {

using namespace copperfin::platform;

constexpr const char* capability_id = "interop.route-v1";
constexpr const char* correlation_id = "route-corr-001";
constexpr const char* protocol_version = "1.0.0";
constexpr const char* arguments_json = R"json({"value":"route"})json";
constexpr const char* expected_request =
    R"json({"envelope_version":"1.0","kind":"invocation","capability_id":"interop.route-v1","correlation_id":"route-corr-001","protocol_version":"1.0.0","arguments":{"value":"route"}})json";
constexpr const char* success_response =
    R"json({"envelope_version":"1.0","kind":"success","capability_id":"interop.route-v1","correlation_id":"route-corr-001","protocol_version":"1.0.0","payload":{"value":"ok"}})json";
constexpr const char* error_response =
    R"json({"envelope_version":"1.0","kind":"error","capability_id":"interop.route-v1","correlation_id":"route-corr-001","protocol_version":"1.0.0","error":{"code":"candidate.synthetic","message":"expected","retryable":false}})json";

int failures = 0;

void expect(const bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

void configure_binary_standard_streams() {
#if defined(_WIN32)
    (void)::_setmode(::_fileno(stdin), _O_BINARY);
    (void)::_setmode(::_fileno(stdout), _O_BINARY);
    (void)::_setmode(::_fileno(stderr), _O_BINARY);
#endif
}

int run_artifact(const std::string& mode) {
    configure_binary_standard_streams();
    const std::string request{
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()};
    if (request != expected_request) {
        return 23;
    }
    if (mode == "--route-success") {
        std::cout << success_response;
        return 0;
    }
    if (mode == "--route-error") {
        std::cout << error_response;
        return 0;
    }
    if (mode == "--route-sleep") {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << success_response;
        return 0;
    }
    return 24;
}

unsigned long process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long>(::GetCurrentProcessId());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

std::filesystem::path unique_root() {
    return std::filesystem::temp_directory_path() /
        ("copperfin_polyglot_route_execution_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

std::string utf8_path(const std::filesystem::path& path) {
    return copperfin::platform::path_to_utf8_string(path);
}

bool copy_executable(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    std::error_code error;
    std::filesystem::copy_file(
        source, destination,
        std::filesystem::copy_options::overwrite_existing, error);
    if (error) {
        return false;
    }
#if !defined(_WIN32)
    std::filesystem::permissions(
        destination,
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write |
            std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::add,
        error);
#endif
    return !error;
}

PolyglotArtifactAdmissionResult admit(
    const std::filesystem::path& artifact,
    const std::filesystem::path& root,
    const std::string& admitted_capability = capability_id) {
    const auto digest = copperfin::security::sha256_hex_for_file(
        utf8_path(artifact));
    expect(digest.ok, "route fixture should have a readable digest");
    return admit_polyglot_artifact({
        .capability_id = admitted_capability,
        .process_policy = {
            .executable_name = utf8_path(artifact),
            .allowed_path_roots = {utf8_path(root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

PolyglotArtifactInvocationRequest candidate(
    const std::filesystem::path& root,
    const std::string& mode,
    const PolyglotFallbackPolicy fallback = PolyglotFallbackPolicy::fail_fast) {
    return {
        .invocation = {
            .capability_id = capability_id,
            .correlation_id = correlation_id,
            .protocol_version = protocol_version,
            .arguments_json = arguments_json},
        .policy = {
            .timeout_ms = 5000U,
            .latency_budget_ms = 4500U,
            .cancellation = PolyglotCancellationPolicy::propagate,
            .fallback = fallback,
            .max_attempts = 1U},
        .artifact_arguments = {mode},
        .working_directory = utf8_path(root),
        .environment = {},
        .poll_interval_ms = 5U,
        .stdin_limit_bytes = 1024U * 1024U,
        .stdout_limit_bytes = 1024U * 1024U,
        .stderr_limit_bytes = 1024U * 1024U,
        .cancellation_requested = {}};
}

PolyglotRouteRegistry registry_for(
    const std::string& state,
    const std::uint8_t canary_percentage = 0U) {
    const auto loaded = load_polyglot_route_registry({
        {capability_id, state, canary_percentage}});
    expect(loaded.ok(), "route execution registry should load");
    return loaded.registry;
}

PolyglotNativeInvocationResult native_success() {
    return {true, {}, "native-ok"};
}

PolyglotShadowParityValues parity_values(
    const PolyglotNativeInvocationResult& native,
    const PolyglotArtifactInvocationResult& artifact,
    const std::string& candidate_value = "ok") {
    return {
        {{"$.value", "string", "string",
          native.success ? "ok" : "", artifact.ok() ? candidate_value : ""}},
        {"$.value"},
        {"$.value"}};
}

bool has_event(
    const PolyglotRouteExecutionResult& result,
    const std::string& category) {
    for (const auto& event : result.telemetry.events) {
        if (event.category == category) {
            return true;
        }
    }
    return false;
}

const PolyglotMigrationEvent* find_event(
    const PolyglotRouteExecutionResult& result,
    const std::string& category) {
    for (const auto& event : result.telemetry.events) {
        if (event.category == category) {
            return &event;
        }
    }
    return nullptr;
}

PolyglotRouteExecutionRequest base_request(
    const PolyglotRouteRegistry& routes,
    PolyglotArtifactAdmissionResult* admission,
    const std::filesystem::path& root,
    const std::string& mode = "--route-success") {
    PolyglotRouteExecutionRequest request;
    request.registry = &routes;
    request.capability_id = capability_id;
    request.artifact_admission = admission;
    request.candidate_request = candidate(root, mode);
    request.invoke_native = native_success;
    request.normalize_shadow_parity = [](
        const PolyglotNativeInvocationResult& native,
        const PolyglotArtifactInvocationResult& artifact) {
        return parity_values(native, artifact);
    };
    return request;
}

void test_native_and_canary_routes(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    const auto off_routes = registry_for("off");
    const auto canary_routes = registry_for("canary", 50U);
    const std::thread::id calling_thread = std::this_thread::get_id();
    std::thread::id native_thread;
    auto off_request = base_request(off_routes, nullptr, root);
    off_request.invoke_native = [&]() {
        native_thread = std::this_thread::get_id();
        return native_success();
    };
    const auto off = execute_polyglot_route(off_request);
    expect(off.ok() && off.authority == PolyglotRouteResultAuthority::native &&
               off.native_invocation_count == 1U &&
               off.candidate_invocation_count == 0U &&
               native_thread == calling_thread,
           "#4937: off should synchronously invoke only native on the caller thread");
    const auto* off_completion = find_event(
        off, "polyglot.execution.completed");
    expect(off.telemetry.events.size() == 2U && off_completion != nullptr &&
               off_completion->capability_id == capability_id &&
               off_completion->reason_code ==
                   "polyglot.execution.native_success" &&
               off_completion->detail == "native" &&
               std::string(polyglot_route_execution_status_name(off.status)) ==
                   "success" &&
               std::string(polyglot_route_result_authority_name(off.authority)) ==
                   "native",
           "#4937: native completion evidence and public names should be invariant");

    const PolyglotRouteRegistry empty_routes;
    auto default_off_request = base_request(empty_routes, nullptr, root);
    const auto default_off = execute_polyglot_route(default_off_request);
    expect(default_off.ok() &&
               default_off.route.reason_code == "polyglot.route.default_off" &&
               default_off.native_invocation_count == 1U &&
               default_off.candidate_invocation_count == 0U,
           "#4937: an absent capability should retain the registry's safe default-off path");

    auto canary_native_request = base_request(
        canary_routes, nullptr, root);
    canary_native_request.selection_sample = 50U;
    const auto canary_native = execute_polyglot_route(canary_native_request);
    expect(canary_native.ok() &&
               canary_native.route.selection == PolyglotRouteSelection::native &&
               canary_native.native_invocation_count == 1U &&
               canary_native.candidate_invocation_count == 0U,
           "#4937: canary percentage boundary should remain native");

    auto canary_candidate_request = base_request(
        canary_routes, &admission, root);
    canary_candidate_request.selection_sample = 49U;
    const auto canary_candidate = execute_polyglot_route(canary_candidate_request);
    expect(canary_candidate.ok() &&
               canary_candidate.authority == PolyglotRouteResultAuthority::candidate &&
               canary_candidate.native_invocation_count == 0U &&
               canary_candidate.candidate_invocation_count == 1U,
           "#4937: sample below the canary boundary should invoke candidate once");
}

void test_shadow_routes(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    const auto routes = registry_for("shadow");
    auto matching_request = base_request(
        routes, &admission, root);
    const auto matching = execute_polyglot_route(matching_request);
    expect(matching.ok() &&
               matching.authority == PolyglotRouteResultAuthority::native &&
               matching.native.payload == "native-ok" &&
               matching.native_invocation_count == 1U &&
               matching.candidate_invocation_count == 1U &&
               matching.parity_evaluated && matching.parity.parity_match,
           "#4937: shadow match should invoke both once and keep native authoritative");
    expect(has_event(matching, "polyglot.parity.checked"),
           "#4937: shadow match should emit deterministic parity evidence");

    auto mismatch_request = matching_request;
    mismatch_request.normalize_shadow_parity = [](
        const PolyglotNativeInvocationResult& native,
        const PolyglotArtifactInvocationResult& artifact) {
        return parity_values(native, artifact, "different");
    };
    const auto mismatch = execute_polyglot_route(mismatch_request);
    expect(mismatch.ok() &&
               mismatch.authority == PolyglotRouteResultAuthority::native &&
               mismatch.parity_evaluated && !mismatch.parity.parity_match &&
               has_event(mismatch, "polyglot.parity.mismatch"),
           "#4937: shadow mismatch should remain native and record the mismatch");

    auto candidate_failure = base_request(
        routes, &admission, root, "--route-error");
    const auto failed_candidate = execute_polyglot_route(candidate_failure);
    expect(failed_candidate.ok() &&
               failed_candidate.authority == PolyglotRouteResultAuthority::native &&
               failed_candidate.parity.first_mismatch ==
                   PolyglotParityMismatchCategory::candidate_failure,
           "#4937: shadow candidate failure should not displace native authority");

    auto native_failure = matching_request;
    native_failure.invoke_native = [] {
        return PolyglotNativeInvocationResult{
            false, "native.synthetic", {}};
    };
    const auto failed_native = execute_polyglot_route(native_failure);
    expect(failed_native.status == PolyglotRouteExecutionStatus::native_failed &&
               failed_native.authority == PolyglotRouteResultAuthority::native &&
               failed_native.candidate_invocation_count == 1U &&
               failed_native.parity.first_mismatch ==
                   PolyglotParityMismatchCategory::native_failure,
           "#4937: shadow should return a failed native result even when candidate succeeds");
}

void test_candidate_and_fallback_routes(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    const auto on_routes = registry_for("on");
    const auto retired_routes = registry_for("retire-legacy");
    auto on = base_request(on_routes, &admission, root);
    const auto candidate_success = execute_polyglot_route(on);
    expect(candidate_success.ok() &&
               candidate_success.authority == PolyglotRouteResultAuthority::candidate &&
               candidate_success.native_invocation_count == 0U &&
               candidate_success.candidate_invocation_count == 1U,
           "#4937: on should return one successful candidate invocation");

    auto fallback = base_request(
        on_routes, &admission, root, "--route-error");
    fallback.candidate_request.policy.fallback =
        PolyglotFallbackPolicy::fallback_native;
    const auto native_fallback = execute_polyglot_route(fallback);
    const auto* executed = find_event(
        native_fallback, "polyglot.fallback.executed");
    expect(native_fallback.ok() && native_fallback.native_fallback_executed &&
               native_fallback.authority == PolyglotRouteResultAuthority::native &&
               native_fallback.native_invocation_count == 1U &&
               native_fallback.candidate_invocation_count == 1U &&
               executed != nullptr &&
               executed->reason_code ==
                   "polyglot.execution.native_fallback_success" &&
               executed->detail == "native" && executed->successful,
           "#4937: permitted native fallback should execute exactly once and be explicit");

    auto fallback_failure = fallback;
    fallback_failure.invoke_native = [] {
        return PolyglotNativeInvocationResult{false, "native.fallback.failed", {}};
    };
    const auto failed_fallback = execute_polyglot_route(fallback_failure);
    expect(failed_fallback.status == PolyglotRouteExecutionStatus::native_failed &&
               failed_fallback.native_fallback_executed &&
               failed_fallback.native_invocation_count == 1U &&
               failed_fallback.candidate_invocation_count == 1U,
           "#4937: a failed native fallback should remain bounded and authoritative");

    auto fail_fast = base_request(
        on_routes, &admission, root, "--route-error");
    const auto failed = execute_polyglot_route(fail_fast);
    expect(failed.status == PolyglotRouteExecutionStatus::candidate_failed &&
               failed.authority == PolyglotRouteResultAuthority::candidate &&
               failed.native_invocation_count == 0U &&
               failed.candidate_invocation_count == 1U,
           "#4937: candidate fail-fast should not invoke native");

    auto second_artifact = fail_fast;
    second_artifact.candidate_request.policy.fallback =
        PolyglotFallbackPolicy::fallback_artifact;
    const auto unsupported = execute_polyglot_route(second_artifact);
    expect(unsupported.error_code ==
               "polyglot.execution.artifact_fallback_unsupported" &&
               unsupported.native_invocation_count == 0U &&
               unsupported.candidate_invocation_count == 1U &&
               has_event(unsupported, "polyglot.fallback.applied") &&
               !has_event(unsupported, "polyglot.fallback.executed"),
           "#4937: second-artifact fallback should be reported but never executed");

    auto retired = base_request(
        retired_routes, &admission, root, "--route-error");
    retired.candidate_request.policy.fallback =
        PolyglotFallbackPolicy::fallback_native;
    const auto retired_failure = execute_polyglot_route(retired);
    expect(retired_failure.status == PolyglotRouteExecutionStatus::candidate_failed &&
               !retired_failure.native_fallback_executed &&
               retired_failure.native_invocation_count == 0U &&
               retired_failure.candidate_invocation_count == 1U &&
               !has_event(retired_failure, "polyglot.fallback.executed"),
           "#4937: retire-legacy must never return to native");
}

void test_timeout_and_cancellation(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    const auto routes = registry_for("on");
    auto timeout = base_request(
        routes, &admission, root, "--route-sleep");
    timeout.candidate_request.policy.timeout_ms = 50U;
    timeout.candidate_request.policy.latency_budget_ms = 40U;
    timeout.candidate_request.policy.fallback =
        PolyglotFallbackPolicy::fallback_native;
    const auto timed_out = execute_polyglot_route(timeout);
    expect(timed_out.ok() && timed_out.native_fallback_executed &&
               timed_out.candidate.process.status == BoundedProcessStatus::timed_out &&
               timed_out.native_invocation_count == 1U &&
               timed_out.candidate_invocation_count == 1U,
           "#4937: timed-out candidate should close then use one permitted native fallback");

    std::atomic_uint polls{0U};
    auto cancellation = base_request(
        routes, &admission, root, "--route-sleep");
    cancellation.candidate_request.cancellation_requested = [&polls]() {
        return polls.fetch_add(1U) >= 2U;
    };
    const auto cancelled = execute_polyglot_route(cancellation);
    expect(cancelled.status == PolyglotRouteExecutionStatus::cancelled &&
               cancelled.authority == PolyglotRouteResultAuthority::candidate &&
               cancelled.native_invocation_count == 0U &&
               cancelled.candidate_invocation_count == 1U &&
               cancelled.candidate.process.process_tree_closed,
           "#4937: propagated cancellation should close candidate and never fall back");
}

void test_fail_closed_configuration(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    const auto off_routes = registry_for("off");
    const auto on_routes = registry_for("on");
    const auto shadow_routes = registry_for("shadow");
    const auto canary_routes = registry_for("canary", 50U);
    PolyglotRouteExecutionRequest absent;
    const auto no_registry = execute_polyglot_route(absent);
    expect(no_registry.error_code == "polyglot.execution.registry_required" &&
               no_registry.telemetry.events.empty(),
           "#4937: missing registry should fail before trusted telemetry");

    auto invalid_id = base_request(off_routes, nullptr, root);
    invalid_id.capability_id = "INVALID/ROUTE";
    const auto invalid = execute_polyglot_route(invalid_id);
    expect(invalid.error_code == "polyglot.execution.invalid_capability_id" &&
               invalid.native_invocation_count == 0U &&
               invalid.telemetry.events.empty(),
           "#4937: invalid capability should fail before callbacks or telemetry");

    PolyglotRouteRegistry malformed_registry;
    malformed_registry.entries = {
        {capability_id, PolyglotRouteState::on, 0U},
        {capability_id, PolyglotRouteState::off, 0U}};
    auto invalid_registry = base_request(
        malformed_registry, &admission, root);
    const auto malformed = execute_polyglot_route(invalid_registry);
    expect(malformed.error_code == "polyglot.execution.invalid_registry" &&
               malformed.native_invocation_count == 0U &&
               malformed.candidate_invocation_count == 0U &&
               malformed.telemetry.events.empty(),
           "#4937: malformed in-memory registry should fail before either path runs");

    auto invalid_sample = base_request(canary_routes, nullptr, root);
    invalid_sample.selection_sample = 100U;
    const auto sample = execute_polyglot_route(invalid_sample);
    expect(sample.error_code == "polyglot.execution.invalid_selection_sample" &&
               sample.native_invocation_count == 0U,
           "#4937: selection samples outside 0..99 should fail closed");

    auto missing_admission = base_request(on_routes, nullptr, root);
    const auto no_admission = execute_polyglot_route(missing_admission);
    expect(no_admission.error_code ==
               "polyglot.execution.artifact_admission_required" &&
               no_admission.candidate_invocation_count == 0U,
           "#4937: a candidate route should require an admission token");

    auto missing_native = base_request(off_routes, nullptr, root);
    missing_native.invoke_native = {};
    const auto no_native = execute_polyglot_route(missing_native);
    expect(no_native.error_code == "polyglot.execution.native_invoker_required" &&
               no_native.native_invocation_count == 0U,
           "#4937: a native route should require a native callback");

    auto missing_normalizer = base_request(
        shadow_routes, &admission, root);
    missing_normalizer.normalize_shadow_parity = {};
    const auto no_normalizer = execute_polyglot_route(missing_normalizer);
    expect(no_normalizer.error_code ==
               "polyglot.execution.shadow_normalizer_required" &&
               no_normalizer.native_invocation_count == 0U &&
               no_normalizer.candidate_invocation_count == 0U,
           "#4937: shadow should validate its normalizer before either path runs");

    auto native_exception = base_request(off_routes, nullptr, root);
    native_exception.invoke_native = []() -> PolyglotNativeInvocationResult {
        throw std::runtime_error("synthetic");
    };
    const auto thrown_native = execute_polyglot_route(native_exception);
    expect(thrown_native.status == PolyglotRouteExecutionStatus::native_failed &&
               thrown_native.error_code == "polyglot.execution.native_exception" &&
               thrown_native.native_invocation_count == 1U,
           "#4937: a native callback exception should fail closed without retry");

    auto normalizer_exception = base_request(
        shadow_routes, &admission, root);
    normalizer_exception.normalize_shadow_parity = [](
        const PolyglotNativeInvocationResult&,
        const PolyglotArtifactInvocationResult&) -> PolyglotShadowParityValues {
        throw std::runtime_error("synthetic");
    };
    const auto thrown_normalizer = execute_polyglot_route(normalizer_exception);
    expect(thrown_normalizer.status == PolyglotRouteExecutionStatus::parity_failed &&
               thrown_normalizer.authority == PolyglotRouteResultAuthority::native &&
               thrown_normalizer.native.payload == "native-ok" &&
               thrown_normalizer.native_invocation_count == 1U &&
               thrown_normalizer.candidate_invocation_count == 1U,
           "#4937: shadow normalizer failure should preserve the native result and fail closed");
}

void test_adapter_identity_cannot_be_bypassed(
    PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root,
    const std::filesystem::path& artifact) {
    const auto routes = registry_for("on");
    auto mismatch = base_request(routes, &admission, root);
    mismatch.candidate_request.invocation.capability_id = "interop.other-v1";
    const auto denied = execute_polyglot_route(mismatch);
    expect(denied.status == PolyglotRouteExecutionStatus::candidate_failed &&
               denied.candidate.error_code ==
                   "polyglot.adapter.capability_id_mismatch" &&
               !denied.candidate.process.started &&
               denied.native_invocation_count == 0U &&
               denied.candidate_invocation_count == 1U,
           "#4937: coordinator must preserve adapter capability identity rejection");

    auto wrong_hash = admit(artifact, root);
    {
        std::ofstream output(artifact, std::ios::binary | std::ios::app);
        output.put('\0');
    }
    auto changed = base_request(routes, &wrong_hash, root);
    const auto revoked = execute_polyglot_route(changed);
    expect(revoked.status == PolyglotRouteExecutionStatus::candidate_failed &&
               !revoked.candidate.process.started &&
               !wrong_hash.ok() &&
               revoked.native_invocation_count == 0U &&
               revoked.candidate_invocation_count == 1U,
           "#4937: coordinator must preserve adapter byte-identity revalidation");
}

int run_tests(const std::filesystem::path& running_executable) {
    const auto root = unique_root();
    std::error_code error;
    std::filesystem::create_directories(root, error);
    expect(!error, "route execution fixture should create its root");
    if (error) {
        return 1;
    }

    const auto artifact = root / running_executable.filename();
    expect(copy_executable(running_executable, artifact),
           "route execution fixture should copy its executable");
    auto admission = admit(artifact, root);
    expect(admission.ok(), "route execution fixture should admit its executable");
    if (admission.ok()) {
        test_native_and_canary_routes(admission, root);
        test_shadow_routes(admission, root);
        test_candidate_and_fallback_routes(admission, root);
        test_timeout_and_cancellation(admission, root);
        test_fail_closed_configuration(admission, root);
        test_adapter_identity_cannot_be_bypassed(
            admission, root, artifact);
    }

    std::filesystem::remove_all(root, error);
    expect(!error, "route execution fixture should remove its root");
    return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]).rfind("--route-", 0U) == 0U) {
            return run_artifact(argv[1]);
        }
        const auto executable = resolve_running_executable_path(
            argc > 0 ? std::filesystem::path(argv[0]) : std::filesystem::path{});
        if (executable.empty()) {
            std::cerr << "FAIL: could not resolve route test executable\n";
            return 1;
        }
        return run_tests(executable);
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: unexpected exception: " << exception.what() << '\n';
        return 1;
    }
}
