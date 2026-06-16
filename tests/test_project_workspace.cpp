#include "copperfin/studio/project_workspace.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

copperfin::vfp::DbfRecord make_record(
    std::size_t record_index,
    std::initializer_list<copperfin::vfp::DbfRecordValue> values) {
    copperfin::vfp::DbfRecord record;
    record.record_index = record_index;
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
            {.field_name = "HOMEDIR", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\samples)"},
            {.field_name = "OUTFILE", .field_type = 'M', .display_value = R"(E:\Project-Copperfin\build\demoapp.exe)"},
            {.field_name = "DEBUG", .field_type = 'L', .display_value = "true"},
            {.field_name = "SAVECODE", .field_type = 'L', .display_value = "false"}
        }),
        make_record(1, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "main.prg"},
            {.field_name = "MAINPROG", .field_type = 'L', .display_value = "true"},
            {.field_name = "COMMENTS", .field_type = 'M', .display_value = "Application entry point"},
            {.field_name = "LOCAL", .field_type = 'L', .display_value = "true"}
        }),
        make_record(2, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "forms\\customer.scx"},
            {.field_name = "COMMENTS", .field_type = 'M', .display_value = "Customer maintenance"},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "false"}
        }),
        make_record(3, {
            {.field_name = "TYPE", .field_type = 'C', .display_value = "K"},
            {.field_name = "NAME", .field_type = 'M', .display_value = "reports\\invoice.frx"},
            {.field_name = "EXCLUDE", .field_type = 'L', .display_value = "true"}
        })
    };

    const auto workspace = copperfin::studio::build_project_workspace(document);
    expect(workspace.available, "project workspace should be available for PJX documents");
    expect(workspace.project_title == "DEMOAPP", "workspace should use the project key as its title");
    expect(workspace.project_title_field_index == 1U, "#678: workspace project title should retain selected KEY field provenance");
    expect(workspace.project_key_field_index == 1U, "#678: workspace project key field provenance should be preserved");
    expect(workspace.home_directory_field_index == 2U, "#678: workspace home directory field provenance should be preserved");
    expect(workspace.output_path_field_index == 3U, "#678: workspace output path field provenance should be preserved");
    expect(workspace.entries.size() == 4U, "workspace should include all project records");
    expect(workspace.groups.size() >= 3U, "workspace should group header, program, and form/report items");
    expect(workspace.build_plan.available, "build plan should be available");
    expect(workspace.build_plan.can_build, "build plan should be buildable with entries and an output path");
    expect(workspace.build_plan.output_path == R"(E:\Project-Copperfin\build\demoapp.exe)", "build plan should keep the explicit output path");
    expect(workspace.build_plan.output_kind == "executable", "build plan should infer executable output kind from .exe output path");
    expect(workspace.build_plan.build_target == "x64 Windows executable", "build plan should label .exe outputs as Windows executables");
    expect(workspace.build_plan.startup_item == "main.prg", "build plan should choose the main program as startup item");
    expect(workspace.build_plan.startup_item_field_index == 1U, "#681: build plan startup item provenance should retain selected NAME field ordinal");
    expect(workspace.build_plan.startup_record_index == 1U, "build plan should keep the startup record index");
    expect(workspace.build_plan.debug_enabled, "build plan should capture project debug settings");
    expect(workspace.build_plan.project_key_field_index == 1U, "#663: build plan should preserve project key field provenance");
    expect(workspace.build_plan.home_directory_field_index == 2U, "#663: build plan should preserve home directory field provenance");
    expect(workspace.build_plan.output_path_field_index == 3U, "#663: build plan should preserve output path field provenance");
    expect(workspace.build_plan.debug_field_index == 4U, "#663: build plan should preserve DEBUG field provenance");
    expect(workspace.build_plan.save_code_field_index == 5U, "#663: build plan should preserve SAVECODE field provenance");
    expect(workspace.build_plan.encrypt_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#663: missing ENCRYPT build-flag provenance should be explicit");
    expect(workspace.build_plan.no_logo_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#663: missing NOLOGO build-flag provenance should be explicit");
    expect(workspace.build_plan.excluded_items == 1U, "build plan should count excluded items");
    expect(workspace.entries[0].type_field_index == 0U, "#662: project header type field ordinal should be preserved");
    expect(workspace.entries[0].type_title_field_index == 0U, "#680: TYPE-derived project entry classification provenance should be preserved");
    expect(workspace.entries[0].group_id_field_index == 0U, "#680: TYPE-derived project group id provenance should be preserved");
    expect(workspace.entries[0].group_title_field_index == 0U, "#680: TYPE-derived project group title provenance should be preserved");
    expect(workspace.entries[0].name_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#662: missing project header name provenance should be explicit");
    expect(workspace.entries[0].relative_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#679: missing project header relative path provenance should be explicit");
    expect(workspace.entries[0].key_field_index == 1U, "#662: project header key field ordinal should be preserved");
    expect(workspace.entries[1].type_field_index == 0U, "#662: project entry type field ordinal should be preserved");
    expect(workspace.entries[1].name_field_index == 1U, "#662: project entry name field ordinal should be preserved");
    expect(workspace.entries[1].type_title == "Program", "#680: project entry classification should still derive from NAME extension");
    expect(workspace.entries[1].type_title_field_index == 1U, "#680: NAME-derived project entry classification provenance should be preserved");
    expect(workspace.entries[1].group_id_field_index == 1U, "#680: NAME-derived project group id provenance should be preserved");
    expect(workspace.entries[1].group_title_field_index == 1U, "#680: NAME-derived project group title provenance should be preserved");
    expect(workspace.entries[1].relative_path == "main.prg", "#679: project entry relative path should preserve normalized path text");
    expect(workspace.entries[1].relative_path_field_index == 1U, "#679: project entry relative path provenance should retain selected NAME field ordinal");
    expect(workspace.entries[1].comments_field_index == 3U, "#662: project entry comment field ordinal should be preserved");
    expect(workspace.entries[1].main_program_field_index == 2U, "#677: MAINPROG project entry flag provenance should be preserved");
    expect(workspace.entries[1].local_field_index == 4U, "#677: LOCAL project entry flag provenance should be preserved");
    expect(workspace.entries[1].exclude_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#677: missing EXCLUDE project entry flag provenance should be explicit");
    expect(workspace.entries[2].exclude_field_index == 3U, "#677: EXCLUDE project entry flag provenance should be preserved");
    expect(workspace.entries[2].main_program_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#677: missing MAINPROG project entry flag provenance should be explicit");
    expect(workspace.entries[2].group_id == "forms", "#680: project entry grouping should still derive from NAME extension");
    expect(workspace.entries[2].group_id_field_index == 1U, "#680: form grouping provenance should retain NAME field ordinal");

    const auto forms_group = std::find_if(workspace.groups.begin(), workspace.groups.end(), [](const auto& group) {
        return group.id == "forms";
    });
    expect(forms_group != workspace.groups.end(), "workspace should include a forms group");
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
        workspace.build_plan.output_path == R"(E:\Project-Copperfin\samples\LEGACYAPP.exe)",
        "workspace should fall back to a default output path when the stored memo output is unresolved");
    expect(workspace.output_path_field_index == copperfin::studio::StudioProjectMissingFieldIndex,
           "#678: unresolved memo output fallback should not masquerade as stored OUTFILE provenance");
    expect(workspace.project_title_field_index == 1U, "#678: workspace title should keep KEY provenance when KEY supplies title");
    expect(workspace.build_plan.output_kind == "executable", "default output path fallback should still infer executable output kind");
    expect(
        workspace.build_plan.startup_item == R"(forms\legacy.scx)",
        "workspace should choose a real asset as startup even when every asset is excluded");
    expect(workspace.build_plan.startup_item_field_index == 1U,
           "#681: excluded fallback startup item provenance should retain selected NAME field ordinal");
    expect(
        workspace.build_plan.startup_record_index == 1U,
        "workspace should keep the excluded asset record index when it becomes the startup fallback");
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

}  // namespace

int main() {
    test_build_project_workspace();
    test_build_project_workspace_with_excluded_assets();
    test_build_project_workspace_with_dll_output();
    test_build_project_workspace_with_fll_output();
    test_build_project_workspace_with_fxp_output();
    test_build_project_workspace_with_app_output();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
