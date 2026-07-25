// Copyright (c) 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "test_environment_support.h"

#include <charconv>
#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace copperfin::test_support {

inline bool extract_json_integer(
    const std::string& json_text,
    std::string_view key,
    int& value) {
    const std::size_t key_position = json_text.find(key);
    if (key_position == std::string::npos) {
        return false;
    }

    const std::size_t colon_position = json_text.find(':', key_position + key.size());
    if (colon_position == std::string::npos) {
        return false;
    }

    const char* begin = json_text.data() + colon_position + 1U;
    const char* end = json_text.data() + json_text.size();
    while (begin != end && std::isspace(static_cast<unsigned char>(*begin)) != 0) {
        ++begin;
    }

    const auto parsed = std::from_chars(begin, end, value);
    return parsed.ec == std::errc{};
}

inline bool json_integer_delta(
    const std::string& updated_json,
    const std::string& restored_json,
    std::string_view key,
    int expected_delta) {
    int updated_value = 0;
    int restored_value = 0;
    return extract_json_integer(updated_json, key, updated_value) &&
           extract_json_integer(restored_json, key, restored_value) &&
           updated_value - restored_value == expected_delta;
}

inline std::filesystem::path find_vfp9_reports_root() {
    namespace fs = std::filesystem;

    const auto contains_report_samples = [](const fs::path& candidate) {
        const auto contains_regular_file = [](const fs::path& path) {
            std::error_code error;
            return fs::is_regular_file(path, error) && !error;
        };

        return contains_regular_file(candidate / "invoice.frx") &&
               contains_regular_file(candidate / "invoice.frt") &&
               contains_regular_file(candidate / "cust.lbx") &&
               contains_regular_file(candidate / "cust.lbt");
    };

    if (const std::string override_root = getenv_value("COPPERFIN_VFP9_REPORTS_ROOT");
        !override_root.empty()) {
        const fs::path candidate = fs::path(override_root);
        if (contains_report_samples(candidate)) {
            return candidate;
        }
        return {};
    }

    const fs::path windows_candidate =
        R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports)";
    if (contains_report_samples(windows_candidate)) {
        return windows_candidate;
    }

    const std::vector<fs::path> media_roots{
        "/run/media",
        "/media"
    };
    for (const auto& media_root : media_roots) {
        std::error_code error;
        if (!fs::exists(media_root, error) || error) {
            continue;
        }
        fs::directory_iterator entries(media_root, error);
        if (error) {
            continue;
        }
        while (entries != fs::directory_iterator()) {
            const fs::path candidate =
                entries->path() / "VFPPROD1" / "program files" / "microsoft visual foxpro 9" /
                "samples" / "solution" / "reports";
            if (contains_report_samples(candidate)) {
                return candidate;
            }

            error.clear();
            entries.increment(error);
            if (error) {
                break;
            }
        }
    }

    return {};
}

}  // namespace copperfin::test_support
