// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/file_version.h"

#include "copperfin/platform/bounded_wide_string.h"
#include "copperfin/platform/path.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <set>
#include <string_view>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winver.h>
#endif

namespace copperfin::platform {
namespace {

FileVersionMetadata default_metadata(const std::filesystem::path& path) {
    FileVersionMetadata metadata;
    metadata.file_description = path_to_utf8_string(path.filename());
    return metadata;
}

#if defined(_WIN32)

std::string utf8_from_wide(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }
    const int count = ::WideCharToMultiByte(
        CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
        nullptr, 0, nullptr, nullptr);
    if (count <= 0) {
        return {};
    }
    std::string result(static_cast<std::size_t>(count), '\0');
    if (::WideCharToMultiByte(
            CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()),
            result.data(), count, nullptr, nullptr) != count) {
        return {};
    }
    return result;
}

#else

bool is_control_code_unit(const std::uint16_t code_unit) {
    return code_unit < 0x20U && code_unit != 0x09U &&
        code_unit != 0x0AU && code_unit != 0x0DU;
}

std::string utf8_from_utf16_code_units(
    const std::vector<std::uint16_t>& code_units) {
    std::string utf8;
    utf8.reserve(code_units.size());
    const auto append_code_point = [&utf8](const char32_t code_point) {
        if (code_point <= 0x7FU) {
            utf8.push_back(static_cast<char>(code_point));
        } else if (code_point <= 0x7FFU) {
            utf8.push_back(static_cast<char>(0xC0U | ((code_point >> 6U) & 0x1FU)));
            utf8.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        } else if (code_point <= 0xFFFFU) {
            utf8.push_back(static_cast<char>(0xE0U | ((code_point >> 12U) & 0x0FU)));
            utf8.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
            utf8.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        } else {
            utf8.push_back(static_cast<char>(0xF0U | ((code_point >> 18U) & 0x07U)));
            utf8.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
            utf8.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
            utf8.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
        }
    };

    for (std::size_t index = 0U; index < code_units.size(); ++index) {
        const std::uint16_t unit = code_units[index];
        if (unit >= 0xD800U && unit <= 0xDBFFU && index + 1U < code_units.size()) {
            const std::uint16_t low = code_units[index + 1U];
            if (low >= 0xDC00U && low <= 0xDFFFU) {
                append_code_point(
                    0x10000U +
                    ((static_cast<char32_t>(unit - 0xD800U) << 10U) |
                     static_cast<char32_t>(low - 0xDC00U)));
                ++index;
                continue;
            }
        }
        append_code_point(static_cast<char32_t>(unit));
    }
    return utf8;
}

std::vector<std::string> scan_utf16le_strings(
    const std::vector<std::uint8_t>& bytes) {
    std::vector<std::string> strings;
    for (std::size_t offset = 0U; offset + 1U < bytes.size();) {
        const std::uint16_t candidate = static_cast<std::uint16_t>(bytes[offset]) |
            (static_cast<std::uint16_t>(bytes[offset + 1U]) << 8U);
        if (candidate == 0U || is_control_code_unit(candidate)) {
            offset += 2U;
            continue;
        }

        std::vector<std::uint16_t> code_units;
        std::size_t cursor = offset;
        while (cursor + 1U < bytes.size()) {
            const std::uint16_t code_unit = static_cast<std::uint16_t>(bytes[cursor]) |
                (static_cast<std::uint16_t>(bytes[cursor + 1U]) << 8U);
            if (code_unit == 0U) {
                break;
            }
            if (is_control_code_unit(code_unit)) {
                code_units.clear();
                break;
            }
            code_units.push_back(code_unit);
            cursor += 2U;
        }

        if (code_units.size() >= 4U) {
            strings.push_back(utf8_from_utf16_code_units(code_units));
            offset = cursor + 2U;
        } else {
            offset += 2U;
        }
    }
    return strings;
}

std::string trim_ascii_copy(std::string value) {
    const auto is_space = [](const unsigned char character) {
        return std::isspace(character) != 0;
    };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), is_space));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), is_space).base(), value.end());
    return value;
}

bool is_structural_string(const std::string& value) {
    static const std::set<std::string, std::less<>> structural_strings = {
        "VS_VERSION_INFO", "StringFileInfo", "VarFileInfo", "Translation",
        "OleSelfRegister", "Comments", "CompanyName", "FileDescription",
        "FileVersion", "InternalName", "LegalCopyright", "LegalTrademarks",
        "OriginalFilename", "ProductName", "ProductVersion", "PrivateBuild",
        "SpecialBuild", "Copyright",
    };
    if (value.empty() || structural_strings.contains(value)) {
        return true;
    }
    return value.size() == 8U &&
        std::all_of(value.begin(), value.end(), [](const unsigned char character) {
            return std::isxdigit(character) != 0;
        });
}

std::string lookup_string(
    const std::vector<std::string>& strings,
    const std::string_view key) {
    for (std::size_t index = 0U; index < strings.size(); ++index) {
        if (strings[index] != key) {
            continue;
        }
        for (std::size_t value_index = index + 1U;
             value_index < strings.size(); ++value_index) {
            const std::string candidate = trim_ascii_copy(strings[value_index]);
            if (candidate.empty() || candidate == "Copyright") {
                continue;
            }
            if (is_structural_string(candidate)) {
                break;
            }
            return candidate;
        }
    }
    return {};
}

#endif

}  // namespace

FileVersionMetadata read_file_version_metadata(const std::filesystem::path& path) {
    FileVersionMetadata metadata = default_metadata(path);
#if defined(_WIN32)
    const std::wstring wide_path = path.native();
    if (wide_path.empty()) {
        return metadata;
    }

    DWORD ignored_handle = 0U;
    const DWORD size = ::GetFileVersionInfoSizeW(wide_path.c_str(), &ignored_handle);
    if (size == 0U) {
        return metadata;
    }
    std::vector<std::uint8_t> version_block(size);
    if (::GetFileVersionInfoW(wide_path.c_str(), 0U, size, version_block.data()) == FALSE) {
        return metadata;
    }

    struct LangCodePage {
        WORD language;
        WORD code_page;
    };
    LangCodePage* translation = nullptr;
    UINT translation_size = 0U;
    if (::VerQueryValueW(
            version_block.data(), L"\\VarFileInfo\\Translation",
            reinterpret_cast<LPVOID*>(&translation), &translation_size) == FALSE ||
        translation == nullptr || translation_size < sizeof(LangCodePage)) {
        return metadata;
    }

    const auto query_string = [&](const std::string_view key) -> std::string {
        const std::wstring wide_key(key.begin(), key.end());
        wchar_t query[64]{};
        if (swprintf_s(
                query, L"\\StringFileInfo\\%04x%04x\\%ls",
                translation[0].language, translation[0].code_page,
                wide_key.c_str()) < 0) {
            return {};
        }
        LPVOID value = nullptr;
        UINT value_size = 0U;
        if (::VerQueryValueW(
                version_block.data(), query, &value, &value_size) == FALSE ||
            value == nullptr || value_size == 0U) {
            return {};
        }
        return utf8_from_wide(copperfin::platform::bounded_wide_string(
            static_cast<const wchar_t*>(value), value_size));
    };
#else
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return metadata;
    }
    input.seekg(0, std::ios::end);
    const std::streamoff size = input.tellg();
    if (size <= 0) {
        return metadata;
    }
    input.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(size));
    if (!input.read(
            reinterpret_cast<char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()))) {
        return metadata;
    }
    const std::vector<std::string> strings = scan_utf16le_strings(bytes);
    if (strings.empty()) {
        return metadata;
    }
    const auto query_string = [&](const std::string_view key) {
        return lookup_string(strings, key);
    };
#endif

    if (const std::string value = query_string("FileVersion"); !value.empty()) {
        metadata.full_version = value;
        metadata.file_version = value;
    }
    if (const std::string value = query_string("FileDescription"); !value.empty()) {
        metadata.file_description = value;
    }
    if (const std::string value = query_string("CompanyName"); !value.empty()) {
        metadata.company_name = value;
    }
    if (const std::string value = query_string("ProductName"); !value.empty()) {
        metadata.product_name = value;
    }
    if (const std::string value = query_string("ProductVersion"); !value.empty()) {
        metadata.product_version = value;
    }
    const std::string legal_trademarks = query_string("LegalTrademarks");
    const std::string legal_copyright = query_string("LegalCopyright");
    metadata.trademark_or_copyright =
        !legal_trademarks.empty() ? legal_trademarks : legal_copyright;
    return metadata;
}

}  // namespace copperfin::platform
