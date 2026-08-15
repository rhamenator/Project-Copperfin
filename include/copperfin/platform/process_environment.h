// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class ProcessEnvironmentTarget : std::uint32_t {
    posix_v1 = 1U,
    windows_utf16_v1 = 2U
};

struct ProcessEnvironmentEntry {
    std::string name;
    std::string value;

    bool operator==(const ProcessEnvironmentEntry&) const = default;
};

struct SerializedProcessEnvironment {
    bool ok = false;
    ProcessEnvironmentTarget target = ProcessEnvironmentTarget::posix_v1;
    // POSIX entries are complete name=value byte strings. A launcher owns the
    // transient pointer array and its final null pointer.
    std::vector<std::string> posix_entries;
    // Windows receives one sorted UTF-16 block terminated by two null code
    // units. char16_t keeps this representation portable and directly testable.
    std::u16string windows_block;
    std::string diagnostic_code;
};

// Serializes a complete child environment without reading or modifying the
// parent environment. Names use the portable [A-Za-z_][A-Za-z0-9_]* grammar.
// POSIX values are exact non-NUL bytes and retain input order. Windows values
// must be strict UTF-8, are converted losslessly to UTF-16, and entries are
// sorted case-insensitively as required by the platform block contract.
// maximum_serialized_units includes each POSIX entry's terminating null byte.
// For Windows it also includes the final block terminator (or both terminators
// for an empty block). The caller supplies its product/resource bound; the
// 32,767-character CreateProcess limit applies to ANSI, not Unicode, blocks.
[[nodiscard]] SerializedProcessEnvironment serialize_process_environment(
    const std::vector<ProcessEnvironmentEntry>& entries,
    ProcessEnvironmentTarget target,
    std::size_t maximum_serialized_units);

}  // namespace copperfin::platform
