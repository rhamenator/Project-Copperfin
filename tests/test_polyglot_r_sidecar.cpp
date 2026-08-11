// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_artifact_adapter.h"
#include "copperfin/runtime/polyglot_runtime_host.h"
#include "copperfin/security/sha256.h"
#include "prg_engine_test_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

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

constexpr const char* capability_id = "samples.r.mean-v1";
int failures = 0;

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

std::string utf8_path(const fs::path& path) {
    return path_to_utf8_string(path);
}

fs::path unique_root() {
    return fs::temp_directory_path() /
        ("copperfin_polyglot_r_sidecar_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

PolyglotArtifactAdmissionResult admit_rscript(const fs::path& executable) {
    const auto digest =
        copperfin::security::sha256_hex_for_file(utf8_path(executable));
    expect_local(digest.ok, "the Rscript executable should be hashable");
    return admit_polyglot_artifact({
        .capability_id = capability_id,
        .process_policy = {
            .executable_name = utf8_path(executable),
            .allowed_path_roots = {utf8_path(executable.parent_path())},
            .allowed_publishers = {},
            .require_trusted_signature = false},
        .expected_sha256 = digest.hex_digest});
}

PolyglotSupportingArtifactAdmissionResult admit_script(
    const fs::path& script,
    const fs::path& root) {
    const auto digest =
        copperfin::security::sha256_hex_for_file(utf8_path(script));
    expect_local(digest.ok, "the R sidecar script should be hashable");
    return admit_polyglot_supporting_artifact({
        .capability_id = capability_id,
        .artifact_path = utf8_path(script),
        .allowed_root = utf8_path(root),
        .expected_sha256 = digest.hex_digest,
        .maximum_bytes = 1024U * 1024U});
}

PolyglotArtifactInvocationRequest invocation(
    const fs::path& root,
    const fs::path& script,
    std::string arguments_json = R"({"values":[1,2,3,4]})") {
    PolyglotArtifactInvocationRequest request;
    request.invocation = {
        .capability_id = capability_id,
        .correlation_id = "r-sidecar-correlation",
        .protocol_version = "1.0.0",
        .arguments_json = std::move(arguments_json)};
    request.policy = {
        .timeout_ms = 10000U,
        .latency_budget_ms = 9000U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = PolyglotFallbackPolicy::fail_fast,
        .max_attempts = 1U};
    request.artifact_arguments = {"--vanilla", utf8_path(script)};
    request.working_directory = utf8_path(root);
    request.environment = {{"R_DEFAULT_PACKAGES", "base"}};
    request.poll_interval_ms = 2U;
    request.stdin_limit_bytes = 64U * 1024U;
    request.stdout_limit_bytes = 64U * 1024U;
    request.stderr_limit_bytes = 4U * 1024U;
    return request;
}

std::vector<PolyglotSupportingArtifactArgumentBinding> script_binding() {
    return {{1U, 0U}};
}

std::string describe(const PolyglotArtifactInvocationResult& result) {
    return " status=" + std::string(
        polyglot_artifact_invocation_status_name(result.status)) +
        " error=" + result.error_code +
        " exit=" + std::to_string(result.process.exit_code) +
        " stdout=" + result.process.standard_output +
        " stderr=" + result.process.standard_error;
}

void test_closed_json_reader(
    const fs::path& rscript,
    const fs::path& root,
    const fs::path& script) {
    const std::vector<std::pair<std::string, std::string>> rejected_requests{
        {"duplicate object members",
         R"({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.r.mean-v1","correlation_id":"r-direct","protocol_version":"1.0.0","arguments":{"values":[1],"values":[2]}})"},
        {"an unknown top-level member",
         R"({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.r.mean-v1","correlation_id":"r-direct","protocol_version":"1.0.0","arguments":{"values":[1]},"unknown":true})"},
        {"an unpaired Unicode surrogate",
         R"({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.r.mean-v1","correlation_id":"\uD800","protocol_version":"1.0.0","arguments":{"values":[1]}})"},
        {"excessive nesting",
         R"({"envelope_version":"1.0","kind":"invocation","capability_id":"samples.r.mean-v1","correlation_id":"r-direct","protocol_version":"1.0.0","arguments":{"values":[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]}})"}};
    for (const auto& [description, document] : rejected_requests) {
        const auto result = run_bounded_process({
            .executable_path = utf8_path(rscript),
            .arguments = {"--vanilla", utf8_path(script)},
            .working_directory = utf8_path(root),
            .environment = {{"R_DEFAULT_PACKAGES", "base"}},
            .standard_input = document,
            .timeout_ms = 10000U,
            .poll_interval_ms = 2U,
            .stdin_limit_bytes = 64U * 1024U,
            .stdout_limit_bytes = 64U * 1024U,
            .stderr_limit_bytes = 4U * 1024U,
            .cancellation_requested = {}});
        expect_local(
            result.completed() && result.started && result.process_tree_closed &&
                result.exit_code == 2 && result.standard_output.empty() &&
                result.standard_error.empty(),
            "the base-R JSON reader should reject " + description +
                " status=" + bounded_process_status_name(result.status) +
                " exit=" + std::to_string(result.exit_code) +
                " stdout=" + result.standard_output +
                " stderr=" + result.standard_error);
    }
}

void test_round_trip_and_fail_closed(
    PolyglotArtifactAdmissionResult& rscript,
    const fs::path& root,
    const fs::path& script) {
    {
        auto admitted = admit_script(script, root);
        const fs::path admitted_script =
            path_from_utf8_string(admitted.resolved_path());
        std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
            std::move(admitted)};
        const auto result = invoke_polyglot_artifact(
            rscript,
            supporting,
            script_binding(),
            invocation(root, admitted_script));
        expect_local(
            result.ok() && result.process.started &&
                result.process.process_tree_closed &&
                result.response.envelope.payload_json == R"({"mean":2.5})" &&
                result.process.standard_error.empty(),
            "the admitted R sidecar should complete one clean bounded round trip" +
                describe(result));
    }
    {
        auto admitted = admit_script(script, root);
        const fs::path admitted_script =
            path_from_utf8_string(admitted.resolved_path());
        std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
            std::move(admitted)};
        const auto rejected = invoke_polyglot_artifact(
            rscript,
            supporting,
            script_binding(),
            invocation(root, admitted_script, R"({"values":[]})"));
        expect_local(
            rejected.status == PolyglotArtifactInvocationStatus::candidate_error &&
                rejected.response.ok() &&
                rejected.response.envelope.candidate_error_code ==
                    "sample.r.invalid_arguments" &&
                rejected.process.standard_error.empty(),
            "invalid R sample arguments should return one typed error envelope" +
                describe(rejected));
    }
    {
        auto admitted = admit_script(script, root);
        const fs::path admitted_script =
            path_from_utf8_string(admitted.resolved_path());
        std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
            std::move(admitted)};
        auto substituted = invocation(root, admitted_script);
        substituted.artifact_arguments[1] = utf8_path(root / "other.R");
        const auto result = invoke_polyglot_artifact(
            rscript, supporting, script_binding(), substituted);
        expect_local(
            !result.process.started &&
                result.status == PolyglotArtifactInvocationStatus::artifact_rejected &&
                result.error_code ==
                    "polyglot.adapter.supporting_artifact_argument_mismatch",
            "a substituted script argument should reject before R starts");
    }

    auto admitted_before_change = admit_script(script, root);
    const fs::path admitted_script =
        path_from_utf8_string(admitted_before_change.resolved_path());
    {
        std::ofstream output(admitted_script, std::ios::binary | std::ios::app);
        output << "\n# changed after admission\n";
    }
    std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
        std::move(admitted_before_change)};
    const auto changed = invoke_polyglot_artifact(
        rscript,
        supporting,
        script_binding(),
        invocation(root, admitted_script));
    expect_local(
        !changed.process.started &&
            changed.error_code ==
                "polyglot.supporting_artifact.changed_before_execution" &&
            !supporting.front().ok(),
        "a changed script should revoke admission before R starts");
}

PolyglotRuntimeHostConfiguration configuration(
    const PolyglotArtifactAdmissionResult& rscript,
    PolyglotSupportingArtifactAdmissionResult script_admission,
    const fs::path& root,
    const fs::path& script) {
    auto routes = load_polyglot_route_registry({{capability_id, "on", 0U}});
    expect_local(routes.ok(), "the R sample route should load");
    auto candidate = invocation(root, script, {});
    candidate.invocation.correlation_id = "r-host-correlation";
    PolyglotRuntimeCapabilityBinding binding{
        .capability_id = capability_id,
        .artifact_admission = rscript,
        .supporting_artifact_admissions = {std::move(script_admission)},
        .supporting_artifact_arguments = script_binding(),
        .candidate_request_template = std::move(candidate),
        .invoke_native = {},
        .normalize_shadow_parity = {},
        .parity_policy = {}};
    return {std::move(routes.registry), {std::move(binding)}};
}

void test_prg_control(
    const PolyglotArtifactAdmissionResult& rscript,
    const fs::path& root,
    const fs::path& script) {
    auto script_admission = admit_script(script, root);
    const fs::path admitted_script =
        path_from_utf8_string(script_admission.resolved_path());
    auto invalid_configuration = configuration(
        rscript, script_admission, root, admitted_script);
    invalid_configuration.capabilities.front()
        .supporting_artifact_arguments.front().argument_index = 0U;
    const auto invalid_host = PolyglotRuntimeHost::create(
        std::move(invalid_configuration));
    expect_local(
        !invalid_host.ok() &&
            invalid_host.error_code ==
                "polyglot.host.supporting_artifact_binding_required",
        "host creation should reject an R script bound to the wrong argument");

    auto built = PolyglotRuntimeHost::create(
        configuration(
            rscript,
            std::move(script_admission),
            root,
            admitted_script));
    expect_local(built.ok(), "the R sidecar should bind to the trusted runtime host");
    if (!built.ok()) {
        return;
    }
    const auto callback = built.host->dispatch_callback();
    const fs::path program = root / "r-sidecar.prg";
    write_text(
        program,
        "cDispatch = CFPOLYGLOTDISPATCH('samples.r.mean-v1', "
        "'{\"values\":[1,2,3,4]}', 0)\n"
        "cStatus = CFJSONGET(cDispatch, '/status')\n"
        "cAuthority = CFJSONGET(cDispatch, '/authority')\n"
        "nMean = CFJSONGET(cDispatch, '/payload/mean')\n"
        "nCandidateCalls = CFJSONGET(cDispatch, '/candidate_invocation_count')\n"
        "RETURN\n");
    auto options = make_runtime_session_options(program, root);
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
            value("cauthority") == "candidate" && value("nmean") == "2.5" &&
            value("ncandidatecalls") == "1",
        "ordinary PRG should control and inspect the external R result");
}
}  // namespace

int main() {
    std::error_code error;
    const fs::path root = unique_root();
    fs::create_directories(root, error);
    expect_local(!error, "the R integration fixture root should be created");
    const fs::path rscript = fs::canonical(
        path_from_utf8_string(COPPERFIN_POLYGLOT_RSCRIPT_EXECUTABLE), error);
    expect_local(!error && fs::is_regular_file(rscript),
           "the Rscript executable should resolve to a regular file");
    const fs::path script = root / "candidate.R";
    fs::copy_file(
        path_from_utf8_string(COPPERFIN_POLYGLOT_R_SIDECAR_PATH),
        script,
        fs::copy_options::overwrite_existing,
        error);
    expect_local(!error, "the checked-in R sidecar should copy into its admitted root");

    const auto script_digest =
        copperfin::security::sha256_hex_for_file(utf8_path(script));
    const auto size_rejected = admit_polyglot_supporting_artifact({
        .capability_id = capability_id,
        .artifact_path = utf8_path(script),
        .allowed_root = utf8_path(root),
        .expected_sha256 = script_digest.hex_digest,
        .maximum_bytes = 1U});
    expect_local(
        !size_rejected.ok() && size_rejected.error_code() ==
            "polyglot.supporting_artifact.size_limit_exceeded",
        "supporting admission should reject an oversized R script before reading it");

    auto rscript_admission = admit_rscript(rscript);
    expect_local(rscript_admission.ok(), "the Rscript executable should be admitted");
    if (rscript_admission.ok() && !error) {
        test_closed_json_reader(rscript, root, script);
        test_prg_control(rscript_admission, root, script);
        test_round_trip_and_fail_closed(rscript_admission, root, script);
    }
    fs::remove_all(root, error);
    if (failures == 0) {
        std::cout << "polyglot R sidecar tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
