// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#pragma once

#if !defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#error "Runtime pipeline test hooks are available only in test builds."
#endif

#include "copperfin/runtime/runtime_pipeline.h"

#include <string>

namespace copperfin::runtime::test_hooks {

enum class ManifestPairPromotionFault {
    none,
    before_first_promotion,
    before_second_promotion
};

void set_manifest_pair_promotion_fault(ManifestPairPromotionFault fault);

void force_package_backup_cleanup_warning_once();

void arm_package_materialization_pause_after_begin();
bool wait_for_package_materialization_pause();
void release_package_materialization_pause();

bool seed_stale_manifest_pair_transaction(
    const RuntimePackagePlan& plan,
    const std::string& staged_runtime_manifest,
    const std::string& staged_debug_manifest,
    std::string& error);

}  // namespace copperfin::runtime::test_hooks
