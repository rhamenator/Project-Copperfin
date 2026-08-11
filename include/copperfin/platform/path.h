// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace copperfin::platform {

std::string path_to_utf8_string(const std::filesystem::path& value);

std::filesystem::path path_from_utf8_string(std::string_view value);

bool path_component_equal_for_platform(
    const std::filesystem::path& left,
    const std::filesystem::path& right);

}  // namespace copperfin::platform
