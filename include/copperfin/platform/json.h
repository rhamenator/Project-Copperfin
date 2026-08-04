// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>
#include <string_view>

namespace copperfin::platform {

std::string json_escape_string(std::string_view value);

}  // namespace copperfin::platform
