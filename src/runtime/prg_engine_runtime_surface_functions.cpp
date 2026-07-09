// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "prg_engine_runtime_surface_functions.h"

#include "copperfin/platform/environment.h"
#include "prg_engine_file_io_functions.h"
#include "prg_engine_helpers.h"
#include "localized_text.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <iconv.h>
#include <langinfo.h>
#endif

namespace copperfin::runtime {

namespace {

std::uint32_t bitwise_value(const PrgValue& value) {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(std::llround(value_as_number(value))));
}

std::int64_t signed_bitwise_result(std::uint32_t value) {
    return static_cast<std::int64_t>(static_cast<std::int32_t>(value));
}

int bit_position(const PrgValue& value) {
    const int position = static_cast<int>(std::llround(value_as_number(value)));
    if (position < 0 || position > 31) {
        throw std::runtime_error(runtime_text(
            "Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange",
            {
                {"maximum", "31"},
                {"minimum", "0"}
            }));
    }
    return position;
}

std::string host_os_name() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__unix__)
    return "Unix";
#else
    return "Unknown";
#endif
}

std::filesystem::path filesystem_probe_path(const std::string& raw_path, const std::string& default_directory) {
    std::filesystem::path path(raw_path.empty() ? default_directory : raw_path);
    if (path.is_relative()) {
        path = std::filesystem::path(default_directory) / path;
    }
    return path.lexically_normal();
}

std::string strip_surrounding_quotes(std::string text) {
    text = trim_copy(std::move(text));
    if (text.size() >= 2U) {
        const char first = text.front();
        const char last = text.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
            return text.substr(1U, text.size() - 2U);
        }
    }
    return text;
}

std::optional<int> parse_codeset_to_code_page(std::string codeset) {
    codeset = uppercase_copy(trim_copy(std::move(codeset)));
    if (codeset.empty()) {
        return std::nullopt;
    }

    if (codeset == "UTF-8" || codeset == "UTF8" || codeset == "C.UTF-8") {
        return 65001;
    }
    if (codeset == "US-ASCII" || codeset == "ASCII" || codeset == "ANSI_X3.4-1968" || codeset == "C") {
        return 20127;
    }

    std::string digits;
    digits.reserve(codeset.size());
    for (const unsigned char ch : codeset) {
        if (std::isdigit(ch) != 0) {
            digits.push_back(static_cast<char>(ch));
        }
    }

    if (!digits.empty()) {
        try {
            return std::stoi(digits);
        } catch (const std::exception&) {
        }
    }

    return std::nullopt;
}

int current_host_code_page() {
#if defined(_WIN32)
    const UINT active_code_page = GetACP();
    return active_code_page == 0U ? 1252 : static_cast<int>(active_code_page);
#else
    if (const char* codeset = nl_langinfo(CODESET); codeset != nullptr) {
        if (const auto parsed = parse_codeset_to_code_page(codeset); parsed.has_value()) {
            return *parsed;
        }
    }

    const std::optional<std::string> locale_candidates[] = {
        platform::read_environment_variable("LC_ALL"),
        platform::read_environment_variable("LC_CTYPE"),
        platform::read_environment_variable("LANG"),
    };
    for (const auto& candidate : locale_candidates) {
        if (!candidate.has_value()) {
            continue;
        }
        if (const auto parsed = parse_codeset_to_code_page(*candidate); parsed.has_value()) {
            return *parsed;
        }
    }

    return 1252;
#endif
}

int current_host_oem_code_page() {
#if defined(_WIN32)
    const UINT oem_code_page = GetOEMCP();
    return oem_code_page == 0U ? current_host_code_page() : static_cast<int>(oem_code_page);
#else
    return current_host_code_page();
#endif
}

bool is_supported_vfp_code_page(int code_page) {
    switch (code_page) {
        case 437:
        case 620:
        case 737:
        case 850:
        case 852:
        case 857:
        case 861:
        case 865:
        case 866:
        case 874:
        case 895:
        case 932:
        case 936:
        case 949:
        case 950:
        case 1250:
        case 1251:
        case 1252:
        case 1253:
        case 1254:
        case 1255:
        case 1256:
        case 10000:
        case 10006:
        case 10007:
        case 10029:
            return true;
        default:
            return false;
    }
}

#if defined(_WIN32)
std::optional<std::string> convert_between_host_code_pages(
    int source_code_page,
    int target_code_page,
    const std::string& input) {
    const int wide_count = MultiByteToWideChar(
        static_cast<UINT>(source_code_page),
        0,
        input.data(),
        static_cast<int>(input.size()),
        nullptr,
        0);
    if (wide_count <= 0) {
        return std::nullopt;
    }

    std::wstring wide_text(static_cast<std::size_t>(wide_count), L'\0');
    if (MultiByteToWideChar(
            static_cast<UINT>(source_code_page),
            0,
            input.data(),
            static_cast<int>(input.size()),
            wide_text.data(),
            wide_count) <= 0) {
        return std::nullopt;
    }

    const int byte_count = WideCharToMultiByte(
        static_cast<UINT>(target_code_page),
        0,
        wide_text.data(),
        wide_count,
        nullptr,
        0,
        nullptr,
        nullptr);
    if (byte_count <= 0) {
        return std::nullopt;
    }

    std::string output(static_cast<std::size_t>(byte_count), '\0');
    if (WideCharToMultiByte(
            static_cast<UINT>(target_code_page),
            0,
            wide_text.data(),
            wide_count,
            output.data(),
            byte_count,
            nullptr,
            nullptr) <= 0) {
        return std::nullopt;
    }

    return output;
}
#else
std::optional<std::string> iconv_encoding_name_for_code_page(int code_page) {
    switch (code_page) {
        case 437:
            return "CP437";
        case 620:
            return "CP620";
        case 737:
            return "CP737";
        case 850:
            return "CP850";
        case 852:
            return "CP852";
        case 857:
            return "CP857";
        case 861:
            return "CP861";
        case 865:
            return "CP865";
        case 866:
            return "CP866";
        case 874:
            return "CP874";
        case 895:
            return "CP895";
        case 932:
            return "CP932";
        case 936:
            return "CP936";
        case 949:
            return "CP949";
        case 950:
            return "CP950";
        case 1250:
            return "CP1250";
        case 1251:
            return "CP1251";
        case 1252:
            return "CP1252";
        case 1253:
            return "CP1253";
        case 1254:
            return "CP1254";
        case 1255:
            return "CP1255";
        case 1256:
            return "CP1256";
        case 10000:
            return "MACINTOSH";
        case 10006:
            return "MACGREEK";
        case 10007:
            return "MACCYRILLIC";
        case 10029:
            return "MACCENTRALEUROPE";
        default:
            return std::nullopt;
    }
}

std::optional<std::string> convert_between_host_code_pages(
    int source_code_page,
    int target_code_page,
    const std::string& input) {
    const std::optional<std::string> source_name = iconv_encoding_name_for_code_page(source_code_page);
    const std::optional<std::string> target_name = iconv_encoding_name_for_code_page(target_code_page);
    if (!source_name.has_value() || !target_name.has_value()) {
        return std::nullopt;
    }

    iconv_t converter = iconv_open(target_name->c_str(), source_name->c_str());
    if (converter == reinterpret_cast<iconv_t>(-1)) {
        return std::nullopt;
    }

    std::string output(std::max<std::size_t>(input.size() * 4U, 16U), '\0');
    char* input_buffer = const_cast<char*>(input.data());
    std::size_t input_remaining = input.size();
    char* output_buffer = output.data();
    std::size_t output_remaining = output.size();

    while (true) {
        const std::size_t result = iconv(
            converter,
            &input_buffer,
            &input_remaining,
            &output_buffer,
            &output_remaining);
        if (result != static_cast<std::size_t>(-1)) {
            break;
        }

        if (errno == E2BIG) {
            const std::size_t bytes_written = output.size() - output_remaining;
            output.resize(output.size() * 2U, '\0');
            output_buffer = output.data() + bytes_written;
            output_remaining = output.size() - bytes_written;
            continue;
        }

        iconv_close(converter);
        return std::nullopt;
    }

    iconv_close(converter);
    output.resize(output.size() - output_remaining);
    return output;
}
#endif

std::vector<std::filesystem::path> parse_set_path_entries(const std::string& set_path_value,
                                                          const std::string& default_directory) {
    std::string value = trim_copy(set_path_value);
    if (starts_with_insensitive(value, "TO ")) {
        value = trim_copy(value.substr(3U));
    }
    value = strip_surrounding_quotes(std::move(value));

    std::vector<std::filesystem::path> entries;
    std::size_t token_start = 0U;
    while (token_start <= value.size()) {
        const std::size_t separator = value.find(';', token_start);
        std::string token = separator == std::string::npos
                                ? value.substr(token_start)
                                : value.substr(token_start, separator - token_start);
        token = strip_surrounding_quotes(std::move(token));
        if (!token.empty()) {
            std::filesystem::path entry(token);
            if (entry.is_relative()) {
                entry = std::filesystem::path(default_directory) / entry;
            }
            entries.push_back(entry.lexically_normal());
        }

        if (separator == std::string::npos) {
            break;
        }
        token_start = separator + 1U;
    }

    return entries;
}

std::filesystem::path resolve_runtime_file_probe_path(
    const std::string& raw_path,
    const std::string& default_directory,
    const std::function<std::string(const std::string&)>& set_callback) {
    std::error_code ignored;
    std::filesystem::path path(raw_path.empty() ? default_directory : raw_path);
    if (!path.is_relative()) {
        return path.lexically_normal();
    }

    const std::filesystem::path default_candidate =
        (std::filesystem::path(default_directory) / path).lexically_normal();
    if (std::filesystem::exists(default_candidate, ignored)) {
        return default_candidate;
    }

    const std::vector<std::filesystem::path> set_path_entries =
        parse_set_path_entries(set_callback("PATH"), default_directory);
    for (const auto& entry : set_path_entries) {
        const std::filesystem::path candidate = (entry / path).lexically_normal();
        if (std::filesystem::exists(candidate, ignored)) {
            return candidate;
        }
    }

    return default_candidate;
}

double available_disk_space(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const auto info = std::filesystem::space(filesystem_probe_path(raw_path, default_directory), ignored);
    return ignored ? 0.0 : static_cast<double>(info.available);
}

int drive_type_value(const std::string& raw_path, const std::string& default_directory) {
    std::error_code ignored;
    const std::filesystem::path path = filesystem_probe_path(raw_path, default_directory);
    if (!std::filesystem::exists(path, ignored)) {
        return 0;
    }
    return std::filesystem::is_directory(path, ignored) || std::filesystem::is_regular_file(path, ignored)
               ? 3
               : 1;
}

std::string class_token_from_prog_id(const std::string& prog_id) {
    std::string token = trim_copy(prog_id);
    const std::size_t separator = token.find_last_of('.');
    if (separator != std::string::npos && separator + 1U < token.size()) {
        token = token.substr(separator + 1U);
    }
    token = uppercase_copy(trim_copy(std::move(token)));
    return token.empty() ? "CUSTOM" : token;
}

bool object_has_member(const std::vector<std::string>& members, const std::string& normalized_member_name) {
    return std::find_if(members.begin(), members.end(), [&](const std::string& member) {
               return normalize_identifier(member) == normalized_member_name;
           }) != members.end();
}

bool is_native_visual_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "commandgroup" ||
           normalized_base_class == "container" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "image" ||
           normalized_base_class == "label" ||
           normalized_base_class == "line" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "optiongroup" ||
           normalized_base_class == "page" ||
           normalized_base_class == "pageframe" ||
           normalized_base_class == "separator" ||
           normalized_base_class == "shape" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox" ||
           normalized_base_class == "toolbar";
}

bool is_native_focusable_runtime_object(const RuntimeOleObjectState& runtime_object) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "checkbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "commandbutton" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "form" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "olecontrol" ||
           normalized_base_class == "optionbutton" ||
           normalized_base_class == "page" ||
           normalized_base_class == "spinner" ||
           normalized_base_class == "textbox";
}

bool is_builtin_native_runtime_method_name(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }

    if (normalized_member_name == "release" &&
        !runtime_object.source.empty()) {
        return true;
    }

    if (normalized_member_name == "refresh" ||
        normalized_member_name == "resettodefault") {
        return true;
    }

    if (normalized_member_name == "move" &&
        is_native_visual_runtime_object(runtime_object)) {
        return true;
    }

    if ((normalized_member_name == "show" || normalized_member_name == "hide") &&
        is_native_visual_runtime_object(runtime_object)) {
        return true;
    }

    if (normalized_member_name == "setfocus" &&
        is_native_focusable_runtime_object(runtime_object)) {
        return true;
    }

    if (normalized_member_name == "additem" ||
        normalized_member_name == "addlistitem" ||
        normalized_member_name == "removeitem" ||
        normalized_member_name == "removelistitem") {
        const std::string normalized_base_class =
            normalize_identifier(trim_copy(runtime_object.base_class_name));
        return normalized_base_class == "combobox" ||
               normalized_base_class == "listbox";
    }

    return false;
}

bool is_scripting_dictionary_object(const RuntimeOleObjectState& runtime_object) {
    return normalize_identifier(runtime_object.prog_id) == "scripting.dictionary";
}

bool is_native_list_control_runtime_object(const RuntimeOleObjectState& runtime_object) {
    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool is_native_listbox_runtime_object(const RuntimeOleObjectState& runtime_object) {
    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_list_control_allows_multiple_selection(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return false;
    }

    const auto multiselect = runtime_object.properties.find("multiselect");
    return multiselect != runtime_object.properties.end() &&
           value_as_bool(multiselect->second);
}

bool native_list_control_rowsourcetype_supports_additem(const RuntimeOleObjectState& runtime_object);

bool native_list_control_sorted_enabled(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object) ||
        !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
        return false;
    }

    const auto sorted = runtime_object.properties.find("sorted");
    return sorted != runtime_object.properties.end() &&
           value_as_bool(sorted->second);
}

std::optional<std::size_t> native_list_control_selected_slot(const RuntimeOleObjectState& runtime_object);

bool native_list_control_rowsourcetype_supports_additem(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    const auto rowsourcetype = runtime_object.properties.find("rowsourcetype");
    if (rowsourcetype == runtime_object.properties.end()) {
        return true;
    }

    const long long value = std::llround(value_as_number(rowsourcetype->second));
    return value == 0LL || value == 1LL;
}

void sync_native_list_control_selected_state_size(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const std::size_t row_count =
        runtime_object.list_rows.empty() ? runtime_object.collection_items.size()
                                         : runtime_object.list_rows.size();
    runtime_object.list_selected.resize(row_count, false);
}

std::optional<std::size_t> find_last_native_list_control_selected_slot(
    const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    for (std::size_t index = runtime_object.list_selected.size(); index > 0U; --index) {
        if (runtime_object.list_selected[index - 1U]) {
            return index - 1U;
        }
    }
    return std::nullopt;
}

void sync_native_list_control_selected_state_from_listindex(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    sync_native_list_control_selected_state_size(runtime_object);

    if (!native_list_control_allows_multiple_selection(runtime_object)) {
        std::fill(runtime_object.list_selected.begin(), runtime_object.list_selected.end(), false);
    }

    if (const auto selected_slot = native_list_control_selected_slot(runtime_object);
        selected_slot.has_value() && *selected_slot < runtime_object.list_selected.size()) {
        runtime_object.list_selected[*selected_slot] = true;
    }
}

std::optional<std::size_t> native_list_control_selected_slot(const RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        return std::nullopt;
    }

    const long long selected_index = std::llround(value_as_number(listindex->second));
    if (selected_index < 1LL ||
        static_cast<std::size_t>(selected_index) > runtime_object.collection_items.size()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(selected_index - 1LL);
}

void sync_native_list_control_displayvalue_from_selection_impl(RuntimeOleObjectState& runtime_object) {
    sync_native_list_control_selected_state_from_listindex(runtime_object);

    if (const auto selected_slot = native_list_control_selected_slot(runtime_object);
        selected_slot.has_value()) {
        runtime_object.properties["displayvalue"] =
            make_string_value(value_as_string(runtime_object.collection_items[*selected_slot]));
        if (*selected_slot < runtime_object.collection_item_keys.size()) {
            try {
                runtime_object.properties["listitemid"] = make_number_value(
                    static_cast<double>(std::stoll(runtime_object.collection_item_keys[*selected_slot])));
            } catch (const std::exception&) {
                runtime_object.properties["listitemid"] = make_number_value(0.0);
            }
        } else {
            runtime_object.properties["listitemid"] = make_number_value(0.0);
        }
        return;
    }

    runtime_object.properties["displayvalue"] = make_string_value("");
    runtime_object.properties["listitemid"] = make_number_value(0.0);
}

void sync_native_list_control_count_impl(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    runtime_object.properties["listcount"] =
        make_number_value(static_cast<double>(
            runtime_object.list_rows.empty() ? runtime_object.collection_items.size()
                                             : runtime_object.list_rows.size()));
}

void sync_native_list_control_primary_state_from_rows(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    runtime_object.collection_items.clear();
    runtime_object.collection_items.reserve(runtime_object.list_rows.size());
    for (const auto& row : runtime_object.list_rows) {
        runtime_object.collection_items.push_back(
            row.empty() ? make_string_value("") : row.front());
    }

    if (runtime_object.collection_item_keys.size() > runtime_object.list_rows.size()) {
        runtime_object.collection_item_keys.resize(runtime_object.list_rows.size());
    }
    sync_native_list_control_selected_state_size(runtime_object);
}

void materialize_native_list_control_rows(RuntimeOleObjectState& runtime_object) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    if (!runtime_object.list_rows.empty()) {
        sync_native_list_control_primary_state_from_rows(runtime_object);
        return;
    }

    runtime_object.list_rows.reserve(runtime_object.collection_items.size());
    for (const auto& item : runtime_object.collection_items) {
        runtime_object.list_rows.push_back({item});
    }
    sync_native_list_control_selected_state_size(runtime_object);
}

std::optional<std::size_t> find_native_list_control_row_by_item_id(
    const RuntimeOleObjectState& runtime_object,
    long long item_id) {
    if (!is_native_list_control_runtime_object(runtime_object) ||
        item_id < 1LL) {
        return std::nullopt;
    }

    const std::string expected_key = std::to_string(item_id);
    const auto found = std::find(runtime_object.collection_item_keys.begin(),
                                 runtime_object.collection_item_keys.end(),
                                 expected_key);
    if (found == runtime_object.collection_item_keys.end()) {
        return std::nullopt;
    }

    return static_cast<std::size_t>(
        std::distance(runtime_object.collection_item_keys.begin(), found));
}

std::int64_t next_native_list_control_item_id(const RuntimeOleObjectState& runtime_object) {
    std::int64_t next_id = 1;
    for (const std::string& key : runtime_object.collection_item_keys) {
        if (key.empty()) {
            continue;
        }
        try {
            next_id = std::max(next_id, static_cast<std::int64_t>(std::stoll(key) + 1LL));
        } catch (const std::exception&) {
        }
    }
    return next_id;
}

void sort_native_list_control_rows_if_needed(RuntimeOleObjectState& runtime_object) {
    if (!native_list_control_sorted_enabled(runtime_object)) {
        return;
    }

    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);

    const auto active_slot = native_list_control_selected_slot(runtime_object);
    std::optional<std::size_t> latest_added_slot;
    if (const auto newindex = runtime_object.properties.find("newindex");
        newindex != runtime_object.properties.end()) {
        const long long requested_index = std::llround(value_as_number(newindex->second));
        if (requested_index >= 1LL &&
            static_cast<std::size_t>(requested_index) <= runtime_object.list_rows.size()) {
            latest_added_slot = static_cast<std::size_t>(requested_index - 1LL);
        }
    }

    struct NativeListControlSortEntry {
        std::vector<PrgValue> row;
        std::string item_key;
        bool selected = false;
        bool active = false;
        bool latest_added = false;
    };

    std::vector<NativeListControlSortEntry> entries;
    entries.reserve(runtime_object.list_rows.size());
    for (std::size_t index = 0U; index < runtime_object.list_rows.size(); ++index) {
        entries.push_back(NativeListControlSortEntry{
            .row = runtime_object.list_rows[index],
            .item_key = index < runtime_object.collection_item_keys.size()
                            ? runtime_object.collection_item_keys[index]
                            : std::string{},
            .selected = index < runtime_object.list_selected.size() &&
                        runtime_object.list_selected[index],
            .active = active_slot.has_value() && *active_slot == index,
            .latest_added = latest_added_slot.has_value() && *latest_added_slot == index});
    }

    std::stable_sort(
        entries.begin(),
        entries.end(),
        [](const NativeListControlSortEntry& left, const NativeListControlSortEntry& right) {
            const std::string left_key =
                left.row.empty() ? std::string{} : value_as_string(left.row.front());
            const std::string right_key =
                right.row.empty() ? std::string{} : value_as_string(right.row.front());
            return left_key < right_key;
        });

    runtime_object.list_rows.clear();
    runtime_object.collection_item_keys.clear();
    runtime_object.list_selected.clear();
    runtime_object.list_rows.reserve(entries.size());
    runtime_object.collection_item_keys.reserve(entries.size());
    runtime_object.list_selected.reserve(entries.size());

    bool active_found = false;
    bool latest_added_found = false;
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        auto& entry = entries[index];
        runtime_object.list_rows.push_back(std::move(entry.row));
        runtime_object.collection_item_keys.push_back(std::move(entry.item_key));
        runtime_object.list_selected.push_back(entry.selected);
        if (entry.active) {
            runtime_object.properties["listindex"] =
                make_number_value(static_cast<double>(index + 1U));
            active_found = true;
        }
        if (entry.latest_added) {
            runtime_object.properties["newindex"] =
                make_number_value(static_cast<double>(index + 1U));
            latest_added_found = true;
        }
    }

    if (!active_found) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
    }
    if (!latest_added_found) {
        runtime_object.properties["newindex"] = make_number_value(0.0);
    }

    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
}

bool remove_native_list_control_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t slot) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    if (slot >= runtime_object.collection_items.size()) {
        return false;
    }

    if (slot < runtime_object.list_rows.size()) {
        runtime_object.list_rows.erase(
            runtime_object.list_rows.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    if (slot < runtime_object.collection_item_keys.size()) {
        runtime_object.collection_item_keys.erase(
            runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    if (slot < runtime_object.list_selected.size()) {
        runtime_object.list_selected.erase(
            runtime_object.list_selected.begin() + static_cast<std::ptrdiff_t>(slot));
    }
    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex != runtime_object.properties.end()) {
        long long selected_index = std::llround(value_as_number(listindex->second));
        const long long removed_index = static_cast<long long>(slot + 1U);
        const long long new_count = static_cast<long long>(runtime_object.collection_items.size());
        if (selected_index == removed_index) {
            if (new_count <= 0LL) {
                selected_index = 0LL;
            } else if (selected_index > new_count) {
                selected_index = new_count;
            }
        } else if (selected_index > removed_index) {
            --selected_index;
        } else if (selected_index > new_count) {
            selected_index = std::max(0LL, new_count);
        }
        listindex->second = make_number_value(static_cast<double>(selected_index));
    }

    const auto newindex = runtime_object.properties.find("newindex");
    if (newindex != runtime_object.properties.end()) {
        long long last_added_index = std::llround(value_as_number(newindex->second));
        const long long removed_index = static_cast<long long>(slot + 1U);
        const long long new_count = static_cast<long long>(runtime_object.collection_items.size());
        if (last_added_index == removed_index) {
            last_added_index = 0LL;
        } else if (last_added_index > removed_index) {
            --last_added_index;
        } else if (last_added_index > new_count) {
            last_added_index = std::max(0LL, new_count);
        }
        newindex->second = make_number_value(static_cast<double>(last_added_index));
    }

    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

std::optional<std::size_t> parse_native_list_control_selected_member_slot_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const std::string normalized = normalize_identifier(trim_copy(member_name));
    if (!starts_with_insensitive(normalized, "selected(") || normalized.back() != ')') {
        return std::nullopt;
    }

    const std::size_t open_paren = normalized.find('(');
    if (open_paren == std::string::npos || open_paren + 1U >= normalized.size() - 1U) {
        return std::nullopt;
    }

    try {
        const long long requested_index = std::stoll(
            trim_copy(normalized.substr(open_paren + 1U, normalized.size() - open_paren - 2U)));
        if (requested_index < 1LL) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(requested_index - 1LL);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const std::string normalized = normalize_identifier(trim_copy(member_name));
    if (!starts_with_insensitive(normalized, "list(") || normalized.back() != ')') {
        return std::nullopt;
    }

    const std::size_t open_paren = normalized.find('(');
    if (open_paren == std::string::npos || open_paren + 1U >= normalized.size() - 1U) {
        return std::nullopt;
    }

    const std::string selector_text =
        trim_copy(normalized.substr(open_paren + 1U, normalized.size() - open_paren - 2U));
    if (selector_text.empty()) {
        return std::nullopt;
    }

    const std::size_t comma = selector_text.find(',');
    const std::string row_text = trim_copy(selector_text.substr(0U, comma));
    const std::string column_text =
        comma == std::string::npos ? std::string("1") : trim_copy(selector_text.substr(comma + 1U));
    if (row_text.empty() || column_text.empty()) {
        return std::nullopt;
    }

    try {
        const long long requested_row = std::stoll(row_text);
        const long long requested_column = std::stoll(column_text);
        if (requested_row < 1LL || requested_column < 1LL) {
            return std::nullopt;
        }
        return NativeListControlCellReference{
            .row_slot = static_cast<std::size_t>(requested_row - 1LL),
            .column_slot = static_cast<std::size_t>(requested_column - 1LL)};
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<long long> parse_native_list_control_selectedid_member_item_id_impl(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name) {
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    const std::string normalized = normalize_identifier(trim_copy(member_name));
    if (!starts_with_insensitive(normalized, "selectedid(") || normalized.back() != ')') {
        return std::nullopt;
    }

    const std::size_t open_paren = normalized.find('(');
    if (open_paren == std::string::npos || open_paren + 1U >= normalized.size() - 1U) {
        return std::nullopt;
    }

    try {
        const long long requested_item_id = std::stoll(
            trim_copy(normalized.substr(open_paren + 1U, normalized.size() - open_paren - 2U)));
        if (requested_item_id < 1LL) {
            return std::nullopt;
        }
        return requested_item_id;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::optional<std::size_t> resolve_native_list_control_insert_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::vector<PrgValue>& arguments) {
    if (arguments.size() < 2U) {
        return runtime_object.collection_items.size();
    }

    const long long requested_index = std::llround(value_as_number(arguments[1]));
    if (requested_index < 1LL) {
        return std::nullopt;
    }
    if (static_cast<std::size_t>(requested_index) > runtime_object.collection_items.size()) {
        return runtime_object.collection_items.size();
    }
    return static_cast<std::size_t>(requested_index - 1LL);
}

void sync_native_collection_count(RuntimeOleObjectState& runtime_object) {
    runtime_object.properties["count"] = make_number_value(static_cast<double>(runtime_object.collection_items.size()));
}

std::optional<std::size_t> resolve_native_collection_slot(
    const RuntimeOleObjectState& runtime_object,
    const PrgValue& selector) {
    switch (selector.kind) {
        case PrgValueKind::number: {
            const long long index = std::llround(selector.number_value);
            if (index >= 1LL && static_cast<std::size_t>(index) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(index - 1LL);
            }
            return std::nullopt;
        }
        case PrgValueKind::int64:
            if (selector.int64_value >= 1LL &&
                static_cast<std::size_t>(selector.int64_value) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(selector.int64_value - 1LL);
            }
            return std::nullopt;
        case PrgValueKind::uint64:
            if (selector.uint64_value >= 1ULL &&
                static_cast<std::size_t>(selector.uint64_value) <= runtime_object.collection_items.size()) {
                return static_cast<std::size_t>(selector.uint64_value - 1ULL);
            }
            return std::nullopt;
        case PrgValueKind::boolean:
        case PrgValueKind::string:
        case PrgValueKind::empty:
            break;
    }

    const std::string key = normalize_identifier(trim_copy(value_as_string(selector)));
    if (key.empty()) {
        return std::nullopt;
    }

    const auto found = std::find(runtime_object.collection_item_keys.begin(),
                                 runtime_object.collection_item_keys.end(),
                                 key);
    if (found == runtime_object.collection_item_keys.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(runtime_object.collection_item_keys.begin(), found));
}

std::optional<PrgValue> get_native_identity_reflection_metadata(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name == "hwnd" && runtime_object.native_hwnd.has_value()) {
        return make_int64_value(static_cast<std::int64_t>(*runtime_object.native_hwnd));
    }
    if (runtime_object.class_hierarchy.empty()) {
        return std::nullopt;
    }
    if (normalized_member_name == "class" && !trim_copy(runtime_object.prog_id).empty()) {
        return make_string_value(runtime_object.prog_id);
    }
    if (normalized_member_name == "baseclass" && !trim_copy(runtime_object.base_class_name).empty()) {
        return make_string_value(runtime_object.base_class_name);
    }
    if (normalized_member_name == "parentclass" && !trim_copy(runtime_object.base_class_name).empty()) {
        return make_string_value(runtime_object.base_class_name);
    }
    if (normalized_member_name == "classlibrary" && !trim_copy(runtime_object.class_library).empty()) {
        return make_string_value(runtime_object.class_library);
    }
    return std::nullopt;
}

bool native_identity_member_name_matches(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name) {
    if (normalized_member_name == "hwnd" && runtime_object.native_hwnd.has_value()) {
        return true;
    }
    if (runtime_object.class_hierarchy.empty()) {
        return false;
    }
    return normalized_member_name == "class" ||
           normalized_member_name == "baseclass" ||
           normalized_member_name == "parentclass" ||
           normalized_member_name == "classlibrary";
}

bool native_olecontrol_creation_time_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    if (!is_olecontrol) {
        return false;
    }
    return (normalized_member_name == "oleclass" ||
            normalized_member_name == "documentfile" ||
            normalized_member_name == "oletypeallowed") &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_olecontrol_object_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_olecontrol &&
           normalized_member_name == "object" &&
           runtime_object.properties.contains("object");
}

bool native_olecontrol_inspection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    if (!is_olecontrol) {
        return false;
    }
    return (normalized_member_name == "objectverbscount" ||
            normalized_member_name == "objectverbs") &&
           (normalized_member_name == "objectverbs" ||
           runtime_object.properties.contains("objectverbscount"));
}

bool native_olecontrol_conflict_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_olecontrol &&
           normalized_member_name == "application" &&
           runtime_object.properties.contains("application");
}

bool native_child_parent_member_name_matches(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name) {
    if (normalized_member_name != "parent") {
        return false;
    }

    const auto parent = runtime_object.properties.find("parent");
    if (parent == runtime_object.properties.end()) {
        return false;
    }

    int handle = 0;
    std::string prog_id;
    return parse_object_handle_reference(parent->second, handle, prog_id);
}

bool native_controlcount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "controlcount" &&
           runtime_object.properties.contains("controlcount");
}

bool native_pagecount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "pagecount" ||
        !runtime_object.properties.contains("pagecount")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "pageframe";
}

bool native_form_alwaysontop_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "alwaysontop") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("alwaysontop");
}

bool native_form_showwindow_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "showwindow") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("showwindow");
}

bool native_form_windowtype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "windowtype") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("windowtype");
}

bool native_form_windowstate_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "windowstate") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("windowstate");
}

bool native_form_borderstyle_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "borderstyle") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("borderstyle");
}

bool native_form_titlebar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "titlebar") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("titlebar");
}

bool native_form_desktop_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "desktop") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("desktop");
}

bool native_form_scrollbars_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "scrollbars") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("scrollbars");
}

bool native_form_lockscreen_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "lockscreen") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("lockscreen");
}

bool native_form_controlbox_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "controlbox") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("controlbox");
}

bool native_form_closable_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "closable") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("closable");
}

bool native_form_minbutton_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "minbutton") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("minbutton");
}

bool native_form_maxbutton_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "maxbutton") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("maxbutton");
}

bool native_form_autocenter_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "autocenter") {
        return false;
    }

    const std::string normalized_base_class = normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "form" &&
           runtime_object.properties.contains("autocenter");
}

bool native_visual_enabled_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "enabled" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("enabled");
}

bool native_visual_visible_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "visible" &&
           is_native_visual_runtime_object(runtime_object) &&
           runtime_object.properties.contains("visible");
}

bool native_visual_backcolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return normalized_member_name == "backcolor" &&
           is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains("backcolor");
}

bool native_visual_forecolor_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return normalized_member_name == "forecolor" &&
           is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains("forecolor");
}

bool native_visual_geometry_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "left" &&
        normalized_member_name != "top" &&
        normalized_member_name != "width" &&
        normalized_member_name != "height") {
        return false;
    }

    const bool is_olecontrol =
        normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
        normalize_identifier(runtime_object.prog_id) == "olecontrol";
    return is_native_visual_runtime_object(runtime_object) &&
           !is_olecontrol &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_string_control_value_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "value" ||
        !runtime_object.properties.contains("value")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "combobox";
}

bool native_controlsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "controlsource" ||
        !runtime_object.properties.contains("controlsource")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "column" ||
           normalized_base_class == "checkbox" ||
           normalized_base_class == "spinner";
}

bool native_recordsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordsource" ||
        !runtime_object.properties.contains("recordsource")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_allowaddnew_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowaddnew" ||
        !runtime_object.properties.contains("allowaddnew")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_allowcellselection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "allowcellselection" ||
        !runtime_object.properties.contains("allowcellselection")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_gridlines_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "gridlines" ||
        !runtime_object.properties.contains("gridlines")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_highlight_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "highlight" ||
        !runtime_object.properties.contains("highlight")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_highlightrow_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "highlightrow" ||
        !runtime_object.properties.contains("highlightrow")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_deletemark_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "deletemark" ||
        !runtime_object.properties.contains("deletemark")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_splitbar_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "splitbar" ||
        !runtime_object.properties.contains("splitbar")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_leftcolumn_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "leftcolumn" ||
        !runtime_object.properties.contains("leftcolumn")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_child_collection_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return (normalized_member_name == "objects" ||
            normalized_member_name == "controls" ||
            normalized_member_name == "columns" ||
            normalized_member_name == "pages") &&
           runtime_object.properties.contains(normalized_member_name);
}

bool native_columnorder_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columnorder" ||
        !runtime_object.properties.contains("columnorder")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_recordmark_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordmark" ||
        !runtime_object.properties.contains("recordmark")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_currentcontrol_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "currentcontrol" ||
        !runtime_object.properties.contains("currentcontrol")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_dynamiccurrentcontrol_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "dynamiccurrentcontrol" ||
        !runtime_object.properties.contains("dynamiccurrentcontrol")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_recordsourcetype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "recordsourcetype" ||
        !runtime_object.properties.contains("recordsourcetype")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "grid";
}

bool native_rowsource_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "rowsource" ||
        !runtime_object.properties.contains("rowsource")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_rowsourcetype_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "rowsourcetype" ||
        !runtime_object.properties.contains("rowsourcetype")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listindex" ||
        !runtime_object.properties.contains("listindex")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_displayvalue_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "displayvalue" ||
        !runtime_object.properties.contains("displayvalue")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listcount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listcount" ||
        !runtime_object.properties.contains("listcount")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_sorted_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "sorted" ||
        !runtime_object.properties.contains("sorted")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_multiselect_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "multiselect" ||
        !runtime_object.properties.contains("multiselect")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "listbox";
}

bool native_newindex_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "newindex" ||
        !runtime_object.properties.contains("newindex")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_newitemid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "newitemid" ||
        !runtime_object.properties.contains("newitemid")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_listitemid_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "listitemid" ||
        !runtime_object.properties.contains("listitemid")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox";
}

bool native_boundcolumn_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "boundcolumn" ||
        !runtime_object.properties.contains("boundcolumn")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox";
}

bool native_columncount_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columncount" ||
        !runtime_object.properties.contains("columncount")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox" ||
           normalized_base_class == "listbox" ||
           normalized_base_class == "grid";
}

bool native_column_bound_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "bound" ||
        !runtime_object.properties.contains("bound")) {
        return false;
    }

    return normalize_identifier(trim_copy(runtime_object.base_class_name)) == "column";
}

bool native_columnwidths_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "columnwidths" ||
        !runtime_object.properties.contains("columnwidths")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox";
}

bool native_combobox_style_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "style" ||
        !runtime_object.properties.contains("style")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "combobox";
}

bool native_combobox_is_drop_down_list_style(const RuntimeOleObjectState& runtime_object) {
    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    if (normalized_base_class != "combobox") {
        return false;
    }

    const auto style = runtime_object.properties.find("style");
    if (style == runtime_object.properties.end()) {
        return false;
    }

    return std::llround(value_as_number(style->second)) == 2LL;
}

bool native_control_readonly_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    if (normalized_member_name != "readonly" ||
        !runtime_object.properties.contains("readonly")) {
        return false;
    }

    const std::string normalized_base_class =
        normalize_identifier(trim_copy(runtime_object.base_class_name));
    return normalized_base_class == "textbox" ||
           normalized_base_class == "combobox" ||
           normalized_base_class == "editbox" ||
           normalized_base_class == "grid" ||
           normalized_base_class == "column" ||
           normalized_base_class == "checkbox" ||
           normalized_base_class == "spinner";
}

bool native_name_member_name_matches(
    const RuntimeOleObjectState& runtime_object,
    const std::string& normalized_member_name) {
    return normalized_member_name == "name" &&
           !runtime_object.class_hierarchy.empty() &&
           runtime_object.properties.contains("name");
}

std::vector<std::string> collect_native_identity_member_names(const RuntimeOleObjectState& runtime_object) {
    std::vector<std::string> members;
    if (get_native_identity_reflection_metadata(runtime_object, "hwnd").has_value()) {
        members.push_back("hwnd");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "class").has_value()) {
        members.push_back("class");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "baseclass").has_value()) {
        members.push_back("baseclass");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "classlibrary").has_value()) {
        members.push_back("classlibrary");
    }
    if (get_native_identity_reflection_metadata(runtime_object, "parentclass").has_value()) {
        members.push_back("parentclass");
    }
    return members;
}

bool method_ends_with_suffix(
    const std::string& method_name,
    const std::string& suffix,
    std::string* stem = nullptr) {
    const std::string normalized_method = normalize_identifier(method_name);
    if (normalized_method.size() <= suffix.size() ||
        normalized_method.compare(normalized_method.size() - suffix.size(), suffix.size(), suffix) != 0) {
        return false;
    }

    if (normalized_method[normalized_method.size() - suffix.size() - 1U] != '_') {
        return false;
    }

    if (stem != nullptr) {
        *stem = normalized_method.substr(0U, normalized_method.size() - suffix.size() - 1U);
    }
    return true;
}

bool object_has_accessor_property(const RuntimeOleObjectState& runtime_object, const std::string& normalized_property_name) {
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string& method_name) {
        std::string stem;
        return method_ends_with_suffix(method_name, "access", &stem) && stem == normalized_property_name;
    });
}

bool object_has_assigner_property(const RuntimeOleObjectState& runtime_object, const std::string& normalized_property_name) {
    return std::any_of(runtime_object.methods.begin(), runtime_object.methods.end(), [&](const std::string& method_name) {
        std::string stem;
        return method_ends_with_suffix(method_name, "assign", &stem) && stem == normalized_property_name;
    });
}

bool looks_like_file_path(const std::string& text) {
    const std::string trimmed = trim_copy(text);
    if (trimmed.empty()) {
        return false;
    }
    if (trimmed.find('/') != std::string::npos || trimmed.find('\\') != std::string::npos) {
        return true;
    }
    if (trimmed.size() >= 2U && std::isalpha(static_cast<unsigned char>(trimmed[0])) != 0 && trimmed[1] == ':') {
        return true;
    }
    const std::string lower = lowercase_copy(trimmed);
    const auto has_suffix = [&](const std::string& suffix) {
        return lower.size() >= suffix.size() && lower.compare(lower.size() - suffix.size(), suffix.size(), suffix) == 0;
    };
    return has_suffix(".xml") || has_suffix(".txt");
}

std::string xml_escape(std::string value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&':
                escaped += "&amp;";
                break;
            case '<':
                escaped += "&lt;";
                break;
            case '>':
                escaped += "&gt;";
                break;
            case '"':
                escaped += "&quot;";
                break;
            case '\'':
                escaped += "&apos;";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::string xml_unescape(std::string value) {
    const auto replace_all = [&](const std::string& token, const std::string& replacement) {
        std::size_t position = 0U;
        while ((position = value.find(token, position)) != std::string::npos) {
            value.replace(position, token.size(), replacement);
            position += replacement.size();
        }
    };
    replace_all("&lt;", "<");
    replace_all("&gt;", ">");
    replace_all("&quot;", "\"");
    replace_all("&apos;", "'");
    replace_all("&amp;", "&");
    return value;
}

std::string xml_attribute(const std::string& tag_text, const std::string& name) {
    const std::string needle = name + "=\"";
    const std::size_t start = tag_text.find(needle);
    if (start == std::string::npos) {
        return {};
    }
    const std::size_t value_start = start + needle.size();
    const std::size_t value_end = tag_text.find('"', value_start);
    if (value_end == std::string::npos) {
        return {};
    }
    return xml_unescape(tag_text.substr(value_start, value_end - value_start));
}

std::string serialize_cursor_snapshot_xml(const RuntimeSurfaceCursorSnapshot& snapshot) {
    std::ostringstream xml;
    xml << "<CopperfinCursor alias=\"" << xml_escape(snapshot.alias) << "\">\n";
    xml << "  <Fields>\n";
    for (const auto& field : snapshot.fields) {
        xml << "    <Field name=\"" << xml_escape(field.name)
            << "\" type=\"" << xml_escape(std::string(1U, field.type))
            << "\" width=\"" << field.width
            << "\" decimals=\"" << field.decimals
            << "\" />\n";
    }
    xml << "  </Fields>\n";
    xml << "  <Rows>\n";
    for (const auto& row : snapshot.rows) {
        xml << "    <Row>";
        for (const auto& value : row.values) {
            xml << "<Col>" << xml_escape(value) << "</Col>";
        }
        xml << "</Row>\n";
    }
    xml << "  </Rows>\n";
    xml << "</CopperfinCursor>\n";
    return xml.str();
}

std::optional<RuntimeSurfaceCursorSnapshot> parse_cursor_snapshot_xml(const std::string& xml_text) {
    RuntimeSurfaceCursorSnapshot snapshot;

    const std::size_t root_start = xml_text.find("<CopperfinCursor");
    if (root_start == std::string::npos) {
        return std::nullopt;
    }
    const std::size_t root_tag_end = xml_text.find('>', root_start);
    const std::size_t root_end = xml_text.find("</CopperfinCursor>", root_tag_end == std::string::npos ? 0U : root_tag_end);
    if (root_tag_end == std::string::npos || root_end == std::string::npos) {
        return std::nullopt;
    }
    snapshot.alias = xml_attribute(xml_text.substr(root_start, root_tag_end - root_start + 1U), "alias");

    const std::size_t fields_start = xml_text.find("<Fields>", root_tag_end);
    const std::size_t fields_end = xml_text.find("</Fields>", fields_start == std::string::npos ? 0U : fields_start);
    if (fields_start == std::string::npos || fields_end == std::string::npos) {
        return std::nullopt;
    }

    std::size_t scan = fields_start;
    while (true) {
        const std::size_t field_start = xml_text.find("<Field ", scan);
        if (field_start == std::string::npos || field_start >= fields_end) {
            break;
        }
        const std::size_t field_end = xml_text.find("/>", field_start);
        if (field_end == std::string::npos || field_end > fields_end) {
            return std::nullopt;
        }
        const std::string field_tag = xml_text.substr(field_start, field_end - field_start + 2U);
        RuntimeSurfaceCursorField field;
        field.name = xml_attribute(field_tag, "name");
        const std::string type_text = xml_attribute(field_tag, "type");
        field.type = type_text.empty() ? 'C' : type_text.front();
        try {
            field.width = static_cast<std::size_t>(std::stoul(xml_attribute(field_tag, "width")));
        } catch (...) {
            field.width = 0U;
        }
        try {
            field.decimals = static_cast<std::size_t>(std::stoul(xml_attribute(field_tag, "decimals")));
        } catch (...) {
            field.decimals = 0U;
        }
        if (field.name.empty()) {
            return std::nullopt;
        }
        snapshot.fields.push_back(std::move(field));
        scan = field_end + 2U;
    }

    const std::size_t rows_start = xml_text.find("<Rows>", fields_end);
    const std::size_t rows_end = xml_text.find("</Rows>", rows_start == std::string::npos ? 0U : rows_start);
    if (rows_start == std::string::npos || rows_end == std::string::npos) {
        return std::nullopt;
    }

    scan = rows_start;
    while (true) {
        const std::size_t row_start = xml_text.find("<Row>", scan);
        if (row_start == std::string::npos || row_start >= rows_end) {
            break;
        }
        const std::size_t row_end = xml_text.find("</Row>", row_start);
        if (row_end == std::string::npos || row_end > rows_end) {
            return std::nullopt;
        }

        RuntimeSurfaceCursorRow row;
        std::size_t col_scan = row_start;
        while (true) {
            const std::size_t col_start = xml_text.find("<Col>", col_scan);
            if (col_start == std::string::npos || col_start >= row_end) {
                break;
            }
            const std::size_t col_value_start = col_start + 5U;
            const std::size_t col_end = xml_text.find("</Col>", col_value_start);
            if (col_end == std::string::npos || col_end > row_end) {
                return std::nullopt;
            }
            row.values.push_back(xml_unescape(xml_text.substr(col_value_start, col_end - col_value_start)));
            col_scan = col_end + 6U;
        }
        snapshot.rows.push_back(std::move(row));
        scan = row_end + 6U;
    }

    return snapshot;
}

std::vector<std::string> collect_object_member_names(const RuntimeOleObjectState& runtime_object, int flags) {
    const bool include_all = flags == 0;
    const bool include_properties = include_all || ((flags & 1) != 0);
    const bool include_methods = include_all || ((flags & 2) != 0);
    const bool include_events = include_all || ((flags & 4) != 0);

    std::set<std::string> unique_members;
    if (include_properties) {
        for (const auto& [name, value] : runtime_object.properties) {
            (void)value;
            if (native_name_member_name_matches(runtime_object, normalize_identifier(name))) {
                continue;
            }
            unique_members.insert(normalize_identifier(name));
        }
        if (is_native_collection_object(runtime_object)) {
            unique_members.insert("count");
        }
        for (const auto& metadata_name : collect_native_identity_member_names(runtime_object)) {
            unique_members.insert(metadata_name);
        }
        for (const auto& method_name : runtime_object.methods) {
            std::string stem;
            if ((method_ends_with_suffix(method_name, "access", &stem) ||
                 method_ends_with_suffix(method_name, "assign", &stem)) &&
                !stem.empty()) {
                unique_members.insert(stem);
            }
        }
    }
    if (include_methods) {
        for (const auto& method_name : runtime_object.methods) {
            unique_members.insert(normalize_identifier(method_name));
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "release")) {
            unique_members.insert("release");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "refresh")) {
            unique_members.insert("refresh");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "move")) {
            unique_members.insert("move");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "show")) {
            unique_members.insert("show");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "hide")) {
            unique_members.insert("hide");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "setfocus")) {
            unique_members.insert("setfocus");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "resettodefault")) {
            unique_members.insert("resettodefault");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "additem")) {
            unique_members.insert("additem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "addlistitem")) {
            unique_members.insert("addlistitem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "removeitem")) {
            unique_members.insert("removeitem");
        }
        if (is_builtin_native_runtime_method_name(runtime_object, "removelistitem")) {
            unique_members.insert("removelistitem");
        }
        if (is_native_collection_object(runtime_object)) {
            unique_members.insert("item");
            if (!runtime_object.read_only_collection_surface) {
                unique_members.insert("add");
                unique_members.insert("remove");
                unique_members.insert("removeall");
            }
        }
    }
    if (include_events) {
        for (const auto& event_name : runtime_object.events) {
            unique_members.insert(normalize_identifier(event_name));
        }
    }

    std::vector<std::string> members;
    members.reserve(unique_members.size());
    for (const std::string& member_name : unique_members) {
        members.push_back(uppercase_copy(member_name));
    }

    std::sort(members.begin(), members.end(), [](const std::string& left, const std::string& right) {
        const std::string normalized_left = lowercase_copy(left);
        const std::string normalized_right = lowercase_copy(right);
        if (normalized_left == normalized_right) {
            return left < right;
        }
        return normalized_left < normalized_right;
    });
    return members;
}

}  // namespace

std::optional<std::size_t> parse_native_list_control_selected_member_slot(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_selected_member_slot_impl(runtime_object, member_name);
}

std::optional<long long> parse_native_list_control_selectedid_member_item_id(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_selectedid_member_item_id_impl(runtime_object, member_name);
}

void sync_native_list_control_displayvalue_from_selection(RuntimeOleObjectState& runtime_object)
{
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
}

void sync_native_list_control_count(RuntimeOleObjectState& runtime_object)
{
    sync_native_list_control_count_impl(runtime_object);
}

bool write_native_list_control_item_id(RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);

    const long long requested_item_id = std::llround(value_as_number(assigned_value));
    auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
        listindex = runtime_object.properties.find("listindex");
    }

    if (requested_item_id == 0LL) {
        listindex->second = make_number_value(0.0);
        sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
        return true;
    }

    if (requested_item_id < 0LL) {
        return false;
    }

    const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
    if (!slot.has_value()) {
        return false;
    }

    listindex->second = make_number_value(static_cast<double>(*slot + 1U));
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

bool write_native_list_control_cell(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot,
    std::size_t column_slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    if (row_slot >= runtime_object.list_rows.size()) {
        return false;
    }

    auto& row = runtime_object.list_rows[row_slot];
    if (row.size() <= column_slot) {
        row.resize(column_slot + 1U, make_string_value(""));
    }
    row[column_slot] = make_string_value(value_as_string(assigned_value));
    sync_native_list_control_primary_state_from_rows(runtime_object);
    if (column_slot == 0U) {
        sort_native_list_control_rows_if_needed(runtime_object);
    }
    sync_native_list_control_count_impl(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

bool write_native_list_control_selected_slot(
    RuntimeOleObjectState& runtime_object,
    std::size_t slot,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);
    sync_native_list_control_selected_state_size(runtime_object);
    if (slot >= runtime_object.list_selected.size()) {
        return false;
    }

    const bool selected = value_as_bool(assigned_value);
    if (selected && !native_list_control_allows_multiple_selection(runtime_object)) {
        std::fill(runtime_object.list_selected.begin(), runtime_object.list_selected.end(), false);
    }
    runtime_object.list_selected[slot] = selected;

    auto listindex = runtime_object.properties.find("listindex");
    if (listindex == runtime_object.properties.end()) {
        runtime_object.properties["listindex"] = make_number_value(0.0);
        listindex = runtime_object.properties.find("listindex");
    }

    if (selected) {
        listindex->second = make_number_value(static_cast<double>(slot + 1U));
    } else if (std::llround(value_as_number(listindex->second)) == static_cast<long long>(slot + 1U)) {
        if (const auto replacement = find_last_native_list_control_selected_slot(runtime_object);
            replacement.has_value()) {
            listindex->second = make_number_value(static_cast<double>(*replacement + 1U));
        } else {
            listindex->second = make_number_value(0.0);
        }
    }

    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    return true;
}

bool write_native_list_control_selected_item_id(
    RuntimeOleObjectState& runtime_object,
    long long requested_item_id,
    const PrgValue& assigned_value)
{
    if (!is_native_list_control_runtime_object(runtime_object) ||
        requested_item_id < 1LL) {
        return false;
    }

    materialize_native_list_control_rows(runtime_object);

    const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
    if (!slot.has_value()) {
        return false;
    }

    return write_native_list_control_selected_slot(runtime_object, *slot, assigned_value);
}

bool is_native_identity_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_identity_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_creation_time_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_creation_time_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_object_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_object_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_inspection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_inspection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_olecontrol_conflict_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_olecontrol_conflict_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_child_parent_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_child_parent_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controlcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controlcount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_pagecount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_pagecount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_alwaysontop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_alwaysontop_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_showwindow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_showwindow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_windowtype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_windowtype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_windowstate_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_windowstate_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_borderstyle_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_borderstyle_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_titlebar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_titlebar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_desktop_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_desktop_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_scrollbars_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_scrollbars_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_lockscreen_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_lockscreen_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_controlbox_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_controlbox_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_closable_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_closable_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_minbutton_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_minbutton_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_maxbutton_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_maxbutton_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_form_autocenter_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_form_autocenter_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_enabled_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_enabled_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_visible_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_visible_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_backcolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_backcolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_forecolor_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_forecolor_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_visual_geometry_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_visual_geometry_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_control_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_control_readonly_member_name_matches(runtime_object, normalized_member_name);
}

bool native_combobox_readonly_assignment_blocked(const RuntimeOleObjectState& runtime_object, const PrgValue& assigned_value)
{
    return native_combobox_is_drop_down_list_style(runtime_object) &&
           value_as_bool(assigned_value);
}

void normalize_native_combobox_readonly_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!native_combobox_is_drop_down_list_style(runtime_object)) {
        return;
    }

    const auto readonly = runtime_object.properties.find("readonly");
    if (readonly != runtime_object.properties.end() &&
        value_as_bool(readonly->second)) {
        readonly->second = make_boolean_value(false);
    }
}

void normalize_native_list_control_sorted_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return;
    }

    const auto sorted = runtime_object.properties.find("sorted");
    if (sorted == runtime_object.properties.end()) {
        return;
    }

    sorted->second = make_boolean_value(value_as_bool(sorted->second));
    sort_native_list_control_rows_if_needed(runtime_object);
}

void normalize_native_listbox_multiselect_invariant(RuntimeOleObjectState& runtime_object)
{
    if (!is_native_listbox_runtime_object(runtime_object)) {
        return;
    }

    const auto multiselect = runtime_object.properties.find("multiselect");
    if (multiselect == runtime_object.properties.end()) {
        return;
    }

    multiselect->second = make_boolean_value(value_as_bool(multiselect->second));
    sync_native_list_control_selected_state_from_listindex(runtime_object);
}

bool is_native_combobox_style_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_combobox_style_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_string_control_value_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_string_control_value_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_controlsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_controlsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_allowaddnew_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_allowaddnew_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_allowcellselection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_allowcellselection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_gridlines_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_gridlines_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_highlight_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_highlight_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_highlightrow_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_highlightrow_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_deletemark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_deletemark_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_splitbar_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_splitbar_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_leftcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_leftcolumn_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columnorder_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columnorder_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_child_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_child_collection_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordmark_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordmark_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_currentcontrol_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_currentcontrol_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_dynamiccurrentcontrol_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_dynamiccurrentcontrol_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_recordsourcetype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_recordsourcetype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_rowsource_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_rowsource_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_rowsourcetype_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_rowsourcetype_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_displayvalue_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_displayvalue_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listcount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listcount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_sorted_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_sorted_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_multiselect_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_multiselect_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_newindex_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_newindex_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_newitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_newitemid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_listitemid_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_listitemid_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_boundcolumn_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_boundcolumn_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columncount_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columncount_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_column_bound_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_column_bound_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_columnwidths_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_columnwidths_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_name_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return native_name_member_name_matches(runtime_object, normalized_member_name);
}

bool is_native_collection_object(const RuntimeOleObjectState& runtime_object)
{
    return normalize_identifier(runtime_object.base_class_name) == "collection" ||
           normalize_identifier(runtime_object.prog_id) == "collection";
}

bool is_native_collection_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    if (!is_native_collection_object(runtime_object)) {
        return false;
    }
    return normalized_member_name == "count" ||
           normalized_member_name == "item" ||
           (!runtime_object.read_only_collection_surface &&
            (normalized_member_name == "add" ||
             normalized_member_name == "remove" ||
             normalized_member_name == "removeall"));
}

bool is_native_collection_readonly_member_name(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    return is_native_collection_object(runtime_object) && normalized_member_name == "count";
}

std::optional<PrgValue> read_native_collection_member(RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    if (!is_native_collection_object(runtime_object) || normalized_member_name != "count") {
        return std::nullopt;
    }
    sync_native_collection_count(runtime_object);
    return runtime_object.properties["count"];
}

std::optional<PrgValue> invoke_native_collection_method(RuntimeOleObjectState& runtime_object,
                                                        const std::string& normalized_method_name,
                                                        const std::vector<PrgValue>& arguments)
{
    if (!is_native_collection_object(runtime_object)) {
        return std::nullopt;
    }

    if (runtime_object.read_only_collection_surface &&
        normalized_method_name != "item") {
        return make_boolean_value(false);
    }

    if (normalized_method_name == "add" && !arguments.empty()) {
        const std::string key =
            arguments.size() >= 2U ? normalize_identifier(trim_copy(value_as_string(arguments[1]))) : std::string{};
        if (!key.empty()) {
            const auto existing = std::find(runtime_object.collection_item_keys.begin(),
                                            runtime_object.collection_item_keys.end(),
                                            key);
            if (existing != runtime_object.collection_item_keys.end()) {
                runtime_object.collection_items[static_cast<std::size_t>(
                    std::distance(runtime_object.collection_item_keys.begin(), existing))] = arguments[0];
            } else {
                runtime_object.collection_items.push_back(arguments[0]);
                runtime_object.collection_item_keys.push_back(key);
            }
        } else {
            runtime_object.collection_items.push_back(arguments[0]);
            runtime_object.collection_item_keys.push_back({});
        }
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    if (normalized_method_name == "item" && !arguments.empty()) {
        const auto slot = resolve_native_collection_slot(runtime_object, arguments[0]);
        return slot.has_value() ? runtime_object.collection_items[*slot] : make_empty_value();
    }

    if (normalized_method_name == "remove" && !arguments.empty()) {
        const auto slot = resolve_native_collection_slot(runtime_object, arguments[0]);
        if (!slot.has_value()) {
            return make_boolean_value(false);
        }
        runtime_object.collection_items.erase(runtime_object.collection_items.begin() + static_cast<std::ptrdiff_t>(*slot));
        runtime_object.collection_item_keys.erase(runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(*slot));
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    if (normalized_method_name == "removeall") {
        runtime_object.collection_items.clear();
        runtime_object.collection_item_keys.clear();
        sync_native_collection_count(runtime_object);
        return make_boolean_value(true);
    }

    return std::nullopt;
}

std::optional<PrgValue> invoke_native_list_control_method(RuntimeOleObjectState& runtime_object,
                                                          const std::string& normalized_method_name,
                                                          const std::vector<PrgValue>& arguments)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);

    if (normalized_method_name == "list") {
        if (arguments.empty()) {
            return make_string_value("");
        }

        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.list_rows.size()) {
            return make_string_value("");
        }

        long long requested_column = 1LL;
        if (arguments.size() >= 2U) {
            requested_column = std::llround(value_as_number(arguments[1]));
            if (requested_column < 1LL) {
                return make_string_value("");
            }
        }

        const auto& row = runtime_object.list_rows[static_cast<std::size_t>(requested_index - 1LL)];
        if (static_cast<std::size_t>(requested_column) > row.size()) {
            return make_string_value("");
        }
        return make_string_value(
            value_as_string(row[static_cast<std::size_t>(requested_column - 1LL)]));
    }

    if (normalized_method_name == "selected") {
        if (arguments.empty()) {
            return make_boolean_value(false);
        }

        sync_native_list_control_selected_state_size(runtime_object);
        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.list_selected.size()) {
            return make_boolean_value(false);
        }

        return make_boolean_value(
            runtime_object.list_selected[static_cast<std::size_t>(requested_index - 1LL)]);
    }

    if (normalized_method_name == "selectedid") {
        if (arguments.empty()) {
            return make_boolean_value(false);
        }

        sync_native_list_control_selected_state_size(runtime_object);
        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_boolean_value(false);
        }

        const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
        if (!slot.has_value()) {
            return make_boolean_value(false);
        }

        return make_boolean_value(runtime_object.list_selected[*slot]);
    }

    if (normalized_method_name == "addlistitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_number_value(0.0);
        }

        long long requested_column = 1LL;
        if (arguments.size() >= 3U) {
            requested_column = std::llround(value_as_number(arguments[2]));
            if (requested_column < 1LL) {
                return make_number_value(0.0);
            }
        }

        const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));

        if (requested_column == 1LL) {
            long long item_id = next_native_list_control_item_id(runtime_object);
            if (arguments.size() >= 2U) {
                item_id = std::llround(value_as_number(arguments[1]));
                if (item_id < 1LL ||
                    find_native_list_control_row_by_item_id(runtime_object, item_id).has_value()) {
                    return make_number_value(0.0);
                }
            }

            runtime_object.list_rows.push_back({inserted_item});
            runtime_object.collection_item_keys.push_back(std::to_string(item_id));
            runtime_object.list_selected.push_back(false);
            sync_native_list_control_primary_state_from_rows(runtime_object);
            sync_native_list_control_count_impl(runtime_object);
            runtime_object.properties["newindex"] =
                make_number_value(static_cast<double>(runtime_object.list_rows.size()));
            runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));
            sort_native_list_control_rows_if_needed(runtime_object);
            return make_number_value(static_cast<double>(item_id));
        }

        if (arguments.size() < 2U) {
            return make_number_value(0.0);
        }

        const long long item_id = std::llround(value_as_number(arguments[1]));
        const auto slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
        if (!slot.has_value()) {
            return make_number_value(0.0);
        }

        auto& row = runtime_object.list_rows[*slot];
        if (row.size() < static_cast<std::size_t>(requested_column)) {
            row.resize(static_cast<std::size_t>(requested_column), make_string_value(""));
        }
        row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;
        sync_native_list_control_primary_state_from_rows(runtime_object);
        if (requested_column == 1LL) {
            sort_native_list_control_rows_if_needed(runtime_object);
        }
        sync_native_list_control_count_impl(runtime_object);
        runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));
        sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
        return make_number_value(static_cast<double>(item_id));
    }

    if (normalized_method_name == "removeitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_empty_value();
        }

        const long long requested_index = std::llround(value_as_number(arguments[0]));
        if (requested_index < 1LL ||
            static_cast<std::size_t>(requested_index) > runtime_object.collection_items.size()) {
            return make_empty_value();
        }
        if (!remove_native_list_control_slot(
                runtime_object,
                static_cast<std::size_t>(requested_index - 1LL))) {
            return make_empty_value();
        }
        return make_empty_value();
    }

    if (normalized_method_name == "removelistitem") {
        if (arguments.empty() ||
            !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
            return make_empty_value();
        }

        const long long requested_item_id = std::llround(value_as_number(arguments[0]));
        if (requested_item_id < 1LL) {
            return make_empty_value();
        }

        const auto slot = find_native_list_control_row_by_item_id(runtime_object, requested_item_id);
        if (!slot.has_value() ||
            !remove_native_list_control_slot(runtime_object, *slot)) {
            return make_empty_value();
        }
        return make_empty_value();
    }

    if (normalized_method_name != "additem") {
        return std::nullopt;
    }

    if (arguments.empty() ||
        !native_list_control_rowsourcetype_supports_additem(runtime_object)) {
        return make_number_value(0.0);
    }

    long long requested_column = 1LL;
    if (arguments.size() >= 3U) {
        requested_column = std::llround(value_as_number(arguments[2]));
        if (requested_column < 1LL) {
            return make_number_value(0.0);
        }
    }

    const auto insert_slot = resolve_native_list_control_insert_slot(runtime_object, arguments);
    if (!insert_slot.has_value()) {
        return make_number_value(0.0);
    }

    const PrgValue inserted_item = make_string_value(value_as_string(arguments[0]));
    const std::int64_t item_id = next_native_list_control_item_id(runtime_object);
    std::vector<PrgValue> inserted_row;
    inserted_row.resize(static_cast<std::size_t>(requested_column), make_string_value(""));
    inserted_row[static_cast<std::size_t>(requested_column - 1LL)] = inserted_item;
    runtime_object.list_rows.insert(
        runtime_object.list_rows.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        std::move(inserted_row));
    runtime_object.collection_item_keys.insert(
        runtime_object.collection_item_keys.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        std::to_string(item_id));
    runtime_object.list_selected.insert(
        runtime_object.list_selected.begin() + static_cast<std::ptrdiff_t>(*insert_slot),
        false);
    sync_native_list_control_primary_state_from_rows(runtime_object);
    sync_native_list_control_count_impl(runtime_object);
    runtime_object.properties["newindex"] =
        make_number_value(static_cast<double>(*insert_slot + 1U));
    runtime_object.properties["newitemid"] = make_number_value(static_cast<double>(item_id));

    const auto listindex = runtime_object.properties.find("listindex");
    if (listindex != runtime_object.properties.end()) {
        const long long selected_index = std::llround(value_as_number(listindex->second));
        if (selected_index >= 1LL &&
            static_cast<std::size_t>(selected_index) >= *insert_slot + 1U) {
            listindex->second = make_number_value(static_cast<double>(selected_index + 1LL));
        }
    }
    sort_native_list_control_rows_if_needed(runtime_object);
    sync_native_list_control_displayvalue_from_selection_impl(runtime_object);
    if (const auto actual_slot = find_native_list_control_row_by_item_id(runtime_object, item_id);
        actual_slot.has_value()) {
        runtime_object.properties["newindex"] =
            make_number_value(static_cast<double>(*actual_slot + 1U));
        return make_number_value(static_cast<double>(*actual_slot + 1U));
    }
    return make_number_value(static_cast<double>(*insert_slot + 1U));
}

std::optional<NativeListControlCellReference> parse_native_list_control_list_member_cell(
    const RuntimeOleObjectState& runtime_object,
    const std::string& member_name)
{
    return parse_native_list_control_list_member_cell_impl(runtime_object, member_name);
}

std::optional<PrgValue> read_native_list_control_cell(
    RuntimeOleObjectState& runtime_object,
    std::size_t row_slot,
    std::size_t column_slot)
{
    if (!is_native_list_control_runtime_object(runtime_object)) {
        return std::nullopt;
    }

    materialize_native_list_control_rows(runtime_object);
    if (row_slot >= runtime_object.list_rows.size()) {
        return make_string_value("");
    }

    const auto& row = runtime_object.list_rows[row_slot];
    if (column_slot >= row.size()) {
        return make_string_value("");
    }

    return make_string_value(value_as_string(row[column_slot]));
}

std::optional<PrgValue> read_native_identity_metadata(const RuntimeOleObjectState& runtime_object, const std::string& normalized_member_name)
{
    // Ordinary dotted reads intentionally trail reflection parity for metadata we have not widened yet.
    if (normalized_member_name != "hwnd" &&
        normalized_member_name != "class" &&
        normalized_member_name != "baseclass" &&
        normalized_member_name != "parentclass" &&
        normalized_member_name != "classlibrary")
    {
        return std::nullopt;
    }
    return get_native_identity_reflection_metadata(runtime_object, normalized_member_name);
}

std::optional<PrgValue> evaluate_runtime_surface_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::vector<std::string>& raw_arguments,
    const std::string& default_directory,
    const std::string& frame_file_path,
    const std::string& last_error_message,
    int last_error_code,
    const std::string& last_error_procedure,
    std::size_t last_error_line,
    const std::string& error_handler,
    const std::string& shutdown_handler,
    const std::function<int(const std::string&)>& aerror_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback,
    const std::function<std::optional<RuntimeSurfaceCursorSnapshot>(const std::string&)>& snapshot_cursor_callback,
    const std::function<std::optional<std::size_t>(const RuntimeSurfaceCursorSnapshot&, const std::string&)>& load_cursor_snapshot_callback,
    const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
    const std::function<std::optional<PrgValue>(const PrgValue&, const std::string&)>& read_native_member_callback,
    const std::function<bool(const PrgValue&, const std::string&, const PrgValue&)>& write_native_member_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& whandle_from_hwnd_callback,
    const std::function<std::optional<std::int64_t>(std::int64_t)>& hwnd_from_whandle_callback,
    const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
    const std::function<void(const std::string&, const std::string&)>& record_event_callback) {
    auto record_runtime_warning = [&](const std::string& detail) {
        if (record_event_callback) {
            record_event_callback("runtime.warning", detail);
        }
    };

    if (const auto file_io_result = evaluate_file_io_function(function, arguments, default_directory)) {
        return file_io_result;
    }

    auto safe_int_argument = [&](std::size_t index, int default_value) {
        if (index >= arguments.size()) {
            return default_value;
        }
        const PrgValue& value = arguments[index];
        switch (value.kind) {
            case PrgValueKind::boolean:
                return value.boolean_value ? 1 : 0;
            case PrgValueKind::number:
                return static_cast<int>(std::llround(value.number_value));
            case PrgValueKind::int64:
                return static_cast<int>(value.int64_value);
            case PrgValueKind::uint64:
                return static_cast<int>(value.uint64_value);
            case PrgValueKind::string:
                try {
                    return value.string_value.empty() ? default_value : static_cast<int>(std::llround(std::stod(value.string_value)));
                } catch (...) {
                    return default_value;
                }
            case PrgValueKind::empty:
                return default_value;
        }
        return default_value;
    };
    auto safe_int64_argument = [&](std::size_t index, std::int64_t default_value) {
        if (index >= arguments.size()) {
            return default_value;
        }
        const PrgValue& value = arguments[index];
        switch (value.kind) {
            case PrgValueKind::boolean:
                return value.boolean_value ? static_cast<std::int64_t>(1) : static_cast<std::int64_t>(0);
            case PrgValueKind::number:
                return static_cast<std::int64_t>(std::llround(value.number_value));
            case PrgValueKind::int64:
                return value.int64_value;
            case PrgValueKind::uint64:
                return static_cast<std::int64_t>(value.uint64_value);
            case PrgValueKind::string:
                try {
                    return static_cast<std::int64_t>(std::stoll(trim_copy(value.string_value)));
                } catch (...) {
                    return default_value;
                }
            case PrgValueKind::empty:
                return default_value;
        }
        return default_value;
    };

    const auto is_native_olecontrol_host_runtime_object = [](const RuntimeOleObjectState& runtime_object) {
        return normalize_identifier(runtime_object.base_class_name) == "olecontrol" ||
               normalize_identifier(runtime_object.prog_id) == "olecontrol";
    };
    const auto reflectable_member_exists_locally = [](const RuntimeOleObjectState& runtime_object,
                                                      const std::string& member_name) {
        return get_native_identity_reflection_metadata(runtime_object, member_name).has_value() ||
               native_controlcount_member_name_matches(runtime_object, member_name) ||
               native_pagecount_member_name_matches(runtime_object, member_name) ||
               native_listcount_member_name_matches(runtime_object, member_name) ||
               native_sorted_member_name_matches(runtime_object, member_name) ||
               native_newindex_member_name_matches(runtime_object, member_name) ||
               native_newitemid_member_name_matches(runtime_object, member_name) ||
               native_listitemid_member_name_matches(runtime_object, member_name) ||
               native_visual_geometry_member_name_matches(runtime_object, member_name) ||
               is_native_collection_member_name(runtime_object, member_name) ||
               runtime_object.properties.contains(member_name) ||
               object_has_accessor_property(runtime_object, member_name) ||
               is_builtin_native_runtime_method_name(runtime_object, member_name) ||
               object_has_member(runtime_object.methods, member_name) ||
               object_has_member(runtime_object.events, member_name);
    };
    const auto reflectable_member_readonly_locally = [](const RuntimeOleObjectState& runtime_object,
                                                        const std::string& member_name) {
        return get_native_identity_reflection_metadata(runtime_object, member_name).has_value() ||
               native_controlcount_member_name_matches(runtime_object, member_name) ||
               native_pagecount_member_name_matches(runtime_object, member_name) ||
               native_listcount_member_name_matches(runtime_object, member_name) ||
               native_newindex_member_name_matches(runtime_object, member_name) ||
               native_newitemid_member_name_matches(runtime_object, member_name) ||
               native_child_collection_member_name_matches(runtime_object, member_name) ||
               is_native_splitbar_member_name(runtime_object, member_name) ||
               is_native_leftcolumn_member_name(runtime_object, member_name) ||
               is_native_form_desktop_member_name(runtime_object, member_name) ||
               is_native_form_scrollbars_member_name(runtime_object, member_name) ||
               is_native_olecontrol_creation_time_member_name(runtime_object, member_name) ||
               is_native_olecontrol_object_member_name(runtime_object, member_name) ||
               is_native_olecontrol_inspection_member_name(runtime_object, member_name) ||
               is_native_olecontrol_conflict_member_name(runtime_object, member_name) ||
               native_name_member_name_matches(runtime_object, member_name) ||
               native_child_parent_member_name_matches(runtime_object, member_name) ||
               is_native_collection_readonly_member_name(runtime_object, member_name) ||
               (is_scripting_dictionary_object(runtime_object) && member_name == "count") ||
               (object_has_accessor_property(runtime_object, member_name) &&
                !object_has_assigner_property(runtime_object, member_name));
    };
    const auto resolve_direct_olecontrol_reflection_surface =
        [&](RuntimeOleObjectState& runtime_object) -> RuntimeOleObjectState*
    {
        if (!resolve_object_callback || !is_native_olecontrol_host_runtime_object(runtime_object)) {
            return nullptr;
        }
        const auto object_surface = runtime_object.properties.find("object");
        if (object_surface == runtime_object.properties.end()) {
            return nullptr;
        }
        RuntimeOleObjectState* nested_object = resolve_object_callback(object_surface->second);
        if (nested_object == nullptr || nested_object == &runtime_object) {
            return nullptr;
        }
        return nested_object;
    };
    const auto merge_member_tokens = [](const std::vector<std::string>& primary,
                                        const std::vector<std::string>& secondary) {
        std::vector<std::string> merged = primary;
        merged.reserve(primary.size() + secondary.size());
        merged.insert(merged.end(), secondary.begin(), secondary.end());
        std::sort(merged.begin(), merged.end(), [](const std::string& left, const std::string& right) {
            const std::string normalized_left = lowercase_copy(left);
            const std::string normalized_right = lowercase_copy(right);
            if (normalized_left == normalized_right) {
                return left < right;
            }
            return normalized_left < normalized_right;
        });
        merged.erase(std::unique(merged.begin(), merged.end(), [](const std::string& left, const std::string& right) {
            return lowercase_copy(left) == lowercase_copy(right);
        }), merged.end());
        return merged;
    };

    if (function == "compobj" && arguments.size() >= 2U) {
        int handle_left = 0;
        int handle_right = 0;
        std::string prog_id_left;
        std::string prog_id_right;
        if (!parse_object_handle_reference(arguments[0], handle_left, prog_id_left) ||
            !parse_object_handle_reference(arguments[1], handle_right, prog_id_right)) {
            return make_boolean_value(false);
        }
        if (handle_left != handle_right) {
            return make_boolean_value(false);
        }
        // VFP: same handle means same object; use callback for pointer-level confirm when available
        if (resolve_object_callback) {
            RuntimeOleObjectState* left = resolve_object_callback(arguments[0]);
            RuntimeOleObjectState* right = resolve_object_callback(arguments[1]);
            return make_boolean_value(left != nullptr && right != nullptr && left == right);
        }
        return make_boolean_value(true);
    }

    if (function == "amembers" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            const int flags = safe_int_argument(2U, 0);
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
                    {
                        {"capability", "object/array"},
                        {"function", "AMEMBERS()"}
                    }));
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            std::vector<PrgValue> member_names;
            std::vector<std::string> member_tokens = collect_object_member_names(*runtime_object, flags);
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                member_tokens = merge_member_tokens(
                    member_tokens,
                    collect_object_member_names(*object_surface, flags));
            }
            member_names.reserve(member_tokens.size());
            for (const std::string& member_name : member_tokens) {
                member_names.push_back(make_string_value(member_name));
            }
            assign_array_callback(array_name, member_names);
            return make_number_value(static_cast<double>(member_names.size()));
        } catch (...) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.AmembersFallback",
                {{"function", "AMEMBERS()"}}));
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "aclass" && arguments.size() >= 2U && !raw_arguments.empty()) {
        const std::string array_name = trim_copy(raw_arguments[0]);
        try {
            if (!resolve_object_callback || !assign_array_callback) {
                record_runtime_warning(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
                    {
                        {"capability", "object/array"},
                        {"function", "ACLASS()"}
                    }));
                if (assign_array_callback && !array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[1]);
            if (runtime_object == nullptr || array_name.empty()) {
                if (!array_name.empty()) {
                    assign_array_callback(array_name, {});
                }
                return make_number_value(0.0);
            }

            std::vector<PrgValue> class_chain;
            if (!runtime_object->class_hierarchy.empty()) {
                class_chain.reserve(runtime_object->class_hierarchy.size());
                for (const std::string& class_name : runtime_object->class_hierarchy) {
                    class_chain.push_back(make_string_value(class_name));
                }
            } else {
                const std::string class_name = class_token_from_prog_id(runtime_object->prog_id);
                class_chain.push_back(make_string_value(class_name));
                class_chain.push_back(make_string_value("OBJECT"));
            }
            assign_array_callback(array_name, class_chain);
            return make_number_value(static_cast<double>(class_chain.size()));
        } catch (...) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.AclassFallback",
                {{"function", "ACLASS()"}}));
            if (assign_array_callback && !array_name.empty()) {
                assign_array_callback(array_name, {});
            }
            return make_number_value(0.0);
        }
    }

    if (function == "pemstatus" && arguments.size() >= 3U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubObjectResolution",
                {{"function", "PEMSTATUS()"}}));
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        const int attribute = safe_int_argument(2U, 0);
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (attribute == 1) {
            bool exists = reflectable_member_exists_locally(*runtime_object, member_name);
            if (!exists) {
                if (RuntimeOleObjectState* object_surface =
                        resolve_direct_olecontrol_reflection_surface(*runtime_object);
                    object_surface != nullptr) {
                    exists = reflectable_member_exists_locally(*object_surface, member_name);
                }
            }
            return make_boolean_value(exists);
        }
        if (attribute == 3) {
            return make_boolean_value(false);
        }
        if (attribute == 5) {
            bool readonly = reflectable_member_readonly_locally(*runtime_object, member_name);
            if (!readonly && !reflectable_member_exists_locally(*runtime_object, member_name)) {
                if (RuntimeOleObjectState* object_surface =
                        resolve_direct_olecontrol_reflection_surface(*runtime_object);
                    object_surface != nullptr) {
                    readonly = reflectable_member_readonly_locally(*object_surface, member_name);
                }
            }
            return make_boolean_value(readonly);
        }
        return make_boolean_value(false);
    }

    if (function == "addproperty" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "ADDPROPERTY()"}}));
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        if (parse_native_list_control_list_member_cell(*runtime_object, property_name).has_value()) {
            return make_boolean_value(false);
        }
        if (is_native_identity_member_name(*runtime_object, property_name) ||
            is_native_controlcount_member_name(*runtime_object, property_name) ||
            is_native_pagecount_member_name(*runtime_object, property_name) ||
            is_native_child_collection_member_name(*runtime_object, property_name) ||
            is_native_name_member_name(*runtime_object, property_name) ||
            is_native_form_alwaysontop_member_name(*runtime_object, property_name) ||
            is_native_form_showwindow_member_name(*runtime_object, property_name) ||
            is_native_form_windowtype_member_name(*runtime_object, property_name) ||
            is_native_form_windowstate_member_name(*runtime_object, property_name) ||
            is_native_form_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_titlebar_member_name(*runtime_object, property_name) ||
            is_native_form_desktop_member_name(*runtime_object, property_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_form_lockscreen_member_name(*runtime_object, property_name) ||
            is_native_form_controlbox_member_name(*runtime_object, property_name) ||
            is_native_form_closable_member_name(*runtime_object, property_name) ||
            is_native_form_minbutton_member_name(*runtime_object, property_name) ||
            is_native_form_maxbutton_member_name(*runtime_object, property_name) ||
            is_native_form_autocenter_member_name(*runtime_object, property_name) ||
            is_native_visual_enabled_member_name(*runtime_object, property_name) ||
            is_native_visual_visible_member_name(*runtime_object, property_name) ||
            is_native_visual_backcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_forecolor_member_name(*runtime_object, property_name) ||
            is_native_visual_geometry_member_name(*runtime_object, property_name) ||
            is_native_combobox_style_member_name(*runtime_object, property_name) ||
            is_native_control_readonly_member_name(*runtime_object, property_name) ||
            is_native_string_control_value_member_name(*runtime_object, property_name) ||
            is_native_controlsource_member_name(*runtime_object, property_name) ||
            is_native_allowaddnew_member_name(*runtime_object, property_name) ||
            is_native_allowcellselection_member_name(*runtime_object, property_name) ||
            is_native_gridlines_member_name(*runtime_object, property_name) ||
            is_native_highlight_member_name(*runtime_object, property_name) ||
            is_native_highlightrow_member_name(*runtime_object, property_name) ||
            is_native_deletemark_member_name(*runtime_object, property_name) ||
            is_native_splitbar_member_name(*runtime_object, property_name) ||
            is_native_currentcontrol_member_name(*runtime_object, property_name) ||
            is_native_dynamiccurrentcontrol_member_name(*runtime_object, property_name) ||
            is_native_columnorder_member_name(*runtime_object, property_name) ||
            is_native_recordsource_member_name(*runtime_object, property_name) ||
            is_native_leftcolumn_member_name(*runtime_object, property_name) ||
            is_native_recordmark_member_name(*runtime_object, property_name) ||
            is_native_recordsourcetype_member_name(*runtime_object, property_name) ||
            is_native_rowsource_member_name(*runtime_object, property_name) ||
            is_native_rowsourcetype_member_name(*runtime_object, property_name) ||
            is_native_listindex_member_name(*runtime_object, property_name) ||
            is_native_displayvalue_member_name(*runtime_object, property_name) ||
            is_native_listcount_member_name(*runtime_object, property_name) ||
            is_native_sorted_member_name(*runtime_object, property_name) ||
            is_native_multiselect_member_name(*runtime_object, property_name) ||
            is_native_newindex_member_name(*runtime_object, property_name) ||
            is_native_newitemid_member_name(*runtime_object, property_name) ||
            is_native_listitemid_member_name(*runtime_object, property_name) ||
            is_native_boundcolumn_member_name(*runtime_object, property_name) ||
            is_native_columncount_member_name(*runtime_object, property_name) ||
            is_native_column_bound_member_name(*runtime_object, property_name) ||
            is_native_columnwidths_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, property_name) ||
            native_child_parent_member_name_matches(*runtime_object, property_name) ||
            is_native_collection_member_name(*runtime_object, property_name)) {
            return make_boolean_value(false);
        }
        if (!reflectable_member_exists_locally(*runtime_object, property_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr &&
                reflectable_member_exists_locally(*object_surface, property_name)) {
                return make_boolean_value(false);
            }
        }
        const PrgValue initial_value = arguments.size() >= 3U ? arguments[2] : make_empty_value();
        runtime_object->properties[property_name] = initial_value;
        return make_boolean_value(true);
    }

    if (function == "getpem" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "GETPEM()"}}));
            return make_empty_value();
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_empty_value();
        }
        if (const auto list_cell = parse_native_list_control_list_member_cell(*runtime_object, member_name);
            list_cell.has_value()) {
            return *read_native_list_control_cell(
                *runtime_object,
                list_cell->row_slot,
                list_cell->column_slot);
        }
        if (is_native_listcount_member_name(*runtime_object, member_name)) {
            sync_native_list_control_count(*runtime_object);
        }
        if (is_native_listitemid_member_name(*runtime_object, member_name)) {
            sync_native_list_control_displayvalue_from_selection(*runtime_object);
            const auto property = runtime_object->properties.find(member_name);
            if (property != runtime_object->properties.end()) {
                return property->second;
            }
        }
        if (const auto metadata_value = get_native_identity_reflection_metadata(*runtime_object, member_name);
            metadata_value.has_value()) {
            return *metadata_value;
        }
        if (const auto collection_value = read_native_collection_member(*runtime_object, member_name);
            collection_value.has_value()) {
            return *collection_value;
        }
        if (runtime_object->source.empty()) {
            const auto prop_it = runtime_object->properties.find(member_name);
            if (prop_it != runtime_object->properties.end()) {
                return prop_it->second;
            }
        }
        if (!runtime_object->source.empty()) {
            if (read_native_member_callback) {
                const auto member_value = read_native_member_callback(arguments[0], member_name);
                if (member_value.has_value()) {
                    return *member_value;
                }
            }
            if (object_has_accessor_property(*runtime_object, member_name)) {
                return make_boolean_value(true);
            }
        } else if (object_has_accessor_property(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (is_native_collection_member_name(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            return make_boolean_value(true);
        }
        if (is_builtin_native_runtime_method_name(*runtime_object, member_name)) {
            return make_boolean_value(true);
        }
        if (!reflectable_member_exists_locally(*runtime_object, member_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                if (const auto metadata_value =
                        get_native_identity_reflection_metadata(*object_surface, member_name);
                    metadata_value.has_value()) {
                    return *metadata_value;
                }
                if (const auto collection_value =
                        read_native_collection_member(*object_surface, member_name);
                    collection_value.has_value()) {
                    return *collection_value;
                }
                const auto property = object_surface->properties.find(member_name);
                if (property != object_surface->properties.end()) {
                    return property->second;
                }
                if (object_has_accessor_property(*object_surface, member_name) ||
                    is_native_collection_member_name(*object_surface, member_name) ||
                    is_builtin_native_runtime_method_name(*object_surface, member_name) ||
                    object_has_member(object_surface->methods, member_name) ||
                    object_has_member(object_surface->events, member_name)) {
                    return make_boolean_value(true);
                }
            }
        }
        return make_empty_value();
    }
    
    if (function == "setpem" && arguments.size() >= 3U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "SETPEM()"}}));
            return make_boolean_value(false);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        const std::string member_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (runtime_object == nullptr || member_name.empty()) {
            return make_boolean_value(false);
        }
        if (is_scripting_dictionary_object(*runtime_object) && member_name == "count") {
            return make_boolean_value(false);
        }
        if (const auto list_cell = parse_native_list_control_list_member_cell(*runtime_object, member_name);
            list_cell.has_value()) {
            return make_boolean_value(
                write_native_list_control_cell(
                    *runtime_object,
                    list_cell->row_slot,
                    list_cell->column_slot,
                    arguments[2]));
        }
        if (native_child_parent_member_name_matches(*runtime_object, member_name) ||
            is_native_controlcount_member_name(*runtime_object, member_name) ||
            is_native_pagecount_member_name(*runtime_object, member_name) ||
            is_native_listcount_member_name(*runtime_object, member_name) ||
            is_native_newindex_member_name(*runtime_object, member_name) ||
            is_native_newitemid_member_name(*runtime_object, member_name) ||
            is_native_child_collection_member_name(*runtime_object, member_name) ||
            is_native_name_member_name(*runtime_object, member_name) ||
            is_native_splitbar_member_name(*runtime_object, member_name) ||
            is_native_leftcolumn_member_name(*runtime_object, member_name) ||
            is_native_form_desktop_member_name(*runtime_object, member_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, member_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, member_name) ||
            is_native_collection_readonly_member_name(*runtime_object, member_name)) {
            return make_boolean_value(false);
        }
        if (const auto selected_slot =
                parse_native_list_control_selected_member_slot(*runtime_object, member_name);
            selected_slot.has_value()) {
            return make_boolean_value(
                write_native_list_control_selected_slot(
                    *runtime_object,
                    *selected_slot,
                    arguments[2]));
        }
        if (const auto selected_item_id =
                parse_native_list_control_selectedid_member_item_id(*runtime_object, member_name);
            selected_item_id.has_value()) {
            return make_boolean_value(
                write_native_list_control_selected_item_id(
                    *runtime_object,
                    *selected_item_id,
                    arguments[2]));
        }
        if (is_native_listitemid_member_name(*runtime_object, member_name)) {
            return make_boolean_value(
                write_native_list_control_item_id(*runtime_object, arguments[2]));
        }
        if (!reflectable_member_exists_locally(*runtime_object, member_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
        }
        if (!runtime_object->source.empty() &&
            (object_has_assigner_property(*runtime_object, member_name) ||
             runtime_object->properties.contains(member_name) ||
             object_has_member(runtime_object->methods, member_name) ||
             object_has_member(runtime_object->events, member_name))) {
            return make_boolean_value(
                write_native_member_callback &&
                write_native_member_callback(arguments[0], member_name, arguments[2]));
        }
        if (object_has_assigner_property(*runtime_object, member_name)) {
            return make_boolean_value(false);
        }
        if (object_has_member(runtime_object->methods, member_name) ||
            object_has_member(runtime_object->events, member_name)) {
            if (member_name == "columnorder" ||
                member_name == "columncount" ||
                is_native_column_bound_member_name(*runtime_object, member_name) ||
                is_native_controlsource_member_name(*runtime_object, member_name)) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
            if (member_name == "readonly" &&
                native_combobox_readonly_assignment_blocked(*runtime_object, arguments[2])) {
                return make_boolean_value(false);
            }
            runtime_object->properties[member_name] = arguments[2];
            if (member_name == "style" || member_name == "readonly") {
                normalize_native_combobox_readonly_invariant(*runtime_object);
            }
            if (member_name == "multiselect") {
                normalize_native_listbox_multiselect_invariant(*runtime_object);
            }
            if (member_name == "sorted" ||
                member_name == "rowsourcetype") {
                normalize_native_list_control_sorted_invariant(*runtime_object);
            }
            return make_boolean_value(true);
        }
        if (runtime_object->properties.contains(member_name)) {
            if (member_name == "columnorder" ||
                member_name == "columncount" ||
                is_native_column_bound_member_name(*runtime_object, member_name) ||
                is_native_controlsource_member_name(*runtime_object, member_name)) {
                return make_boolean_value(
                    write_native_member_callback &&
                    write_native_member_callback(arguments[0], member_name, arguments[2]));
            }
            if (member_name == "readonly" &&
                native_combobox_readonly_assignment_blocked(*runtime_object, arguments[2])) {
                return make_boolean_value(false);
            }
            runtime_object->properties[member_name] = arguments[2];
            if (member_name == "style" || member_name == "readonly") {
                normalize_native_combobox_readonly_invariant(*runtime_object);
            }
            if (member_name == "multiselect") {
                normalize_native_listbox_multiselect_invariant(*runtime_object);
            }
            if (member_name == "sorted" ||
                member_name == "rowsourcetype") {
                normalize_native_list_control_sorted_invariant(*runtime_object);
            }
            return make_boolean_value(true);
        }
        return make_boolean_value(false);
    }

    if (function == "removeproperty" && arguments.size() >= 2U) {
        if (!resolve_object_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback",
                {{"function", "REMOVEPROPERTY()"}}));
            return make_boolean_value(true);
        }
        RuntimeOleObjectState* runtime_object = resolve_object_callback(arguments[0]);
        if (runtime_object == nullptr) {
            return make_boolean_value(false);
        }
        const std::string property_name = normalize_identifier(trim_copy(value_as_string(arguments[1])));
        if (property_name.empty()) {
            return make_boolean_value(false);
        }
        if (parse_native_list_control_list_member_cell(*runtime_object, property_name).has_value()) {
            return make_boolean_value(false);
        }
        if (is_native_identity_member_name(*runtime_object, property_name) ||
            is_native_controlcount_member_name(*runtime_object, property_name) ||
            is_native_pagecount_member_name(*runtime_object, property_name) ||
            is_native_child_collection_member_name(*runtime_object, property_name) ||
            is_native_name_member_name(*runtime_object, property_name) ||
            is_native_form_alwaysontop_member_name(*runtime_object, property_name) ||
            is_native_form_showwindow_member_name(*runtime_object, property_name) ||
            is_native_form_windowtype_member_name(*runtime_object, property_name) ||
            is_native_form_windowstate_member_name(*runtime_object, property_name) ||
            is_native_form_borderstyle_member_name(*runtime_object, property_name) ||
            is_native_form_titlebar_member_name(*runtime_object, property_name) ||
            is_native_form_desktop_member_name(*runtime_object, property_name) ||
            is_native_form_scrollbars_member_name(*runtime_object, property_name) ||
            is_native_form_lockscreen_member_name(*runtime_object, property_name) ||
            is_native_form_controlbox_member_name(*runtime_object, property_name) ||
            is_native_form_closable_member_name(*runtime_object, property_name) ||
            is_native_form_minbutton_member_name(*runtime_object, property_name) ||
            is_native_form_maxbutton_member_name(*runtime_object, property_name) ||
            is_native_form_autocenter_member_name(*runtime_object, property_name) ||
            is_native_visual_enabled_member_name(*runtime_object, property_name) ||
            is_native_visual_visible_member_name(*runtime_object, property_name) ||
            is_native_visual_backcolor_member_name(*runtime_object, property_name) ||
            is_native_visual_forecolor_member_name(*runtime_object, property_name) ||
            is_native_visual_geometry_member_name(*runtime_object, property_name) ||
            is_native_combobox_style_member_name(*runtime_object, property_name) ||
            is_native_control_readonly_member_name(*runtime_object, property_name) ||
            is_native_string_control_value_member_name(*runtime_object, property_name) ||
            is_native_controlsource_member_name(*runtime_object, property_name) ||
            is_native_allowaddnew_member_name(*runtime_object, property_name) ||
            is_native_allowcellselection_member_name(*runtime_object, property_name) ||
            is_native_gridlines_member_name(*runtime_object, property_name) ||
            is_native_highlight_member_name(*runtime_object, property_name) ||
            is_native_highlightrow_member_name(*runtime_object, property_name) ||
            is_native_deletemark_member_name(*runtime_object, property_name) ||
            is_native_splitbar_member_name(*runtime_object, property_name) ||
            is_native_currentcontrol_member_name(*runtime_object, property_name) ||
            is_native_dynamiccurrentcontrol_member_name(*runtime_object, property_name) ||
            is_native_columnorder_member_name(*runtime_object, property_name) ||
            is_native_recordsource_member_name(*runtime_object, property_name) ||
            is_native_leftcolumn_member_name(*runtime_object, property_name) ||
            is_native_recordmark_member_name(*runtime_object, property_name) ||
            is_native_recordsourcetype_member_name(*runtime_object, property_name) ||
            is_native_rowsource_member_name(*runtime_object, property_name) ||
            is_native_rowsourcetype_member_name(*runtime_object, property_name) ||
            is_native_listindex_member_name(*runtime_object, property_name) ||
            is_native_displayvalue_member_name(*runtime_object, property_name) ||
            is_native_listcount_member_name(*runtime_object, property_name) ||
            is_native_sorted_member_name(*runtime_object, property_name) ||
            is_native_multiselect_member_name(*runtime_object, property_name) ||
            is_native_newindex_member_name(*runtime_object, property_name) ||
            is_native_newitemid_member_name(*runtime_object, property_name) ||
            is_native_listitemid_member_name(*runtime_object, property_name) ||
            is_native_boundcolumn_member_name(*runtime_object, property_name) ||
            is_native_columncount_member_name(*runtime_object, property_name) ||
            is_native_column_bound_member_name(*runtime_object, property_name) ||
            is_native_columnwidths_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_creation_time_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_object_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_inspection_member_name(*runtime_object, property_name) ||
            is_native_olecontrol_conflict_member_name(*runtime_object, property_name) ||
            native_child_parent_member_name_matches(*runtime_object, property_name) ||
            is_native_collection_member_name(*runtime_object, property_name)) {
            return make_boolean_value(false);
        }
        if (!runtime_object->properties.contains(property_name)) {
            if (RuntimeOleObjectState* object_surface =
                    resolve_direct_olecontrol_reflection_surface(*runtime_object);
                object_surface != nullptr &&
                reflectable_member_exists_locally(*object_surface, property_name)) {
                return make_boolean_value(false);
            }
        }
        const std::size_t removed = runtime_object->properties.erase(property_name);
        return make_boolean_value(removed != 0U);
    }

    if (function == "file" && !arguments.empty()) {
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(value_as_string(arguments[0]), default_directory, set_callback);
        return make_boolean_value(std::filesystem::exists(path, ignored));
    }
    if (function == "sys") {
        if (!arguments.empty()) {
            const long long sys_code = std::llround(value_as_number(arguments[0]));
            if (sys_code == 3) {
                return make_string_value("Copperfin Runtime 0.1");
            }
            if (sys_code == 5 || sys_code == 2003 || sys_code == 2004) {
                return make_string_value(default_directory);
            }
            if (sys_code == 7) {
                return make_string_value(host_os_name());
            }
            if (sys_code == 11) {
                return make_string_value("0");
            }
            if (sys_code == 13) {
                return make_string_value("0");
            }
            if (sys_code == 16) {
                return make_string_value(frame_file_path);
            }
            if (sys_code == 2018) {
                return make_string_value(uppercase_copy(runtime_error_parameter(last_error_message)));
            }
            if (sys_code == 2020) {
                return make_string_value(format_value(make_number_value(available_disk_space({}, default_directory))));
            }
            if (sys_code == 2023) {
                std::error_code ignored;
                return make_string_value(std::filesystem::temp_directory_path(ignored).string());
            }
            if (sys_code == 2326 && arguments.size() >= 2U && whandle_from_hwnd_callback) {
                const std::int64_t hwnd = safe_int64_argument(1U, 0);
                if (const auto whandle = whandle_from_hwnd_callback(hwnd); whandle.has_value()) {
                    return make_int64_value(*whandle);
                }
                return make_number_value(0.0);
            }
            if (sys_code == 2327 && arguments.size() >= 2U && hwnd_from_whandle_callback) {
                const std::int64_t whandle = safe_int64_argument(1U, 0);
                if (const auto hwnd = hwnd_from_whandle_callback(whandle); hwnd.has_value()) {
                    return make_int64_value(*hwnd);
                }
                return make_number_value(0.0);
            }
        }
        return make_string_value("0");
    }
    if (function == "home") {
        return make_string_value(default_directory);
    }
    if (function == "os") {
        return make_string_value(host_os_name());
    }
    if (function == "diskspace") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(available_disk_space(path, default_directory));
    }
    if (function == "drivetype") {
        const std::string path = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        return make_number_value(static_cast<double>(drive_type_value(path, default_directory)));
    }
    if (function == "filesize") {
        if (arguments.empty()) {
            return make_number_value(0.0);
        }
        std::error_code ignored;
        const std::filesystem::path path =
            resolve_runtime_file_probe_path(value_as_string(arguments[0]), default_directory, set_callback);
        if (!std::filesystem::exists(path, ignored)) {
            return make_number_value(0.0);
        }
        const auto size = std::filesystem::file_size(path, ignored);
        return make_number_value(ignored ? 0.0 : static_cast<double>(size));
    }
    if (function == "message") {
        return make_string_value(last_error_message);
    }
    if (function == "aerror" && !raw_arguments.empty()) {
        return make_number_value(static_cast<double>(aerror_callback(raw_arguments[0])));
    }
    if ((function == "eval" || function == "evaluate") && !arguments.empty()) {
        std::string expression_text = value_as_string(arguments[0]);
        std::string last_identifier_text;
        const auto expand_identifier_chain =
            [&](std::string expanded_text) {
                if (expanded_text.empty()) {
                    return expanded_text;
                }
                constexpr std::size_t max_macro_expression_depth = 16U;
                std::vector<std::string> visited_macros;
                visited_macros.reserve(8U);
                for (std::size_t depth = 0U; depth < max_macro_expression_depth; ++depth) {
                    const bool bare_identifier =
                        std::all_of(
                            expanded_text.begin(),
                            expanded_text.end(),
                            [](unsigned char ch) {
                                return std::isalnum(ch) != 0 || ch == '_';
                            });
                    if (!bare_identifier) {
                        break;
                    }
                    const std::string normalized_identifier = normalize_memory_variable_identifier(expanded_text);
                    if (std::find(visited_macros.begin(), visited_macros.end(), normalized_identifier) != visited_macros.end()) {
                        break;
                    }
                    visited_macros.push_back(normalized_identifier);
                    const std::string next_text =
                        trim_copy(value_as_string(eval_expression_callback(expanded_text)));
                    if (next_text.empty() || next_text == expanded_text) {
                        break;
                    }
                    last_identifier_text = expanded_text;
                    expanded_text = next_text;
                }
                return expanded_text;
            };
        if (!raw_arguments.empty()) {
            const std::string raw_text = trim_copy(raw_arguments[0]);
            if (!raw_text.empty() && raw_text.front() == '&') {
                const std::string macro_variable_text = trim_copy(raw_text.substr(1U));
                const bool simple_macro_variable =
                    !macro_variable_text.empty() &&
                    std::all_of(
                        macro_variable_text.begin(),
                        macro_variable_text.end(),
                        [](unsigned char ch) {
                            return std::isalnum(ch) != 0 || ch == '_';
                        });
                if (simple_macro_variable) {
                    expression_text =
                        trim_copy(value_as_string(eval_expression_callback(macro_variable_text)));
                    expression_text = expand_identifier_chain(expression_text);
                }
            } else if (std::all_of(
                           raw_text.begin(),
                           raw_text.end(),
                           [](unsigned char ch) {
                               return std::isalnum(ch) != 0 || ch == '_';
                           })) {
                const std::string expanded_text = expand_identifier_chain(expression_text);
                if (!last_identifier_text.empty()) {
                    expression_text = last_identifier_text;
                } else {
                    expression_text = expanded_text;
                }
            }
        }
        PrgValue evaluated = eval_expression_callback(expression_text);
        const bool empty_string_result =
            evaluated.kind == PrgValueKind::string && value_as_string(evaluated).empty();
        if ((evaluated.kind == PrgValueKind::empty || empty_string_result) &&
            !last_identifier_text.empty() &&
            expression_text != last_identifier_text) {
            evaluated = eval_expression_callback(last_identifier_text);
        }
        return evaluated;
    }
    if (function == "cursortoxml") {
        const std::string cursor_designator = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        const std::string output_target = arguments.size() >= 2U ? trim_copy(value_as_string(arguments[1])) : std::string{};
        if (!snapshot_cursor_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback",
                {
                    {"capability", "cursor snapshot"},
                    {"function", "CURSORTOXML()"}
                }));
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> snapshot = snapshot_cursor_callback(cursor_designator);
        if (!snapshot.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlTargetUnreadable",
                {{"function", "CURSORTOXML()"}}));
            if (!output_target.empty() && looks_like_file_path(output_target)) {
                return make_boolean_value(false);
            }
            return make_string_value(std::string{});
        }

        const std::string xml_payload = serialize_cursor_snapshot_xml(*snapshot);
        if (record_event_callback) {
            record_event_callback(
                "runtime.cursortoxml",
                snapshot->alias + " rows=" + std::to_string(snapshot->rows.size()));
        }

        if (output_target.empty() || !looks_like_file_path(output_target)) {
            return make_string_value(xml_payload);
        }

        std::error_code ignored;
        std::filesystem::path output_path(output_target);
        if (output_path.is_relative()) {
            output_path = std::filesystem::path(default_directory) / output_path;
        }
        output_path = output_path.lexically_normal();
        std::filesystem::create_directories(output_path.parent_path(), ignored);
        std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
        output << xml_payload;
        output.close();
        if (!output.good()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlWriteFailed",
                {{"function", "CURSORTOXML()"}}));
            return make_boolean_value(false);
        }
        return make_boolean_value(true);
    }
    if (function == "xmltocursor") {
        if (arguments.size() < 2U) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorInputAndAliasRequired",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }
        const std::string xml_or_path = value_as_string(arguments[0]);
        const std::string destination_alias = trim_copy(value_as_string(arguments[1]));
        if (destination_alias.empty()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorDestinationAliasRequired",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }
        if (!load_cursor_snapshot_callback) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback",
                {
                    {"capability", "cursor load"},
                    {"function", "XMLTOCURSOR()"}
                }));
            return make_number_value(0.0);
        }

        std::string xml_payload = xml_or_path;
        std::error_code ignored;
        std::filesystem::path probe_path(xml_or_path);
        if (looks_like_file_path(xml_or_path)) {
            if (probe_path.is_relative()) {
                probe_path = std::filesystem::path(default_directory) / probe_path;
            }
            probe_path = probe_path.lexically_normal();
            if (std::filesystem::exists(probe_path, ignored)) {
                std::ifstream input(probe_path, std::ios::binary);
                std::ostringstream buffer;
                buffer << input.rdbuf();
                xml_payload = buffer.str();
            }
        }

        const std::optional<RuntimeSurfaceCursorSnapshot> parsed = parse_cursor_snapshot_xml(xml_payload);
        if (!parsed.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }

        std::optional<std::size_t> loaded_count = load_cursor_snapshot_callback(*parsed, destination_alias);
        if (!loaded_count.has_value()) {
            record_runtime_warning(runtime_text(
                "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorMaterializeFailed",
                {{"function", "XMLTOCURSOR()"}}));
            return make_number_value(0.0);
        }

        if (record_event_callback) {
            record_event_callback(
                "runtime.xmltocursor",
                destination_alias + " rows=" + std::to_string(*loaded_count));
        }
        return make_number_value(static_cast<double>(*loaded_count));
    }
    if (function == "set" && !arguments.empty()) {
        return make_string_value(set_callback(value_as_string(arguments[0])));
    }
    if (function == "error") {
        return make_number_value(static_cast<double>(last_error_code));
    }
    if (function == "program") {
        return make_string_value(last_error_procedure);
    }
    if (function == "lineno") {
        return make_number_value(static_cast<double>(last_error_line));
    }
    if (function == "version") {
        return make_number_value(arguments.empty() ? 9.0 : 0.0);
    }
    if (function == "on" && !arguments.empty()) {
        const std::string topic = uppercase_copy(value_as_string(arguments[0]));
        if (topic == "ERROR") {
            return make_string_value(error_handler);
        }
        if (topic == "SHUTDOWN") {
            return make_string_value(shutdown_handler);
        }
        return make_string_value(std::string{});
    }
    if (function == "messagebox" && !arguments.empty()) {
        return make_number_value(1.0);
    }

    if (function == "cast" && !arguments.empty()) {
        std::string type_name;
        if (!raw_arguments.empty()) {
            const std::string raw = uppercase_copy(raw_arguments[0]);
            const auto as_pos = raw.rfind(" AS ");
            if (as_pos != std::string::npos) {
                type_name = trim_copy(raw.substr(as_pos + 4U));
            }
        }

        const PrgValue source = arguments[0];
        if (type_name == "INT64" || type_name == "LONGLONG" || type_name == "BIGINT") {
            return make_int64_value(static_cast<std::int64_t>(value_as_number(source)));
        }
        if (type_name == "UINT64" || type_name == "ULONGLONG" || type_name == "UBIGINT") {
            return make_uint64_value(static_cast<std::uint64_t>(value_as_number(source)));
        }
        if (type_name == "INT" || type_name == "INT32" || type_name == "INTEGER" ||
            type_name == "LONG" || type_name == "INT16" || type_name == "SHORT") {
            return make_int64_value(static_cast<std::int64_t>(std::trunc(value_as_number(source))));
        }
        if (type_name == "BYTE" || type_name == "UINT8") {
            return make_uint64_value(
                static_cast<std::uint64_t>(value_as_number(source)) & 0xFFULL);
        }
        if (type_name == "FLOAT" || type_name == "SINGLE") {
            return make_number_value(
                static_cast<double>(static_cast<float>(value_as_number(source))));
        }
        if (type_name == "DOUBLE" || type_name == "NUMERIC") {
            return make_number_value(value_as_number(source));
        }
        if (type_name == "STRING" || type_name == "CHAR" || type_name == "VARCHAR" ||
            type_name == "CHARACTER") {
            return make_string_value(value_as_string(source));
        }
        if (type_name == "LOGICAL" || type_name == "BOOL" || type_name == "BOOLEAN") {
            return make_boolean_value(value_as_bool(source));
        }
        return source;
    }

    if (function == "bitand" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result &= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result |= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitxor" && arguments.size() >= 2U) {
        std::uint32_t result = bitwise_value(arguments[0]);
        for (std::size_t index = 1U; index < arguments.size(); ++index) {
            result ^= bitwise_value(arguments[index]);
        }
        return make_int64_value(signed_bitwise_result(result));
    }
    if (function == "bitnot" && !arguments.empty()) {
        return make_int64_value(signed_bitwise_result(~bitwise_value(arguments[0])));
    }
    if (function == "bitclear" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value & ~mask));
    }
    if (function == "bitset" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_int64_value(signed_bitwise_result(value | mask));
    }
    if (function == "bittest" && arguments.size() >= 2U) {
        const std::uint32_t value = bitwise_value(arguments[0]);
        const std::uint32_t mask = 1U << bit_position(arguments[1]);
        return make_boolean_value((value & mask) != 0U);
    }
    if (function == "bitlshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value << count);
    }
    if (function == "bitrshift" && arguments.size() >= 2U) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int count = static_cast<int>(value_as_number(arguments[1]));
        return make_int64_value(value >> count);
    }

    if (function == "bintoc" && !arguments.empty()) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const int width = arguments.size() >= 2U
                              ? static_cast<int>(value_as_number(arguments[1]))
                              : 4;
        std::string result(static_cast<std::size_t>(std::max(width, 0)), '\0');
        std::uint64_t unsigned_value = static_cast<std::uint64_t>(value);
        for (int index = 0; index < width; ++index) {
            result[static_cast<std::size_t>(index)] =
                static_cast<char>(unsigned_value & 0xFFU);
            unsigned_value >>= 8;
        }
        return make_string_value(std::move(result));
    }
    if (function == "ctobin" && !arguments.empty()) {
        const std::string source = value_as_string(arguments[0]);
        const std::string type = arguments.size() >= 2U
                                     ? uppercase_copy(value_as_string(arguments[1]))
                                     : std::string("N");
        std::uint64_t unsigned_value = 0U;
        for (std::size_t index = source.size(); index-- > 0U;) {
            unsigned_value = (unsigned_value << 8) |
                             static_cast<std::uint8_t>(source[index]);
        }
        if (type == "N" || type == "INTEGER" || type == "INT") {
            return make_int64_value(static_cast<std::int64_t>(unsigned_value));
        }
        return make_uint64_value(unsigned_value);
    }

    if (function == "numlock" || function == "capslock" || function == "scrolllock") {
        return make_boolean_value(false);
    }
    if (function == "cursorsetprop" || function == "cursorgetprop") {
        return make_number_value(0.0);
    }
    // NEWID([cDatabase]) — generate a unique identifier string (UUID v4-style, no braces)
    if (function == "newid") {
        static thread_local std::mt19937_64 uuid_gen{std::random_device{}()};
        std::uniform_int_distribution<std::uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());
        const std::uint64_t hi = dist(uuid_gen);
        const std::uint64_t lo = dist(uuid_gen);
        // Set UUID v4 bits
        const std::uint64_t hi4 = (hi & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        const std::uint64_t lo4 = (lo & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        std::ostringstream oss;
        oss << std::uppercase << std::hex << std::setfill('0')
            << std::setw(8) << ((hi4 >> 32) & 0xFFFFFFFFU) << '-'
            << std::setw(4) << ((hi4 >> 16) & 0xFFFFU) << '-'
            << std::setw(4) << (hi4 & 0xFFFFU) << '-'
            << std::setw(4) << ((lo4 >> 48) & 0xFFFFU) << '-'
            << std::setw(12) << (lo4 & 0x0000FFFFFFFFFFFFULL);
        return make_string_value(oss.str());
    }
    // VFP9 help (dv_foxhelp.chm, CPCURRENT() topic):
    // - omitted/0 => configured VFP code page, or current OS code page when no
    //   CODEPAGE config item is in effect
    // - 1 => current OS code page regardless of CODEPAGE config
    // - 2 => underlying OS code page (MS-DOS/OEM on Windows)
    //
    // Copperfin's runtime does not yet project a CODEPAGE config item, so omitted
    // and 0 explicitly read back the current host code page.
    if (function == "cpcurrent") {
        const int type_flag = arguments.empty() ? 0 : static_cast<int>(std::llround(value_as_number(arguments[0])));
        if (type_flag == 2) {
            return make_number_value(static_cast<double>(current_host_oem_code_page()));
        }
        return make_number_value(static_cast<double>(current_host_code_page()));
    }
    // VFP9 help (dv_foxhelp.chm, CPCONVERT() topic): convert a character expression
    // from one explicit code page to another.
    //
    // Copperfin preserves same-codepage calls as identity. For supported VFP code pages,
    // it uses host conversion APIs where available; otherwise it falls back to the
    // original byte sequence unchanged.
    if (function == "cpconvert" && arguments.size() >= 3U) {
        const int source_code_page = safe_int_argument(0, 0);
        const int target_code_page = safe_int_argument(1, 0);
        const std::string input = value_as_string(arguments[2]);
        if (source_code_page == target_code_page) {
            return make_string_value(input);
        }
        if (!is_supported_vfp_code_page(source_code_page) ||
            !is_supported_vfp_code_page(target_code_page)) {
            return make_string_value(input);
        }
        if (const auto converted =
                convert_between_host_code_pages(source_code_page, target_code_page, input);
            converted.has_value()) {
            return make_string_value(*converted);
        }
        return make_string_value(input);
    }
    // VFP9 help (dv_foxhelp.chm, CPDBF() topic):
    // - omitted => code page of the table in the current work area
    // - nWorkArea => code page of that work area, returning 0 when no table is open
    // - cTableAlias => code page of that alias, raising an error when no such alias exists
    //
    // Copperfin projects the DBF header code-page mark when the selected cursor is
    // table-backed. Synthetic/remote cursors and missing header metadata fall back to 0.
    if (function == "cpdbf") {
        const bool explicit_alias = !arguments.empty() &&
            arguments[0].kind == PrgValueKind::string;
        const std::string cursor_designator = arguments.empty()
            ? std::string{}
            : value_as_string(arguments[0]);
        const std::optional<RuntimeSurfaceCursorSnapshot> snapshot =
            snapshot_cursor_callback ? snapshot_cursor_callback(cursor_designator) : std::nullopt;
        if (!snapshot.has_value()) {
            if (explicit_alias) {
                throw std::runtime_error(runtime_text(
                    "Runtime.Prg.RuntimeSurface.Error.CpDbfAliasNotFound",
                    {{"alias", cursor_designator}}));
            }
            return make_number_value(0.0);
        }

        return make_number_value(static_cast<double>(snapshot->code_page.value_or(0)));
    }
    // GETPICT([cTitle [, cFileName]]) — headless contract: emit payload and preserve
    // the current selection when the host does not provide a replacement.
    if (function == "getpict") {
        const std::string title = arguments.empty() ? std::string{} : value_as_string(arguments[0]);
        const std::string current_file = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " title=" << std::quoted(title);
            detail << " current=" << std::quoted(current_file);
            detail << " result=" << std::quoted(current_file);
            record_event_callback("runtime.getpict", detail.str());
        }
        return make_string_value(current_file);
    }
    // GETCOLOR([nDefaultColor [, cTitle]]) — headless contract: emit payload and
    // preserve the provided default color when the host does not override it.
    if (function == "getcolor") {
        const double default_color = arguments.empty() ? 0.0 : value_as_number(arguments[0]);
        const std::string title = arguments.size() >= 2U ? value_as_string(arguments[1]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " default=" << std::llround(default_color);
            detail << " title=" << std::quoted(title);
            detail << " result=" << std::llround(default_color);
            record_event_callback("runtime.getcolor", detail.str());
        }
        return make_number_value(default_color);
    }
    // GETFONT([cFontName [, nFontSize [, cFontStyle]]]) — headless contract: emit
    // payload and preserve the provided current font when the host does not override it.
    if (function == "getfont") {
        const std::string font_name = !arguments.empty() ? value_as_string(arguments[0]) : std::string{};
        const long long font_size =
            arguments.size() >= 2U ? std::llround(value_as_number(arguments[1])) : 0LL;
        const std::string font_style = arguments.size() >= 3U ? value_as_string(arguments[2]) : std::string{};
        if (record_event_callback) {
            std::ostringstream detail;
            detail << "mode=headless";
            detail << " name=" << std::quoted(font_name);
            if (arguments.size() >= 2U) {
                detail << " size=" << font_size;
            }
            if (arguments.size() >= 3U) {
                detail << " style=" << std::quoted(font_style);
            }
            detail << " result=" << std::quoted(font_name);
            record_event_callback("runtime.getfont", detail.str());
        }
        return make_string_value(font_name);
    }
    // VARREAD() — name of variable currently being read/edited (interactive mode only).
    // Headless contract: report that no interactive read is active and return "".
    if (function == "varread") {
        if (record_event_callback) {
            record_event_callback("runtime.varread", "mode=headless active=false result=\"\"");
        }
        return make_string_value({});
    }

    return std::nullopt;
}

}  // namespace copperfin::runtime
