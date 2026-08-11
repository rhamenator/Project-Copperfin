// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/runtime/polyglot_runtime_host.h"
#include "copperfin/security/sha256.h"
#include "prg_engine_test_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
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
        const auto admission = admit_candidate(published);
        expect_local(admission.ok(), "the exact Native AOT candidate should be admitted");
        if (admission.ok()) {
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
