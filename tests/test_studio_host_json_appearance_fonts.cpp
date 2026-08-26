// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_dynamic_line_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICLIN", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtNotes", "txtNotes", "one-guid", "OLDHEIGHTONE"},
        {"txtMemo", "txtMemo", "two-guid", "OLDHEIGHTTWO"},
        {"cntDetails", "cntDetails", "three-guid", "THREEHEIGHT"},
        {"txtOther", "txtOther", "other-guid", "OTHERHEIGHT"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1177: synthetic SCX table for object dynamic line height should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_name(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "OLDNAMEONE"},
        {"txtMemo", "txtMemo", "two-guid", "OLDNAMETWO"},
        {"cntDetails", "cntDetails", "three-guid", "THREENAME"},
        {"txtOther", "txtOther", "other-guid", "OTHERNAME"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1188: synthetic SCX table for object dynamic font name should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_size(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "10"},
        {"txtMemo", "txtMemo", "two-guid", "9"},
        {"cntDetails", "cntDetails", "three-guid", "11"},
        {"txtOther", "txtOther", "other-guid", "12"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1189: synthetic SCX table for object dynamic font size should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_bold(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1190: synthetic SCX table for object dynamic font bold should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_italic(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1191: synthetic SCX table for object dynamic font italic should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_underline(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1192: synthetic SCX table for object dynamic font underline should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_strikethru(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1193: synthetic SCX table for object dynamic font strikethru should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_outline(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1194: synthetic SCX table for object dynamic font outline should be created");
}

void write_synthetic_form_table_for_object_dynamic_font_shadow(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1195: synthetic SCX table for object dynamic font shadow should be created");
}

void write_synthetic_form_table_for_object_font_name(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTNAME", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "Arial"},
        {"txtMemo", "txtMemo", "two-guid", "Tahoma"},
        {"cntDetails", "cntDetails", "three-guid", "Verdana"},
        {"txtOther", "txtOther", "other-guid", "Consolas"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1178: synthetic SCX table for object font name should be created");
}

void write_synthetic_form_table_for_object_font_size(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTSIZE", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "10"},
        {"txtMemo", "txtMemo", "two-guid", "9"},
        {"cntDetails", "cntDetails", "three-guid", "11"},
        {"txtOther", "txtOther", "other-guid", "12"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1179: synthetic SCX table for object font size should be created");
}

void write_synthetic_form_table_for_object_font_bold(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTBOLD", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1180: synthetic SCX table for object font bold should be created");
}

void write_synthetic_form_table_for_object_font_italic(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTITALIC", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1181: synthetic SCX table for object font italic should be created");
}

void write_synthetic_form_table_for_object_font_underline(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTUNDERL", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1182: synthetic SCX table for object font underline should be created");
}

void write_synthetic_form_table_for_object_font_strikethru(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTSTRIKE", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1183: synthetic SCX table for object font strikethru should be created");
}

void write_synthetic_form_table_for_object_font_outline(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTOUTLIN", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1184: synthetic SCX table for object font outline should be created");
}

void write_synthetic_form_table_for_object_font_shadow(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FONTSHADOW", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", ".F."},
        {"txtMemo", "txtMemo", "two-guid", ".F."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"txtOther", "txtOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1185: synthetic SCX table for object font shadow should be created");
}

void test_studio_host_json_assigns_dynamic_line_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_line_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., 18, 12)";
    const fs::path dynamic_line_height_path = temp_root / "dynamic_line_height.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(dynamic_line_height_path);
    const auto dynamic_line_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_line_height_path.string(),
            "--dynamic-line-height-object",
            "--dynamic-line-height", expression,
            "--dynamic-line-height-target-object-name", "txtNotes",
            "--dynamic-line-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_line_height_process.exit_code == 0,
        "#1177: host object dynamic-line-height assignment should exit successfully");
    expect(visual_object_property(dynamic_line_height_path, "one-guid", "DYNAMICLINEHEIGHT") == expression &&
            visual_object_property(dynamic_line_height_path, "two-guid", "DYNAMICLINEHEIGHT") == expression &&
            visual_object_property(dynamic_line_height_path, "three-guid", "DYNAMICLINEHEIGHT") == "THREEHEIGHT" &&
            visual_object_property(dynamic_line_height_path, "other-guid", "DYNAMICLINEHEIGHT") == "OTHERHEIGHT",
        "#1177: host object dynamic-line-height assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-line-height-object",
            "--dynamic-line-height", expression,
            "--dynamic-line-height-target-unique-id", "one-guid",
            "--dynamic-line-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1177: missing-target host object dynamic-line-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTONE" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTTWO",
        "#1177: missing-target host object dynamic-line-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-line-height-object",
            "--dynamic-line-height", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1177: dynamic-line-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTONE",
        "#1177: dynamic-line-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-line-height-object",
            "--dynamic-line-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1177: dynamic-line-height-object without dynamic-line-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTONE",
        "#1177: dynamic-line-height-object without dynamic-line-height value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-line-height-object",
            "--dynamic-line-height", expression,
            "--dynamic-line-height-target-unique-id", "one-guid",
            "--dynamic-line-height-target-object-name", "txtNotes",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1177: duplicate-target host object dynamic-line-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTONE",
        "#1177: duplicate-target host object dynamic-line-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_line_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-line-height-object",
            "--allow-output-object",
            "--dynamic-line-height", expression,
            "--dynamic-line-height-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1177: dynamic-line-height-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICLINEHEIGHT") == "OLDHEIGHTONE",
        "#1177: dynamic-line-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_font_name_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_name_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., 'Arial', 'Tahoma')";
    const fs::path dynamic_font_name_path = temp_root / "dynamic_font_name.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(dynamic_font_name_path);
    const auto dynamic_font_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_name_path.string(),
            "--dynamic-font-name-object",
            "--dynamic-font-name", expression,
            "--dynamic-font-name-target-object-name", "txtName",
            "--dynamic-font-name-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_name_process.exit_code == 0,
        "#1188: host object dynamic-font-name assignment should exit successfully");
    expect(visual_object_property(dynamic_font_name_path, "one-guid", "DYNAMICFONTNAME") == expression &&
            visual_object_property(dynamic_font_name_path, "two-guid", "DYNAMICFONTNAME") == expression &&
            visual_object_property(dynamic_font_name_path, "three-guid", "DYNAMICFONTNAME") == "THREENAME" &&
            visual_object_property(dynamic_font_name_path, "other-guid", "DYNAMICFONTNAME") == "OTHERNAME",
        "#1188: host object dynamic-font-name assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-name-object",
            "--dynamic-font-name", expression,
            "--dynamic-font-name-target-unique-id", "one-guid",
            "--dynamic-font-name-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1188: missing-target host object dynamic-font-name assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTNAME") == "OLDNAMEONE" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTNAME") == "OLDNAMETWO",
        "#1188: missing-target host object dynamic-font-name assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-name-object",
            "--dynamic-font-name", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1188: dynamic-font-name-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTNAME") == "OLDNAMEONE",
        "#1188: dynamic-font-name-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-name-object",
            "--dynamic-font-name-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1188: dynamic-font-name-object without dynamic-font-name value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTNAME") == "OLDNAMEONE",
        "#1188: dynamic-font-name-object without dynamic-font-name value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-name-object",
            "--dynamic-font-name", expression,
            "--dynamic-font-name-target-unique-id", "one-guid",
            "--dynamic-font-name-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1188: duplicate-target host object dynamic-font-name assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTNAME") == "OLDNAMEONE",
        "#1188: duplicate-target host object dynamic-font-name assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_name(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-name-object",
            "--allow-output-object",
            "--dynamic-font-name", expression,
            "--dynamic-font-name-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1188: dynamic-font-name-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTNAME") == "OLDNAMEONE",
        "#1188: dynamic-font-name-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_font_size_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_size_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., 14, 10)";
    const fs::path dynamic_font_size_path = temp_root / "dynamic_font_size.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(dynamic_font_size_path);
    const auto dynamic_font_size_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_size_path.string(),
            "--dynamic-font-size-object",
            "--dynamic-font-size", expression,
            "--dynamic-font-size-target-object-name", "txtName",
            "--dynamic-font-size-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_size_process.exit_code == 0,
        "#1189: host object dynamic-font-size assignment should exit successfully");
    expect(visual_object_property(dynamic_font_size_path, "one-guid", "DYNAMICFONTSIZE") == expression &&
            visual_object_property(dynamic_font_size_path, "two-guid", "DYNAMICFONTSIZE") == expression &&
            visual_object_property(dynamic_font_size_path, "three-guid", "DYNAMICFONTSIZE") == "11" &&
            visual_object_property(dynamic_font_size_path, "other-guid", "DYNAMICFONTSIZE") == "12",
        "#1189: host object dynamic-font-size assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-size-object",
            "--dynamic-font-size", expression,
            "--dynamic-font-size-target-unique-id", "one-guid",
            "--dynamic-font-size-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1189: missing-target host object dynamic-font-size assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTSIZE") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTSIZE") == "9",
        "#1189: missing-target host object dynamic-font-size assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-size-object",
            "--dynamic-font-size", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1189: dynamic-font-size-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTSIZE") == "10",
        "#1189: dynamic-font-size-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-size-object",
            "--dynamic-font-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1189: dynamic-font-size-object without dynamic-font-size value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTSIZE") == "10",
        "#1189: dynamic-font-size-object without dynamic-font-size value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-size-object",
            "--dynamic-font-size", expression,
            "--dynamic-font-size-target-unique-id", "one-guid",
            "--dynamic-font-size-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1189: duplicate-target host object dynamic-font-size assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTSIZE") == "10",
        "#1189: duplicate-target host object dynamic-font-size assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_size(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-size-object",
            "--allow-output-object",
            "--dynamic-font-size", expression,
            "--dynamic-font-size-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1189: dynamic-font-size-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTSIZE") == "10",
        "#1189: dynamic-font-size-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_appearance_fonts_dynamic_bold.inl"

#include "test_studio_host_json_appearance_fonts_dynamic_italic.inl"

void test_studio_host_json_assigns_dynamic_font_underline_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_underline_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_underline_path = temp_root / "dynamic_font_underline.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(dynamic_font_underline_path);
    const auto dynamic_font_underline_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_underline_path.string(),
            "--dynamic-font-underline-object",
            "--dynamic-font-underline", expression,
            "--dynamic-font-underline-target-object-name", "txtName",
            "--dynamic-font-underline-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_underline_process.exit_code == 0,
        "#1192: host object dynamic-font-underline assignment should exit successfully");
    expect(visual_object_property(dynamic_font_underline_path, "one-guid", "DYNAMICFONTUNDERLINE") == expression &&
            visual_object_property(dynamic_font_underline_path, "two-guid", "DYNAMICFONTUNDERLINE") == expression &&
            visual_object_property(dynamic_font_underline_path, "three-guid", "DYNAMICFONTUNDERLINE") == ".F." &&
            visual_object_property(dynamic_font_underline_path, "other-guid", "DYNAMICFONTUNDERLINE") == ".T.",
        "#1192: host object dynamic-font-underline assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-underline-object",
            "--dynamic-font-underline", expression,
            "--dynamic-font-underline-target-unique-id", "one-guid",
            "--dynamic-font-underline-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1192: missing-target host object dynamic-font-underline assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTUNDERLINE") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTUNDERLINE") == ".T.",
        "#1192: missing-target host object dynamic-font-underline assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-underline-object",
            "--dynamic-font-underline", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1192: dynamic-font-underline-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTUNDERLINE") == ".F.",
        "#1192: dynamic-font-underline-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-underline-object",
            "--dynamic-font-underline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1192: dynamic-font-underline-object without dynamic-font-underline value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTUNDERLINE") == ".F.",
        "#1192: dynamic-font-underline-object without dynamic-font-underline value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-underline-object",
            "--dynamic-font-underline", expression,
            "--dynamic-font-underline-target-unique-id", "one-guid",
            "--dynamic-font-underline-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1192: duplicate-target host object dynamic-font-underline assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTUNDERLINE") == ".F.",
        "#1192: duplicate-target host object dynamic-font-underline assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_underline(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-underline-object",
            "--allow-output-object",
            "--dynamic-font-underline", expression,
            "--dynamic-font-underline-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1192: dynamic-font-underline-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTUNDERLINE") == ".F.",
        "#1192: dynamic-font-underline-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_font_strikethru_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_strikethru_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_strikethru_path = temp_root / "dynamic_font_strikethru.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(dynamic_font_strikethru_path);
    const auto dynamic_font_strikethru_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_strikethru_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-object-name", "txtName",
            "--dynamic-font-strikethru-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_strikethru_process.exit_code == 0,
        "#1193: host object dynamic-font-strikethru assignment should exit successfully");
    expect(visual_object_property(dynamic_font_strikethru_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == expression &&
            visual_object_property(dynamic_font_strikethru_path, "two-guid", "DYNAMICFONTSTRIKETHRU") == expression &&
            visual_object_property(dynamic_font_strikethru_path, "three-guid", "DYNAMICFONTSTRIKETHRU") == ".F." &&
            visual_object_property(dynamic_font_strikethru_path, "other-guid", "DYNAMICFONTSTRIKETHRU") == ".T.",
        "#1193: host object dynamic-font-strikethru assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--dynamic-font-strikethru-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1193: missing-target host object dynamic-font-strikethru assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTSTRIKETHRU") == ".T.",
        "#1193: missing-target host object dynamic-font-strikethru assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object without dynamic-font-strikethru value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object without dynamic-font-strikethru value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-strikethru-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--dynamic-font-strikethru-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1193: duplicate-target host object dynamic-font-strikethru assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: duplicate-target host object dynamic-font-strikethru assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_strikethru(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-strikethru-object",
            "--allow-output-object",
            "--dynamic-font-strikethru", expression,
            "--dynamic-font-strikethru-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1193: dynamic-font-strikethru-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTSTRIKETHRU") == ".F.",
        "#1193: dynamic-font-strikethru-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_font_outline_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_outline_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_outline_path = temp_root / "dynamic_font_outline.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(dynamic_font_outline_path);
    const auto dynamic_font_outline_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_outline_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-object-name", "txtName",
            "--dynamic-font-outline-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_outline_process.exit_code == 0,
        "#1194: host object dynamic-font-outline assignment should exit successfully");
    expect(visual_object_property(dynamic_font_outline_path, "one-guid", "DYNAMICFONTOUTLINE") == expression &&
            visual_object_property(dynamic_font_outline_path, "two-guid", "DYNAMICFONTOUTLINE") == expression &&
            visual_object_property(dynamic_font_outline_path, "three-guid", "DYNAMICFONTOUTLINE") == ".F." &&
            visual_object_property(dynamic_font_outline_path, "other-guid", "DYNAMICFONTOUTLINE") == ".T.",
        "#1194: host object dynamic-font-outline assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--dynamic-font-outline-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1194: missing-target host object dynamic-font-outline assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTOUTLINE") == ".T.",
        "#1194: missing-target host object dynamic-font-outline assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1194: dynamic-font-outline-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1194: dynamic-font-outline-object without dynamic-font-outline value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object without dynamic-font-outline value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-outline-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--dynamic-font-outline-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1194: duplicate-target host object dynamic-font-outline assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: duplicate-target host object dynamic-font-outline assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_outline(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-outline-object",
            "--allow-output-object",
            "--dynamic-font-outline", expression,
            "--dynamic-font-outline-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1194: dynamic-font-outline-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTOUTLINE") == ".F.",
        "#1194: dynamic-font-outline-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_font_shadow_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_font_shadow_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., .T., .F.)";
    const fs::path dynamic_font_shadow_path = temp_root / "dynamic_font_shadow.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(dynamic_font_shadow_path);
    const auto dynamic_font_shadow_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_font_shadow_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-object-name", "txtName",
            "--dynamic-font-shadow-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_font_shadow_process.exit_code == 0,
        "#1195: host object dynamic-font-shadow assignment should exit successfully");
    expect(visual_object_property(dynamic_font_shadow_path, "one-guid", "DYNAMICFONTSHADOW") == expression &&
            visual_object_property(dynamic_font_shadow_path, "two-guid", "DYNAMICFONTSHADOW") == expression &&
            visual_object_property(dynamic_font_shadow_path, "three-guid", "DYNAMICFONTSHADOW") == ".F." &&
            visual_object_property(dynamic_font_shadow_path, "other-guid", "DYNAMICFONTSHADOW") == ".T.",
        "#1195: host object dynamic-font-shadow assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--dynamic-font-shadow-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1195: missing-target host object dynamic-font-shadow assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICFONTSHADOW") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICFONTSHADOW") == ".T.",
        "#1195: missing-target host object dynamic-font-shadow assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object without dynamic-font-shadow value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object without dynamic-font-shadow value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-font-shadow-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--dynamic-font-shadow-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1195: duplicate-target host object dynamic-font-shadow assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: duplicate-target host object dynamic-font-shadow assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_font_shadow(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-font-shadow-object",
            "--allow-output-object",
            "--dynamic-font-shadow", expression,
            "--dynamic-font-shadow-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1195: dynamic-font-shadow-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICFONTSHADOW") == ".F.",
        "#1195: dynamic-font-shadow-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_name_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_name_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string font_name = "Courier New";
    const fs::path font_name_path = temp_root / "font_name.scx";
    write_synthetic_form_table_for_object_font_name(font_name_path);
    const auto font_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_name_path.string(),
            "--font-name-object",
            "--font-name", font_name,
            "--font-name-target-object-name", "txtName",
            "--font-name-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_name_process.exit_code == 0,
        "#1178: host object font-name assignment should exit successfully");
    expect(visual_object_property(font_name_path, "one-guid", "FONTNAME") == font_name &&
            visual_object_property(font_name_path, "two-guid", "FONTNAME") == font_name &&
            visual_object_property(font_name_path, "three-guid", "FONTNAME") == "Verdana" &&
            visual_object_property(font_name_path, "other-guid", "FONTNAME") == "Consolas",
        "#1178: host object font-name assignment should assign selected font text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_name(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-name-object",
            "--font-name", font_name,
            "--font-name-target-unique-id", "one-guid",
            "--font-name-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1178: missing-target host object font-name assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTNAME") == "Arial" &&
            visual_object_property(missing_target_path, "two-guid", "FONTNAME") == "Tahoma",
        "#1178: missing-target host object font-name assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_name(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-name-object",
            "--font-name", font_name,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1178: font-name-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTNAME") == "Arial",
        "#1178: font-name-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_name(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-name-object",
            "--font-name-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1178: font-name-object without font-name value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTNAME") == "Arial",
        "#1178: font-name-object without font-name value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_name(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-name-object",
            "--font-name", font_name,
            "--font-name-target-unique-id", "one-guid",
            "--font-name-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1178: duplicate-target host object font-name assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTNAME") == "Arial",
        "#1178: duplicate-target host object font-name assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_name(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-name-object",
            "--allow-output-object",
            "--font-name", font_name,
            "--font-name-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1178: font-name-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTNAME") == "Arial",
        "#1178: font-name-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_size_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_size_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_size_path = temp_root / "font_size.scx";
    write_synthetic_form_table_for_object_font_size(font_size_path);
    const auto font_size_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_size_path.string(),
            "--font-size-object",
            "--font-size", "13.5",
            "--font-size-target-object-name", "txtName",
            "--font-size-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_size_process.exit_code == 0,
        "#1179: host object font-size assignment should exit successfully");
    expect(visual_object_property(font_size_path, "one-guid", "FONTSIZE") == "13.5" &&
            visual_object_property(font_size_path, "two-guid", "FONTSIZE") == "13.5" &&
            visual_object_property(font_size_path, "three-guid", "FONTSIZE") == "11" &&
            visual_object_property(font_size_path, "other-guid", "FONTSIZE") == "12",
        "#1179: host object font-size assignment should assign selected numeric text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_size(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-size-object",
            "--font-size", "13.5",
            "--font-size-target-unique-id", "one-guid",
            "--font-size-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1179: missing-target host object font-size assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTSIZE") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "FONTSIZE") == "9",
        "#1179: missing-target host object font-size assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_size(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-size-object",
            "--font-size", "13.5",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1179: font-size-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTSIZE") == "10",
        "#1179: font-size-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_size(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-size-object",
            "--font-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1179: font-size-object without font-size value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTSIZE") == "10",
        "#1179: font-size-object without font-size value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_font_size(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--font-size-object",
            "--font-size", "-1",
            "--font-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1179: negative font-size values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "FONTSIZE") == "10",
        "#1179: negative font-size values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_size(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-size-object",
            "--font-size", "13.5",
            "--font-size-target-unique-id", "one-guid",
            "--font-size-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1179: duplicate-target host object font-size assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTSIZE") == "10",
        "#1179: duplicate-target host object font-size assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_size(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-size-object",
            "--allow-output-object",
            "--font-size", "13.5",
            "--font-size-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1179: font-size-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTSIZE") == "10",
        "#1179: font-size-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_bold_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_bold_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_bold_path = temp_root / "font_bold.scx";
    write_synthetic_form_table_for_object_font_bold(font_bold_path);
    const auto font_bold_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_bold_path.string(),
            "--font-bold-object",
            "--font-bold", "true",
            "--font-bold-target-object-name", "txtName",
            "--font-bold-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_bold_process.exit_code == 0,
        "#1180: host object font-bold assignment should exit successfully");
    expect(visual_object_property(font_bold_path, "one-guid", "FONTBOLD") == ".T." &&
            visual_object_property(font_bold_path, "two-guid", "FONTBOLD") == ".T." &&
            visual_object_property(font_bold_path, "three-guid", "FONTBOLD") == ".F." &&
            visual_object_property(font_bold_path, "other-guid", "FONTBOLD") == ".T.",
        "#1180: host object font-bold assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_bold(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-bold-object",
            "--font-bold", "true",
            "--font-bold-target-unique-id", "one-guid",
            "--font-bold-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1180: missing-target host object font-bold assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTBOLD") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTBOLD") == ".F.",
        "#1180: missing-target host object font-bold assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_bold(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-bold-object",
            "--font-bold", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1180: font-bold-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTBOLD") == ".F.",
        "#1180: font-bold-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_bold(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-bold-object",
            "--font-bold-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1180: font-bold-object without font-bold value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTBOLD") == ".F.",
        "#1180: font-bold-object without font-bold value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_bold(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-bold-object",
            "--font-bold", "sometimes",
            "--font-bold-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1180: invalid font-bold values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTBOLD") == ".F.",
        "#1180: invalid font-bold values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_bold(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-bold-object",
            "--font-bold", "true",
            "--font-bold-target-unique-id", "one-guid",
            "--font-bold-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1180: duplicate-target host object font-bold assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTBOLD") == ".F.",
        "#1180: duplicate-target host object font-bold assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_bold(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-bold-object",
            "--allow-output-object",
            "--font-bold", "true",
            "--font-bold-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1180: font-bold-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTBOLD") == ".F.",
        "#1180: font-bold-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_italic_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_italic_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_italic_path = temp_root / "font_italic.scx";
    write_synthetic_form_table_for_object_font_italic(font_italic_path);
    const auto font_italic_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_italic_path.string(),
            "--font-italic-object",
            "--font-italic", "true",
            "--font-italic-target-object-name", "txtName",
            "--font-italic-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_italic_process.exit_code == 0,
        "#1181: host object font-italic assignment should exit successfully");
    expect(visual_object_property(font_italic_path, "one-guid", "FONTITALIC") == ".T." &&
            visual_object_property(font_italic_path, "two-guid", "FONTITALIC") == ".T." &&
            visual_object_property(font_italic_path, "three-guid", "FONTITALIC") == ".F." &&
            visual_object_property(font_italic_path, "other-guid", "FONTITALIC") == ".T.",
        "#1181: host object font-italic assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_italic(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-italic-object",
            "--font-italic", "true",
            "--font-italic-target-unique-id", "one-guid",
            "--font-italic-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1181: missing-target host object font-italic assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTITALIC") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTITALIC") == ".F.",
        "#1181: missing-target host object font-italic assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_italic(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-italic-object",
            "--font-italic", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1181: font-italic-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTITALIC") == ".F.",
        "#1181: font-italic-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_italic(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-italic-object",
            "--font-italic-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1181: font-italic-object without font-italic value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTITALIC") == ".F.",
        "#1181: font-italic-object without font-italic value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_italic(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-italic-object",
            "--font-italic", "sometimes",
            "--font-italic-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1181: invalid font-italic values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTITALIC") == ".F.",
        "#1181: invalid font-italic values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_italic(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-italic-object",
            "--font-italic", "true",
            "--font-italic-target-unique-id", "one-guid",
            "--font-italic-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1181: duplicate-target host object font-italic assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTITALIC") == ".F.",
        "#1181: duplicate-target host object font-italic assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_italic(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-italic-object",
            "--allow-output-object",
            "--font-italic", "true",
            "--font-italic-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1181: font-italic-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTITALIC") == ".F.",
        "#1181: font-italic-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_underline_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_underline_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_underline_path = temp_root / "font_underline.scx";
    write_synthetic_form_table_for_object_font_underline(font_underline_path);
    const auto font_underline_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_underline_path.string(),
            "--font-underline-object",
            "--font-underline", "true",
            "--font-underline-target-object-name", "txtName",
            "--font-underline-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_underline_process.exit_code == 0,
        "#1182: host object font-underline assignment should exit successfully");
    expect(visual_object_property(font_underline_path, "one-guid", "FONTUNDERLINE") == ".T." &&
            visual_object_property(font_underline_path, "two-guid", "FONTUNDERLINE") == ".T." &&
            visual_object_property(font_underline_path, "three-guid", "FONTUNDERLINE") == ".F." &&
            visual_object_property(font_underline_path, "other-guid", "FONTUNDERLINE") == ".T.",
        "#1182: host object font-underline assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_underline(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-underline-object",
            "--font-underline", "true",
            "--font-underline-target-unique-id", "one-guid",
            "--font-underline-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1182: missing-target host object font-underline assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTUNDERLINE") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTUNDERLINE") == ".F.",
        "#1182: missing-target host object font-underline assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_underline(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-underline-object",
            "--font-underline", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1182: font-underline-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTUNDERLINE") == ".F.",
        "#1182: font-underline-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_underline(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-underline-object",
            "--font-underline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1182: font-underline-object without font-underline value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTUNDERLINE") == ".F.",
        "#1182: font-underline-object without font-underline value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_underline(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-underline-object",
            "--font-underline", "sometimes",
            "--font-underline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1182: invalid font-underline values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTUNDERLINE") == ".F.",
        "#1182: invalid font-underline values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_underline(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-underline-object",
            "--font-underline", "true",
            "--font-underline-target-unique-id", "one-guid",
            "--font-underline-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1182: duplicate-target host object font-underline assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTUNDERLINE") == ".F.",
        "#1182: duplicate-target host object font-underline assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_underline(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-underline-object",
            "--allow-output-object",
            "--font-underline", "true",
            "--font-underline-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1182: font-underline-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTUNDERLINE") == ".F.",
        "#1182: font-underline-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_strikethru_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_strikethru_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_strikethru_path = temp_root / "font_strikethru.scx";
    write_synthetic_form_table_for_object_font_strikethru(font_strikethru_path);
    const auto font_strikethru_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_strikethru_path.string(),
            "--font-strikethru-object",
            "--font-strikethru", "true",
            "--font-strikethru-target-object-name", "txtName",
            "--font-strikethru-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_strikethru_process.exit_code == 0,
        "#1183: host object font-strikethru assignment should exit successfully");
    expect(visual_object_property(font_strikethru_path, "one-guid", "FONTSTRIKETHRU") == ".T." &&
            visual_object_property(font_strikethru_path, "two-guid", "FONTSTRIKETHRU") == ".T." &&
            visual_object_property(font_strikethru_path, "three-guid", "FONTSTRIKETHRU") == ".F." &&
            visual_object_property(font_strikethru_path, "other-guid", "FONTSTRIKETHRU") == ".T.",
        "#1183: host object font-strikethru assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_strikethru(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-strikethru-object",
            "--font-strikethru", "true",
            "--font-strikethru-target-unique-id", "one-guid",
            "--font-strikethru-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1183: missing-target host object font-strikethru assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTSTRIKETHRU") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: missing-target host object font-strikethru assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_strikethru(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-strikethru-object",
            "--font-strikethru", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1183: font-strikethru-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: font-strikethru-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_strikethru(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-strikethru-object",
            "--font-strikethru-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1183: font-strikethru-object without font-strikethru value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: font-strikethru-object without font-strikethru value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_strikethru(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-strikethru-object",
            "--font-strikethru", "sometimes",
            "--font-strikethru-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1183: invalid font-strikethru values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: invalid font-strikethru values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_strikethru(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-strikethru-object",
            "--font-strikethru", "true",
            "--font-strikethru-target-unique-id", "one-guid",
            "--font-strikethru-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1183: duplicate-target host object font-strikethru assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: duplicate-target host object font-strikethru assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_strikethru(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-strikethru-object",
            "--allow-output-object",
            "--font-strikethru", "true",
            "--font-strikethru-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1183: font-strikethru-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTSTRIKETHRU") == ".F.",
        "#1183: font-strikethru-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_outline_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_outline_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_outline_path = temp_root / "font_outline.scx";
    write_synthetic_form_table_for_object_font_outline(font_outline_path);
    const auto font_outline_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_outline_path.string(),
            "--font-outline-object",
            "--font-outline", "true",
            "--font-outline-target-object-name", "txtName",
            "--font-outline-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_outline_process.exit_code == 0,
        "#1184: host object font-outline assignment should exit successfully");
    expect(visual_object_property(font_outline_path, "one-guid", "FONTOUTLINE") == ".T." &&
            visual_object_property(font_outline_path, "two-guid", "FONTOUTLINE") == ".T." &&
            visual_object_property(font_outline_path, "three-guid", "FONTOUTLINE") == ".F." &&
            visual_object_property(font_outline_path, "other-guid", "FONTOUTLINE") == ".T.",
        "#1184: host object font-outline assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_outline(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-outline-object",
            "--font-outline", "true",
            "--font-outline-target-unique-id", "one-guid",
            "--font-outline-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1184: missing-target host object font-outline assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTOUTLINE") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTOUTLINE") == ".F.",
        "#1184: missing-target host object font-outline assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_outline(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-outline-object",
            "--font-outline", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1184: font-outline-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTOUTLINE") == ".F.",
        "#1184: font-outline-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_outline(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-outline-object",
            "--font-outline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1184: font-outline-object without font-outline value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTOUTLINE") == ".F.",
        "#1184: font-outline-object without font-outline value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_outline(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-outline-object",
            "--font-outline", "sometimes",
            "--font-outline-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1184: invalid font-outline values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTOUTLINE") == ".F.",
        "#1184: invalid font-outline values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_outline(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-outline-object",
            "--font-outline", "true",
            "--font-outline-target-unique-id", "one-guid",
            "--font-outline-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1184: duplicate-target host object font-outline assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTOUTLINE") == ".F.",
        "#1184: duplicate-target host object font-outline assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_outline(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-outline-object",
            "--allow-output-object",
            "--font-outline", "true",
            "--font-outline-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1184: font-outline-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTOUTLINE") == ".F.",
        "#1184: font-outline-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_font_shadow_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_font_shadow_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path font_shadow_path = temp_root / "font_shadow.scx";
    write_synthetic_form_table_for_object_font_shadow(font_shadow_path);
    const auto font_shadow_process = run_process_capture(
        studio_host_path,
        {
            "--path", font_shadow_path.string(),
            "--font-shadow-object",
            "--font-shadow", "true",
            "--font-shadow-target-object-name", "txtName",
            "--font-shadow-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(font_shadow_process.exit_code == 0,
        "#1185: host object font-shadow assignment should exit successfully");
    expect(visual_object_property(font_shadow_path, "one-guid", "FONTSHADOW") == ".T." &&
            visual_object_property(font_shadow_path, "two-guid", "FONTSHADOW") == ".T." &&
            visual_object_property(font_shadow_path, "three-guid", "FONTSHADOW") == ".F." &&
            visual_object_property(font_shadow_path, "other-guid", "FONTSHADOW") == ".T.",
        "#1185: host object font-shadow assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_font_shadow(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--font-shadow-object",
            "--font-shadow", "true",
            "--font-shadow-target-unique-id", "one-guid",
            "--font-shadow-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1185: missing-target host object font-shadow assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FONTSHADOW") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "FONTSHADOW") == ".F.",
        "#1185: missing-target host object font-shadow assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_font_shadow(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--font-shadow-object",
            "--font-shadow", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1185: font-shadow-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FONTSHADOW") == ".F.",
        "#1185: font-shadow-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_font_shadow(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--font-shadow-object",
            "--font-shadow-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1185: font-shadow-object without font-shadow value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FONTSHADOW") == ".F.",
        "#1185: font-shadow-object without font-shadow value should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_object_font_shadow(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--font-shadow-object",
            "--font-shadow", "sometimes",
            "--font-shadow-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1185: invalid font-shadow values should fail during launch parsing");
    expect(visual_object_property(invalid_value_path, "one-guid", "FONTSHADOW") == ".F.",
        "#1185: invalid font-shadow values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_font_shadow(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--font-shadow-object",
            "--font-shadow", "true",
            "--font-shadow-target-unique-id", "one-guid",
            "--font-shadow-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1185: duplicate-target host object font-shadow assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FONTSHADOW") == ".F.",
        "#1185: duplicate-target host object font-shadow assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_font_shadow(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--font-shadow-object",
            "--allow-output-object",
            "--font-shadow", "true",
            "--font-shadow-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1185: font-shadow-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FONTSHADOW") == ".F.",
        "#1185: font-shadow-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
