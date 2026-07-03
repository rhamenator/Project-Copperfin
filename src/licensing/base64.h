// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
