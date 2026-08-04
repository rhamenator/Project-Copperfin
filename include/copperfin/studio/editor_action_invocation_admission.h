// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/context_editor_actions.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioEditorActionInvocationAdmissionRequest {
    StudioEditorActionLaunchPlan launch_plan;
    bool admit_editor_invocation = false;
};

struct StudioEditorActionInvocationAdmissionPlan {
    StudioEditorActionDescriptor action;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string command_token;
    std::string target_surface;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    bool editor_invocation_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioEditorActionInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioEditorActionInvocationAdmissionPlan plan;
};

struct StudioEditorActionInvocationAdmissionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    bool admit_editor_invocations = false;
};

struct StudioEditorActionInvocationAdmissionCatalogEntry {
    StudioEditorActionDescriptor action;
    StudioEditorActionLaunchPlanResult launch_plan;
    StudioEditorActionInvocationAdmissionResult invocation_admission;
};

struct StudioEditorActionInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t action_count = 0;
    std::size_t admission_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioEditorActionInvocationAdmissionCatalogEntry> entries;
};

[[nodiscard]] StudioEditorActionInvocationAdmissionResult plan_studio_editor_action_invocation_admission(
    const StudioEditorActionInvocationAdmissionRequest& request);
[[nodiscard]] StudioEditorActionInvocationAdmissionCatalogResult
plan_studio_editor_action_invocation_admission_catalog(
    const StudioEditorActionInvocationAdmissionCatalogRequest& request);

}  // namespace copperfin::studio
