// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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
void arm_package_content_materialization_pause_before_first_asset();
bool wait_for_package_content_materialization_pause();
void release_package_content_materialization_pause();
void pause_before_package_content_parent_open();
void arm_package_content_parent_open_pause();
bool wait_for_package_content_parent_open_pause();
void release_package_content_parent_open_pause();

bool seed_stale_manifest_pair_transaction(
    const RuntimePackagePlan& plan,
    const std::string& staged_runtime_manifest,
    const std::string& staged_debug_manifest,
    std::string& error);

// Single-shot hook fired immediately before the post-read
// inspect_physical_path_containment() re-walk in admit_launcher_artifact()
// (issue #5426/PR #5428). Runs synchronously on the calling thread --
// inventory_generated_launcher_artifacts() is called directly from
// materialize_runtime_package()'s own thread, never a background one -- so
// a test can rename/replace the artifact from within the hook itself with
// no cross-thread synchronization needed.
void set_launcher_artifact_post_read_test_hook(void (*hook)());

}  // namespace copperfin::runtime::test_hooks
