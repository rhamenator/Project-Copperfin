// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>
#include <vector>

namespace copperfin::platform {

// Returns host printer queue names without invoking a command shell. Failure or
// an unavailable host printer service is represented by an empty collection.
[[nodiscard]] std::vector<std::string> enumerate_printer_names();

}  // namespace copperfin::platform
