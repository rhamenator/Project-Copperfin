// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
