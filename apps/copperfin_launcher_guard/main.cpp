// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/package/launcher_inventory_trust.h"
#include "copperfin/package/launcher_trust_registry_generated.h"
#include "copperfin/platform/path.h"
#include "copperfin/security/physical_path_containment.h"
#include "copperfin/security/sha256.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr int kManifestMissingExitCode = 3;
constexpr int kVerificationFailedExitCode = 4;
constexpr int kTargetStartFailedExitCode = 5;
constexpr std::string_view kInternalAppHostName = "Copperfin.GeneratedLauncher.apphost.exe";
constexpr std::string_view kRequiredSidecarDll = "Copperfin.GeneratedLauncher.dll";
constexpr std::string_view kRequiredSidecarDeps = "Copperfin.GeneratedLauncher.deps.json";
constexpr std::string_view kRequiredSidecarRuntimeConfig =
    "Copperfin.GeneratedLauncher.runtimeconfig.json";
constexpr std::string_view kTrustEnvelopeName = "app.cftrust";
constexpr std::string_view kTrustSignatureName = "app.cftrust.sig";

bool regular_file_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_regular_file(path, error) && !error;
}

std::string unquote_manifest_value(std::string_view value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1U >= value.size()) {
            result.push_back(value[index]);
            continue;
        }
        const char escaped = value[++index];
        switch (escaped) {
            case 'n':
                result.push_back('\n');
                break;
            case 'r':
                result.push_back('\r');
                break;
            default:
                result.push_back(escaped);
                break;
        }
    }
    return result;
}

std::vector<std::string> split_manifest_fields(std::string_view value) {
    std::vector<std::string> fields;
    std::size_t field_start = 0U;
    bool escaped = false;
    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (escaped) {
            escaped = false;
            continue;
        }
        if (value[index] == '\\') {
            escaped = true;
            continue;
        }
        if (value[index] == '|') {
            fields.push_back(unquote_manifest_value(value.substr(field_start, index - field_start)));
            field_start = index + 1U;
        }
    }
    fields.push_back(unquote_manifest_value(value.substr(field_start)));
    return fields;
}

struct ArtifactRecord {
    std::string relative_path;
    std::string role;
    std::string digest;
};

std::vector<ArtifactRecord> launcher_artifacts(const std::filesystem::path& manifest) {
    std::vector<ArtifactRecord> records;
    std::ifstream input(manifest, std::ios::binary);
    if (!input) {
        return records;
    }
    std::string line;
    constexpr std::string_view prefix = "launcher_artifact=";
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.starts_with(prefix)) {
            continue;
        }
        const auto fields = split_manifest_fields(std::string_view(line).substr(prefix.size()));
        if (fields.size() != 3U) {
            return {};
        }
        records.push_back({fields[0], fields[1], fields[2]});
    }
    return records;
}

bool same_name(std::string_view left, std::string_view right) {
    if (left.size() != right.size()) {
        return false;
    }
    return std::equal(left.begin(), left.end(), right.begin(), [](char left_char, char right_char) {
        if (left_char >= 'A' && left_char <= 'Z') {
            left_char = static_cast<char>(left_char + ('a' - 'A'));
        }
        if (right_char >= 'A' && right_char <= 'Z') {
            right_char = static_cast<char>(right_char + ('a' - 'A'));
        }
        return left_char == right_char;
    });
}

std::string narrow_utf8_argument(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }
    const int required = WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0,
        nullptr,
        nullptr);
    if (required <= 0) {
        return std::string(value.begin(), value.end());
    }
    std::string result(static_cast<std::size_t>(required), '\0');
    WideCharToMultiByte(
        CP_UTF8,
        WC_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        required,
        nullptr,
        nullptr);
    return result;
}

bool is_required_name(std::string_view name) {
    return same_name(name, kInternalAppHostName) ||
        same_name(name, kRequiredSidecarDll) ||
        same_name(name, kRequiredSidecarDeps) ||
        same_name(name, kRequiredSidecarRuntimeConfig);
}

std::filesystem::path executable_path() {
    std::wstring buffer(32768U, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0U || length >= buffer.size()) {
        return {};
    }
    buffer.resize(length);
    return std::filesystem::path(buffer);
}

std::wstring quote_windows_argument(const std::wstring& value) {
    if (!value.empty() && value.find_first_of(L" \t\n\v\"") == std::wstring::npos) {
        return value;
    }
    std::wstring quoted = L"\"";
    std::size_t backslashes = 0U;
    for (const wchar_t ch : value) {
        if (ch == L'\\') {
            ++backslashes;
        } else if (ch == L'\"') {
            quoted.append(backslashes * 2U + 1U, L'\\');
            quoted.push_back(ch);
            backslashes = 0U;
        } else {
            quoted.append(backslashes, L'\\');
            backslashes = 0U;
            quoted.push_back(ch);
        }
    }
    quoted.append(backslashes * 2U, L'\\');
    quoted.push_back(L'\"');
    return quoted;
}

std::string localized_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view key,
    const std::string& path = {}) {
    return catalog.translate(key, { {"path", path} });
}

std::optional<std::string> read_trust_file(
    const std::filesystem::path& package_root,
    const std::filesystem::path& relative_path,
    std::string& error,
    const copperfin::localization::LocalizedCatalog& catalog) {
    const auto containment = copperfin::security::inspect_physical_path_containment(
        package_root / relative_path,
        package_root);
    if (!containment.allowed) {
        error = localized_message(
            catalog,
            "Runtime.Package.LauncherGuard.Error.TrustFileRejected",
            copperfin::platform::path_to_utf8_string(relative_path));
        return std::nullopt;
    }
    const auto snapshot = copperfin::security::read_physically_contained_file_snapshot(
        containment,
        package_root);
    if (!snapshot.ok) {
        error = localized_message(
            catalog,
            "Runtime.Package.LauncherGuard.Error.TrustFileRejected",
            copperfin::platform::path_to_utf8_string(relative_path));
        return std::nullopt;
    }
    return snapshot.bytes;
}

bool verify_launcher_trust(
    const std::filesystem::path& package_root,
    const std::filesystem::path& manifest,
    std::string& error,
    const copperfin::localization::LocalizedCatalog& catalog) {
    const std::filesystem::path envelope_path = package_root / kTrustEnvelopeName;
    const std::filesystem::path signature_path = package_root / kTrustSignatureName;
    const bool envelope_present = regular_file_exists_without_error(envelope_path);
    const bool signature_present = regular_file_exists_without_error(signature_path);

    if (!envelope_present && !signature_present) {
#if defined(COPPERFIN_ENFORCE_LAUNCHER_TRUST)
        error = localized_message(
            catalog,
            "Runtime.Package.LauncherGuard.Error.TrustRequired");
        return false;
#else
        return true;
#endif
    }
    if (!envelope_present || !signature_present) {
        error = localized_message(
            catalog,
            "Runtime.Package.LauncherGuard.Error.TrustRequired");
        return false;
    }

    const auto envelope = read_trust_file(
        package_root,
        std::filesystem::path(kTrustEnvelopeName),
        error,
        catalog);
    if (!envelope.has_value()) {
        return false;
    }
    const auto signature = read_trust_file(
        package_root,
        std::filesystem::path(kTrustSignatureName),
        error,
        catalog);
    if (!signature.has_value()) {
        return false;
    }

    const auto result = copperfin::package_trust::verify_signed_launcher_inventory(
        *envelope,
        *signature,
        copperfin::package_trust::kKnownLauncherInventoryTrustedKeys);
    using VerificationStatus =
        copperfin::package_trust::LauncherInventoryVerificationStatus;
    switch (result.status) {
        case VerificationStatus::valid:
            {
                const auto manifest_records = launcher_artifacts(manifest);
                std::vector<copperfin::package_trust::LauncherInventoryArtifact> artifacts;
                artifacts.reserve(manifest_records.size());
                for (const auto& record : manifest_records) {
                    artifacts.push_back({record.role, record.relative_path, record.digest});
                }
                if (!copperfin::package_trust::launcher_inventory_envelope_matches_artifacts(
                        *envelope,
                        result.signer_key_id,
                        artifacts)) {
                    error = localized_message(
                        catalog,
                        "Runtime.Package.LauncherGuard.Error.TrustInvalid");
                    return false;
                }
            }
            return true;
        case VerificationStatus::unknown_signer:
            error = localized_message(
                catalog,
                "Runtime.Package.LauncherGuard.Error.TrustUnknownSigner");
            return false;
        case VerificationStatus::ambiguous_signer:
        case VerificationStatus::malformed_envelope:
        case VerificationStatus::invalid_signature:
            error = localized_message(
                catalog,
                "Runtime.Package.LauncherGuard.Error.TrustInvalid");
            return false;
    }
    error = localized_message(
        catalog,
        "Runtime.Package.LauncherGuard.Error.TrustInvalid");
    return false;
}

bool verify_artifacts(
    const std::filesystem::path& package_root,
    const std::filesystem::path& manifest,
    const std::filesystem::path& guard_path,
    std::string& error,
    const copperfin::localization::LocalizedCatalog& catalog,
    copperfin::security::PhysicalPathIdentity& verified_target_identity) {
    const auto records = launcher_artifacts(manifest);
    if (records.empty()) {
        error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryMissing");
        return false;
    }

    bool saw_public = false;
    bool saw_internal_target = false;
    std::vector<std::string> seen_paths;
    std::size_t required_count = 0U;
    for (const auto& record : records) {
        const std::filesystem::path relative = copperfin::platform::path_from_utf8_string(record.relative_path);
        if (relative.empty() || relative.is_absolute() || relative.has_parent_path() || record.digest.size() != 64U) {
            error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryInvalid", record.relative_path);
            return false;
        }
        if (std::any_of(seen_paths.begin(), seen_paths.end(), [&](const std::string& path) {
                return same_name(path, record.relative_path);
            })) {
            error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryInvalid", record.relative_path);
            return false;
        }
        seen_paths.push_back(record.relative_path);
        const auto containment = copperfin::security::inspect_physical_path_containment(
            package_root / relative,
            package_root);
        if (!containment.allowed) {
            error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.ArtifactRejected", record.relative_path);
            return false;
        }
        const auto snapshot = copperfin::security::read_physically_contained_file_snapshot(containment, package_root);
        if (!snapshot.ok) {
            error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.ArtifactRejected", record.relative_path);
            return false;
        }
        const auto digest = copperfin::security::sha256_hex_for_text(snapshot.bytes);
        if (!digest.ok || !same_name(digest.hex_digest, record.digest)) {
            error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.DigestMismatch", record.relative_path);
            return false;
        }
        if (same_name(record.relative_path, kInternalAppHostName)) {
            if (saw_internal_target || !same_name(record.role, "runtime_required")) {
                error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryInvalid", record.relative_path);
                return false;
            }
            saw_internal_target = true;
            verified_target_identity = snapshot.containment.identity;
        }
        if (same_name(record.role, "public_apphost")) {
            if (saw_public) {
                error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryIncomplete");
                return false;
            }
            saw_public = true;
            if (!same_name(
                    copperfin::platform::path_to_utf8_string(relative.filename()),
                    copperfin::platform::path_to_utf8_string(guard_path.filename()))) {
                error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.PublicIdentityMismatch");
                return false;
            }
        }
        if (is_required_name(copperfin::platform::path_to_utf8_string(relative.filename()))) {
            if (!same_name(record.role, "runtime_required")) {
                error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryInvalid", record.relative_path);
                return false;
            }
            ++required_count;
        }
    }

    if (!saw_public || !saw_internal_target || required_count != 4U) {
        error = localized_message(catalog, "Runtime.Package.LauncherGuard.Error.InventoryIncomplete");
        return false;
    }
    return true;
}

int run_target(
    const std::filesystem::path& target,
    const copperfin::security::PhysicalPathIdentity& verified_identity,
    int argc,
    wchar_t** argv) {
    const HANDLE target_lock = ::CreateFileW(
        target.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (target_lock == INVALID_HANDLE_VALUE) {
        return kTargetStartFailedExitCode;
    }

    BY_HANDLE_FILE_INFORMATION target_information{};
    const bool target_read = ::GetFileInformationByHandle(target_lock, &target_information) != 0;
    const auto target_identity = copperfin::security::PhysicalPathIdentity{
        .storage_id = target_information.dwVolumeSerialNumber,
        .file_id =
            (static_cast<std::uint64_t>(target_information.nFileIndexHigh) << 32U) |
            target_information.nFileIndexLow,
        .file_size =
            (static_cast<std::uint64_t>(target_information.nFileSizeHigh) << 32U) |
            target_information.nFileSizeLow,
        .modified_ticks =
            (static_cast<std::uint64_t>(target_information.ftLastWriteTime.dwHighDateTime) << 32U) |
            target_information.ftLastWriteTime.dwLowDateTime,
        .link_count = target_information.nNumberOfLinks
    };
    if (!target_read ||
        (target_information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0U ||
        (target_information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U ||
        target_identity != verified_identity) {
        ::CloseHandle(target_lock);
        return kTargetStartFailedExitCode;
    }

    std::wstring command_line = quote_windows_argument(target.wstring());
    for (int index = 1; index < argc; ++index) {
        command_line.push_back(L' ');
        command_line += quote_windows_argument(argv[index]);
    }
    std::vector<wchar_t> mutable_command(command_line.begin(), command_line.end());
    mutable_command.push_back(L'\0');

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};
    const BOOL started = CreateProcessW(
        target.c_str(),
        mutable_command.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        nullptr,
        target.parent_path().c_str(),
        &startup_info,
        &process_info);
    if (!started) {
        ::CloseHandle(target_lock);
        return kTargetStartFailedExitCode;
    }
    WaitForSingleObject(process_info.hProcess, INFINITE);
    DWORD exit_code = kTargetStartFailedExitCode;
    (void)GetExitCodeProcess(process_info.hProcess, &exit_code);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);
    ::CloseHandle(target_lock);
    return static_cast<int>(exit_code);
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
    const std::filesystem::path guard = executable_path();
    if (guard.empty()) {
        return kVerificationFailedExitCode;
    }
    const std::filesystem::path package_root = guard.parent_path();
    const std::filesystem::path debug_manifest = package_root / "app.cfdebug";
    const std::filesystem::path runtime_manifest = package_root / "app.cfmanifest";
    std::string explicit_locale;
    for (int index = 1; index + 1 < argc; ++index) {
        if (std::wstring_view(argv[index]) == L"--locale" ||
            std::wstring_view(argv[index]) == L"/locale") {
            explicit_locale = narrow_utf8_argument(argv[index + 1]);
            break;
        }
    }
    const std::string requested_locale = copperfin::localization::select_locale(explicit_locale);
    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(guard),
        requested_locale);
    const std::filesystem::path verification_manifest =
        regular_file_exists_without_error(debug_manifest) ? debug_manifest : runtime_manifest;
    if (!regular_file_exists_without_error(verification_manifest)) {
        std::cerr << localized_message(catalog, "Runtime.Package.LauncherGuard.Error.ManifestMissing") << "\n";
        return kManifestMissingExitCode;
    }
    std::string error;
    if (!verify_launcher_trust(package_root, verification_manifest, error, catalog)) {
        std::cerr << error << "\n";
        return kVerificationFailedExitCode;
    }
    copperfin::security::PhysicalPathIdentity verified_target_identity{};
    if (!verify_artifacts(
            package_root,
            verification_manifest,
            guard,
            error,
            catalog,
            verified_target_identity)) {
        std::cerr << error << "\n";
        return kVerificationFailedExitCode;
    }
    const std::filesystem::path target = package_root / std::string(kInternalAppHostName);
    return run_target(target, verified_target_identity, argc, argv);
}
