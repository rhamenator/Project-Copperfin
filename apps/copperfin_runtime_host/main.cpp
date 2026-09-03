// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/licensing/license_status.h"
#include "copperfin/licensing/license_status_display.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"
#include "copperfin/platform/executable_path.h"
#include "copperfin/platform/path.h"
#include "copperfin/platform/sqlite_federation_connector.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/platform/federation_execution.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/physical_path_containment.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/sha256.h"
#include "copperfin/studio/document_model.h"
#include "copperfin/vfp/sidecar_path.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <optional>
#include <random>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

unsigned long long current_process_id() {
#if defined(_WIN32)
    return static_cast<unsigned long long>(::_getpid());
#else
    return static_cast<unsigned long long>(::getpid());
#endif
}

std::string trim_copy(std::string value) {
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) == 0;
    }));
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.pop_back();
    }
    return value;
}

std::string lowercase_copy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool starts_with_insensitive(const std::string& value, const std::string& prefix) {
    if (value.size() < prefix.size()) {
        return false;
    }
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (std::tolower(static_cast<unsigned char>(value[index])) !=
            std::tolower(static_cast<unsigned char>(prefix[index]))) {
            return false;
        }
    }
    return true;
}

bool equals_insensitive(const std::string& value, const std::string& expected) {
    return value.size() == expected.size() && starts_with_insensitive(value, expected);
}

std::filesystem::path path_from_utf8(const std::string& value) {
    return copperfin::platform::path_from_utf8_string(value);
}

std::string packaged_runtime_host_file_name() {
#if defined(_WIN32)
    return "copperfin_runtime_host.exe";
#else
    return "copperfin_runtime_host";
#endif
}

const copperfin::runtime::XAssetActionBinding* find_pause_xasset_action(
    const copperfin::runtime::RuntimePauseState& state,
    const copperfin::runtime::XAssetExecutableModel& model) {
    if (model.actions.empty()) {
        return nullptr;
    }

    for (const auto& frame : state.call_stack) {
        const std::string normalized_routine_name = lowercase_copy(trim_copy(frame.routine_name));
        if (normalized_routine_name.empty()) {
            continue;
        }

        const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
            return lowercase_copy(action.routine_name) == normalized_routine_name;
        });
        if (found != model.actions.end()) {
            return &(*found);
        }
    }

    return nullptr;
}

bool parse_manifest_bool_token(const std::string& value, bool& parsed) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "true") {
        parsed = true;
        return true;
    }
    if (normalized == "false") {
        parsed = false;
        return true;
    }
    return false;
}

bool parse_cli_bool_token(const std::string& value, bool& parsed) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    if (normalized == "true" || normalized == ".t." || normalized == "t" || normalized == "1" ||
        normalized == "yes" || normalized == "on") {
        parsed = true;
        return true;
    }
    if (normalized == "false" || normalized == ".f." || normalized == "f" || normalized == "0" ||
        normalized == "no" || normalized == "off") {
        parsed = false;
        return true;
    }
    return false;
}

std::string escape_json_string(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result.push_back(ch);
                break;
        }
    }
    return result;
}

std::string escape_debug_line_value(const std::string& value) {
    std::string result;
    result.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result.push_back(ch);
                break;
        }
    }
    return result;
}

std::optional<std::string> parse_json_string_at(
    const std::string& document,
    std::size_t value_start,
    std::size_t& value_end);

std::optional<std::uint16_t> parse_json_hex_quad(
    const std::string& document,
    std::size_t offset) {
    if (offset + 4U > document.size()) {
        return std::nullopt;
    }
    std::uint16_t value = 0U;
    for (std::size_t index = offset; index < offset + 4U; ++index) {
        const unsigned char ch = static_cast<unsigned char>(document[index]);
        std::uint16_t digit = 0U;
        if (ch >= '0' && ch <= '9') {
            digit = static_cast<std::uint16_t>(ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            digit = static_cast<std::uint16_t>(ch - 'a' + 10U);
        } else if (ch >= 'A' && ch <= 'F') {
            digit = static_cast<std::uint16_t>(ch - 'A' + 10U);
        } else {
            return std::nullopt;
        }
        value = static_cast<std::uint16_t>((value << 4U) | digit);
    }
    return value;
}

bool append_json_code_point(std::string& result, std::uint32_t code_point) {
    if (code_point > 0x10FFFFU || (code_point >= 0xD800U && code_point <= 0xDFFFU)) {
        return false;
    }
    if (code_point <= 0x7FU) {
        result.push_back(static_cast<char>(code_point));
    } else if (code_point <= 0x7FFU) {
        result.push_back(static_cast<char>(0xC0U | (code_point >> 6U)));
        result.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else if (code_point <= 0xFFFFU) {
        result.push_back(static_cast<char>(0xE0U | (code_point >> 12U)));
        result.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        result.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    } else {
        result.push_back(static_cast<char>(0xF0U | (code_point >> 18U)));
        result.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3FU)));
        result.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3FU)));
        result.push_back(static_cast<char>(0x80U | (code_point & 0x3FU)));
    }
    return true;
}

bool find_json_field_value_start(
    const std::string& document,
    const std::string& field_name,
    std::size_t& value_start) {
    const auto field_token = std::string("\"") + field_name + "\"";
    value_start = std::string::npos;
    std::size_t object_depth = 0U;
    std::size_t array_depth = 0U;
    for (std::size_t index = 0U; index < document.size(); ++index) {
        const char ch = document[index];
        if (ch == '"') {
            std::size_t string_end = index + 1U;
            bool escaping = false;
            for (; string_end < document.size(); ++string_end) {
                const char string_ch = document[string_end];
                if (escaping) {
                    escaping = false;
                    continue;
                }
                if (string_ch == '\\') {
                    escaping = true;
                    continue;
                }
                if (string_ch == '"') {
                    break;
                }
            }
            if (string_end >= document.size()) {
                return false;
            }
            if (object_depth == 1U && array_depth == 0U &&
                string_end + 1U == index + field_token.size() &&
                document.compare(index, field_token.size(), field_token) == 0) {
                const auto colon_offset = document.find_first_not_of(" \t\r\n", string_end + 1U);
                if (colon_offset != std::string::npos && document[colon_offset] == ':') {
                    value_start = document.find_first_not_of(" \t\r\n", colon_offset + 1U);
                    return value_start != std::string::npos;
                }
            }
            index = string_end;
            continue;
        }
        if (ch == '{') {
            ++object_depth;
            continue;
        }
        if (ch == '}' && object_depth > 0U) {
            --object_depth;
            continue;
        }
        if (ch == '[') {
            ++array_depth;
            continue;
        }
        if (ch == ']' && array_depth > 0U) {
            --array_depth;
        }
    }
    return false;
}

std::string extract_json_field(const std::string& document, const std::string& field_name) {
    std::size_t value_start = std::string::npos;
    if (!find_json_field_value_start(document, field_name, value_start)) {
        return {};
    }
    if (document[value_start] == '"') {
        std::size_t value_end = value_start;
        const auto parsed = parse_json_string_at(document, value_start, value_end);
        return parsed.has_value() ? *parsed : std::string{};
    }
    const auto value_end = document.find_first_of(",}", value_start);
    return trim_copy(document.substr(
        value_start,
        value_end == std::string::npos ? std::string::npos : value_end - value_start));
}

std::optional<std::string> parse_json_string_at(
    const std::string& document,
    std::size_t value_start,
    std::size_t& value_end) {
    if (value_start >= document.size() || document[value_start] != '"') {
        return std::nullopt;
    }
    std::string result;
    for (std::size_t index = value_start + 1U; index < document.size(); ++index) {
        const char ch = document[index];
        if (ch == '\\' && (index + 1U) < document.size()) {
            const char escaped = document[++index];
            switch (escaped) {
                case '"':
                case '\\':
                case '/':
                    result.push_back(escaped);
                    break;
                case 'b':
                    result.push_back('\b');
                    break;
                case 'f':
                    result.push_back('\f');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'u': {
                    const auto high_surrogate = parse_json_hex_quad(document, index + 1U);
                    if (!high_surrogate.has_value()) {
                        return std::nullopt;
                    }
                    index += 4U;
                    if (*high_surrogate >= 0xD800U && *high_surrogate <= 0xDBFFU) {
                        if (index + 2U >= document.size() || document[index + 1U] != '\\' ||
                            document[index + 2U] != 'u') {
                            return std::nullopt;
                        }
                        const auto low_surrogate = parse_json_hex_quad(document, index + 3U);
                        if (!low_surrogate.has_value() ||
                            *low_surrogate < 0xDC00U || *low_surrogate > 0xDFFFU) {
                            return std::nullopt;
                        }
                        index += 6U;
                        const std::uint32_t code_point =
                            0x10000U + ((static_cast<std::uint32_t>(*high_surrogate) - 0xD800U) << 10U) +
                            (static_cast<std::uint32_t>(*low_surrogate) - 0xDC00U);
                        if (!append_json_code_point(result, code_point)) {
                            return std::nullopt;
                        }
                    } else if (*high_surrogate >= 0xDC00U && *high_surrogate <= 0xDFFFU) {
                        return std::nullopt;
                    } else if (!append_json_code_point(result, *high_surrogate)) {
                        return std::nullopt;
                    }
                    break;
                }
                default:
                    return std::nullopt;
            }
            continue;
        }
        if (ch == '"') {
            value_end = index + 1U;
            return result;
        }
        if (static_cast<unsigned char>(ch) < 0x20U) {
            return std::nullopt;
        }
        result.push_back(ch);
    }
    return std::nullopt;
}

std::optional<std::size_t> find_json_array_end(const std::string& document, std::size_t array_start) {
    std::size_t depth = 0;
    bool inside_string = false;
    bool escaping = false;
    for (std::size_t index = array_start; index < document.size(); ++index) {
        const char ch = document[index];
        if (inside_string) {
            if (escaping) {
                escaping = false;
                continue;
            }
            if (ch == '\\') {
                escaping = true;
                continue;
            }
            if (ch == '"') {
                inside_string = false;
            }
            continue;
        }
        if (ch == '"') {
            inside_string = true;
            continue;
        }
        if (ch == '[') {
            ++depth;
            continue;
        }
        if (ch == ']') {
            if (depth == 0U) {
                return std::nullopt;
            }
            --depth;
            if (depth == 0U) {
                return index;
            }
        }
    }
    return std::nullopt;
}

std::vector<std::string> extract_bridge_parameter_field_values(
    const std::string& document,
    const std::string& field_name) {
    std::vector<std::string> values;
    std::size_t array_start = std::string::npos;
    if (!find_json_field_value_start(document, "parameters", array_start) ||
        array_start >= document.size() ||
        document[array_start] != '[') {
        return values;
    }
    const auto array_end = find_json_array_end(document, array_start);
    if (!array_end.has_value()) {
        return values;
    }

    const auto value_token = std::string("\"") + field_name + "\"";
    std::size_t object_depth = 0U;
    std::size_t array_depth = 1U;
    for (std::size_t cursor = array_start + 1U; cursor < *array_end; ++cursor) {
        const char ch = document[cursor];
        if (ch == '"') {
            std::size_t string_end = cursor + 1U;
            bool escaping = false;
            for (; string_end < *array_end; ++string_end) {
                const char string_ch = document[string_end];
                if (escaping) {
                    escaping = false;
                    continue;
                }
                if (string_ch == '\\') {
                    escaping = true;
                    continue;
                }
                if (string_ch == '"') {
                    break;
                }
            }
            if (string_end >= *array_end) {
                break;
            }
            if (object_depth == 1U &&
                array_depth == 1U &&
                string_end + 1U == cursor + value_token.size() &&
                document.compare(cursor, value_token.size(), value_token) == 0) {
                const auto colon_offset = document.find_first_not_of(" \t\r\n", string_end + 1U);
                if (colon_offset == std::string::npos ||
                    colon_offset >= *array_end ||
                    document[colon_offset] != ':') {
                    cursor = string_end;
                    continue;
                }
                const auto value_start = document.find_first_not_of(" \t\r\n", colon_offset + 1U);
                if (value_start == std::string::npos || value_start >= *array_end) {
                    break;
                }
                if (document[value_start] == '"') {
                    std::size_t value_end = value_start;
                    const auto parsed = parse_json_string_at(document, value_start, value_end);
                    if (!parsed.has_value() || value_end > *array_end + 1U) {
                        break;
                    }
                    values.push_back(*parsed);
                    cursor = value_end - 1U;
                    continue;
                }
                const auto value_end = document.find_first_of(",}", value_start);
                if (value_end == std::string::npos || value_end > *array_end) {
                    break;
                }
                values.push_back(trim_copy(document.substr(value_start, value_end - value_start)));
                cursor = value_end - 1U;
                continue;
            }
            cursor = string_end;
            continue;
        }
        if (ch == '{') {
            ++object_depth;
            continue;
        }
        if (ch == '}' && object_depth > 0U) {
            --object_depth;
            continue;
        }
        if (ch == '[') {
            ++array_depth;
            continue;
        }
        if (ch == ']' && array_depth > 0U) {
            --array_depth;
        }
    }
    return values;
}

std::vector<std::string> split_bridge_parameter_name_list(const std::string& names) {
    std::vector<std::string> result;
    std::size_t segment_start = 0;
    while (segment_start <= names.size()) {
        const std::size_t separator = names.find('|', segment_start);
        const std::string segment = trim_copy(names.substr(
            segment_start,
            separator == std::string::npos ? std::string::npos : separator - segment_start));
        if (!segment.empty()) {
            result.push_back(segment);
        }
        if (separator == std::string::npos) {
            break;
        }
        segment_start = separator + 1U;
    }
    return result;
}

bool bridge_parameter_names_match(
    const std::vector<std::string>& expected_names,
    const std::vector<std::string>& request_names) {
    if (expected_names.size() != request_names.size()) {
        return false;
    }
    for (std::size_t index = 0; index < expected_names.size(); ++index) {
        if (lowercase_copy(trim_copy(expected_names[index])) != lowercase_copy(trim_copy(request_names[index]))) {
            return false;
        }
    }
    return true;
}

struct RuntimeBridgeInvocationOptions {
    std::string library_export;
    std::string routine_kind;
    std::string source_path;
    std::string source_line;
    std::string parameter_declaration;
    std::string parameter_names;
    std::string parameter_count;
    std::string request_path;
    std::string response_path;
    std::string request_media_type;
    std::string response_media_type;
    std::string schema_version;
};

bool runtime_bridge_mode_requested(const RuntimeBridgeInvocationOptions& options) {
    return !trim_copy(options.request_path).empty() ||
           !trim_copy(options.response_path).empty() ||
           !trim_copy(options.request_media_type).empty() ||
           !trim_copy(options.response_media_type).empty() ||
           !trim_copy(options.schema_version).empty();
}

std::string localized_message(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& key,
    const copperfin::localization::PlaceholderMap& placeholders = {}) {
    return catalog.translate(key, placeholders);
}

std::string localized_message_or_default(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& key,
    const std::string& fallback,
    const copperfin::localization::PlaceholderMap& placeholders = {}) {
    const std::string translated = localized_message(catalog, key, placeholders);
    return translated == key ? fallback : translated;
}

std::string runtime_host_parse_boolean_value_required(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& option) {
    return localized_message(
        catalog,
        "RuntimeHost.Error.TrueFalseValueRequired",
        {{"option", option}});
}

void print_error_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& error) {
    std::cout << localized_message_or_default(catalog, "RuntimeHost.Prefix.Error", "error: ") << error << "\n";
}

void print_warning_line(
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::string& warning) {
    std::cerr << localized_message_or_default(catalog, "RuntimeHost.Prefix.Warning", "warning: ") << warning << "\n";
}

void print_license_status(
    const copperfin::licensing::LicenseStatus& status,
    const copperfin::localization::LocalizedCatalog& catalog) {
    using copperfin::licensing::LicenseState;

    std::cout << "status: ok\n";
    std::cout << "state: " << copperfin::licensing::license_state_name(status.state) << "\n";
    if (status.state == LicenseState::free) {
        return;
    }

    if (!status.license_id.empty()) {
        std::cout << "license_id: " << status.license_id << "\n";
    }
    if (!status.license_type.empty()) {
        std::cout << "license_type: " << status.license_type << "\n";
    }
    if (!status.pricing_model.empty()) {
        std::cout << "pricing_model: " << status.pricing_model << "\n";
    }
    if (!status.licensee_name.empty()) {
        std::cout << "licensee_name: " << status.licensee_name << "\n";
    }
    if (!status.licensee_email.empty()) {
        std::cout << "licensee_email: " << status.licensee_email << "\n";
    }
    if (status.seats > 0) {
        std::cout << "seats: " << status.seats << "\n";
    }
    if (!status.issued_date.empty()) {
        std::cout << "issued_date: " << status.issued_date << "\n";
    }
    if (!status.subscription_expires.empty()) {
        std::cout << "subscription_expires: " << status.subscription_expires << "\n";
    }
    if (status.perpetual_max_major_version > 0) {
        std::cout << "perpetual_max_major_version: " << status.perpetual_max_major_version << "\n";
    }
    if (!status.source_path.empty()) {
        std::cout << "source_path: " << status.source_path << "\n";
    }
    if (!status.diagnostic.empty()) {
        std::cout << "diagnostic: " << copperfin::licensing::localized_license_diagnostic(status, catalog) << "\n";
    }
}

bool is_runtime_bridge_routine_identifier(const std::string& value) {
    const std::string identifier = trim_copy(value);
    if (identifier.empty()) {
        return false;
    }
    const auto is_identifier_start = [](unsigned char ch) {
        return std::isalpha(ch) != 0 || ch == '_';
    };
    const auto is_identifier_part = [](unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '_';
    };
    if (!is_identifier_start(static_cast<unsigned char>(identifier.front()))) {
        return false;
    }
    return std::all_of(identifier.begin() + 1, identifier.end(), [&](char ch) {
        return is_identifier_part(static_cast<unsigned char>(ch));
    });
}

std::optional<std::filesystem::path> materialize_runtime_bridge_routine_bootstrap(
    const RuntimeBridgeInvocationOptions& options,
    const std::vector<std::string>& parameter_values,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error_message,
    std::string& bootstrap_source_text,
    const std::optional<std::string>& verified_source_text = std::nullopt) {
    const std::string export_name = trim_copy(options.library_export);
    if (!is_runtime_bridge_routine_identifier(export_name)) {
        error_message = localized_message(catalog, "RuntimeHost.Bridge.Error.UnsupportedRoutineExportName");
        return std::nullopt;
    }

    std::string source_text;
    if (verified_source_text.has_value()) {
        source_text = *verified_source_text;
    } else {
        std::ifstream source_input(path_from_utf8(options.source_path), std::ios::binary);
        if (!source_input.good()) {
            error_message = localized_message(catalog, "RuntimeHost.Bridge.Error.SourceArtifactNotFound");
            return std::nullopt;
        }
        std::ostringstream source_stream;
        source_stream << source_input.rdbuf();
        source_text = source_stream.str();
    }

    const std::string bootstrap_key = options.source_path + "|" + options.request_path + "|" + export_name;
    const std::filesystem::path bootstrap_path =
        std::filesystem::temp_directory_path() /
        ("copperfin_bridge_" + export_name + "_" + std::to_string(std::hash<std::string>{}(bootstrap_key)) + ".prg");
    std::ostringstream bootstrap_source;
    bootstrap_source << "DO " << export_name;
    if (!parameter_values.empty()) {
        bootstrap_source << " WITH ";
        for (std::size_t index = 0; index < parameter_values.size(); ++index) {
            if (index > 0U) {
                bootstrap_source << ", ";
            }
            bootstrap_source << parameter_values[index];
        }
    }
    bootstrap_source << "\n";
    bootstrap_source << source_text;
    if (!source_text.empty() && source_text.back() != '\n') {
        bootstrap_source << "\n";
    }
    bootstrap_source_text = bootstrap_source.str();

    std::ofstream bootstrap_output(bootstrap_path, std::ios::binary | std::ios::trunc);
    bootstrap_output << bootstrap_source_text;
    bootstrap_output.close();
    if (!bootstrap_output.good()) {
        error_message = localized_message(catalog, "RuntimeHost.Bridge.Error.WriteRoutineBootstrapFailed");
        return std::nullopt;
    }

    return bootstrap_path;
}

void remove_runtime_bridge_routine_bootstrap(const std::optional<std::filesystem::path>& bootstrap_path) {
    if (!bootstrap_path.has_value()) {
        return;
    }
    std::error_code ignored;
    std::filesystem::remove(*bootstrap_path, ignored);
}

int run_runtime_bridge_invocation(
    const RuntimeBridgeInvocationOptions& options,
    const std::string& startup_source,
    const std::string& working_directory,
    const copperfin::localization::LocalizedCatalog& catalog,
    const std::optional<std::string>& verified_startup_source_text,
    const std::optional<std::string>& verified_bridge_source_text,
    const std::optional<std::string>& verified_bridge_source_path,
    const std::map<std::string, std::string>& verified_source_texts,
    const std::map<std::string, std::string>& verified_file_bytes,
    const bool require_verified_source_texts) {
    if (trim_copy(options.request_path).empty() ||
        trim_copy(options.response_path).empty() ||
        trim_copy(options.request_media_type).empty() ||
        trim_copy(options.response_media_type).empty() ||
        trim_copy(options.schema_version).empty()) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequiredArguments"));
        return 2;
    }

    std::ifstream request_input(path_from_utf8(options.request_path), std::ios::binary);
    if (!request_input.good()) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestArtifactNotFound"));
        return 6;
    }
    std::ostringstream request_document_stream;
    request_document_stream << request_input.rdbuf();
    const std::string request_document = request_document_stream.str();
    const std::string request_media_type = extract_json_field(request_document, "request_media_type");
    const std::string request_schema_version = extract_json_field(request_document, "schema_version");
    const std::vector<std::string> parameter_names = extract_bridge_parameter_field_values(request_document, "name");
    const std::vector<std::string> parameter_values = extract_bridge_parameter_field_values(request_document, "value");
    if (request_media_type != options.request_media_type) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestMediaTypeMismatch"));
        return 6;
    }
    if (request_schema_version != options.schema_version) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestSchemaVersionMismatch"));
        return 6;
    }
    const auto descriptor_matches = [&](const std::string& field_name, const std::string& expected_value) {
        return extract_json_field(request_document, field_name) == expected_value;
    };
    if (!descriptor_matches("export_name", options.library_export) ||
        !descriptor_matches("routine_kind", options.routine_kind) ||
        !descriptor_matches("source_path", options.source_path) ||
        !descriptor_matches("source_line", options.source_line) ||
        !descriptor_matches("parameter_declaration", options.parameter_declaration) ||
        !descriptor_matches("parameter_names", options.parameter_names) ||
        !descriptor_matches("parameter_count", options.parameter_count)) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestDescriptorMismatch"));
        return 6;
    }

    std::string execution_source = trim_copy(options.source_path).empty()
        ? startup_source
        : options.source_path;
    bool routine_bootstrap_materialized = false;
    std::optional<std::filesystem::path> routine_bootstrap_path;
    std::optional<std::string> execution_source_text;
    if (!trim_copy(options.library_export).empty() &&
        !trim_copy(options.source_path).empty()) {
        if (trim_copy(options.parameter_count) != std::to_string(parameter_values.size())) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: bridge-invocation\n";
            print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestParameterCountMismatch"));
            return 6;
        }
        if (!bridge_parameter_names_match(split_bridge_parameter_name_list(options.parameter_names), parameter_names)) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: bridge-invocation\n";
            print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.RequestParameterNameMismatch"));
            return 6;
        }
        std::string bootstrap_error;
        std::string bootstrap_source_text;
        const auto bootstrap_path = materialize_runtime_bridge_routine_bootstrap(
            options,
            parameter_values,
            catalog,
            bootstrap_error,
            bootstrap_source_text,
            verified_bridge_source_text);
        if (!bootstrap_path.has_value()) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: bridge-invocation\n";
            print_error_line(catalog, bootstrap_error);
            return 6;
        }
        execution_source = copperfin::platform::path_to_utf8_string(*bootstrap_path);
        routine_bootstrap_path = *bootstrap_path;
        routine_bootstrap_materialized = true;
        execution_source_text = std::move(bootstrap_source_text);
    }
    if (lowercase_copy(copperfin::platform::path_to_utf8_string(path_from_utf8(execution_source).extension())) != ".prg") {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.PrgStartupRequired"));
        return 6;
    }

    copperfin::runtime::RuntimeSessionOptions session_options{};
    session_options.startup_path = execution_source;
    session_options.localization_catalog =
        std::make_shared<copperfin::localization::LocalizedCatalog>(catalog);
    if (execution_source_text.has_value()) {
        session_options.startup_source_text = std::move(execution_source_text);
    } else if (!routine_bootstrap_materialized && execution_source == startup_source) {
        session_options.startup_source_text = verified_startup_source_text;
    }
    session_options.source_text_overrides = verified_source_texts;
    if (routine_bootstrap_materialized && verified_bridge_source_path.has_value()) {
        const std::filesystem::path bridge_source_root =
            copperfin::platform::path_from_utf8_string(*verified_bridge_source_path).parent_path();
        const std::filesystem::path bootstrap_root =
            copperfin::platform::path_from_utf8_string(execution_source).parent_path();
        for (const auto& [source_path_text, source_text] : verified_source_texts) {
            const std::filesystem::path source_path =
                copperfin::platform::path_from_utf8_string(source_path_text);
            const std::filesystem::path relative = source_path.lexically_relative(bridge_source_root);
            if (!relative.empty() && relative != source_path && !relative.is_absolute()) {
                session_options.source_text_overrides.emplace(
                    copperfin::platform::path_to_utf8_string(
                        (bootstrap_root / relative).lexically_normal()),
                    source_text);
            }
        }
    }
    session_options.require_source_text_overrides = require_verified_source_texts;
    session_options.verified_file_byte_overrides = verified_file_bytes;
    session_options.require_verified_file_byte_overrides = require_verified_source_texts;
    session_options.working_directory = working_directory;
    session_options.stop_on_entry = false;
    session_options.quit_confirm_callback = []() -> bool {
        return true;
    };
    std::optional<copperfin::runtime::PrgRuntimeSession> created_session;
    try {
        created_session.emplace(copperfin::runtime::PrgRuntimeSession::create(session_options));
    } catch (const std::exception&) {
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            localized_message(
                catalog,
                "RuntimeHost.Error.VerifiedSourceUnavailable",
                {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
        return 6;
    }
    auto& session = *created_session;
    const auto runtime_state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    remove_runtime_bridge_routine_bootstrap(routine_bootstrap_path);
    if (runtime_state.reason == copperfin::runtime::DebugPauseReason::error) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, runtime_state.message);
        return 5;
    }

    const std::string return_value = runtime_state.last_return_value.has_value()
        ? copperfin::runtime::format_value(*runtime_state.last_return_value)
        : std::string{};

    const std::filesystem::path response_path = path_from_utf8(options.response_path);
    const auto parent_path = response_path.parent_path();
    if (!parent_path.empty()) {
        std::error_code directory_error;
        std::filesystem::create_directories(parent_path, directory_error);
        if (directory_error) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: bridge-invocation\n";
            print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.CreateResponseDirectoryFailed"));
            return 6;
        }
    }

    std::ofstream response_output(response_path, std::ios::binary | std::ios::trunc);
    response_output << "{\n"
                    << "  \"status\": \"ok\",\n"
                    << "  \"return_value\": \"" << escape_json_string(return_value) << "\",\n"
                    << "  \"response_media_type\": \"" << escape_json_string(options.response_media_type) << "\",\n"
                    << "  \"schema_version\": \"" << escape_json_string(options.schema_version) << "\",\n"
                    << "  \"diagnostics\": \"bridge_response_written\"\n"
                    << "}\n";
    response_output.close();
    if (!response_output.good()) {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.WriteResponseArtifactFailed"));
        return 6;
    }

    std::cout << "status: ok\n";
    std::cout << "runtime.mode: bridge-invocation\n";
    std::cout << "bridge.library_export: " << options.library_export << "\n";
    std::cout << "bridge.routine_kind: " << options.routine_kind << "\n";
    std::cout << "bridge.source: " << options.source_path << ":" << options.source_line << "\n";
    std::cout << "bridge.execution_source: " << execution_source << "\n";
    std::cout << "bridge.routine_bootstrap: " << (routine_bootstrap_materialized ? "true" : "false") << "\n";
    std::cout << "bridge.parameter_declaration: " << options.parameter_declaration << "\n";
    std::cout << "bridge.parameter_names: " << options.parameter_names << "\n";
    std::cout << "bridge.parameter_count: " << options.parameter_count << "\n";
    std::cout << "bridge.request_media_type: " << options.request_media_type << "\n";
    std::cout << "bridge.response_media_type: " << options.response_media_type << "\n";
    std::cout << "bridge.schema_version: " << options.schema_version << "\n";
    std::cout << "bridge.return_value: " << return_value << "\n";
    return 0;
}

std::string unescape_manifest_value(std::string value) {
    std::string result;
    result.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '\\' && (index + 1U) < value.size()) {
            const char next = value[index + 1U];
            if (next == 'n') {
                result.push_back('\n');
                ++index;
                continue;
            }
            if (next == 'r') {
                result.push_back('\r');
                ++index;
                continue;
            }
            if (next == '|') {
                result.push_back('|');
                ++index;
                continue;
            }
            if (next == '\\') {
                result.push_back('\\');
                ++index;
                continue;
            }
        }
        result.push_back(value[index]);
    }
    return result;
}

using ManifestMap = std::multimap<std::string, std::string>;
struct VerifiedPackagePath {
    copperfin::security::PhysicalPathContainmentResult containment;
    std::string sha256;
    bool declared_asset = false;
};
using VerifiedPackagePaths = std::vector<VerifiedPackagePath>;
constexpr int kMinimumSupportedManifestVersion = 1;
constexpr int kMaximumSupportedManifestVersion = 3;

enum class PackagePathBindingMode {
    allow_filename_fallback,
    allow_external_fidelity,
    strict_relative_fidelity
};

bool is_existing_directory(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_directory(path, error) && !error;
}

bool path_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

bool manifest_value_uses_pipe_delimiters(const std::string& key) {
    return key == "asset" ||
        key == "data_asset" ||
        key == "data_payload" ||
        key == "extension_payload";
}

ManifestMap load_manifest(const std::string& path) {
    ManifestMap values;
    std::ifstream input(path_from_utf8(path), std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        const auto delimiter = line.find('=');
        if (delimiter == std::string::npos) {
            continue;
        }

        const std::string key = trim_copy(line.substr(0U, delimiter));
        const std::string raw_value = line.substr(delimiter + 1U);
        const std::string value = trim_copy(
            manifest_value_uses_pipe_delimiters(key)
                ? raw_value
                : unescape_manifest_value(raw_value));
        values.emplace(key, value);
    }
    return values;
}

std::string first_value(const ManifestMap& values, const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? std::string{} : found->second;
}

std::vector<std::string> all_values(const ManifestMap& values, const std::string& key) {
    std::vector<std::string> result;
    const auto [begin, end] = values.equal_range(key);
    for (auto iterator = begin; iterator != end; ++iterator) {
        result.push_back(iterator->second);
    }
    return result;
}

std::optional<int> parse_manifest_version_value(const std::string& value) {
    const std::string trimmed = trim_copy(value);
    if (trimmed.empty()) {
        return std::nullopt;
    }
    if (!std::all_of(trimmed.begin(), trimmed.end(), [](unsigned char ch) {
            return std::isdigit(ch) != 0;
        })) {
        return std::nullopt;
    }

    try {
        return std::stoi(trimmed);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

enum class ManifestDocumentKind {
    missing,
    runtime_package,
    debug,
    ambiguous
};

struct ManifestVersionContract {
    ManifestDocumentKind kind = ManifestDocumentKind::missing;
    std::string raw_version;
};

ManifestVersionContract inspect_manifest_version_contract(const ManifestMap& manifest) {
    const auto runtime_versions = all_values(manifest, "manifest_version");
    const auto debug_versions = all_values(manifest, "debug_manifest_version");
    if ((runtime_versions.size() + debug_versions.size()) > 1U) {
        return {
            .kind = ManifestDocumentKind::ambiguous,
            .raw_version = {}
        };
    }
    if (runtime_versions.size() == 1U) {
        return {
            .kind = ManifestDocumentKind::runtime_package,
            .raw_version = runtime_versions.front()
        };
    }
    if (debug_versions.size() == 1U) {
        return {
            .kind = ManifestDocumentKind::debug,
            .raw_version = debug_versions.front()
        };
    }
    return {};
}

bool has_debug_manifest_path_identity(const std::string& manifest_path) {
    const std::filesystem::path normalized_manifest_path =
        path_from_utf8(manifest_path).lexically_normal();
    return equals_insensitive(copperfin::platform::path_to_utf8_string(normalized_manifest_path.filename()), "app.cfdebug") ||
           equals_insensitive(copperfin::platform::path_to_utf8_string(normalized_manifest_path.extension()), ".cfdebug");
}

std::optional<int> resolved_manifest_version(const ManifestMap& manifest) {
    const ManifestVersionContract contract = inspect_manifest_version_contract(manifest);
    if (contract.kind != ManifestDocumentKind::runtime_package &&
        contract.kind != ManifestDocumentKind::debug) {
        return std::nullopt;
    }
    return parse_manifest_version_value(contract.raw_version);
}

bool is_sha256_hex(const std::string& value) {
    return value.size() == 64U &&
        std::all_of(value.begin(), value.end(), [](unsigned char ch) {
            return std::isxdigit(ch) != 0;
        });
}

bool validate_manifest_version(
    const ManifestMap& manifest,
    ManifestDocumentKind& document_kind,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error) {
    const ManifestVersionContract contract = inspect_manifest_version_contract(manifest);
    document_kind = contract.kind;
    if (contract.kind == ManifestDocumentKind::ambiguous) {
        error = localized_message(catalog, "RuntimeHost.Error.ManifestVersionContractAmbiguous");
        return false;
    }
    if (contract.kind == ManifestDocumentKind::missing ||
        trim_copy(contract.raw_version).empty()) {
        error = localized_message(catalog, "RuntimeHost.Error.ManifestVersionMissing");
        return false;
    }

    const auto parsed_version = parse_manifest_version_value(contract.raw_version);
    if (!parsed_version.has_value() ||
        *parsed_version < kMinimumSupportedManifestVersion ||
        *parsed_version > kMaximumSupportedManifestVersion) {
        error = localized_message(
            catalog,
            "RuntimeHost.Error.ManifestVersionUnsupported",
            {
                {"supportedVersions", "1, 2, 3"},
                {"version", contract.raw_version}
            });
        return false;
    }

    return true;
}

bool validate_manifest_data_contract_header(
    const ManifestMap& manifest,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error) {
    const int manifest_version = resolved_manifest_version(manifest).value_or(0);
    const auto data_policies = all_values(manifest, "data_policy");
    const std::string data_policy = data_policies.empty() ? std::string{} : data_policies.front();
    const bool has_data_entries =
        !all_values(manifest, "data_asset").empty() ||
        !all_values(manifest, "data_payload").empty();
    if ((manifest_version >= 3 &&
         (data_policies.size() != 1U || data_policy != "package_writable")) ||
        (manifest_version < 3 && (!data_policies.empty() || has_data_entries))) {
        error = localized_message(catalog, "RuntimeHost.Error.DataPolicyMalformed");
        return false;
    }
    return true;
}

std::vector<std::string> split_pipe(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    current.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        const char ch = value[index];
        if (ch == '\\' && (index + 1U) < value.size()) {
            const char next = value[index + 1U];
            if (next == 'n') {
                current.push_back('\n');
                ++index;
                continue;
            }
            if (next == 'r') {
                current.push_back('\r');
                ++index;
                continue;
            }
            if (next == '\\' || next == '|') {
                current.push_back(next);
                ++index;
                continue;
            }
        }
        if (ch == '|') {
            result.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(ch);
    }
    result.push_back(current);
    return result;
}

bool relative_path_escapes_root(const std::filesystem::path& relative_path) {
    for (const auto& part : relative_path) {
        if (part == "..") {
            return true;
        }
    }
    return false;
}

bool package_path_component_equal(
    const std::filesystem::path& left,
    const std::filesystem::path& right) {
    return copperfin::platform::path_component_equal_for_platform(left, right);
}

std::optional<std::filesystem::path> package_relative_path(
    const std::filesystem::path& path,
    const std::filesystem::path& root) {
    auto path_part = path.begin();
    bool components_match = true;
    for (auto root_part = root.begin(); root_part != root.end(); ++root_part, ++path_part) {
        if (path_part == path.end() || !package_path_component_equal(*path_part, *root_part)) {
            components_match = false;
            break;
        }
    }

    if (components_match) {
        std::filesystem::path relative;
        for (; path_part != path.end(); ++path_part) {
            relative /= *path_part;
        }
        if (!relative.empty()) {
            return relative;
        }
    }

#if defined(_WIN32)
    // A supported Windows installation can expose an accented case variant
    // that the Win32 text-comparison APIs do not fold consistently. When the
    // recorded package root exists, use filesystem identity for the root
    // prefix and retain the recorded suffix exactly. Physical containment is
    // still applied to the rebound path before it is admitted.
    std::size_t root_component_count = 0U;
    for (auto root_part = root.begin(); root_part != root.end(); ++root_part) {
        ++root_component_count;
    }
    auto prefix_part = path.begin();
    std::filesystem::path path_prefix;
    for (std::size_t index = 0U; index < root_component_count && prefix_part != path.end(); ++index) {
        path_prefix /= *prefix_part;
        ++prefix_part;
    }
    if (prefix_part != path.end()) {
        std::error_code identity_error;
        if (std::filesystem::equivalent(path_prefix, root, identity_error) &&
            !identity_error) {
            std::filesystem::path identity_relative;
            for (; prefix_part != path.end(); ++prefix_part) {
                identity_relative /= *prefix_part;
            }
            if (!identity_relative.empty()) {
                return identity_relative;
            }
        }
    }
#endif

    return std::nullopt;
}

std::filesystem::path portable_manifest_path(std::string value) {
#if !defined(_WIN32)
    const bool has_windows_drive_prefix =
        value.size() >= 3U &&
        std::isalpha(static_cast<unsigned char>(value[0])) != 0 &&
        value[1] == ':' &&
        (value[2] == '\\' || value[2] == '/');
    const bool has_windows_unc_prefix = value.rfind("\\\\", 0U) == 0U;
    if (has_windows_drive_prefix || has_windows_unc_prefix) {
        std::replace(value.begin(), value.end(), '\\', '/');
    }
#endif
    return path_from_utf8(value).lexically_normal();
}

bool manifest_path_is_absolute(const std::filesystem::path& path) {
    if (path.is_absolute()) {
        return true;
    }
#if !defined(_WIN32)
    const std::string generic_path = path.generic_string();
    return generic_path.size() >= 3U &&
           std::isalpha(static_cast<unsigned char>(generic_path[0])) != 0 &&
           generic_path[1] == ':' &&
           generic_path[2] == '/';
#else
    return false;
#endif
}

std::optional<std::filesystem::path> admit_existing_packaged_path(
    const std::filesystem::path& candidate,
    const std::filesystem::path& manifest_directory,
    const PackagePathBindingMode binding_mode,
    copperfin::security::PhysicalPathContainmentFailure* containment_failure = nullptr) {
    if (binding_mode == PackagePathBindingMode::strict_relative_fidelity) {
        const auto containment = copperfin::security::inspect_physical_path_containment(
            candidate,
            manifest_directory);
        if (!containment.allowed) {
            if (containment_failure != nullptr &&
                (*containment_failure == copperfin::security::PhysicalPathContainmentFailure::none ||
                 containment.failure == copperfin::security::PhysicalPathContainmentFailure::indirect_component ||
                 containment.failure == copperfin::security::PhysicalPathContainmentFailure::cross_device_component)) {
                *containment_failure = containment.failure;
            }
            return std::nullopt;
        }
        return containment.canonical_path;
    }

    std::error_code filesystem_error;
    if (!std::filesystem::exists(candidate, filesystem_error) || filesystem_error) {
        return std::nullopt;
    }
    return candidate.lexically_normal();
}

bool physical_indirection_was_rejected(
    const copperfin::security::PhysicalPathContainmentFailure failure) {
    return failure == copperfin::security::PhysicalPathContainmentFailure::indirect_component ||
           failure == copperfin::security::PhysicalPathContainmentFailure::cross_device_component;
}

bool physical_identity_has_multiple_links(
    const copperfin::security::PhysicalPathContainmentResult& containment) {
    return containment.allowed && containment.identity.link_count > 1U;
}

// Atomic check-and-open primitive (issue #5409/#5420): re-verifies a
// PhysicalPathContainmentResult captured potentially much earlier (e.g. by
// verify_manifest_hashes(), stored in verified_package_paths) via a fresh
// handle bound to its canonical_path, then reads through that same handle
// -- never reopening by path string the way this function's callers'
// pre-migration code did. Fails closed (an empty, !ok snapshot) if the
// fresh walk's identity no longer matches the stored one, rather than
// trusting the stale stored identity for the read.
copperfin::security::PhysicalFileSnapshotResult read_verified_package_path_snapshot(
    const copperfin::security::PhysicalPathContainmentResult& stored,
    const std::filesystem::path& manifest_directory) {
    auto handle = copperfin::security::inspect_and_open_physically_contained_path(
        stored.canonical_path, manifest_directory);
    if (!handle.result().allowed || handle.result().identity != stored.identity) {
        return copperfin::security::PhysicalFileSnapshotResult{};
    }
    return copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
        handle, manifest_directory);
}

bool packaged_source_text_extension(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(path.extension()));
    return extension == ".prg" || extension == ".mpr" ||
           extension == ".h" || extension == ".inc" || extension == ".ch" ||
           extension == ".txt";
}

bool packaged_query_extension(const std::filesystem::path& path) {
    return lowercase_copy(copperfin::platform::path_to_utf8_string(path.extension())) == ".qpr";
}

bool packaged_database_component_extension(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(path.extension()));
    return extension == ".dbf" || extension == ".fpt" || extension == ".cdx" ||
           extension == ".idx" || extension == ".ndx" || extension == ".mdx" ||
           extension == ".dbc" || extension == ".dct" || extension == ".dcx";
}

bool packaged_xasset_extension(const std::filesystem::path& path) {
    const std::string extension = lowercase_copy(copperfin::platform::path_to_utf8_string(path.extension()));
    return extension == ".scx" || extension == ".sct" ||
           extension == ".vcx" || extension == ".vct" ||
           extension == ".frx" || extension == ".frt" ||
           extension == ".lbx" || extension == ".lbt" ||
           extension == ".mnx" || extension == ".mnt";
}

std::optional<std::filesystem::path> bind_packaged_path(
    const std::string& manifest_value,
    const std::string& recorded_package_root,
    const std::filesystem::path& manifest_directory,
    const PackagePathBindingMode binding_mode = PackagePathBindingMode::allow_filename_fallback,
    copperfin::security::PhysicalPathContainmentFailure* containment_failure = nullptr) {
    if (trim_copy(manifest_value).empty()) {
        return std::nullopt;
    }

    const std::filesystem::path native_recorded_path =
        path_from_utf8(manifest_value).lexically_normal();
    if (trim_copy(recorded_package_root).empty() &&
        binding_mode == PackagePathBindingMode::strict_relative_fidelity &&
        native_recorded_path.is_absolute()) {
        const std::filesystem::path native_relative =
            native_recorded_path.lexically_relative(manifest_directory);
        if (!native_relative.empty() &&
            native_relative != native_recorded_path &&
            !relative_path_escapes_root(native_relative)) {
            if (const auto admitted = admit_existing_packaged_path(
                    native_recorded_path,
                    manifest_directory,
                    binding_mode,
                    containment_failure)) {
                return admitted;
            }
        }
    }

    const std::filesystem::path recorded_path = portable_manifest_path(manifest_value);
    const bool recorded_path_is_absolute = manifest_path_is_absolute(recorded_path);

    if (!trim_copy(recorded_package_root).empty()) {
        const std::filesystem::path package_root = portable_manifest_path(recorded_package_root);
        const std::filesystem::path relative =
            recorded_path.lexically_relative(package_root);
        if (!relative.empty() &&
            relative != recorded_path &&
            !relative_path_escapes_root(relative)) {
            const std::filesystem::path rebound =
                (manifest_directory / relative).lexically_normal();
            if (const auto admitted = admit_existing_packaged_path(
                    rebound,
                    manifest_directory,
                    binding_mode,
                    containment_failure)) {
                return admitted;
            }
            if (binding_mode == PackagePathBindingMode::strict_relative_fidelity) {
                return std::nullopt;
            }
        }
        if (binding_mode == PackagePathBindingMode::strict_relative_fidelity &&
            recorded_path_is_absolute) {
            return std::nullopt;
        }
    } else if (binding_mode == PackagePathBindingMode::strict_relative_fidelity &&
               recorded_path_is_absolute) {
        const std::filesystem::path relative =
            recorded_path.lexically_relative(manifest_directory);
        if (relative.empty() ||
            relative == recorded_path ||
            relative_path_escapes_root(relative) ||
            !path_exists_without_error(recorded_path)) {
            return std::nullopt;
        }
        return admit_existing_packaged_path(
            recorded_path,
            manifest_directory,
            binding_mode,
            containment_failure);
    }

    if (recorded_path_is_absolute && path_exists_without_error(recorded_path)) {
        return admit_existing_packaged_path(
            recorded_path,
            manifest_directory,
            binding_mode,
            containment_failure);
    }

    if (!recorded_path_is_absolute) {
        if (binding_mode == PackagePathBindingMode::strict_relative_fidelity &&
            relative_path_escapes_root(recorded_path)) {
            return std::nullopt;
        }
        const std::filesystem::path relative_candidate =
            (manifest_directory / recorded_path).lexically_normal();
        if (const auto admitted = admit_existing_packaged_path(
                relative_candidate,
                manifest_directory,
                binding_mode,
                containment_failure)) {
            return admitted;
        }
    }

    if (binding_mode == PackagePathBindingMode::allow_filename_fallback) {
        const std::filesystem::path filename_candidate =
            (manifest_directory / recorded_path.filename()).lexically_normal();
        if (const auto admitted = admit_existing_packaged_path(
                filename_candidate,
                manifest_directory,
                binding_mode,
                containment_failure)) {
            return admitted;
        }
    }

    return std::nullopt;
}

std::filesystem::path logical_deployment_path(
    const std::filesystem::path& admitted_path,
    const std::filesystem::path& manifest_directory) {
    std::error_code filesystem_error;
    const std::filesystem::path absolute_manifest_directory =
        std::filesystem::absolute(manifest_directory, filesystem_error).lexically_normal();
    if (filesystem_error) {
        return admitted_path.lexically_normal();
    }

    const std::filesystem::path canonical_manifest_directory =
        std::filesystem::canonical(absolute_manifest_directory, filesystem_error);
    if (filesystem_error) {
        return admitted_path.lexically_normal();
    }
    const auto relative_path =
        package_relative_path(admitted_path, canonical_manifest_directory);
    if (!relative_path.has_value()) {
        return admitted_path.lexically_normal();
    }

    std::filesystem::path logical_manifest_directory = absolute_manifest_directory;
    const std::filesystem::path manifest_parent =
        absolute_manifest_directory.parent_path();
    // Resolve package-root indirection while retaining namespace aliases above it.
    if (!manifest_parent.empty() && manifest_parent != absolute_manifest_directory) {
        const std::filesystem::path canonical_parent =
            std::filesystem::canonical(manifest_parent, filesystem_error);
        if (!filesystem_error) {
            if (const auto physical_root_from_parent =
                    package_relative_path(canonical_manifest_directory, canonical_parent)) {
                logical_manifest_directory =
                    (manifest_parent / *physical_root_from_parent).lexically_normal();
            }
        }
    }

    return (logical_manifest_directory / *relative_path).lexically_normal();
}

void add_verified_deployment_bytes(
    std::map<std::string, std::string>& verified_bytes,
    const std::filesystem::path& admitted_path,
    const std::filesystem::path& manifest_directory,
    const std::filesystem::path& canonical_startup_path,
    const std::filesystem::path& logical_startup_path,
    const std::string& bytes) {
    verified_bytes.emplace(
        copperfin::platform::path_to_utf8_string(admitted_path.lexically_normal()),
        bytes);
    verified_bytes.emplace(
        copperfin::platform::path_to_utf8_string(
            logical_deployment_path(admitted_path, manifest_directory)),
        bytes);

    const std::filesystem::path canonical_startup_parent =
        canonical_startup_path.parent_path();
    const std::filesystem::path logical_startup_parent =
        logical_startup_path.parent_path();
    const std::filesystem::path startup_relative_path =
        admitted_path.lexically_relative(canonical_startup_parent);
    if (!canonical_startup_parent.empty() &&
        !logical_startup_parent.empty() &&
        !startup_relative_path.empty() &&
        startup_relative_path != admitted_path &&
        !startup_relative_path.is_absolute()) {
        // Rootless manifests can retain an admitted alias after implicit manifest
        // discovery has canonicalized its directory.
        verified_bytes.emplace(
            copperfin::platform::path_to_utf8_string(
                (logical_startup_parent / startup_relative_path).lexically_normal()),
            bytes);
    }
}

bool verify_manifest_hashes(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error,
    VerifiedPackagePaths& verified_paths) {
    verified_paths.clear();
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string expected_runtime_host_hash = first_value(manifest, "runtime_host_sha256");
    if (expected_runtime_host_hash.empty()) {
        error = localized_message(catalog, "RuntimeHost.Error.ManifestMissingRuntimeHostSha256");
        return false;
    }

    const std::filesystem::path runtime_host_path =
        manifest_directory / packaged_runtime_host_file_name();
    // Atomic check-and-open primitive (issue #5409/#5420): the read below
    // is bound to the exact object this walk verifies, never reopened by
    // path string.
    auto runtime_host_handle = copperfin::security::inspect_and_open_physically_contained_path(
        runtime_host_path,
        manifest_directory);
    const auto& contained_runtime_host = runtime_host_handle.result();
    if (!contained_runtime_host.allowed) {
        error = localized_message(
            catalog,
            "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
            {{"fileName", copperfin::platform::path_to_utf8_string(runtime_host_path.filename())}});
        return false;
    }
    const auto runtime_host_snapshot =
        copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
            runtime_host_handle,
            manifest_directory);
    if (!runtime_host_snapshot.ok) {
        error = localized_message(
            catalog,
            "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
            {{"fileName", copperfin::platform::path_to_utf8_string(runtime_host_path.filename())}});
        return false;
    }
    const auto runtime_host_hash = copperfin::security::sha256_hex_for_text(
        runtime_host_snapshot.bytes);
    if (!runtime_host_hash.ok) {
        error = runtime_host_hash.error;
        return false;
    }
    if (lowercase_copy(runtime_host_hash.hex_digest) != lowercase_copy(expected_runtime_host_hash)) {
        error = localized_message(catalog, "RuntimeHost.Error.RuntimeHostSha256Mismatch");
        return false;
    }
    verified_paths.push_back({
        .containment = runtime_host_snapshot.containment,
        .sha256 = lowercase_copy(runtime_host_hash.hex_digest)
    });

    const int manifest_version = resolved_manifest_version(manifest).value_or(0);
    const auto data_asset_values = all_values(manifest, "data_asset");
    const auto data_payload_values = all_values(manifest, "data_payload");
    const std::string data_policy = first_value(manifest, "data_policy");
    if ((manifest_version >= 3 && data_policy != "package_writable") ||
        (manifest_version < 3 &&
         (!data_policy.empty() || !data_asset_values.empty() || !data_payload_values.empty()))) {
        error = localized_message(catalog, "RuntimeHost.Error.DataPolicyMalformed");
        return false;
    }

    std::vector<std::filesystem::path> writable_data_paths;
    std::vector<std::filesystem::path> writable_data_payload_paths;
    for (const auto& data_asset : data_asset_values) {
        const auto parts = split_pipe(data_asset);
        if (parts.size() != 2U || parts[1] != "package_writable") {
            error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
            return false;
        }
        copperfin::security::PhysicalPathContainmentFailure containment_failure =
            copperfin::security::PhysicalPathContainmentFailure::none;
        const auto bound_path = bind_packaged_path(
            parts[0],
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity,
            &containment_failure);
        if (!bound_path.has_value()) {
            error = localized_message(
                catalog,
                physical_indirection_was_rejected(containment_failure)
                    ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                    : "RuntimeHost.Error.PackagedAssetMissing",
                {{"fileName", copperfin::platform::path_to_utf8_string(portable_manifest_path(parts[0]).filename())}});
            return false;
        }
        if (std::find(writable_data_paths.begin(), writable_data_paths.end(), *bound_path) !=
            writable_data_paths.end()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
            return false;
        }
        const auto writable_containment = copperfin::security::inspect_physical_path_containment(
            *bound_path,
            manifest_directory);
        if (!writable_containment.allowed || physical_identity_has_multiple_links(writable_containment)) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_path->filename())}});
            return false;
        }
        writable_data_paths.push_back(*bound_path);
    }

    if (const auto startup_path = bind_packaged_path(
            first_value(manifest, "startup_source"),
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity);
        startup_path.has_value() &&
        std::find(writable_data_paths.begin(), writable_data_paths.end(), *startup_path) !=
            writable_data_paths.end()) {
        error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
        return false;
    }

    for (const auto& data_payload : data_payload_values) {
        const auto parts = split_pipe(data_payload);
        if (parts.size() != 3U || parts[1] != "package_writable" || !is_sha256_hex(parts[2])) {
            error = localized_message(catalog, "RuntimeHost.Error.DataPayloadMalformed");
            return false;
        }
        copperfin::security::PhysicalPathContainmentFailure containment_failure =
            copperfin::security::PhysicalPathContainmentFailure::none;
        const auto bound_path = bind_packaged_path(
            parts[0],
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity,
            &containment_failure);
        if (!bound_path.has_value()) {
            error = localized_message(
                catalog,
                physical_indirection_was_rejected(containment_failure)
                    ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                    : "RuntimeHost.Error.ExtensionPayloadMissingFromPackage",
                {{"fileName", copperfin::platform::path_to_utf8_string(portable_manifest_path(parts[0]).filename())}});
            return false;
        }
        if (std::find(
                writable_data_payload_paths.begin(),
                writable_data_payload_paths.end(),
                *bound_path) != writable_data_payload_paths.end()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataPayloadMalformed");
            return false;
        }
        // Atomic check-and-open primitive (issue #5409/#5420): the read
        // below is bound to the exact object this walk verifies, never
        // reopened by path string.
        auto payload_handle = copperfin::security::inspect_and_open_physically_contained_path(
            *bound_path,
            manifest_directory);
        const auto& contained_payload = payload_handle.result();
        if (!contained_payload.allowed || physical_identity_has_multiple_links(contained_payload)) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_path->filename())}});
            return false;
        }
        const auto payload_snapshot =
            copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                payload_handle,
                manifest_directory);
        // The read and its post-read re-walk both deliberately use
        // content_equal() (excluding link_count), so a hard link added
        // between the pre-read gate above and this point would otherwise
        // let a multiply linked package-writable file through undetected --
        // a sandbox-escape-relevant gap, not merely a data-integrity one,
        // since this path is later written to (Codex review finding, P1).
        if (!payload_snapshot.ok ||
            physical_identity_has_multiple_links(payload_snapshot.containment)) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_path->filename())}});
            return false;
        }
        writable_data_payload_paths.push_back(payload_snapshot.containment.canonical_path);
    }

    const auto payload_values = all_values(manifest, "extension_payload");
    for (const auto& payload : payload_values) {
        const auto parts = split_pipe(payload);
        if (parts.size() != 2U) {
            error = localized_message(catalog, "RuntimeHost.Error.ExtensionPayloadMalformed");
            return false;
        }

        copperfin::security::PhysicalPathContainmentFailure payload_containment_failure =
            copperfin::security::PhysicalPathContainmentFailure::none;
        const auto bound_payload_path = bind_packaged_path(
            parts[0],
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity,
            &payload_containment_failure);
        if (!bound_payload_path.has_value()) {
            error = localized_message(
                catalog,
                physical_indirection_was_rejected(payload_containment_failure)
                    ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                    : "RuntimeHost.Error.ExtensionPayloadMissingFromPackage",
                {{"fileName", copperfin::platform::path_to_utf8_string(portable_manifest_path(parts[0]).filename())}});
            return false;
        }

        // Atomic check-and-open primitive (issue #5409/#5420): the read
        // below is bound to the exact object this walk verifies, never
        // reopened by path string.
        auto payload_handle = copperfin::security::inspect_and_open_physically_contained_path(
            *bound_payload_path,
            manifest_directory);
        const auto& contained_payload = payload_handle.result();
        if (!contained_payload.allowed) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_payload_path->filename())}});
            return false;
        }
        if (std::find(
                writable_data_payload_paths.begin(),
                writable_data_payload_paths.end(),
                contained_payload.canonical_path) != writable_data_payload_paths.end()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataPayloadMalformed");
            return false;
        }
        const auto payload_snapshot =
            copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                payload_handle,
                manifest_directory);
        if (!payload_snapshot.ok) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_payload_path->filename())}});
            return false;
        }
        const auto digest = copperfin::security::sha256_hex_for_text(payload_snapshot.bytes);
        if (!digest.ok) {
            error = digest.error;
            return false;
        }
        if (lowercase_copy(digest.hex_digest) != lowercase_copy(parts[1])) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.ExtensionPayloadSha256Mismatch",
                {{"fileName", copperfin::platform::path_to_utf8_string(bound_payload_path->filename())}});
            return false;
        }
        verified_paths.push_back({
            .containment = payload_snapshot.containment,
            .sha256 = lowercase_copy(digest.hex_digest)
        });
    }

    const auto asset_values = all_values(manifest, "asset");
    for (const auto& asset : asset_values) {
        const auto parts = split_pipe(asset);
        if (parts.size() != 8U) {
            error = localized_message(catalog, "RuntimeHost.Error.AssetEntryMalformed");
            return false;
        }
        if (!equals_insensitive(parts[7], "true")) {
            continue;
        }
        if (trim_copy(parts[6]).empty()) {
            error = localized_message(catalog, "RuntimeHost.Error.AssetEntryMalformed");
            return false;
        }

        copperfin::security::PhysicalPathContainmentFailure asset_containment_failure =
            copperfin::security::PhysicalPathContainmentFailure::none;
        const auto bound_asset_path = bind_packaged_path(
            parts[2],
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity,
            &asset_containment_failure);
        const std::string asset_file_name = copperfin::platform::path_to_utf8_string(portable_manifest_path(parts[2]).filename());
        if (!bound_asset_path.has_value()) {
            error = localized_message(
                catalog,
                physical_indirection_was_rejected(asset_containment_failure)
                    ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                    : "RuntimeHost.Error.PackagedAssetMissing",
                {{"fileName", asset_file_name}});
            return false;
        }

        // Atomic check-and-open primitive (issue #5409/#5420): the read
        // below is bound to the exact object this walk verifies, never
        // reopened by path string.
        auto asset_handle = copperfin::security::inspect_and_open_physically_contained_path(
            *bound_asset_path,
            manifest_directory);
        const auto& contained_asset = asset_handle.result();
        if (!contained_asset.allowed) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", asset_file_name}});
            return false;
        }
        const auto asset_snapshot =
            copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                asset_handle,
                manifest_directory);
        if (!asset_snapshot.ok) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", asset_file_name}});
            return false;
        }
        const auto digest = copperfin::security::sha256_hex_for_text(asset_snapshot.bytes);
        if (!digest.ok) {
            error = digest.error;
            return false;
        }
        const bool package_writable =
            std::find(
                writable_data_paths.begin(),
                writable_data_paths.end(),
                contained_asset.canonical_path) != writable_data_paths.end();
        // Checked against asset_snapshot.containment (the fresh post-read
        // identity), not contained_asset (the pre-read identity bound to
        // asset_handle.result()): the read and its post-read re-walk both
        // deliberately use content_equal() (excluding link_count), so a
        // hard link added during the read would otherwise let a multiply
        // linked package-writable asset through undetected -- a
        // sandbox-escape-relevant gap, not merely a data-integrity one,
        // since this path is later written to (Codex review finding, P1,
        // same class as the writable data payload site above).
        if (package_writable &&
            physical_identity_has_multiple_links(asset_snapshot.containment)) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", asset_file_name}});
            return false;
        }
        if (package_writable && !is_sha256_hex(parts[6])) {
            error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
            return false;
        }
        if (!package_writable &&
            lowercase_copy(digest.hex_digest) != lowercase_copy(parts[6])) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagedAssetSha256Mismatch",
                {{"fileName", asset_file_name}});
            return false;
        }
        // A copied xAsset is not fully verified until its memo table is admitted by the same manifest.
        const std::string asset_extension =
            lowercase_copy(copperfin::platform::path_to_utf8_string(bound_asset_path->extension()));
        const bool is_executable_xasset =
            asset_extension == ".scx" || asset_extension == ".vcx" ||
            asset_extension == ".frx" || asset_extension == ".lbx" ||
            asset_extension == ".mnx";
        const copperfin::vfp::SidecarPathResolution sidecar_resolution =
            is_executable_xasset
                ? copperfin::vfp::resolve_vfp_memo_sidecar_path(*bound_asset_path)
                : copperfin::vfp::SidecarPathResolution{};
        if (sidecar_resolution.ambiguous) {
            error = localized_message(
                catalog,
                "Vfp.Sidecar.Error.AmbiguousPath",
                {{"path", copperfin::platform::path_to_utf8_string(sidecar_resolution.requested_path)}});
            return false;
        }
        const std::string sidecar_path = copperfin::platform::path_to_utf8_string(
            sidecar_resolution.path.value_or(sidecar_resolution.requested_path));
        if (!sidecar_path.empty()) {
            std::error_code sidecar_error;
            const bool sidecar_exists = std::filesystem::exists(path_from_utf8(sidecar_path), sidecar_error);
            if (sidecar_error || !sidecar_exists) {
                error = localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagedAssetMissing",
            {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
                return false;
            }

            const auto sidecar_containment = copperfin::security::inspect_physical_path_containment(
                path_from_utf8(sidecar_path),
                manifest_directory);
            if (!sidecar_containment.allowed) {
                error = localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
                return false;
            }
            const auto verified_sidecar = std::find_if(
                verified_paths.begin(),
                verified_paths.end(),
                [&](const auto& candidate) {
                    return !candidate.declared_asset &&
                        candidate.containment.canonical_path == sidecar_containment.canonical_path;
                });
            if (verified_sidecar == verified_paths.end()) {
                error = localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagedAssetDigestMissing",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
                return false;
            }
        }
        verified_paths.push_back({
            .containment = asset_snapshot.containment,
            .sha256 = lowercase_copy(digest.hex_digest),
            .declared_asset = true
        });
    }

    for (const auto& data_path : writable_data_paths) {
        const auto declared_asset = std::find_if(
            verified_paths.begin(),
            verified_paths.end(),
            [&](const auto& candidate) {
                return candidate.declared_asset &&
                    candidate.containment.canonical_path == data_path;
            });
        if (declared_asset == verified_paths.end()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
            return false;
        }

        const std::string primary_extension = lowercase_copy(copperfin::platform::path_to_utf8_string(data_path.extension()));
        const std::vector<std::string> companion_extensions = primary_extension == ".dbf"
            ? std::vector<std::string>{".fpt", ".cdx", ".idx", ".ndx", ".mdx"}
            : std::vector<std::string>{};
        if (companion_extensions.empty()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataAssetMalformed");
            return false;
        }

        std::error_code directory_error;
        for (const auto& entry : std::filesystem::directory_iterator(data_path.parent_path(), directory_error)) {
            if (directory_error) {
                break;
            }
            const std::filesystem::path candidate = entry.path();
            if (lowercase_copy(copperfin::platform::path_to_utf8_string(candidate.stem())) !=
                    lowercase_copy(copperfin::platform::path_to_utf8_string(data_path.stem())) ||
                std::find(
                    companion_extensions.begin(),
                    companion_extensions.end(),
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate.extension()))) == companion_extensions.end()) {
                continue;
            }
            const auto containment = copperfin::security::inspect_physical_path_containment(
                candidate,
                manifest_directory);
            if (!containment.allowed) {
                error = localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                    {{"fileName", copperfin::platform::path_to_utf8_string(candidate.filename())}});
                return false;
            }
            if (std::find(
                    writable_data_payload_paths.begin(),
                    writable_data_payload_paths.end(),
                    containment.canonical_path) == writable_data_payload_paths.end()) {
                error = localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagedAssetDigestMissing",
                    {{"fileName", copperfin::platform::path_to_utf8_string(candidate.filename())}});
                return false;
            }
        }
        if (directory_error) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(data_path.filename())}});
            return false;
        }
    }

    for (const auto& payload_path : writable_data_payload_paths) {
        const std::string payload_stem = lowercase_copy(copperfin::platform::path_to_utf8_string(payload_path.stem()));
        const auto owner = std::find_if(
            writable_data_paths.begin(),
            writable_data_paths.end(),
            [&](const auto& candidate) {
                return candidate.parent_path() == payload_path.parent_path() &&
                    lowercase_copy(copperfin::platform::path_to_utf8_string(candidate.stem())) == payload_stem;
            });
        if (owner == writable_data_paths.end()) {
            error = localized_message(catalog, "RuntimeHost.Error.DataPayloadMalformed");
            return false;
        }
        const std::string owner_extension = lowercase_copy(copperfin::platform::path_to_utf8_string(owner->extension()));
        const std::string payload_extension = lowercase_copy(copperfin::platform::path_to_utf8_string(payload_path.extension()));
        const bool allowed_payload = owner_extension == ".dbf" &&
            (payload_extension == ".fpt" || payload_extension == ".cdx" ||
             payload_extension == ".idx" || payload_extension == ".ndx" ||
             payload_extension == ".mdx");
        if (!allowed_payload) {
            error = localized_message(catalog, "RuntimeHost.Error.DataPayloadMalformed");
            return false;
        }
    }

    return true;
}

std::string resolve_federation_security_role() {
    const std::string configured_role =
        trim_copy(copperfin::platform::read_environment_variable_or_empty("COPPERFIN_SECURITY_ROLE"));
    return configured_role.empty() ? "developer" : configured_role;
}

std::string explicit_locale_from_arguments(int argc, char** argv) {
    for (int index = 1; index + 1 < argc; ++index) {
        if (equals_insensitive(argv[index], "--locale") || equals_insensitive(argv[index], "/locale")) {
            return argv[index + 1];
        }
    }
    return {};
}

copperfin::localization::LocalizedCatalog load_localization(
    const std::filesystem::path& executable_path,
    const std::string& explicit_locale) {
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root(executable_path);
    const auto configured_locale_root =
        copperfin::platform::read_environment_path("COPPERFIN_LOCALE_DIR");
    if (!configured_locale_root.has_value() || *configured_locale_root != locale_root) {
        (void)copperfin::platform::write_environment_path("COPPERFIN_LOCALE_DIR", locale_root);
    }
    return copperfin::localization::load_catalogs(
        locale_root,
        copperfin::localization::select_locale(explicit_locale));
}

void print_usage(const copperfin::localization::LocalizedCatalog& catalog) {
    const copperfin::localization::PlaceholderMap invariant_tokens{
        {"booleanValue", "<true|false>"},
        {"breakpointOption", "--breakpoint"},
        {"breakpointValue", "<file:line>"},
        {"commandName", "copperfin_runtime_host"},
        {"debugCommandOption", "--debug-command"},
        {"debugCommandValue", "<continue|step|next|out|watch:<expr>|select:<action-id>|invoke:<action-id>|break:add:<file:line>|break:remove:<file:line>|break:add-action:<action-id>|break:remove-action:<action-id>|break:clear|break:list>"},
        {"debugOption", "--debug"},
        {"debugStopOnEntryOption", "--debug-stop-on-entry"},
        {"federationBackendOption", "--federation-backend"},
        {"federationBackendValue", "<sqlite|postgresql|sqlserver|oracle>"},
        {"federationQueryOption", "--federation-query"},
        {"federationQueryValue", "<fox-sql>"},
        {"federationReadOnlyExecuteOption", "--federation-execute-read-only"},
        {"federationTargetOption", "--federation-target"},
        {"federationTargetValue", "<name>"},
        {"manifestOption", "--manifest"},
        {"manifestValue", "<path>"},
        {"planningAuditOption", "--federation-planning-audit"},
        {"planningEnableOption", "--federation-planning-enable"},
        {"planningRequireOption", "--federation-planning-require"}
    };
    std::cout << catalog.translate("RuntimeHost.Usage.Manifest", invariant_tokens) << "\n";
    std::cout << catalog.translate("RuntimeHost.Usage.Federation", invariant_tokens) << "\n";
    std::cout << catalog.translate("RuntimeHost.Usage.FederationPlanning", invariant_tokens) << "\n";
    std::cout << catalog.translate("RuntimeHost.Usage.FederationExecution", invariant_tokens) << "\n";
}

std::optional<copperfin::runtime::RuntimeBreakpoint> parse_breakpoint(const std::string& value, const std::string& startup_source) {
    const auto parse_line = [](const std::string& text) -> std::optional<std::size_t> {
        if (text.empty()) {
            return std::nullopt;
        }
        std::size_t parsed = 0;
        try {
            const auto line = static_cast<std::size_t>(std::stoull(text, &parsed));
            if (parsed != text.size()) {
                return std::nullopt;
            }
            return line;
        } catch (const std::exception&) {
            return std::nullopt;
        }
    };

    const auto separator = value.rfind(':');
    if (separator == std::string::npos) {
        const auto line = parse_line(value);
        if (!line.has_value()) {
            return std::nullopt;
        }
        return copperfin::runtime::RuntimeBreakpoint{
            .file_path = startup_source,
            .line = *line
        };
    }

    const auto line = parse_line(value.substr(separator + 1U));
    if (!line.has_value()) {
        return std::nullopt;
    }
    return copperfin::runtime::RuntimeBreakpoint{
        .file_path = value.substr(0U, separator),
        .line = *line
    };
}

std::optional<copperfin::runtime::DebugResumeAction> parse_resume_action(const std::string& value) {
    const std::string normalized = lowercase_copy(value);
    if (normalized == "continue") {
        return copperfin::runtime::DebugResumeAction::continue_run;
    }
    if (normalized == "step") {
        return copperfin::runtime::DebugResumeAction::step_into;
    }
    if (normalized == "next") {
        return copperfin::runtime::DebugResumeAction::step_over;
    }
    if (normalized == "out") {
        return copperfin::runtime::DebugResumeAction::step_out;
    }
    return std::nullopt;
}

const copperfin::runtime::XAssetActionBinding* find_breakpoint_xasset_action(
    const copperfin::runtime::RuntimeBreakpoint& breakpoint,
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source);

void print_pause_state(
    const copperfin::runtime::RuntimePauseState& state,
    const copperfin::runtime::XAssetExecutableModel* xasset_model = nullptr,
    const std::vector<copperfin::runtime::RuntimeBreakpoint>* breakpoints = nullptr,
    const std::string& xasset_bootstrap_path = {},
    const std::string& xasset_bootstrap_source = {}) {
    std::cout << "debug.reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) << "\n";
    std::cout << "debug.location: " << state.location.file_path << ":" << state.location.line << "\n";
    std::cout << "debug.statement: " << state.statement_text << "\n";
    std::cout << "debug.message: " << state.message << "\n";
    std::cout << "debug.stack.depth: " << state.call_stack.size() << "\n";
    std::cout << "debug.executed.statements: " << state.executed_statement_count << "\n";
    if (breakpoints != nullptr) {
        std::cout << "debug.breakpoint.count: " << breakpoints->size() << "\n";
        for (std::size_t index = 0; index < breakpoints->size(); ++index) {
            const auto& breakpoint = (*breakpoints)[index];
            std::cout << "debug.breakpoint[" << index << "]: " << breakpoint.file_path << ":" << breakpoint.line << "\n";
            if (xasset_model != nullptr) {
                if (const auto* action = find_breakpoint_xasset_action(
                        breakpoint,
                        *xasset_model,
                        xasset_bootstrap_path,
                        xasset_bootstrap_source)) {
                    std::cout << "debug.breakpoint[" << index << "].xasset.action_id: " << action->action_id << "\n";
                    std::cout << "debug.breakpoint[" << index << "].xasset.title: " << action->title << "\n";
                }
            }
        }
    }
    if (xasset_model != nullptr) {
        if (const auto* xasset_action = find_pause_xasset_action(state, *xasset_model)) {
            std::cout << "debug.xasset.action_id: " << xasset_action->action_id << "\n";
            std::cout << "debug.xasset.record_index: " << xasset_action->record_index << "\n";
            std::cout << "debug.xasset.kind: " << xasset_action->kind << "\n";
            std::cout << "debug.xasset.title: " << xasset_action->title << "\n";
        }
    }
    std::cout << "debug.workarea.selected: " << state.work_area.selected << "\n";
    std::cout << "debug.datasession.current: " << state.work_area.data_session << "\n";
    for (const auto& [area, alias] : state.work_area.aliases) {
        std::cout << "debug.workarea[" << area << "].alias: " << alias << "\n";
    }
    for (const auto& cursor : state.cursors) {
        std::cout << "debug.cursor[" << cursor.work_area << "].alias: " << cursor.alias << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].source: " << cursor.source_path << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].kind: " << cursor.source_kind << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].remote: " << (cursor.remote ? "true" : "false") << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].reccount: " << cursor.record_count << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].recno: " << cursor.recno << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].bof: " << (cursor.bof ? "true" : "false") << "\n";
        std::cout << "debug.cursor[" << cursor.work_area << "].eof: " << (cursor.eof ? "true" : "false") << "\n";
    }
    for (std::size_t index = 0; index < state.databases.size(); ++index) {
        const auto& database = state.databases[index];
        std::cout << "debug.database[" << index << "].path: " << database.path << "\n";
        std::cout << "debug.database[" << index << "].name: " << database.name << "\n";
        std::cout << "debug.database[" << index << "].exclusive: " << (database.exclusive ? "true" : "false") << "\n";
        std::cout << "debug.database[" << index << "].readonly: " << (database.read_only ? "true" : "false") << "\n";
        std::cout << "debug.database[" << index << "].current: " << (database.current ? "true" : "false") << "\n";
    }
    for (const auto& connection : state.sql_connections) {
        std::cout << "debug.sql[" << connection.handle << "].target: " << connection.target << "\n";
        std::cout << "debug.sql[" << connection.handle << "].provider: " << connection.provider << "\n";
        std::cout << "debug.sql[" << connection.handle << "].cursor: " << connection.last_cursor_alias << "\n";
        std::cout << "debug.sql[" << connection.handle << "].rows: " << connection.last_result_count << "\n";
    }
    for (const auto& object : state.ole_objects) {
        std::cout << "debug.ole[" << object.handle << "].progid: " << object.prog_id << "\n";
        std::cout << "debug.ole[" << object.handle << "].source: " << object.source << "\n";
        std::cout << "debug.ole[" << object.handle << "].lastaction: " << object.last_action << "\n";
        std::cout << "debug.ole[" << object.handle << "].actioncount: " << object.action_count << "\n";
    }
    for (std::size_t index = 0; index < state.call_stack.size(); ++index) {
        const auto& frame = state.call_stack[index];
        std::cout << "debug.frame[" << index << "]: " << frame.routine_name << "@" << frame.file_path << ":" << frame.line << "\n";
        for (const auto& [name, value] : frame.locals) {
            std::cout << "debug.frame[" << index << "].local." << name << ": "
                      << escape_debug_line_value(copperfin::runtime::format_value(value)) << "\n";
        }
    }
    for (const auto& [name, value] : state.globals) {
        std::cout << "debug.global." << name << ": "
                  << escape_debug_line_value(copperfin::runtime::format_value(value)) << "\n";
    }
    for (std::size_t index = 0; index < state.events.size(); ++index) {
        const auto& event = state.events[index];
        std::cout << "debug.event[" << index << "].category: " << event.category << "\n";
        std::cout << "debug.event[" << index << "].detail: " << event.detail << "\n";
        std::cout << "debug.event[" << index << "].location: " << event.location.file_path << ":" << event.location.line << "\n";
    }
}

void print_breakpoint_inventory(
    const copperfin::runtime::PrgRuntimeSession& session,
    const copperfin::runtime::XAssetExecutableModel* xasset_model = nullptr,
    const std::string& xasset_bootstrap_path = {},
    const std::string& xasset_bootstrap_source = {}) {
    const auto breakpoints = session.list_breakpoints();
    std::cout << "debug.breakpoint.count: " << breakpoints.size() << "\n";
    for (std::size_t index = 0; index < breakpoints.size(); ++index) {
        const auto& breakpoint = breakpoints[index];
        std::cout << "debug.breakpoint[" << index << "]: " << breakpoint.file_path << ":" << breakpoint.line << "\n";
        if (xasset_model != nullptr) {
            if (const auto* action = find_breakpoint_xasset_action(
                    breakpoint,
                    *xasset_model,
                    xasset_bootstrap_path,
                    xasset_bootstrap_source)) {
                std::cout << "debug.breakpoint[" << index << "].xasset.action_id: " << action->action_id << "\n";
                std::cout << "debug.breakpoint[" << index << "].xasset.title: " << action->title << "\n";
            }
        }
    }
}

struct XAssetFileSnapshotResult {
    bool ok = false;
    std::filesystem::path root;
    std::filesystem::path primary_path;
    std::string error;
};

bool write_binary_snapshot_file(
    const std::filesystem::path& path,
    const std::string& bytes) {
    if (bytes.size() > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        return false;
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    output.close();
    return output.good();
}

std::optional<std::filesystem::path> create_private_xasset_snapshot_root() {
    static std::atomic<unsigned long long> snapshot_nonce_counter{0ULL};
    unsigned long long random_bits = static_cast<unsigned long long>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    try {
        std::random_device random_source;
        random_bits ^=
            (static_cast<unsigned long long>(random_source()) << 32U) |
            static_cast<unsigned long long>(random_source());
    } catch (const std::exception&) {
    }
    for (std::size_t attempt = 0U; attempt < 16U; ++attempt) {
        const unsigned long long nonce =
            snapshot_nonce_counter.fetch_add(1ULL, std::memory_order_relaxed);
        const std::string unique_name =
            "copperfin_xasset_snapshot_" + std::to_string(current_process_id()) + "_" +
            std::to_string(random_bits) + "_" + std::to_string(nonce);
        const std::filesystem::path candidate =
            std::filesystem::temp_directory_path() / unique_name;
        std::error_code filesystem_error;
        if (!std::filesystem::create_directory(candidate, filesystem_error)) {
            continue;
        }
#if !defined(_WIN32)
        std::filesystem::permissions(
            candidate,
            std::filesystem::perms::owner_all,
            std::filesystem::perm_options::replace,
            filesystem_error);
        if (filesystem_error) {
            std::filesystem::remove(candidate, filesystem_error);
            continue;
        }
#endif
        return candidate;
    }
    return std::nullopt;
}

XAssetFileSnapshotResult materialize_xasset_file_snapshot(
    const std::string& logical_startup_source,
    const std::string& primary_bytes,
    const std::filesystem::path& manifest_directory,
    const copperfin::localization::LocalizedCatalog& catalog,
    const bool security_enabled,
    const VerifiedPackagePaths& verified_package_paths) {
    const std::filesystem::path logical_path = path_from_utf8(logical_startup_source);
    XAssetFileSnapshotResult result;
    const copperfin::vfp::SidecarPathResolution sidecar_resolution =
        copperfin::vfp::resolve_vfp_memo_sidecar_path(logical_path);
    if (sidecar_resolution.ambiguous) {
        result.error = localized_message(
            catalog,
            "Vfp.Sidecar.Error.AmbiguousPath",
            {{"path", copperfin::platform::path_to_utf8_string(sidecar_resolution.requested_path)}});
        return result;
    }
    const std::string sidecar_path = copperfin::platform::path_to_utf8_string(
        sidecar_resolution.path.value_or(sidecar_resolution.requested_path));
    const auto snapshot_root = create_private_xasset_snapshot_root();
    if (!snapshot_root.has_value()) {
        result.error = localized_message(
            catalog,
            "RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed",
            {{"fileName", copperfin::platform::path_to_utf8_string(logical_path.filename())}});
        return result;
    }
    result.root = *snapshot_root;

    result.primary_path = result.root / logical_path.filename();
    if (!write_binary_snapshot_file(result.primary_path, primary_bytes)) {
        result.error = localized_message(
            catalog,
            "RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed",
            {{"fileName", copperfin::platform::path_to_utf8_string(logical_path.filename())}});
        return result;
    }

    std::error_code filesystem_error;
    const bool sidecar_exists = !sidecar_path.empty() &&
        std::filesystem::exists(path_from_utf8(sidecar_path), filesystem_error);
    if (filesystem_error) {
        result.error = localized_message(
            catalog,
            "RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
        return result;
    }
    if (security_enabled && !sidecar_path.empty() && !sidecar_exists) {
        result.error = localized_message(
            catalog,
            "RuntimeHost.Error.PackagedAssetMissing",
            {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
        return result;
    }
    if (sidecar_exists) {
        // Atomic check-and-open primitive (issue #5409/#5420): opened once
        // here to resolve sidecar_path's canonical identity for the
        // verified_package_paths lookup below; kept open so an unverified
        // sidecar (only reachable when !security_enabled, below) can read
        // through this same handle rather than reopening by path string.
        auto sidecar_handle = copperfin::security::inspect_and_open_physically_contained_path(
            path_from_utf8(sidecar_path),
            manifest_directory);
        const auto& sidecar_containment = sidecar_handle.result();
        if (!sidecar_containment.allowed) {
            result.error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
            return result;
        }
        const auto verified_sidecar = std::find_if(
            verified_package_paths.begin(),
            verified_package_paths.end(),
            [&](const auto& candidate) {
                return candidate.containment.canonical_path == sidecar_containment.canonical_path;
            });
        if (security_enabled && verified_sidecar == verified_package_paths.end()) {
            result.error = localized_message(
                catalog,
                "RuntimeHost.Error.PackagedAssetDigestMissing",
                {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
            return result;
        }
        copperfin::security::PhysicalFileSnapshotResult sidecar_snapshot;
        if (verified_sidecar == verified_package_paths.end()) {
            sidecar_snapshot =
                copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                    sidecar_handle, manifest_directory);
        } else {
            // verified_sidecar->containment was captured potentially much
            // earlier, by verify_manifest_hashes(): re-verify and read via
            // a fresh handle bound to that stored identity, rather than
            // reusing sidecar_handle (whose identity was captured at this
            // function's own, later, inspection above) or reopening by
            // path string.
            sidecar_snapshot = read_verified_package_path_snapshot(
                verified_sidecar->containment, manifest_directory);
        }
        copperfin::security::Sha256Result sidecar_digest{
            .ok = true,
            .hex_digest = {},
            .error = {}
        };
        if (sidecar_snapshot.ok && security_enabled) {
            sidecar_digest = copperfin::security::sha256_hex_for_text(sidecar_snapshot.bytes);
        }
        if (!sidecar_snapshot.ok ||
            !sidecar_digest.ok ||
            (security_enabled &&
             lowercase_copy(sidecar_digest.hex_digest) != verified_sidecar->sha256) ||
            !write_binary_snapshot_file(
                result.root / path_from_utf8(sidecar_path).filename(),
                sidecar_snapshot.bytes)) {
            result.error = localized_message(
                catalog,
                "RuntimeHost.Error.MaterializeVerifiedStartupSnapshotFailed",
                {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(sidecar_path).filename())}});
            return result;
        }
    }

    result.ok = true;
    return result;
}

void remove_xasset_file_snapshot(const std::filesystem::path& root) {
    if (root.empty()) {
        return;
    }
    std::error_code ignored;
    std::filesystem::remove_all(root, ignored);
}

struct XAssetBootstrapResult {
    std::optional<std::string> bootstrap_path;
    std::string bootstrap_source;
    copperfin::runtime::XAssetExecutableModel model;
    std::string error;
};

std::filesystem::path make_runtime_host_xasset_bootstrap_path(const std::filesystem::path& startup_path) {
    static std::atomic<unsigned long long> bootstrap_nonce_counter{0ULL};
    const auto now_ticks = static_cast<unsigned long long>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
    const unsigned long long nonce_counter = bootstrap_nonce_counter.fetch_add(1ULL, std::memory_order_relaxed);
    std::string startup_stem = copperfin::platform::path_to_utf8_string(startup_path.stem());
    if (startup_stem.empty()) {
        startup_stem = "startup";
    }
    return std::filesystem::temp_directory_path() /
           (startup_stem + "_copperfin_host_bootstrap_" +
            std::to_string(now_ticks) + "_" +
            std::to_string(current_process_id()) + "_" +
            std::to_string(nonce_counter) + ".prg");
}

void remove_xasset_bootstrap(const std::optional<std::string>& bootstrap_path) {
    if (!bootstrap_path.has_value()) {
        return;
    }
    std::error_code ignored;
    std::filesystem::remove(path_from_utf8(*bootstrap_path), ignored);
}

bool xasset_bootstrap_write_failure_requested(const std::filesystem::path& path) {
    const auto marker = copperfin::platform::read_environment_variable(
        "COPPERFIN_TEST_FAIL_WRITE_PATH_CONTAINS");
    const auto stage = copperfin::platform::read_environment_variable(
        "COPPERFIN_TEST_FAIL_WRITE_STAGE");
    return marker.has_value() && stage.has_value() &&
        !marker->empty() && *stage == "xasset-bootstrap" &&
        copperfin::platform::path_to_utf8_string(path).find(*marker) != std::string::npos;
}

XAssetBootstrapResult materialize_xasset_bootstrap(
    const std::string& startup_read_path,
    const std::string& logical_startup_source,
    bool include_read_events,
    const copperfin::localization::LocalizedCatalog& catalog) {
    XAssetBootstrapResult result;
    copperfin::studio::StudioOpenRequest request{};
    request.path = startup_read_path;
    request.read_only = true;
    request.load_full_table = true;
    auto open_result = copperfin::studio::open_document(request, catalog);
    if (!open_result.ok) {
        result.error = open_result.error;
        return result;
    }

    open_result.document.path = logical_startup_source;
    open_result.document.display_name =
        copperfin::platform::path_to_utf8_string(path_from_utf8(logical_startup_source).filename());
    open_result.document.inspection.path = logical_startup_source;
    const copperfin::vfp::SidecarPathResolution sidecar_resolution =
        copperfin::vfp::resolve_vfp_memo_sidecar_path(logical_startup_source);
    if (sidecar_resolution.ambiguous) {
        result.error = localized_message(
            catalog,
            "Vfp.Sidecar.Error.AmbiguousPath",
            {{"path", copperfin::platform::path_to_utf8_string(sidecar_resolution.requested_path)}});
        return result;
    }
    open_result.document.sidecar_path = copperfin::platform::path_to_utf8_string(
        sidecar_resolution.path.value_or(sidecar_resolution.requested_path));
    result.model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    if (!result.model.ok || !result.model.runnable_startup) {
        result.error = result.model.error.empty()
            ? localized_message(catalog, "RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound")
            : result.model.error;
        return result;
    }

    const std::filesystem::path startup_path = path_from_utf8(logical_startup_source);
    const std::filesystem::path bootstrap_path = make_runtime_host_xasset_bootstrap_path(startup_path);
    result.bootstrap_source =
        copperfin::runtime::build_xasset_bootstrap_source(
            result.model,
            include_read_events,
            logical_startup_source,
            true);

    struct ScopedXAssetBootstrapFileCleanup {
        std::filesystem::path path;
        bool preserve = false;

        ~ScopedXAssetBootstrapFileCleanup() {
            if (preserve) {
                return;
            }
            std::error_code ignored;
            std::filesystem::remove(path, ignored);
        }
    } bootstrap_file_cleanup{bootstrap_path};

    std::ofstream output(bootstrap_path, std::ios::binary | std::ios::trunc);
    output << result.bootstrap_source;
    if (xasset_bootstrap_write_failure_requested(bootstrap_path)) {
        output.setstate(std::ios::badbit);
    }
    output.close();
    if (!output.good()) {
        result.error = localized_message(catalog, "RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed");
        return result;
    }

    result.bootstrap_path = copperfin::platform::path_to_utf8_string(bootstrap_path);
    bootstrap_file_cleanup.preserve = true;
    return result;
}

std::optional<std::string> resolve_action_routine_name(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& command) {
    std::size_t prefix_length = 0;
    if (starts_with_insensitive(command, "select:")) {
        prefix_length = 7U;
    } else if (starts_with_insensitive(command, "invoke:")) {
        prefix_length = 7U;
    } else {
        return std::nullopt;
    }

    const std::string action_id = lowercase_copy(trim_copy(command.substr(prefix_length)));
    const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
        return lowercase_copy(action.action_id) == action_id;
    });
    if (found == model.actions.end()) {
        return std::nullopt;
    }
    return found->routine_name;
}

std::optional<copperfin::runtime::RuntimeBreakpoint> resolve_action_breakpoint(
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source,
    const std::string& action_id) {
    const std::string normalized_action_id = lowercase_copy(trim_copy(action_id));
    const auto found = std::find_if(model.actions.begin(), model.actions.end(), [&](const copperfin::runtime::XAssetActionBinding& action) {
        return lowercase_copy(action.action_id) == normalized_action_id;
    });
    if (found == model.actions.end()) {
        return std::nullopt;
    }

    std::size_t current_line = 0;
    bool in_target_routine = false;
    std::size_t line_start = 0;
    while (line_start <= bootstrap_source.size()) {
        const std::size_t line_end = bootstrap_source.find('\n', line_start);
        std::string line = bootstrap_source.substr(
            line_start,
            line_end == std::string::npos ? std::string::npos : line_end - line_start);
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        ++current_line;
        if (line == "PROCEDURE " + found->routine_name) {
            in_target_routine = true;
        } else if (in_target_routine) {
            const std::string trimmed = trim_copy(line);
            if (starts_with_insensitive(trimmed, "ENDPROC")) {
                return std::nullopt;
            }
            if (!trimmed.empty() && trimmed[0] != '*') {
                return copperfin::runtime::RuntimeBreakpoint{
                    .file_path = bootstrap_path,
                    .line = current_line
                };
            }
        }

        if (line_end == std::string::npos) {
            break;
        }
        line_start = line_end + 1U;
    }

    return std::nullopt;
}

const copperfin::runtime::XAssetActionBinding* find_breakpoint_xasset_action(
    const copperfin::runtime::RuntimeBreakpoint& breakpoint,
    const copperfin::runtime::XAssetExecutableModel& model,
    const std::string& bootstrap_path,
    const std::string& bootstrap_source) {
    for (const auto& action : model.actions) {
        const auto resolved = resolve_action_breakpoint(
            model,
            bootstrap_path,
            bootstrap_source,
            action.action_id);
        if (!resolved.has_value()) {
            continue;
        }
        if (resolved->file_path == breakpoint.file_path && resolved->line == breakpoint.line) {
            return &action;
        }
    }

    return nullptr;
}

std::string resolve_effective_working_directory(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory,
    const bool allow_external_debug_directory = false) {
    const std::string recorded_working_directory = first_value(manifest, "working_directory");
    if (!trim_copy(recorded_working_directory).empty()) {
        const auto bound = bind_packaged_path(
            recorded_working_directory,
            first_value(manifest, "package_root"),
            manifest_directory,
            allow_external_debug_directory
                ? PackagePathBindingMode::allow_external_fidelity
                : PackagePathBindingMode::strict_relative_fidelity);
        if (bound.has_value() && is_existing_directory(*bound)) {
            return copperfin::platform::path_to_utf8_string(*bound);
        }
    }
    return copperfin::platform::path_to_utf8_string(
        (manifest_directory / "content").lexically_normal());
}

std::optional<std::filesystem::path> admit_direct_packaged_output_path(
    const std::filesystem::path& candidate,
    const std::filesystem::path& manifest_directory) {
    const std::filesystem::path normalized_candidate = candidate.lexically_normal();
    const auto relative = package_relative_path(normalized_candidate, manifest_directory);
    if (!relative.has_value() ||
        relative_path_escapes_root(*relative) ||
        manifest_path_is_absolute(*relative) ||
        normalized_candidate.filename().empty()) {
        return std::nullopt;
    }

    std::error_code status_error;
    const std::filesystem::file_status status =
        std::filesystem::symlink_status(normalized_candidate, status_error);
    if (status_error && status_error != std::errc::no_such_file_or_directory) {
        return std::nullopt;
    }
    const bool target_missing =
        status_error == std::errc::no_such_file_or_directory ||
        status.type() == std::filesystem::file_type::not_found;
    if (!target_missing) {
        if (!std::filesystem::is_regular_file(status)) {
            return std::nullopt;
        }
        const auto containment = copperfin::security::inspect_physical_path_containment(
            normalized_candidate,
            manifest_directory);
        return containment.allowed && containment.identity.link_count == 1U
            ? std::optional<std::filesystem::path>(containment.canonical_path)
            : std::nullopt;
    }

    std::filesystem::path existing_parent = normalized_candidate.parent_path();
    const std::filesystem::path normalized_manifest_directory =
        manifest_directory.lexically_normal();
    while (!existing_parent.empty()) {
        std::error_code parent_status_error;
        const std::filesystem::file_status parent_status =
            std::filesystem::symlink_status(existing_parent, parent_status_error);
        if (parent_status_error == std::errc::no_such_file_or_directory ||
            parent_status.type() == std::filesystem::file_type::not_found) {
            existing_parent = existing_parent.parent_path();
            continue;
        }
        const bool is_manifest_directory =
            existing_parent.lexically_normal() == normalized_manifest_directory;
        std::error_code directory_error;
        const bool is_directory = std::filesystem::is_directory(
            existing_parent,
            directory_error);
        // The admitted manifest directory may itself be a symlink or junction. Redirects
        // beneath that trust root remain rejected by the symlink-status condition.
        if (parent_status_error ||
            directory_error ||
            !is_directory ||
            (!std::filesystem::is_directory(parent_status) && !is_manifest_directory)) {
            return std::nullopt;
        }

        const auto parent_containment = copperfin::security::inspect_physical_path_containment(
            existing_parent,
            manifest_directory);
        if (!parent_containment.allowed ||
            !is_existing_directory(parent_containment.canonical_path)) {
            return std::nullopt;
        }
        const auto missing_suffix =
            package_relative_path(normalized_candidate, existing_parent);
        if (!missing_suffix.has_value() ||
            relative_path_escapes_root(*missing_suffix)) {
            return std::nullopt;
        }
        return (parent_containment.canonical_path / *missing_suffix).lexically_normal();
    }
    return std::nullopt;
}

std::optional<std::filesystem::path> bind_packaged_output_path(
    const std::string& manifest_value,
    const std::string& recorded_package_root,
    const std::filesystem::path& manifest_directory) {
    if (trim_copy(manifest_value).empty()) {
        return std::nullopt;
    }

    const std::filesystem::path recorded_path = portable_manifest_path(manifest_value);
    std::filesystem::path relative_path;
    if (!trim_copy(recorded_package_root).empty()) {
        const std::filesystem::path package_root = portable_manifest_path(recorded_package_root);
        if (const auto recorded_relative = package_relative_path(recorded_path, package_root)) {
            relative_path = *recorded_relative;
        } else if (!manifest_path_is_absolute(recorded_path) &&
                   !relative_path_escapes_root(recorded_path)) {
            relative_path = recorded_path;
        } else {
            return std::nullopt;
        }
    } else if (manifest_path_is_absolute(recorded_path)) {
        const auto recorded_relative = package_relative_path(recorded_path, manifest_directory);
        if (!recorded_relative.has_value()) {
            return std::nullopt;
        }
        relative_path = *recorded_relative;
    } else {
        relative_path = recorded_path;
    }

    if (relative_path.empty() ||
        manifest_path_is_absolute(relative_path) ||
        relative_path_escapes_root(relative_path)) {
        return std::nullopt;
    }
    return admit_direct_packaged_output_path(
        manifest_directory / relative_path,
        manifest_directory);
}

std::optional<std::filesystem::path> resolve_effective_audit_log_path(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    const std::string recorded_audit_log_path = first_value(manifest, "audit_log_path");
    if (!trim_copy(recorded_audit_log_path).empty()) {
        return bind_packaged_output_path(
            recorded_audit_log_path,
            first_value(manifest, "package_root"),
            manifest_directory);
    }

    return admit_direct_packaged_output_path(
        manifest_directory / "security_audit.log",
        manifest_directory);
}

std::optional<std::filesystem::path> bind_relative_startup_item(
    const std::string& startup_item,
    const std::string& root,
    const std::filesystem::path& manifest_directory,
    const bool allow_external_debug_path,
    copperfin::security::PhysicalPathContainmentFailure* containment_failure) {
    if (trim_copy(startup_item).empty() || trim_copy(root).empty()) {
        return std::nullopt;
    }

    const std::filesystem::path relative_item = portable_manifest_path(startup_item);
    if (manifest_path_is_absolute(relative_item) || relative_path_escapes_root(relative_item)) {
        return std::nullopt;
    }

    const std::filesystem::path candidate =
        (path_from_utf8(root) / relative_item).lexically_normal();
    return admit_existing_packaged_path(
        candidate,
        manifest_directory,
        allow_external_debug_path
            ? PackagePathBindingMode::allow_filename_fallback
            : PackagePathBindingMode::strict_relative_fidelity,
        containment_failure);
}

std::optional<std::string> resolve_startup_root(
    const ManifestMap& manifest,
    const std::string& key,
    const std::filesystem::path& manifest_directory,
    const std::filesystem::path& empty_value_fallback,
    const bool allow_external_debug_directory,
    copperfin::security::PhysicalPathContainmentFailure* containment_failure,
    const bool require_directory = false) {
    const std::filesystem::path fallback =
        (manifest_directory / empty_value_fallback).lexically_normal();
    const std::string recorded_root = first_value(manifest, key);
    if (trim_copy(recorded_root).empty()) {
        return copperfin::platform::path_to_utf8_string(fallback);
    }

    const auto bound = bind_packaged_path(
        recorded_root,
        first_value(manifest, "package_root"),
        manifest_directory,
        allow_external_debug_directory
            ? (require_directory
                ? PackagePathBindingMode::allow_external_fidelity
                : PackagePathBindingMode::allow_filename_fallback)
            : PackagePathBindingMode::strict_relative_fidelity,
        containment_failure);
    if (bound.has_value() && (!require_directory || is_existing_directory(*bound))) {
        return copperfin::platform::path_to_utf8_string(*bound);
    }
    if (require_directory && is_existing_directory(fallback)) {
        return copperfin::platform::path_to_utf8_string(fallback);
    }
    return std::nullopt;
}

std::optional<std::string> resolve_startup_source(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory,
    const bool allow_external_debug_source,
    copperfin::security::PhysicalPathContainmentFailure* containment_failure) {
    if (containment_failure != nullptr) {
        *containment_failure = copperfin::security::PhysicalPathContainmentFailure::none;
    }
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string startup_source = first_value(manifest, "startup_source");
    const std::filesystem::path normalized_startup_source = portable_manifest_path(startup_source);
    if (allow_external_debug_source &&
        manifest_path_is_absolute(normalized_startup_source) &&
        path_exists_without_error(normalized_startup_source)) {
        return copperfin::platform::path_to_utf8_string(normalized_startup_source);
    }
    const std::filesystem::path native_startup_source =
        path_from_utf8(startup_source).lexically_normal();
    if (trim_copy(recorded_package_root).empty() &&
        native_startup_source.is_absolute()) {
        if (admit_existing_packaged_path(
                native_startup_source,
                manifest_directory,
                PackagePathBindingMode::strict_relative_fidelity,
                containment_failure).has_value()) {
            return copperfin::platform::path_to_utf8_string(native_startup_source);
        }
    }
    if (const auto bound_startup = bind_packaged_path(
            startup_source,
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity,
            containment_failure)) {
        return copperfin::platform::path_to_utf8_string(logical_deployment_path(
            *bound_startup,
            manifest_directory));
    }

    const std::string startup_item = first_value(manifest, "startup_item");
    const std::filesystem::path normalized_startup_item = portable_manifest_path(startup_item);
    const bool legacy_item_fallback_allowed =
        trim_copy(startup_source).empty() ||
        (!manifest_path_is_absolute(normalized_startup_source) &&
         normalized_startup_source == normalized_startup_item);
    if (!legacy_item_fallback_allowed) {
        return std::nullopt;
    }

    const std::optional<std::string> content_root = resolve_startup_root(
        manifest,
        "content_root",
        manifest_directory,
        "content",
        allow_external_debug_source,
        containment_failure);
    if (content_root.has_value()) {
        if (const auto candidate = bind_relative_startup_item(
                startup_item,
                *content_root,
                manifest_directory,
                allow_external_debug_source,
                containment_failure)) {
            return copperfin::platform::path_to_utf8_string(logical_deployment_path(
                *candidate,
                manifest_directory));
        }
    }

    const std::optional<std::string> working_directory = resolve_startup_root(
        manifest,
        "working_directory",
        manifest_directory,
        "content",
        allow_external_debug_source,
        containment_failure,
        true);
    if (working_directory.has_value()) {
        if (const auto candidate = bind_relative_startup_item(
                startup_item,
                *working_directory,
                manifest_directory,
                allow_external_debug_source,
                containment_failure)) {
            return copperfin::platform::path_to_utf8_string(logical_deployment_path(
                *candidate,
                manifest_directory));
        }
    }

    return std::nullopt;
}

std::string resolve_implicit_manifest_path(
    const std::filesystem::path& executable_path,
    bool debug_mode) {
    if (executable_path.empty()) {
        return {};
    }

    const auto deployed_path = [&](std::string_view file_name) {
        return executable_path.parent_path() / file_name;
    };

    if (debug_mode) {
        const std::filesystem::path debug_manifest_path = deployed_path("app.cfdebug");
        if (path_exists_without_error(debug_manifest_path)) {
            return copperfin::platform::path_to_utf8_string(debug_manifest_path.lexically_normal());
        }
    }

    const std::filesystem::path manifest_path = deployed_path("app.cfmanifest");
    if (!path_exists_without_error(manifest_path)) {
        return {};
    }
    return copperfin::platform::path_to_utf8_string(manifest_path.lexically_normal());
}

}  // namespace

int run_runtime_host_main_impl(int argc, char** argv) {
    const std::filesystem::path invocation_path =
        argc > 0 && argv[0] != nullptr ? path_from_utf8(argv[0]) : std::filesystem::path();
    const std::filesystem::path running_executable_path =
        copperfin::platform::resolve_running_executable_path(invocation_path);
    const std::string explicit_locale = explicit_locale_from_arguments(argc, argv);
    const copperfin::localization::LocalizedCatalog catalog =
        load_localization(running_executable_path, explicit_locale);

#if defined(COPPERFIN_RUNTIME_HOST_TEST_HOOKS)
    if (copperfin::platform::read_environment_variable_or_empty(
            "COPPERFIN_TEST_THROW_RUNTIME_HOST") == "1") {
        throw std::runtime_error("test-injected host fault");
    }
#endif

    const auto hardening = copperfin::security::apply_default_process_hardening();
    if (!hardening.applied) {
        print_warning_line(catalog, hardening.message);
    }

    std::string manifest_path;
    std::string federation_backend;
    std::string federation_query;
    std::string federation_target;
    bool federation_planning_enable = false;
    bool federation_planning_require = false;
    bool federation_policy_audit = true;
    bool federation_execute_read_only = false;
    bool federation_execute_option_seen = false;
    bool debug_mode = false;
    bool debug_stop_on_entry = false;
    bool debug_server_mode = false;
    bool license_status_requested = false;
    std::vector<std::string> breakpoint_args;
    std::vector<std::string> debug_commands;
    RuntimeBridgeInvocationOptions bridge_options;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--license-status") {
            if constexpr (copperfin::licensing::kProductLicensingEnabled) {
                license_status_requested = true;
            } else {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.UnknownArgument",
                        {{"argument", arg}}));
                print_usage(catalog);
                return 2;
            }
        } else if (arg == "--manifest" && (index + 1) < argc) {
            manifest_path = argv[++index];
        } else if (arg == "--federation-backend" && (index + 1) < argc) {
            federation_backend = argv[++index];
        } else if (arg == "--federation-query" && (index + 1) < argc) {
            federation_query = argv[++index];
        } else if (arg == "--federation-target" && (index + 1) < argc) {
            federation_target = argv[++index];
        } else if (arg == "--federation-execute-read-only" && (index + 1) < argc) {
            federation_execute_option_seen = true;
            const std::string token = argv[++index];
            if (!parse_cli_bool_token(token, federation_execute_read_only)) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    runtime_host_parse_boolean_value_required(catalog, "--federation-execute-read-only"));
                print_usage(catalog);
                return 2;
            }
        } else if (arg == "--federation-planning-enable" && (index + 1) < argc) {
            const std::string token = argv[++index];
            if (!parse_cli_bool_token(token, federation_planning_enable)) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    runtime_host_parse_boolean_value_required(catalog, "--federation-planning-enable"));
                print_usage(catalog);
                return 2;
            }
        } else if (arg == "--federation-planning-require" && (index + 1) < argc) {
            const std::string token = argv[++index];
            if (!parse_cli_bool_token(token, federation_planning_require)) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    runtime_host_parse_boolean_value_required(catalog, "--federation-planning-require"));
                print_usage(catalog);
                return 2;
            }
        } else if (arg == "--federation-planning-audit" && (index + 1) < argc) {
            const std::string token = argv[++index];
            if (!parse_cli_bool_token(token, federation_policy_audit)) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    runtime_host_parse_boolean_value_required(catalog, "--federation-planning-audit"));
                print_usage(catalog);
                return 2;
            }
        } else if (equals_insensitive(arg, "--debug") || equals_insensitive(arg, "/debug")) {
            debug_mode = true;
        } else if (arg == "--debug-stop-on-entry") {
            debug_stop_on_entry = true;
        } else if (arg == "--debug-server") {
            debug_mode = true;
            debug_server_mode = true;
        } else if (arg == "--breakpoint" && (index + 1) < argc) {
            breakpoint_args.emplace_back(argv[++index]);
        } else if (arg == "--debug-command" && (index + 1) < argc) {
            debug_commands.emplace_back(argv[++index]);
        } else if (arg == "--library-export" && (index + 1) < argc) {
            bridge_options.library_export = argv[++index];
        } else if (arg == "--routine-kind" && (index + 1) < argc) {
            bridge_options.routine_kind = argv[++index];
        } else if (arg == "--source-path" && (index + 1) < argc) {
            bridge_options.source_path = argv[++index];
        } else if (arg == "--source-line" && (index + 1) < argc) {
            bridge_options.source_line = argv[++index];
        } else if (arg == "--parameter-declaration" && (index + 1) < argc) {
            bridge_options.parameter_declaration = argv[++index];
        } else if (arg == "--parameter-names" && (index + 1) < argc) {
            bridge_options.parameter_names = argv[++index];
        } else if (arg == "--parameter-count" && (index + 1) < argc) {
            bridge_options.parameter_count = argv[++index];
        } else if (arg == "--request-path" && (index + 1) < argc) {
            bridge_options.request_path = argv[++index];
        } else if (arg == "--response-path" && (index + 1) < argc) {
            bridge_options.response_path = argv[++index];
        } else if (arg == "--request-media-type" && (index + 1) < argc) {
            bridge_options.request_media_type = argv[++index];
        } else if (arg == "--response-media-type" && (index + 1) < argc) {
            bridge_options.response_media_type = argv[++index];
        } else if (arg == "--schema-version" && (index + 1) < argc) {
            bridge_options.schema_version = argv[++index];
        } else if ((equals_insensitive(arg, "--locale") || equals_insensitive(arg, "/locale")) && (index + 1) < argc) {
            ++index;
        } else {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.UnknownArgument",
                    {{"argument", arg}}));
            print_usage(catalog);
            return 2;
        }
    }

    if (license_status_requested) {
        print_license_status(copperfin::licensing::load_license_status(running_executable_path), catalog);
        return 0;
    }

    const bool federation_mode_requested =
        !trim_copy(federation_backend).empty() || !trim_copy(federation_query).empty() ||
        federation_execute_option_seen;
    if (federation_mode_requested && runtime_bridge_mode_requested(bridge_options)) {
        std::cout << "status: error\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Error.BridgeFederationModeConflict"));
        return 2;
    }
    if (federation_mode_requested) {
        if (trim_copy(federation_backend).empty() || trim_copy(federation_query).empty()) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.FederationRequiredOptions",
                    {
                        {"federationBackendOption", "--federation-backend"},
                        {"federationQueryOption", "--federation-query"}
                    }));
            return 2;
        }

        const auto backend = copperfin::platform::federation_backend_from_string(federation_backend);
        if (!backend.has_value()) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.UnknownFederationBackend",
                    {{"backend", federation_backend}}));
            return 2;
        }

        const bool ai_planning_requested = federation_planning_enable || federation_planning_require;
        if (ai_planning_requested) {
            const auto security_profile = copperfin::security::default_native_security_profile(catalog);
            const std::string security_role = resolve_federation_security_role();
            if (!copperfin::security::role_has_permission(security_profile, security_role, "ai.mcp")) {
                std::cout << "status: error\n";
                std::cout << "runtime.mode: federation-query-plan\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.SecurityPolicyDenied",
                        {
                            {"permission", "ai.mcp"},
                            {"role", security_role}
                        }));
                return 7;
            }
        }

        const auto plan = copperfin::platform::build_federation_execution_plan({
            .backend = *backend,
            .fox_sql = federation_query,
            .target = federation_target,
            .planning_policy = {.enable_ai_assistance = federation_planning_enable,
                               .require_ai_assistance = federation_planning_require,
                               .policy_audit_enabled = federation_policy_audit}
        });
        if (!plan.ok) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: federation-query-plan\n";
            print_error_line(catalog, plan.error);
            return 6;
        }

        if (federation_execute_read_only) {
            if (*backend != copperfin::platform::FederationBackend::sqlite) {
                std::cout << "status: error\n";
                std::cout << "runtime.mode: federation-query-execute\n";
                print_error_line(
                    catalog,
                    localized_message(catalog, "RuntimeHost.Error.FederationExecutionRequiresSqlite"));
                return 2;
            }
            if (trim_copy(federation_target).empty()) {
                std::cout << "status: error\n";
                std::cout << "runtime.mode: federation-query-execute\n";
                print_error_line(
                    catalog,
                    localized_message(catalog, "RuntimeHost.Error.FederationExecutionTargetRequired"));
                return 2;
            }

            const auto security_profile = copperfin::security::default_native_security_profile(catalog);
            const std::string security_role = resolve_federation_security_role();
            if (!copperfin::security::role_has_permission(
                    security_profile,
                    security_role,
                    "project.open")) {
                std::cout << "status: error\n";
                std::cout << "runtime.mode: federation-query-execute\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.SecurityPolicyDenied",
                        {
                            {"permission", "project.open"},
                            {"role", security_role}
                        }));
                return 7;
            }

            const auto result =
                copperfin::platform::execute_sqlite_federation_plan_read_only(plan);
            std::cerr << "audit.event: federation.sqlite_read outcome="
                      << (result.ok ? "success" : "rejected") << "\n";
            if (!result.ok) {
                std::cout << "status: error\n";
                std::cout << "runtime.mode: federation-query-execute\n";
                std::cout << "federation.error_code: " << result.error_code << "\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.FederationExecutionFailed",
                        {{"errorCode", result.error_code}}));
                return 8;
            }

            std::cout << "status: ok\n";
            std::cout << "runtime.mode: federation-query-execute\n";
            std::cout << "federation.backend: sqlite\n";
            std::cout << "federation.connector: sqlite\n";
            std::cout << "federation.target: " << plan.target << "\n";
            std::cout << "federation.result_json: "
                      << copperfin::platform::serialize_sqlite_federation_result_json(result)
                      << "\n";
            return 0;
        }

        std::cout << "status: ok\n";
        std::cout << "runtime.mode: federation-query-plan\n";
        std::cout << "federation.backend: " << copperfin::platform::federation_backend_name(plan.backend) << "\n";
        std::cout << "federation.connector: " << plan.connector << "\n";
        std::cout << "federation.target: " << plan.target << "\n";
        std::cout << "federation.planning_mode: " << plan.planning_mode << "\n";
        std::cout << "federation.ai_assisted: " << (plan.ai_assisted ? "true" : "false") << "\n";
        std::cout << "federation.deterministic_translation_succeeded: " << (plan.deterministic_translation_succeeded ? "true" : "false") << "\n";
        std::cout << "federation.planning_policy_allows_ai: " << (plan.planning_policy_allows_ai ? "true" : "false") << "\n";
        std::cout << "federation.planning_policy_audit_enabled: " << (plan.planning_policy_audit_enabled ? "true" : "false") << "\n";
        std::cout << "federation.translated_sql: " << plan.translated_sql << "\n";
        std::cout << "federation.command: " << plan.execution_command << "\n";
        return 0;
    }

    if (manifest_path.empty()) {
        manifest_path = resolve_implicit_manifest_path(running_executable_path, debug_mode);
        if (manifest_path.empty()) {
            print_usage(catalog);
            return 2;
        }
    }

    if (!path_exists_without_error(path_from_utf8(manifest_path))) {
        std::cout << "status: error\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Error.ManifestNotFound"));
        return 3;
    }

    const auto manifest = load_manifest(manifest_path);
    if (manifest.empty()) {
        std::cout << "status: error\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Error.ManifestEmptyOrInvalid"));
        return 4;
    }
    ManifestDocumentKind manifest_document_kind = ManifestDocumentKind::missing;
    std::string manifest_version_error;
    if (!validate_manifest_version(
            manifest,
            manifest_document_kind,
            catalog,
            manifest_version_error)) {
        std::cout << "status: error\n";
        print_error_line(catalog, manifest_version_error);
        return 4;
    }
    std::string data_contract_error;
    if (!validate_manifest_data_contract_header(
            manifest,
            catalog,
            data_contract_error)) {
        std::cout << "status: error\n";
        print_error_line(catalog, data_contract_error);
        return 4;
    }

    std::error_code manifest_path_error;
    std::filesystem::path normalized_manifest_path =
        path_from_utf8(manifest_path).lexically_normal();
    if (normalized_manifest_path.is_relative()) {
        const std::filesystem::path absolute_manifest_path =
            std::filesystem::absolute(normalized_manifest_path, manifest_path_error);
        if (!manifest_path_error) {
            normalized_manifest_path = absolute_manifest_path.lexically_normal();
        }
    }
    const std::filesystem::path manifest_directory =
        normalized_manifest_path.parent_path();
    const auto assets = all_values(manifest, "asset");
    const auto warnings = all_values(manifest, "warning");
    bool security_enabled = false;
    if (!parse_manifest_bool_token(first_value(manifest, "security_enabled"), security_enabled)) {
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            runtime_host_parse_boolean_value_required(catalog, "security_enabled"));
        return 4;
    }
    const std::string security_role = first_value(manifest, "security_role");
    const auto resolved_audit_log_path =
        resolve_effective_audit_log_path(manifest, manifest_directory);
    if (security_enabled && !resolved_audit_log_path.has_value()) {
        const std::filesystem::path recorded_audit_log_path = portable_manifest_path(
            first_value(manifest, "audit_log_path"));
        const std::string audit_log_name = recorded_audit_log_path.filename().empty()
            ? "security_audit.log"
            : copperfin::platform::path_to_utf8_string(recorded_audit_log_path.filename());
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            localized_message(
                catalog,
                "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                {{"fileName", audit_log_name}}));
        return 8;
    }
    const std::string audit_log_path = resolved_audit_log_path.has_value()
        ? copperfin::platform::path_to_utf8_string(*resolved_audit_log_path)
        : std::string{};
    const auto append_audit_event = [&](const std::string& event_name, const std::string& detail) {
        return copperfin::security::append_immutable_audit_event_to_contained_file(
            audit_log_path,
            copperfin::platform::path_to_utf8_string(manifest_directory),
            event_name,
            detail);
    };
    const auto security_profile = copperfin::security::default_native_security_profile(catalog);
    VerifiedPackagePaths verified_package_paths;

    if (security_enabled) {
        if (!copperfin::security::role_has_permission(security_profile, security_role, "project.open")) {
            if (!audit_log_path.empty()) {
                (void)append_audit_event(
                    "policy.denied",
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.SecurityPolicyDenied",
                        {
                            {"permission", "project.open"},
                            {"role", security_role}
                        }));
            }
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.SecurityPolicyDenied",
                    {
                        {"permission", "project.open"},
                        {"role", security_role}
                    }));
            return 7;
        }

        std::string verification_error;
        if (!verify_manifest_hashes(
                manifest,
                manifest_directory,
                catalog,
                verification_error,
                verified_package_paths)) {
            if (!audit_log_path.empty()) {
                (void)append_audit_event(
                    "policy.denied",
                    verification_error);
            }
            std::cout << "status: error\n";
            print_error_line(catalog, verification_error);
            return 8;
        }

        if (!audit_log_path.empty()) {
            (void)append_audit_event(
                "runtime.start",
                "role=" + security_role + ",manifest=" + manifest_path);
        }
    }

    const bool debug_manifest_privileges =
        debug_mode &&
        manifest_document_kind == ManifestDocumentKind::debug &&
        has_debug_manifest_path_identity(manifest_path);
    copperfin::security::PhysicalPathContainmentFailure startup_containment_failure =
        copperfin::security::PhysicalPathContainmentFailure::none;
    const std::optional<std::string> resolved_startup_source =
        resolve_startup_source(
            manifest,
            manifest_directory,
            debug_manifest_privileges,
            &startup_containment_failure);
    if (!resolved_startup_source.has_value()) {
        const std::string recorded_startup_source = first_value(manifest, "startup_source");
        const std::string recorded_startup_item = first_value(manifest, "startup_item");
        const std::filesystem::path missing_startup_path = portable_manifest_path(
            recorded_startup_source.empty() ? recorded_startup_item : recorded_startup_source);
        const std::string missing_startup_name = missing_startup_path.filename().empty()
            ? (recorded_startup_source.empty() ? recorded_startup_item : recorded_startup_source)
            : copperfin::platform::path_to_utf8_string(missing_startup_path.filename());
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            localized_message(
                catalog,
                physical_indirection_was_rejected(startup_containment_failure)
                    ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                    : "RuntimeHost.Error.StartupSourceMissingFromPackage",
                {{"fileName", missing_startup_name}}));
        return 4;
    }
    const std::string startup_source = *resolved_startup_source;
    const std::string working_directory =
        resolve_effective_working_directory(
            manifest,
            manifest_directory,
            debug_manifest_privileges);

    std::optional<std::string> verified_startup_bytes;
    std::map<std::string, std::string> verified_source_texts;
    std::map<std::string, std::string> verified_file_bytes;
    if (!debug_manifest_privileges) {
        // Atomic check-and-open primitive (issue #5409/#5420): kept open so
        // an unverified startup source (only reachable when
        // !security_enabled, below) can read through this same handle
        // rather than reopening by path string.
        auto current_handle = copperfin::security::inspect_and_open_physically_contained_path(
            path_from_utf8(startup_source),
            manifest_directory);
        const auto& current_identity = current_handle.result();
        if (!current_identity.allowed) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
            return 4;
        }

        const auto verified = std::find_if(
            verified_package_paths.begin(),
            verified_package_paths.end(),
            [&](const auto& candidate) {
                return candidate.declared_asset &&
                       candidate.containment.canonical_path == current_identity.canonical_path;
            });
        if (security_enabled && verified == verified_package_paths.end()) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.StartupAssetDigestMissing",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
            return 8;
        }

        copperfin::security::PhysicalFileSnapshotResult startup_snapshot;
        if (verified == verified_package_paths.end()) {
            startup_snapshot =
                copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                    current_handle, manifest_directory);
        } else {
            // verified->containment was captured potentially much earlier,
            // by verify_manifest_hashes(): re-verify and read via a fresh
            // handle bound to that stored identity, rather than reusing
            // current_handle (whose identity was captured at this
            // function's own, later, inspection above) or reopening by
            // path string.
            startup_snapshot = read_verified_package_path_snapshot(
                verified->containment, manifest_directory);
        }
        if (!startup_snapshot.ok) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                    {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
            return 4;
        }
        if (verified != verified_package_paths.end()) {
            const auto digest = copperfin::security::sha256_hex_for_text(startup_snapshot.bytes);
            if (!digest.ok || lowercase_copy(digest.hex_digest) != verified->sha256) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.PackagedAssetSha256Mismatch",
                        {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
                return 8;
            }
        }
        verified_startup_bytes = startup_snapshot.bytes;
        if (security_enabled && packaged_source_text_extension(startup_source)) {
            add_verified_deployment_bytes(
                verified_source_texts,
                current_identity.canonical_path,
                manifest_directory,
                current_identity.canonical_path,
                startup_source,
                startup_snapshot.bytes);
        }

        if (security_enabled) {
            for (const auto& verified_path : verified_package_paths) {
                const auto& source_path = verified_path.containment.canonical_path;
                const bool source_text = packaged_source_text_extension(source_path);
                const bool query_file = packaged_query_extension(source_path);
                const bool database_component = packaged_database_component_extension(source_path);
                const bool xasset = packaged_xasset_extension(source_path);
                if ((!source_text && !query_file && !database_component && !xasset) ||
                    (source_text && verified_source_texts.contains(
                        copperfin::platform::path_to_utf8_string(source_path))) ||
                    ((query_file || database_component || xasset) && verified_file_bytes.contains(
                        copperfin::platform::path_to_utf8_string(source_path)))) {
                    continue;
                }
                // verified_path.containment was captured potentially much
                // earlier, by verify_manifest_hashes(): re-verify and read
                // via a fresh handle bound to that stored identity, never
                // reopening by path string (issue #5409/#5420).
                const auto source_snapshot =
                    read_verified_package_path_snapshot(verified_path.containment, manifest_directory);
                const auto source_digest = source_snapshot.ok
                    ? copperfin::security::sha256_hex_for_text(source_snapshot.bytes)
                    : copperfin::security::Sha256Result{};
                if (!source_snapshot.ok ||
                    !source_digest.ok ||
                    lowercase_copy(source_digest.hex_digest) != verified_path.sha256) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                            {{"fileName", copperfin::platform::path_to_utf8_string(source_path.filename())}}));
                    return 8;
                }
                if (source_text) {
                    add_verified_deployment_bytes(
                        verified_source_texts,
                        source_path,
                        manifest_directory,
                        current_identity.canonical_path,
                        startup_source,
                        source_snapshot.bytes);
                }
                if (query_file || database_component) {
                    add_verified_deployment_bytes(
                        verified_file_bytes,
                        source_path,
                        manifest_directory,
                        current_identity.canonical_path,
                        startup_source,
                        source_snapshot.bytes);
                }
                if (xasset) {
                    add_verified_deployment_bytes(
                        verified_file_bytes,
                        source_path,
                        manifest_directory,
                        current_identity.canonical_path,
                        startup_source,
                        source_snapshot.bytes);
                }
            }
        }
    }

    // External FFC headers are staged under a namespaced physical path to
    // avoid trust-root collisions, while VFP source keeps its logical
    // include path (for example, ffc\_ws3.h). Add only aliases derived from
    // manifest-admitted PRG Include assets; never search arbitrary host paths.
    if (!debug_manifest_privileges) {
        for (const auto& asset : assets) {
            const auto parts = split_pipe(asset);
            if (parts.size() != 8U || parts[3] != "PRG Include") {
                continue;
            }
            const std::filesystem::path relative_asset = path_from_utf8(parts[1]);
            auto component = relative_asset.begin();
            if (component == relative_asset.end() || *component != "external") {
                continue;
            }
            ++component;
            if (component == relative_asset.end()) {
                continue;
            }
            ++component;
            std::filesystem::path logical_relative;
            for (; component != relative_asset.end(); ++component) {
                logical_relative /= *component;
            }
            if (logical_relative.empty() || logical_relative.is_absolute() ||
                relative_path_escapes_root(logical_relative)) {
                continue;
            }

            copperfin::security::PhysicalPathContainmentFailure alias_failure =
                copperfin::security::PhysicalPathContainmentFailure::none;
            const auto bound_asset = bind_packaged_path(
                parts[2],
                first_value(manifest, "package_root"),
                manifest_directory,
                PackagePathBindingMode::strict_relative_fidelity,
                &alias_failure);
            if (!bound_asset.has_value()) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        physical_indirection_was_rejected(alias_failure)
                            ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                            : "RuntimeHost.Error.PackagedAssetMissing",
                        {{"fileName", copperfin::platform::path_to_utf8_string(relative_asset.filename())}}));
                return 4;
            }

            const std::string physical_key =
                copperfin::platform::path_to_utf8_string(bound_asset->lexically_normal());
            auto source_text = verified_source_texts.find(physical_key);
            if (source_text == verified_source_texts.end()) {
                // Atomic check-and-open primitive (issue #5409/#5420): the
                // read below is bound to the exact object this walk
                // verifies, never reopened by path string.
                auto handle =
                    copperfin::security::inspect_and_open_physically_contained_path(
                        *bound_asset,
                        manifest_directory);
                const auto snapshot =
                    copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                        handle,
                        manifest_directory);
                if (!snapshot.ok) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                            {{"fileName", copperfin::platform::path_to_utf8_string(relative_asset.filename())}}));
                    return 4;
                }
                source_text = verified_source_texts.emplace(physical_key, snapshot.bytes).first;
            }

            const std::filesystem::path logical_path =
                (manifest_directory / "content" / logical_relative).lexically_normal();
            verified_source_texts.emplace(
                copperfin::platform::path_to_utf8_string(logical_path),
                source_text->second);
        }
    }

    if (runtime_bridge_mode_requested(bridge_options)) {
        std::optional<std::string> verified_bridge_source_text;
        std::optional<std::string> verified_bridge_source_path;
        if (!trim_copy(bridge_options.source_path).empty() && !debug_manifest_privileges) {
            copperfin::security::PhysicalPathContainmentFailure bridge_containment_failure =
                copperfin::security::PhysicalPathContainmentFailure::none;
            const auto bound_bridge_source = bind_packaged_path(
                bridge_options.source_path,
                first_value(manifest, "package_root"),
                manifest_directory,
                PackagePathBindingMode::strict_relative_fidelity,
                &bridge_containment_failure);
            if (!bound_bridge_source.has_value()) {
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        physical_indirection_was_rejected(bridge_containment_failure)
                            ? "RuntimeHost.Error.PackagePathPhysicalContainmentFailed"
                            : "RuntimeHost.Bridge.Error.SourceMissingFromPackage",
                        {{"fileName", copperfin::platform::path_to_utf8_string(portable_manifest_path(bridge_options.source_path).filename())}}));
                return 4;
            }

            if (security_enabled) {
                const std::string bound_bridge_source_text =
                    copperfin::platform::path_to_utf8_string(*bound_bridge_source);
                const auto source_found = verified_source_texts.find(bound_bridge_source_text);
                if (source_found == verified_source_texts.end()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.PackagedAssetDigestMissing",
                        {{"fileName", copperfin::platform::path_to_utf8_string(bound_bridge_source->filename())}}));
                    return 8;
                }
                verified_bridge_source_text = source_found->second;
                verified_bridge_source_path = bound_bridge_source_text;
            } else {
                // Atomic check-and-open primitive (issue #5409/#5420): the
                // read below is bound to the exact object this walk
                // verifies, never reopened by path string.
                auto bridge_handle =
                    copperfin::security::inspect_and_open_physically_contained_path(
                        *bound_bridge_source,
                        manifest_directory);
                const auto bridge_snapshot =
                    copperfin::security::read_physically_contained_file_snapshot_from_handle_and_revalidate_path(
                        bridge_handle,
                        manifest_directory);
                if (!bridge_snapshot.ok) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.PackagePathPhysicalContainmentFailed",
                        {{"fileName", copperfin::platform::path_to_utf8_string(bound_bridge_source->filename())}}));
                    return 4;
                }
                verified_bridge_source_text = bridge_snapshot.bytes;
                verified_bridge_source_path = copperfin::platform::path_to_utf8_string(*bound_bridge_source);
            }
        }
        return run_runtime_bridge_invocation(
            bridge_options,
            startup_source,
            working_directory,
            catalog,
            verified_startup_bytes,
            verified_bridge_source_text,
            verified_bridge_source_path,
            verified_source_texts,
            verified_file_bytes,
            security_enabled && !debug_manifest_privileges);
    }

    const std::string startup_extension = lowercase_copy(copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).extension()));
    const bool prg_startup = startup_extension == ".prg";
    copperfin::runtime::XAssetExecutableModel xasset_model;

    const auto print_runtime_summary = [&]() {
        std::cout << "status: ok\n";
        std::cout << "project.title: " << first_value(manifest, "project_title") << "\n";
        std::cout << "startup.item: " << first_value(manifest, "startup_item") << "\n";
        std::cout << "startup.source: " << startup_source << "\n";
        std::cout << "working.directory: " << working_directory << "\n";
        std::cout << "security.enabled: " << first_value(manifest, "security_enabled") << "\n";
        std::cout << "security.role: " << security_role << "\n";
        std::cout << "security.audit_log_path: " << (security_enabled ? audit_log_path : std::string{}) << "\n";
        std::cout << "security.mode: " << first_value(manifest, "security_mode") << "\n";
        std::cout << "dotnet.story: " << first_value(manifest, "dotnet_story") << "\n";
        std::cout << "data.policy: " << first_value(manifest, "data_policy") << "\n";
        std::cout << "data.asset_count: " << all_values(manifest, "data_asset").size() << "\n";
        std::cout << "data.payload_count: " << all_values(manifest, "data_payload").size() << "\n";
        std::cout << "asset.count: " << assets.size() << "\n";
        std::cout << "warning.count: " << warnings.size() << "\n";
    };

    std::string effective_startup_source = startup_source;
    std::string runtime_mode = "prg-engine";
    std::string xasset_bootstrap_source;
    std::optional<std::string> xasset_bootstrap_path;
    std::filesystem::path xasset_file_snapshot_root;
    std::string xasset_execution_asset_path;
    struct ScopedXAssetBootstrapCleanup {
        std::optional<std::string>* bootstrap_path = nullptr;

        ~ScopedXAssetBootstrapCleanup() {
            if (bootstrap_path != nullptr) {
                remove_xasset_bootstrap(*bootstrap_path);
            }
        }
    } xasset_bootstrap_cleanup{&xasset_bootstrap_path};
    struct ScopedXAssetFileSnapshotCleanup {
        std::filesystem::path* snapshot_root = nullptr;

        ~ScopedXAssetFileSnapshotCleanup() {
            if (snapshot_root != nullptr) {
                remove_xasset_file_snapshot(*snapshot_root);
            }
        }
    } xasset_file_snapshot_cleanup{&xasset_file_snapshot_root};
    if (!prg_startup) {
        std::string startup_read_path = startup_source;
        if (verified_startup_bytes.has_value()) {
            const auto file_snapshot = materialize_xasset_file_snapshot(
                startup_source,
                *verified_startup_bytes,
                manifest_directory,
                catalog,
                security_enabled,
                verified_package_paths);
            xasset_file_snapshot_root = file_snapshot.root;
            if (!file_snapshot.ok) {
                std::cout << "status: error\n";
                print_error_line(catalog, file_snapshot.error);
                return 4;
            }
            startup_read_path = copperfin::platform::path_to_utf8_string(file_snapshot.primary_path);
            xasset_execution_asset_path = startup_read_path;
        }
        const auto bootstrap = materialize_xasset_bootstrap(
            startup_read_path,
            startup_source,
            true,
            catalog);
        xasset_model = bootstrap.model;
        if (!bootstrap.bootstrap_path.has_value()) {
            print_runtime_summary();
            std::cout << "runtime.mode: compatibility-launcher\n";
            std::cout << "launch.note: " << localized_message(catalog, "RuntimeHost.Launch.Note.CompatibilityLauncher") << "\n";
            std::cout << "launch.note: " << bootstrap.error << "\n";
            std::cout << "debug.breakpoint_support: false\n";
            std::cout << "debug.step_support: false\n";
            return 0;
        }
        effective_startup_source = *bootstrap.bootstrap_path;
        xasset_bootstrap_path = bootstrap.bootstrap_path;
        xasset_bootstrap_source = bootstrap.bootstrap_source;
        runtime_mode = "xasset-bootstrap";
    }

    copperfin::runtime::RuntimeSessionOptions session_options{};
    session_options.startup_path = effective_startup_source;
    session_options.localization_catalog =
        std::make_shared<copperfin::localization::LocalizedCatalog>(catalog);
    if (prg_startup) {
        session_options.startup_source_text = verified_startup_bytes;
        session_options.source_text_overrides = verified_source_texts;
        session_options.require_source_text_overrides = security_enabled && !debug_manifest_privileges;
    } else if (!xasset_bootstrap_source.empty()) {
        session_options.startup_source_text = xasset_bootstrap_source;
    }
    session_options.verified_file_byte_overrides = verified_file_bytes;
    session_options.require_verified_file_byte_overrides = security_enabled && !debug_manifest_privileges;
    if (!xasset_execution_asset_path.empty()) {
        session_options.source_path_display_aliases.emplace(
            copperfin::platform::path_to_utf8_string(
                path_from_utf8(xasset_execution_asset_path).lexically_normal()),
            startup_source);
    }
    session_options.working_directory = working_directory;
    session_options.stop_on_entry = debug_mode && (debug_stop_on_entry || debug_server_mode);
    const std::string quit_confirm_prompt = localized_message_or_default(
        catalog,
        "RuntimeHost.Prompt.QuitConfirm",
        "Do you want to quit this application? [y/N]: ",
        {{"yesToken", "y"}, {"defaultNoToken", "N"}});
    session_options.quit_confirm_callback = [quit_confirm_prompt]() -> bool {
            std::cerr << "\n" << quit_confirm_prompt;
            std::cerr.flush();
            std::string answer;
            if (!std::getline(std::cin, answer)) {
                return true;  // EOF or non-interactive stdin — allow quit
            }
            return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
        };
    std::optional<copperfin::runtime::PrgRuntimeSession> created_session;
    try {
        created_session.emplace(copperfin::runtime::PrgRuntimeSession::create(session_options));
    } catch (const std::exception&) {
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            localized_message(
                catalog,
                "RuntimeHost.Error.VerifiedSourceUnavailable",
                {{"fileName", copperfin::platform::path_to_utf8_string(path_from_utf8(startup_source).filename())}}));
        return security_enabled ? 8 : 4;
    }
    auto& session = *created_session;
    for (const auto& breakpoint_arg : breakpoint_args) {
        const auto breakpoint = parse_breakpoint(breakpoint_arg, effective_startup_source);
        if (!breakpoint.has_value()) {
            std::cout << "status: error\n";
            print_error_line(
                catalog,
                localized_message(
                    catalog,
                    "RuntimeHost.Debug.Error.InvalidBreakpointCommand",
                    {{"command", breakpoint_arg}}));
            return 5;
        }
        session.add_breakpoint(*breakpoint);
    }

    print_runtime_summary();
    std::cout << "runtime.mode: " << runtime_mode << "\n";
    std::cout << "debug.breakpoint_support: true\n";
    std::cout << "debug.step_support: true\n";

    copperfin::runtime::RuntimePauseState state;
    if (debug_server_mode) {
        if (!debug_commands.empty()) {
            std::cout << "status: error\n";
            print_error_line(catalog, localized_message(catalog, "RuntimeHost.Debug.Error.InvalidCommand", {{"command", "--debug-command"}}));
            return 2;
        }

        std::cout << "debug.server.protocol: 1\n";
        std::cout << "debug.server.ready: true\n";
        std::cout.flush();

        std::size_t server_command_index = 0;
        int server_exit_code = 0;
        bool server_requested_exit = false;
        std::string server_command;
        while (std::getline(std::cin, server_command)) {
            server_command = trim_copy(server_command);
            std::cout << "debug.response.begin\n";

            bool command_completed = false;
            bool response_has_state_output = false;
            if (equals_insensitive(server_command, "exit") ||
                equals_insensitive(server_command, "quit") ||
                equals_insensitive(server_command, "stop")) {
                std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                std::cout << "debug.exit: true\n";
                server_requested_exit = true;
                command_completed = true;
            } else if (security_enabled && !copperfin::security::role_has_permission(security_profile, security_role, "runtime.admin")) {
                if (!audit_log_path.empty()) {
                    (void)append_audit_event(
                        "policy.denied",
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.SecurityPolicyDenied",
                            {{"permission", "runtime.admin"}, {"role", security_role}}));
                }
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.SecurityPolicyDenied",
                        {{"permission", "runtime.admin"}, {"role", security_role}}));
                std::cout << "debug.response.error: true\n";
                server_exit_code = 9;
            } else if (starts_with_insensitive(server_command, "select:") || starts_with_insensitive(server_command, "invoke:")) {
                const auto action_routine = resolve_action_routine_name(xasset_model, server_command);
                if (!action_routine.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownXAssetAction",
                            {{"command", server_command}}));
                    std::cout << "debug.response.error: true\n";
                } else if (!session.dispatch_event_handler(*action_routine)) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.DispatchXAssetActionFailed",
                            {{"command", server_command}}));
                    std::cout << "debug.response.error: true\n";
                } else {
                    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
                    command_completed = true;
                }
            } else if (starts_with_insensitive(server_command, "watch:")) {
                if (!state.paused || state.completed) {
                    std::cout << "status: error\n";
                    print_error_line(catalog, localized_message(catalog, "RuntimeHost.Debug.Error.WatchRequiresPausedState"));
                    std::cout << "debug.response.error: true\n";
                } else {
                    const auto watch = session.evaluate_watch_expression(server_command.substr(6U));
                    std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                    std::cout << "debug.watch.expression: " << watch.expression << "\n";
                    std::cout << "debug.watch.ok: " << (watch.ok ? "true" : "false") << "\n";
                    if (watch.ok) {
                        std::cout << "debug.watch.value: "
                                  << escape_debug_line_value(copperfin::runtime::format_value(watch.value)) << "\n";
                    } else {
                        std::cout << "debug.watch.error: " << escape_debug_line_value(watch.message) << "\n";
                    }
                    const auto breakpoints = session.list_breakpoints();
                    print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
                    response_has_state_output = true;
                    command_completed = true;
                }
            } else if (starts_with_insensitive(server_command, "break:add:")) {
                const auto breakpoint = parse_breakpoint(server_command.substr(10U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidBreakpointCommand",
                            {{"command", server_command}}));
                    std::cout << "debug.response.error: true\n";
                } else {
                    session.add_breakpoint(*breakpoint);
                    std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                    print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                    response_has_state_output = true;
                    command_completed = true;
                }
            } else if (starts_with_insensitive(server_command, "break:remove:")) {
                const auto breakpoint = parse_breakpoint(server_command.substr(13U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidBreakpointCommand",
                            {{"command", server_command}}));
                    std::cout << "debug.response.error: true\n";
                } else if (!session.remove_breakpoint(*breakpoint)) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownBreakpoint",
                            {{"path", breakpoint->file_path}, {"line", std::to_string(breakpoint->line)}}));
                    std::cout << "debug.response.error: true\n";
                } else {
                    std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                    print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                    response_has_state_output = true;
                    command_completed = true;
                }
            } else if (lowercase_copy(server_command) == "break:clear" || lowercase_copy(server_command) == "break:list") {
                if (lowercase_copy(server_command) == "break:clear") {
                    session.clear_breakpoints();
                }
                std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                response_has_state_output = true;
                command_completed = true;
            } else {
                const auto action = parse_resume_action(server_command);
                if (!action.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidCommand",
                            {{"command", server_command}}));
                    std::cout << "debug.response.error: true\n";
                } else {
                    state = session.run(*action);
                    command_completed = true;
                }
            }

            if (command_completed && !server_requested_exit && !response_has_state_output) {
                std::cout << "debug.command[" << server_command_index << "]: " << server_command << "\n";
                const auto breakpoints = session.list_breakpoints();
                print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
                if (state.reason == copperfin::runtime::DebugPauseReason::error) {
                    std::cout << "status: error\n";
                    print_error_line(catalog, localized_message(catalog, state.message));
                }
            }
            std::cout << "debug.response.end\n";
            std::cout.flush();
            ++server_command_index;
            if (server_requested_exit || server_exit_code != 0) {
                break;
            }
        }

        if (server_exit_code != 0) {
            return server_exit_code;
        }
        return 0;
    } else if (!debug_mode) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    } else if (debug_commands.empty()) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto breakpoints = session.list_breakpoints();
        print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
        if (state.reason == copperfin::runtime::DebugPauseReason::error) {
            std::cout << "status: error\n";
            print_error_line(catalog, localized_message(catalog, state.message));
        }
    } else {
        for (std::size_t index = 0; index < debug_commands.size(); ++index) {
            const std::string& command = debug_commands[index];
            if (security_enabled && !copperfin::security::role_has_permission(security_profile, security_role, "runtime.admin")) {
                if (!audit_log_path.empty()) {
                    (void)append_audit_event(
                        "policy.denied",
                        localized_message(
                            catalog,
                            "RuntimeHost.Error.SecurityPolicyDenied",
                            {
                                {"permission", "runtime.admin"},
                                {"role", security_role}
                            }));
                }
                std::cout << "status: error\n";
                print_error_line(
                    catalog,
                    localized_message(
                        catalog,
                        "RuntimeHost.Error.SecurityPolicyDenied",
                        {{"permission", "runtime.admin"}, {"role", security_role}}));
                return 9;
            }

            if (starts_with_insensitive(command, "select:") || starts_with_insensitive(command, "invoke:")) {
                const auto action_routine = resolve_action_routine_name(xasset_model, command);
                if (!action_routine.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownXAssetAction",
                            {{"command", command}}));
                    return 5;
                }
                if (!session.dispatch_event_handler(*action_routine)) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.DispatchXAssetActionFailed",
                            {{"command", command}}));
                    return 5;
                }
                state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            } else if (starts_with_insensitive(command, "watch:")) {
                if (!state.paused || state.completed) {
                    std::cout << "status: error\n";
                    print_error_line(catalog, localized_message(catalog, "RuntimeHost.Debug.Error.WatchRequiresPausedState"));
                    return 5;
                }
                const auto watch = session.evaluate_watch_expression(command.substr(6U));
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                std::cout << "debug.watch.expression: "
                          << escape_debug_line_value(watch.expression) << "\n";
                std::cout << "debug.watch.ok: " << (watch.ok ? "true" : "false") << "\n";
                if (watch.ok) {
                    std::cout << "debug.watch.value: "
                              << escape_debug_line_value(copperfin::runtime::format_value(watch.value)) << "\n";
                } else {
                    std::cout << "debug.watch.error: " << escape_debug_line_value(watch.message) << "\n";
                }
                const auto breakpoints = session.list_breakpoints();
                print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:add:")) {
                const auto breakpoint = parse_breakpoint(command.substr(10U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidBreakpointCommand",
                            {{"command", command}}));
                    return 5;
                }
                session.add_breakpoint(*breakpoint);
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:remove:")) {
                const auto breakpoint = parse_breakpoint(command.substr(13U), effective_startup_source);
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidBreakpointCommand",
                            {{"command", command}}));
                    return 5;
                }
                if (!session.remove_breakpoint(*breakpoint)) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownBreakpoint",
                            {{"path", breakpoint->file_path}, {"line", std::to_string(breakpoint->line)}}));
                    return 5;
                }
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:add-action:")) {
                if (runtime_mode != "xasset-bootstrap") {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(catalog, "RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode"));
                    return 5;
                }
                const auto breakpoint = resolve_action_breakpoint(
                    xasset_model,
                    effective_startup_source,
                    xasset_bootstrap_source,
                    command.substr(17U));
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction",
                            {{"action", trim_copy(command.substr(17U))}}));
                    return 5;
                }
                session.add_breakpoint(*breakpoint);
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (starts_with_insensitive(command, "break:remove-action:")) {
                if (runtime_mode != "xasset-bootstrap") {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(catalog, "RuntimeHost.Debug.Error.XAssetActionBreakpointsRequireBootstrapMode"));
                    return 5;
                }
                const auto breakpoint = resolve_action_breakpoint(
                    xasset_model,
                    effective_startup_source,
                    xasset_bootstrap_source,
                    command.substr(20U));
                if (!breakpoint.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownOrNonBreakpointableXAssetAction",
                            {{"action", trim_copy(command.substr(20U))}}));
                    return 5;
                }
                if (!session.remove_breakpoint(*breakpoint)) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.UnknownBreakpointForXAssetAction",
                            {{"action", trim_copy(command.substr(20U))}}));
                    return 5;
                }
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (lowercase_copy(trim_copy(command)) == "break:clear") {
                session.clear_breakpoints();
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else if (lowercase_copy(trim_copy(command)) == "break:list") {
                std::cout << "debug.command[" << index << "]: " << command << "\n";
                print_breakpoint_inventory(session, &xasset_model, effective_startup_source, xasset_bootstrap_source);
                continue;
            } else {
                const auto action = parse_resume_action(command);
                if (!action.has_value()) {
                    std::cout << "status: error\n";
                    print_error_line(
                        catalog,
                        localized_message(
                            catalog,
                            "RuntimeHost.Debug.Error.InvalidCommand",
                            {{"command", command}}));
                    return 5;
                }
                state = session.run(*action);
            }
            std::cout << "debug.command[" << index << "]: " << command << "\n";
            const auto breakpoints = session.list_breakpoints();
            print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
            if (state.reason == copperfin::runtime::DebugPauseReason::error) {
                std::cout << "status: error\n";
                print_error_line(catalog, localized_message(catalog, state.message));
            }
            if (state.completed) {
                break;
            }
        }
    }

    if (!debug_mode) {
        std::cout << "runtime.completed: " << (state.completed ? "true" : "false") << "\n";
        std::cout << "runtime.waiting_for_events: " << (state.waiting_for_events ? "true" : "false") << "\n";
        std::cout << "runtime.reason: " << copperfin::runtime::debug_pause_reason_name(state.reason) << "\n";
        std::cout << "runtime.executed.statements: " << state.executed_statement_count << "\n";
    }

    if (security_enabled && !audit_log_path.empty()) {
        (void)append_audit_event(
            "runtime.complete",
            std::string("completed=") + (state.completed ? "true" : "false") + ",reason=" + copperfin::runtime::debug_pause_reason_name(state.reason));
    }

    return state.reason == copperfin::runtime::DebugPauseReason::error ? 5 : 0;
}

int report_contained_runtime_host_fault(
    int argc,
    char** argv,
    const std::string& detail) noexcept {
    try {
        const std::filesystem::path invocation_path =
            argc > 0 && argv[0] != nullptr ? path_from_utf8(argv[0]) : std::filesystem::path();
        const std::filesystem::path running_executable_path =
            copperfin::platform::resolve_running_executable_path(invocation_path);
        const std::string explicit_locale = explicit_locale_from_arguments(argc, argv);
        const auto catalog = load_localization(running_executable_path, explicit_locale);
        std::cout << "status: error\n";
        print_error_line(
            catalog,
            localized_message(
                catalog,
                "RuntimeHost.Error.UnhandledFault",
                {{"detail", detail}}));
    } catch (...) {
        // Preserve the machine-readable failure boundary even if localization
        // or diagnostic formatting is unavailable during fault recovery.
        std::cout << "status: error\n";
        std::cout << "error: RuntimeHost.Error.UnhandledFault\n";
    }
    return 5;
}

int run_runtime_host_main(int argc, char** argv) noexcept {
    try {
        return run_runtime_host_main_impl(argc, argv);
    } catch (const std::bad_alloc&) {
        return report_contained_runtime_host_fault(argc, argv, "out of memory");
    } catch (const std::exception& error) {
        return report_contained_runtime_host_fault(argc, argv, error.what());
    } catch (...) {
        return report_contained_runtime_host_fault(argc, argv, "unknown exception");
    }
}

#if defined(_WIN32)
int wmain(int argc, wchar_t* argv[]) {
    std::vector<std::string> utf8_arguments;
    std::vector<char*> narrow_arguments;
    utf8_arguments.reserve(static_cast<std::size_t>(argc));
    narrow_arguments.reserve(static_cast<std::size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        utf8_arguments.push_back(copperfin::platform::path_to_utf8_string(
            std::filesystem::path(argv[index])));
        narrow_arguments.push_back(utf8_arguments.back().data());
    }
    return run_runtime_host_main(argc, narrow_arguments.data());
}
#else
int main(int argc, char** argv) {
    return run_runtime_host_main(argc, argv);
}
#endif
