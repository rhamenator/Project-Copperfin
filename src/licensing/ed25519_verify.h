// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace copperfin::licensing {

// Thin wrapper around the vendored, verify-only Ed25519 implementation in
// third_party/ed25519_ref/. This header intentionally exposes no signing,
// key-generation, or key-exchange entry point.
[[nodiscard]] bool ed25519_verify_detached(
    std::string_view message,
    const std::array<std::uint8_t, 64>& signature,
    const std::array<std::uint8_t, 32>& public_key);

}  // namespace copperfin::licensing
