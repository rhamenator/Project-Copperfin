// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <string_view>

#include "license_payload_value.h"

namespace copperfin::licensing {

struct ParsedLicenseFile {
    bool ok = false;
    std::string error;  // non-localized, diagnostic-only
    std::string error_key; // optional catalog key for human-facing display
    std::string error_argument; // optional invariant value for the display placeholder
    PayloadFields payload_fields;
    std::string signature_algorithm;
    std::string signature_base64;
};

// Minimal, hand-rolled JSON parser scoped deliberately to the license file
// schema: a top-level object with exactly "payload" (a flat object of
// string/integer scalars only -- no nesting, arrays, floats, booleans, or
// null), "signature_algorithm" (string), and "signature" (string). Anything
// else is rejected as malformed rather than accepted loosely, since this
// parser is exposed to untrusted input.
[[nodiscard]] ParsedLicenseFile parse_license_file(std::string_view json_text);

}  // namespace copperfin::licensing
