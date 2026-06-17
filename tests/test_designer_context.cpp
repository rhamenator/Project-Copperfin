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
    expect(has_id(visual_context.builders, "control-builder"),
           "#959: visual context should include control builder");
    expect(has_id(visual_context.builders, "grid-builder"), "#959: visual context should include grid builder");
    expect(has_id(visual_context.toolbox_items, "textbox"), "#959: visual context should include TextBox toolbox item");
    expect(has_id(visual_context.toolbox_items, "pageframe"),
           "#959: visual context should include PageFrame toolbox item");

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

    const auto project_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::project_item
    });
    expect(has_id(project_context.editor_actions, "navigate-project-item"),
           "#959: project context should include project navigation action");
    expect(has_id(project_context.builders, "application-wizard"),
           "#959: project context should include application wizard");
    expect(project_context.toolbox_items.empty(), "#959: project context should not expose toolbox items");

    const auto data_context = copperfin::studio::studio_designer_context_for_selection({
        .selection_context = StudioEditorSelectionContext::data_environment
    });
    expect(has_id(data_context.editor_actions, "edit-data-environment"),
           "#959: data-environment context should include data-environment editor action");
    expect(has_id(data_context.builders, "data-environment-builder"),
           "#959: data-environment context should include data-environment builder");
    expect(data_context.toolbox_items.empty(), "#959: data-environment context should not expose toolbox items");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
