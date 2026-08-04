// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::package_trust {

enum class LauncherInventoryVerificationStatus {
    valid,
    malformed_envelope,
    unknown_signer,
    ambiguous_signer,
    invalid_signature
};

struct LauncherInventoryArtifact {
    std::string role;
    std::string package_relative_path;
    std::string sha256;
};

struct LauncherInventoryTrustedKey {
    std::string_view key_id;
    std::array<std::uint8_t, 32> public_key{};
};

struct LauncherInventorySignatureSidecar {
    std::string signer_key_id;
    std::array<std::uint8_t, 64> detached_signature{};
};

struct LauncherInventoryVerificationResult {
    LauncherInventoryVerificationStatus status =
        LauncherInventoryVerificationStatus::malformed_envelope;
    std::string signer_key_id;
};

// Returns the exact UTF-8 byte sequence signed by the release signer. The
// result ends each record with LF and contains no source/debug paths.
[[nodiscard]] std::string canonical_launcher_inventory_envelope(
    std::string_view signer_key_id,
    std::span<const LauncherInventoryArtifact> artifacts);

// Confirms that a signed envelope is the canonical inventory for the selected
// package manifest records, rather than merely a valid signature for another
// package.
[[nodiscard]] bool launcher_inventory_envelope_matches_artifacts(
    std::string_view envelope,
    std::string_view signer_key_id,
    std::span<const LauncherInventoryArtifact> artifacts);

// Parses the exact UTF-8/LF textual app.cftrust.sig sidecar. The returned
// signature is decoded only after the sidecar's version, algorithm, signer,
// and line-ending contract have been validated.
[[nodiscard]] std::optional<LauncherInventorySignatureSidecar>
parse_launcher_inventory_signature_sidecar(std::string_view sidecar);

// Verifies only the launcher-inventory trust contract. Package-file hashes,
// containment, and Windows process identity remain separate guard checks.
[[nodiscard]] LauncherInventoryVerificationResult verify_signed_launcher_inventory(
    std::string_view envelope,
    const std::array<std::uint8_t, 64>& detached_signature,
    std::span<const LauncherInventoryTrustedKey> trusted_keys);

// Parses and verifies the textual detached-signature sidecar, including the
// requirement that its signer ID exactly match the signed inventory.
[[nodiscard]] LauncherInventoryVerificationResult verify_signed_launcher_inventory(
    std::string_view envelope,
    std::string_view signature_sidecar,
    std::span<const LauncherInventoryTrustedKey> trusted_keys);

}  // namespace copperfin::package_trust
