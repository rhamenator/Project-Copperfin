// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/licensing/license_status.h"
#include "copperfin/localization/localization.h"
#include "copperfin/platform/environment.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/runtime/xasset_methods.h"
#include "copperfin/platform/federation_execution.h"
#include "copperfin/security/audit_stream.h"
#include "copperfin/security/authorization.h"
#include "copperfin/security/process_hardening.h"
#include "copperfin/security/security_model.h"
#include "copperfin/security/sha256.h"
#include "copperfin/studio/document_model.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#if defined(_WIN32)
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

void set_environment_value(const char* name, const std::string& value) {
    if (name == nullptr || *name == '\0') {
        return;
    }
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
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

bool parse_bool(const std::string& value) {
    const std::string normalized = lowercase_copy(trim_copy(value));
    return normalized == "1" || normalized == "true" || normalized == "yes";
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

std::optional<std::string> parse_json_string_at(
    const std::string& document,
    std::size_t value_start,
    std::size_t& value_end);

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
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                default:
                    result.push_back(escaped);
                    break;
            }
            continue;
        }
        if (ch == '"') {
            value_end = index + 1U;
            return result;
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

void print_license_status(const copperfin::licensing::LicenseStatus& status) {
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
        std::cout << "diagnostic: " << status.diagnostic << "\n";
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
    std::string& error_message) {
    const std::string export_name = trim_copy(options.library_export);
    if (!is_runtime_bridge_routine_identifier(export_name)) {
        error_message = localized_message(catalog, "RuntimeHost.Bridge.Error.UnsupportedRoutineExportName");
        return std::nullopt;
    }

    std::ifstream source_input(options.source_path, std::ios::binary);
    if (!source_input.good()) {
        error_message = localized_message(catalog, "RuntimeHost.Bridge.Error.SourceArtifactNotFound");
        return std::nullopt;
    }
    std::ostringstream source_stream;
    source_stream << source_input.rdbuf();
    const std::string source_text = source_stream.str();

    const std::string bootstrap_key = options.source_path + "|" + options.request_path + "|" + export_name;
    const std::filesystem::path bootstrap_path =
        std::filesystem::temp_directory_path() /
        ("copperfin_bridge_" + export_name + "_" + std::to_string(std::hash<std::string>{}(bootstrap_key)) + ".prg");
    std::ofstream bootstrap_output(bootstrap_path, std::ios::binary | std::ios::trunc);
    bootstrap_output << "DO " << export_name;
    if (!parameter_values.empty()) {
        bootstrap_output << " WITH ";
        for (std::size_t index = 0; index < parameter_values.size(); ++index) {
            if (index > 0U) {
                bootstrap_output << ", ";
            }
            bootstrap_output << parameter_values[index];
        }
    }
    bootstrap_output << "\n";
    bootstrap_output << source_text;
    if (!source_text.empty() && source_text.back() != '\n') {
        bootstrap_output << "\n";
    }
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
    const copperfin::localization::LocalizedCatalog& catalog) {
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

    std::ifstream request_input(options.request_path, std::ios::binary);
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
        const auto bootstrap_path = materialize_runtime_bridge_routine_bootstrap(options, parameter_values, catalog, bootstrap_error);
        if (!bootstrap_path.has_value()) {
            std::cout << "status: error\n";
            std::cout << "runtime.mode: bridge-invocation\n";
            print_error_line(catalog, bootstrap_error);
            return 6;
        }
        execution_source = bootstrap_path->string();
        routine_bootstrap_path = *bootstrap_path;
        routine_bootstrap_materialized = true;
    }
    if (lowercase_copy(std::filesystem::path(execution_source).extension().string()) != ".prg") {
        std::cout << "status: error\n";
        std::cout << "runtime.mode: bridge-invocation\n";
        print_error_line(catalog, localized_message(catalog, "RuntimeHost.Bridge.Error.PrgStartupRequired"));
        return 6;
    }

    copperfin::runtime::RuntimeSessionOptions session_options{};
    session_options.startup_path = execution_source;
    session_options.working_directory = working_directory;
    session_options.stop_on_entry = false;
    session_options.quit_confirm_callback = []() -> bool {
        return true;
    };
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);
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

    const std::filesystem::path response_path(options.response_path);
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
constexpr int kMinimumSupportedManifestVersion = 1;
constexpr int kMaximumSupportedManifestVersion = 2;

enum class PackagePathBindingMode {
    allow_filename_fallback,
    strict_relative_fidelity
};

ManifestMap load_manifest(const std::string& path) {
    ManifestMap values;
    std::ifstream input(path, std::ios::binary);
    std::string line;
    while (std::getline(input, line)) {
        const auto delimiter = line.find('=');
        if (delimiter == std::string::npos) {
            continue;
        }

        const std::string key = trim_copy(line.substr(0U, delimiter));
        const std::string value = trim_copy(unescape_manifest_value(line.substr(delimiter + 1U)));
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

bool validate_manifest_version(
    const ManifestMap& manifest,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error) {
    const std::string raw_version = first_value(manifest, "manifest_version");
    if (trim_copy(raw_version).empty()) {
        error = localized_message(catalog, "RuntimeHost.Error.ManifestVersionMissing");
        return false;
    }

    const auto parsed_version = parse_manifest_version_value(raw_version);
    if (!parsed_version.has_value() ||
        *parsed_version < kMinimumSupportedManifestVersion ||
        *parsed_version > kMaximumSupportedManifestVersion) {
        error = localized_message(
            catalog,
            "RuntimeHost.Error.ManifestVersionUnsupported",
            {
                {"supportedVersions", "1, 2"},
                {"version", raw_version}
            });
        return false;
    }

    return true;
}

std::vector<std::string> split_pipe(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    for (const char ch : value) {
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

std::optional<std::filesystem::path> bind_packaged_path(
    const std::string& manifest_value,
    const std::string& recorded_package_root,
    const std::filesystem::path& manifest_directory,
    const PackagePathBindingMode binding_mode = PackagePathBindingMode::allow_filename_fallback) {
    if (trim_copy(manifest_value).empty()) {
        return std::nullopt;
    }

    const std::filesystem::path recorded_path(manifest_value);
    if (std::filesystem::exists(recorded_path)) {
        return recorded_path.lexically_normal();
    }

    if (recorded_path.is_relative()) {
        const std::filesystem::path relative_candidate =
            (manifest_directory / recorded_path).lexically_normal();
        if (std::filesystem::exists(relative_candidate)) {
            return relative_candidate;
        }
    }

    if (!trim_copy(recorded_package_root).empty()) {
        const std::filesystem::path package_root(recorded_package_root);
        const std::filesystem::path relative =
            recorded_path.lexically_relative(package_root);
        if (!relative.empty() &&
            relative != recorded_path &&
            !relative_path_escapes_root(relative)) {
            const std::filesystem::path rebound =
                (manifest_directory / relative).lexically_normal();
            if (std::filesystem::exists(rebound)) {
                return rebound;
            }
        }
    }

    if (binding_mode == PackagePathBindingMode::allow_filename_fallback) {
        const std::filesystem::path filename_candidate =
            (manifest_directory / recorded_path.filename()).lexically_normal();
        if (std::filesystem::exists(filename_candidate)) {
            return filename_candidate;
        }
    }

    return std::nullopt;
}

std::string resolve_manifest_bound_directory(
    const ManifestMap& manifest,
    const std::string& key,
    const std::filesystem::path& manifest_directory,
    const std::filesystem::path& fallback_relative_path) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    if (const auto bound = bind_packaged_path(first_value(manifest, key), recorded_package_root, manifest_directory)) {
        return bound->string();
    }

    const std::filesystem::path fallback_path =
        (manifest_directory / fallback_relative_path).lexically_normal();
    return fallback_path.string();
}

bool verify_manifest_hashes(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory,
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string& error) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string expected_runtime_host_hash = first_value(manifest, "runtime_host_sha256");
    if (expected_runtime_host_hash.empty()) {
        error = localized_message(catalog, "RuntimeHost.Error.ManifestMissingRuntimeHostSha256");
        return false;
    }

    const auto runtime_host_hash = copperfin::security::sha256_hex_for_file(
        (manifest_directory / "copperfin_runtime_host.exe").string());
    if (!runtime_host_hash.ok) {
        error = runtime_host_hash.error;
        return false;
    }
    if (lowercase_copy(runtime_host_hash.hex_digest) != lowercase_copy(expected_runtime_host_hash)) {
        error = localized_message(catalog, "RuntimeHost.Error.RuntimeHostSha256Mismatch");
        return false;
    }

    const auto payload_values = all_values(manifest, "extension_payload");
    for (const auto& payload : payload_values) {
        const auto parts = split_pipe(payload);
        if (parts.size() != 2U) {
            error = localized_message(catalog, "RuntimeHost.Error.ExtensionPayloadMalformed");
            return false;
        }

        const auto bound_payload_path = bind_packaged_path(
            parts[0],
            recorded_package_root,
            manifest_directory,
            PackagePathBindingMode::strict_relative_fidelity);
        if (!bound_payload_path.has_value()) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.ExtensionPayloadMissingFromPackage",
                {{"fileName", std::filesystem::path(parts[0]).filename().string()}});
            return false;
        }

        const auto digest = copperfin::security::sha256_hex_for_file(bound_payload_path->string());
        if (!digest.ok) {
            error = digest.error;
            return false;
        }
        if (lowercase_copy(digest.hex_digest) != lowercase_copy(parts[1])) {
            error = localized_message(
                catalog,
                "RuntimeHost.Error.ExtensionPayloadSha256Mismatch",
                {{"fileName", bound_payload_path->filename().string()}});
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
        if (std::string(argv[index]) == "--locale") {
            return argv[index + 1];
        }
    }
    return {};
}

copperfin::localization::LocalizedCatalog load_localization(
    const char* executable_path,
    const std::string& explicit_locale) {
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root(executable_path);
    if (trim_copy(copperfin::platform::read_environment_variable_or_empty("COPPERFIN_LOCALE_DIR")).empty()) {
        set_environment_value("COPPERFIN_LOCALE_DIR", locale_root.string());
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
        {"federationBackendOption", "--federation-backend"},
        {"federationBackendValue", "<sqlite|postgresql|sqlserver|oracle>"},
        {"federationQueryOption", "--federation-query"},
        {"federationQueryValue", "<fox-sql>"},
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

copperfin::runtime::DebugResumeAction parse_resume_action(const std::string& value) {
    const std::string normalized = lowercase_copy(value);
    if (normalized == "step") {
        return copperfin::runtime::DebugResumeAction::step_into;
    }
    if (normalized == "next") {
        return copperfin::runtime::DebugResumeAction::step_over;
    }
    if (normalized == "out") {
        return copperfin::runtime::DebugResumeAction::step_out;
    }
    return copperfin::runtime::DebugResumeAction::continue_run;
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
            std::cout << "debug.frame[" << index << "].local." << name << ": " << copperfin::runtime::format_value(value) << "\n";
        }
    }
    for (const auto& [name, value] : state.globals) {
        std::cout << "debug.global." << name << ": " << copperfin::runtime::format_value(value) << "\n";
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
    std::string startup_stem = startup_path.stem().string();
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
    std::filesystem::remove(*bootstrap_path, ignored);
}

XAssetBootstrapResult materialize_xasset_bootstrap(
    const std::string& startup_source,
    bool include_read_events,
    const copperfin::localization::LocalizedCatalog& catalog) {
    XAssetBootstrapResult result;
    copperfin::studio::StudioOpenRequest request{};
    request.path = startup_source;
    request.read_only = true;
    request.load_full_table = true;
    const auto open_result = copperfin::studio::open_document(request);
    if (!open_result.ok) {
        result.error = open_result.error;
        return result;
    }

    result.model = copperfin::runtime::build_xasset_executable_model(open_result.document);
    if (!result.model.ok || !result.model.runnable_startup) {
        result.error = result.model.error.empty()
            ? localized_message(catalog, "RuntimeHost.Debug.Error.NoRunnableStartupMethodsFound")
            : result.model.error;
        return result;
    }

    const std::filesystem::path startup_path(startup_source);
    const std::filesystem::path bootstrap_path = make_runtime_host_xasset_bootstrap_path(startup_path);
    result.bootstrap_source =
        copperfin::runtime::build_xasset_bootstrap_source(result.model, include_read_events);

    std::ofstream output(bootstrap_path, std::ios::binary | std::ios::trunc);
    output << result.bootstrap_source;
    output.close();
    if (!output.good()) {
        result.error = localized_message(catalog, "RuntimeHost.Debug.Error.MaterializeXAssetBootstrapFailed");
        return result;
    }

    result.bootstrap_path = bootstrap_path.string();
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
    const std::filesystem::path& manifest_directory) {
    return resolve_manifest_bound_directory(
        manifest,
        "working_directory",
        manifest_directory,
        "content");
}

std::string resolve_effective_audit_log_path(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    return resolve_manifest_bound_directory(
        manifest,
        "audit_log_path",
        manifest_directory,
        "security_audit.log");
}

std::string resolve_startup_source(
    const ManifestMap& manifest,
    const std::filesystem::path& manifest_directory) {
    const std::string recorded_package_root = first_value(manifest, "package_root");
    const std::string startup_source = first_value(manifest, "startup_source");
    if (const auto bound_startup = bind_packaged_path(startup_source, recorded_package_root, manifest_directory)) {
        return bound_startup->string();
    }

    const std::string startup_item = first_value(manifest, "startup_item");
    const std::string content_root = resolve_manifest_bound_directory(
        manifest,
        "content_root",
        manifest_directory,
        "content");
    if (!startup_item.empty() && !content_root.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(content_root) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    const std::string working_directory =
        resolve_effective_working_directory(manifest, manifest_directory);
    if (!startup_item.empty() && !working_directory.empty()) {
        const std::filesystem::path candidate =
            (std::filesystem::path(working_directory) / startup_item).lexically_normal();
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
    }

    return startup_source;
}

std::string resolve_implicit_manifest_path(const char* argv0) {
    if (argv0 == nullptr || *argv0 == '\0') {
        return {};
    }

    std::error_code path_error;
    std::filesystem::path executable_path(argv0);
    if (executable_path.is_relative()) {
        executable_path = std::filesystem::absolute(executable_path, path_error);
        if (path_error) {
            return {};
        }
    }

    const std::filesystem::path manifest_path =
        executable_path.parent_path() / "app.cfmanifest";
    if (!std::filesystem::exists(manifest_path)) {
        return {};
    }
    return manifest_path.lexically_normal().string();
}

}  // namespace

int main(int argc, char** argv) {
    const std::string explicit_locale = explicit_locale_from_arguments(argc, argv);
    const copperfin::localization::LocalizedCatalog catalog =
        load_localization(argc > 0 ? argv[0] : nullptr, explicit_locale);

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
    bool debug_mode = false;
    bool license_status_requested = false;
    std::vector<std::string> breakpoint_args;
    std::vector<std::string> debug_commands;
    RuntimeBridgeInvocationOptions bridge_options;

    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg == "--license-status") {
            license_status_requested = true;
        } else if (arg == "--manifest" && (index + 1) < argc) {
            manifest_path = argv[++index];
        } else if (arg == "--federation-backend" && (index + 1) < argc) {
            federation_backend = argv[++index];
        } else if (arg == "--federation-query" && (index + 1) < argc) {
            federation_query = argv[++index];
        } else if (arg == "--federation-target" && (index + 1) < argc) {
            federation_target = argv[++index];
        } else if (arg == "--federation-planning-enable" && (index + 1) < argc) {
            federation_planning_enable = parse_bool(argv[++index]);
        } else if (arg == "--federation-planning-require" && (index + 1) < argc) {
            federation_planning_require = parse_bool(argv[++index]);
        } else if (arg == "--federation-planning-audit" && (index + 1) < argc) {
            federation_policy_audit = parse_bool(argv[++index]);
        } else if (arg == "--debug") {
            debug_mode = true;
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
        } else if (arg == "--locale" && (index + 1) < argc) {
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
        print_license_status(copperfin::licensing::load_license_status(argv[0]));
        return 0;
    }

    const bool federation_mode_requested =
        !trim_copy(federation_backend).empty() || !trim_copy(federation_query).empty();
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
            const auto security_profile = copperfin::security::default_native_security_profile();
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
        manifest_path = resolve_implicit_manifest_path(argc > 0 ? argv[0] : nullptr);
        if (manifest_path.empty()) {
            print_usage(catalog);
            return 2;
        }
    }

    if (!std::filesystem::exists(manifest_path)) {
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
    std::string manifest_version_error;
    if (!validate_manifest_version(manifest, catalog, manifest_version_error)) {
        std::cout << "status: error\n";
        print_error_line(catalog, manifest_version_error);
        return 4;
    }

    const std::filesystem::path manifest_directory =
        std::filesystem::path(manifest_path).parent_path().lexically_normal();
    const auto assets = all_values(manifest, "asset");
    const auto warnings = all_values(manifest, "warning");
    const bool security_enabled = parse_bool(first_value(manifest, "security_enabled"));
    const std::string security_role = first_value(manifest, "security_role");
    const std::string audit_log_path =
        resolve_effective_audit_log_path(manifest, manifest_directory);
    const auto security_profile = copperfin::security::default_native_security_profile();

    if (security_enabled) {
        if (!copperfin::security::role_has_permission(security_profile, security_role, "project.open")) {
            if (!audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    audit_log_path,
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
        if (!verify_manifest_hashes(manifest, manifest_directory, catalog, verification_error)) {
            if (!audit_log_path.empty()) {
                (void)copperfin::security::append_immutable_audit_event(
                    audit_log_path,
                    "policy.denied",
                    verification_error);
            }
            std::cout << "status: error\n";
            print_error_line(catalog, verification_error);
            return 8;
        }

        if (!audit_log_path.empty()) {
            (void)copperfin::security::append_immutable_audit_event(
                audit_log_path,
                "runtime.start",
                "role=" + security_role + ",manifest=" + manifest_path);
        }
    }

    const std::string startup_source =
        resolve_startup_source(manifest, manifest_directory);
    const std::string working_directory =
        resolve_effective_working_directory(manifest, manifest_directory);

    if (runtime_bridge_mode_requested(bridge_options)) {
        return run_runtime_bridge_invocation(bridge_options, startup_source, working_directory, catalog);
    }

    const std::string startup_extension = lowercase_copy(std::filesystem::path(startup_source).extension().string());
    const bool prg_startup = startup_extension == ".prg";
    copperfin::runtime::XAssetExecutableModel xasset_model;

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
    std::cout << "asset.count: " << assets.size() << "\n";
    std::cout << "warning.count: " << warnings.size() << "\n";

    std::string effective_startup_source = startup_source;
    std::string runtime_mode = "prg-engine";
    std::string xasset_bootstrap_source;
    std::optional<std::string> xasset_bootstrap_path;
    struct ScopedXAssetBootstrapCleanup {
        std::optional<std::string>* bootstrap_path = nullptr;

        ~ScopedXAssetBootstrapCleanup() {
            if (bootstrap_path != nullptr) {
                remove_xasset_bootstrap(*bootstrap_path);
            }
        }
    } xasset_bootstrap_cleanup{&xasset_bootstrap_path};
    if (!prg_startup) {
        const auto bootstrap = materialize_xasset_bootstrap(startup_source, true, catalog);
        xasset_model = bootstrap.model;
        if (!bootstrap.bootstrap_path.has_value()) {
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
    session_options.working_directory = working_directory;
    session_options.stop_on_entry = false;
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
    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(session_options);
    for (const auto& breakpoint_arg : breakpoint_args) {
        if (const auto breakpoint = parse_breakpoint(breakpoint_arg, effective_startup_source)) {
            session.add_breakpoint(*breakpoint);
        }
    }

    std::cout << "runtime.mode: " << runtime_mode << "\n";
    std::cout << "debug.breakpoint_support: true\n";
    std::cout << "debug.step_support: true\n";

    copperfin::runtime::RuntimePauseState state;
    if (!debug_mode) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    } else if (debug_commands.empty()) {
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        const auto breakpoints = session.list_breakpoints();
        print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
    } else {
        for (std::size_t index = 0; index < debug_commands.size(); ++index) {
            const std::string& command = debug_commands[index];
            if (security_enabled && !copperfin::security::role_has_permission(security_profile, security_role, "runtime.admin")) {
                if (!audit_log_path.empty()) {
                    (void)copperfin::security::append_immutable_audit_event(
                        audit_log_path,
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
                std::cout << "debug.watch.expression: " << watch.expression << "\n";
                std::cout << "debug.watch.ok: " << (watch.ok ? "true" : "false") << "\n";
                if (watch.ok) {
                    std::cout << "debug.watch.value: " << copperfin::runtime::format_value(watch.value) << "\n";
                } else {
                    std::cout << "debug.watch.error: " << watch.message << "\n";
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
                state = session.run(parse_resume_action(command));
            }
            std::cout << "debug.command[" << index << "]: " << command << "\n";
            const auto breakpoints = session.list_breakpoints();
            print_pause_state(state, &xasset_model, &breakpoints, effective_startup_source, xasset_bootstrap_source);
            if (state.completed || state.reason == copperfin::runtime::DebugPauseReason::error) {
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
        (void)copperfin::security::append_immutable_audit_event(
            audit_log_path,
            "runtime.complete",
            std::string("completed=") + (state.completed ? "true" : "false") + ",reason=" + copperfin::runtime::debug_pause_reason_name(state.reason));
    }

    return state.reason == copperfin::runtime::DebugPauseReason::error ? 5 : 0;
}
