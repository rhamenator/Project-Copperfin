#include "copperfin/studio/toolbox_palette.h"

#include <algorithm>
#include <optional>
#include <utility>

namespace copperfin::studio {

namespace {

bool supports_context(const StudioToolboxItemDescriptor& item, StudioToolboxContext context) {
    return std::find(item.contexts.begin(), item.contexts.end(), context) != item.contexts.end();
}

std::optional<StudioToolboxContext> toolbox_context_for_selection_context(
    StudioEditorSelectionContext context) {
    switch (context) {
        case StudioEditorSelectionContext::visual_object:
        case StudioEditorSelectionContext::visual_method:
            return StudioToolboxContext::form;
        case StudioEditorSelectionContext::container_object:
            return StudioToolboxContext::container;
        case StudioEditorSelectionContext::class_designer:
            return StudioToolboxContext::class_designer;
        case StudioEditorSelectionContext::report_expression:
        case StudioEditorSelectionContext::label_expression:
            return StudioToolboxContext::report;
        case StudioEditorSelectionContext::menu_item:
        case StudioEditorSelectionContext::project_item:
        case StudioEditorSelectionContext::data_environment:
            return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace

const char* studio_toolbox_context_name(StudioToolboxContext context) {
    switch (context) {
        case StudioToolboxContext::form:
            return "form";
        case StudioToolboxContext::class_designer:
            return "class_designer";
        case StudioToolboxContext::container:
            return "container";
        case StudioToolboxContext::report:
            return "report";
    }
    return "form";
}

const std::vector<StudioToolboxItemDescriptor>& studio_toolbox_palette() {
    using Context = StudioToolboxContext;
    static const std::vector<StudioToolboxItemDescriptor> items = {
        {
            .id = "label",
            .title = "Label",
            .category = "Standard Controls",
            .vfp_class = "Label",
            .base_class = "Label",
            .default_name_prefix = "lbl",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = "Display static text or report captions with VFP Label semantics."
        },
        {
            .id = "textbox",
            .title = "TextBox",
            .category = "Standard Controls",
            .vfp_class = "TextBox",
            .base_class = "TextBox",
            .default_name_prefix = "txt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Edit character, numeric, date, and bound field values."
        },
        {
            .id = "editbox",
            .title = "EditBox",
            .category = "Standard Controls",
            .vfp_class = "EditBox",
            .base_class = "EditBox",
            .default_name_prefix = "edt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Edit memo and multi-line text values."
        },
        {
            .id = "commandbutton",
            .title = "CommandButton",
            .category = "Standard Controls",
            .vfp_class = "CommandButton",
            .base_class = "CommandButton",
            .default_name_prefix = "cmd",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Run click actions and command methods."
        },
        {
            .id = "checkbox",
            .title = "CheckBox",
            .category = "Standard Controls",
            .vfp_class = "CheckBox",
            .base_class = "CheckBox",
            .default_name_prefix = "chk",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Edit logical values with VFP CheckBox behavior."
        },
        {
            .id = "combobox",
            .title = "ComboBox",
            .category = "List Controls",
            .vfp_class = "ComboBox",
            .base_class = "ComboBox",
            .default_name_prefix = "cbo",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Pick or enter values from RowSource-driven lists."
        },
        {
            .id = "listbox",
            .title = "ListBox",
            .category = "List Controls",
            .vfp_class = "ListBox",
            .base_class = "ListBox",
            .default_name_prefix = "lst",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Display RowSource-driven selectable lists."
        },
        {
            .id = "grid",
            .title = "Grid",
            .category = "Data Controls",
            .vfp_class = "Grid",
            .base_class = "Grid",
            .default_name_prefix = "grd",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = "Display and edit cursor/table rows with VFP Grid column semantics."
        },
        {
            .id = "image",
            .title = "Image",
            .category = "Graphics",
            .vfp_class = "Image",
            .base_class = "Image",
            .default_name_prefix = "img",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = "Display bitmap and linked image assets."
        },
        {
            .id = "line",
            .title = "Line",
            .category = "Graphics",
            .vfp_class = "Line",
            .base_class = "Line",
            .default_name_prefix = "lin",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = "Draw VFP-compatible line shapes."
        },
        {
            .id = "shape",
            .title = "Shape",
            .category = "Graphics",
            .vfp_class = "Shape",
            .base_class = "Shape",
            .default_name_prefix = "shp",
            .contexts = {Context::form, Context::class_designer, Context::container, Context::report},
            .container = false,
            .description = "Draw rectangles, rounded rectangles, and other VFP shape variants."
        },
        {
            .id = "container",
            .title = "Container",
            .category = "Containers",
            .vfp_class = "Container",
            .base_class = "Container",
            .default_name_prefix = "cnt",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = "Group nested controls under VFP Container semantics."
        },
        {
            .id = "pageframe",
            .title = "PageFrame",
            .category = "Containers",
            .vfp_class = "PageFrame",
            .base_class = "PageFrame",
            .default_name_prefix = "pgf",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = true,
            .description = "Host tabbed pages and nested controls."
        },
        {
            .id = "olecontrol",
            .title = "OLEControl",
            .category = "Interop",
            .vfp_class = "OLEControl",
            .base_class = "OLEControl",
            .default_name_prefix = "ole",
            .contexts = {Context::form, Context::class_designer, Context::container},
            .container = false,
            .description = "Represent VFP OLE control placeholders for compatibility-focused editing."
        }
    };

    return items;
}

std::vector<StudioToolboxItemDescriptor> studio_toolbox_items_for_context(StudioToolboxContext context) {
    std::vector<StudioToolboxItemDescriptor> filtered;
    const auto& items = studio_toolbox_palette();
    std::copy_if(items.begin(), items.end(), std::back_inserter(filtered), [&](const StudioToolboxItemDescriptor& item) {
        return supports_context(item, context);
    });
    return filtered;
}

StudioToolboxPaletteLaunchPlanResult plan_studio_toolbox_palette_launch(
    const StudioToolboxPaletteLaunchRequest& request) {
    const auto toolbox_context = toolbox_context_for_selection_context(request.selection_context);
    if (!toolbox_context.has_value()) {
        return {
            .ok = false,
            .error = "The selected Studio context does not expose a toolbox palette.",
            .plan = {}
        };
    }

    auto items = studio_toolbox_items_for_context(*toolbox_context);
    if (items.empty()) {
        return {
            .ok = false,
            .error = "The selected Studio context has no toolbox items.",
            .plan = {}
        };
    }

    const auto item_count = items.size();
    return {
        .ok = true,
        .error = {},
        .plan = {
            .selection_context = request.selection_context,
            .toolbox_context = *toolbox_context,
            .asset_path = request.asset_path,
            .record_index = request.record_index,
            .object_name = request.object_name,
            .unique_id = request.unique_id,
            .item_count = item_count,
            .items = std::move(items)
        }
    };
}

}  // namespace copperfin::studio
