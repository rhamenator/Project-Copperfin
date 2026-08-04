// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>

namespace copperfin::security {

struct Sha256Result {
    bool ok = false;
    std::string hex_digest;
    std::string error;
};

[[nodiscard]] Sha256Result sha256_hex_for_text(const std::string& text);
[[nodiscard]] Sha256Result sha256_hex_for_file(const std::string& path);

}  // namespace copperfin::security
