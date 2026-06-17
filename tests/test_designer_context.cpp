#include "copperfin/studio/designer_context.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

template <typename Descriptor>
bool has_id(const std::vector<Descriptor>& descriptors, std::string_view id) {
    for (const auto& descriptor : descriptors) {
        if (descriptor.id == id) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    using copperfin::studio::StudioDesignerContextRequest;
    using copperfin::studio::StudioEditorSelectionContext;

    const auto visual_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_object
    });
    expect(visual_context.selection_context == StudioEditorSelectionContext::visual_object,
           "#959: visual context result should preserve the requested selection context");
    expect(has_id(visual_context.editor_actions, "show-property-grid"),
           "#959: visual context should include property-grid action");
    expect(has_id(visual_context.editor_actions, "edit-visual-method"),
           "#959: visual context should include method-editor action");
    expect(visual_context.editor_action_count == visual_context.editor_actions.size(),
           "#1009: visual context should report editor-action count metadata");
    expect(has_id(visual_context.builders, "control-builder"),
           "#959: visual context should include control builder");
    expect(has_id(visual_context.builders, "grid-builder"), "#959: visual context should include grid builder");
    expect(visual_context.builder_count == visual_context.builders.size(),
           "#1009: visual context should report builder count metadata");
    expect(visual_context.builder_count == 3U,
           "#1010: visual context should expose form plus control builders");
    expect(has_id(visual_context.builders, "form-builder"),
           "#1010: visual context should include form builder");
    expect(has_id(visual_context.toolbox_items, "textbox"), "#959: visual context should include TextBox toolbox item");
    expect(has_id(visual_context.toolbox_items, "pageframe"),
           "#959: visual context should include PageFrame toolbox item");
    expect(visual_context.toolbox_item_count == visual_context.toolbox_items.size(),
           "#1009: visual context should report toolbox-item count metadata");

    const auto report_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::report_expression
    });
    expect(has_id(report_context.editor_actions, "edit-report-expression"),
           "#959: report context should include expression editor action");
    expect(!has_id(report_context.editor_actions, "edit-visual-method"),
           "#959: report context should exclude visual method editor action");
    expect(has_id(report_context.builders, "report-builder"),
           "#959: report context should include report builder");
    expect(has_id(report_context.toolbox_items, "label"), "#959: report context should include Label toolbox item");
    expect(!has_id(report_context.toolbox_items, "textbox"),
           "#959: report context should exclude form-only TextBox toolbox item");
    expect(report_context.editor_action_count == report_context.editor_actions.size() &&
               report_context.builder_count == report_context.builders.size() &&
               report_context.toolbox_item_count == report_context.toolbox_items.size(),
           "#1009: report context should report counts from filtered descriptor vectors");

    const auto visual_method_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::visual_method
    });
    expect(has_id(visual_method_context.builders, "control-builder"),
           "#1010: visual-method context should preserve control builder availability");
    expect(!has_id(visual_method_context.builders, "form-builder"),
           "#1010: visual-method context should not inherit form builder availability");

    const auto project_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::project_item
    });
    expect(has_id(project_context.editor_actions, "navigate-project-item"),
           "#959: project context should include project navigation action");
    expect(has_id(project_context.builders, "application-wizard"),
           "#959: project context should include application wizard");
    expect(project_context.toolbox_items.empty(), "#959: project context should not expose toolbox items");
    expect(project_context.toolbox_item_count == 0U,
           "#1009: project context should report zero toolbox-item count");

    const auto data_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::data_environment
    });
    expect(has_id(data_context.editor_actions, "edit-data-environment"),
           "#959: data-environment context should include data-environment editor action");
    expect(has_id(data_context.builders, "data-environment-builder"),
           "#959: data-environment context should include data-environment builder");
    expect(data_context.toolbox_items.empty(), "#959: data-environment context should not expose toolbox items");
    expect(data_context.editor_action_count == data_context.editor_actions.size() &&
               data_context.builder_count == data_context.builders.size() &&
               data_context.toolbox_item_count == 0U,
           "#1009: data-environment context should report filtered descriptor counts");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
