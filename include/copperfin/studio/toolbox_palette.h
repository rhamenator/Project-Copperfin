#pragma once

#include "copperfin/studio/context_editor_actions.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace copperfin::studio {

enum class StudioToolboxContext {
    form,
    class_designer,
    container,
    report
};

struct StudioToolboxItemDescriptor {
    std::string_view id;
    std::string_view title;
    std::string_view category;
    std::string_view vfp_class;
    std::string_view base_class;
    std::string_view default_name_prefix;
    std::vector<StudioToolboxContext> contexts;
    bool container = false;
    std::string_view description;
};

struct StudioToolboxPaletteLaunchRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
};

struct StudioToolboxPaletteLaunchPlan {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    StudioToolboxContext toolbox_context = StudioToolboxContext::form;
    std::string asset_path;
    std::size_t record_index = 0;
    std::string object_name;
    std::string unique_id;
    std::size_t item_count = 0;
    std::vector<StudioToolboxItemDescriptor> items;
};

struct StudioToolboxPaletteLaunchPlanResult {
    bool ok = false;
    std::string error;
    StudioToolboxPaletteLaunchPlan plan;
};

[[nodiscard]] const char* studio_toolbox_context_name(StudioToolboxContext context);
[[nodiscard]] const std::vector<StudioToolboxItemDescriptor>& studio_toolbox_palette();
[[nodiscard]] std::vector<StudioToolboxItemDescriptor> studio_toolbox_items_for_context(StudioToolboxContext context);
[[nodiscard]] StudioToolboxPaletteLaunchPlanResult plan_studio_toolbox_palette_launch(
    const StudioToolboxPaletteLaunchRequest& request);

}  // namespace copperfin::studio
