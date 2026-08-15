// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/process_arguments.h"

#include <string_view>
#include <utility>

namespace copperfin::platform {
namespace {

SerializedProcessArguments denied(
    const ProcessArgumentTarget target,
    std::string diagnostic_code) {
    SerializedProcessArguments result;
    result.target = target;
    result.diagnostic_code = std::move(diagnostic_code);
    return result;
}

bool contains_nul(const std::string_view value) noexcept {
    return value.find('\0') != std::string_view::npos;
}

bool can_add(
    const std::size_t current,
    const std::size_t addition,
    const std::size_t maximum) noexcept {
    return current <= maximum && addition <= maximum - current;
}

void append_utf16_scalar(std::u16string& output, const std::uint32_t scalar) {
    if (scalar <= 0xffffU) {
        output.push_back(static_cast<char16_t>(scalar));
        return;
    }
    const std::uint32_t adjusted = scalar - 0x10000U;
    output.push_back(static_cast<char16_t>(0xd800U + (adjusted >> 10U)));
    output.push_back(static_cast<char16_t>(0xdc00U + (adjusted & 0x3ffU)));
}

enum class Utf8ConversionResult {
    ok,
    invalid,
    size_limit_exceeded
};

Utf8ConversionResult append_utf8_as_utf16(
    const std::string_view input,
    std::u16string& output,
    const std::size_t maximum_code_units) {
    std::size_t offset = 0U;
    while (offset < input.size()) {
        const auto lead = static_cast<unsigned char>(input[offset]);
        std::size_t continuation_count = 0U;
        std::uint32_t scalar = 0U;
        if (lead <= 0x7fU) {
            scalar = lead;
        } else if (lead >= 0xc2U && lead <= 0xdfU) {
            continuation_count = 1U;
            scalar = lead & 0x1fU;
        } else if (lead >= 0xe0U && lead <= 0xefU) {
            continuation_count = 2U;
            scalar = lead & 0x0fU;
        } else if (lead >= 0xf0U && lead <= 0xf4U) {
            continuation_count = 3U;
            scalar = lead & 0x07U;
        } else {
            return Utf8ConversionResult::invalid;
        }
        if (continuation_count > input.size() - offset - 1U) {
            return Utf8ConversionResult::invalid;
        }
        for (std::size_t index = 1U; index <= continuation_count; ++index) {
            const auto continuation =
                static_cast<unsigned char>(input[offset + index]);
            if ((continuation & 0xc0U) != 0x80U) {
                return Utf8ConversionResult::invalid;
            }
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if ((continuation_count == 1U && scalar < 0x80U) ||
            (continuation_count == 2U && scalar < 0x800U) ||
            (continuation_count == 3U && scalar < 0x10000U) ||
            scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return Utf8ConversionResult::invalid;
        }
        const std::size_t units = scalar <= 0xffffU ? 1U : 2U;
        if (!can_add(output.size(), units, maximum_code_units)) {
            return Utf8ConversionResult::size_limit_exceeded;
        }
        append_utf16_scalar(output, scalar);
        offset += continuation_count + 1U;
    }
    return Utf8ConversionResult::ok;
}

bool append_repeated(
    std::u16string& output,
    const char16_t value,
    const std::size_t count,
    const std::size_t maximum) {
    if (!can_add(output.size(), count, maximum)) {
        return false;
    }
    output.append(count, value);
    return true;
}

bool append_one(
    std::u16string& output,
    const char16_t value,
    const std::size_t maximum) {
    return append_repeated(output, value, 1U, maximum);
}

bool append_windows_quoted_argument(
    const std::u16string& value,
    std::u16string& output,
    const std::size_t maximum) {
    if (!append_one(output, u'"', maximum)) {
        return false;
    }
    std::size_t backslashes = 0U;
    for (const char16_t character : value) {
        if (character == u'\\') {
            ++backslashes;
            continue;
        }
        if (character == u'"') {
            if (!append_repeated(output, u'\\', backslashes, maximum) ||
                !append_repeated(output, u'\\', backslashes, maximum) ||
                !append_one(output, u'\\', maximum) ||
                !append_one(output, u'"', maximum)) {
                return false;
            }
            backslashes = 0U;
            continue;
        }
        if (!append_repeated(output, u'\\', backslashes, maximum) ||
            !append_one(output, character, maximum)) {
            return false;
        }
        backslashes = 0U;
    }
    return append_repeated(output, u'\\', backslashes, maximum) &&
        append_repeated(output, u'\\', backslashes, maximum) &&
        append_one(output, u'"', maximum);
}

}  // namespace

SerializedProcessArguments serialize_process_arguments(
    const std::string& executable_argument,
    const std::vector<std::string>& arguments,
    const ProcessArgumentTarget target,
    const std::size_t maximum_serialized_units) {
    if (maximum_serialized_units == 0U ||
        (target != ProcessArgumentTarget::posix_v1 &&
         target != ProcessArgumentTarget::windows_command_line_v1)) {
        return denied(target, "platform.process_arguments.invalid_policy");
    }
    if (executable_argument.empty()) {
        return denied(target, "platform.process_arguments.empty_executable");
    }
    if (contains_nul(executable_argument)) {
        return denied(target, "platform.process_arguments.embedded_nul");
    }
    for (const auto& argument : arguments) {
        if (contains_nul(argument)) {
            return denied(target, "platform.process_arguments.embedded_nul");
        }
    }

    SerializedProcessArguments result;
    result.target = target;
    if (target == ProcessArgumentTarget::posix_v1) {
        std::size_t total = 0U;
        result.posix_arguments.reserve(arguments.size() + 1U);
        const auto append = [&](const std::string& argument) {
            if (!can_add(total, argument.size(), maximum_serialized_units) ||
                !can_add(total + argument.size(), 1U, maximum_serialized_units)) {
                return false;
            }
            result.posix_arguments.push_back(argument);
            total += argument.size() + 1U;
            return true;
        };
        if (!append(executable_argument)) {
            return denied(target, "platform.process_arguments.size_limit_exceeded");
        }
        for (const auto& argument : arguments) {
            if (!append(argument)) {
                return denied(target, "platform.process_arguments.size_limit_exceeded");
            }
        }
    } else {
        // Reserve the implicit std::u16string terminator passed to
        // CreateProcessW before performing any input-sized work.
        const std::size_t maximum_payload = maximum_serialized_units - 1U;
        const auto append = [&](const std::string& argument, const bool first) {
            if (!first &&
                !append_one(result.windows_command_line, u' ', maximum_payload)) {
                return Utf8ConversionResult::size_limit_exceeded;
            }
            std::u16string converted;
            const auto conversion =
                append_utf8_as_utf16(argument, converted, maximum_payload);
            if (conversion != Utf8ConversionResult::ok) {
                return conversion;
            }
            if (!append_windows_quoted_argument(
                    converted, result.windows_command_line, maximum_payload)) {
                return Utf8ConversionResult::size_limit_exceeded;
            }
            return Utf8ConversionResult::ok;
        };
        auto conversion = append(executable_argument, true);
        if (conversion == Utf8ConversionResult::invalid) {
            return denied(target, "platform.process_arguments.invalid_utf8");
        }
        if (conversion == Utf8ConversionResult::size_limit_exceeded) {
            return denied(target, "platform.process_arguments.size_limit_exceeded");
        }
        for (const auto& argument : arguments) {
            conversion = append(argument, false);
            if (conversion == Utf8ConversionResult::invalid) {
                return denied(target, "platform.process_arguments.invalid_utf8");
            }
            if (conversion == Utf8ConversionResult::size_limit_exceeded) {
                return denied(target, "platform.process_arguments.size_limit_exceeded");
            }
        }
    }
    result.ok = true;
    result.diagnostic_code = "platform.process_arguments.serialized";
    return result;
}

}  // namespace copperfin::platform
