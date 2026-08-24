// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/localization/localization.h"
#include "copperfin/vfp/asset_inspector.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <locale>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::tests::vfp_assets_support {

inline int failures = 0;

class grouped_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class global_locale_guard final {
public:
    explicit global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~global_locale_guard() { std::locale::global(previous_); }

    global_locale_guard(const global_locale_guard&) = delete;
    global_locale_guard& operator=(const global_locale_guard&) = delete;

private:
    std::locale previous_;
};

inline void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

inline std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
}

inline bool has_validation_issue(
    const copperfin::vfp::AssetInspectionResult& result,
    const std::string& code,
    const std::string& path_suffix = {}) {
    return std::any_of(
        result.validation_issues.begin(),
        result.validation_issues.end(),
        [&](const copperfin::vfp::AssetValidationIssue& issue) {
            if (issue.code != code) {
                return false;
            }
            return path_suffix.empty() ||
                   (issue.path.size() >= path_suffix.size() && issue.path.ends_with(path_suffix));
        });
}

inline std::size_t count_validation_issues(
    const copperfin::vfp::AssetInspectionResult& result,
    const std::string& code) {
    return static_cast<std::size_t>(std::count_if(
        result.validation_issues.begin(),
        result.validation_issues.end(),
        [&](const copperfin::vfp::AssetValidationIssue& issue) { return issue.code == code; }));
}

inline const copperfin::vfp::AssetValidationIssue* find_validation_issue(
    const copperfin::vfp::AssetInspectionResult& result,
    const std::string& code,
    const std::string& path_suffix = {}) {
    const auto found = std::find_if(
        result.validation_issues.begin(),
        result.validation_issues.end(),
        [&](const copperfin::vfp::AssetValidationIssue& issue) {
            if (issue.code != code) {
                return false;
            }
            return path_suffix.empty() ||
                   (issue.path.size() >= path_suffix.size() && issue.path.ends_with(path_suffix));
        });
    return found == result.validation_issues.end() ? nullptr : &(*found);
}

inline std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[4] = 10U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x05U;
    bytes[29] = 0x03U;
    return bytes;
}

inline void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

inline void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

inline void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

inline void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

inline void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(value[index]);
    }
}

inline void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t length) {
    for (std::size_t index = 0; index < 11U && index < name.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(name[index]);
    }
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = length;
}

}  // namespace copperfin::tests::vfp_assets_support
