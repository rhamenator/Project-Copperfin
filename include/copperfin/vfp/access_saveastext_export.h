// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::vfp {

// #5477/#5478 (parent #138): wires
// samples/access-saveastext-export/export_access_design.ps1 -- the
// PowerShell reference script that drives real Access automation
// (Application.SaveAsText) to export every Form, Report, and standalone
// Module in an Access database -- into Copperfin's own C++ runtime, with
// the same external-process admission treatment
// samples/polyglot-python-sidecar/README.md establishes: a pinned
// lowercase SHA-256 digest and admitted physical root for the script, a
// fixed argument position for its resolved path, revalidation
// immediately before launch, and a complete explicit child environment
// (no ambient host/agent variables). parse_access_saveastext_design_from_file()
// (access_saveastext_design.h) then parses each per-object .txt file this
// helper's manifest names.
//
// This module does not itself resolve where the script or a
// PowerShell host live on a real installation, nor does it compute the
// script's own pinned digest from the file it is about to run --
// pinning against a value computed from the same file being checked
// would defeat the whole point of pinning (a tampered file would simply
// produce a different "expected" digest to match itself). The caller
// supplies both, sourced out-of-band (e.g. a constant recorded when the
// exact script version shipped with a given Copperfin build was last
// reviewed), exactly the separation of concerns
// admit_polyglot_supporting_artifact() already establishes for every
// other checked-in sidecar script this codebase runs.
//
// Access COM automation -- and therefore this whole feature -- only
// exists on Windows with a licensed Access installation present. This
// module's own admission-and-launch mechanics are portable and directly
// testable on any platform PowerShell (pwsh) runs on (proven via a real,
// non-Access-dependent process round trip in this module's own test
// suite), but a real end-to-end SaveAsText export requires both.

struct AccessSaveAsTextManifestEntry {
    // "Form", "Report", or "Module" -- the export_access_design.ps1
    // script's own Kind label, verbatim.
    std::string kind;
    std::string name;
    // The per-object .txt file export_access_design.ps1 wrote (or
    // attempted to write) via Application.SaveAsText, suitable for
    // parse_access_saveastext_design_from_file().
    std::string file;
    bool ok = false;
    // The Access/PowerShell-reported error message for this one object,
    // empty when ok is true.
    std::string error;
};

enum class AccessSaveAsTextExportError {
    none,
    invalid_request,
    executable_admission_failed,
    script_admission_failed,
    executable_revalidation_failed,
    script_revalidation_failed,
    process_launch_failed,
    process_timed_out,
    process_cancelled,
    process_output_limit_exceeded,
    // The process exited non-zero and no manifest.json was ever written
    // (e.g. the source database path did not exist) -- standard_error
    // carries the PowerShell/Access-level diagnostic text.
    manifest_missing,
    manifest_unreadable,
    manifest_invalid,
    // manifest.json exceeded the JSON parser's own document-size limit
    // -- rejected by its file size alone, before ever being read into
    // memory (#5562 review, copilot-pull-request-reviewer: reading an
    // unbounded file first, only to have the parser reject it afterward,
    // lets an oversized/malformed manifest cause unbounded memory growth
    // in this process).
    manifest_too_large,
    // The process exited non-zero, but did write a manifest.json listing
    // which specific object(s) failed -- manifest is still populated so
    // the caller can see exactly what went wrong per-object.
    process_exited_with_failures,
};

struct AccessSaveAsTextExportRequest {
    // The admitted PowerShell host's absolute, resolved executable path.
    // export_access_design.ps1's own header comment targets Windows
    // PowerShell 5.1 (powershell.exe) specifically, not PowerShell 7's
    // separately-installed pwsh.exe -- resolving and choosing between
    // them is the caller's responsibility, not this function's.
    std::string powershell_executable_path;
    // The one explicit, independently trusted physical root the
    // PowerShell host must live under (e.g. Windows' own
    // "%SystemRoot%\System32\WindowsPowerShell\v1.0" for the documented
    // 5.1 target) -- deliberately a *separate* field from
    // powershell_executable_path, not derived from it (#5562 review,
    // chatgpt-codex-connector and copilot-pull-request-reviewer,
    // independently: deriving the allowed root from the same candidate
    // path being checked makes the containment check tautological --
    // any caller-supplied executable would automatically be "inside its
    // own directory" -- exactly the same independent-root relationship
    // script_allowed_root already has to script_path below, which this
    // field now mirrors for the executable side too).
    std::string powershell_allowed_root;
    // export_access_design.ps1's own deployed location on this
    // installation. Must resolve beneath script_allowed_root.
    std::string script_path;
    // The one explicit physical root the script must live under --
    // admission is denied if script_path resolves outside it.
    std::string script_allowed_root;
    // The script's pinned lowercase SHA-256 digest. See this header's
    // own top comment: never compute this from script_path itself.
    std::string expected_script_sha256;
    // The source .mdb/.accdb to export.
    std::string source_database_path;
    // Where per-object .txt exports and manifest.json are written. Must
    // not already contain a manifest.json this run did not itself
    // produce -- a caller reusing a directory across runs is responsible
    // for its own cleanup.
    std::string output_directory;
    // Real Access automation opening a large database and exporting many
    // objects can take a while; default generous relative to the
    // in-process bounded_process.h default (5000ms).
    std::uint32_t timeout_ms = 120000U;
};

struct AccessSaveAsTextExportResult {
    bool ok = false;
    AccessSaveAsTextExportError error = AccessSaveAsTextExportError::none;
    // A diagnostic string safe to surface to an operator -- never
    // localized here (this is a library-level result, not a PRG runtime
    // diagnostic); a caller wiring this into PRG dispatch localizes its
    // own error message keyed off `error`.
    std::string error_message;
    // Populated whenever a manifest.json was successfully read, even on
    // a partial-failure run (see AccessSaveAsTextExportError::process_exited_with_failures).
    std::vector<AccessSaveAsTextManifestEntry> manifest;
    std::string manifest_path;
    std::string standard_output;
    std::string standard_error;
    int exit_code = -1;
};

// Admits the PowerShell host and the export script, revalidates both
// immediately before launch, runs export_access_design.ps1 against
// `request.source_database_path`, and reads back the manifest.json it
// writes into `request.output_directory`. Never executes a shell, never
// inherits ambient host/agent environment variables, and never launches
// anything the immediately-preceding revalidation did not just confirm
// is still the exact admitted file.
//
// Every path field must be absolute (#5562 review, copilot-pull-
// request-reviewer: the child process's own working directory is
// `request.script_allowed_root`, so a relative `source_database_path`/
// `output_directory` would resolve there from the child's perspective,
// while this function's own manifest read afterward resolves a relative
// `output_directory` against this process's own current directory
// instead -- a caller-visible relative path could silently write to one
// location and read back from another). A request with any relative
// path fails closed as invalid_request before anything is admitted.
[[nodiscard]] AccessSaveAsTextExportResult run_access_saveastext_export(
    const AccessSaveAsTextExportRequest& request);

[[nodiscard]] const char* access_saveastext_export_error_name(
    AccessSaveAsTextExportError error) noexcept;

}  // namespace copperfin::vfp
