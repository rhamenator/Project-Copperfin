// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#include "test_runtime_pipeline_support.h"
#include "runtime_pipeline_support.h"

namespace cf_test_runtime_pipeline {

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
