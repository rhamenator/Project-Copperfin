// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace copperfin::licensing {

struct SignerPublicKey {
    std::string_view key_id;
    std::array<std::uint8_t, 32> public_key;
};

// Production signing keys go here, one entry per active signer_key_id.
// Generate a key with tools/license-signer/generate_signing_key.sh, then
// copy the contents of its generated `<name>_public_key.h` into this table.
//
// This table ships EMPTY by default: no license file will verify, and every
// build defaults to LicenseState::free, until a real key is added below.
// The corresponding private key must never appear in this repository.
inline constexpr std::array<SignerPublicKey, 0> kKnownSignerPublicKeys{};

}  // namespace copperfin::licensing
