#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_runtime_surface_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::vector<std::string>& raw_arguments,
    const std::string& default_directory,
    const std::string& frame_file_path,
    const std::string& last_error_message,
    int last_error_code,
    const std::string& last_error_procedure,
    std::size_t last_error_line,
    const std::string& error_handler,
    const std::string& shutdown_handler,
    const std::function<int(const std::string&)>& aerror_callback,
    const std::function<PrgValue(const std::string&)>& eval_expression_callback,
    const std::function<std::string(const std::string&)>& set_callback,
    const std::function<RuntimeOleObjectState*(const PrgValue&)>& resolve_object_callback,
    const std::function<void(const std::string&, std::vector<PrgValue>)>& assign_array_callback,
    const std::function<void(const std::string&, const std::string&)>& record_event_callback);

}  // namespace copperfin::runtime