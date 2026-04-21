#include "prg_engine_runtime_surface_functions.h"

#include "prg_engine_helpers.h"

#include <cmath>
#include <cstdint>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_runtime_surface_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::vector<std::string>& raw_arguments) {
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
        const auto left = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const auto right = static_cast<std::int64_t>(value_as_number(arguments[1]));
        return make_int64_value(left & right);
    }
    if (function == "bitor" && arguments.size() >= 2U) {
        const auto left = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const auto right = static_cast<std::int64_t>(value_as_number(arguments[1]));
        return make_int64_value(left | right);
    }
    if (function == "bitxor" && arguments.size() >= 2U) {
        const auto left = static_cast<std::int64_t>(value_as_number(arguments[0]));
        const auto right = static_cast<std::int64_t>(value_as_number(arguments[1]));
        return make_int64_value(left ^ right);
    }
    if (function == "bitnot" && !arguments.empty()) {
        const auto value = static_cast<std::int64_t>(value_as_number(arguments[0]));
        return make_int64_value(~value);
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