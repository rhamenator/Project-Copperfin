// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "copperfin/studio/builder_invocation_admission.h"
#include "copperfin/studio/designer_launch_surfaces.h"
#include "copperfin/studio/editor_action_invocation_admission.h"
#include "copperfin/studio/toolbox_invocation_admission.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioDesignerInvocationAdmissionRequest {
    StudioDesignerLaunchSurfacePlan launch_surface_plan;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
};

struct StudioDesignerInvocationAdmissionPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    std::size_t editor_action_invocation_count = 0;
    std::size_t builder_invocation_count = 0;
    bool toolbox_available = false;
    std::size_t toolbox_item_count = 0;
    std::string toolbox_error;
    std::vector<StudioEditorActionInvocationAdmissionResult> editor_action_invocations;
    std::vector<StudioBuilderInvocationAdmissionResult> builder_invocations;
    StudioToolboxInvocationAdmissionResult toolbox_invocation;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioDesignerInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioDesignerInvocationAdmissionPlan plan;
};

struct StudioDesignerInvocationAdmissionCatalogRequest {
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    bool admit_editor_invocations = false;
    bool admit_builder_invocations = false;
    bool admit_toolbox_invocation = false;
};

struct StudioDesignerInvocationAdmissionCatalogEntry {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t editor_action_invocation_count = 0;
    std::size_t builder_invocation_count = 0;
    bool toolbox_available = false;
    std::size_t toolbox_item_count = 0;
    std::string toolbox_error;
    bool dry_run = true;
    bool mutates_asset = false;
    StudioDesignerInvocationAdmissionResult invocation_admission;
};

struct StudioDesignerInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    std::size_t context_count = 0;
    std::vector<StudioDesignerInvocationAdmissionCatalogEntry> contexts;
};

[[nodiscard]] StudioDesignerInvocationAdmissionResult plan_studio_designer_invocation_admission(
    const StudioDesignerInvocationAdmissionRequest& request);
[[nodiscard]] StudioDesignerInvocationAdmissionCatalogResult plan_studio_designer_invocation_admission_catalog(
    const StudioDesignerInvocationAdmissionCatalogRequest& request);

}  // namespace copperfin::studio
