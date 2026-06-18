#pragma once

#include "copperfin/studio/builder_dispatch.h"
#include "copperfin/studio/designer_invocation_admission.h"
#include "copperfin/studio/editor_action_dispatch.h"
#include "copperfin/studio/toolbox_dispatch.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioDesignerDispatchRequest {
    StudioDesignerInvocationAdmissionPlan invocation_admission_plan;
};

struct StudioDesignerDispatchPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string symbol;
    std::size_t line = 0;
    std::size_t column = 0;
    std::size_t editor_action_dispatch_count = 0;
    std::size_t builder_dispatch_count = 0;
    std::size_t toolbox_dispatch_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    std::vector<StudioEditorActionDispatchResult> editor_action_dispatches;
    std::vector<StudioBuilderDispatchResult> builder_dispatches;
    StudioToolboxDispatchResult toolbox_dispatch;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioDesignerDispatchResult {
    bool ok = false;
    std::string error;
    StudioDesignerDispatchPlan plan;
};

struct StudioDesignerDispatchCatalogRequest {
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

struct StudioDesignerDispatchCatalogEntry {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::size_t editor_action_dispatch_count = 0;
    std::size_t builder_dispatch_count = 0;
    std::size_t toolbox_dispatch_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    StudioDesignerDispatchResult dispatch;
};

struct StudioDesignerDispatchCatalogResult {
    bool ok = false;
    std::string error;
    std::size_t context_count = 0;
    std::vector<StudioDesignerDispatchCatalogEntry> contexts;
};

[[nodiscard]] StudioDesignerDispatchResult plan_studio_designer_dispatch(
    const StudioDesignerDispatchRequest& request);
[[nodiscard]] StudioDesignerDispatchCatalogResult plan_studio_designer_dispatch_catalog(
    const StudioDesignerDispatchCatalogRequest& request);

}  // namespace copperfin::studio
