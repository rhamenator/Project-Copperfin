#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_string_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    bool exact_string_compare);

}  // namespace copperfin::runtime
