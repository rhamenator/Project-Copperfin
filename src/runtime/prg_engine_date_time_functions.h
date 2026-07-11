// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_date_time_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<std::string(const std::string&)>& set_callback);

std::optional<PrgValue> evaluate_date_time_additive(
    const PrgValue& left,
    const PrgValue& right,
    bool subtract,
    const std::function<std::string(const std::string&)>& set_callback);

std::optional<int> compare_date_time_values(
    const PrgValue& left,
    const PrgValue& right,
    const std::function<std::string(const std::string&)>& set_callback);

}  // namespace copperfin::runtime
