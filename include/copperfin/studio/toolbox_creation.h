#pragma once

#include "copperfin/studio/toolbox_palette.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include <cstddef>
#include <string>
#include <vector>

namespace copperfin::studio {

struct StudioToolboxObjectCreateRequest {
    std::string path;
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreatePlan {
    std::string path;
    StudioToolboxItemDescriptor toolbox_item;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t target_record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreatePlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreatePlan plan;
};

struct StudioToolboxObjectCreatePlanCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreatePlanCatalogEntry {
    StudioToolboxItemDescriptor toolbox_item;
    StudioToolboxObjectCreatePlanResult create_plan;
};

struct StudioToolboxObjectCreatePlanCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::size_t plan_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreatePlanCatalogEntry> entries;
};

[[nodiscard]] StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);
[[nodiscard]] StudioToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_context(
    const StudioToolboxObjectCreatePlanCatalogRequest& request);
[[nodiscard]] vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);

}  // namespace copperfin::studio
