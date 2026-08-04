// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/studio/project_workspace.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
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

std::string expected_default_workspace_output_path(const std::string& project_path, const std::string& stem) {
#if defined(_WIN32)
    const std::string leaf = stem + ".exe";
#else
    const std::string leaf = stem;
#endif
    const std::size_t separator = project_path.find_last_of("/\\");
    if (separator != std::string::npos) {
        return project_path.substr(0U, separator + 1U) + leaf;
    }
    return (std::filesystem::path(project_path).parent_path() / leaf).string();
}

std::string expected_default_workspace_build_target() {
#if defined(_WIN32)
    return "x64 Windows executable";
#else
    return "x64 native executable";
#endif
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

copperfin::vfp::DbfRecord make_record(
    std::size_t record_index,
    std::initializer_list<copperfin::vfp::DbfRecordValue> values,
    bool deleted = false) {
    copperfin::vfp::DbfRecord record;
    record.record_index = record_index;
    record.deleted = deleted;
    record.values.assign(values.begin(), values.end());
    return record;
}

void test_build_project_workspace() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\demo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "DEMOAPP"},
            {.field_name = "HOMEDIR", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\samples)", .memo_block_number = 3U},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\demoapp.exe)", .memo_block_number = 4U},
            {.field_name = "DEBUG", .field_type = 'L', .display_value = "true", .memo_block_number = 5U},
            {.field_name = "SAVECODE", .field_type = 'L', .display_value = "false", .memo_block_number = 6U}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg", .memo_block_number = 11U},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true", .memo_block_number = 12U},
            {.field_name = "COMMENTS", .field_type = 'M', .display_value = "Application entry point", .memo_block_number = 13U},
            {.field_name = "LOCAL", .field_type = 'L', .display_value = "true", .memo_block_number = 14U}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms\\customer.scx", .memo_block_number = 21U},
            {.field_name = "COMMENTS", .field_type = 'M', .display_value = "Customer maintenance", .memo_block_number = 22U},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "false", .memo_block_number = 23U}
        }),
        make_record(3, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "reports\\invoice.frx"},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "true", .memo_block_number = 32U}
        }),
        make_record(4, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "menus\\oldmenu.mnx", .memo_block_number = 41U}
        }, true)
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "project workspace should be available for PJX documents");
    expect(workspace.project_title == "DEMOAPP", "workspace should use the project key as its title");
    expect(workspace.project_title_field_index == 1U, "#678: workspace project title should retain selected KEY field provenance");
    expect(workspace.project_title_memo_block_number == 0U, "#714: project titles from non-memo KEY fields should expose memo block zero");
    expect(workspace.project_key_field_index == 1U, "#678: workspace project key field provenance should be preserved");
    expect(workspace.project_key_memo_block_number == 0U, "#714: project keys from non-memo fields should expose memo block zero");
    expect(workspace.home_directory_field_index == 2U, "#678: workspace home directory field provenance should be preserved");
    expect(workspace.home_directory_memo_block_number == 3U, "#714: workspace home directories should retain source memo block provenance");
    expect(workspace.output_path_field_index == 3U, "#678: workspace output path field provenance should be preserved");
    expect(workspace.output_path_memo_block_number == 4U, "#714: workspace output paths should retain source memo block provenance");
    expect(workspace.entries.size() == 5U, "workspace should include all project records");
    expect(workspace.groups.size() >= 3U, "workspace should group header, program, and form/report items");
    expect(workspace.build_plan.available, "build plan should be available");
    expect(workspace.build_plan.can_build, "build plan should be buildable with entries and an output path");
    expect(workspace.build_plan.project_title == "DEMOAPP", "#726: build-plan project titles should mirror workspace titles");
    expect(workspace.build_plan.project_title_field_index == 1U,
           "#726: build-plan project titles should mirror workspace title field provenance");
    expect(workspace.build_plan.project_title_memo_block_number == 0U,
           "#726: build-plan project titles should mirror workspace title memo block provenance");
    expect(workspace.build_plan.output_path == R"(E:\Project-Copperfin\build\demoapp.exe)", "build plan should keep the explicit output path");
    expect(workspace.build_plan.output_kind == "executable", "build plan should infer executable output kind from .exe output path");
    expect(workspace.build_plan.output_kind_field_index == 3U, "#683: build plan output kind provenance should retain OUTFILE field ordinal");
    expect(workspace.build_plan.build_target == "x64 Windows executable", "build plan should label .exe outputs as Windows executables");
    expect(workspace.build_plan.build_target_field_index == 3U, "#683: build plan target provenance should retain OUTFILE field ordinal");
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc");
    const auto pseudo_workspace = copperfin::studio::build_project_workspace(document, pseudo_catalog);
    expect(
        pseudo_workspace.build_plan.output_kind == "executable",
        "#2495: pseudo-localized build plan should preserve output_kind machine values");
    expect(
        pseudo_workspace.build_plan.output_kind_field_index == 3U,
        "#2495: pseudo-localized build plan should preserve output_kind provenance");
    expect(
        pseudo_workspace.build_plan.build_target.find("[!! ") != std::string::npos,
        "#2495: pseudo-localized build target label should route through the catalog");
    expect(
        pseudo_workspace.build_plan.build_target.find("x64 Windows executable") == std::string::npos,
        "#2495: pseudo-localized build target label should not fall back to raw English prose");
    expect(
        pseudo_workspace.build_plan.build_target_field_index == 3U,
        "#2495: pseudo-localized build target should preserve field provenance");
    expect(
        pseudo_workspace.build_plan.build_target_memo_block_number == 4U,
        "#2495: pseudo-localized build target should preserve memo provenance");
    expect(
        !pseudo_workspace.entries.empty() && pseudo_workspace.entries[0].type_code == "H",
        "#2496: pseudo-localized project entry should preserve type codes");
    expect(
        !pseudo_workspace.entries.empty() && pseudo_workspace.entries[0].type_title.find("[!! ") != std::string::npos,
        "#2496: pseudo-localized project header type title should route through the catalog");
    expect(
        !pseudo_workspace.entries.empty() && pseudo_workspace.entries[0].type_title.find("Project Header") == std::string::npos,
        "#2496: pseudo-localized project header type title should not fall back to raw English prose");
    expect(
        !pseudo_workspace.entries.empty() && pseudo_workspace.entries[0].group_id == "project",
        "#2496: pseudo-localized project entry should preserve group ids");
    expect(
        !pseudo_workspace.entries.empty() && pseudo_workspace.entries[0].group_title.find("[!! ") != std::string::npos,
        "#2496: pseudo-localized project group title should route through the catalog");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].type_title.find("[!! ") != std::string::npos,
        "#2496: pseudo-localized program type title should route through the catalog");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].group_id == "programs",
        "#2496: pseudo-localized program entry should preserve group ids");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].type_title_field_index == 1U,
        "#2496: pseudo-localized project type title should preserve field provenance");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].type_title_memo_block_number == 11U,
        "#2496: pseudo-localized project type title should preserve memo provenance");
    expect(workspace.build_plan.startup_item == "main.prg", "build plan should choose the main program as startup item");
    expect(workspace.build_plan.startup_item_field_index == 1U, "#681: build plan startup item provenance should retain selected NAME field ordinal");
    expect(workspace.build_plan.startup_item_memo_block_number == 11U, "#715: build plan startup items should inherit selected entry NAME memo block provenance");
    expect(workspace.build_plan.startup_record_index == 1U, "build plan should keep the startup record index");
    expect(workspace.build_plan.debug_enabled, "build plan should capture project debug settings");
    expect(workspace.build_plan.project_key_field_index == 1U, "#663: build plan should preserve project key field provenance");
    expect(workspace.build_plan.project_key_memo_block_number == 0U, "#714: build-plan project keys should expose source memo block provenance");
    expect(workspace.build_plan.home_directory_field_index == 2U, "#663: build plan should preserve home directory field provenance");
    expect(workspace.build_plan.home_directory_memo_block_number == 3U, "#714: build-plan home directories should retain source memo block provenance");
    expect(workspace.build_plan.output_path_field_index == 3U, "#663: build plan should preserve output path field provenance");
    expect(workspace.build_plan.output_path_memo_block_number == 4U, "#714: build-plan output paths should retain source memo block provenance");
    expect(workspace.build_plan.output_kind_memo_block_number == 4U, "#714: build-plan output kind should inherit output path memo block provenance");
    expect(workspace.build_plan.build_target_memo_block_number == 4U, "#714: build-plan target should inherit output path memo block provenance");
    expect(workspace.build_plan.debug_field_index == 4U, "#663: build plan should preserve DEBUG field provenance");
    expect(workspace.build_plan.debug_memo_block_number == 5U, "#725: build DEBUG flags should retain memo block provenance");
    expect(workspace.build_plan.save_code_field_index == 5U, "#663: build plan should preserve SAVECODE field provenance");
    expect(workspace.build_plan.save_code_memo_block_number == 6U, "#725: build SAVECODE flags should retain memo block provenance");
    expect(workspace.build_plan.encrypt_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#663: missing ENCRYPT build-flag provenance should be explicit");
    expect(workspace.build_plan.encrypt_memo_block_number == 0U, "#725: missing ENCRYPT flags should expose memo block zero");
    expect(workspace.build_plan.no_logo_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#663: missing NOLOGO build-flag provenance should be explicit");
    expect(workspace.build_plan.no_logo_memo_block_number == 0U, "#725: missing NOLOGO flags should expose memo block zero");
    expect(workspace.build_plan.excluded_items == 1U, "build plan should count excluded items");
    expect(workspace.build_plan.deleted_items == 1U, "#687: build plan should count deleted source rows explicitly");
    expect(workspace.entries[0].type_field_index == 0U, "#662: project header type field ordinal should be preserved");
    expect(workspace.entries[0].type_memo_block_number == 0U, "#715: non-memo TYPE fields should expose memo block zero");
    expect(!workspace.entries[0].deleted, "#685: live project entries should preserve non-deleted state");
    expect(workspace.entries[0].type_title == "Project Header", "#2496: project header type title should preserve en-US prose");
    expect(workspace.entries[0].group_title == "Project", "#2496: project header group title should preserve en-US prose");
    expect(workspace.entries[0].type_title_field_index == 0U, "#680: TYPE-derived project entry classification provenance should be preserved");
    expect(workspace.entries[0].type_title_memo_block_number == 0U, "#715: TYPE-derived classifications should inherit TYPE memo block provenance");
    expect(workspace.entries[0].group_id_field_index == 0U, "#680: TYPE-derived project group id provenance should be preserved");
    expect(workspace.entries[0].group_id_memo_block_number == 0U, "#715: TYPE-derived group ids should inherit TYPE memo block provenance");
    expect(workspace.entries[0].group_title_field_index == 0U, "#680: TYPE-derived project group title provenance should be preserved");
    expect(workspace.entries[0].group_title_memo_block_number == 0U, "#715: TYPE-derived group titles should inherit TYPE memo block provenance");
    expect(workspace.entries[0].name_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#662: missing project header name provenance should be explicit");
    expect(workspace.entries[0].relative_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#679: missing project header relative path provenance should be explicit");
    expect(workspace.entries[0].key_field_index == 1U, "#662: project header key field ordinal should be preserved");
    expect(workspace.entries[1].type_field_index == 0U, "#662: project entry type field ordinal should be preserved");
    expect(workspace.entries[1].name_field_index == 1U, "#662: project entry name field ordinal should be preserved");
    expect(workspace.entries[1].name_memo_block_number == 11U, "#715: project entry names should retain NAME memo block provenance");
    expect(workspace.entries[1].type_title == "Program", "#680: project entry classification should still derive from NAME extension");
    expect(workspace.entries[1].type_title_field_index == 1U, "#680: NAME-derived project entry classification provenance should be preserved");
    expect(workspace.entries[1].type_title_memo_block_number == 11U, "#715: NAME-derived classifications should inherit NAME memo block provenance");
    expect(workspace.entries[1].group_id_field_index == 1U, "#680: NAME-derived project group id provenance should be preserved");
    expect(workspace.entries[1].group_id_memo_block_number == 11U, "#715: NAME-derived group ids should inherit NAME memo block provenance");
    expect(workspace.entries[1].group_title_field_index == 1U, "#680: NAME-derived project group title provenance should be preserved");
    expect(workspace.entries[1].group_title_memo_block_number == 11U, "#715: NAME-derived group titles should inherit NAME memo block provenance");
    expect(workspace.entries[1].relative_path == "main.prg", "#679: project entry relative path should preserve normalized path text");
    expect(workspace.entries[1].relative_path_field_index == 1U, "#679: project entry relative path provenance should retain selected NAME field ordinal");
    expect(workspace.entries[1].relative_path_memo_block_number == 11U, "#715: project entry relative paths should inherit NAME memo block provenance");
    expect(workspace.entries[1].comments_field_index == 3U, "#662: project entry comment field ordinal should be preserved");
    expect(workspace.entries[1].comments_memo_block_number == 13U, "#715: project entry comments should retain COMMENTS memo block provenance");
    expect(workspace.entries[1].main_program_field_index == 2U, "#677: MAINPROG project entry flag provenance should be preserved");
    expect(workspace.entries[1].main_program_memo_block_number == 12U, "#725: MAINPROG entry flags should retain memo block provenance");
    expect(workspace.entries[1].local_field_index == 4U, "#677: LOCAL project entry flag provenance should be preserved");
    expect(workspace.entries[1].local_memo_block_number == 14U, "#725: LOCAL entry flags should retain memo block provenance");
    expect(workspace.entries[1].exclude_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#677: missing EXCLUDE project entry flag provenance should be explicit");
    expect(workspace.entries[1].exclude_memo_block_number == 0U, "#725: missing EXCLUDE entry flags should expose memo block zero");
    expect(workspace.entries[2].exclude_field_index == 3U, "#677: EXCLUDE project entry flag provenance should be preserved");
    expect(workspace.entries[2].exclude_memo_block_number == 23U, "#725: EXCLUDE entry flags should retain memo block provenance even when false");
    expect(workspace.entries[2].main_program_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#677: missing MAINPROG project entry flag provenance should be explicit");
    expect(workspace.entries[2].main_program_memo_block_number == 0U, "#725: missing MAINPROG entry flags should expose memo block zero");
    expect(workspace.entries[2].group_id == "forms", "#680: project entry grouping should still derive from NAME extension");
    expect(workspace.entries[2].group_id_field_index == 1U, "#680: form grouping provenance should retain NAME field ordinal");
    expect(workspace.entries[2].relative_path_memo_block_number == 21U, "#715: form relative paths should inherit NAME memo block provenance");
    expect(workspace.entries[2].comments_memo_block_number == 22U, "#715: form comments should retain COMMENTS memo block provenance");
    expect(workspace.entries[3].excluded, "#677: EXCLUDE project entry flags should retain truth values");
    expect(workspace.entries[3].exclude_memo_block_number == 32U, "#725: true EXCLUDE entry flags should retain memo block provenance");
    expect(workspace.entries[4].deleted, "#685: deleted PJX records should stay visible on project entries");
    expect(workspace.entries[4].relative_path == R"(menus\oldmenu.mnx)", "#685: deleted entries should keep normalized path metadata");
    expect(workspace.entries[4].relative_path_memo_block_number == 41U, "#715: deleted entry relative paths should retain NAME memo block provenance");

    const auto project_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
        return group.id == "project";
    });
    expect(project_group != workspace.groups.end(), "#727: workspace should include a project metadata group");
    if (project_group != workspace.groups.end()) {
        expect(project_group->id_field_index == 0U, "#727: project group ids should retain TYPE field provenance");
        expect(project_group->id_memo_block_number == 0U, "#727: project group ids from non-memo TYPE should expose block zero");
        expect(project_group->title_field_index == 0U, "#727: project group titles should retain TYPE field provenance");
        expect(project_group->title_memo_block_number == 0U, "#727: project group titles from non-memo TYPE should expose block zero");
    }
    const auto programs_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
        return group.id == "programs";
    });
    expect(programs_group != workspace.groups.end(), "#727: workspace should include a programs group");
    if (programs_group != workspace.groups.end()) {
        expect(programs_group->id_field_index == 1U, "#727: program group ids should retain selected NAME field provenance");
        expect(programs_group->id_memo_block_number == 11U, "#727: program group ids should inherit selected NAME memo block provenance");
        expect(programs_group->title_field_index == 1U, "#727: program group titles should retain selected NAME field provenance");
        expect(programs_group->title_memo_block_number == 11U, "#727: program group titles should inherit selected NAME memo block provenance");
    }
    const auto forms_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
        return group.id == "forms";
    });
    expect(forms_group != workspace.groups.end(), "workspace should include a forms group");
    if (forms_group != workspace.groups.end()) {
        expect(forms_group->deleted_count == 0U, "#686: live-only project groups should expose zero deleted entries");
        expect(forms_group->id_field_index == 1U, "#727: form group ids should retain selected NAME field provenance");
        expect(forms_group->id_memo_block_number == 21U, "#727: form group ids should inherit selected NAME memo block provenance");
        expect(forms_group->title_field_index == 1U, "#727: form group titles should retain selected NAME field provenance");
        expect(forms_group->title_memo_block_number == 21U, "#727: form group titles should inherit selected NAME memo block provenance");
    }
    const auto menus_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
        return group.id == "menus";
    });
    expect(menus_group != workspace.groups.end(), "#686: deleted menu entries should still contribute to their project group");
    if (menus_group != workspace.groups.end()) {
        expect(menus_group->item_count == 1U, "#686: project group item count should continue to include deleted source rows");
        expect(menus_group->deleted_count == 1U, "#686: project groups should count deleted source rows explicitly");
        expect(menus_group->id_field_index == 1U, "#727: deleted-menu group ids should retain selected NAME field provenance");
        expect(menus_group->id_memo_block_number == 41U, "#727: deleted-menu group ids should inherit selected NAME memo block provenance");
        expect(menus_group->title_field_index == 1U, "#727: deleted-menu group titles should retain selected NAME field provenance");
        expect(menus_group->title_memo_block_number == 41U, "#727: deleted-menu group titles should inherit selected NAME memo block provenance");
    }
}

void test_project_workspace_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string_view> keys = {
        "Studio.ProjectWorkspace.Group.ClassLibraries",
        "Studio.ProjectWorkspace.Group.Code",
        "Studio.ProjectWorkspace.Group.Databases",
        "Studio.ProjectWorkspace.Group.Forms",
        "Studio.ProjectWorkspace.Group.Labels",
        "Studio.ProjectWorkspace.Group.Libraries",
        "Studio.ProjectWorkspace.Group.Menus",
        "Studio.ProjectWorkspace.Group.OtherAssets",
        "Studio.ProjectWorkspace.Group.OtherRecords",
        "Studio.ProjectWorkspace.Group.Programs",
        "Studio.ProjectWorkspace.Group.Project",
        "Studio.ProjectWorkspace.Group.ProjectItems",
        "Studio.ProjectWorkspace.Group.Queries",
        "Studio.ProjectWorkspace.Group.Reports",
        "Studio.ProjectWorkspace.Group.Tables",
        "Studio.ProjectWorkspace.ItemType.ClassLibrary",
        "Studio.ProjectWorkspace.ItemType.Database",
        "Studio.ProjectWorkspace.ItemType.Form",
        "Studio.ProjectWorkspace.ItemType.Header",
        "Studio.ProjectWorkspace.ItemType.Label",
        "Studio.ProjectWorkspace.ItemType.Library",
        "Studio.ProjectWorkspace.ItemType.Menu",
        "Studio.ProjectWorkspace.ItemType.Program",
        "Studio.ProjectWorkspace.ItemType.ProjectHeader",
        "Studio.ProjectWorkspace.ItemType.ProjectItem",
        "Studio.ProjectWorkspace.ItemType.ProjectRecord",
        "Studio.ProjectWorkspace.ItemType.Query",
        "Studio.ProjectWorkspace.ItemType.Report",
        "Studio.ProjectWorkspace.ItemType.Table",
        "Studio.ProjectWorkspace.BuildTarget.VisualFoxProApplicationArchive",
        "Studio.ProjectWorkspace.BuildTarget.VisualFoxProLibrary",
        "Studio.ProjectWorkspace.BuildTarget.VisualFoxProTokenizedProgram",
        "Studio.ProjectWorkspace.BuildTarget.WindowsActiveXControl",
        "Studio.ProjectWorkspace.BuildTarget.WindowsDynamicLinkLibrary",
        "Studio.ProjectWorkspace.BuildTarget.NativeExecutable",
        "Studio.ProjectWorkspace.BuildTarget.WindowsExecutable",
        "Studio.ProjectWorkspace.Fallback.RecordTitle"};

    expect(
        english_catalog.translate("Studio.ProjectWorkspace.Group.Forms") == "Forms",
        "#2611: project-workspace forms group should remain catalog-backed in en-US");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.Group.Forms") == "Formularios",
        "#2611: es-419 project-workspace forms group should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.ItemType.ProjectHeader") == "Encabezado del proyecto",
        "#2611: es-419 project-workspace project-header type should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.Group.Reports") == "Relatorios",
        "#2611: pt-BR project-workspace reports group should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.ItemType.Label") == "Rotulo",
        "#2611: pt-BR project-workspace label type should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.BuildTarget.WindowsExecutable") ==
            "Ejecutable de Windows x64",
        "#2622: es-419 project-workspace executable build target should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.BuildTarget.NativeExecutable") ==
            "Ejecutable nativo x64",
        "#2622: es-419 project-workspace native executable build target should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.BuildTarget.VisualFoxProLibrary") ==
            "Biblioteca de Visual FoxPro x64",
        "#2622: es-419 project-workspace VFP library build target should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.BuildTarget.WindowsDynamicLinkLibrary") ==
            "Biblioteca de vinculo dinamico do Windows x64",
        "#2622: pt-BR project-workspace DLL build target should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.BuildTarget.NativeExecutable") ==
            "Executavel nativo x64",
        "#2622: pt-BR project-workspace native executable build target should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.BuildTarget.VisualFoxProTokenizedProgram") ==
            "Programa tokenizado do Visual FoxPro x64",
        "#2622: pt-BR project-workspace tokenized-program build target should localize through the catalog");
    expect(
        spanish_catalog.translate("Studio.ProjectWorkspace.Fallback.RecordTitle") == "Registro {recordIndex}",
        "#2652: es-419 project-workspace fallback record title should localize through the catalog");
    expect(
        portuguese_catalog.translate("Studio.ProjectWorkspace.Fallback.RecordTitle") == "Registro {recordIndex}",
        "#2652: pt-BR project-workspace fallback record title should localize through the catalog");
    expect(
        pseudo_catalog.translate("Studio.ProjectWorkspace.Group.ProjectItems") ==
            copperfin::localization::pseudo_localize("Project Items"),
        "#2611: qps-ploc project-workspace group titles should resolve through the pseudo-localization transform");
    expect(
        pseudo_catalog.translate("Studio.ProjectWorkspace.BuildTarget.WindowsActiveXControl") ==
            copperfin::localization::pseudo_localize("x64 Windows ActiveX control"),
        "#2622: qps-ploc project-workspace build targets should resolve through the pseudo-localization transform");
    expect(
        pseudo_catalog.translate("Studio.ProjectWorkspace.BuildTarget.NativeExecutable") ==
            copperfin::localization::pseudo_localize("x64 native executable"),
        "#2622: qps-ploc native executable build target should resolve through the pseudo-localization transform");

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", keys) == 0U,
        "#2611: es-419 should define every remaining Studio.ProjectWorkspace group/item-type localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", keys) == 0U,
        "#2611: pt-BR should define every remaining Studio.ProjectWorkspace group/item-type localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", keys) == 0U,
        "#2611: qps-ploc should define every remaining Studio.ProjectWorkspace group/item-type localization key");
}

void test_build_project_workspace_localizes_titles_without_changing_ids() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\localized.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LOCALIZED"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms\\customer.scx", .memo_block_number = 11U}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "reports\\invoice.frx", .memo_block_number = 12U}
        })
    };

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");

    const auto spanish_workspace = copperfin::studio::build_project_workspace(document, spanish_catalog);
    expect(spanish_workspace.entries.size() == 3U, "#2611: es-419 project workspace should preserve entry detection");
    if (spanish_workspace.entries.size() >= 3U) {
        expect(spanish_workspace.entries[0].group_id == "project",
            "#2611: es-419 project workspace should preserve invariant group ids");
        expect(spanish_workspace.entries[0].group_title == "Proyecto",
            "#2611: es-419 project workspace should localize project group titles");
        expect(spanish_workspace.entries[1].type_title == "Formulario",
            "#2611: es-419 project workspace should localize form type titles");
        expect(spanish_workspace.entries[1].group_title == "Formularios",
            "#2611: es-419 project workspace should localize forms group titles");
        expect(spanish_workspace.entries[1].group_id == "forms",
            "#2611: es-419 project workspace should preserve invariant forms group ids");
        expect(spanish_workspace.entries[2].type_title == "Reporte",
            "#2611: es-419 project workspace should localize report type titles");
        expect(spanish_workspace.entries[2].group_title == "Reportes",
            "#2611: es-419 project workspace should localize reports group titles");
    }

    const auto portuguese_workspace = copperfin::studio::build_project_workspace(document, portuguese_catalog);
    expect(portuguese_workspace.entries.size() == 3U, "#2611: pt-BR project workspace should preserve entry detection");
    if (portuguese_workspace.entries.size() >= 3U) {
        expect(portuguese_workspace.entries[1].type_title == "Formulario",
            "#2611: pt-BR project workspace should localize form type titles");
        expect(portuguese_workspace.entries[1].group_title == "Formularios",
            "#2611: pt-BR project workspace should localize forms group titles");
        expect(portuguese_workspace.entries[2].type_title == "Relatorio",
            "#2611: pt-BR project workspace should localize report type titles");
        expect(portuguese_workspace.entries[2].group_title == "Relatorios",
            "#2611: pt-BR project workspace should localize reports group titles");
        expect(portuguese_workspace.entries[2].group_id == "reports",
            "#2611: pt-BR project workspace should preserve invariant reports group ids");
    }
}

void test_project_workspace_group_order_is_locale_invariant() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\group-order.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "GROUPORDER"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "assets\\library.dll"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "reports\\labels.lbx"}
        })
    };

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const std::vector<std::string_view> locales = {"en-US", "es-419", "pt-BR", "qps-ploc"};
    const std::vector<std::string> expected_group_ids = {"project", "labels", "libraries"};
    for (const auto locale : locales) {
        const auto catalog = copperfin::localization::load_catalogs(catalog_root, locale);
        const auto workspace = copperfin::studio::build_project_workspace(document, catalog);
        std::vector<std::string> group_ids;
        for (const auto& group : workspace.groups) {
            group_ids.push_back(group.id);
        }
        expect(group_ids == expected_group_ids,
               "#4328: workspace group IDs should have invariant project-first ordering under " +
                   std::string(locale));

        const auto labels_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
            return group.id == "labels";
        });
        const auto libraries_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
            return group.id == "libraries";
        });
        expect(labels_group != workspace.groups.end() && libraries_group != workspace.groups.end(),
               "#4328: workspace should retain labels and libraries groups under " + std::string(locale));
        if (labels_group != workspace.groups.end() && libraries_group != workspace.groups.end()) {
            expect(labels_group->title == catalog.translate("Studio.ProjectWorkspace.Group.Labels"),
                   "#4328: labels display title should remain catalog-backed under " + std::string(locale));
            expect(libraries_group->title == catalog.translate("Studio.ProjectWorkspace.Group.Libraries"),
                   "#4328: libraries display title should remain catalog-backed under " + std::string(locale));
        }
    }
}

void test_project_workspace_default_catalog_refreshes_after_locale_switch() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\locale-switch.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LOCALE_SWITCH"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms\\customer.scx"}
        })
    };

    copperfin::test_support::ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto english_workspace = copperfin::studio::build_project_workspace(document);
    expect(!english_workspace.groups.empty() && english_workspace.groups[0].title == "Project",
           "#4353: default project workspace should use the selected English catalog");
    locale.set("es-419");
    const auto spanish_workspace = copperfin::studio::build_project_workspace(document);
    expect(!spanish_workspace.groups.empty() && spanish_workspace.groups[0].title == "Proyecto",
           "#4353: default project workspace should refresh to Spanish after an in-process locale switch");
    expect(spanish_workspace.entries.size() > 1U && spanish_workspace.entries[1].type_title == "Formulario",
           "#4353: default project workspace entry labels should refresh after an in-process locale switch");
    locale.set("qps-ploc");
    const auto pseudo_workspace = copperfin::studio::build_project_workspace(document);
    expect(!pseudo_workspace.groups.empty() && pseudo_workspace.groups[0].title.find("[!! ") != std::string::npos,
           "#4353: default project workspace should refresh to pseudo-localization after an in-process locale switch");
    expect(!pseudo_workspace.groups.empty() && pseudo_workspace.groups[0].id == "project" &&
               pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].group_id == "forms",
           "#4353: locale refresh should preserve invariant workspace group identifiers");
}

void test_build_project_workspace_with_excluded_assets() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\legacy.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LEGACYAPP"},
            {.field_name = "HOMEDIR", .field_type = 'M', .display_value = R"(D:\OLD\APP)"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = "<memo block 918>"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms\\legacy.scx"},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "true"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LEGACY"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "legacy project workspace should still be available");
    expect(
        workspace.build_plan.output_path == expected_default_workspace_output_path(document.path, "LEGACYAPP"),
        "workspace should fall back to a platform-appropriate default output path when the stored memo output is unresolved");
    expect(workspace.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#678: unresolved memo output fallback should not masquerade as stored OUTFILE provenance");
    expect(workspace.project_title_field_index == 1U, "#678: workspace title should keep KEY provenance when KEY supplies title");
    expect(workspace.build_plan.output_kind == "executable", "default output path fallback should still infer executable output kind");
    expect(workspace.build_plan.output_kind_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#683: fallback output kind provenance should not masquerade as stored OUTFILE provenance");
    expect(workspace.build_plan.build_target == expected_default_workspace_build_target(),
           "#3790: fallback executable build target should stay coherent with the synthesized output path");
    expect(workspace.build_plan.build_target_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#683: fallback build target provenance should not masquerade as stored OUTFILE provenance");
    expect(
        workspace.build_plan.startup_item == R"(forms\legacy.scx)",
        "workspace should choose a real asset as startup even when every asset is excluded");
    expect(workspace.build_plan.startup_item_field_index == 1U,
           "#681: excluded fallback startup item provenance should retain selected NAME field ordinal");
    expect(
        workspace.build_plan.startup_record_index == 1U,
           "workspace should keep the excluded asset record index when it becomes the startup fallback");
}

void test_build_project_workspace_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\memodemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'M', .display_value = "<memo block 918>", .memo_block_number = 918U},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = "<memo block 919>", .memo_block_number = 919U}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "<memo block 920>", .memo_block_number = 920U},
            {.field_name = "COMMENTS", .field_type = 'M', .display_value = "<memo block 921>", .memo_block_number = 921U}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.project_key.empty(), "#694: unresolved memo placeholders should not become project keys");
    expect(workspace.project_key_field_index == 1U, "#694: unresolved project key fields should retain source provenance");
    expect(workspace.project_key_memo_block_number == 918U, "#714: unresolved project keys should retain raw memo block provenance");
    expect(workspace.project_title == "memodemo", "#694: unresolved memo project keys should fall back to the project filename");
    expect(workspace.project_title_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#694: fallback project titles should not masquerade as stored memo provenance");
    expect(workspace.project_title_memo_block_number == 0U, "#714: fallback project titles should expose memo block zero");
    expect(workspace.build_plan.project_title == "memodemo", "#726: build-plan fallback titles should mirror workspace titles");
    expect(workspace.build_plan.project_title_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#726: build-plan fallback titles should preserve missing-field provenance");
    expect(workspace.build_plan.project_title_memo_block_number == 0U,
           "#726: build-plan fallback titles should expose memo block zero");
    expect(workspace.output_path == expected_default_workspace_output_path(document.path, "memodemo"),
           "#694: unresolved memo OUTFILE values should keep platform-appropriate default output fallback behavior");
    expect(workspace.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#694: unresolved output placeholders should not masquerade as usable output provenance");
    expect(workspace.output_path_memo_block_number == 0U, "#714: fallback output paths should expose memo block zero");
    expect(workspace.build_plan.project_key_memo_block_number == 918U,
           "#714: build plans should retain unresolved project key memo block provenance");
    expect(workspace.build_plan.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#714: fallback build-plan output paths should keep missing-field provenance");
    expect(workspace.build_plan.output_path_memo_block_number == 0U,
           "#714: fallback build-plan output paths should expose memo block zero");
    expect(workspace.build_plan.output_kind_memo_block_number == 0U,
           "#714: fallback build-plan output kind should expose memo block zero");
    expect(workspace.build_plan.output_kind == "executable",
           "#3790: unresolved memo OUTFILE fallbacks should still classify as executables");
    expect(workspace.build_plan.build_target == expected_default_workspace_build_target(),
           "#3790: unresolved memo OUTFILE fallbacks should keep a coherent executable build target");
    expect(workspace.build_plan.build_target_memo_block_number == 0U,
           "#714: fallback build-plan target should expose memo block zero");
    expect(workspace.entries[1].name == "Record 1", "#694: unresolved memo names should use the synthetic entry fallback");
    const auto spanish_catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "es-419");
    const auto portuguese_catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "pt-BR");
    const auto spanish_workspace = copperfin::studio::build_project_workspace(document, spanish_catalog);
    const auto portuguese_workspace = copperfin::studio::build_project_workspace(document, portuguese_catalog);
    const auto pseudo_catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "qps-ploc");
    expect(
        spanish_workspace.entries.size() > 1U && spanish_workspace.entries[1].name == "Registro 1",
        "#2652: es-419 fallback record title should flow through the project workspace model");
    expect(
        portuguese_workspace.entries.size() > 1U && portuguese_workspace.entries[1].name == "Registro 1",
        "#2652: pt-BR fallback record title should flow through the project workspace model");
    const auto pseudo_workspace = copperfin::studio::build_project_workspace(document, pseudo_catalog);
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].record_index == 1U,
        "#2497: pseudo-localized fallback record title should preserve record index metadata");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].name.find("[!! ") != std::string::npos,
        "#2497: fallback record title should route through pseudo-localization");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].name.find("Record 1") == std::string::npos,
        "#2497: pseudo-localized fallback record title should not fall back to raw English prose");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].name.find("1") != std::string::npos,
        "#2497: fallback record title should preserve the named recordIndex placeholder value");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].name_field_index == 1U,
        "#2497: pseudo-localized fallback record title should preserve name field provenance");
    expect(
        pseudo_workspace.entries.size() > 1U && pseudo_workspace.entries[1].name_memo_block_number == 920U,
        "#2497: pseudo-localized fallback record title should preserve name memo provenance");
    expect(workspace.entries[1].name_field_index == 1U, "#694: unresolved memo name fields should retain source provenance");
    expect(workspace.entries[1].name_memo_block_number == 920U, "#715: unresolved entry names should retain source memo block provenance");
    expect(workspace.entries[1].relative_path.empty(), "#694: unresolved memo names should not become relative paths");
    expect(workspace.entries[1].relative_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#694: relative path provenance should be missing when usable name text is absent");
    expect(workspace.entries[1].relative_path_memo_block_number == 0U,
           "#715: missing relative paths should expose memo block zero");
    expect(workspace.entries[1].comments.empty(), "#694: unresolved memo comments should not become normalized comments");
    expect(workspace.entries[1].comments_field_index == 2U, "#694: unresolved memo comment fields should retain source provenance");
    expect(workspace.entries[1].comments_memo_block_number == 921U,
           "#715: unresolved entry comments should retain source memo block provenance");
}

void test_build_project_workspace_suppresses_vfp_source_output_sentinel() {
    const std::vector<std::string> sentinels = {"<Source>", " <sOuRcE> "};
    for (const auto& sentinel : sentinels) {
        copperfin::studio::StudioDocumentModel document;
        document.path = R"(E:\VFPSource\tasklist\tasklist.PJX)";
        document.kind = copperfin::studio::StudioAssetKind::project;
        document.table_preview_available = true;
        document.table_preview.records = {
            make_record(0, {
                {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
                {.field_name = "KEY", .field_type = 'C', .display_value = "TASKLIST"},
                {.field_name = "OUTFILE", .field_type = 'M', .display_value = sentinel, .memo_block_number = 42U}
            }),
            make_record(1, {
                {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
                {.field_name = "NAME", .field_type = 'M', .display_value = "twips.prg"},
                {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"}
            })
        };

        const auto workspace = copperfin::studio::build_project_workspace(document);
        expect(
            workspace.output_path == expected_default_workspace_output_path(document.path, "TASKLIST"),
            "#4672: VFP <Source> OUTFILE sentinels should synthesize the platform default output path");
        expect(
            workspace.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
            "#4672: VFP <Source> OUTFILE sentinels should not retain source field provenance");
        expect(
            workspace.output_path_memo_block_number == 0U,
            "#4672: VFP <Source> OUTFILE sentinels should not retain memo provenance");
        expect(
            workspace.build_plan.output_path == workspace.output_path &&
                workspace.build_plan.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
            "#4672: build plans should inherit normalized VFP <Source> output semantics");
        expect(
            workspace.build_plan.startup_item == "twips.prg",
            "#4672: output sentinel normalization should preserve the project startup item");
    }
}

void test_build_project_workspace_normalizes_vfp_absolute_item_paths() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\paths.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "PATHS"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\paths.exe)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\samples\forms\customer.scx)"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(E:\Shared\lib\helper.prg)"}
        }),
        make_record(3, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(reports\invoice.frx)"}
        }),
        make_record(4, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms/preview.scx"}
        }),
        make_record(5, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(reports\.\drafts/preview.frx)"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.entries.size() == 6U, "#700: workspace should include all path-spelling fixture entries");
    expect(workspace.entries[1].relative_path == R"(forms\customer.scx)",
           "#700: absolute VFP paths under the project directory should normalize to project-relative text");
    expect(workspace.entries[1].relative_path_field_index == 1U,
           "#700: normalized absolute project item paths should retain NAME provenance");
    expect(workspace.entries[2].relative_path == "helper.prg",
           "#700: absolute VFP paths outside the project directory should fall back to a deterministic filename");
    expect(workspace.entries[2].relative_path_field_index == 1U,
           "#700: outside-path fallback should still retain NAME provenance");
    expect(workspace.entries[3].relative_path == R"(reports\invoice.frx)",
           "#700: existing relative VFP paths should remain unchanged");
    expect(workspace.entries[4].relative_path == "forms/preview.scx",
           "#4078: existing forward-slash VFP paths should remain unchanged");
    expect(workspace.entries[5].relative_path == R"(reports\.\drafts/preview.frx)",
           "#4078: mixed separators and dot segments should remain unchanged in VFP metadata");
}

void test_build_project_workspace_normalizes_unc_item_paths() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(\\fileserver\share\Project-Copperfin\samples\paths.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "UNCPATHS"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(\\fileserver\share\Project-Copperfin\build\paths.exe)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(\\fileserver\share\Project-Copperfin\samples\forms\customer.scx)"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = R"(\\otherserver\share\lib\helper.prg)"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.entries.size() == 3U, "#701: workspace should include UNC path fixture entries");
    expect(workspace.entries[1].relative_path == R"(forms\customer.scx)",
           "#701: UNC paths under the project directory should normalize to project-relative text");
    expect(workspace.entries[1].relative_path_field_index == 1U,
           "#701: normalized UNC project item paths should retain NAME provenance");
    expect(workspace.entries[2].relative_path == "helper.prg",
           "#701: UNC paths outside the project directory should fall back to a deterministic filename");
    expect(workspace.entries[2].relative_path_field_index == 1U,
           "#701: outside UNC fallback should still retain NAME provenance");
}

void test_build_project_workspace_normalizes_vfp_logical_flags() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\flags.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "FLAGS"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\flags.exe)"},
            {.field_name = "DEBUG", .field_type = 'L', .display_value = ".T."},
            {.field_name = "ENCRYPT", .field_type = 'L', .display_value = " T "},
            {.field_name = "SAVECODE", .field_type = 'L', .display_value = "Y"},
            {.field_name = "NOLOGO", .field_type = 'L', .display_value = "TRUE"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = ".T."},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = " F "},
            {.field_name = "LOCAL", .field_type = 'L', .display_value = "Y"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "excluded.prg"},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "T"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.build_plan.debug_enabled, "#703: DEBUG should accept uppercase VFP .T. true values");
    expect(workspace.build_plan.encrypt_enabled, "#703: ENCRYPT should accept padded T true values");
    expect(workspace.build_plan.save_code, "#703: SAVECODE should accept Y true values");
    expect(workspace.build_plan.no_logo, "#703: NOLOGO should accept TRUE true values");
    expect(workspace.entries[1].main_program, "#703: MAINPROG should accept VFP .T. true values");
    expect(!workspace.entries[1].excluded, "#703: EXCLUDE should keep padded F values false");
    expect(workspace.entries[1].local, "#703: LOCAL should accept Y true values");
    expect(workspace.entries[2].excluded, "#703: EXCLUDE should accept uppercase T true values");
    expect(workspace.build_plan.startup_item == "main.prg",
           "#703: normalized MAINPROG should drive startup selection");
    expect(workspace.build_plan.excluded_items == 1U,
           "#703: normalized EXCLUDE should drive excluded item counts");
}

void test_build_project_workspace_normalizes_project_type_codes() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\typecodes.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = " h "},
            {.field_name = "KEY", .field_type = 'C', .display_value = "TYPECODES"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\typecodes.exe)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = " k "},
            {.field_name = "NAME", .field_type = 'M', .display_value = "README"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.project_title == "TYPECODES", "#704: lowercase padded header TYPE should still select the project header");
    expect(workspace.entries[0].type_code == "H", "#704: header type code should be normalized for downstream metadata");
    expect(workspace.entries[0].type_title == "Project Header", "#704: normalized header TYPE should keep header classification");
    expect(workspace.entries[1].type_code == "K", "#704: item type code should be normalized for downstream metadata");
    expect(workspace.entries[1].type_title == "Project Item",
           "#704: normalized K type should preserve Project Item fallback when extension classification is unavailable");
    expect(workspace.entries[1].type_title_field_index == 0U,
           "#704: TYPE-derived fallback classification should retain TYPE field provenance");
    expect(workspace.entries[1].group_id == "project_items",
           "#704: normalized K type should keep Project Items grouping fallback");
}

void test_build_project_workspace_prefers_live_header() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\headerdemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "DELETEDAPP"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\deleted.exe)"}
        }, true),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LIVEAPP"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\live.app)"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true", .memo_block_number = 57U}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "#692: workspace should still load projects with deleted header rows");
    expect(workspace.project_title == "LIVEAPP", "#692: workspace metadata should prefer the live project header");
    expect(workspace.project_title_field_index == 1U, "#692: live header title should retain KEY provenance");
    expect(workspace.build_plan.project_title == "LIVEAPP", "#726: build-plan titles should prefer the live project header");
    expect(workspace.build_plan.project_title_field_index == 1U,
           "#726: build-plan live-header titles should retain KEY field provenance");
    expect(workspace.build_plan.project_title_memo_block_number == 0U,
           "#726: build-plan live-header titles should retain KEY memo block provenance");
    expect(workspace.output_path == R"(E:\Project-Copperfin\build\live.app)",
           "#692: active output metadata should come from the live project header");
    expect(workspace.build_plan.output_kind == "app", "#692: build plan should infer output kind from the live header");
    expect(workspace.entries.size() == 3U, "#692: deleted project headers should remain visible as entries");
    expect(workspace.entries[0].deleted, "#692: deleted header entries should retain deleted state");
    expect(workspace.build_plan.deleted_items == 1U, "#692: build plan should still count deleted header rows");
}

void test_build_project_workspace_skips_deleted_startup_candidates() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\startupdemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "STARTUPDEMO"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\startupdemo.exe)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "deletedmain.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true", .memo_block_number = 57U}
        }, true),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "livemain.prg"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.build_plan.startup_item == "livemain.prg",
           "#693: deleted MAINPROG entries should not become active startup candidates");
    expect(workspace.build_plan.startup_item_field_index == 1U,
           "#693: live fallback startup item should retain selected NAME field ordinal");
    expect(workspace.build_plan.startup_record_index == 2U,
           "#693: startup record index should point at the selected live program");
    expect(workspace.entries[1].deleted, "#693: deleted startup candidate entries should remain visible");
    expect(workspace.entries[1].main_program_memo_block_number == 57U,
           "#725: deleted MAINPROG candidate flags should retain memo block provenance");
    expect(workspace.build_plan.deleted_items == 1U, "#693: deleted startup candidates should remain counted");
}

void test_build_project_workspace_with_dll_output() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\librarydemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LIBRARYDEMO"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\librarydemo.dll)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "librarymain.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "library project workspace should be available");
    expect(workspace.build_plan.output_kind == "dll", "build plan should infer DLL output kind from .dll output path");
    expect(workspace.build_plan.build_target == "x64 Windows dynamic-link library",
           "build plan should label .dll outputs as Windows dynamic-link libraries");
}

void test_build_project_workspace_with_fll_output() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\librarydemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "LIBRARYDEMO"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\librarydemo.fll)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "librarymain.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "fll project workspace should be available");
    expect(workspace.build_plan.output_kind == "fll", "build plan should infer FLL output kind from .fll output path");
    expect(workspace.build_plan.build_target == "x64 Visual FoxPro library",
           "build plan should label .fll outputs as Visual FoxPro libraries");
}

void test_build_project_workspace_with_fxp_output() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\compiledemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "COMPILEDEMO"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\compiledemo.fxp)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "fxp project workspace should be available");
    expect(workspace.build_plan.output_kind == "fxp", "build plan should infer FXP output kind from .fxp output path");
    expect(workspace.build_plan.build_target == "x64 Visual FoxPro tokenized program",
           "build plan should label .fxp outputs as Visual FoxPro tokenized programs");
}

void test_build_project_workspace_with_app_output() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\archivedemo.pjx)";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "H"},
            {.field_name = "KEY", .field_type = 'C', .display_value = "ARCHIVEDEMO"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\archivedemo.app)"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "app project workspace should be available");
    expect(workspace.build_plan.output_kind == "app", "build plan should infer APP output kind from .app output path");
    expect(workspace.build_plan.build_target == "x64 Visual FoxPro application archive",
           "build plan should label .app outputs as Visual FoxPro application archives");
}

void test_project_workspace_ascii_folding_preserves_utf8_metadata() {
    const std::string utf8_stem = "caf\xC3\xA9";
    copperfin::studio::StudioDocumentModel document;
    document.path = "C:/projects/" + utf8_stem + ".pjx";
    document.kind = copperfin::studio::StudioAssetKind::project;
    document.table_preview_available = true;
    document.table_preview.records = {
        make_record(0, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "h"},
            {.field_name = "KEY", .field_type = 'C', .display_value = utf8_stem},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = "C:/build/" + utf8_stem + ".EXE"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "k"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms/" + utf8_stem + ".SCX"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = ".t."}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "#3973: UTF-8 project metadata should remain classifiable");
    expect(workspace.project_key == utf8_stem,
           "#3973: project-key bytes should survive ASCII type normalization");
    expect(workspace.entries.size() == 2U,
           "#3973: UTF-8 project metadata should preserve both project records");
    if (workspace.entries.size() == 2U) {
        expect(workspace.entries[1].group_id == "forms",
               "#3973: uppercase ASCII SCX extensions should still classify as forms");
        expect(workspace.entries[1].relative_path == "forms/" + utf8_stem + ".SCX",
               "#3973: project-item path folding should preserve UTF-8 filename bytes");
    }
    expect(workspace.build_plan.output_kind == "executable",
           "#3973: uppercase ASCII output extensions should retain existing classification");
    expect(workspace.build_plan.startup_item == "forms/" + utf8_stem + ".SCX",
           "#3973: startup-item selection should preserve UTF-8 path bytes");
}

}  // namespace

int main() {
    test_build_project_workspace();
    test_project_workspace_catalog_entries_cover_placeholder_locales();
    test_build_project_workspace_localizes_titles_without_changing_ids();
    test_project_workspace_group_order_is_locale_invariant();
    test_project_workspace_default_catalog_refreshes_after_locale_switch();
    test_build_project_workspace_with_excluded_assets();
    test_build_project_workspace_suppresses_unresolved_memo_placeholders();
    test_build_project_workspace_suppresses_vfp_source_output_sentinel();
    test_build_project_workspace_normalizes_vfp_absolute_item_paths();
    test_build_project_workspace_normalizes_unc_item_paths();
    test_build_project_workspace_normalizes_vfp_logical_flags();
    test_build_project_workspace_normalizes_project_type_codes();
    test_build_project_workspace_prefers_live_header();
    test_build_project_workspace_skips_deleted_startup_candidates();
    test_build_project_workspace_with_dll_output();
    test_build_project_workspace_with_fll_output();
    test_build_project_workspace_with_fxp_output();
    test_build_project_workspace_with_app_output();
    test_project_workspace_ascii_folding_preserves_utf8_metadata();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
