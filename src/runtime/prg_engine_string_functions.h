// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::runtime {

// Internal control-flow signal: LIKE/LIKEC detected the session's cooperative
// cancellation token while expression evaluation was still inside one
// statement. The session boundary converts it through the same cancellation
// path used between statements; PRG TRY/CATCH must not consume it as a normal
// compatibility error.
class PrgStringFunctionCancellation final {};

std::string format_value_for_display(
    const PrgValue& value,
    const std::function<std::string(const std::string&)>& set_callback);

std::optional<PrgValue> evaluate_string_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    bool exact_string_compare,
    std::size_t memo_width,
    const std::function<std::string(const std::string&)>& set_callback,
    const std::function<bool()>& cancellation_requested = {});

}  // namespace copperfin::runtime
