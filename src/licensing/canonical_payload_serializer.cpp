// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "canonical_payload_serializer.h"

#include <cstdio>

namespace copperfin::licensing {

namespace {

std::string escape_minimal(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (ch < 0x20U) {
                    char buffer[8] = {};
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(ch));
                    out += buffer;
                } else {
                    out.push_back(static_cast<char>(ch));
                }
        }
    }
    return out;
}

}  // namespace

std::string canonicalize_payload(const PayloadFields& fields) {
    std::string out = "{";
    bool first = true;
    for (const auto& [key, value] : fields) {
        if (!first) {
            out += ",";
        }
        first = false;

        out += "\"";
        out += escape_minimal(key);
        out += "\":";

        if (value.kind == PayloadValue::Kind::string_value) {
            out += "\"";
            out += escape_minimal(value.as_string);
            out += "\"";
        } else {
            out += std::to_string(value.as_integer);
        }
    }
    out += "}";
    return out;
}

}  // namespace copperfin::licensing
