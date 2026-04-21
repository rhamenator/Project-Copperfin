#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_path_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::string& default_directory);

}  // namespace copperfin::runtime
