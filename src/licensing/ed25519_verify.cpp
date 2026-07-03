// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "ed25519_verify.h"

extern "C" {
#include "third_party/ed25519_ref/ed25519.h"
}

namespace copperfin::licensing {

bool ed25519_verify_detached(
    std::string_view message,
    const std::array<std::uint8_t, 64>& signature,
    const std::array<std::uint8_t, 32>& public_key) {
    const auto* message_bytes = reinterpret_cast<const unsigned char*>(message.data());
    return ed25519_verify(signature.data(), message_bytes, message.size(), public_key.data()) != 0;
}

}  // namespace copperfin::licensing
