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

}  // namespace copperfin::runtime
