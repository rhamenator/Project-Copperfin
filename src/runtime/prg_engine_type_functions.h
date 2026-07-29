// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_type_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::function<bool(const std::string&)>& array_exists_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback);

}  // namespace copperfin::runtime
