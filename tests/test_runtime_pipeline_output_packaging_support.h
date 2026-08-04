// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
