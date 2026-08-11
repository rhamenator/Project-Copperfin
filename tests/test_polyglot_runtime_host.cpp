// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/polyglot_runtime_host.h"
#include "copperfin/security/sha256.h"
#include "copperfin/platform/executable_path.h"
#include "prg_engine_test_support.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

using namespace copperfin::platform;
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;

constexpr const char* capability_id = "interop.runtime-host-v1";
constexpr const char* protocol_version = "1.0.0";
int failures = 0;

void expect_local(const bool condition, const std::string& message) {
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

std::string extract_json_string(
    const std::string& document,
    const std::string& field) {
    const std::string prefix = "\"" + field + "\":\"";
    const std::size_t begin = document.find(prefix);
    if (begin == std::string::npos) {
        return {};
    }
    const std::size_t value_begin = begin + prefix.size();
    const std::size_t end = document.find('"', value_begin);
    return end == std::string::npos
        ? std::string{}
        : document.substr(value_begin, end - value_begin);
}

int run_artifact(
    const std::string& mode,
    const std::string& ready_path = {}) {
    configure_binary_standard_streams();
    const std::string request{
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()};
    if (mode == "--host-sleep") {
        if (!ready_path.empty()) {
            std::ofstream ready(
                copperfin::platform::path_from_utf8_string(ready_path),
                std::ios::binary | std::ios::trunc);
            ready << "ready\n";
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 0;
    }
    if (mode == "--host-error") {
        return 42;
    }
    const std::string correlation = extract_json_string(
        request, "correlation_id");
    if (extract_json_string(request, "capability_id") != capability_id ||
        extract_json_string(request, "protocol_version") != protocol_version ||
        correlation.empty() ||
        request.find("\"arguments\":{\"value\":7}") == std::string::npos) {
        return 41;
    }
    std::cout
        << "{\"envelope_version\":\"1.0\",\"kind\":\"success\","
        << "\"capability_id\":\"" << capability_id << "\","
        << "\"correlation_id\":\"" << correlation << "\","
        << "\"protocol_version\":\"" << protocol_version << "\","
        << "\"payload\":{\"source\":\"candidate\",\"value\":7,"
        << "\"correlation_id\":\"" << correlation << "\"}}";
    return 0;
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
        ("copperfin_polyglot_runtime_host_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

std::string utf8_path(const fs::path& path) {
    return copperfin::platform::path_to_utf8_string(path);
}

bool copy_executable(const fs::path& source, const fs::path& destination) {
    std::error_code error;
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, error);
    if (error) {
        return false;
    }
#if !defined(_WIN32)
    fs::permissions(
        destination,
        fs::perms::owner_read | fs::perms::owner_write | fs::perms::owner_exec,
        fs::perm_options::add,
        error);
#endif
    return !error;
}

PolyglotArtifactAdmissionResult admit(
    const fs::path& artifact,
    const fs::path& root,
    const std::string& admitted_capability = capability_id) {
    const auto digest = copperfin::security::sha256_hex_for_file(
        utf8_path(artifact));
    expect_local(digest.ok, "runtime-host fixture should have a readable digest");
    return admit_polyglot_artifact({
        .capability_id = admitted_capability,
        .process_policy = {
            .executable_name = utf8_path(artifact),
            .allowed_path_roots = {utf8_path(root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

PolyglotRuntimeHostConfiguration configuration(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root,
    const std::string& state,
    const std::string& artifact_mode = "--host-success",
    const std::uint8_t canary_percentage = 0U,
    const PolyglotFallbackPolicy fallback = PolyglotFallbackPolicy::fail_fast,
    const PolyglotCancellationPolicy cancellation =
        PolyglotCancellationPolicy::propagate,
    const std::string& ready_path = {}) {
    auto routes = load_polyglot_route_registry({
        {capability_id, state, canary_percentage}});
    expect_local(routes.ok(), "runtime-host fixture route should load");
    PolyglotArtifactInvocationRequest candidate;
    candidate.invocation = {
        .capability_id = capability_id,
        .correlation_id = "runtime-host-correlation",
        .protocol_version = protocol_version,
        .arguments_json = {}};
    candidate.policy = {
        .timeout_ms = 3000U,
        .latency_budget_ms = 2500U,
        .cancellation = cancellation,
        .fallback = fallback,
        .max_attempts = 1U};
    candidate.artifact_arguments = {artifact_mode};
    if (!ready_path.empty()) {
        candidate.artifact_arguments.push_back(ready_path);
    }
    candidate.working_directory = utf8_path(root);
    candidate.poll_interval_ms = 2U;
    PolyglotRuntimeCapabilityBinding binding{
        .capability_id = capability_id,
        .artifact_admission = admission,
        .supporting_artifact_admissions = {},
        .supporting_artifact_arguments = {},
        .candidate_request_template = std::move(candidate),
        .invoke_native = []() {
            return PolyglotNativeInvocationResult{
                true, {}, R"json({"source":"native","value":7})json"};
        },
        .normalize_shadow_parity = [](
            const PolyglotNativeInvocationResult& native,
            const PolyglotArtifactInvocationResult& candidate_result) {
            return PolyglotShadowParityValues{
                {{"$.value", "number", "number",
                  native.success ? "7" : "",
                  candidate_result.ok() ? "7" : ""}},
                {"$.value"},
                {"$.value"}};
        },
        .parity_policy = {}};
    return {std::move(routes.registry), {std::move(binding)}};
}

RuntimePolyglotDispatchResult dispatch_once(
    const std::shared_ptr<const PolyglotRuntimeHost>& host,
    const std::uint8_t sample = 0U,
    std::function<bool()> cancellation = []() { return false; }) {
    return host->dispatch_callback()({
        capability_id,
        R"json({"value":7})json",
        sample,
        std::move(cancellation)});
}

void test_route_states_and_owned_callback_lifetime(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    auto off = PolyglotRuntimeHost::create(
        configuration(admission, root, "off"));
    expect_local(off.ok(), "off runtime host should compose");
    const auto off_result = dispatch_once(off.host);
    expect_local(
        off_result.status == RuntimePolyglotDispatchStatus::success &&
        off_result.selection == RuntimePolyglotDispatchSelection::native &&
        off_result.authority == RuntimePolyglotDispatchAuthority::native &&
        off_result.native_invocation_count == 1U &&
        off_result.candidate_invocation_count == 0U,
        "off should preserve exact native route evidence");

    auto shadow = PolyglotRuntimeHost::create(
        configuration(admission, root, "shadow"));
    const auto shadow_result = dispatch_once(shadow.host);
    expect_local(
        shadow_result.status == RuntimePolyglotDispatchStatus::success &&
        shadow_result.selection == RuntimePolyglotDispatchSelection::shadow &&
        shadow_result.authority == RuntimePolyglotDispatchAuthority::native &&
        shadow_result.native_invocation_count == 1U &&
        shadow_result.candidate_invocation_count == 1U,
        "shadow should preserve native authority and exact invocation counts");

    auto canary = PolyglotRuntimeHost::create(
        configuration(admission, root, "canary", "--host-success", 50U));
    expect_local(
        dispatch_once(canary.host, 50U).selection ==
            RuntimePolyglotDispatchSelection::native,
        "canary boundary should remain native");
    const auto canary_candidate = dispatch_once(canary.host, 49U);
    expect_local(
        canary_candidate.selection == RuntimePolyglotDispatchSelection::candidate &&
        canary_candidate.authority == RuntimePolyglotDispatchAuthority::candidate &&
        canary_candidate.payload_json.find("candidate") != std::string::npos,
        "canary-selected candidate should preserve candidate authority and payload");

    for (const char* state : {"on", "retire-legacy"}) {
        auto built = PolyglotRuntimeHost::create(
            configuration(admission, root, state));
        auto callback = built.host->dispatch_callback();
        built.host.reset();
        const auto result = callback({
            capability_id, R"json({"value":7})json", 0U, []() { return false; }});
        expect_local(
            result.status == RuntimePolyglotDispatchStatus::success &&
            result.selection == RuntimePolyglotDispatchSelection::candidate &&
            result.authority == RuntimePolyglotDispatchAuthority::candidate &&
            result.native_invocation_count == 0U &&
            result.candidate_invocation_count == 1U,
            std::string(state) +
                " callback should own trusted state beyond the host handle lifetime");
    }
}

void test_configuration_fails_before_launch(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    auto missing = configuration(admission, root, "on");
    missing.capabilities.clear();
    expect_local(
        PolyglotRuntimeHost::create(std::move(missing)).error_code ==
            "polyglot.host.capability_binding_required",
        "a route without a capability binding should fail construction");

    auto duplicate = configuration(admission, root, "on");
    duplicate.capabilities.push_back(duplicate.capabilities.front());
    expect_local(
        PolyglotRuntimeHost::create(std::move(duplicate)).error_code ==
            "polyglot.host.duplicate_capability_binding",
        "duplicate capability bindings should fail construction");

    auto mismatch = configuration(admission, root, "on");
    mismatch.capabilities.front().candidate_request_template.invocation.capability_id =
        "interop.other-v1";
    expect_local(
        PolyglotRuntimeHost::create(std::move(mismatch)).error_code ==
            "polyglot.host.capability_binding_mismatch",
        "route, admission, and invocation capabilities should match exactly");

    auto invalid_protocol = configuration(admission, root, "on");
    invalid_protocol.capabilities.front()
        .candidate_request_template.invocation.protocol_version = "1.0";
    expect_local(
        PolyglotRuntimeHost::create(std::move(invalid_protocol)).error_code ==
            "polyglot.host.invalid_candidate_configuration",
        "invalid protocol identity should fail construction");

    auto invalid_policy = configuration(admission, root, "on");
    invalid_policy.capabilities.front().candidate_request_template.policy.timeout_ms = 0U;
    expect_local(
        PolyglotRuntimeHost::create(std::move(invalid_policy)).error_code ==
            "polyglot.host.invalid_candidate_configuration",
        "invalid bridge policy should fail construction");

    auto mutable_binding = configuration(admission, root, "on");
    mutable_binding.capabilities.front()
        .candidate_request_template.invocation.arguments_json = "{}";
    expect_local(
        PolyglotRuntimeHost::create(std::move(mutable_binding)).error_code ==
            "polyglot.host.mutable_request_binding_forbidden",
        "trusted configuration must not pre-bind mutable PRG request data");

    auto invalid_route = configuration(admission, root, "on");
    invalid_route.route_registry.entries.front().state =
        static_cast<PolyglotRouteState>(255);
    expect_local(
        PolyglotRuntimeHost::create(std::move(invalid_route)).error_code ==
            "polyglot.host.invalid_route_registry",
        "an invalid in-memory route state should fail construction");

    auto absent_admission = configuration(admission, root, "on");
    absent_admission.capabilities.front().artifact_admission =
        admit_polyglot_artifact({
            .capability_id = {},
            .process_policy = {},
            .expected_sha256 = {}});
    expect_local(
        PolyglotRuntimeHost::create(std::move(absent_admission)).error_code ==
            "polyglot.host.artifact_admission_required",
        "an absent or rejected admission token should fail construction");

    auto native_gap = configuration(admission, root, "off");
    native_gap.capabilities.front().invoke_native = {};
    expect_local(
        PolyglotRuntimeHost::create(std::move(native_gap)).error_code ==
            "polyglot.host.native_invoker_required",
        "a route that can invoke native work should require its trusted callback");

    auto callback_gap = configuration(admission, root, "shadow");
    callback_gap.capabilities.front().normalize_shadow_parity = {};
    expect_local(
        PolyglotRuntimeHost::create(std::move(callback_gap)).error_code ==
            "polyglot.host.shadow_normalizer_required",
        "shadow callback gaps should fail construction before execution");

    auto extra_binding = configuration(admission, root, "on");
    auto extra = extra_binding.capabilities.front();
    extra.capability_id = "interop.extra-v1";
    extra_binding.capabilities.push_back(std::move(extra));
    expect_local(
        PolyglotRuntimeHost::create(std::move(extra_binding)).error_code ==
            "polyglot.host.capability_route_required",
        "a capability binding without an exact route should fail construction");

    auto built = PolyglotRuntimeHost::create(
        configuration(admission, root, "on"));
    const auto unknown = built.host->dispatch_callback()({
        "interop.unknown-v1", "{}", 0U, []() { return false; }});
    expect_local(
        unknown.status == RuntimePolyglotDispatchStatus::invalid_request &&
        unknown.error_code == "polyglot.host.capability_unavailable" &&
        unknown.native_invocation_count == 0U &&
        unknown.candidate_invocation_count == 0U,
        "unknown capabilities should fail before native or candidate launch");
}

void test_fallback_error_and_correlation_identity(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    auto fail_fast = PolyglotRuntimeHost::create(
        configuration(admission, root, "on", "--host-error"));
    const auto failed = dispatch_once(fail_fast.host);
    expect_local(
        failed.status == RuntimePolyglotDispatchStatus::candidate_failed &&
        failed.error_code == "polyglot.adapter.nonzero_exit" &&
        failed.selection == RuntimePolyglotDispatchSelection::candidate &&
        failed.authority == RuntimePolyglotDispatchAuthority::candidate &&
        failed.native_invocation_count == 0U &&
        failed.candidate_invocation_count == 1U &&
        !failed.native_fallback_executed,
        "fail-fast should preserve the existing candidate error identity and counts");

    auto failed_native_config = configuration(admission, root, "off");
    failed_native_config.capabilities.front().invoke_native = []() {
        return PolyglotNativeInvocationResult{
            false,
            "native.synthetic.failure",
            R"json({"sensitive":"must-not-cross-runtime-boundary"})json"};
    };
    auto failed_native = PolyglotRuntimeHost::create(
        std::move(failed_native_config));
    const auto native_failure = dispatch_once(failed_native.host);
    expect_local(
        native_failure.status == RuntimePolyglotDispatchStatus::native_failed &&
        native_failure.error_code == "native.synthetic.failure" &&
        native_failure.selection == RuntimePolyglotDispatchSelection::native &&
        native_failure.authority == RuntimePolyglotDispatchAuthority::native &&
        native_failure.native_invocation_count == 1U &&
        native_failure.candidate_invocation_count == 0U &&
        !native_failure.native_fallback_executed &&
        native_failure.payload_json.empty(),
        "failed native work must preserve evidence without exposing payload bytes");

    auto fallback = PolyglotRuntimeHost::create(configuration(
        admission, root, "on", "--host-error", 0U,
        PolyglotFallbackPolicy::fallback_native));
    const auto recovered = dispatch_once(fallback.host);
    expect_local(
        recovered.status == RuntimePolyglotDispatchStatus::success &&
        recovered.error_code == "polyglot.execution.native_success" &&
        recovered.selection == RuntimePolyglotDispatchSelection::candidate &&
        recovered.authority == RuntimePolyglotDispatchAuthority::native &&
        recovered.native_invocation_count == 1U &&
        recovered.candidate_invocation_count == 1U &&
        recovered.native_fallback_executed &&
        recovered.payload_json.find("native") != std::string::npos,
        "permitted native fallback should preserve authority, counts, and payload");

    auto bounded_config = configuration(admission, root, "on");
    bounded_config.capabilities.front()
        .candidate_request_template.stdout_limit_bytes = 64U;
    auto bounded = PolyglotRuntimeHost::create(std::move(bounded_config));
    const auto bounded_result = dispatch_once(bounded.host);
    expect_local(
        bounded_result.status == RuntimePolyglotDispatchStatus::candidate_failed &&
        bounded_result.error_code == "polyglot.process.stdout_limit_exceeded" &&
        bounded_result.payload_json.empty() &&
        bounded_result.candidate_invocation_count == 1U,
        "candidate output beyond the explicit bound should fail without exposing bytes");

    auto parity_config = configuration(admission, root, "shadow");
    parity_config.capabilities.front().normalize_shadow_parity = [](
        const PolyglotNativeInvocationResult&,
        const PolyglotArtifactInvocationResult&) -> PolyglotShadowParityValues {
        throw std::runtime_error("synthetic normalization failure");
    };
    auto parity = PolyglotRuntimeHost::create(std::move(parity_config));
    const auto parity_result = dispatch_once(parity.host);
    expect_local(
        parity_result.status == RuntimePolyglotDispatchStatus::parity_failed &&
        parity_result.error_code ==
            "polyglot.execution.shadow_normalizer_exception" &&
        parity_result.selection == RuntimePolyglotDispatchSelection::shadow &&
        parity_result.authority == RuntimePolyglotDispatchAuthority::native &&
        parity_result.native_invocation_count == 1U &&
        parity_result.candidate_invocation_count == 1U,
        "shadow parity failure should preserve the executor's exact identity and counts");

    auto concurrent = PolyglotRuntimeHost::create(
        configuration(admission, root, "on"));
    const auto callback = concurrent.host->dispatch_callback();
    RuntimePolyglotDispatchResult first;
    RuntimePolyglotDispatchResult second;
    std::thread first_thread([&]() {
        first = callback({
            capability_id, R"json({"value":7})json", 0U,
            []() { return false; }});
    });
    std::thread second_thread([&]() {
        second = callback({
            capability_id, R"json({"value":7})json", 0U,
            []() { return false; }});
    });
    first_thread.join();
    second_thread.join();
    const bool first_sequence =
        first.payload_json.find("runtime-host-correlation-1") != std::string::npos;
    const bool second_sequence =
        second.payload_json.find("runtime-host-correlation-2") != std::string::npos;
    const bool reversed_sequence =
        first.payload_json.find("runtime-host-correlation-2") != std::string::npos &&
        second.payload_json.find("runtime-host-correlation-1") != std::string::npos;
    expect_local(
        first.status == RuntimePolyglotDispatchStatus::success &&
        second.status == RuntimePolyglotDispatchStatus::success &&
        ((first_sequence && second_sequence) || reversed_sequence),
        "concurrent dispatches should serialize admission use and receive unique correlation identities");
}

void test_cancellation_probe_reaches_bounded_candidate(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    auto built = PolyglotRuntimeHost::create(
        configuration(admission, root, "on", "--host-sleep"));
    std::atomic<std::uint32_t> polls{0U};
    const auto result = dispatch_once(built.host, 0U, [&]() {
        return polls.fetch_add(1U, std::memory_order_relaxed) >= 2U;
    });
    expect_local(
        result.status == RuntimePolyglotDispatchStatus::cancelled &&
        result.selection == RuntimePolyglotDispatchSelection::candidate &&
        result.authority == RuntimePolyglotDispatchAuthority::candidate &&
        result.candidate_invocation_count == 1U && polls.load() >= 3U,
        "the read-only cancellation probe should cooperatively stop one bounded candidate");

    auto ignored = PolyglotRuntimeHost::create(configuration(
        admission, root, "on", "--host-sleep", 0U,
        PolyglotFallbackPolicy::fallback_native,
        PolyglotCancellationPolicy::ignore));
    std::atomic<std::uint32_t> ignored_polls{0U};
    const auto ignored_result = dispatch_once(ignored.host, 0U, [&]() {
        return ignored_polls.fetch_add(1U, std::memory_order_relaxed) >= 2U;
    });
    expect_local(
        ignored_result.status == RuntimePolyglotDispatchStatus::success &&
        ignored_result.authority == RuntimePolyglotDispatchAuthority::native &&
        ignored_result.native_fallback_executed &&
        ignored_result.native_invocation_count == 1U &&
        ignored_result.candidate_invocation_count == 1U,
        "ignored cancellation should remain governed by bridge fallback policy");
}

void test_prg_session_uses_production_host_callback(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    const fs::path program = root / "runtime-host.prg";
    write_text(
        program,
        "cDispatch = CFPOLYGLOTDISPATCH('interop.runtime-host-v1', "
        "'{\"value\":7}', 0)\n"
        "cStatus = CFJSONGET(cDispatch, '/status')\n"
        "cAuthority = CFJSONGET(cDispatch, '/authority')\n"
        "cSource = CFJSONGET(cDispatch, '/payload/source')\n"
        "nCalls = CFJSONGET(cDispatch, '/candidate_invocation_count')\n"
        "RETURN\n");
    auto built = PolyglotRuntimeHost::create(
        configuration(admission, root, "on"));
    auto options = make_runtime_session_options(program, root);
    options.polyglot_dispatch_callback = built.host->dispatch_callback();
    built.host.reset();
    auto session = PrgRuntimeSession::create(options);
    const auto state = session.run(DebugResumeAction::continue_run);
    expect_local(state.completed,
                 "PRG session should complete through the production host callback");
    const auto value = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        return found == state.globals.end()
            ? std::string{}
            : format_value(found->second);
    };
    expect_local(
        value("cstatus") == "success" &&
        value("cauthority") == "candidate" &&
        value("csource") == "candidate" && value("ncalls") == "1",
        "PRG should receive only bounded invariant route evidence and candidate payload");
    for (const auto& event : state.events) {
        if (event.category == "runtime.polyglot.dispatch") {
            expect_local(
                event.detail.find("{\"value\":7}") == std::string::npos &&
                event.detail.find("runtime-host-correlation") == std::string::npos,
                "composed dispatch telemetry should redact request and correlation bytes");
        }
    }
}

void test_cftaskcancel_reaches_production_candidate(
    const PolyglotArtifactAdmissionResult& admission,
    const fs::path& root) {
    const fs::path ready_path = root / "production-candidate.ready";
    const fs::path program = root / "runtime-host-cancel.prg";
    write_text(
        program,
        "PROCEDURE bridgeworker\n"
        "    RETURN CFPOLYGLOTDISPATCH('interop.runtime-host-v1', "
        "'{\"value\":7}', 0)\n"
        "ENDPROC\n"
        "SPAWN bridgeworker TO nTask\n"
        "DO WHILE NOT FILE('" + ready_path.string() + "')\n"
        "    YIELD\n"
        "ENDDO\n"
        "lCancelRequested = CFTASKCANCEL(nTask)\n"
        "DO WHILE CFTASKSTATUS(nTask) == 'running' OR "
        "CFTASKSTATUS(nTask) == 'cancel-requested'\n"
        "    YIELD\n"
        "ENDDO\n"
        "cTaskStatus = CFTASKSTATUS(nTask)\n"
        "cResult = CFTASKRESULT(nTask)\n"
        "cDispatchStatus = CFJSONGET(cResult, '/status')\n"
        "cReason = CFJSONGET(cResult, '/error_code')\n"
        "AWAIT nTask TO lJoined\n"
        "RETURN\n");
    auto built = PolyglotRuntimeHost::create(configuration(
        admission, root, "on", "--host-sleep", 0U,
        PolyglotFallbackPolicy::fail_fast,
        PolyglotCancellationPolicy::propagate,
        utf8_path(ready_path)));
    auto options = make_runtime_session_options(program, root);
    options.polyglot_dispatch_callback = built.host->dispatch_callback();
    auto session = PrgRuntimeSession::create(options);
    const auto state = session.run(DebugResumeAction::continue_run);
    const auto value = [&](const std::string& name) {
        const auto found = state.globals.find(name);
        return found == state.globals.end()
            ? std::string{}
            : format_value(found->second);
    };
    expect_local(
        state.completed && value("lcancelrequested") == "true" &&
        value("ctaskstatus") == "completed" &&
        value("cdispatchstatus") == "cancelled" &&
        value("creason") == "polyglot.process.cancelled" &&
        value("ljoined") == "true",
        "CFTASKCANCEL should reach bounded production candidate execution through the read-only probe");
}

int run_tests(const fs::path& running_executable) {
    const fs::path root = unique_root();
    std::error_code error;
    fs::create_directories(root, error);
    expect_local(!error, "runtime-host fixture root should be created");
    const fs::path artifact = root /
        (std::string("runtime-host-artifact") + running_executable.extension().string());
    expect_local(copy_executable(running_executable, artifact),
                 "runtime-host fixture executable should be copied");
    if (!error && fs::exists(artifact)) {
        const auto admission = admit(artifact, root);
        expect_local(admission.ok(), "runtime-host fixture should be admitted once");
        test_route_states_and_owned_callback_lifetime(admission, root);
        test_configuration_fails_before_launch(admission, root);
        test_fallback_error_and_correlation_identity(admission, root);
        test_cancellation_probe_reaches_bounded_candidate(admission, root);
        test_prg_session_uses_production_host_callback(admission, root);
        test_cftaskcancel_reaches_production_candidate(admission, root);
    }
    fs::remove_all(root, error);
    return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc == 2 || argc == 3) {
        return run_artifact(argv[1], argc == 3 ? argv[2] : std::string{});
    }
    if (argc != 1) {
        return 40;
    }
    return run_tests(fs::absolute(argv[0]));
}
