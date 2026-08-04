// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#pragma once

#include "copperfin/studio/toolbox_palette.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxInvocationAdmissionRequest {
    StudioToolboxPaletteLaunchPlan launch_plan;
    bool admit_palette_invocation = false;
};

struct StudioToolboxInvocationAdmissionPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    bool palette_invocation_admitted = false;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxInvocationAdmissionResult {
    bool ok = false;
    std::string error;
    StudioToolboxInvocationAdmissionPlan plan;
};

struct StudioToolboxInvocationAdmissionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
};

struct StudioToolboxInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    std::size_t admission_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioSelectionToolboxInvocationAdmissionCatalogRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    bool admit_palette_invocation = false;
};

struct StudioSelectionToolboxInvocationAdmissionCatalogResult {
    bool ok = false;
    std::string error;
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string command_token;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
    StudioToolboxPaletteLaunchPlanResult launch_plan;
    StudioToolboxInvocationAdmissionResult invocation_admission;
    std::size_t admission_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

[[nodiscard]] StudioToolboxInvocationAdmissionResult plan_studio_toolbox_invocation_admission(
    const StudioToolboxInvocationAdmissionRequest& request);
[[nodiscard]] StudioToolboxInvocationAdmissionCatalogResult plan_studio_toolbox_invocation_admission_catalog(
    const StudioToolboxInvocationAdmissionCatalogRequest& request);
[[nodiscard]] StudioSelectionToolboxInvocationAdmissionCatalogResult
plan_studio_toolbox_invocation_admission_catalog_for_selection(
    const StudioSelectionToolboxInvocationAdmissionCatalogRequest& request);

}  // namespace copperfin::studio
