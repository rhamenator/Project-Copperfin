#include "copperfin/security/external_process_policy.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <softpub.h>
#include <wintrust.h>
#endif

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace copperfin::security {

namespace {

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

    wchar_t buffer[MAX_PATH]{};
    const DWORD result = SearchPathW(nullptr, executable.c_str(), nullptr, MAX_PATH, buffer, nullptr);
    if (result == 0 || result >= MAX_PATH) {
        return {};
    }

    std::error_code error;
    const std::filesystem::path canonical = std::filesystem::weakly_canonical(std::filesystem::path(buffer), error);
    if (error) {
        return {};
    }

    return canonical.string();
}

bool has_trusted_signature(const std::string& path) {
    const std::wstring wide_path = std::filesystem::path(path).wstring();

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

    trust_data.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(nullptr, &policy_guid, &trust_data);

    return status == ERROR_SUCCESS;
}

std::string get_company_name(const std::string& path) {
    const std::wstring wide_path = std::filesystem::path(path).wstring();
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

    return narrow(std::wstring(static_cast<wchar_t*>(value)));
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

    const auto path_string = canonical_path.generic_string();
    std::string root_string = canonical_root.generic_string();
    if (!root_string.empty() && root_string.back() != '/') {
        root_string.push_back('/');
    }

    return path_string == canonical_root.generic_string() || path_string.rfind(root_string, 0) == 0;
}
#endif

}  // namespace

ExternalProcessAuthorizationResult authorize_external_process(const ExternalProcessPolicy& policy) {
#ifdef _WIN32
    const std::string resolved_path = resolve_executable_from_path(policy.executable_name);
    if (resolved_path.empty()) {
        return {.allowed = false, .error = "Unable to resolve executable on PATH: " + policy.executable_name};
    }

    const std::filesystem::path executable_path(resolved_path);
    bool root_match = policy.allowed_path_roots.empty();
    for (const auto& root : policy.allowed_path_roots) {
        if (path_under_root(executable_path, std::filesystem::path(root))) {
            root_match = true;
            break;
        }
    }
    if (!root_match) {
        return {.allowed = false, .resolved_path = resolved_path, .error = "Executable path is outside allowed roots."};
    }

    if (policy.require_trusted_signature && !has_trusted_signature(resolved_path)) {
        return {.allowed = false, .resolved_path = resolved_path, .error = "Executable does not have a trusted Authenticode signature."};
    }

    if (!policy.allowed_publishers.empty()) {
        const std::string company_name = get_company_name(resolved_path);
        const auto match = std::find_if(policy.allowed_publishers.begin(), policy.allowed_publishers.end(), [&](const std::string& publisher) {
            return !_stricmp(company_name.c_str(), publisher.c_str());
        });

        if (match == policy.allowed_publishers.end()) {
            return {.allowed = false, .resolved_path = resolved_path, .error = "Executable publisher is not in the allow-list."};
        }
    }

    return {.allowed = true, .resolved_path = resolved_path};
#else
    (void)policy;
    return {.allowed = false, .error = "External process policy authorization is implemented for Windows only."};
#endif
}

}  // namespace copperfin::security
