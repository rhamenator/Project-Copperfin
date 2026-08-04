// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <string_view>

namespace copperfin::platform {

std::string json_escape_string(std::string_view value);

}  // namespace copperfin::platform
