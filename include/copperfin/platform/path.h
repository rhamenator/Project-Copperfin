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

// Reports whether an existing filesystem entry carries the Windows Hidden
// attribute. POSIX has no equivalent attribute, so this approximates VFP9's
// Hidden semantics with the traditional leading-dot filename convention.
bool path_is_hidden(const std::filesystem::path& value);

// Reports whether an existing filesystem entry carries the Windows System
// attribute. POSIX has no equivalent and always reports false.
bool path_is_system(const std::filesystem::path& value);

// Convenience predicate for callers that intentionally group Hidden and
// System visibility, such as FILE() and DIRECTORY().
bool path_is_hidden_or_system(const std::filesystem::path& value);

}  // namespace copperfin::platform
