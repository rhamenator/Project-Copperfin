// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "test_runtime_pipeline_support.h"
#include "runtime_pipeline_support.h"

namespace cf_test_runtime_pipeline {

inline std::string decode_manifest_value(const std::string& value) {
    std::string decoded;
    decoded.reserve(value.size());
    for (std::size_t index = 0U; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1U >= value.size()) {
            decoded.push_back(value[index]);
            continue;
        }
        const char escaped = value[++index];
        if (escaped == '\\') {
            decoded.push_back('\\');
        } else if (escaped == 'n') {
            decoded.push_back('\n');
        } else if (escaped == 'r') {
            decoded.push_back('\r');
        } else if (escaped == '|') {
            decoded.push_back('|');
        } else {
            decoded.push_back('\\');
            decoded.push_back(escaped);
        }
    }
    return decoded;
}

inline void expect_manifest_omits_keys(
    const std::string& manifest_text,
    const std::vector<std::string>& keys,
    const std::string& manifest_label) {
    for (const auto& key : keys) {
        expect(manifest_text.find(key + "=") == std::string::npos,
               manifest_label + " should omit " + key);
    }
}

}  // namespace cf_test_runtime_pipeline
