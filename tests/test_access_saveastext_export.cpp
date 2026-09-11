// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "copperfin/security/sha256.h"
#include "copperfin/vfp/access_saveastext_export.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

using namespace copperfin::vfp;
namespace fs = std::filesystem;

int failures = 0;

void expect(const bool condition, const std::string& message) {
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
    return copperfin::platform::path_to_utf8_string(path);
}

fs::path unique_root() {
    return fs::temp_directory_path() /
        ("copperfin_access_saveastext_export_" +
         std::to_string(process_id()) + "_" +
         std::to_string(
             std::chrono::steady_clock::now().time_since_epoch().count()));
}

void write_text_file(const fs::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary);
    output << content;
}

std::string hash_of(const fs::path& path) {
    const auto digest = copperfin::security::sha256_hex_for_file(utf8_path(path));
    expect(digest.ok, "test fixture should be hashable: " + utf8_path(path));
    return digest.hex_digest;
}

AccessSaveAsTextExportRequest base_request(
    const fs::path& powershell,
    const fs::path& script,
    const fs::path& root,
    const fs::path& source_db,
    const fs::path& output_dir) {
    AccessSaveAsTextExportRequest request;
    request.powershell_executable_path = utf8_path(powershell);
    request.script_path = utf8_path(script);
    request.script_allowed_root = utf8_path(root);
    request.expected_script_sha256 = hash_of(script);
    request.source_database_path = utf8_path(source_db);
    request.output_directory = utf8_path(output_dir);
    request.timeout_ms = 30000U;
    return request;
}

// #5477/#5478: the real checked-in export_access_design.ps1 requires a
// licensed Windows Access installation to complete a genuine export --
// unavailable in this test environment -- but this proves the
// admission-and-launch mechanics (script digest pinning, admitted root,
// revalidation, explicit environment, argument passing) run a real
// external PowerShell process end-to-end, the same "real interpreter,
// synthetic/unavailable payload" split test_polyglot_python_sidecar.cpp's
// own fail-closed cases already use.
void test_real_script_missing_database_fails_before_any_output(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    const fs::path output_dir = root / "missing_db_output";
    auto request = base_request(
        powershell, script, root, root / "does-not-exist.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(!result.ok, "a missing source database should fail the export");
    expect(result.error == AccessSaveAsTextExportError::manifest_missing,
           "a missing source database should fail before any manifest.json is written");
    expect(result.exit_code == 1, "export_access_design.ps1 exits 1 when the database path does not exist");
    expect(result.standard_error.find("Database not found") != std::string::npos,
           "the script's own diagnostic should surface in standard_error");
    expect(!fs::exists(output_dir),
           "the output directory should never be created when the database path check fails first");
}

void test_real_script_unavailable_access_automation_fails_closed(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    const fs::path source_db = root / "fixture.mdb";
    write_text_file(source_db, "not a real Access database -- content is irrelevant here");
    const fs::path output_dir = root / "unavailable_access_output";
    auto request = base_request(powershell, script, root, source_db, output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(!result.ok, "the export should fail when real Access automation is unavailable");
    expect(result.error == AccessSaveAsTextExportError::manifest_missing,
           "Access.Application COM creation fails before the try/finally block's manifest-writing code ever runs");
    expect(result.exit_code != 0, "a genuine process failure should surface a non-zero exit code");
    expect(!result.standard_error.empty(),
           "the real PowerShell process should report a non-empty diagnostic to standard_error");
    expect(fs::exists(output_dir),
           "the output directory is created (New-Item) before Access automation is even attempted");
    expect(!fs::exists(output_dir / "manifest.json"),
           "no manifest.json is ever written when Access.Application itself could not be created");
}

void test_admission_rejects_tampered_script(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    auto request = base_request(
        powershell, script, root, root / "unused.mdb", root / "tamper_output");
    // Deliberately wrong digest -- the script on disk is genuinely
    // unmodified, but the pinned "expected" value the caller supplies
    // does not match it, exactly modeling a tampered/substituted file.
    request.expected_script_sha256 =
        "0000000000000000000000000000000000000000000000000000000000000";
    const auto result = run_access_saveastext_export(request);
    expect(!result.ok, "a digest mismatch should fail admission");
    expect(result.error == AccessSaveAsTextExportError::script_admission_failed,
           "a digest mismatch should be reported as script_admission_failed, not a process failure");
}

void test_admission_rejects_script_outside_allowed_root(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    const fs::path unrelated_root = unique_root();
    std::error_code ignored;
    fs::create_directories(unrelated_root, ignored);
    auto request = base_request(
        powershell, script, unrelated_root, root / "unused.mdb", root / "wrong_root_output");
    const auto result = run_access_saveastext_export(request);
    expect(!result.ok, "a script outside its claimed allowed root should fail admission");
    expect(result.error == AccessSaveAsTextExportError::script_admission_failed,
           "a containment failure should be reported as script_admission_failed");
    fs::remove_all(unrelated_root, ignored);
}

void test_invalid_request_never_launches_a_process(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    auto request = base_request(
        powershell, script, root, root / "unused.mdb", root / "invalid_output");
    request.output_directory.clear();
    const auto result = run_access_saveastext_export(request);
    expect(!result.ok, "an empty required field should fail before admission");
    expect(result.error == AccessSaveAsTextExportError::invalid_request,
           "an empty required field should be reported as invalid_request");
    expect(result.exit_code == -1, "invalid_request must never reach process launch");
}

// A small synthetic PowerShell script (not the real export_access_design.ps1)
// exercises run_access_saveastext_export()'s manifest-parsing code paths
// via a genuine real-process round trip, without needing real Access
// automation to reach a successful/partial-failure/malformed manifest.
fs::path write_synthetic_script(const fs::path& root, const std::string& body) {
    const fs::path path = root / "synthetic_export.ps1";
    write_text_file(path, body);
    return path;
}

constexpr const char* kSyntheticSuccessBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$manifest = @(
    [PSCustomObject]@{ Kind = "Form"; Name = "frmOne"; File = (Join-Path $OutputDirectory "Form_frmOne.txt"); Ok = $true; Error = "" }
    [PSCustomObject]@{ Kind = "Report"; Name = "rptTwo"; File = (Join-Path $OutputDirectory "Report_rptTwo.txt"); Ok = $true; Error = "" }
)
$manifest | ConvertTo-Json -Depth 4 | Out-File -FilePath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8
exit 0
)ps1";

constexpr const char* kSyntheticPartialFailureBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$manifest = @(
    [PSCustomObject]@{ Kind = "Form"; Name = "frmOk"; File = (Join-Path $OutputDirectory "Form_frmOk.txt"); Ok = $true; Error = "" }
    [PSCustomObject]@{ Kind = "Module"; Name = "modBroken"; File = (Join-Path $OutputDirectory "Module_modBroken.txt"); Ok = $false; Error = "simulated export failure" }
)
$manifest | ConvertTo-Json -Depth 4 | Out-File -FilePath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8
exit 1
)ps1";

constexpr const char* kSyntheticInvalidManifestBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
"this is not a JSON array of manifest entries" | Out-File -FilePath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8
exit 0
)ps1";

void test_synthetic_success_round_trip(const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticSuccessBody);
    const fs::path output_dir = root / "synthetic_success_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(result.ok, "a full-success synthetic run should report ok: " + result.error_message +
        " / stderr: " + result.standard_error);
    expect(result.error == AccessSaveAsTextExportError::none, "a successful run should carry no error");
    expect(result.exit_code == 0, "the synthetic success script exits 0");
    expect(result.manifest.size() == 2U, "both manifest entries should be parsed");
    if (result.manifest.size() == 2U) {
        expect(result.manifest[0].kind == "Form" && result.manifest[0].name == "frmOne" &&
                   result.manifest[0].ok && result.manifest[0].error.empty(),
               "the first manifest entry's fields should round-trip exactly");
        expect(result.manifest[1].kind == "Report" && result.manifest[1].name == "rptTwo" &&
                   result.manifest[1].ok,
               "the second manifest entry's fields should round-trip exactly");
    }
}

void test_synthetic_partial_failure_still_exposes_manifest(
    const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticPartialFailureBody);
    const fs::path output_dir = root / "synthetic_partial_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(!result.ok, "a partial-failure run should not report ok");
    expect(result.error == AccessSaveAsTextExportError::process_exited_with_failures,
           "a partial failure with a written manifest should be reported as process_exited_with_failures");
    expect(result.manifest.size() == 2U,
           "the manifest should still be parsed and exposed so the caller can see which object failed");
    if (result.manifest.size() == 2U) {
        expect(result.manifest[0].ok && !result.manifest[1].ok,
               "the manifest should distinguish the successful object from the failed one");
        expect(result.manifest[1].error == "simulated export failure",
               "the failed object's own error text should be preserved");
    }
}

void test_synthetic_invalid_manifest_fails_closed(
    const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticInvalidManifestBody);
    const fs::path output_dir = root / "synthetic_invalid_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(!result.ok, "a malformed manifest.json should not report ok even though the process exited 0");
    expect(result.error == AccessSaveAsTextExportError::manifest_invalid,
           "a manifest.json that is not the expected shape should fail closed as manifest_invalid");
    expect(result.manifest.empty(), "no manifest entries should be trusted from an invalid document");
}

}  // namespace

int main() {
    if (!fs::exists(fs::path(COPPERFIN_ACCESS_SAVEASTEXT_EXPORT_SCRIPT_PATH))) {
        std::cerr << "the checked-in export_access_design.ps1 should exist at the configured path\n";
        return 1;
    }

    std::error_code error;
    const fs::path powershell = fs::canonical(
        copperfin::platform::resolve_executable_invocation_path(
            COPPERFIN_POLYGLOT_POWERSHELL_EXECUTABLE),
        error);
    expect(!error && fs::is_regular_file(powershell),
           "the configured PowerShell host should resolve to a regular file");

    const fs::path root = unique_root();
    fs::create_directories(root, error);
    expect(!error, "the test fixture root should be created");

    const fs::path script = root / "export_access_design.ps1";
    fs::copy_file(
        fs::path(COPPERFIN_ACCESS_SAVEASTEXT_EXPORT_SCRIPT_PATH),
        script,
        fs::copy_options::overwrite_existing,
        error);
    expect(!error, "the checked-in export script should copy into its admitted root");

    if (!error) {
        test_real_script_missing_database_fails_before_any_output(powershell, script, root);
        test_real_script_unavailable_access_automation_fails_closed(powershell, script, root);
        test_admission_rejects_tampered_script(powershell, script, root);
        test_admission_rejects_script_outside_allowed_root(powershell, script, root);
        test_invalid_request_never_launches_a_process(powershell, script, root);
        test_synthetic_success_round_trip(powershell, root);
        test_synthetic_partial_failure_still_exposes_manifest(powershell, root);
        test_synthetic_invalid_manifest_fails_closed(powershell, root);
    }

    fs::remove_all(root, error);
    if (failures == 0) {
        std::cout << "access saveastext export tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
