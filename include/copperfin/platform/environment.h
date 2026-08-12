// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace copperfin::platform {

std::optional<std::string> read_environment_variable(
    std::string_view name);
std::string read_environment_variable_or_empty(
    std::string_view name);
std::optional<std::filesystem::path> read_environment_path(
    std::string_view name);
bool write_environment_variable(
    std::string_view name,
    std::string_view value);
bool write_environment_path(
    std::string_view name,
    const std::filesystem::path& value);
bool clear_environment_variable(std::string_view name);
bool clear_environment_path(std::string_view name);

}  // namespace copperfin::platform
