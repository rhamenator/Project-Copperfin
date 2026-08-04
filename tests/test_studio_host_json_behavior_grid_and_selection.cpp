// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_continuous_scroll(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CONTINUOUS", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1081: synthetic SCX table for object continuous scroll should be created");
}

void write_synthetic_form_table_for_object_sparse(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SPARSE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1084: synthetic SCX table for object sparse should be created");
}

void write_synthetic_form_table_for_object_allow_cell_selection(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALLOWCELLS", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1086: synthetic SCX table for object allow cell selection should be created");
}

void write_synthetic_form_table_for_object_hide_selection(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIDESELECT", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1109: synthetic SCX table for object hide selection should be created");
}

void write_synthetic_form_table_for_object_record_mark(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "RECORDMARK", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1088: synthetic SCX table for object record mark should be created");
}

void write_synthetic_form_table_for_object_highlight_row(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIGHLIGHTR", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1090: synthetic SCX table for object highlight row should be created");
}

void write_synthetic_form_table_for_object_allow_header_sizing(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALLOWHEADE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1092: synthetic SCX table for object allow header sizing should be created");
}

void write_synthetic_form_table_for_object_allow_row_sizing(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALLOWROWSI", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1093: synthetic SCX table for object allow row sizing should be created");
}

void test_studio_host_json_assigns_continuous_scroll_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_continuous_scroll_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path continuous_scroll_path = temp_root / "continuous_scroll.scx";
    write_synthetic_form_table_for_object_continuous_scroll(continuous_scroll_path);
    const auto continuous_scroll_process = run_process_capture(
        studio_host_path,
        {
            "--path", continuous_scroll_path.string(),
            "--continuous-scroll-object",
            "--continuous-scroll", "false",
            "--continuous-scroll-target-object-name", "frmCustomer",
            "--continuous-scroll-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(continuous_scroll_process.exit_code == 0,
        "#1081: host object continuous-scroll assignment should exit successfully");
    expect(visual_object_property(continuous_scroll_path, "one-guid", "CONTINUOUSSCROLL") == "false" &&
            visual_object_property(continuous_scroll_path, "two-guid", "CONTINUOUSSCROLL") == "false" &&
            visual_object_property(continuous_scroll_path, "three-guid", "CONTINUOUSSCROLL") == "false" &&
            visual_object_property(continuous_scroll_path, "other-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: host object continuous-scroll assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_continuous_scroll(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--continuous-scroll-object",
            "--continuous-scroll", "false",
            "--continuous-scroll-target-unique-id", "one-guid",
            "--continuous-scroll-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1081: missing-target host object continuous-scroll assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CONTINUOUSSCROLL") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: missing-target host object continuous-scroll assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_continuous_scroll(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--continuous-scroll-object",
            "--continuous-scroll", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1081: continuous-scroll-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: continuous-scroll-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_continuous_scroll(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--continuous-scroll-object",
            "--continuous-scroll-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1081: continuous-scroll-object without continuous-scroll value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: continuous-scroll-object without continuous-scroll value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_continuous_scroll(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--continuous-scroll-object",
            "--continuous-scroll", "false",
            "--continuous-scroll-target-unique-id", "one-guid",
            "--continuous-scroll-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1081: duplicate-target host object continuous-scroll assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: duplicate-target host object continuous-scroll assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_continuous_scroll(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--continuous-scroll-object",
            "--auto-size-object",
            "--continuous-scroll", "false",
            "--continuous-scroll-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1081: continuous-scroll-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CONTINUOUSSCROLL") == "true",
        "#1081: continuous-scroll-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_sparse_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_sparse_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path sparse_path = temp_root / "sparse.scx";
    write_synthetic_form_table_for_object_sparse(sparse_path);
    const auto sparse_process = run_process_capture(
        studio_host_path,
        {
            "--path", sparse_path.string(),
            "--sparse-object",
            "--sparse", "false",
            "--sparse-target-object-name", "frmCustomer",
            "--sparse-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(sparse_process.exit_code == 0,
        "#1084: host object sparse assignment should exit successfully");
    expect(visual_object_property(sparse_path, "one-guid", "SPARSE") == "false" &&
            visual_object_property(sparse_path, "two-guid", "SPARSE") == "false" &&
            visual_object_property(sparse_path, "three-guid", "SPARSE") == "false" &&
            visual_object_property(sparse_path, "other-guid", "SPARSE") == "true",
        "#1084: host object sparse assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_sparse(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--sparse-object",
            "--sparse", "false",
            "--sparse-target-unique-id", "one-guid",
            "--sparse-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1084: missing-target host object sparse assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SPARSE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "SPARSE") == "true",
        "#1084: missing-target host object sparse assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_sparse(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--sparse-object",
            "--sparse", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1084: sparse-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SPARSE") == "true",
        "#1084: sparse-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_sparse(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--sparse-object",
            "--sparse-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1084: sparse-object without sparse value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SPARSE") == "true",
        "#1084: sparse-object without sparse value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_sparse(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--sparse-object",
            "--sparse", "false",
            "--sparse-target-unique-id", "one-guid",
            "--sparse-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1084: duplicate-target host object sparse assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SPARSE") == "true",
        "#1084: duplicate-target host object sparse assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_sparse(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--sparse-object",
            "--auto-size-object",
            "--sparse", "false",
            "--sparse-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1084: sparse-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SPARSE") == "true",
        "#1084: sparse-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_allow_cell_selection_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_allow_cell_selection_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path allow_cell_selection_path = temp_root / "allow_cell_selection.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(allow_cell_selection_path);
    const auto allow_cell_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", allow_cell_selection_path.string(),
            "--allow-cell-selection-object",
            "--allow-cell-selection", "false",
            "--allow-cell-selection-target-object-name", "frmCustomer",
            "--allow-cell-selection-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(allow_cell_selection_process.exit_code == 0,
        "#1086: host object allow-cell-selection assignment should exit successfully");
    expect(visual_object_property(allow_cell_selection_path, "one-guid", "ALLOWCELLSELECTION") == "false" &&
            visual_object_property(allow_cell_selection_path, "two-guid", "ALLOWCELLSELECTION") == "false" &&
            visual_object_property(allow_cell_selection_path, "three-guid", "ALLOWCELLSELECTION") == "false" &&
            visual_object_property(allow_cell_selection_path, "other-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: host object allow-cell-selection assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--allow-cell-selection-object",
            "--allow-cell-selection", "false",
            "--allow-cell-selection-target-unique-id", "one-guid",
            "--allow-cell-selection-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1086: missing-target host object allow-cell-selection assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALLOWCELLSELECTION") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: missing-target host object allow-cell-selection assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--allow-cell-selection-object",
            "--allow-cell-selection", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1086: allow-cell-selection-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: allow-cell-selection-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--allow-cell-selection-object",
            "--allow-cell-selection-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1086: allow-cell-selection-object without allow-cell-selection value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: allow-cell-selection-object without allow-cell-selection value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--allow-cell-selection-object",
            "--allow-cell-selection", "false",
            "--allow-cell-selection-target-unique-id", "one-guid",
            "--allow-cell-selection-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1086: duplicate-target host object allow-cell-selection assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: duplicate-target host object allow-cell-selection assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_allow_cell_selection(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--allow-cell-selection-object",
            "--auto-size-object",
            "--allow-cell-selection", "false",
            "--allow-cell-selection-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1086: allow-cell-selection-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALLOWCELLSELECTION") == "true",
        "#1086: allow-cell-selection-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_hide_selection_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_hide_selection_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path hide_selection_path = temp_root / "hide_selection.scx";
    write_synthetic_form_table_for_object_hide_selection(hide_selection_path);
    const auto hide_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", hide_selection_path.string(),
            "--hide-selection-object",
            "--hide-selection", "false",
            "--hide-selection-target-object-name", "frmCustomer",
            "--hide-selection-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(hide_selection_process.exit_code == 0,
        "#1109: host object hide-selection assignment should exit successfully");
    expect(visual_object_property(hide_selection_path, "one-guid", "HIDESELECTION") == "false" &&
            visual_object_property(hide_selection_path, "two-guid", "HIDESELECTION") == "false" &&
            visual_object_property(hide_selection_path, "three-guid", "HIDESELECTION") == "false" &&
            visual_object_property(hide_selection_path, "other-guid", "HIDESELECTION") == "true",
        "#1109: host object hide-selection assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_hide_selection(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--hide-selection-object",
            "--hide-selection", "false",
            "--hide-selection-target-unique-id", "one-guid",
            "--hide-selection-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1109: missing-target host object hide-selection assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIDESELECTION") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "HIDESELECTION") == "true",
        "#1109: missing-target host object hide-selection assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_hide_selection(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--hide-selection-object",
            "--hide-selection", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1109: hide-selection-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIDESELECTION") == "true",
        "#1109: hide-selection-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_hide_selection(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--hide-selection-object",
            "--hide-selection-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1109: hide-selection-object without hide-selection value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIDESELECTION") == "true",
        "#1109: hide-selection-object without hide-selection value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_hide_selection(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--hide-selection-object",
            "--hide-selection", "false",
            "--hide-selection-target-unique-id", "one-guid",
            "--hide-selection-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1109: duplicate-target host object hide-selection assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIDESELECTION") == "true",
        "#1109: duplicate-target host object hide-selection assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_hide_selection(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--hide-selection-object",
            "--auto-size-object",
            "--hide-selection", "false",
            "--hide-selection-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1109: hide-selection-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIDESELECTION") == "true",
        "#1109: hide-selection-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_record_mark_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_record_mark_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path record_mark_path = temp_root / "record_mark.scx";
    write_synthetic_form_table_for_object_record_mark(record_mark_path);
    const auto record_mark_process = run_process_capture(
        studio_host_path,
        {
            "--path", record_mark_path.string(),
            "--record-mark-object",
            "--record-mark", "false",
            "--record-mark-target-object-name", "frmCustomer",
            "--record-mark-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(record_mark_process.exit_code == 0,
        "#1088: host object record-mark assignment should exit successfully");
    expect(visual_object_property(record_mark_path, "one-guid", "RECORDMARK") == "false" &&
            visual_object_property(record_mark_path, "two-guid", "RECORDMARK") == "false" &&
            visual_object_property(record_mark_path, "three-guid", "RECORDMARK") == "false" &&
            visual_object_property(record_mark_path, "other-guid", "RECORDMARK") == "true",
        "#1088: host object record-mark assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_record_mark(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--record-mark-object",
            "--record-mark", "false",
            "--record-mark-target-unique-id", "one-guid",
            "--record-mark-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1088: missing-target host object record-mark assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "RECORDMARK") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "RECORDMARK") == "true",
        "#1088: missing-target host object record-mark assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_record_mark(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--record-mark-object",
            "--record-mark", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1088: record-mark-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "RECORDMARK") == "true",
        "#1088: record-mark-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_record_mark(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--record-mark-object",
            "--record-mark-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1088: record-mark-object without record-mark value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "RECORDMARK") == "true",
        "#1088: record-mark-object without record-mark value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_record_mark(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--record-mark-object",
            "--record-mark", "false",
            "--record-mark-target-unique-id", "one-guid",
            "--record-mark-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1088: duplicate-target host object record-mark assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "RECORDMARK") == "true",
        "#1088: duplicate-target host object record-mark assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_record_mark(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--record-mark-object",
            "--auto-size-object",
            "--record-mark", "false",
            "--record-mark-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1088: record-mark-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "RECORDMARK") == "true",
        "#1088: record-mark-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_row_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_highlight_row_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_row_path = temp_root / "highlight_row.scx";
    write_synthetic_form_table_for_object_highlight_row(highlight_row_path);
    const auto highlight_row_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_row_path.string(),
            "--highlight-row-object",
            "--highlight-row", "false",
            "--highlight-row-target-object-name", "frmCustomer",
            "--highlight-row-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_row_process.exit_code == 0,
        "#1090: host object highlight-row assignment should exit successfully");
    expect(visual_object_property(highlight_row_path, "one-guid", "HIGHLIGHTROW") == "false" &&
            visual_object_property(highlight_row_path, "two-guid", "HIGHLIGHTROW") == "false" &&
            visual_object_property(highlight_row_path, "three-guid", "HIGHLIGHTROW") == "false" &&
            visual_object_property(highlight_row_path, "other-guid", "HIGHLIGHTROW") == "true",
        "#1090: host object highlight-row assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_row(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-row-object",
            "--highlight-row", "false",
            "--highlight-row-target-unique-id", "one-guid",
            "--highlight-row-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1090: missing-target host object highlight-row assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTROW") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTROW") == "true",
        "#1090: missing-target host object highlight-row assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_row(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-row-object",
            "--highlight-row", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1090: highlight-row-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTROW") == "true",
        "#1090: highlight-row-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_row(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-row-object",
            "--highlight-row-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1090: highlight-row-object without highlight-row value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTROW") == "true",
        "#1090: highlight-row-object without highlight-row value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_row(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-row-object",
            "--highlight-row", "false",
            "--highlight-row-target-unique-id", "one-guid",
            "--highlight-row-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1090: duplicate-target host object highlight-row assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTROW") == "true",
        "#1090: duplicate-target host object highlight-row assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_row(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-row-object",
            "--auto-size-object",
            "--highlight-row", "false",
            "--highlight-row-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1090: highlight-row-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTROW") == "true",
        "#1090: highlight-row-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_allow_header_sizing_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_allow_header_sizing_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path allow_header_sizing_path = temp_root / "allow_header_sizing.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(allow_header_sizing_path);
    const auto allow_header_sizing_process = run_process_capture(
        studio_host_path,
        {
            "--path", allow_header_sizing_path.string(),
            "--allow-header-sizing-object",
            "--allow-header-sizing", "false",
            "--allow-header-sizing-target-object-name", "frmCustomer",
            "--allow-header-sizing-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(allow_header_sizing_process.exit_code == 0,
        "#1092: host object allow-header-sizing assignment should exit successfully");
    expect(visual_object_property(allow_header_sizing_path, "one-guid", "ALLOWHEADERSIZING") == "false" &&
            visual_object_property(allow_header_sizing_path, "two-guid", "ALLOWHEADERSIZING") == "false" &&
            visual_object_property(allow_header_sizing_path, "three-guid", "ALLOWHEADERSIZING") == "false" &&
            visual_object_property(allow_header_sizing_path, "other-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: host object allow-header-sizing assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--allow-header-sizing-object",
            "--allow-header-sizing", "false",
            "--allow-header-sizing-target-unique-id", "one-guid",
            "--allow-header-sizing-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1092: missing-target host object allow-header-sizing assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALLOWHEADERSIZING") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: missing-target host object allow-header-sizing assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--allow-header-sizing-object",
            "--allow-header-sizing", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1092: allow-header-sizing-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: allow-header-sizing-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--allow-header-sizing-object",
            "--allow-header-sizing-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1092: allow-header-sizing-object without allow-header-sizing value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: allow-header-sizing-object without allow-header-sizing value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--allow-header-sizing-object",
            "--allow-header-sizing", "false",
            "--allow-header-sizing-target-unique-id", "one-guid",
            "--allow-header-sizing-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1092: duplicate-target host object allow-header-sizing assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: duplicate-target host object allow-header-sizing assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_allow_header_sizing(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--allow-header-sizing-object",
            "--auto-size-object",
            "--allow-header-sizing", "false",
            "--allow-header-sizing-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1092: allow-header-sizing-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALLOWHEADERSIZING") == "true",
        "#1092: allow-header-sizing-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_allow_row_sizing_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_allow_row_sizing_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path allow_row_sizing_path = temp_root / "allow_row_sizing.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(allow_row_sizing_path);
    const auto allow_row_sizing_process = run_process_capture(
        studio_host_path,
        {
            "--path", allow_row_sizing_path.string(),
            "--allow-row-sizing-object",
            "--allow-row-sizing", "false",
            "--allow-row-sizing-target-object-name", "frmCustomer",
            "--allow-row-sizing-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(allow_row_sizing_process.exit_code == 0,
        "#1093: host object allow-row-sizing assignment should exit successfully");
    expect(visual_object_property(allow_row_sizing_path, "one-guid", "ALLOWROWSIZING") == "false" &&
            visual_object_property(allow_row_sizing_path, "two-guid", "ALLOWROWSIZING") == "false" &&
            visual_object_property(allow_row_sizing_path, "three-guid", "ALLOWROWSIZING") == "false" &&
            visual_object_property(allow_row_sizing_path, "other-guid", "ALLOWROWSIZING") == "true",
        "#1093: host object allow-row-sizing assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--allow-row-sizing-object",
            "--allow-row-sizing", "false",
            "--allow-row-sizing-target-unique-id", "one-guid",
            "--allow-row-sizing-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1093: missing-target host object allow-row-sizing assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALLOWROWSIZING") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALLOWROWSIZING") == "true",
        "#1093: missing-target host object allow-row-sizing assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--allow-row-sizing-object",
            "--allow-row-sizing", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1093: allow-row-sizing-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALLOWROWSIZING") == "true",
        "#1093: allow-row-sizing-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--allow-row-sizing-object",
            "--allow-row-sizing-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1093: allow-row-sizing-object without allow-row-sizing value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALLOWROWSIZING") == "true",
        "#1093: allow-row-sizing-object without allow-row-sizing value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--allow-row-sizing-object",
            "--allow-row-sizing", "false",
            "--allow-row-sizing-target-unique-id", "one-guid",
            "--allow-row-sizing-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1093: duplicate-target host object allow-row-sizing assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALLOWROWSIZING") == "true",
        "#1093: duplicate-target host object allow-row-sizing assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_allow_row_sizing(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--allow-row-sizing-object",
            "--auto-size-object",
            "--allow-row-sizing", "false",
            "--allow-row-sizing-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1093: allow-row-sizing-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALLOWROWSIZING") == "true",
        "#1093: allow-row-sizing-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
