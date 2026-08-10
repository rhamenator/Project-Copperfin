// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_artifact_adapter.h"
#include "copperfin/security/sha256.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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

constexpr const char* capability_id = "interop.sample-v1";
constexpr const char* correlation_id = "corr-001";
constexpr const char* protocol_version = "1.0.0";
constexpr const char* arguments_json =
    R"json({"value":"line\n\u96ea","n":9007199254740993})json";
constexpr const char* expected_request =
    R"json({"envelope_version":"1.0","kind":"invocation","capability_id":"interop.sample-v1","correlation_id":"corr-001","protocol_version":"1.0.0","arguments":{"value":"line\n\u96ea","n":9007199254740993}})json";
constexpr const char* success_response =
    R"json({"envelope_version":"1.0","kind":"success","capability_id":"interop.sample-v1","correlation_id":"corr-001","protocol_version":"1.0.0","payload":{"value":"ok","exact":9007199254740993}})json";
constexpr const char* error_response =
    R"json({"envelope_version":"1.0","kind":"error","capability_id":"interop.sample-v1","correlation_id":"corr-001","protocol_version":"1.0.0","error":{"code":"candidate.synthetic","message":"expected","retryable":false}})json";

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

std::string read_standard_input() {
    return {std::istreambuf_iterator<char>(std::cin),
            std::istreambuf_iterator<char>()};
}

int run_artifact_mode(const std::string& mode) {
    configure_binary_standard_streams();
    const std::string request = read_standard_input();
    if (request != expected_request) {
        std::cerr << "request-mismatch";
        return 23;
    }
    if (mode == "--adapter-success") {
        std::cout << success_response;
        std::cerr << "candidate-diagnostic";
        return 0;
    }
    if (mode == "--adapter-error") {
        std::cout << error_response;
        return 0;
    }
    if (mode == "--adapter-malformed") {
        std::cout << R"json({"kind":"success")json";
        return 0;
    }
    if (mode == "--adapter-nonzero") {
        std::cout << success_response;
        return 17;
    }
    if (mode == "--adapter-sleep") {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << success_response;
        return 0;
    }
    if (mode == "--adapter-delay") {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        std::cout << success_response;
        return 0;
    }
    return 24;
}

unsigned long current_process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long>(::GetCurrentProcessId());
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

std::filesystem::path unique_temp_root() {
    return std::filesystem::temp_directory_path() /
        ("copperfin_polyglot_artifact_adapter_" +
         std::to_string(current_process_id()) + "_" +
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
        source,
        destination,
        std::filesystem::copy_options::overwrite_existing,
        error);
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

copperfin::platform::PolyglotArtifactAdmissionResult admit(
    const std::filesystem::path& artifact,
    const std::filesystem::path& root) {
    const auto digest = copperfin::security::sha256_hex_for_file(artifact);
    expect(digest.ok, "adapter fixture should have a readable SHA-256 identity");
    return copperfin::platform::admit_polyglot_artifact({
        .capability_id = capability_id,
        .process_policy = {
            .executable_name = utf8_path(artifact),
            .allowed_path_roots = {utf8_path(root)},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

copperfin::platform::PolyglotArtifactInvocationRequest invocation(
    const std::filesystem::path& root,
    const std::string& mode) {
    return {
        .invocation = {
            .capability_id = capability_id,
            .correlation_id = correlation_id,
            .protocol_version = protocol_version,
            .arguments_json = arguments_json},
        .policy = {
            .timeout_ms = 5000U,
            .latency_budget_ms = 4500U,
            .cancellation = copperfin::platform::PolyglotCancellationPolicy::propagate,
            .fallback = copperfin::platform::PolyglotFallbackPolicy::fail_fast,
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

void expect_one_bridge_event(
    const copperfin::platform::PolyglotArtifactInvocationResult& result,
    const std::string& reason,
    const std::string& message) {
    expect(
        result.telemetry.events.size() == 1U &&
            result.telemetry.events.front().category ==
                "polyglot.latency.outcome" &&
            result.telemetry.events.front().capability_id == capability_id &&
            result.telemetry.events.front().reason_code == reason,
        message);
}

void test_success(
    copperfin::platform::PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    using copperfin::platform::PolyglotArtifactInvocationStatus;
    const auto result = copperfin::platform::invoke_polyglot_artifact(
        admission,
        invocation(root, "--adapter-success"));
    expect(result.ok() && result.status == PolyglotArtifactInvocationStatus::success,
           "#4700: admitted artifact should complete the exact v1 envelope round trip");
    expect(result.artifact_revalidated && result.process.started &&
               result.process.process_tree_closed && result.process.exit_code == 0,
           "#4700: adapter success should revalidate and close the owned process tree");
    expect(result.request_document == expected_request,
           "#4700: adapter should deliver the deterministic serialized request bytes");
    expect(result.response.envelope.payload_json ==
               R"json({"value":"ok","exact":9007199254740993})json",
           "#4700: adapter should retain exact admitted payload bytes");
    expect(result.process.standard_error == "candidate-diagnostic",
           "#4700: stderr should remain separately captured and unparsed");
    expect_one_bridge_event(
        result,
        "polyglot.bridge.success",
        "#4700: success should emit one invariant bridge outcome event");
    expect(
        std::string(copperfin::platform::polyglot_artifact_invocation_status_name(
            result.status)) == "success",
        "#4700: adapter status names should remain invariant");
}

void test_candidate_error_and_fallback(
    copperfin::platform::PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    auto request = invocation(root, "--adapter-error");
    request.policy.fallback =
        copperfin::platform::PolyglotFallbackPolicy::fallback_native;
    const auto result = copperfin::platform::invoke_polyglot_artifact(
        admission, request);
    expect(
        result.status ==
                copperfin::platform::PolyglotArtifactInvocationStatus::candidate_error &&
            result.response.ok() &&
            result.response.envelope.candidate_error_code == "candidate.synthetic",
        "#4700: a valid candidate-error envelope should remain distinct from protocol failure");
    expect(result.decision.outcome ==
               copperfin::platform::PolyglotBridgeOutcome::fallback_native &&
               result.decision.use_native_fallback,
           "#4700: candidate failure should report the configured native fallback decision");
    expect(result.telemetry.events.size() == 2U &&
               result.telemetry.events[1].category ==
                   "polyglot.fallback.applied",
           "#4700: selected fallback should emit latency and fallback telemetry");
}

void test_fail_closed_outcomes(
    copperfin::platform::PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    using copperfin::platform::PolyglotArtifactInvocationStatus;
    const auto malformed = copperfin::platform::invoke_polyglot_artifact(
        admission,
        invocation(root, "--adapter-malformed"));
    expect(malformed.status == PolyglotArtifactInvocationStatus::response_rejected &&
               malformed.decision.failure ==
                   copperfin::platform::PolyglotBridgeFailure::protocol_error &&
               !malformed.response.ok(),
           "#4700: malformed candidate stdout should fail as a protocol error");

    const auto nonzero = copperfin::platform::invoke_polyglot_artifact(
        admission,
        invocation(root, "--adapter-nonzero"));
    expect(nonzero.status == PolyglotArtifactInvocationStatus::process_failed &&
               nonzero.process.exit_code == 17 &&
               nonzero.error_code == "polyglot.adapter.nonzero_exit" &&
               nonzero.decision.failure ==
                   copperfin::platform::PolyglotBridgeFailure::candidate_error,
           "#4700: nonzero exit should not admit otherwise valid stdout");

    auto overflow_request = invocation(root, "--adapter-success");
    overflow_request.stdout_limit_bytes = 64U;
    const auto overflow = copperfin::platform::invoke_polyglot_artifact(
        admission, overflow_request);
    expect(overflow.status == PolyglotArtifactInvocationStatus::process_failed &&
               overflow.process.status ==
                   copperfin::platform::BoundedProcessStatus::output_limit_exceeded &&
               overflow.decision.failure ==
                   copperfin::platform::PolyglotBridgeFailure::protocol_error,
           "#4700: stdout overflow should close the tree and fail before parsing");
}

void test_timeout_and_cancellation(
    copperfin::platform::PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root) {
    auto timeout_request = invocation(root, "--adapter-sleep");
    timeout_request.policy.timeout_ms = 100U;
    timeout_request.policy.latency_budget_ms = 90U;
    const auto timed_out = copperfin::platform::invoke_polyglot_artifact(
        admission, timeout_request);
    expect(timed_out.process.status ==
               copperfin::platform::BoundedProcessStatus::timed_out &&
               timed_out.process.process_tree_closed &&
               timed_out.decision.failure ==
                   copperfin::platform::PolyglotBridgeFailure::timeout,
           "#4700: adapter timeout should close the tree and map deterministically");

    std::atomic_uint polls{0U};
    auto cancellation_request = invocation(root, "--adapter-sleep");
    cancellation_request.cancellation_requested = [&polls]() {
        return polls.fetch_add(1U) >= 2U;
    };
    const auto cancelled = copperfin::platform::invoke_polyglot_artifact(
        admission, cancellation_request);
    expect(cancelled.status ==
               copperfin::platform::PolyglotArtifactInvocationStatus::cancelled &&
               cancelled.process.process_tree_closed &&
               cancelled.decision.outcome ==
                   copperfin::platform::PolyglotBridgeOutcome::cancelled &&
               cancelled.decision.cancellation_propagated,
           "#4700: propagated cancellation should not select a fallback");

    auto latency_request = invocation(root, "--adapter-delay");
    latency_request.policy.timeout_ms = 1000U;
    latency_request.policy.latency_budget_ms = 20U;
    latency_request.policy.fallback =
        copperfin::platform::PolyglotFallbackPolicy::fallback_native;
    const auto late = copperfin::platform::invoke_polyglot_artifact(
        admission, latency_request);
    expect(
        late.response.ok() &&
            late.status == copperfin::platform::
                PolyglotArtifactInvocationStatus::latency_budget_exceeded &&
            late.decision.error_code ==
                "polyglot.bridge.latency_budget_exceeded" &&
            late.decision.outcome ==
                copperfin::platform::PolyglotBridgeOutcome::fallback_native,
        "#4700: a valid late response should retain its envelope and report the configured fallback");
}

void test_prelaunch_rejection(
    copperfin::platform::PolyglotArtifactAdmissionResult& admission,
    const std::filesystem::path& root,
    const std::filesystem::path& running_executable) {
    auto mismatch_request = invocation(root, "--adapter-success");
    mismatch_request.invocation.capability_id = "interop.other-v1";
    const auto mismatch = copperfin::platform::invoke_polyglot_artifact(
        admission, mismatch_request);
    expect(!mismatch.artifact_revalidated && !mismatch.process.started &&
               mismatch.error_code == "polyglot.adapter.capability_id_mismatch" &&
               mismatch.telemetry.events.empty(),
           "#4700: capability confusion should reject without launch or untrusted telemetry");

    auto retry_request = invocation(root, "--adapter-success");
    retry_request.policy.max_attempts = 2U;
    const auto retry = copperfin::platform::invoke_polyglot_artifact(
        admission, retry_request);
    expect(!retry.artifact_revalidated && !retry.process.started &&
               retry.error_code ==
                   "polyglot.adapter.multiple_attempts_unsupported",
           "#4700: the single-attempt adapter must not silently implement retries");

    auto invalid_request = invocation(root, "--adapter-success");
    invalid_request.invocation.arguments_json = "[]";
    const auto invalid = copperfin::platform::invoke_polyglot_artifact(
        admission, invalid_request);
    expect(!invalid.artifact_revalidated && !invalid.process.started &&
               invalid.error_code == "polyglot.request.arguments_object_required" &&
               invalid.telemetry.events.empty(),
           "#4700: invalid arguments should reject before revalidation or telemetry");

    const auto changed_path = root /
        (std::string("changed-") + running_executable.filename().string());
    expect(copy_executable(running_executable, changed_path),
           "changed-artifact fixture should copy the test executable");
    auto changed_admission = admit(changed_path, root);
    expect(changed_admission.ok(), "changed-artifact fixture should initially admit");
    {
        std::ofstream output(changed_path, std::ios::binary | std::ios::app);
        output.put('\0');
    }
    const auto changed = copperfin::platform::invoke_polyglot_artifact(
        changed_admission,
        invocation(root, "--adapter-success"));
    expect(!changed.artifact_revalidated && !changed.process.started &&
               changed.status ==
                   copperfin::platform::PolyglotArtifactInvocationStatus::artifact_rejected &&
               !changed_admission.ok(),
           "#4700: changed bytes should revoke the token immediately before launch");
}

int run_tests(const std::filesystem::path& running_executable) {
    const auto root = unique_temp_root();
    std::error_code error;
    std::filesystem::create_directories(root, error);
    expect(!error, "adapter test should create its process-unique root");
    if (error) {
        return 1;
    }

    const auto artifact = root / running_executable.filename();
    expect(copy_executable(running_executable, artifact),
           "adapter fixture should copy the test executable");
    auto admission = admit(artifact, root);
    expect(admission.ok(), "adapter fixture executable should be admitted");
    if (admission.ok()) {
        test_success(admission, root);
        test_candidate_error_and_fallback(admission, root);
        test_fail_closed_outcomes(admission, root);
        test_timeout_and_cancellation(admission, root);
        test_prelaunch_rejection(admission, root, running_executable);
    }

    std::filesystem::remove_all(root, error);
    expect(!error, "adapter test should remove its process-unique root");
    return failures == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 2 && std::string(argv[1]).rfind("--adapter-", 0U) == 0U) {
            return run_artifact_mode(argv[1]);
        }
        const auto executable = copperfin::platform::resolve_running_executable_path(
            argc > 0 ? std::filesystem::path(argv[0]) : std::filesystem::path{});
        if (executable.empty()) {
            std::cerr << "FAIL: could not resolve adapter test executable\n";
            return 1;
        }
        return run_tests(executable);
    } catch (const std::exception& exception) {
        std::cerr << "FAIL: unexpected exception: " << exception.what() << '\n';
        return 1;
    }
}
