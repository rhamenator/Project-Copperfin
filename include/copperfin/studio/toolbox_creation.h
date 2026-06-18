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

struct StudioToolboxObjectCreateBatchItem {
    std::string toolbox_item_id;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
};

struct StudioToolboxObjectCreateBatchPlan {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::vector<StudioToolboxObjectCreatePlan> plans;
    bool dry_run = true;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchPlanRequest {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::vector<StudioToolboxObjectCreateBatchItem> items;
};

struct StudioToolboxObjectCreateBatchPlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateBatchPlan plan;
};

struct StudioToolboxObjectCreateDispatchRequest {
    StudioToolboxObjectCreatePlan create_plan;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateDispatchPlan {
    std::string path;
    StudioToolboxItemDescriptor toolbox_item;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t target_record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateDispatchPlan plan;
};

struct StudioToolboxObjectCreateBatchDispatchRequest {
    StudioToolboxObjectCreateBatchPlan batch_plan;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateBatchDispatchPlan {
    std::string path;
    bool toolbox_context_provided = false;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::vector<StudioToolboxObjectCreatePlan> plans;
    std::vector<std::string> dispatch_arguments;
    bool dispatch_admitted = false;
    bool dry_run = true;
    bool executed = false;
    bool mutates_asset = false;
};

struct StudioToolboxObjectCreateBatchDispatchResult {
    bool ok = false;
    std::string error;
    StudioToolboxObjectCreateBatchDispatchPlan plan;
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

struct StudioToolboxObjectCreateDispatchCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateDispatchCatalogEntry {
    StudioToolboxItemDescriptor toolbox_item;
    StudioToolboxObjectCreatePlanResult create_plan;
    StudioToolboxObjectCreateDispatchResult dispatch;
};

struct StudioToolboxObjectCreateDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
    std::vector<StudioToolboxObjectCreateDispatchCatalogEntry> entries;
};

struct StudioToolboxObjectCreateBatchDispatchCatalogRequest {
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string path;
    std::string parent_name;
    std::vector<vfp::VisualObjectPropertyChange> field_values;
    bool admit_create_operation = false;
};

struct StudioToolboxObjectCreateBatchDispatchCatalogResult {
    bool ok = false;
    std::string error;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::size_t item_count = 0;
    StudioToolboxObjectCreateBatchPlanResult batch_plan;
    StudioToolboxObjectCreateBatchDispatchResult dispatch;
    std::size_t dispatch_count = 0;
    std::size_t error_count = 0;
    bool dry_run = true;
    bool mutates_asset = false;
};

[[nodiscard]] StudioToolboxObjectCreatePlanResult plan_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchPlanResult plan_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request);
[[nodiscard]] StudioToolboxObjectCreateDispatchResult plan_visual_object_create_dispatch(
    const StudioToolboxObjectCreateDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchDispatchResult plan_visual_object_batch_create_dispatch(
    const StudioToolboxObjectCreateBatchDispatchRequest& request);
[[nodiscard]] StudioToolboxObjectCreatePlanCatalogResult plan_visual_object_catalog_from_toolbox_context(
    const StudioToolboxObjectCreatePlanCatalogRequest& request);
[[nodiscard]] StudioToolboxObjectCreateDispatchCatalogResult plan_visual_object_create_dispatch_catalog(
    const StudioToolboxObjectCreateDispatchCatalogRequest& request);
[[nodiscard]] StudioToolboxObjectCreateBatchDispatchCatalogResult plan_visual_object_batch_create_dispatch_catalog(
    const StudioToolboxObjectCreateBatchDispatchCatalogRequest& request);
[[nodiscard]] vfp::VisualObjectCreateResult create_visual_object_from_toolbox_item(
    const StudioToolboxObjectCreateRequest& request);
[[nodiscard]] vfp::VisualObjectCreateBatchResult create_visual_objects_from_toolbox_items(
    const StudioToolboxObjectCreateBatchPlanRequest& request);

}  // namespace copperfin::studio
