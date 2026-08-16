// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/security/sha256.h"

#include <cstdint>
#include <vector>

namespace copperfin::security {

struct NativeFileSha256SnapshotResult {
    Sha256Result digest;
    std::vector<std::uint8_t> bytes;
};

// Internal streaming helpers for an already-open regular file. The caller owns
// the native handle and is responsible for type and identity validation. The
// snapshot form binds the digest to the exact private bytes returned by the
// same read, so later execution need not trust a mutable filesystem object.
// The handle is encoded as intptr_t so this private header does not expose
// platform headers through the installed public API.
[[nodiscard]] Sha256Result sha256_hex_for_native_file(
    std::intptr_t native_handle,
    std::uint64_t maximum_bytes);
[[nodiscard]] NativeFileSha256SnapshotResult
sha256_snapshot_for_native_file(
    std::intptr_t native_handle,
    std::uint64_t maximum_bytes);
[[nodiscard]] Sha256Result sha256_hex_for_native_bytes(
    const std::vector<std::uint8_t>& bytes);

}  // namespace copperfin::security
