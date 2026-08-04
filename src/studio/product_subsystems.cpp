// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/studio/product_subsystems.h"

#include "copperfin/localization/localization.h"

#include <filesystem>
#include <mutex>
#include <string_view>

namespace copperfin::studio {

namespace {

struct ProductSubsystemSource {
    std::string_view id;
    std::string_view title_key;
    std::string_view vfp9_equivalent;
    std::string_view vfp9_equivalent_key;
    std::string_view copperfin_component;
    ProductHostKind host_kind = ProductHostKind::native_ide;
    std::string_view current_status;
    std::string_view parity_scope_key;
    std::string_view modern_editor_direction_key;
};

const std::vector<ProductSubsystemSource>& product_subsystem_sources() {
    static const std::vector<ProductSubsystemSource> subsystems = {
        {
            .id = "ide-shell",
            .title_key = "Studio.ProductSubsystem.IdeShell.Title",
            .vfp9_equivalent = "vfp9.exe shell, taskpane.app, Toolbox.app",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.IdeShell.Vfp9Equivalent",
            .copperfin_component = "copperfin-studio-shell + copperfin-vsix",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.IdeShell.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.IdeShell.ModernEditorDirection"
        },
        {
            .id = "forms-designer",
            .title_key = "Studio.ProductSubsystem.FormsDesigner.Title",
            .vfp9_equivalent = "SCX/SCT designer",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.FormsDesigner.Vfp9Equivalent",
            .copperfin_component = "cf_form_surface",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.FormsDesigner.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.FormsDesigner.ModernEditorDirection"
        },
        {
            .id = "class-designer",
            .title_key = "Studio.ProductSubsystem.ClassDesigner.Title",
            .vfp9_equivalent = "VCX/VCT designer",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.ClassDesigner.Vfp9Equivalent",
            .copperfin_component = "cf_class_surface",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.ClassDesigner.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.ClassDesigner.ModernEditorDirection"
        },
        {
            .id = "report-designer",
            .title_key = "Studio.ProductSubsystem.ReportDesigner.Title",
            .vfp9_equivalent = "FRX/FRT designer, ReportBuilder.app, ReportPreview.app, ReportOutput.app",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.ReportDesigner.Vfp9Equivalent",
            .copperfin_component = "cf_report_surface + cf_reporting_apps",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.ReportDesigner.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.ReportDesigner.ModernEditorDirection"
        },
        {
            .id = "label-designer",
            .title_key = "Studio.ProductSubsystem.LabelDesigner.Title",
            .vfp9_equivalent = "LBX/LBT designer",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.LabelDesigner.Vfp9Equivalent",
            .copperfin_component = "cf_label_surface",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.LabelDesigner.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.LabelDesigner.ModernEditorDirection"
        },
        {
            .id = "menu-designer",
            .title_key = "Studio.ProductSubsystem.MenuDesigner.Title",
            .vfp9_equivalent = "MNX/MNT menu designer",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.MenuDesigner.Vfp9Equivalent",
            .copperfin_component = "cf_menu_surface",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.MenuDesigner.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.MenuDesigner.ModernEditorDirection"
        },
        {
            .id = "project-manager",
            .title_key = "Studio.ProductSubsystem.ProjectManager.Title",
            .vfp9_equivalent = "PJX/PJT project manager",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.ProjectManager.Vfp9Equivalent",
            .copperfin_component = "cf_project_explorer",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.ProjectManager.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.ProjectManager.ModernEditorDirection"
        },
        {
            .id = "runtime-engine",
            .title_key = "Studio.ProductSubsystem.RuntimeEngine.Title",
            .vfp9_equivalent = "vfp9r.dll, vfp9t.dll, executable runtime behavior",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.RuntimeEngine.Vfp9Equivalent",
            .copperfin_component = "cf_runtime",
            .host_kind = ProductHostKind::shared_service,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.RuntimeEngine.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.RuntimeEngine.ModernEditorDirection"
        },
        {
            .id = "builders-wizards",
            .title_key = "Studio.ProductSubsystem.BuildersWizards.Title",
            .vfp9_equivalent = "builder.app, wizard.app, Wizards folder",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.BuildersWizards.Vfp9Equivalent",
            .copperfin_component = "cf_builders + cf_wizards",
            .host_kind = ProductHostKind::native_ide,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.BuildersWizards.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.BuildersWizards.ModernEditorDirection"
        },
        {
            .id = "object-browser",
            .title_key = "Studio.ProductSubsystem.ObjectBrowser.Title",
            .vfp9_equivalent = "objectbrowser.app, browser.app, Foxref.app",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.ObjectBrowser.Vfp9Equivalent",
            .copperfin_component = "cf_browser_tools",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.ObjectBrowser.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.ObjectBrowser.ModernEditorDirection"
        },
        {
            .id = "data-explorer",
            .title_key = "Studio.ProductSubsystem.DataExplorer.Title",
            .vfp9_equivalent = "DataExplorer.app, data environment tooling",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.DataExplorer.Vfp9Equivalent",
            .copperfin_component = "cf_data_explorer",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.DataExplorer.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.DataExplorer.ModernEditorDirection"
        },
        {
            .id = "debugger",
            .title_key = "Studio.ProductSubsystem.Debugger.Title",
            .vfp9_equivalent = "VFP debugger, coverage.app, tasklist.app",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.Debugger.Vfp9Equivalent",
            .copperfin_component = "cf_debugger + cf_diagnostics",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.Debugger.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.Debugger.ModernEditorDirection"
        },
        {
            .id = "toolbox-task-pane",
            .title_key = "Studio.ProductSubsystem.ToolboxTaskPane.Title",
            .vfp9_equivalent = "Toolbox.app, taskpane.app, gallery.app",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.ToolboxTaskPane.Vfp9Equivalent",
            .copperfin_component = "cf_toolbox + cf_task_panes",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.ToolboxTaskPane.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.ToolboxTaskPane.ModernEditorDirection"
        },
        {
            .id = "build-deploy",
            .title_key = "Studio.ProductSubsystem.BuildDeploy.Title",
            .vfp9_equivalent = "Project build, executable generation, redistributable runtime packaging",
            .vfp9_equivalent_key = "Studio.ProductSubsystem.BuildDeploy.Vfp9Equivalent",
            .copperfin_component = "cf_build_host + cf_packaging",
            .host_kind = ProductHostKind::visual_studio_shell,
            .current_status = "implemented",
            .parity_scope_key = "Studio.ProductSubsystem.BuildDeploy.ParityScope",
            .modern_editor_direction_key = "Studio.ProductSubsystem.BuildDeploy.ModernEditorDirection"
        }
    };

    return subsystems;
}

}  // namespace

const char* product_host_kind_name(ProductHostKind kind) {
    switch (kind) {
        case ProductHostKind::native_ide:
            return "native_ide";
        case ProductHostKind::visual_studio_shell:
            return "visual_studio_shell";
        case ProductHostKind::shared_service:
            return "shared_service";
    }
    return "shared_service";
}

std::vector<ProductSubsystemDescriptor> product_subsystems_for_catalog(
    const copperfin::localization::LocalizedCatalog& catalog) {
    std::vector<ProductSubsystemDescriptor> subsystems;
    const auto& sources = product_subsystem_sources();
    subsystems.reserve(sources.size());
    for (const auto& source : sources) {
        subsystems.push_back({
            .id = std::string(source.id),
            .title = catalog.translate(source.title_key),
            .vfp9_equivalent = std::string(source.vfp9_equivalent),
            .vfp9_equivalent_display = catalog.translate(source.vfp9_equivalent_key),
            .copperfin_component = std::string(source.copperfin_component),
            .host_kind = source.host_kind,
            .current_status = std::string(source.current_status),
            .parity_scope = catalog.translate(source.parity_scope_key),
            .modern_editor_direction = catalog.translate(source.modern_editor_direction_key)
        });
    }
    return subsystems;
}

std::vector<ProductSubsystemDescriptor> product_subsystems() {
    struct SubsystemCache {
        std::filesystem::path locale_root;
        std::string locale;
        std::vector<ProductSubsystemDescriptor> subsystems;
    };

    static std::mutex cache_mutex;
    static SubsystemCache cache{};
    const std::filesystem::path locale_root = copperfin::localization::resolve_catalog_root();
    const std::string locale = copperfin::localization::select_locale();
    std::lock_guard<std::mutex> lock(cache_mutex);
    if (cache.locale_root != locale_root || cache.locale != locale) {
        cache.locale_root = locale_root;
        cache.locale = locale;
        cache.subsystems = product_subsystems_for_catalog(
            copperfin::localization::load_catalogs(locale_root, locale));
    }
    return cache.subsystems;
}

}  // namespace copperfin::studio
