#include "copperfin/localization/localization.h"
#include "copperfin/studio/product_subsystems.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

const copperfin::studio::ProductSubsystemDescriptor* find_subsystem(
    const std::vector<copperfin::studio::ProductSubsystemDescriptor>& subsystems,
    const std::string& id) {
    for (const auto& subsystem : subsystems) {
        if (subsystem.id == id) {
            return &subsystem;
        }
    }
    return nullptr;
}

}  // namespace

int main() {
    const auto& subsystems = copperfin::studio::product_subsystems();
    expect(!subsystems.empty(), "product subsystem registry should not be empty");
    expect(subsystems.size() >= 10U, "product subsystem registry should cover the major VFP 9-equivalent surfaces");

    bool found_report = false;
    bool found_runtime = false;
    bool found_project_manager = false;
    bool found_debugger = false;
    bool found_build_deploy = false;
    bool found_object_browser = false;
    bool found_toolbox_task_pane = false;
    std::size_t implemented_count = 0U;

    for (const auto& subsystem : subsystems) {
        expect(!std::string(subsystem.id).empty(), "each subsystem should have an id");
        expect(!std::string(subsystem.title).empty(), "each subsystem should have a title");
        expect(!std::string(subsystem.vfp9_equivalent).empty(), "each subsystem should name the VFP 9 equivalent");
        expect(!std::string(subsystem.copperfin_component).empty(), "each subsystem should name the Copperfin component");
        expect(!std::string(subsystem.modern_editor_direction).empty(), "each subsystem should capture the modernized editor direction");
        expect(
            std::string(subsystem.current_status) == "implemented" ||
            std::string(subsystem.current_status) == "planned",
            "each subsystem status should use a recognized state");

        if (std::string(subsystem.current_status) == "implemented") {
            ++implemented_count;
        }

        if (subsystem.id == "report-designer") {
            found_report = true;
            expect(std::string(subsystem.current_status) == "implemented", "report designer should now be marked implemented");
        }
        if (subsystem.id == "runtime-engine") {
            found_runtime = true;
            expect(std::string(subsystem.current_status) == "implemented", "runtime engine should now be marked implemented");
        }
        if (subsystem.id == "project-manager") {
            found_project_manager = true;
            expect(std::string(subsystem.current_status) == "implemented", "project manager should now be marked implemented");
        }
        if (subsystem.id == "debugger") {
            found_debugger = true;
            expect(std::string(subsystem.current_status) == "implemented", "debugger should now be marked implemented");
        }
        if (subsystem.id == "build-deploy") {
            found_build_deploy = true;
            expect(std::string(subsystem.current_status) == "implemented", "build/deploy should now be marked implemented");
        }
        if (subsystem.id == "object-browser") {
            found_object_browser = true;
            expect(std::string(subsystem.current_status) == "implemented", "object browser should now be marked implemented");
        }
        if (subsystem.id == "toolbox-task-pane") {
            found_toolbox_task_pane = true;
            expect(std::string(subsystem.current_status) == "planned", "toolbox/task pane should remain planned");
        }
    }

    expect(found_report, "registry should include the report designer subsystem");
    expect(found_runtime, "registry should include the runtime engine subsystem");
    expect(found_project_manager, "registry should include the project manager subsystem");
    expect(found_debugger, "registry should include the debugger subsystem");
    expect(found_build_deploy, "registry should include the build/deploy subsystem");
    expect(found_object_browser, "registry should include the object browser subsystem");
    expect(found_toolbox_task_pane, "registry should include the toolbox/task pane subsystem");
    expect(implemented_count >= 11U, "registry should now mark the Phase C-equivalent surfaces as implemented");

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto english_subsystems = copperfin::studio::product_subsystems_for_catalog(english_catalog);
    const auto pseudo_subsystems = copperfin::studio::product_subsystems_for_catalog(pseudo_catalog);

    const auto* english_report = find_subsystem(english_subsystems, "report-designer");
    const auto* pseudo_report = find_subsystem(pseudo_subsystems, "report-designer");
    expect(english_report != nullptr && pseudo_report != nullptr,
        "#2395: localized registry should preserve report designer subsystem lookup");
    if (english_report != nullptr && pseudo_report != nullptr) {
        expect(english_report->title == "Report Designer",
            "#2395: en-US registry should preserve report designer title");
        expect(english_report->parity_scope ==
                "band editing, expression authoring, grouping, preview, export, report listeners, builder workflows",
            "#2395: en-US registry should preserve report designer parity prose");
        expect(pseudo_report->title.starts_with("[!! "),
            "#2395: pseudo-localized registry should decorate report designer title");
        expect(pseudo_report->parity_scope.starts_with("[!! "),
            "#2395: pseudo-localized registry should decorate report designer parity prose");
        expect(pseudo_report->modern_editor_direction.starts_with("[!! "),
            "#2395: pseudo-localized registry should decorate report designer direction prose");
        expect(pseudo_report->id == english_report->id &&
                pseudo_report->vfp9_equivalent == english_report->vfp9_equivalent &&
                pseudo_report->copperfin_component == english_report->copperfin_component &&
                pseudo_report->host_kind == english_report->host_kind &&
                pseudo_report->current_status == english_report->current_status,
            "#2395: pseudo-localized registry should preserve invariant subsystem metadata");
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
