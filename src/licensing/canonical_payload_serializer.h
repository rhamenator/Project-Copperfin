// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include <string>

#include "license_payload_value.h"

namespace copperfin::licensing {

// Produces the exact byte sequence that is Ed25519-signed for a license
// file's "payload" object: keys in byte-wise ascending order (guaranteed by
// PayloadFields being a std::map), `{"key":value,...}` with no extra
// whitespace, minimal JSON string escaping, integers as plain decimal.
// This must stay byte-for-byte identical to the canonicalizer used by the
// signing tooling in tools/license-signer/, or every existing license file
// will fail to verify.
[[nodiscard]] std::string canonicalize_payload(const PayloadFields& fields);

}  // namespace copperfin::licensing
