#pragma once

#include "copperfin/studio/builder_registry.h"
#include "copperfin/studio/context_editor_actions.h"
#include "copperfin/studio/toolbox_palette.h"

#include <string_view>
#include <vector>

namespace copperfin::studio {

struct StudioDesignerContextRequest {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
};

struct StudioDesignerContextResult {
    StudioEditorSelectionContext selection_context = StudioEditorSelectionContext::visual_object;
    std::vector<StudioEditorActionDescriptor> editor_actions;
    std::vector<StudioBuilderDescriptor> builders;
    std::vector<StudioToolboxItemDescriptor> toolbox_items;
};

[[nodiscard]] StudioDesignerContextResult studio_designer_context_for_selection(
    const StudioDesignerContextRequest& request);

}  // namespace copperfin::studio
