// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_curvature(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CURVATURE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1111: synthetic SCX table for object curvature should be created");
}

void write_synthetic_form_table_for_object_draw_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DRAWMODE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1112: synthetic SCX table for object draw-mode should be created");
}

void write_synthetic_form_table_for_object_draw_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DRAWSTYLE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1113: synthetic SCX table for object draw-style should be created");
}

void write_synthetic_form_table_for_object_draw_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DRAWWIDTH", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1114: synthetic SCX table for object draw-width should be created");
}

void write_synthetic_form_table_for_object_fill_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FILLSTYLE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1115: synthetic SCX table for object fill-style should be created");
}

void write_synthetic_form_table_for_object_grid_line_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "GRIDLINECO", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1120: synthetic SCX table for object grid-line-color should be created");
}

void write_synthetic_form_table_for_object_grid_line_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "GRIDLINEWI", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1125: synthetic SCX table for object grid-line-width should be created");
}

void write_synthetic_form_table_for_object_grid_lines(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "GRIDLINES", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1126: synthetic SCX table for object grid-lines should be created");
}

void write_synthetic_form_table_for_object_fill_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FILLCOLOR", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1134: synthetic SCX table for object fill-color should be created");
}

void write_synthetic_form_table_for_object_back_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BACKSTYLE", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1161: synthetic SCX table for object back style should be created");
}

void write_synthetic_form_table_for_object_border_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BORDERSTYL", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1162: synthetic SCX table for object border style should be created");
}

void write_synthetic_form_table_for_object_border_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BORDERWIDT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "1"},
        {"frmOrder", "frmOrder", "two-guid", "1"},
        {"cntDetails", "cntDetails", "three-guid", "2"},
        {"frmOther", "frmOther", "other-guid", "2"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1163: synthetic SCX table for object border width should be created");
}

void write_synthetic_form_table_for_object_border_color(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BORDERCOLO", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "255"},
        {"frmOther", "frmOther", "other-guid", "255"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1164: synthetic SCX table for object border color should be created");
}

void write_synthetic_form_table_for_object_special_effect(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SPECIALEFF", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1166: synthetic SCX table for object special effect should be created");
}

void test_studio_host_json_assigns_curvature_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_curvature_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path curvature_path = temp_root / "curvature.scx";
    write_synthetic_form_table_for_object_curvature(curvature_path);
    const auto curvature_process = run_process_capture(
        studio_host_path,
        {
            "--path", curvature_path.string(),
            "--curvature-object",
            "--curvature", "4",
            "--curvature-target-object-name", "cmdSave",
            "--curvature-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(curvature_process.exit_code == 0,
        "#1111: host object curvature assignment should exit successfully");
    expect(visual_object_property(curvature_path, "one-guid", "CURVATURE") == "4" &&
            visual_object_property(curvature_path, "two-guid", "CURVATURE") == "4" &&
            visual_object_property(curvature_path, "three-guid", "CURVATURE") == "2" &&
            visual_object_property(curvature_path, "other-guid", "CURVATURE") == "0",
        "#1111: host object curvature assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_curvature(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--curvature-object",
            "--curvature", "2",
            "--curvature-target-unique-id", "one-guid",
            "--curvature-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1111: missing-target host object curvature assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CURVATURE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "CURVATURE") == "1",
        "#1111: missing-target host object curvature assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_curvature(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--curvature-object",
            "--curvature", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1111: curvature-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CURVATURE") == "0",
        "#1111: curvature-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_curvature(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--curvature-object",
            "--curvature-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1111: curvature-object without curvature value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CURVATURE") == "0",
        "#1111: curvature-object without curvature value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_curvature(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--curvature-object",
            "--curvature", "-1",
            "--curvature-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1111: negative curvature values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "CURVATURE") == "0",
        "#1111: negative curvature values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_curvature(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--curvature-object",
            "--curvature", "2",
            "--curvature-target-unique-id", "one-guid",
            "--curvature-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1111: duplicate-target host object curvature assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CURVATURE") == "0",
        "#1111: duplicate-target host object curvature assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_curvature(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--curvature-object",
            "--locked-object",
            "--curvature", "2",
            "--curvature-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1111: curvature-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CURVATURE") == "0",
        "#1111: curvature-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_draw_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_draw_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path draw_mode_path = temp_root / "draw_mode.scx";
    write_synthetic_form_table_for_object_draw_mode(draw_mode_path);
    const auto draw_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", draw_mode_path.string(),
            "--draw-mode-object",
            "--draw-mode", "5",
            "--draw-mode-target-object-name", "cmdSave",
            "--draw-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(draw_mode_process.exit_code == 0,
        "#1112: host object draw-mode assignment should exit successfully");
    expect(visual_object_property(draw_mode_path, "one-guid", "DRAWMODE") == "5" &&
            visual_object_property(draw_mode_path, "two-guid", "DRAWMODE") == "5" &&
            visual_object_property(draw_mode_path, "three-guid", "DRAWMODE") == "2" &&
            visual_object_property(draw_mode_path, "other-guid", "DRAWMODE") == "0",
        "#1112: host object draw-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_draw_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--draw-mode-object",
            "--draw-mode", "2",
            "--draw-mode-target-unique-id", "one-guid",
            "--draw-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1112: missing-target host object draw-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DRAWMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DRAWMODE") == "1",
        "#1112: missing-target host object draw-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_draw_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--draw-mode-object",
            "--draw-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1112: draw-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DRAWMODE") == "0",
        "#1112: draw-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_draw_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--draw-mode-object",
            "--draw-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1112: draw-mode-object without draw-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DRAWMODE") == "0",
        "#1112: draw-mode-object without draw-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_draw_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--draw-mode-object",
            "--draw-mode", "-1",
            "--draw-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1112: negative draw-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DRAWMODE") == "0",
        "#1112: negative draw-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_draw_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--draw-mode-object",
            "--draw-mode", "2",
            "--draw-mode-target-unique-id", "one-guid",
            "--draw-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1112: duplicate-target host object draw-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DRAWMODE") == "0",
        "#1112: duplicate-target host object draw-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_draw_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--draw-mode-object",
            "--locked-object",
            "--draw-mode", "2",
            "--draw-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1112: draw-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DRAWMODE") == "0",
        "#1112: draw-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_draw_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_draw_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path draw_style_path = temp_root / "draw_style.scx";
    write_synthetic_form_table_for_object_draw_style(draw_style_path);
    const auto draw_style_process = run_process_capture(
        studio_host_path,
        {
            "--path", draw_style_path.string(),
            "--draw-style-object",
            "--draw-style", "6",
            "--draw-style-target-object-name", "cmdSave",
            "--draw-style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(draw_style_process.exit_code == 0,
        "#1113: host object draw-style assignment should exit successfully");
    expect(visual_object_property(draw_style_path, "one-guid", "DRAWSTYLE") == "6" &&
            visual_object_property(draw_style_path, "two-guid", "DRAWSTYLE") == "6" &&
            visual_object_property(draw_style_path, "three-guid", "DRAWSTYLE") == "2" &&
            visual_object_property(draw_style_path, "other-guid", "DRAWSTYLE") == "0",
        "#1113: host object draw-style assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_draw_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--draw-style-object",
            "--draw-style", "2",
            "--draw-style-target-unique-id", "one-guid",
            "--draw-style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1113: missing-target host object draw-style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DRAWSTYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DRAWSTYLE") == "1",
        "#1113: missing-target host object draw-style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_draw_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--draw-style-object",
            "--draw-style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1113: draw-style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DRAWSTYLE") == "0",
        "#1113: draw-style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_draw_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--draw-style-object",
            "--draw-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1113: draw-style-object without draw-style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DRAWSTYLE") == "0",
        "#1113: draw-style-object without draw-style value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_draw_style(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--draw-style-object",
            "--draw-style", "-1",
            "--draw-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1113: negative draw-style values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DRAWSTYLE") == "0",
        "#1113: negative draw-style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_draw_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--draw-style-object",
            "--draw-style", "2",
            "--draw-style-target-unique-id", "one-guid",
            "--draw-style-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1113: duplicate-target host object draw-style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DRAWSTYLE") == "0",
        "#1113: duplicate-target host object draw-style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_draw_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--draw-style-object",
            "--locked-object",
            "--draw-style", "2",
            "--draw-style-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1113: draw-style-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DRAWSTYLE") == "0",
        "#1113: draw-style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_draw_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_draw_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path draw_width_path = temp_root / "draw_width.scx";
    write_synthetic_form_table_for_object_draw_width(draw_width_path);
    const auto draw_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", draw_width_path.string(),
            "--draw-width-object",
            "--draw-width", "7",
            "--draw-width-target-object-name", "cmdSave",
            "--draw-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(draw_width_process.exit_code == 0,
        "#1114: host object draw-width assignment should exit successfully");
    expect(visual_object_property(draw_width_path, "one-guid", "DRAWWIDTH") == "7" &&
            visual_object_property(draw_width_path, "two-guid", "DRAWWIDTH") == "7" &&
            visual_object_property(draw_width_path, "three-guid", "DRAWWIDTH") == "2" &&
            visual_object_property(draw_width_path, "other-guid", "DRAWWIDTH") == "0",
        "#1114: host object draw-width assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_draw_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--draw-width-object",
            "--draw-width", "2",
            "--draw-width-target-unique-id", "one-guid",
            "--draw-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1114: missing-target host object draw-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DRAWWIDTH") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DRAWWIDTH") == "1",
        "#1114: missing-target host object draw-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_draw_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--draw-width-object",
            "--draw-width", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1114: draw-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DRAWWIDTH") == "0",
        "#1114: draw-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_draw_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--draw-width-object",
            "--draw-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1114: draw-width-object without draw-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DRAWWIDTH") == "0",
        "#1114: draw-width-object without draw-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_draw_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--draw-width-object",
            "--draw-width", "-1",
            "--draw-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1114: negative draw-width values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DRAWWIDTH") == "0",
        "#1114: negative draw-width values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_draw_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--draw-width-object",
            "--draw-width", "2",
            "--draw-width-target-unique-id", "one-guid",
            "--draw-width-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1114: duplicate-target host object draw-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DRAWWIDTH") == "0",
        "#1114: duplicate-target host object draw-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_draw_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--draw-width-object",
            "--locked-object",
            "--draw-width", "2",
            "--draw-width-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1114: draw-width-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DRAWWIDTH") == "0",
        "#1114: draw-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_fill_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_fill_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fill_style_path = temp_root / "fill_style.scx";
    write_synthetic_form_table_for_object_fill_style(fill_style_path);
    const auto fill_style_process = run_process_capture(
        studio_host_path,
        {
            "--path", fill_style_path.string(),
            "--fill-style-object",
            "--fill-style", "8",
            "--fill-style-target-object-name", "cmdSave",
            "--fill-style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(fill_style_process.exit_code == 0,
        "#1115: host object fill-style assignment should exit successfully");
    expect(visual_object_property(fill_style_path, "one-guid", "FILLSTYLE") == "8" &&
            visual_object_property(fill_style_path, "two-guid", "FILLSTYLE") == "8" &&
            visual_object_property(fill_style_path, "three-guid", "FILLSTYLE") == "2" &&
            visual_object_property(fill_style_path, "other-guid", "FILLSTYLE") == "0",
        "#1115: host object fill-style assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_fill_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--fill-style-object",
            "--fill-style", "2",
            "--fill-style-target-unique-id", "one-guid",
            "--fill-style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1115: missing-target host object fill-style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FILLSTYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "FILLSTYLE") == "1",
        "#1115: missing-target host object fill-style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_fill_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--fill-style-object",
            "--fill-style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1115: fill-style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FILLSTYLE") == "0",
        "#1115: fill-style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_fill_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--fill-style-object",
            "--fill-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1115: fill-style-object without fill-style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FILLSTYLE") == "0",
        "#1115: fill-style-object without fill-style value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_fill_style(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--fill-style-object",
            "--fill-style", "-1",
            "--fill-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1115: negative fill-style values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "FILLSTYLE") == "0",
        "#1115: negative fill-style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_fill_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--fill-style-object",
            "--fill-style", "2",
            "--fill-style-target-unique-id", "one-guid",
            "--fill-style-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1115: duplicate-target host object fill-style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FILLSTYLE") == "0",
        "#1115: duplicate-target host object fill-style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_fill_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--fill-style-object",
            "--locked-object",
            "--fill-style", "2",
            "--fill-style-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1115: fill-style-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FILLSTYLE") == "0",
        "#1115: fill-style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_grid_line_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_grid_line_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path grid_line_color_path = temp_root / "grid_line_color.scx";
    write_synthetic_form_table_for_object_grid_line_color(grid_line_color_path);
    const auto grid_line_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", grid_line_color_path.string(),
            "--grid-line-color-object",
            "--grid-line-color", "9",
            "--grid-line-color-target-object-name", "cmdSave",
            "--grid-line-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(grid_line_color_process.exit_code == 0,
        "#1120: host object grid-line-color assignment should exit successfully");
    expect(visual_object_property(grid_line_color_path, "one-guid", "GRIDLINECOLOR") == "9" &&
            visual_object_property(grid_line_color_path, "two-guid", "GRIDLINECOLOR") == "9" &&
            visual_object_property(grid_line_color_path, "three-guid", "GRIDLINECOLOR") == "2" &&
            visual_object_property(grid_line_color_path, "other-guid", "GRIDLINECOLOR") == "0",
        "#1120: host object grid-line-color assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_grid_line_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--grid-line-color-object",
            "--grid-line-color", "2",
            "--grid-line-color-target-unique-id", "one-guid",
            "--grid-line-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1120: missing-target host object grid-line-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "GRIDLINECOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "GRIDLINECOLOR") == "1",
        "#1120: missing-target host object grid-line-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_grid_line_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--grid-line-color-object",
            "--grid-line-color", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1120: grid-line-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "GRIDLINECOLOR") == "0",
        "#1120: grid-line-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_grid_line_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--grid-line-color-object",
            "--grid-line-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1120: grid-line-color-object without grid-line-color value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "GRIDLINECOLOR") == "0",
        "#1120: grid-line-color-object without grid-line-color value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_grid_line_color(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--grid-line-color-object",
            "--grid-line-color", "-1",
            "--grid-line-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1120: negative grid-line-color values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "GRIDLINECOLOR") == "0",
        "#1120: negative grid-line-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_grid_line_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--grid-line-color-object",
            "--grid-line-color", "2",
            "--grid-line-color-target-unique-id", "one-guid",
            "--grid-line-color-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1120: duplicate-target host object grid-line-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "GRIDLINECOLOR") == "0",
        "#1120: duplicate-target host object grid-line-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_grid_line_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--grid-line-color-object",
            "--locked-object",
            "--grid-line-color", "2",
            "--grid-line-color-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1120: grid-line-color-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "GRIDLINECOLOR") == "0",
        "#1120: grid-line-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_grid_line_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_grid_line_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path grid_line_width_path = temp_root / "grid_line_width.scx";
    write_synthetic_form_table_for_object_grid_line_width(grid_line_width_path);
    const auto grid_line_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", grid_line_width_path.string(),
            "--grid-line-width-object",
            "--grid-line-width", "9",
            "--grid-line-width-target-object-name", "cmdSave",
            "--grid-line-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(grid_line_width_process.exit_code == 0,
        "#1125: host object grid-line-width assignment should exit successfully");
    expect(visual_object_property(grid_line_width_path, "one-guid", "GRIDLINEWIDTH") == "9" &&
            visual_object_property(grid_line_width_path, "two-guid", "GRIDLINEWIDTH") == "9" &&
            visual_object_property(grid_line_width_path, "three-guid", "GRIDLINEWIDTH") == "2" &&
            visual_object_property(grid_line_width_path, "other-guid", "GRIDLINEWIDTH") == "0",
        "#1125: host object grid-line-width assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_grid_line_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--grid-line-width-object",
            "--grid-line-width", "2",
            "--grid-line-width-target-unique-id", "one-guid",
            "--grid-line-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1125: missing-target host object grid-line-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "GRIDLINEWIDTH") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "GRIDLINEWIDTH") == "1",
        "#1125: missing-target host object grid-line-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_grid_line_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--grid-line-width-object",
            "--grid-line-width", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1125: grid-line-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "GRIDLINEWIDTH") == "0",
        "#1125: grid-line-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_grid_line_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--grid-line-width-object",
            "--grid-line-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1125: grid-line-width-object without grid-line-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "GRIDLINEWIDTH") == "0",
        "#1125: grid-line-width-object without grid-line-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_grid_line_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--grid-line-width-object",
            "--grid-line-width", "-1",
            "--grid-line-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1125: negative grid-line-width values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "GRIDLINEWIDTH") == "0",
        "#1125: negative grid-line-width values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_grid_line_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--grid-line-width-object",
            "--grid-line-width", "2",
            "--grid-line-width-target-unique-id", "one-guid",
            "--grid-line-width-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1125: duplicate-target host object grid-line-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "GRIDLINEWIDTH") == "0",
        "#1125: duplicate-target host object grid-line-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_grid_line_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--grid-line-width-object",
            "--locked-object",
            "--grid-line-width", "2",
            "--grid-line-width-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1125: grid-line-width-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "GRIDLINEWIDTH") == "0",
        "#1125: grid-line-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_grid_lines_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_grid_lines_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path grid_lines_path = temp_root / "grid_lines.scx";
    write_synthetic_form_table_for_object_grid_lines(grid_lines_path);
    const auto grid_lines_process = run_process_capture(
        studio_host_path,
        {
            "--path", grid_lines_path.string(),
            "--grid-lines-object",
            "--grid-lines", "9",
            "--grid-lines-target-object-name", "cmdSave",
            "--grid-lines-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(grid_lines_process.exit_code == 0,
        "#1126: host object grid-lines assignment should exit successfully");
    expect(visual_object_property(grid_lines_path, "one-guid", "GRIDLINES") == "9" &&
            visual_object_property(grid_lines_path, "two-guid", "GRIDLINES") == "9" &&
            visual_object_property(grid_lines_path, "three-guid", "GRIDLINES") == "2" &&
            visual_object_property(grid_lines_path, "other-guid", "GRIDLINES") == "0",
        "#1126: host object grid-lines assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_grid_lines(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--grid-lines-object",
            "--grid-lines", "2",
            "--grid-lines-target-unique-id", "one-guid",
            "--grid-lines-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1126: missing-target host object grid-lines assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "GRIDLINES") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "GRIDLINES") == "1",
        "#1126: missing-target host object grid-lines assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_grid_lines(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--grid-lines-object",
            "--grid-lines", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1126: grid-lines-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "GRIDLINES") == "0",
        "#1126: grid-lines-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_grid_lines(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--grid-lines-object",
            "--grid-lines-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1126: grid-lines-object without grid-lines value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "GRIDLINES") == "0",
        "#1126: grid-lines-object without grid-lines value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_grid_lines(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--grid-lines-object",
            "--grid-lines", "-1",
            "--grid-lines-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1126: negative grid-lines values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "GRIDLINES") == "0",
        "#1126: negative grid-lines values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_grid_lines(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--grid-lines-object",
            "--grid-lines", "2",
            "--grid-lines-target-unique-id", "one-guid",
            "--grid-lines-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1126: duplicate-target host object grid-lines assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "GRIDLINES") == "0",
        "#1126: duplicate-target host object grid-lines assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_grid_lines(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--grid-lines-object",
            "--locked-object",
            "--grid-lines", "2",
            "--grid-lines-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1126: grid-lines-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "GRIDLINES") == "0",
        "#1126: grid-lines-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_fill_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_fill_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fill_color_path = temp_root / "fill_color.scx";
    write_synthetic_form_table_for_object_fill_color(fill_color_path);
    const auto fill_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", fill_color_path.string(),
            "--fill-color-object",
            "--fill-color", "9",
            "--fill-color-target-object-name", "cmdSave",
            "--fill-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(fill_color_process.exit_code == 0,
        "#1134: host object fill-color assignment should exit successfully");
    expect(visual_object_property(fill_color_path, "one-guid", "FILLCOLOR") == "9" &&
            visual_object_property(fill_color_path, "two-guid", "FILLCOLOR") == "9" &&
            visual_object_property(fill_color_path, "three-guid", "FILLCOLOR") == "2" &&
            visual_object_property(fill_color_path, "other-guid", "FILLCOLOR") == "0",
        "#1134: host object fill-color assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_fill_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--fill-color-object",
            "--fill-color", "2",
            "--fill-color-target-unique-id", "one-guid",
            "--fill-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1134: missing-target host object fill-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FILLCOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "FILLCOLOR") == "1",
        "#1134: missing-target host object fill-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_fill_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--fill-color-object",
            "--fill-color", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1134: fill-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FILLCOLOR") == "0",
        "#1134: fill-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_fill_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--fill-color-object",
            "--fill-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1134: fill-color-object without fill-color value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FILLCOLOR") == "0",
        "#1134: fill-color-object without fill-color value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_fill_color(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--fill-color-object",
            "--fill-color", "-1",
            "--fill-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1134: negative fill-color values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "FILLCOLOR") == "0",
        "#1134: negative fill-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_fill_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--fill-color-object",
            "--fill-color", "2",
            "--fill-color-target-unique-id", "one-guid",
            "--fill-color-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1134: duplicate-target host object fill-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FILLCOLOR") == "0",
        "#1134: duplicate-target host object fill-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_fill_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--fill-color-object",
            "--locked-object",
            "--fill-color", "2",
            "--fill-color-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1134: fill-color-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FILLCOLOR") == "0",
        "#1134: fill-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_back_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_back_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path back_style_path = temp_root / "back_style.scx";
    write_synthetic_form_table_for_object_back_style(back_style_path);
    const auto back_style_process = run_process_capture(
        studio_host_path,
        {
            "--path", back_style_path.string(),
            "--back-style-object",
            "--back-style", "2",
            "--back-style-target-object-name", "frmCustomer",
            "--back-style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(back_style_process.exit_code == 0,
        "#1161: host object back-style assignment should exit successfully");
    expect(visual_object_property(back_style_path, "one-guid", "BACKSTYLE") == "2" &&
            visual_object_property(back_style_path, "two-guid", "BACKSTYLE") == "2" &&
            visual_object_property(back_style_path, "three-guid", "BACKSTYLE") == "1" &&
            visual_object_property(back_style_path, "other-guid", "BACKSTYLE") == "1",
        "#1161: host object back-style assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_back_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--back-style-object",
            "--back-style", "2",
            "--back-style-target-unique-id", "one-guid",
            "--back-style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1161: missing-target host object back-style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BACKSTYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BACKSTYLE") == "0",
        "#1161: missing-target host object back-style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_back_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--back-style-object",
            "--back-style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1161: back-style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BACKSTYLE") == "0",
        "#1161: back-style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_back_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--back-style-object",
            "--back-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1161: back-style-object without back-style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BACKSTYLE") == "0",
        "#1161: back-style-object without back-style value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_back_style(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--back-style-object",
            "--back-style", "-1",
            "--back-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1161: negative back-style values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BACKSTYLE") == "0",
        "#1161: negative back-style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_back_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--back-style-object",
            "--back-style", "2",
            "--back-style-target-unique-id", "one-guid",
            "--back-style-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1161: duplicate-target host object back-style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BACKSTYLE") == "0",
        "#1161: duplicate-target host object back-style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_back_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--back-style-object",
            "--allow-output-object",
            "--back-style", "2",
            "--back-style-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1161: back-style-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BACKSTYLE") == "0",
        "#1161: back-style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_border_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_border_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path border_style_path = temp_root / "border_style.scx";
    write_synthetic_form_table_for_object_border_style(border_style_path);
    const auto border_style_process = run_process_capture(
        studio_host_path,
        {
            "--path", border_style_path.string(),
            "--border-style-object",
            "--border-style", "2",
            "--border-style-target-object-name", "frmCustomer",
            "--border-style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(border_style_process.exit_code == 0,
        "#1162: host object border-style assignment should exit successfully");
    expect(visual_object_property(border_style_path, "one-guid", "BORDERSTYLE") == "2" &&
            visual_object_property(border_style_path, "two-guid", "BORDERSTYLE") == "2" &&
            visual_object_property(border_style_path, "three-guid", "BORDERSTYLE") == "1" &&
            visual_object_property(border_style_path, "other-guid", "BORDERSTYLE") == "1",
        "#1162: host object border-style assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_border_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--border-style-object",
            "--border-style", "2",
            "--border-style-target-unique-id", "one-guid",
            "--border-style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1162: missing-target host object border-style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BORDERSTYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BORDERSTYLE") == "0",
        "#1162: missing-target host object border-style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_border_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--border-style-object",
            "--border-style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1162: border-style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BORDERSTYLE") == "0",
        "#1162: border-style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_border_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--border-style-object",
            "--border-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1162: border-style-object without border-style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BORDERSTYLE") == "0",
        "#1162: border-style-object without border-style value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_border_style(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--border-style-object",
            "--border-style", "-1",
            "--border-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1162: negative border-style values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BORDERSTYLE") == "0",
        "#1162: negative border-style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_border_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--border-style-object",
            "--border-style", "2",
            "--border-style-target-unique-id", "one-guid",
            "--border-style-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1162: duplicate-target host object border-style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BORDERSTYLE") == "0",
        "#1162: duplicate-target host object border-style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_border_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--border-style-object",
            "--allow-output-object",
            "--border-style", "2",
            "--border-style-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1162: border-style-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BORDERSTYLE") == "0",
        "#1162: border-style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_border_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_border_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path border_width_path = temp_root / "border_width.scx";
    write_synthetic_form_table_for_object_border_width(border_width_path);
    const auto border_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", border_width_path.string(),
            "--border-width-object",
            "--border-width", "3",
            "--border-width-target-object-name", "frmCustomer",
            "--border-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(border_width_process.exit_code == 0,
        "#1163: host object border-width assignment should exit successfully");
    expect(visual_object_property(border_width_path, "one-guid", "BORDERWIDTH") == "3" &&
            visual_object_property(border_width_path, "two-guid", "BORDERWIDTH") == "3" &&
            visual_object_property(border_width_path, "three-guid", "BORDERWIDTH") == "2" &&
            visual_object_property(border_width_path, "other-guid", "BORDERWIDTH") == "2",
        "#1163: host object border-width assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_border_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--border-width-object",
            "--border-width", "3",
            "--border-width-target-unique-id", "one-guid",
            "--border-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1163: missing-target host object border-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BORDERWIDTH") == "1" &&
            visual_object_property(missing_target_path, "two-guid", "BORDERWIDTH") == "1",
        "#1163: missing-target host object border-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_border_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--border-width-object",
            "--border-width", "3",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1163: border-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BORDERWIDTH") == "1",
        "#1163: border-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_border_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--border-width-object",
            "--border-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1163: border-width-object without border-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BORDERWIDTH") == "1",
        "#1163: border-width-object without border-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_border_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--border-width-object",
            "--border-width", "-1",
            "--border-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1163: negative border-width values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BORDERWIDTH") == "1",
        "#1163: negative border-width values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_border_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--border-width-object",
            "--border-width", "3",
            "--border-width-target-unique-id", "one-guid",
            "--border-width-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1163: duplicate-target host object border-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BORDERWIDTH") == "1",
        "#1163: duplicate-target host object border-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_border_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--border-width-object",
            "--allow-output-object",
            "--border-width", "3",
            "--border-width-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1163: border-width-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BORDERWIDTH") == "1",
        "#1163: border-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_border_color_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_border_color_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path border_color_path = temp_root / "border_color.scx";
    write_synthetic_form_table_for_object_border_color(border_color_path);
    const auto border_color_process = run_process_capture(
        studio_host_path,
        {
            "--path", border_color_path.string(),
            "--border-color-object",
            "--border-color", "8421504",
            "--border-color-target-object-name", "frmCustomer",
            "--border-color-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(border_color_process.exit_code == 0,
        "#1164: host object border-color assignment should exit successfully");
    expect(visual_object_property(border_color_path, "one-guid", "BORDERCOLOR") == "8421504" &&
            visual_object_property(border_color_path, "two-guid", "BORDERCOLOR") == "8421504" &&
            visual_object_property(border_color_path, "three-guid", "BORDERCOLOR") == "255" &&
            visual_object_property(border_color_path, "other-guid", "BORDERCOLOR") == "255",
        "#1164: host object border-color assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_border_color(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--border-color-object",
            "--border-color", "8421504",
            "--border-color-target-unique-id", "one-guid",
            "--border-color-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1164: missing-target host object border-color assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BORDERCOLOR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BORDERCOLOR") == "0",
        "#1164: missing-target host object border-color assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_border_color(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--border-color-object",
            "--border-color", "8421504",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1164: border-color-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BORDERCOLOR") == "0",
        "#1164: border-color-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_border_color(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--border-color-object",
            "--border-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1164: border-color-object without border-color value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BORDERCOLOR") == "0",
        "#1164: border-color-object without border-color value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_border_color(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--border-color-object",
            "--border-color", "-1",
            "--border-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1164: negative border-color values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BORDERCOLOR") == "0",
        "#1164: negative border-color values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_border_color(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--border-color-object",
            "--border-color", "8421504",
            "--border-color-target-unique-id", "one-guid",
            "--border-color-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1164: duplicate-target host object border-color assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BORDERCOLOR") == "0",
        "#1164: duplicate-target host object border-color assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_border_color(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--border-color-object",
            "--allow-output-object",
            "--border-color", "8421504",
            "--border-color-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1164: border-color-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BORDERCOLOR") == "0",
        "#1164: border-color-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_special_effect_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_special_effect_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path special_effect_path = temp_root / "special_effect.scx";
    write_synthetic_form_table_for_object_special_effect(special_effect_path);
    const auto special_effect_process = run_process_capture(
        studio_host_path,
        {
            "--path", special_effect_path.string(),
            "--special-effect-object",
            "--special-effect", "2",
            "--special-effect-target-object-name", "frmCustomer",
            "--special-effect-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(special_effect_process.exit_code == 0,
        "#1166: host object special-effect assignment should exit successfully");
    expect(visual_object_property(special_effect_path, "one-guid", "SPECIALEFFECT") == "2" &&
            visual_object_property(special_effect_path, "two-guid", "SPECIALEFFECT") == "2" &&
            visual_object_property(special_effect_path, "three-guid", "SPECIALEFFECT") == "1" &&
            visual_object_property(special_effect_path, "other-guid", "SPECIALEFFECT") == "1",
        "#1166: host object special-effect assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_special_effect(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--special-effect-object",
            "--special-effect", "2",
            "--special-effect-target-unique-id", "one-guid",
            "--special-effect-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1166: missing-target host object special-effect assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SPECIALEFFECT") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SPECIALEFFECT") == "0",
        "#1166: missing-target host object special-effect assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_special_effect(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--special-effect-object",
            "--special-effect", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1166: special-effect-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SPECIALEFFECT") == "0",
        "#1166: special-effect-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_special_effect(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--special-effect-object",
            "--special-effect-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1166: special-effect-object without special-effect value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SPECIALEFFECT") == "0",
        "#1166: special-effect-object without special-effect value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_special_effect(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--special-effect-object",
            "--special-effect", "-1",
            "--special-effect-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1166: negative special-effect values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "SPECIALEFFECT") == "0",
        "#1166: negative special-effect values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_special_effect(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--special-effect-object",
            "--special-effect", "2",
            "--special-effect-target-unique-id", "one-guid",
            "--special-effect-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1166: duplicate-target host object special-effect assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SPECIALEFFECT") == "0",
        "#1166: duplicate-target host object special-effect assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_special_effect(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--special-effect-object",
            "--allow-output-object",
            "--special-effect", "2",
            "--special-effect-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1166: special-effect-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SPECIALEFFECT") == "0",
        "#1166: special-effect-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
