#include "prg_engine_runtime_surface_functions.h"

#include "prg_engine_helpers.h"

#include <filesystem>
#include <cmath>
#include <cstdint>
#include <stdexcept>

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
    if (function == "file" && !arguments.empty()) {
        std::filesystem::path path(value_as_string(arguments[0]));
        if (path.is_relative()) {
            path = std::filesystem::path(default_directory) / path;
        }
        return make_boolean_value(std::filesystem::exists(path));
    }
    if (function == "sys") {
        if (!arguments.empty()) {
            const long long sys_code = std::llround(value_as_number(arguments[0]));
            if (sys_code == 16) {
                return make_string_value(frame_file_path);
            }
            if (sys_code == 2018) {
                return make_string_value(uppercase_copy(runtime_error_parameter(last_error_message)));
            }
        }
        return make_string_value("0");
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
