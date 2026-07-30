// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <string>
#include <string_view>

namespace copperfin::platform {

std::string json_escape_string(std::string_view value);

}  // namespace copperfin::platform
