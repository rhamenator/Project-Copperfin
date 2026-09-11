// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
    // A genuinely independent field from powershell_executable_path --
    // see access_saveastext_export.h's own comment on why deriving it
    // from the executable path itself would be tautological (#5562
    // review). This test's own powershell binary genuinely does live
    // under its own parent directory, so this value happens to match
    // what an auto-derived root would have been, but it is supplied
    // here as its own explicit value, not computed from
    // powershell_executable_path.
    request.powershell_allowed_root = utf8_path(powershell.parent_path());
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

// #5562 review (copilot-pull-request-reviewer): a relative path is
// unsafe here even when nonempty, since the child process and this
// function's own later manifest read would resolve it against two
// different directories.
void test_invalid_request_rejects_relative_output_directory(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    auto request = base_request(
        powershell, script, root, root / "unused.mdb", root / "relative_output");
    request.output_directory = "relative_output_dir";
    const auto result = run_access_saveastext_export(request);
    expect(!result.ok, "a relative output_directory should fail before admission");
    expect(result.error == AccessSaveAsTextExportError::invalid_request,
           "a relative required path should be reported as invalid_request");
    expect(result.exit_code == -1, "invalid_request must never reach process launch");
}

// #5562 review (chatgpt-codex-connector and copilot-pull-request-reviewer,
// independently): deriving powershell_allowed_root from
// powershell_executable_path itself made the containment check
// tautological. This proves the two are now genuinely independent
// fields: an allowed root that does not actually contain the real
// PowerShell binary must be rejected, even though the executable path
// itself is completely unchanged and genuinely valid.
void test_admission_rejects_powershell_outside_its_claimed_allowed_root(
    const fs::path& powershell, const fs::path& script, const fs::path& root) {
    const fs::path unrelated_root = unique_root();
    std::error_code ignored;
    fs::create_directories(unrelated_root, ignored);
    auto request = base_request(
        powershell, script, root, root / "unused.mdb", root / "wrong_powershell_root_output");
    request.powershell_allowed_root = utf8_path(unrelated_root);
    const auto result = run_access_saveastext_export(request);
    expect(!result.ok, "a PowerShell host outside its claimed allowed root should fail admission");
    expect(result.error == AccessSaveAsTextExportError::executable_admission_failed,
           "a containment failure for the host itself should be reported as executable_admission_failed");
    fs::remove_all(unrelated_root, ignored);
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

// #5562 review (chatgpt-codex-connector): the real
// export_access_design.ps1 previously wrapped ConvertTo-Json's own
// already-correct one-element-array output in a second, redundant pair
// of brackets, producing "[[{...}]]" for a database with exactly one
// exported object. This mirrors that script's own now-fixed
// single-branch serialization exactly (see this file's own comment on
// that fix) to prove the parser accepts the real shape a one-object
// database actually produces.
constexpr const char* kSyntheticSingletonManifestBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$manifest = New-Object System.Collections.ArrayList
[void]$manifest.Add([PSCustomObject]@{ Kind = "Form"; Name = "frmOnly"; File = (Join-Path $OutputDirectory "Form_frmOnly.txt"); Ok = $true; Error = "" })
$manifestJson = ConvertTo-Json -InputObject @($manifest) -Depth 4
$manifestJson | Out-File -FilePath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8
exit 0
)ps1";

// #5562 review (chatgpt-codex-connector and copilot-pull-request-reviewer,
// independently): Windows PowerShell 5.1's `Out-File -Encoding utf8` --
// the exact encoding the real script uses -- always writes a leading
// UTF-8 BOM; this dev environment's own PowerShell 7 does not (directly
// confirmed empirically), which is why this went unnoticed until
// review. Writes the BOM bytes explicitly via .NET's File API to
// reproduce that real-target behavior regardless of which PowerShell
// version actually runs this test.
constexpr const char* kSyntheticBomManifestBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$manifest = @(
    [PSCustomObject]@{ Kind = "Form"; Name = "frmBom"; File = (Join-Path $OutputDirectory "Form_frmBom.txt"); Ok = $true; Error = "" }
)
$json = ConvertTo-Json -InputObject $manifest -Depth 4
$bom = [byte[]](0xEF, 0xBB, 0xBF)
$bytes = $bom + [System.Text.Encoding]::UTF8.GetBytes($json)
[System.IO.File]::WriteAllBytes((Join-Path $OutputDirectory "manifest.json"), $bytes)
exit 0
)ps1";

// #5562 review (copilot-pull-request-reviewer): proves manifest_too_large
// is reported from the file's own size, not from reading it in full and
// having the JSON parser reject it afterward -- writes a manifest.json
// larger than parse_json_document()'s own 1 MiB default document-size
// limit.
constexpr const char* kSyntheticOversizedManifestBody = R"ps1(
param(
    [Parameter(Mandatory = $true)][string]$DatabasePath,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$filler = "x" * (2 * 1024 * 1024)
$filler | Out-File -FilePath (Join-Path $OutputDirectory "manifest.json") -Encoding utf8
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

void test_synthetic_singleton_manifest_round_trip(const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticSingletonManifestBody);
    const fs::path output_dir = root / "synthetic_singleton_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(result.ok, "a single-object export using the real script's own fixed serialization logic "
        "should report ok: " + result.error_message + " / stderr: " + result.standard_error);
    expect(result.manifest.size() == 1U,
           "exactly one manifest entry should be parsed from the real single-object array shape");
    if (result.manifest.size() == 1U) {
        expect(result.manifest[0].kind == "Form" && result.manifest[0].name == "frmOnly" &&
                   result.manifest[0].ok,
               "the singleton manifest entry's fields should round-trip exactly");
    }
}

void test_synthetic_manifest_with_bom_is_parsed(const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticBomManifestBody);
    const fs::path output_dir = root / "synthetic_bom_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(result.ok, "a manifest.json carrying a leading UTF-8 BOM (Windows PowerShell 5.1's own "
        "Out-File -Encoding utf8 behavior) should still parse successfully: " + result.error_message);
    expect(result.manifest.size() == 1U, "the BOM-prefixed manifest's single entry should be parsed");
    if (result.manifest.size() == 1U) {
        expect(result.manifest[0].name == "frmBom", "the BOM-prefixed manifest entry's fields should round-trip exactly");
    }
}

void test_synthetic_oversized_manifest_fails_closed(const fs::path& powershell, const fs::path& root) {
    const fs::path script = write_synthetic_script(root, kSyntheticOversizedManifestBody);
    const fs::path output_dir = root / "synthetic_oversized_output";
    auto request = base_request(powershell, script, root, root / "unused.mdb", output_dir);
    const auto result = run_access_saveastext_export(request);

    expect(!result.ok, "an oversized manifest.json should not report ok");
    expect(result.error == AccessSaveAsTextExportError::manifest_too_large,
           "a manifest.json over the JSON parser's own document-size limit should fail closed as manifest_too_large");
    expect(result.manifest.empty(), "no manifest entries should be trusted from an oversized document");
}

}  // namespace

int main() {
    if (!fs::exists(fs::path(COPPERFIN_ACCESS_SAVEASTEXT_EXPORT_SCRIPT_PATH))) {
        std::cerr << "the checked-in export_access_design.ps1 should exist at the configured path\n";
        return 1;
    }

    // Matches test_polyglot_r_sidecar.cpp's own resolution exactly --
    // CMake's find_program() already returns an absolute path, so no
    // further PATH-search resolution is needed or appropriate here.
    // fs::canonical() alone still resolves through any symlink the
    // discovered path itself is.
    std::error_code error;
    const fs::path powershell = fs::canonical(
        fs::path(COPPERFIN_POLYGLOT_POWERSHELL_EXECUTABLE), error);
    const bool powershell_resolved = !error && fs::is_regular_file(powershell);
    expect(powershell_resolved, "the configured PowerShell host should resolve to a regular file");
    if (!powershell_resolved) {
        std::cerr << "configured path: " << COPPERFIN_POLYGLOT_POWERSHELL_EXECUTABLE
                   << "\ncanonical result: " << powershell.string()
                   << "\nerror: " << error.message() << '\n';
    }

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

    if (powershell_resolved && !error) {
        test_real_script_missing_database_fails_before_any_output(powershell, script, root);
        test_real_script_unavailable_access_automation_fails_closed(powershell, script, root);
        test_admission_rejects_tampered_script(powershell, script, root);
        test_admission_rejects_script_outside_allowed_root(powershell, script, root);
        test_admission_rejects_powershell_outside_its_claimed_allowed_root(powershell, script, root);
        test_invalid_request_never_launches_a_process(powershell, script, root);
        test_invalid_request_rejects_relative_output_directory(powershell, script, root);
        test_synthetic_success_round_trip(powershell, root);
        test_synthetic_partial_failure_still_exposes_manifest(powershell, root);
        test_synthetic_invalid_manifest_fails_closed(powershell, root);
        test_synthetic_singleton_manifest_round_trip(powershell, root);
        test_synthetic_manifest_with_bom_is_parsed(powershell, root);
        test_synthetic_oversized_manifest_fails_closed(powershell, root);
    }

    fs::remove_all(root, error);
    if (failures == 0) {
        std::cout << "access saveastext export tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
