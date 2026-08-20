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

bool path_equal_case_insensitive(
    const std::filesystem::path& left,
    const std::filesystem::path& right);

// Reports whether an existing filesystem entry carries the Windows Hidden or
// System attribute. On Windows this reads the real FILE_ATTRIBUTE_HIDDEN /
// FILE_ATTRIBUTE_SYSTEM bits via GetFileAttributesW. POSIX has no equivalent
// attribute, so it approximates VFP9's Hidden semantics with the traditional
// leading-dot filename convention; System is never reported there. Returns
// false if the entry cannot be queried (e.g. it does not exist).
bool path_is_hidden_or_system(const std::filesystem::path& value);

}  // namespace copperfin::platform
