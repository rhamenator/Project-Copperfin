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
// Derives identity from an already-open handle rather than performing a
// fresh path-string lookup -- used where the caller needs the identity of
// the exact file object it is already holding open (issue #5454), as
// opposed to read_file_identity() above, which is for callers that only
// have a path (e.g. the pre-launch revalidation check, which by design
// runs after any handle from authorization has been released).
bool identity_from_handle(HANDLE handle, ExternalProcessFileIdentity& identity) {
    BY_HANDLE_FILE_INFORMATION information{};
    if (!GetFileInformationByHandle(handle, &information) ||
        (information.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
        return false;
    }

    identity.first = information.dwVolumeSerialNumber;
    identity.second = (static_cast<std::uint64_t>(information.nFileIndexHigh) << 32U) |
        information.nFileIndexLow;
    return true;
}

struct ScopedHandle {
    HANDLE handle = INVALID_HANDLE_VALUE;
    ~ScopedHandle() {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};

// Opens the resolved executable for the exclusive duration of
// authorization, sharing read access with other readers (AV scanners,
// the OS image loader, etc.) but denying write and delete sharing --
// this actively prevents another process from replacing, deleting, or
// renaming this specific path for as long as the handle stays open,
// rather than merely detecting such a change after the fact (issue
// #5454). Retries a bounded number of times only on a transient sharing
// violation, since some other legitimate reader could momentarily hold
// an incompatible handle (e.g. a repair/update tool); any other error,
// or exhausting the retries, is treated as a hard failure -- fail
// closed, not fail open.
HANDLE open_executable_exclusive_of_writers(const std::wstring& wide_path) {
    constexpr int max_attempts = 4;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        const HANDLE handle = CreateFileW(
            wide_path.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (handle != INVALID_HANDLE_VALUE) {
            return handle;
        }
        if (GetLastError() != ERROR_SHARING_VIOLATION || attempt + 1 == max_attempts) {
            return INVALID_HANDLE_VALUE;
        }
        Sleep(50);
    }
    return INVALID_HANDLE_VALUE;
}
#endif

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

// Result of a single WinVerifyTrust() call: whether the file has a trusted
// Authenticode signature, and -- if so -- the display name of the
// cryptographically verified signer's leaf certificate. Replaces the old
// two-call design (a separate has_trusted_signature() plus an independent
// get_company_name() reading the PE version-resource CompanyName field) --
// CompanyName is unauthenticated metadata the binary's author can set to
// any string with no cryptographic connection to whatever signature
// actually verified, so matching allowed_publishers against it let a
// binary signed by an untrusted signer (or entirely unsigned, when
// require_trusted_signature is false) claim any publisher name (issue
// #5429).
struct AuthenticodeVerificationResult {
    bool trusted = false;
    std::string signer_display_name;
};

constexpr DWORD kSignerNameBufferChars = 512;

// Walks the trust-provider state WinVerifyTrust() just built to pull out
// the verified signer's leaf certificate display name. Isolated into its
// own function with only POD locals (no destructors to unwind) so it can
// be wrapped in SEH. This guards against an intermittent CI failure
// (issue #5450) narrowed by bisection to this exact call sequence --
// removing it eliminates the flake, keeping it (even with the earlier
// TOCTOU check reverted) reproduces it -- but whose exact internal
// mechanism is unconfirmed, since this repo's dev environment has no
// Windows SDK to attach a debugger or capture a crash dump. The leading
// theory is a rare hardware exception while walking WinTrust's internal
// chain-provider state on a fresh runner (possibly AV/EDR crypto-API
// hooking leaving that state inconsistent), which would otherwise crash
// the whole authorization pipeline and make the calling process vanish
// before it ever spawned the child executable. On any failure this
// returns false and the signer is treated as unknown, which fails
// publisher matching closed rather than silently trusting an unverified
// name.
bool extract_signer_display_name(HANDLE state_data, wchar_t (&buffer)[kSignerNameBufferChars]) {
    __try {
        CRYPT_PROVIDER_DATA* const provider_data = WTHelperProvDataFromStateData(state_data);
        if (provider_data == nullptr) {
            return false;
        }
        CRYPT_PROVIDER_SGNR* const signer =
            WTHelperGetProvSignerFromChain(provider_data, 0, FALSE, 0);
        if (signer == nullptr || signer->csCertChain == 0U || signer->pasCertChain == nullptr) {
            return false;
        }
        const PCCERT_CONTEXT certificate = signer->pasCertChain[0].pCert;
        if (certificate == nullptr) {
            return false;
        }
        const DWORD written = CertGetNameStringW(
            certificate, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, nullptr, buffer, kSignerNameBufferChars);
        return written > 1U;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

// file_handle binds verification to the exact file object the caller
// already holds open (via open_executable_exclusive_of_writers()) rather
// than letting WinVerifyTrust reopen path independently -- path is still
// supplied for subject-type/extension resolution, but hFile overrides it
// for the actual file access (issue #5454).
AuthenticodeVerificationResult verify_authenticode_signature(const std::string& path, HANDLE file_handle) {
    AuthenticodeVerificationResult result;
    const std::wstring wide_path = copperfin::platform::path_from_utf8_string(path).wstring();

    WINTRUST_FILE_INFO file_info{};
    file_info.cbStruct = sizeof(file_info);
    file_info.pcwszFilePath = wide_path.c_str();
    file_info.hFile = file_handle;

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

    if (result.trusted) {
        // Walk the same trust-provider state WinVerifyTrust just built
        // (via hWVTStateData) rather than a second, independent lookup --
        // the signer identity this extracts is the one WinVerifyTrust
        // itself verified, not a separately-resolved one. Retried once
        // after a short backoff in case the first attempt hit a transient
        // failure in extract_signer_display_name() rather than a
        // deterministic one.
        wchar_t signer_name_buffer[kSignerNameBufferChars] = {};
        bool extracted = false;
        for (int attempt = 0; attempt < 2 && !extracted; ++attempt) {
            if (attempt > 0) {
                Sleep(50);
            }
            extracted =
                extract_signer_display_name(trust_data.hWVTStateData, signer_name_buffer);
        }
        if (extracted) {
            result.signer_display_name = narrow(std::wstring(signer_name_buffer));
        }
    }

    trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policy_guid, &trust_data);

    return result;
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

    // Open the executable once, for the exclusive duration of every check
    // below, sharing read access but denying write/delete sharing to
    // other openers. This binds signature verification and identity to a
    // single file object instead of reopening resolved_path by string at
    // multiple points -- a path string can resolve to a *different*
    // underlying file at each open, which let an attacker swap in a
    // trusted binary for verification and swap the original back
    // afterward (an ABA swap defeating the previous pre/post identity
    // compare, which only bound the two identity reads to each other,
    // not to what verification itself actually opened -- issue #5454,
    // originally issue #5430). Holding this handle actively prevents
    // that kind of replacement for as long as it stays open, rather than
    // merely detecting it after the fact.
    const HANDLE verification_handle =
        open_executable_exclusive_of_writers(executable_path.wstring());
    if (verification_handle == INVALID_HANDLE_VALUE) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text(
                    "Security.ExternalProcessPolicy.Error.ResolveExecutableOnPathFailed",
                    {{"executableName", policy.executable_name}}),
                .file_identity = {}};
    }
    const ScopedHandle verification_handle_guard{verification_handle};

#if defined(COPPERFIN_ENABLE_EXTERNAL_PROCESS_POLICY_TEST_HOOKS)
    // Fires with the handle already open and its write/delete-denying
    // share mode already in effect, so a test hook attempting the same
    // rename-and-replace attack this mitigates should observe that
    // attempt fail (a sharing violation), not merely get detected after
    // succeeding.
    if (const auto hook = pre_identity_check_test_hook.exchange(nullptr, std::memory_order_relaxed);
        hook != nullptr) {
        hook();
    }
#endif

    ExternalProcessFileIdentity identity;
    if (!identity_from_handle(verification_handle, identity)) {
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
        ? verify_authenticode_signature(resolved_path, verification_handle)
        : AuthenticodeVerificationResult{};

    if (policy.require_trusted_signature && !verification.trusted) {
        return {.allowed = false,
                .resolved_path = resolved_path,
                .error = security_text("Security.ExternalProcessPolicy.Error.UntrustedAuthenticodeSignature"),
                .file_identity = {}};
    }

    if (!policy.allowed_publishers.empty()) {
        // Denied outright, not just "no match", when verification itself
        // did not succeed (including when require_trusted_signature is
        // false): there is then no verified signer identity to compare
        // allowed_publishers against at all, so treating it as a
        // publisher mismatch rather than a distinct "unverified" case
        // would be accurate either way, but computing a match against an
        // empty signer_display_name is pointless -- short-circuit it.
        const auto match = verification.trusted
            ? std::find_if(
                  policy.allowed_publishers.begin(), policy.allowed_publishers.end(),
                  [&](const std::string& publisher) {
                      return !_stricmp(
                          verification.signer_display_name.c_str(), publisher.c_str());
                  })
            : policy.allowed_publishers.end();

        if (match == policy.allowed_publishers.end()) {
            return {.allowed = false,
                    .resolved_path = resolved_path,
                    .error = security_text("Security.ExternalProcessPolicy.Error.PublisherNotAllowed"),
                    .file_identity = {}};
        }
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
