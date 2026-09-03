// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_artifact_adapter.h"
#include "copperfin/platform/polyglot_supporting_artifact_admission_test_hooks.h"
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
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {
using namespace copperfin::platform;
using namespace copperfin::runtime;
using namespace copperfin::test_support;
namespace fs = std::filesystem;

constexpr const char* capability_id = "samples.python.add-v1";
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

#if defined(_WIN32)
std::string required_windows_system_root() {
    std::vector<wchar_t> buffer(32768U, L'\0');
    const UINT length = GetWindowsDirectoryW(
        buffer.data(), static_cast<UINT>(buffer.size()));
    if (length == 0U || length >= buffer.size()) {
        return {};
    }
    return utf8_path(fs::path(std::wstring(buffer.data(), length)));
}
#endif

fs::path unique_root() {
    return fs::temp_directory_path() /
        ("copperfin_polyglot_python_sidecar_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

PolyglotArtifactAdmissionResult admit_python(const fs::path& executable) {
    const auto digest =
        copperfin::security::sha256_hex_for_file(utf8_path(executable));
    expect_local(digest.ok, "the Python interpreter should be hashable");
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
    expect_local(digest.ok, "the Python sidecar script should be hashable");
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
    std::string arguments_json = R"({"left":20,"right":22})") {
    PolyglotArtifactInvocationRequest request;
    request.invocation = {
        .capability_id = capability_id,
        .correlation_id = "python-sidecar-correlation",
        .protocol_version = "1.0.0",
        .arguments_json = std::move(arguments_json)};
    request.policy = {
        .timeout_ms = 5000U,
        .latency_budget_ms = 4500U,
        .cancellation = PolyglotCancellationPolicy::propagate,
        .fallback = PolyglotFallbackPolicy::fail_fast,
        .max_attempts = 1U};
    request.artifact_arguments = {"-I", "-S", utf8_path(script)};
    request.working_directory = utf8_path(root);
#if defined(_WIN32)
    // The adapter intentionally does not inherit ambient variables. Python
    // launched from the hosted tool cache still needs this explicit Windows
    // loader/standard-library root, so use the OS API rather than an ambient
    // PATH, PYTHONHOME, or runner-provided environment value.
    request.environment = {{"SystemRoot", required_windows_system_root()}};
#else
    request.environment = {};
#endif
    request.poll_interval_ms = 2U;
    request.stdin_limit_bytes = 64U * 1024U;
    request.stdout_limit_bytes = 64U * 1024U;
    request.stderr_limit_bytes = 4U * 1024U;
    return request;
}

std::vector<PolyglotSupportingArtifactArgumentBinding> script_binding() {
    return {{2U, 0U}};
}

void test_round_trip_and_fail_closed(
    PolyglotArtifactAdmissionResult& python,
    const fs::path& root,
    const fs::path& script) {
    {
        auto admitted = admit_script(script, root);
        const fs::path admitted_script =
            path_from_utf8_string(admitted.resolved_path());
        std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
            std::move(admitted)};
        const auto result = invoke_polyglot_artifact(
            python,
            supporting,
            script_binding(),
            invocation(root, admitted_script));
        expect_local(
            result.ok() && result.process.started &&
                result.process.process_tree_closed &&
                result.response.envelope.payload_json == R"({"sum":42})" &&
                result.process.standard_error.empty(),
            "the admitted Python sidecar should complete one clean bounded round trip");
    }
    {
        auto admitted = admit_script(script, root);
        const fs::path admitted_script =
            path_from_utf8_string(admitted.resolved_path());
        std::vector<PolyglotSupportingArtifactAdmissionResult> supporting{
            std::move(admitted)};
        auto substituted = invocation(root, admitted_script);
        substituted.artifact_arguments[2] = utf8_path(root / "other.py");
        const auto result = invoke_polyglot_artifact(
            python, supporting, script_binding(), substituted);
        expect_local(
            !result.process.started &&
                result.status == PolyglotArtifactInvocationStatus::artifact_rejected &&
                result.error_code ==
                    "polyglot.adapter.supporting_artifact_argument_mismatch",
            "a substituted script argument should reject before Python starts");
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
        python,
        supporting,
        script_binding(),
        invocation(root, admitted_script));
    expect_local(
        !changed.process.started &&
            changed.error_code ==
                "polyglot.supporting_artifact.changed_before_execution" &&
            !supporting.front().ok(),
        "a changed script should revoke admission before Python starts");
}

PolyglotRuntimeHostConfiguration configuration(
    const PolyglotArtifactAdmissionResult& python,
    PolyglotSupportingArtifactAdmissionResult script_admission,
    const fs::path& root,
    const fs::path& script) {
    auto routes = load_polyglot_route_registry({{capability_id, "on", 0U}});
    expect_local(routes.ok(), "the Python sample route should load");
    auto candidate = invocation(root, script, {});
    candidate.invocation.correlation_id = "python-host-correlation";
    PolyglotRuntimeCapabilityBinding binding{
        .capability_id = capability_id,
        .artifact_admission = python,
        .supporting_artifact_admissions = {std::move(script_admission)},
        .supporting_artifact_arguments = script_binding(),
        .candidate_request_template = std::move(candidate),
        .invoke_native = {},
        .normalize_shadow_parity = {},
        .parity_policy = {}};
    return {std::move(routes.registry), {std::move(binding)}};
}

void test_prg_control(
    const PolyglotArtifactAdmissionResult& python,
    const fs::path& root,
    const fs::path& script) {
    auto script_admission = admit_script(script, root);
    const fs::path admitted_script =
        path_from_utf8_string(script_admission.resolved_path());
    auto invalid_configuration = configuration(
        python, script_admission, root, admitted_script);
    invalid_configuration.capabilities.front()
        .supporting_artifact_arguments.front().argument_index = 1U;
    const auto invalid_host = PolyglotRuntimeHost::create(
        std::move(invalid_configuration));
    expect_local(
        !invalid_host.ok() &&
            invalid_host.error_code ==
                "polyglot.host.supporting_artifact_binding_required",
        "host creation should reject a script bound to the wrong argument");

    auto built = PolyglotRuntimeHost::create(
        configuration(
            python,
            std::move(script_admission),
            root,
            admitted_script));
    expect_local(built.ok(), "the Python sidecar should bind to the trusted runtime host");
    if (!built.ok()) {
        return;
    }
    const auto callback = built.host->dispatch_callback();
    const fs::path program = root / "python-sidecar.prg";
    write_text(
        program,
        "cDispatch = CFPOLYGLOTDISPATCH('samples.python.add-v1', "
        "'{\"left\":20,\"right\":22}', 0)\n"
        "cStatus = CFJSONGET(cDispatch, '/status')\n"
        "cAuthority = CFJSONGET(cDispatch, '/authority')\n"
        "nSum = CFJSONGET(cDispatch, '/payload/sum')\n"
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
            value("cauthority") == "candidate" && value("nsum") == "42" &&
            value("ncandidatecalls") == "1",
        "ordinary PRG should control and inspect the external Python result");
}

fs::path g_supporting_artifact_rename_hook_original;
fs::path g_supporting_artifact_rename_hook_moved_aside;
bool g_supporting_artifact_rename_hook_renamed = false;

void supporting_artifact_rename_during_read_test_hook() {
    std::error_code rename_error;
    fs::rename(
        g_supporting_artifact_rename_hook_original,
        g_supporting_artifact_rename_hook_moved_aside,
        rename_error);
    g_supporting_artifact_rename_hook_renamed = !rename_error;
    if (g_supporting_artifact_rename_hook_renamed) {
        write_text(
            g_supporting_artifact_rename_hook_original,
            "replacement-supporting-artifact-bytes-different-content");
    }
}

void test_supporting_artifact_admission_rejects_rename_during_read(
    const fs::path& root) {
    const fs::path artifact = root / "rename-race.py";
    write_text(artifact, "original-supporting-artifact-bytes");
    const auto digest = copperfin::security::sha256_hex_for_file(utf8_path(artifact));
    expect_local(digest.ok, "the rename-race fixture artifact should be hashable");

    g_supporting_artifact_rename_hook_original = artifact;
    g_supporting_artifact_rename_hook_moved_aside = root / "rename-race-moved-aside.py";
    g_supporting_artifact_rename_hook_renamed = false;
    set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(
        &supporting_artifact_rename_during_read_test_hook);

    const auto admission = admit_polyglot_supporting_artifact({
        .capability_id = capability_id,
        .artifact_path = utf8_path(artifact),
        .allowed_root = utf8_path(root),
        .expected_sha256 = digest.hex_digest,
        .maximum_bytes = 1024U * 1024U});

    set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(nullptr);

    // Gracefully skip, like this codebase's other rename/hard-link
    // regression tests, rather than fail the suite when the environment
    // doesn't permit renaming an open file (e.g. a restricted sandbox).
    if (g_supporting_artifact_rename_hook_renamed) {
        expect_local(
            !admission.ok() && admission.error_code() ==
                "polyglot.supporting_artifact.changed_during_admission",
            "RQ-CF-CONTAINMENT-001: a supporting artifact renamed/replaced "
            "during the handle-based read must be rejected by the post-read "
            "path re-walk with a distinct artifact_changed error code, not "
            "conflated with the initial containment_denied case -- issue "
            "#5427 regression test");
    }

    // Clean up both the moved-aside original and the replacement the hook
    // wrote back at g_supporting_artifact_rename_hook_original -- root is
    // shared with the rest of this file's tests, which must not see a
    // stray leftover file here (found while investigating an unrelated
    // Windows CI failure in a downstream test after this one; the rename
    // succeeds on Windows since inspect_and_open_physically_contained_path()
    // opens with FILE_SHARE_DELETE, so this cleanup gap was real there, not
    // just on POSIX).
    // fs::remove(path, ec) clears ec (no error) when the path simply
    // doesn't exist -- e.g. the moved-aside file when the rename itself
    // was never permitted -- so checking cleanup_error after each call
    // distinguishes that from an actual removal failure, without a false
    // positive when there was nothing to clean up.
    std::error_code cleanup_error;
    fs::remove(g_supporting_artifact_rename_hook_moved_aside, cleanup_error);
    expect_local(
        !cleanup_error,
        "cleanup of the moved-aside rename-race file should not fail, or "
        "root may leak a stray file into later tests in this file");
    cleanup_error.clear();
    fs::remove(g_supporting_artifact_rename_hook_original, cleanup_error);
    expect_local(
        !cleanup_error,
        "cleanup of the replacement rename-race file should not fail, or "
        "root may leak a stray file into later tests in this file");
}

void test_supporting_artifact_revalidation_rejects_rename_during_read(
    const fs::path& root) {
    const fs::path artifact = root / "revalidate-rename-race.py";
    write_text(artifact, "original-revalidation-bytes");
    const auto digest = copperfin::security::sha256_hex_for_file(utf8_path(artifact));
    expect_local(digest.ok, "the revalidate rename-race fixture artifact should be hashable");

    auto admission = admit_polyglot_supporting_artifact({
        .capability_id = capability_id,
        .artifact_path = utf8_path(artifact),
        .allowed_root = utf8_path(root),
        .expected_sha256 = digest.hex_digest,
        .maximum_bytes = 1024U * 1024U});
    expect_local(
        admission.ok(),
        "the revalidate rename-race fixture admission should succeed before revalidation");

    g_supporting_artifact_rename_hook_original = artifact;
    g_supporting_artifact_rename_hook_moved_aside =
        root / "revalidate-rename-race-moved-aside.py";
    g_supporting_artifact_rename_hook_renamed = false;
    set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(
        &supporting_artifact_rename_during_read_test_hook);

    const bool revalidated = revalidate_polyglot_supporting_artifact_admission(admission);

    set_polyglot_supporting_artifact_admission_post_read_test_hook_for_testing(nullptr);

    // Gracefully skip, like the sibling admission-time test above, rather
    // than fail the suite when the environment doesn't permit renaming an
    // open file.
    if (g_supporting_artifact_rename_hook_renamed) {
        expect_local(
            !revalidated && !admission.ok() &&
                admission.error_code() ==
                    "polyglot.supporting_artifact.changed_before_execution",
            "issue #5409: a supporting artifact renamed/replaced during "
            "revalidate_polyglot_supporting_artifact_admission()'s "
            "handle-based read must be rejected by its own post-read path "
            "re-walk, not merely by the admission-time check this function "
            "no longer relies on -- proves the migration off the "
            "string-reopening primitive preserved (and here, since this "
            "call site previously had no post-read re-walk at all, adds) "
            "this guarantee");
    }

    std::error_code cleanup_error;
    fs::remove(g_supporting_artifact_rename_hook_moved_aside, cleanup_error);
    expect_local(
        !cleanup_error,
        "cleanup of the moved-aside revalidate rename-race file should not "
        "fail, or root may leak a stray file into later tests in this file");
    cleanup_error.clear();
    fs::remove(g_supporting_artifact_rename_hook_original, cleanup_error);
    expect_local(
        !cleanup_error,
        "cleanup of the replacement revalidate rename-race file should not "
        "fail, or root may leak a stray file into later tests in this file");
}
}  // namespace

int main() {
    std::error_code error;
    const fs::path root = unique_root();
    fs::create_directories(root, error);
    expect_local(!error, "the Python integration fixture root should be created");
    const fs::path python = fs::canonical(
        path_from_utf8_string(COPPERFIN_POLYGLOT_PYTHON_EXECUTABLE), error);
    expect_local(!error && fs::is_regular_file(python),
           "the Python interpreter should resolve to a regular file");
    const fs::path script = root / "candidate.py";
    fs::copy_file(
        path_from_utf8_string(COPPERFIN_POLYGLOT_PYTHON_SIDECAR_PATH),
        script,
        fs::copy_options::overwrite_existing,
        error);
    expect_local(!error, "the checked-in Python sidecar should copy into its admitted root");

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
        "supporting admission should reject an oversized script before reading it");

    test_supporting_artifact_admission_rejects_rename_during_read(root);
    test_supporting_artifact_revalidation_rejects_rename_during_read(root);

    auto python_admission = admit_python(python);
    expect_local(python_admission.ok(), "the Python interpreter should be admitted");
    if (python_admission.ok() && !error) {
        test_prg_control(python_admission, root, script);
        test_round_trip_and_fail_closed(python_admission, root, script);
    }
    fs::remove_all(root, error);
    if (failures == 0) {
        std::cout << "polyglot Python sidecar tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
