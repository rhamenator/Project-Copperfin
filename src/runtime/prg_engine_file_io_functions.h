// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/runtime/prg_engine.h"

#include <optional>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace copperfin::runtime {

std::optional<PrgValue> evaluate_file_io_function(
    const std::string& function,
    const std::vector<PrgValue>& arguments,
    const std::string& default_directory,
    bool require_verified_file_byte_overrides,
    const std::function<std::optional<std::string>(const std::filesystem::path&)>& read_verified_file_callback,
    const std::function<void(const std::filesystem::path&)>& verified_file_unavailable_callback,
    const std::function<std::string(const std::string&)>& set_callback);

void close_all_file_io_handles();

}  // namespace copperfin::runtime
