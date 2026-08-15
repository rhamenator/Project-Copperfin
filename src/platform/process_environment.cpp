// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/platform/process_environment.h"

#include <algorithm>
#include <limits>
#include <string_view>
#include <utility>

namespace copperfin::platform {
namespace {

SerializedProcessEnvironment denied(
    const ProcessEnvironmentTarget target,
    std::string diagnostic_code) {
    SerializedProcessEnvironment result;
    result.target = target;
    result.diagnostic_code = std::move(diagnostic_code);
    return result;
}

bool is_name_character(const char character, const bool first) noexcept {
    return (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z') || character == '_' ||
        (!first && character >= '0' && character <= '9');
}

bool valid_name(const std::string_view name) noexcept {
    if (name.empty() || !is_name_character(name.front(), true)) {
        return false;
    }
    return std::all_of(
        name.begin() + 1, name.end(),
        [](const char character) { return is_name_character(character, false); });
}

char ascii_lower(const char character) noexcept {
    return character >= 'A' && character <= 'Z'
        ? static_cast<char>(character - 'A' + 'a')
        : character;
}

bool names_equal(
    const std::string_view left,
    const std::string_view right,
    const ProcessEnvironmentTarget target) noexcept {
    if (left.size() != right.size()) {
        return false;
    }
    if (target == ProcessEnvironmentTarget::posix_v1) {
        return left == right;
    }
    for (std::size_t index = 0U; index < left.size(); ++index) {
        if (ascii_lower(left[index]) != ascii_lower(right[index])) {
            return false;
        }
    }
    return true;
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

enum class Utf8AppendResult {
    ok,
    invalid,
    size_limit_exceeded
};

Utf8AppendResult append_utf8_as_utf16(
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
            return Utf8AppendResult::invalid;
        }
        if (continuation_count > input.size() - offset - 1U) {
            return Utf8AppendResult::invalid;
        }
        for (std::size_t index = 1U; index <= continuation_count; ++index) {
            const auto continuation =
                static_cast<unsigned char>(input[offset + index]);
            if ((continuation & 0xc0U) != 0x80U) {
                return Utf8AppendResult::invalid;
            }
            scalar = (scalar << 6U) | (continuation & 0x3fU);
        }
        if ((continuation_count == 1U && scalar < 0x80U) ||
            (continuation_count == 2U && scalar < 0x800U) ||
            (continuation_count == 3U && scalar < 0x10000U) ||
            scalar > 0x10ffffU ||
            (scalar >= 0xd800U && scalar <= 0xdfffU)) {
            return Utf8AppendResult::invalid;
        }
        const std::size_t scalar_units = scalar <= 0xffffU ? 1U : 2U;
        if (output.size() > maximum_code_units ||
            scalar_units > maximum_code_units - output.size()) {
            return Utf8AppendResult::size_limit_exceeded;
        }
        append_utf16_scalar(output, scalar);
        offset += continuation_count + 1U;
    }
    return Utf8AppendResult::ok;
}

bool total_can_add(
    const std::size_t total,
    const std::size_t addition,
    const std::size_t maximum) noexcept {
    return total <= maximum && addition <= maximum - total;
}

bool windows_name_less(
    const ProcessEnvironmentEntry& left,
    const ProcessEnvironmentEntry& right) noexcept {
    return std::lexicographical_compare(
        left.name.begin(), left.name.end(), right.name.begin(), right.name.end(),
        [](const char lhs, const char rhs) {
            return ascii_lower(lhs) < ascii_lower(rhs);
        });
}

}  // namespace

SerializedProcessEnvironment serialize_process_environment(
    const std::vector<ProcessEnvironmentEntry>& entries,
    const ProcessEnvironmentTarget target,
    const std::size_t maximum_serialized_units) {
    if (maximum_serialized_units == 0U ||
        (target != ProcessEnvironmentTarget::posix_v1 &&
         target != ProcessEnvironmentTarget::windows_utf16_v1)) {
        return denied(target, "platform.process_environment.invalid_policy");
    }
    for (std::size_t index = 0U; index < entries.size(); ++index) {
        if (!valid_name(entries[index].name)) {
            return denied(target, "platform.process_environment.invalid_name");
        }
        if (entries[index].value.find('\0') != std::string::npos) {
            return denied(target, "platform.process_environment.embedded_nul");
        }
        for (std::size_t previous = 0U; previous < index; ++previous) {
            if (names_equal(entries[index].name, entries[previous].name, target)) {
                return denied(target, "platform.process_environment.duplicate_name");
            }
        }
    }

    SerializedProcessEnvironment result;
    result.target = target;
    if (target == ProcessEnvironmentTarget::posix_v1) {
        std::size_t total = 0U;
        result.posix_entries.reserve(entries.size());
        for (const auto& entry : entries) {
            if (entry.name.size() >
                    std::numeric_limits<std::size_t>::max() - 2U ||
                entry.value.size() >
                    std::numeric_limits<std::size_t>::max() - 2U -
                        entry.name.size()) {
                return denied(target, "platform.process_environment.size_limit_exceeded");
            }
            const std::size_t units = entry.name.size() + 1U + entry.value.size() + 1U;
            if (!total_can_add(total, units, maximum_serialized_units)) {
                return denied(target, "platform.process_environment.size_limit_exceeded");
            }
            result.posix_entries.push_back(entry.name + "=" + entry.value);
            total += units;
        }
    } else {
        const std::size_t maximum = std::min(
            maximum_serialized_units,
            windows_process_environment_max_code_units);
        std::vector<ProcessEnvironmentEntry> sorted = entries;
        std::sort(sorted.begin(), sorted.end(), windows_name_less);
        std::u16string block;
        for (const auto& entry : sorted) {
            if (block.size() > maximum ||
                entry.name.size() > maximum - block.size() ||
                maximum - block.size() - entry.name.size() < 3U) {
                return denied(target, "platform.process_environment.size_limit_exceeded");
            }
            // Reserve name, '=', this entry's NUL, and the block's final NUL
            // before decoding so input-sized work cannot outrun the native cap.
            const std::size_t value_capacity =
                maximum - block.size() - entry.name.size() - 3U;
            std::u16string value;
            const auto conversion = append_utf8_as_utf16(
                entry.value, value, value_capacity);
            if (conversion == Utf8AppendResult::invalid) {
                return denied(target, "platform.process_environment.invalid_utf8");
            }
            if (conversion == Utf8AppendResult::size_limit_exceeded) {
                return denied(target, "platform.process_environment.size_limit_exceeded");
            }
            for (const char character : entry.name) {
                block.push_back(static_cast<char16_t>(character));
            }
            block.push_back(u'=');
            block.append(value);
            block.push_back(u'\0');
        }
        if (!total_can_add(block.size(), 1U, maximum)) {
            return denied(target, "platform.process_environment.size_limit_exceeded");
        }
        block.push_back(u'\0');
        if (entries.empty()) {
            if (!total_can_add(block.size(), 1U, maximum)) {
                return denied(target, "platform.process_environment.size_limit_exceeded");
            }
            block.push_back(u'\0');
        }
        result.windows_block = std::move(block);
    }
    result.ok = true;
    result.diagnostic_code = "platform.process_environment.serialized";
    return result;
}

}  // namespace copperfin::platform
