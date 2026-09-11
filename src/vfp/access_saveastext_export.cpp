// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/access_saveastext_export.h"

#include "copperfin/platform/bounded_process.h"
#include "copperfin/platform/json.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/polyglot_supporting_artifact_admission.h"
#include "copperfin/security/external_process_policy.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace copperfin::vfp {

namespace {

using copperfin::platform::admit_polyglot_supporting_artifact;
using copperfin::platform::BoundedProcessRequest;
using copperfin::platform::BoundedProcessStatus;
using copperfin::platform::JsonSelectionError;
using copperfin::platform::JsonValueKind;
using copperfin::platform::parse_json_document;
using copperfin::platform::PolyglotSupportingArtifactAdmissionRequest;
using copperfin::platform::PolyglotSupportingArtifactAdmissionResult;
using copperfin::platform::revalidate_polyglot_supporting_artifact_admission;
using copperfin::platform::run_bounded_process;
using copperfin::security::authorize_external_process;
using copperfin::security::ExternalProcessAuthorizationResult;
using copperfin::security::ExternalProcessPolicy;
using copperfin::security::revalidate_external_process_authorization;

// This capability's own identifier for the supporting-artifact admission
// bookkeeping that ties a script's admission record to the specific
// caller/purpose it was admitted for, matching
// samples/polyglot-python-sidecar/README.md's own "small leaf capability"
// naming convention (e.g. "samples.python.add-v1").
constexpr const char* kCapabilityId = "access.saveastext.export-v1";

// export_access_design.ps1 is a short, checked-in reference script
// (well under 64KiB); a generous but still bounded ceiling catches a
// tampered/oversized substitute before it is ever read into memory.
constexpr std::uint64_t kMaxScriptBytes = 256U * 1024U;

AccessSaveAsTextExportResult fail(
    AccessSaveAsTextExportError error, std::string message) {
    AccessSaveAsTextExportResult result;
    result.ok = false;
    result.error = error;
    result.error_message = std::move(message);
    return result;
}

bool request_is_valid(const AccessSaveAsTextExportRequest& request) {
    return !request.powershell_executable_path.empty() &&
        !request.script_path.empty() &&
        !request.script_allowed_root.empty() &&
        !request.expected_script_sha256.empty() &&
        !request.source_database_path.empty() &&
        !request.output_directory.empty();
}

std::string read_whole_file(const std::string& path, bool& ok) {
    std::ifstream input(
        copperfin::platform::path_from_utf8_string(path), std::ios::binary);
    if (!input.good()) {
        ok = false;
        return {};
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    ok = !input.bad();
    return buffer.str();
}

// Parses export_access_design.ps1's own manifest.json shape: a JSON
// array of {Kind, Name, File, Ok, Error} objects (the empty-array and
// single-element cases are both explicitly normalized by the script
// itself -- see that script's own comment on why -- so this parser does
// not need to special-case either).
bool parse_manifest(
    const std::string& manifest_json,
    std::vector<AccessSaveAsTextManifestEntry>& out_entries) {
    const auto parsed = parse_json_document(manifest_json);
    if (!parsed.ok()) {
        return false;
    }
    const auto root = parsed.document.select({});
    if (!root.ok() || root.kind != JsonValueKind::array) {
        return false;
    }

    std::vector<AccessSaveAsTextManifestEntry> entries;
    for (std::size_t index = 0U;; ++index) {
        const std::string element_pointer = "/" + std::to_string(index);
        const auto element = parsed.document.select(element_pointer);
        if (element.error == JsonSelectionError::value_not_found) {
            break;
        }
        if (!element.ok() || element.kind != JsonValueKind::object) {
            return false;
        }

        const auto kind = parsed.document.select(element_pointer + "/Kind");
        const auto name = parsed.document.select(element_pointer + "/Name");
        const auto file = parsed.document.select(element_pointer + "/File");
        const auto entry_ok = parsed.document.select(element_pointer + "/Ok");
        const auto error_text = parsed.document.select(element_pointer + "/Error");
        if (!kind.ok() || kind.kind != JsonValueKind::string ||
            !name.ok() || name.kind != JsonValueKind::string ||
            !file.ok() || file.kind != JsonValueKind::string ||
            !entry_ok.ok() || entry_ok.kind != JsonValueKind::boolean ||
            !error_text.ok() || error_text.kind != JsonValueKind::string) {
            return false;
        }

        entries.push_back({
            .kind = kind.decoded_string,
            .name = name.decoded_string,
            .file = file.decoded_string,
            .ok = (entry_ok.raw_json == "true"),
            .error = error_text.decoded_string});
    }

    out_entries = std::move(entries);
    return true;
}

}  // namespace

AccessSaveAsTextExportResult run_access_saveastext_export(
    const AccessSaveAsTextExportRequest& request) {
    if (!request_is_valid(request)) {
        return fail(
            AccessSaveAsTextExportError::invalid_request,
            "one or more required AccessSaveAsTextExportRequest fields were empty");
    }

    namespace fs = std::filesystem;
    const fs::path powershell_path =
        copperfin::platform::path_from_utf8_string(request.powershell_executable_path);

    // The PowerShell host itself is a well-known, OS-shipped executable
    // that changes with every Windows/PowerShell servicing update --
    // pinning it by exact byte digest the way the checked-in script is
    // pinned below would break on the next security patch. It is instead
    // authorized by physical location (an explicit allowed root, not an
    // ambient PATH search) -- the same authorize_external_process() path
    // policy/publisher/signature check every other admitted executable
    // in this codebase goes through, deliberately distinct from
    // admit_polyglot_supporting_artifact()'s digest-pinning, which is
    // reserved for small, static, checked-in files like the script
    // itself.
    ExternalProcessPolicy powershell_policy;
    powershell_policy.executable_name = request.powershell_executable_path;
    powershell_policy.allowed_path_roots = {
        copperfin::platform::path_to_utf8_string(powershell_path.parent_path())};
    powershell_policy.allowed_publishers = {};
    powershell_policy.require_trusted_signature = false;
    ExternalProcessAuthorizationResult powershell_authorization =
        authorize_external_process(powershell_policy);
    if (!powershell_authorization.allowed) {
        return fail(
            AccessSaveAsTextExportError::executable_admission_failed,
            powershell_authorization.error);
    }

    PolyglotSupportingArtifactAdmissionResult script_admission =
        admit_polyglot_supporting_artifact(PolyglotSupportingArtifactAdmissionRequest{
            .capability_id = kCapabilityId,
            .artifact_path = request.script_path,
            .allowed_root = request.script_allowed_root,
            .expected_sha256 = request.expected_script_sha256,
            .maximum_bytes = kMaxScriptBytes});
    if (!script_admission.ok()) {
        return fail(
            AccessSaveAsTextExportError::script_admission_failed,
            script_admission.error_code());
    }

    // Revalidate both tokens immediately beside the owned launch call --
    // neither admission result is trusted to still describe the file on
    // disk after any time has passed since it was first admitted.
    if (!revalidate_external_process_authorization(powershell_authorization)) {
        return fail(
            AccessSaveAsTextExportError::executable_revalidation_failed,
            powershell_authorization.error);
    }
    if (!revalidate_polyglot_supporting_artifact_admission(script_admission)) {
        return fail(
            AccessSaveAsTextExportError::script_revalidation_failed,
            script_admission.error_code());
    }

    // export_access_design.ps1 owns creating request.output_directory
    // itself (New-Item -Force, only reached after its own DatabasePath
    // existence check) -- eagerly creating it here first would silently
    // change that observable behavior (the directory would exist even
    // when the script fails before ever reaching that point).
    BoundedProcessRequest process_request;
    process_request.executable_path = powershell_authorization.resolved_path;
    // Fixed argument positions: the script's own admitted, revalidated
    // resolved_path always lands at index 5 of this literal vector, not
    // appended or interpolated from caller-controlled text at any other
    // position.
    process_request.arguments = {
        "-NoProfile",
        "-NonInteractive",
        "-ExecutionPolicy", "Bypass",
        "-File", script_admission.resolved_path(),
        "-DatabasePath", request.source_database_path,
        "-OutputDirectory", request.output_directory};
    // request.output_directory need not exist yet (the script itself
    // creates it) -- script_allowed_root always exists, since the
    // admitted script itself already resolved beneath it.
    process_request.working_directory = request.script_allowed_root;
#if defined(_WIN32)
    {
        std::vector<wchar_t> buffer(32768U, L'\0');
        const UINT length = ::GetWindowsDirectoryW(
            buffer.data(), static_cast<UINT>(buffer.size()));
        if (length != 0U && length < buffer.size()) {
            const std::wstring wide(buffer.data(), length);
            const std::string system_root = copperfin::platform::path_to_utf8_string(
                fs::path(wide));
            // powershell.exe itself needs SystemRoot to start at all --
            // the same explicit-not-ambient requirement
            // samples/polyglot-python-sidecar/'s own real-engine test
            // establishes for launching Python on Windows.
            process_request.environment = {{"SystemRoot", system_root}};
        }
    }
#else
    process_request.environment = {};
#endif
    process_request.timeout_ms = request.timeout_ms;
    process_request.stdout_limit_bytes = 1024U * 1024U;
    process_request.stderr_limit_bytes = 256U * 1024U;

    const auto process_result = run_bounded_process(process_request);

    AccessSaveAsTextExportResult result;
    result.standard_output = process_result.standard_output;
    result.standard_error = process_result.standard_error;
    result.exit_code = process_result.exit_code;

    if (!process_result.completed()) {
        switch (process_result.status) {
            case BoundedProcessStatus::timed_out:
                result.error = AccessSaveAsTextExportError::process_timed_out;
                break;
            case BoundedProcessStatus::cancelled:
                result.error = AccessSaveAsTextExportError::process_cancelled;
                break;
            case BoundedProcessStatus::output_limit_exceeded:
                result.error = AccessSaveAsTextExportError::process_output_limit_exceeded;
                break;
            default:
                result.error = AccessSaveAsTextExportError::process_launch_failed;
                break;
        }
        result.error_message = process_result.error_code;
        return result;
    }

    const fs::path manifest_path =
        copperfin::platform::path_from_utf8_string(request.output_directory) / "manifest.json";
    result.manifest_path = copperfin::platform::path_to_utf8_string(manifest_path);
    std::error_code manifest_exists_error;
    const bool manifest_exists =
        fs::is_regular_file(manifest_path, manifest_exists_error) && !manifest_exists_error;

    if (!manifest_exists) {
        // The script exits non-zero before writing manifest.json only
        // when the source database path did not exist (checked before
        // New-Item creates the output directory at all) -- every other
        // failure path still writes a manifest recording which specific
        // object(s) failed.
        result.error = AccessSaveAsTextExportError::manifest_missing;
        result.error_message = process_result.standard_error;
        return result;
    }

    bool read_ok = false;
    const std::string manifest_text = read_whole_file(result.manifest_path, read_ok);
    if (!read_ok) {
        result.error = AccessSaveAsTextExportError::manifest_unreadable;
        result.error_message = "failed to read " + result.manifest_path;
        return result;
    }

    std::vector<AccessSaveAsTextManifestEntry> entries;
    if (!parse_manifest(manifest_text, entries)) {
        result.error = AccessSaveAsTextExportError::manifest_invalid;
        result.error_message = "manifest.json did not match the expected shape";
        return result;
    }
    result.manifest = std::move(entries);

    if (process_result.exit_code != 0) {
        result.error = AccessSaveAsTextExportError::process_exited_with_failures;
        result.error_message = process_result.standard_error;
        return result;
    }

    result.ok = true;
    result.error = AccessSaveAsTextExportError::none;
    return result;
}

const char* access_saveastext_export_error_name(
    AccessSaveAsTextExportError error) noexcept {
    switch (error) {
        case AccessSaveAsTextExportError::none: return "none";
        case AccessSaveAsTextExportError::invalid_request: return "invalid_request";
        case AccessSaveAsTextExportError::executable_admission_failed:
            return "executable_admission_failed";
        case AccessSaveAsTextExportError::script_admission_failed:
            return "script_admission_failed";
        case AccessSaveAsTextExportError::executable_revalidation_failed:
            return "executable_revalidation_failed";
        case AccessSaveAsTextExportError::script_revalidation_failed:
            return "script_revalidation_failed";
        case AccessSaveAsTextExportError::process_launch_failed:
            return "process_launch_failed";
        case AccessSaveAsTextExportError::process_timed_out:
            return "process_timed_out";
        case AccessSaveAsTextExportError::process_cancelled:
            return "process_cancelled";
        case AccessSaveAsTextExportError::process_output_limit_exceeded:
            return "process_output_limit_exceeded";
        case AccessSaveAsTextExportError::manifest_missing:
            return "manifest_missing";
        case AccessSaveAsTextExportError::manifest_unreadable:
            return "manifest_unreadable";
        case AccessSaveAsTextExportError::manifest_invalid:
            return "manifest_invalid";
        case AccessSaveAsTextExportError::process_exited_with_failures:
            return "process_exited_with_failures";
    }
    return "unknown";
}

}  // namespace copperfin::vfp
