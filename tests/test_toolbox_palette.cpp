#include "copperfin/studio/toolbox_palette.h"

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

bool has_toolbox_item(
    const std::vector<copperfin::studio::StudioToolboxItemDescriptor>& items,
    std::string_view id) {
    for (const auto& item : items) {
        if (item.id == id) {
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    using copperfin::studio::StudioToolboxContext;

    const auto& items = copperfin::studio::studio_toolbox_palette();
    expect(items.size() >= 12U, "#957: toolbox palette should expose common VFP visual controls");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::form)) == "form",
           "#957: form toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::class_designer)) ==
               "class_designer",
           "#957: class-designer toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::container)) ==
               "container",
           "#957: container toolbox context token should be stable");
    expect(std::string(copperfin::studio::studio_toolbox_context_name(StudioToolboxContext::report)) == "report",
           "#957: report toolbox context token should be stable");

    bool found_container = false;
    bool found_report_item = false;
    bool found_data_control = false;

    for (const auto& item : items) {
        expect(!std::string(item.id).empty(), "#957: each toolbox item should have an id");
        expect(!std::string(item.title).empty(), "#957: each toolbox item should have a title");
        expect(!std::string(item.category).empty(), "#957: each toolbox item should have a category");
        expect(!std::string(item.vfp_class).empty(), "#957: each toolbox item should name the VFP class");
        expect(!std::string(item.base_class).empty(), "#957: each toolbox item should name the VFP base class");
        expect(!std::string(item.default_name_prefix).empty(),
               "#957: each toolbox item should provide a default object-name prefix");
        expect(!item.contexts.empty(), "#957: each toolbox item should name at least one target context");
        expect(!std::string(item.description).empty(), "#957: each toolbox item should describe its creation action");
        if (item.container) {
            found_container = true;
        }
        if (item.category == "Data Controls") {
            found_data_control = true;
        }
        for (const auto context : item.contexts) {
            if (context == StudioToolboxContext::report) {
                found_report_item = true;
            }
        }
    }

    expect(found_container, "#957: toolbox palette should identify container controls");
    expect(found_report_item, "#957: toolbox palette should include report-compatible items");
    expect(found_data_control, "#957: toolbox palette should include data controls");
    expect(has_toolbox_item(items, "label"), "#957: toolbox palette should include Label");
    expect(has_toolbox_item(items, "textbox"), "#957: toolbox palette should include TextBox");
    expect(has_toolbox_item(items, "commandbutton"), "#957: toolbox palette should include CommandButton");
    expect(has_toolbox_item(items, "combobox"), "#957: toolbox palette should include ComboBox");
    expect(has_toolbox_item(items, "grid"), "#957: toolbox palette should include Grid");
    expect(has_toolbox_item(items, "container"), "#957: toolbox palette should include Container");
    expect(has_toolbox_item(items, "pageframe"), "#957: toolbox palette should include PageFrame");

    const auto form_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::form);
    expect(has_toolbox_item(form_items, "textbox"), "#957: form context should expose TextBox");
    expect(has_toolbox_item(form_items, "pageframe"), "#957: form context should expose PageFrame");
    expect(has_toolbox_item(form_items, "olecontrol"), "#957: form context should expose OLEControl");

    const auto container_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::container);
    expect(has_toolbox_item(container_items, "checkbox"), "#957: container context should expose CheckBox");
    expect(has_toolbox_item(container_items, "grid"), "#957: container context should expose Grid");

    const auto report_items = copperfin::studio::studio_toolbox_items_for_context(StudioToolboxContext::report);
    expect(has_toolbox_item(report_items, "label"), "#957: report context should expose Label");
    expect(has_toolbox_item(report_items, "line"), "#957: report context should expose Line");
    expect(!has_toolbox_item(report_items, "textbox"), "#957: report context should exclude form-only TextBox");
    expect(!has_toolbox_item(report_items, "pageframe"), "#957: report context should exclude form-only PageFrame");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
