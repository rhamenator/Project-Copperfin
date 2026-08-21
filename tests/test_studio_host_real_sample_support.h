// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "test_environment_support.h"

#include <charconv>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
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

inline std::string normalize_json_line_endings(std::string json) {
    for (std::size_t position = 0;
         (position = json.find("\r\n", position)) != std::string::npos;) {
        json.replace(position, 2U, "\n");
        ++position;
    }
    return json;
}

// A successful DBF mutation, including an undo, stamps bytes 1..3 with the
// local last-update date. Undo fidelity therefore means every other primary
// asset byte returns to its prior value; memo sidecars still rewind exactly.
inline bool dbf_bytes_match_except_last_update_date(
    std::string_view original,
    std::string_view actual) {
    if (original.size() != actual.size() || original.size() < 4U) {
        return false;
    }
    for (std::size_t index = 0U; index < original.size(); ++index) {
        if (index >= 1U && index < 4U) {
            continue;
        }
        if (original[index] != actual[index]) {
            return false;
        }
    }
    return true;
}

inline bool dbf_last_update_date_matches_local_calendar(std::string_view bytes) {
    if (bytes.size() < 4U) {
        return false;
    }

    const std::time_t now = std::time(nullptr);
    std::tm local_time{};
#if defined(_WIN32)
    if (localtime_s(&local_time, &now) != 0) {
        return false;
    }
#else
    if (localtime_r(&now, &local_time) == nullptr) {
        return false;
    }
#endif
    return static_cast<unsigned char>(bytes[1U]) == static_cast<unsigned char>(local_time.tm_year) &&
           static_cast<unsigned char>(bytes[2U]) == static_cast<unsigned char>(local_time.tm_mon + 1) &&
           static_cast<unsigned char>(bytes[3U]) == static_cast<unsigned char>(local_time.tm_mday);
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
