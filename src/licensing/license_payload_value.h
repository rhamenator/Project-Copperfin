// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>
#include <string>
#include <utility>

namespace copperfin::licensing {

// A license file's signed "payload" object is deliberately restricted to
// flat string/integer scalars (see canonical_payload_serializer.h for why).
struct PayloadValue {
    enum class Kind { string_value, integer_value };

    Kind kind = Kind::string_value;
    std::string as_string;
    long long as_integer = 0;

    static PayloadValue make_string(std::string value) {
        PayloadValue result;
        result.kind = Kind::string_value;
        result.as_string = std::move(value);
        return result;
    }

    static PayloadValue make_integer(long long value) {
        PayloadValue result;
        result.kind = Kind::integer_value;
        result.as_integer = value;
        return result;
    }
};

// std::map<std::string, ...> iterates keys in byte-wise ascending order,
// which is exactly the canonical field ordering the signature is computed
// over -- no separate sort step is needed.
using PayloadFields = std::map<std::string, PayloadValue>;

}  // namespace copperfin::licensing
