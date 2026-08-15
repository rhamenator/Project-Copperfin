// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace copperfin::platform {

enum class ProcessArgumentTarget : std::uint32_t {
    posix_v1 = 1U,
    windows_command_line_v1 = 2U
};

struct SerializedProcessArguments {
    bool ok = false;
    ProcessArgumentTarget target = ProcessArgumentTarget::posix_v1;
    // Complete POSIX argument storage, including argv[0]. A launcher owns the
    // transient pointer array and its final null pointer.
    std::vector<std::string> posix_arguments;
    // Complete mutable CreateProcessW command line, excluding the implicit
    // std::u16string terminator. Every element, including argv[0], is quoted
    // with the standard Windows C-runtime backslash/quote convention.
    std::u16string windows_command_line;
    std::string diagnostic_code;
};

// Serializes one explicit executable spelling as argv[0] followed by direct
// argument elements. No shell, PATH search, globbing, variable expansion,
// response-file expansion, or option parsing occurs. POSIX preserves exact
// non-NUL bytes. Windows requires strict UTF-8 and emits the conventional
// CreateProcessW command-line representation; the child remains responsible
// for using compatible argument parsing. maximum_serialized_units includes
// every POSIX argument terminator or the Windows command-line terminator.
[[nodiscard]] SerializedProcessArguments serialize_process_arguments(
    const std::string& executable_argument,
    const std::vector<std::string>& arguments,
    ProcessArgumentTarget target,
    std::size_t maximum_serialized_units);

}  // namespace copperfin::platform
