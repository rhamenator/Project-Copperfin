// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace copperfin::licensing {

// Decodes standard (RFC 4648 with '+'/'/' and '=' padding) base64. Returns
// std::nullopt for any malformed input rather than best-effort decoding,
// since this feeds directly into signature verification.
[[nodiscard]] std::optional<std::vector<std::uint8_t>> base64_decode(const std::string& input);

}  // namespace copperfin::licensing
