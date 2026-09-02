// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/security/external_process_policy.h"

#include "copperfin/platform/bounded_wide_string.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "localized_text.h"

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)
#include "copperfin/security/external_process_policy_test_hooks.h"
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <wincrypt.h>
#include <softpub.h>
#include <wintrust.h>
#endif

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <fileapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace copperfin::security {

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)
namespace {
// See external_process_policy_test_hooks.h. Relaxed ordering is
// sufficient: this exists only for single-threaded test setup/teardown
// around a call to authorize_external_process(), never for production
// synchronization.
std::atomic<void (*)()> pre_identity_check_test_hook{nullptr};
}  // namespace

void set_external_process_policy_pre_identity_check_test_hook_for_testing(
    void (*hook)()) {
    pre_identity_check_test_hook.store(hook, std::memory_order_relaxed);
}
#endif

namespace {

bool read_file_identity(
    const std::filesystem::path& path,
    ExternalProcessFileIdentity& identity) {
#ifdef _WIN32
    const std::wstring wide_path = path.wstring();
    const HANDLE handle = CreateFileW(
        wide_path.c_str(),
        FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    BY_HANDLE_FILE_INFORMATION information{};
    const BOOL read = GetFileInformationByHandle(handle, &information);
    CloseHandle(handle);
    if (!read || (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return false;
    }

    identity.first = information.dwVolumeSerialNumber;
    identity.second = (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
        information.nFileIndexLow;
    return true;
#else
    struct stat information{};
    if (::stat(path.c_str(), &information) != 0 || !S_ISREG(information.st_mode) ||
        ::access(path.c_str(), X_OK) != 0) {
        return false;
    }

    identity.first = static_cast<std::uint64_t>(information.st_dev);
    identity.second = static_cast<std::uint64_t>(information.st_ino);
    return true;
#endif
}

bool file_identities_equal(
    const ExternalProcessFileIdentity& left,
    const ExternalProcessFileIdentity& right) {
    return left.first == right.first && left.second == right.second;
}

#ifdef _WIN32
std::wstring widen(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int count = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) {
        return {};
    }

    std::wstring result(static_cast<std::size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(), count);
    return result;
}

std::string narrow(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }

    const int count = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0) {
        return {};
    }

    std::string result(static_cast<std::size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(), count, nullptr, nullptr);
    return result;
}

std::string resolve_executable_from_path(const std::string& executable_name) {
    std::wstring executable = widen(executable_name);
    if (executable.empty()) {
        return {};
    }

    DWORD capacity = MAX_PATH;
    for (;;) {
        std::vector<wchar_t> buffer(capacity, L'\0');
        const DWORD result = SearchPathW(
            nullptr,
            executable.c_str(),
            nullptr,
            capacity,
            buffer.data(),
            nullptr);
        if (result == 0) {
            return {};
        }
        if (result < capacity) {
            std::error_code error;
            const std::filesystem::path canonical =
                std::filesystem::weakly_canonical(std::filesystem::path(buffer.data()), error);
            if (error) {
                return {};
            }

            return copperfin::platform::path_to_utf8_string(canonical);
        }
        if (result == std::numeric_limits<DWORD>::max()) {
            return {};
        }
        capacity = result + 1U;
    }
}

// EXPERIMENT (diagnostic only, not for merge): the WTHelperProvDataFromStateData
// / WTHelperGetProvSignerFromChain / CertGetNameStringW signer-extraction
// block (issue #5429) is temporarily removed here, isolating it as a
// variable separate from the TOCTOU pre/post identity check (issue #5430,
// kept below) -- a prior experiment already showed removing just the
// pre-verification identity read does NOT fix
// test_generated_launcher_process's persistent Windows CI flake, while a
// full revert of this file DOES fix it, so this experiment narrows down
// which remaining piece is responsible. publisher matching falls back to
// the old get_company_name() (PE version-resource CompanyName, restored
// below) instead of a verified signer identity for this experiment only.
struct AuthenticodeVerificationResult {
    bool trusted = false;
    std::string signer_display_name;
};

AuthenticodeVerificationResult verify_authenticode_signature(const std::string& path) {
    AuthenticodeVerificationResult result;
    const std::wstring wide_path = copperfin::platform::path_from_utf8_string(path).wstring();

    WINTRUST_FILE_INFO file_info{};
    file_info.cbStruct = sizeof(file_info);
    file_info.pcwszFilePath = wide_path.c_str();

    WINTRUST_DATA trust_data{};
    trust_data.cbStruct = sizeof(trust_data);
    trust_data.dwUIChoice = WTD_UI_NONE;
    trust_data.fdwRevocationChecks = WTD_REVOKE_WHOLECHAIN;
    trust_data.dwUnionChoice = WTD_CHOICE_FILE;
    trust_data.dwStateAction = WTD_STATEACTION_VERIFY;
    trust_data.pFile = &file_info;
    trust_data.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

    GUID policy_guid = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    const LONG status = WinVerifyTrust(nullptr, &policy_guid, &trust_data);
    result.trusted = status == ERROR_SUCCESS;

    trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policy_guid, &trust_data);

    return result;
}

std::string get_company_name(const std::string& path) {
    const std::wstring wide_path = copperfin::platform::path_from_utf8_string(path).wstring();
    DWORD handle = 0;
    const DWORD size = GetFileVersionInfoSizeW(wide_path.c_str(), &handle);
    if (size == 0) {
        return {};
    }

    std::vector<std::uint8_t> version_block(size);
    if (!GetFileVersionInfoW(wide_path.c_str(), 0, size, version_block.data())) {
        return {};
    }

    struct LangCodePage {
        WORD language;
        WORD code_page;
    };

    LangCodePage* translation = nullptr;
    UINT translation_size = 0;
    if (!VerQueryValueW(version_block.data(), L"\\VarFileInfo\\Translation", reinterpret_cast<LPVOID*>(&translation), &translation_size)
        || translation == nullptr
        || translation_size < sizeof(LangCodePage)) {
        return {};
    }

    wchar_t query[64]{};
    swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\CompanyName", translation[0].language, translation[0].code_page);

    LPVOID value = nullptr;
    UINT value_size = 0;
    if (!VerQueryValueW(version_block.data(), query, &value, &value_size) || value == nullptr || value_size == 0) {
        return {};
    }

    return narrow(copperfin::platform::bounded_wide_string(
        static_cast<const wchar_t*>(value), value_size));
}

bool path_under_root(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::error_code path_error;
    const auto canonical_path = std::filesystem::weakly_canonical(path, path_error);
    if (path_error) {
        return false;
    }

    std::error_code root_error;
    const auto canonical_root = std::filesystem::weakly_canonical(root, root_error);
    if (root_error) {
        return false;
    }

    auto path_component = canonical_path.begin();
    auto root_component = canonical_root.begin();
    for (; root_component != canonical_root.end(); ++root_component, ++path_component) {
        if (path_component == canonical_path.end() ||
            !copperfin::platform::path_component_equal_for_platform(
                *path_component,
                *root_component)) {
            return false;
        }
    }
    return true;
}
#else
std::string resolve_executable_from_path(const std::string& executable_name) {
    if (executable_name.empty()) {
        return {};
    }

    const std::filesystem::path requested =
        copperfin::platform::path_from_utf8_string(executable_name);
    std::vector<std::filesystem::path> candidates;
    if (requested.has_parent_path()) {
        candidates.push_back(requested);
    } else {
        const auto configured_path = copperfin::platform::read_environment_variable("PATH");
        const std::string path_value = configured_path.has_value()
            ? *configured_path
            : copperfin::platform::default_executable_search_path().value_or(std::string{});
        std::size_t start = 0U;
        for (;;) {
            const std::size_t separator = path_value.find(':', start);
            const std::string directory = path_value.substr(
                start,
                separator == std::string::npos ? std::string::npos : separator - start);
            candidates.push_back(
                (directory.empty()
                     ? std::filesystem::path(".")
                     : copperfin::platform::path_from_utf8_string(directory)) /
                requested);
            if (separator == std::string::npos) {
                break;
            }
            start = separator + 1U;
        }
    }

    for (const auto& candidate : candidates) {
        std::error_code status_error;
        if (!std::filesystem::is_regular_file(candidate, status_error) ||
            status_error ||
            ::access(candidate.c_str(), X_OK) != 0) {
            continue;
        }
        std::error_code canonical_error;
        const std::filesystem::path canonical =
            std::filesystem::weakly_canonical(candidate, canonical_error);
        if (canonical_error) {
            continue;
        }
        return copperfin::platform::path_to_utf8_string(canonical);
    }
    return {};
}

bool path_under_root(const std::filesystem::path& path, const std::filesystem::path& root) {
    if (path.empty() || root.empty()) {
        return false;
    }

    std::error_code path_error;
    const auto canonical_path = std::filesystem::weakly_canonical(path, path_error);
    if (path_error) {
        return false;
    }

    std::error_code root_error;
    const auto canonical_root = std::filesystem::weakly_canonical(root, root_error);
    if (root_error) {
        return false;
    }

    auto path_component = canonical_path.begin();
    std::filesystem::path candidate_root;
    for (auto root_component = canonical_root.begin();
         root_component != canonical_root.end();
         ++root_component, ++path_component) {
        if (path_component == canonical_path.end()) {
            return false;
        }
        candidate_root /= *path_component;
    }

    std::error_code equivalent_error;
    return std::filesystem::equivalent(
               candidate_root,
               canonical_root,
               equivalent_error) &&
        !equivalent_error;
}
#endif

}  // namespace

ExternalProcessAuthorizationResult authorize_external_process(const ExternalProcessPolicy& policy) {
#ifdef _WIN32
    const std::string resolved_path = resolve_executable_from_path(policy.executable_name);
    if (resolved_path.empty()) {
        return {.allowed = false,
                .resolved_path = {},
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }

    const std::filesystem::path executable_path =
        copperfin::platform::path_from_utf8_string(resolved_path);
    if (policy.allowed_path_roots.empty()) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.EmptyAllowedPathRoots"),
                .file_identity = {}};
    }
    bool root_match = false;
    for (const auto& root : policy.allowed_path_roots) {
        if (path_under_root(
                executable_path,
                copperfin::platform::path_from_utf8_string(root))) {
            root_match = true;
            break;
        }
    }
    if (!root_match) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.PathOutsideAllowedRoots"),
                .file_identity = {}};
    }

    // Capture the file identity here, before signature/publisher
    // verification, so a rename/replace of the executable at any point
    // between this and the post-verification identity read below is
    // detectable. Nothing otherwise binds verify_authenticode_signature()'s
    // path-string open, and the final read_file_identity() call's own
    // path-string open, to the same underlying file object -- an attacker
    // who can replace the file at resolved_path in that window could pass
    // verification against one binary while a different one is what this
    // authorization actually ends up describing (issue #5430).
    ExternalProcessFileIdentity pre_verification_identity;
    if (!read_file_identity(executable_path, pre_verification_identity)) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }

    // A single WinVerifyTrust() call serves both the require_trusted_signature
    // check and, when allowed_publishers is non-empty, the publisher match --
    // the latter needs a verified signer identity to match against
    // regardless of require_trusted_signature's own value, since there is
    // no other authenticated source of publisher identity (issue #5429).
    const bool needs_signature_verification =
        policy.require_trusted_signature || !policy.allowed_publishers.empty();
    const AuthenticodeVerificationResult verification = needs_signature_verification
        ? verify_authenticode_signature(resolved_path)
        : AuthenticodeVerificationResult{};

    if (policy.require_trusted_signature && !verification.trusted) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.UntrustedAuthenticodeSignature"),
                .file_identity = {}};
    }

    if (!policy.allowed_publishers.empty()) {
        // EXPERIMENT: publisher matching temporarily uses the old
        // get_company_name() (unauthenticated PE version-resource
        // CompanyName) instead of a verified signer identity, matching
        // this file's verify_authenticode_signature() experiment above.
        const std::string company_name = get_company_name(resolved_path);
        const auto match = std::find_if(
            policy.allowed_publishers.begin(), policy.allowed_publishers.end(),
            [&](const std::string& publisher) {
                return !_stricmp(company_name.c_str(), publisher.c_str());
            });

        if (match == policy.allowed_publishers.end()) {
            return {.allowed = false,
                    .resolved_path = resolved_path,
                    .error = security_text("Security.ExternalProcessPolicy.Error.PublisherNotAllowed"),
                    .file_identity = {}};
        }
    }

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)
    if (const auto hook = pre_identity_check_test_hook.exchange(nullptr, std::memory_order_relaxed);
        hook != nullptr) {
        hook();
    }
#endif

    ExternalProcessFileIdentity identity;
    if (!read_file_identity(executable_path, identity)) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }
    if (!file_identities_equal(identity, pre_verification_identity)) {
        // The object the checks above verified is not the object this
        // authorization would actually describe going forward: reject
        // rather than silently trust a binary swapped in during
        // verification (issue #5430).
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ExecutableChangedDuringAuthorization"),
                .file_identity = {}};
    }
    return {.allowed = true,
            .resolved_path = resolved_path,
            .error = {},
            .file_identity = identity};
#else
    if (policy.require_trusted_signature) {
        return {.allowed = false,
                .resolved_path = {},
                .error = security_text("Security.ExternalProcessPolicy.Error.WindowsOnly"),
                .file_identity = {}};
    }
    const std::string resolved_path = resolve_executable_from_path(policy.executable_name);
    if (resolved_path.empty()) {
        return {.allowed = false,
                .resolved_path = {},
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }

    const std::filesystem::path executable_path =
        copperfin::platform::path_from_utf8_string(resolved_path);
    if (policy.allowed_path_roots.empty()) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.EmptyAllowedPathRoots"),
                .file_identity = {}};
    }
    const bool root_match = std::any_of(
        policy.allowed_path_roots.begin(),
        policy.allowed_path_roots.end(),
        [&](const std::string& root) {
            return path_under_root(
                executable_path,
                copperfin::platform::path_from_utf8_string(root));
        });
    if (!root_match) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.PathOutsideAllowedRoots"),
                .file_identity = {}};
    }
    ExternalProcessFileIdentity identity;
    if (!read_file_identity(executable_path, identity)) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }
    return {.allowed = true,
            .resolved_path = resolved_path,
            .error = {},
            .file_identity = identity};
#endif
}

bool revalidate_external_process_authorization(
    ExternalProcessAuthorizationResult& authorization) {
    if (!authorization.allowed || authorization.resolved_path.empty()) {
        return false;
    }

    ExternalProcessFileIdentity current_identity;
    if (!read_file_identity(
            copperfin::platform::path_from_utf8_string(authorization.resolved_path),
            current_identity) ||
        !file_identities_equal(current_identity, authorization.file_identity)) {
        authorization.allowed = false;
        authorization.error = security_text(
            "Security.ExternalProcessPolicy.Error.ExecutableChangedAfterAuthorization");
        return false;
    }
    return true;
}

}  // namespace copperfin::security
