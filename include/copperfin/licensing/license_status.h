// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>

#include "copperfin/licensing/ed25519_public_key.h"

namespace copperfin::licensing {

// The major version this build of Copperfin belongs to. Compared against a
// Perpetual License's `perpetual_max_major_version` for *display purposes
// only* -- see LicenseState::perpetual_out_of_version, which is a fully
// valid, unrestricted state, never a degraded one.
inline constexpr int kCopperfinMajorVersion = 1;

enum class LicenseState {
    free,                      // no license file resolved -- default, always fine
    subscription_active,
    subscription_expired,
    perpetual_current,
    perpetual_out_of_version,  // still fully valid; purely informational
    invalid_signature,
    malformed,
    file_unreadable
};

[[nodiscard]] std::string_view license_state_name(LicenseState state);

struct LicenseStatus {
    LicenseState state = LicenseState::free;
    std::string license_id;
    std::string license_type;      // "subscription" | "perpetual" | "" (free)
    std::string pricing_model;     // "seat" | "revenue" | ""
    std::string licensee_name;
    std::string licensee_email;
    int seats = 0;
    std::string issued_date;
    std::string subscription_expires;   // "" if perpetual/free
    int perpetual_max_major_version = 0;
    std::string source_path;       // "" for free
    std::string diagnostic;        // raw, non-localized troubleshooting detail
    std::string diagnostic_key;    // optional catalog key for human-facing display
    std::string diagnostic_argument; // optional invariant value for the display placeholder
};

// Resolves and verifies a license file, entirely offline -- this function
// never makes a network call and never blocks on external state. Path
// resolution order: `explicit_override` (if provided and non-empty) ->
// `COPPERFIN_LICENSE_PATH` environment variable -> `license.cflicense` next
// to `executable_path` -> LicenseState::free if none of those resolve to a
// readable file.
//
// `signer_keys` defaults to the real product key table and should be left
// at its default in every product call site; it exists as a parameter only
// so tests can verify against throwaway fixture keys without ever needing
// to populate the real (and normally empty) kKnownSignerPublicKeys table.
[[nodiscard]] LicenseStatus load_license_status(
    const std::filesystem::path& executable_path,
    const std::optional<std::filesystem::path>& explicit_override = std::nullopt,
    std::span<const SignerPublicKey> signer_keys = kKnownSignerPublicKeys);

}  // namespace copperfin::licensing
