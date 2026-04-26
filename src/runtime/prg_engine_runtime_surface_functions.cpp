#include "prg_engine_runtime_surface_functions.h"

#include "prg_engine_file_io_functions.h"
#include "prg_engine_helpers.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>
#include <system_error>

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
        throw std::runtime_error("Bit position must be between 0 and 31");
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

}  // namespace

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
    const std::function<int(const std::string&)>& aerror_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback) {
    if (const auto file_io_result = evaluate_file_io_function(function, arguments, default_directory)) {
        return file_io_result;
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
            if (sys_code == 5 || sys_code == 2003 || sys_code == 2004) {
                return make_string_value(default_directory);
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
        return eval_expression_callback(value_as_string(arguments[0]));
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
        return make_string_value(uppercase_copy(value_as_string(arguments[0])) == "ERROR" ? error_handler : std::string{});
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

    return std::nullopt;
}

}  // namespace copperfin::runtime
