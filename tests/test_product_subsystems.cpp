#include "copperfin/localization/localization.h"
#include "copperfin/studio/product_subsystems.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

std::size_t count_missing_locale_keys(
    const copperfin::localization::LocalizedCatalog& catalog,
    std::string_view locale,
    const std::vector<std::string_view>& keys) {
    const auto locale_entries = catalog.catalogs.find(std::string(locale));
    if (locale_entries == catalog.catalogs.end()) {
        return keys.size();
    }

    std::size_t missing = 0U;
    for (const auto key : keys) {
        if (locale_entries->second.find(std::string(key)) == locale_entries->second.end()) {
            ++missing;
        }
    }
    return missing;
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
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const auto english_subsystems = copperfin::studio::product_subsystems_for_catalog(english_catalog);
    const auto spanish_subsystems = copperfin::studio::product_subsystems_for_catalog(spanish_catalog);
    const auto portuguese_subsystems = copperfin::studio::product_subsystems_for_catalog(portuguese_catalog);
    const auto pseudo_subsystems = copperfin::studio::product_subsystems_for_catalog(pseudo_catalog);
    const std::vector<std::string_view> product_subsystem_keys = {
        "Studio.ProductSubsystem.BuildDeploy.ModernEditorDirection",
        "Studio.ProductSubsystem.BuildDeploy.ParityScope",
        "Studio.ProductSubsystem.BuildDeploy.Title",
        "Studio.ProductSubsystem.BuildersWizards.ModernEditorDirection",
        "Studio.ProductSubsystem.BuildersWizards.ParityScope",
        "Studio.ProductSubsystem.BuildersWizards.Title",
        "Studio.ProductSubsystem.ClassDesigner.ModernEditorDirection",
        "Studio.ProductSubsystem.ClassDesigner.ParityScope",
        "Studio.ProductSubsystem.ClassDesigner.Title",
        "Studio.ProductSubsystem.DataExplorer.ModernEditorDirection",
        "Studio.ProductSubsystem.DataExplorer.ParityScope",
        "Studio.ProductSubsystem.DataExplorer.Title",
        "Studio.ProductSubsystem.Debugger.ModernEditorDirection",
        "Studio.ProductSubsystem.Debugger.ParityScope",
        "Studio.ProductSubsystem.Debugger.Title",
        "Studio.ProductSubsystem.FormsDesigner.ModernEditorDirection",
        "Studio.ProductSubsystem.FormsDesigner.ParityScope",
        "Studio.ProductSubsystem.FormsDesigner.Title",
        "Studio.ProductSubsystem.IdeShell.ModernEditorDirection",
        "Studio.ProductSubsystem.IdeShell.ParityScope",
        "Studio.ProductSubsystem.IdeShell.Title",
        "Studio.ProductSubsystem.LabelDesigner.ModernEditorDirection",
        "Studio.ProductSubsystem.LabelDesigner.ParityScope",
        "Studio.ProductSubsystem.LabelDesigner.Title",
        "Studio.ProductSubsystem.MenuDesigner.ModernEditorDirection",
        "Studio.ProductSubsystem.MenuDesigner.ParityScope",
        "Studio.ProductSubsystem.MenuDesigner.Title",
        "Studio.ProductSubsystem.ObjectBrowser.ModernEditorDirection",
        "Studio.ProductSubsystem.ObjectBrowser.ParityScope",
        "Studio.ProductSubsystem.ObjectBrowser.Title",
        "Studio.ProductSubsystem.ProjectManager.ModernEditorDirection",
        "Studio.ProductSubsystem.ProjectManager.ParityScope",
        "Studio.ProductSubsystem.ProjectManager.Title",
        "Studio.ProductSubsystem.ReportDesigner.ModernEditorDirection",
        "Studio.ProductSubsystem.ReportDesigner.ParityScope",
        "Studio.ProductSubsystem.ReportDesigner.Title",
        "Studio.ProductSubsystem.RuntimeEngine.ModernEditorDirection",
        "Studio.ProductSubsystem.RuntimeEngine.ParityScope",
        "Studio.ProductSubsystem.RuntimeEngine.Title",
        "Studio.ProductSubsystem.ToolboxTaskPane.ModernEditorDirection",
        "Studio.ProductSubsystem.ToolboxTaskPane.ParityScope",
        "Studio.ProductSubsystem.ToolboxTaskPane.Title"};

    const auto* english_report = find_subsystem(english_subsystems, "report-designer");
    const auto* spanish_report = find_subsystem(spanish_subsystems, "report-designer");
    const auto* portuguese_project = find_subsystem(portuguese_subsystems, "project-manager");
    const auto* pseudo_report = find_subsystem(pseudo_subsystems, "report-designer");
    expect(english_report != nullptr && spanish_report != nullptr &&
               portuguese_project != nullptr && pseudo_report != nullptr,
        "#2395: localized registry should preserve report designer subsystem lookup");
    if (english_report != nullptr && spanish_report != nullptr &&
        portuguese_project != nullptr && pseudo_report != nullptr) {
        expect(english_report->title == "Report Designer",
            "#2395: en-US registry should preserve report designer title");
        expect(english_report->parity_scope ==
                "band editing, expression authoring, grouping, preview, export, report listeners, builder workflows",
            "#2395: en-US registry should preserve report designer parity prose");
        expect(spanish_report->title == "Disenador De Reportes",
            "#2648: es-419 registry should localize the report designer title");
        expect(
            spanish_report->modern_editor_direction ==
                "parecerse mas a SSRS y a las herramientas actuales de reportes de Visual Studio con contornos mas fuertes, inspectores, vista previa en vivo y paneles mas claros de bandas/propiedades",
            "#2648: es-419 registry should localize report designer direction prose");
        expect(portuguese_project->title == "Gerenciador De Projetos",
            "#2648: pt-BR registry should localize the project manager title");
        expect(
            portuguese_project->parity_scope ==
                "inventario de assets, configuracoes de compilacao, grafo de dependencias, selecao do programa principal, tratamento de itens excluidos",
            "#2648: pt-BR registry should localize project manager parity prose");
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
    expect(
        pseudo_catalog.translate("Studio.ProductSubsystem.ToolboxTaskPane.Title") ==
            copperfin::localization::pseudo_localize("Toolbox And Task Pane"),
        "#2648: qps-ploc toolbox/task-pane title should resolve through the pseudo-localization transform");
    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", product_subsystem_keys) == 0U,
        "#2648: es-419 should define every remaining Studio.ProductSubsystem localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", product_subsystem_keys) == 0U,
        "#2648: pt-BR should define every remaining Studio.ProductSubsystem localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", product_subsystem_keys) == 0U,
        "#2648: qps-ploc should define every remaining Studio.ProductSubsystem localization key");

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
