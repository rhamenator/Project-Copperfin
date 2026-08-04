// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_scale_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SCALEMODE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1116: synthetic SCX table for object scale-mode should be created");
}

void write_synthetic_form_table_for_object_header_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HEADERHEIG", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1121: synthetic SCX table for object header-height should be created");
}

void write_synthetic_form_table_for_object_row_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ROWHEIGHT", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1122: synthetic SCX table for object row-height should be created");
}

void write_synthetic_form_table_for_object_highlight_row_line_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIGHLIGHTR", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1127: synthetic SCX table for object highlight-row-line-width should be created");
}

void write_synthetic_form_table_for_object_highlight_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HIGHLIGHTS", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1132: synthetic SCX table for object highlight-style should be created");
}

void write_synthetic_form_table_for_object_input_mask(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "INPUTMASK", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtPhone", "txtPhone", "one-guid", "(999) 999-9999"},
        {"txtZip", "txtZip", "two-guid", "99999"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "XXXXXXXX"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1046: synthetic SCX table for object input mask should be created");
}

void write_synthetic_form_table_for_object_format(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "FORMAT", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtAmount", "txtAmount", "one-guid", "999,999.99"},
        {"txtPercent", "txtPercent", "two-guid", "99.99%"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "!"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1047: synthetic SCX table for object format should be created");
}

void write_synthetic_form_table_for_object_scroll_bars(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SCROLLBARS", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1167: synthetic SCX table for object scroll bars should be created");
}

void write_synthetic_form_table_for_object_window_state(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "WINDOWSTAT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1168: synthetic SCX table for object window state should be created");
}

void write_synthetic_form_table_for_object_show_window(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SHOWWINDOW", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1169: synthetic SCX table for object show window should be created");
}

void write_synthetic_form_table_for_object_title_bar(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TITLEBAR", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1170: synthetic SCX table for object title bar should be created");
}

void write_synthetic_form_table_for_object_mouse_pointer(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MOUSEPOINT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "0"},
        {"frmOrder", "frmOrder", "two-guid", "0"},
        {"cntDetails", "cntDetails", "three-guid", "1"},
        {"frmOther", "frmOther", "other-guid", "1"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1171: synthetic SCX table for object mouse pointer should be created");
}

void write_synthetic_form_table_for_object_dynamic_input_mask(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICINP", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtSsn", "txtSsn", "one-guid", "OLDMASKONE"},
        {"txtPhone", "txtPhone", "two-guid", "OLDMASKTWO"},
        {"cntDetails", "cntDetails", "three-guid", "THREEMASK"},
        {"txtOther", "txtOther", "other-guid", "OTHERMASK"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1176: synthetic SCX table for object dynamic input mask should be created");
}

void write_synthetic_form_table_for_object_dynamic_current_control(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICCUR", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"grdOrders", "grdOrders", "one-guid", "OLDCURRENTONE"},
        {"grdDetails", "grdDetails", "two-guid", "OLDCURRENTTWO"},
        {"cntDetails", "cntDetails", "three-guid", "THREECURRENT"},
        {"txtOther", "txtOther", "other-guid", "OTHERCURRENT"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1187: synthetic SCX table for object dynamic current control should be created");
}

void test_studio_host_json_assigns_scale_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_scale_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path scale_mode_path = temp_root / "scale_mode.scx";
    write_synthetic_form_table_for_object_scale_mode(scale_mode_path);
    const auto scale_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", scale_mode_path.string(),
            "--scale-mode-object",
            "--scale-mode", "9",
            "--scale-mode-target-object-name", "cmdSave",
            "--scale-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(scale_mode_process.exit_code == 0,
        "#1116: host object scale-mode assignment should exit successfully");
    expect(visual_object_property(scale_mode_path, "one-guid", "SCALEMODE") == "9" &&
            visual_object_property(scale_mode_path, "two-guid", "SCALEMODE") == "9" &&
            visual_object_property(scale_mode_path, "three-guid", "SCALEMODE") == "2" &&
            visual_object_property(scale_mode_path, "other-guid", "SCALEMODE") == "0",
        "#1116: host object scale-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_scale_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--scale-mode-object",
            "--scale-mode", "2",
            "--scale-mode-target-unique-id", "one-guid",
            "--scale-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1116: missing-target host object scale-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SCALEMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SCALEMODE") == "1",
        "#1116: missing-target host object scale-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_scale_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--scale-mode-object",
            "--scale-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1116: scale-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SCALEMODE") == "0",
        "#1116: scale-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_scale_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--scale-mode-object",
            "--scale-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1116: scale-mode-object without scale-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SCALEMODE") == "0",
        "#1116: scale-mode-object without scale-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_scale_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--scale-mode-object",
            "--scale-mode", "-1",
            "--scale-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1116: negative scale-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "SCALEMODE") == "0",
        "#1116: negative scale-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_scale_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--scale-mode-object",
            "--scale-mode", "2",
            "--scale-mode-target-unique-id", "one-guid",
            "--scale-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1116: duplicate-target host object scale-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SCALEMODE") == "0",
        "#1116: duplicate-target host object scale-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_scale_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--scale-mode-object",
            "--locked-object",
            "--scale-mode", "2",
            "--scale-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1116: scale-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SCALEMODE") == "0",
        "#1116: scale-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_header_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_header_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path header_height_path = temp_root / "header_height.scx";
    write_synthetic_form_table_for_object_header_height(header_height_path);
    const auto header_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", header_height_path.string(),
            "--header-height-object",
            "--header-height", "9",
            "--header-height-target-object-name", "cmdSave",
            "--header-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(header_height_process.exit_code == 0,
        "#1121: host object header-height assignment should exit successfully");
    expect(visual_object_property(header_height_path, "one-guid", "HEADERHEIGHT") == "9" &&
            visual_object_property(header_height_path, "two-guid", "HEADERHEIGHT") == "9" &&
            visual_object_property(header_height_path, "three-guid", "HEADERHEIGHT") == "2" &&
            visual_object_property(header_height_path, "other-guid", "HEADERHEIGHT") == "0",
        "#1121: host object header-height assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_header_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--header-height-object",
            "--header-height", "2",
            "--header-height-target-unique-id", "one-guid",
            "--header-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1121: missing-target host object header-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HEADERHEIGHT") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HEADERHEIGHT") == "1",
        "#1121: missing-target host object header-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_header_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--header-height-object",
            "--header-height", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1121: header-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HEADERHEIGHT") == "0",
        "#1121: header-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_header_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--header-height-object",
            "--header-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1121: header-height-object without header-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HEADERHEIGHT") == "0",
        "#1121: header-height-object without header-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_header_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--header-height-object",
            "--header-height", "-1",
            "--header-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1121: negative header-height values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "HEADERHEIGHT") == "0",
        "#1121: negative header-height values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_header_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--header-height-object",
            "--header-height", "2",
            "--header-height-target-unique-id", "one-guid",
            "--header-height-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1121: duplicate-target host object header-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HEADERHEIGHT") == "0",
        "#1121: duplicate-target host object header-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_header_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--header-height-object",
            "--locked-object",
            "--header-height", "2",
            "--header-height-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1121: header-height-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HEADERHEIGHT") == "0",
        "#1121: header-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_row_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_row_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path row_height_path = temp_root / "row_height.scx";
    write_synthetic_form_table_for_object_row_height(row_height_path);
    const auto row_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", row_height_path.string(),
            "--row-height-object",
            "--row-height", "9",
            "--row-height-target-object-name", "cmdSave",
            "--row-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(row_height_process.exit_code == 0,
        "#1122: host object row-height assignment should exit successfully");
    expect(visual_object_property(row_height_path, "one-guid", "ROWHEIGHT") == "9" &&
            visual_object_property(row_height_path, "two-guid", "ROWHEIGHT") == "9" &&
            visual_object_property(row_height_path, "three-guid", "ROWHEIGHT") == "2" &&
            visual_object_property(row_height_path, "other-guid", "ROWHEIGHT") == "0",
        "#1122: host object row-height assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_row_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--row-height-object",
            "--row-height", "2",
            "--row-height-target-unique-id", "one-guid",
            "--row-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1122: missing-target host object row-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ROWHEIGHT") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "ROWHEIGHT") == "1",
        "#1122: missing-target host object row-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_row_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--row-height-object",
            "--row-height", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1122: row-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ROWHEIGHT") == "0",
        "#1122: row-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_row_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--row-height-object",
            "--row-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1122: row-height-object without row-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ROWHEIGHT") == "0",
        "#1122: row-height-object without row-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_row_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--row-height-object",
            "--row-height", "-1",
            "--row-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1122: negative row-height values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "ROWHEIGHT") == "0",
        "#1122: negative row-height values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_row_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--row-height-object",
            "--row-height", "2",
            "--row-height-target-unique-id", "one-guid",
            "--row-height-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1122: duplicate-target host object row-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ROWHEIGHT") == "0",
        "#1122: duplicate-target host object row-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_row_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--row-height-object",
            "--locked-object",
            "--row-height", "2",
            "--row-height-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1122: row-height-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ROWHEIGHT") == "0",
        "#1122: row-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_row_line_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_highlight_row_line_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_row_line_width_path = temp_root / "highlight_row_line_width.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(highlight_row_line_width_path);
    const auto highlight_row_line_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_row_line_width_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width", "9",
            "--highlight-row-line-width-target-object-name", "cmdSave",
            "--highlight-row-line-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_row_line_width_process.exit_code == 0,
        "#1127: host object highlight-row-line-width assignment should exit successfully");
    expect(visual_object_property(highlight_row_line_width_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "9" &&
            visual_object_property(highlight_row_line_width_path, "two-guid", "HIGHLIGHTROWLINEWIDTH") == "9" &&
            visual_object_property(highlight_row_line_width_path, "three-guid", "HIGHLIGHTROWLINEWIDTH") == "2" &&
            visual_object_property(highlight_row_line_width_path, "other-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: host object highlight-row-line-width assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width", "2",
            "--highlight-row-line-width-target-unique-id", "one-guid",
            "--highlight-row-line-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1127: missing-target host object highlight-row-line-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTROWLINEWIDTH") == "1",
        "#1127: missing-target host object highlight-row-line-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1127: highlight-row-line-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: highlight-row-line-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1127: highlight-row-line-width-object without highlight-row-line-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: highlight-row-line-width-object without highlight-row-line-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width", "-1",
            "--highlight-row-line-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1127: negative highlight-row-line-width values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: negative highlight-row-line-width values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-row-line-width-object",
            "--highlight-row-line-width", "2",
            "--highlight-row-line-width-target-unique-id", "one-guid",
            "--highlight-row-line-width-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1127: duplicate-target host object highlight-row-line-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: duplicate-target host object highlight-row-line-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_row_line_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-row-line-width-object",
            "--locked-object",
            "--highlight-row-line-width", "2",
            "--highlight-row-line-width-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1127: highlight-row-line-width-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTROWLINEWIDTH") == "0",
        "#1127: highlight-row-line-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_highlight_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_highlight_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path highlight_style_path = temp_root / "highlight_style.scx";
    write_synthetic_form_table_for_object_highlight_style(highlight_style_path);
    const auto highlight_style_process = run_process_capture(
        studio_host_path,
        {
            "--path", highlight_style_path.string(),
            "--highlight-style-object",
            "--highlight-style", "9",
            "--highlight-style-target-object-name", "cmdSave",
            "--highlight-style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(highlight_style_process.exit_code == 0,
        "#1132: host object highlight-style assignment should exit successfully");
    expect(visual_object_property(highlight_style_path, "one-guid", "HIGHLIGHTSTYLE") == "9" &&
            visual_object_property(highlight_style_path, "two-guid", "HIGHLIGHTSTYLE") == "9" &&
            visual_object_property(highlight_style_path, "three-guid", "HIGHLIGHTSTYLE") == "2" &&
            visual_object_property(highlight_style_path, "other-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: host object highlight-style assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_highlight_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--highlight-style-object",
            "--highlight-style", "2",
            "--highlight-style-target-unique-id", "one-guid",
            "--highlight-style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1132: missing-target host object highlight-style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HIGHLIGHTSTYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HIGHLIGHTSTYLE") == "1",
        "#1132: missing-target host object highlight-style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_highlight_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--highlight-style-object",
            "--highlight-style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1132: highlight-style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: highlight-style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_highlight_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--highlight-style-object",
            "--highlight-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1132: highlight-style-object without highlight-style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: highlight-style-object without highlight-style value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_highlight_style(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--highlight-style-object",
            "--highlight-style", "-1",
            "--highlight-style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1132: negative highlight-style values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: negative highlight-style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_highlight_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--highlight-style-object",
            "--highlight-style", "2",
            "--highlight-style-target-unique-id", "one-guid",
            "--highlight-style-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1132: duplicate-target host object highlight-style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: duplicate-target host object highlight-style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_highlight_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--highlight-style-object",
            "--locked-object",
            "--highlight-style", "2",
            "--highlight-style-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1132: highlight-style-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HIGHLIGHTSTYLE") == "0",
        "#1132: highlight-style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_input_mask_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_input_mask_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path input_mask_path = temp_root / "input_mask.scx";
    write_synthetic_form_table_for_object_input_mask(input_mask_path);
    const auto input_mask_process = run_process_capture(
        studio_host_path,
        {
            "--path", input_mask_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-object-name", "txtPhone",
            "--input-mask-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(input_mask_process.exit_code == 0,
        "#1046: host object input-mask assignment should exit successfully");
    expect(visual_object_property(input_mask_path, "one-guid", "INPUTMASK") == "AA 9999" &&
            visual_object_property(input_mask_path, "two-guid", "INPUTMASK") == "AA 9999" &&
            visual_object_property(input_mask_path, "three-guid", "INPUTMASK") == "Ready" &&
            visual_object_property(input_mask_path, "other-guid", "INPUTMASK") == "XXXXXXXX",
        "#1046: host object input-mask assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_input_mask(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--input-mask-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1046: missing-target host object input-mask assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "INPUTMASK") == "(999) 999-9999" &&
            visual_object_property(missing_target_path, "two-guid", "INPUTMASK") == "99999",
        "#1046: missing-target host object input-mask assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_input_mask(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1046: input-mask-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_input_mask(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--input-mask-object",
            "--input-mask-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1046: input-mask-object without input-mask value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object without input-mask value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_input_mask(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--input-mask-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--input-mask-target-object-name", "txtPhone",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1046: duplicate-target host object input-mask assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: duplicate-target host object input-mask assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_input_mask(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--input-mask-object",
            "--control-source-object",
            "--input-mask", "AA 9999",
            "--input-mask-target-unique-id", "one-guid",
            "--control-source", "customers.name",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1046: input-mask-object plus control-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "INPUTMASK") == "(999) 999-9999",
        "#1046: input-mask-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_format_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_format_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path format_path = temp_root / "format.scx";
    write_synthetic_form_table_for_object_format(format_path);
    const auto format_process = run_process_capture(
        studio_host_path,
        {
            "--path", format_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-object-name", "txtAmount",
            "--format-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(format_process.exit_code == 0,
        "#1047: host object format assignment should exit successfully");
    expect(visual_object_property(format_path, "one-guid", "FORMAT") == "@R 999,999.99" &&
            visual_object_property(format_path, "two-guid", "FORMAT") == "@R 999,999.99" &&
            visual_object_property(format_path, "three-guid", "FORMAT") == "Ready" &&
            visual_object_property(format_path, "other-guid", "FORMAT") == "!",
        "#1047: host object format assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_format(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--format-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1047: missing-target host object format assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "FORMAT") == "999,999.99" &&
            visual_object_property(missing_target_path, "two-guid", "FORMAT") == "99.99%",
        "#1047: missing-target host object format assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_format(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1047: format-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_format(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--format-object",
            "--format-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1047: format-object without format value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object without format value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_format(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--format-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--format-target-object-name", "txtAmount",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1047: duplicate-target host object format assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: duplicate-target host object format assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_format(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--format-object",
            "--input-mask-object",
            "--format", "@R 999,999.99",
            "--format-target-unique-id", "one-guid",
            "--input-mask", "99999",
            "--input-mask-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1047: format-object plus input-mask-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "FORMAT") == "999,999.99",
        "#1047: format-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_scroll_bars_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_scroll_bars_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path scroll_bars_path = temp_root / "scroll_bars.scx";
    write_synthetic_form_table_for_object_scroll_bars(scroll_bars_path);
    const auto scroll_bars_process = run_process_capture(
        studio_host_path,
        {
            "--path", scroll_bars_path.string(),
            "--scroll-bars-object",
            "--scroll-bars", "2",
            "--scroll-bars-target-object-name", "frmCustomer",
            "--scroll-bars-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(scroll_bars_process.exit_code == 0,
        "#1167: host object scroll-bars assignment should exit successfully");
    expect(visual_object_property(scroll_bars_path, "one-guid", "SCROLLBARS") == "2" &&
            visual_object_property(scroll_bars_path, "two-guid", "SCROLLBARS") == "2" &&
            visual_object_property(scroll_bars_path, "three-guid", "SCROLLBARS") == "1" &&
            visual_object_property(scroll_bars_path, "other-guid", "SCROLLBARS") == "1",
        "#1167: host object scroll-bars assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_scroll_bars(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--scroll-bars-object",
            "--scroll-bars", "2",
            "--scroll-bars-target-unique-id", "one-guid",
            "--scroll-bars-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1167: missing-target host object scroll-bars assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SCROLLBARS") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SCROLLBARS") == "0",
        "#1167: missing-target host object scroll-bars assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_scroll_bars(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--scroll-bars-object",
            "--scroll-bars", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1167: scroll-bars-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SCROLLBARS") == "0",
        "#1167: scroll-bars-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_scroll_bars(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--scroll-bars-object",
            "--scroll-bars-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1167: scroll-bars-object without scroll-bars value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SCROLLBARS") == "0",
        "#1167: scroll-bars-object without scroll-bars value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_scroll_bars(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--scroll-bars-object",
            "--scroll-bars", "-1",
            "--scroll-bars-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1167: negative scroll-bars values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "SCROLLBARS") == "0",
        "#1167: negative scroll-bars values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_scroll_bars(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--scroll-bars-object",
            "--scroll-bars", "2",
            "--scroll-bars-target-unique-id", "one-guid",
            "--scroll-bars-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1167: duplicate-target host object scroll-bars assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SCROLLBARS") == "0",
        "#1167: duplicate-target host object scroll-bars assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_scroll_bars(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--scroll-bars-object",
            "--allow-output-object",
            "--scroll-bars", "2",
            "--scroll-bars-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1167: scroll-bars-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SCROLLBARS") == "0",
        "#1167: scroll-bars-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_window_state_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_window_state_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path window_state_path = temp_root / "window_state.scx";
    write_synthetic_form_table_for_object_window_state(window_state_path);
    const auto window_state_process = run_process_capture(
        studio_host_path,
        {
            "--path", window_state_path.string(),
            "--window-state-object",
            "--window-state", "2",
            "--window-state-target-object-name", "frmCustomer",
            "--window-state-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(window_state_process.exit_code == 0,
        "#1168: host object window-state assignment should exit successfully");
    expect(visual_object_property(window_state_path, "one-guid", "WINDOWSTATE") == "2" &&
            visual_object_property(window_state_path, "two-guid", "WINDOWSTATE") == "2" &&
            visual_object_property(window_state_path, "three-guid", "WINDOWSTATE") == "1" &&
            visual_object_property(window_state_path, "other-guid", "WINDOWSTATE") == "1",
        "#1168: host object window-state assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_window_state(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--window-state-object",
            "--window-state", "2",
            "--window-state-target-unique-id", "one-guid",
            "--window-state-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1168: missing-target host object window-state assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "WINDOWSTATE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "WINDOWSTATE") == "0",
        "#1168: missing-target host object window-state assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_window_state(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--window-state-object",
            "--window-state", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1168: window-state-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "WINDOWSTATE") == "0",
        "#1168: window-state-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_window_state(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--window-state-object",
            "--window-state-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1168: window-state-object without window-state value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "WINDOWSTATE") == "0",
        "#1168: window-state-object without window-state value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_window_state(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--window-state-object",
            "--window-state", "-1",
            "--window-state-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1168: negative window-state values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "WINDOWSTATE") == "0",
        "#1168: negative window-state values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_window_state(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--window-state-object",
            "--window-state", "2",
            "--window-state-target-unique-id", "one-guid",
            "--window-state-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1168: duplicate-target host object window-state assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "WINDOWSTATE") == "0",
        "#1168: duplicate-target host object window-state assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_window_state(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--window-state-object",
            "--allow-output-object",
            "--window-state", "2",
            "--window-state-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1168: window-state-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "WINDOWSTATE") == "0",
        "#1168: window-state-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_show_window_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_show_window_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path show_window_path = temp_root / "show_window.scx";
    write_synthetic_form_table_for_object_show_window(show_window_path);
    const auto show_window_process = run_process_capture(
        studio_host_path,
        {
            "--path", show_window_path.string(),
            "--show-window-object",
            "--show-window", "2",
            "--show-window-target-object-name", "frmCustomer",
            "--show-window-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(show_window_process.exit_code == 0,
        "#1169: host object show-window assignment should exit successfully");
    expect(visual_object_property(show_window_path, "one-guid", "SHOWWINDOW") == "2" &&
            visual_object_property(show_window_path, "two-guid", "SHOWWINDOW") == "2" &&
            visual_object_property(show_window_path, "three-guid", "SHOWWINDOW") == "1" &&
            visual_object_property(show_window_path, "other-guid", "SHOWWINDOW") == "1",
        "#1169: host object show-window assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_show_window(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--show-window-object",
            "--show-window", "2",
            "--show-window-target-unique-id", "one-guid",
            "--show-window-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1169: missing-target host object show-window assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SHOWWINDOW") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "SHOWWINDOW") == "0",
        "#1169: missing-target host object show-window assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_show_window(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--show-window-object",
            "--show-window", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1169: show-window-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SHOWWINDOW") == "0",
        "#1169: show-window-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_show_window(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--show-window-object",
            "--show-window-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1169: show-window-object without show-window value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SHOWWINDOW") == "0",
        "#1169: show-window-object without show-window value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_show_window(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--show-window-object",
            "--show-window", "-1",
            "--show-window-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1169: negative show-window values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "SHOWWINDOW") == "0",
        "#1169: negative show-window values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_show_window(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--show-window-object",
            "--show-window", "2",
            "--show-window-target-unique-id", "one-guid",
            "--show-window-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1169: duplicate-target host object show-window assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SHOWWINDOW") == "0",
        "#1169: duplicate-target host object show-window assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_show_window(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--show-window-object",
            "--allow-output-object",
            "--show-window", "2",
            "--show-window-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1169: show-window-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SHOWWINDOW") == "0",
        "#1169: show-window-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_title_bar_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_title_bar_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path title_bar_path = temp_root / "title_bar.scx";
    write_synthetic_form_table_for_object_title_bar(title_bar_path);
    const auto title_bar_process = run_process_capture(
        studio_host_path,
        {
            "--path", title_bar_path.string(),
            "--title-bar-object",
            "--title-bar", "2",
            "--title-bar-target-object-name", "frmCustomer",
            "--title-bar-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(title_bar_process.exit_code == 0,
        "#1170: host object title-bar assignment should exit successfully");
    expect(visual_object_property(title_bar_path, "one-guid", "TITLEBAR") == "2" &&
            visual_object_property(title_bar_path, "two-guid", "TITLEBAR") == "2" &&
            visual_object_property(title_bar_path, "three-guid", "TITLEBAR") == "1" &&
            visual_object_property(title_bar_path, "other-guid", "TITLEBAR") == "1",
        "#1170: host object title-bar assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_title_bar(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--title-bar-object",
            "--title-bar", "2",
            "--title-bar-target-unique-id", "one-guid",
            "--title-bar-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1170: missing-target host object title-bar assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TITLEBAR") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "TITLEBAR") == "0",
        "#1170: missing-target host object title-bar assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_title_bar(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--title-bar-object",
            "--title-bar", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1170: title-bar-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TITLEBAR") == "0",
        "#1170: title-bar-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_title_bar(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--title-bar-object",
            "--title-bar-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1170: title-bar-object without title-bar value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TITLEBAR") == "0",
        "#1170: title-bar-object without title-bar value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_title_bar(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--title-bar-object",
            "--title-bar", "-1",
            "--title-bar-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1170: negative title-bar values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "TITLEBAR") == "0",
        "#1170: negative title-bar values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_title_bar(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--title-bar-object",
            "--title-bar", "2",
            "--title-bar-target-unique-id", "one-guid",
            "--title-bar-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1170: duplicate-target host object title-bar assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TITLEBAR") == "0",
        "#1170: duplicate-target host object title-bar assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_title_bar(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--title-bar-object",
            "--allow-output-object",
            "--title-bar", "2",
            "--title-bar-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1170: title-bar-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TITLEBAR") == "0",
        "#1170: title-bar-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_mouse_pointer_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mouse_pointer_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mouse_pointer_path = temp_root / "mouse_pointer.scx";
    write_synthetic_form_table_for_object_mouse_pointer(mouse_pointer_path);
    const auto mouse_pointer_process = run_process_capture(
        studio_host_path,
        {
            "--path", mouse_pointer_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer", "2",
            "--mouse-pointer-target-object-name", "frmCustomer",
            "--mouse-pointer-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mouse_pointer_process.exit_code == 0,
        "#1171: host object mouse-pointer assignment should exit successfully");
    expect(visual_object_property(mouse_pointer_path, "one-guid", "MOUSEPOINTER") == "2" &&
            visual_object_property(mouse_pointer_path, "two-guid", "MOUSEPOINTER") == "2" &&
            visual_object_property(mouse_pointer_path, "three-guid", "MOUSEPOINTER") == "1" &&
            visual_object_property(mouse_pointer_path, "other-guid", "MOUSEPOINTER") == "1",
        "#1171: host object mouse-pointer assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mouse_pointer(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer", "2",
            "--mouse-pointer-target-unique-id", "one-guid",
            "--mouse-pointer-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1171: missing-target host object mouse-pointer assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MOUSEPOINTER") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "MOUSEPOINTER") == "0",
        "#1171: missing-target host object mouse-pointer assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mouse_pointer(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1171: mouse-pointer-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MOUSEPOINTER") == "0",
        "#1171: mouse-pointer-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mouse_pointer(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1171: mouse-pointer-object without mouse-pointer value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MOUSEPOINTER") == "0",
        "#1171: mouse-pointer-object without mouse-pointer value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_mouse_pointer(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer", "-1",
            "--mouse-pointer-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1171: negative mouse-pointer values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MOUSEPOINTER") == "0",
        "#1171: negative mouse-pointer values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mouse_pointer(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mouse-pointer-object",
            "--mouse-pointer", "2",
            "--mouse-pointer-target-unique-id", "one-guid",
            "--mouse-pointer-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1171: duplicate-target host object mouse-pointer assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MOUSEPOINTER") == "0",
        "#1171: duplicate-target host object mouse-pointer assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mouse_pointer(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mouse-pointer-object",
            "--allow-output-object",
            "--mouse-pointer", "2",
            "--mouse-pointer-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1171: mouse-pointer-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MOUSEPOINTER") == "0",
        "#1171: mouse-pointer-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_input_mask_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_input_mask_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., '999-99-9999', '')";
    const fs::path dynamic_input_mask_path = temp_root / "dynamic_input_mask.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(dynamic_input_mask_path);
    const auto dynamic_input_mask_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_input_mask_path.string(),
            "--dynamic-input-mask-object",
            "--dynamic-input-mask", expression,
            "--dynamic-input-mask-target-object-name", "txtSsn",
            "--dynamic-input-mask-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_input_mask_process.exit_code == 0,
        "#1176: host object dynamic-input-mask assignment should exit successfully");
    expect(visual_object_property(dynamic_input_mask_path, "one-guid", "DYNAMICINPUTMASK") == expression &&
            visual_object_property(dynamic_input_mask_path, "two-guid", "DYNAMICINPUTMASK") == expression &&
            visual_object_property(dynamic_input_mask_path, "three-guid", "DYNAMICINPUTMASK") == "THREEMASK" &&
            visual_object_property(dynamic_input_mask_path, "other-guid", "DYNAMICINPUTMASK") == "OTHERMASK",
        "#1176: host object dynamic-input-mask assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-input-mask-object",
            "--dynamic-input-mask", expression,
            "--dynamic-input-mask-target-unique-id", "one-guid",
            "--dynamic-input-mask-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1176: missing-target host object dynamic-input-mask assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICINPUTMASK") == "OLDMASKONE" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICINPUTMASK") == "OLDMASKTWO",
        "#1176: missing-target host object dynamic-input-mask assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-input-mask-object",
            "--dynamic-input-mask", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1176: dynamic-input-mask-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICINPUTMASK") == "OLDMASKONE",
        "#1176: dynamic-input-mask-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-input-mask-object",
            "--dynamic-input-mask-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1176: dynamic-input-mask-object without dynamic-input-mask value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICINPUTMASK") == "OLDMASKONE",
        "#1176: dynamic-input-mask-object without dynamic-input-mask value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-input-mask-object",
            "--dynamic-input-mask", expression,
            "--dynamic-input-mask-target-unique-id", "one-guid",
            "--dynamic-input-mask-target-object-name", "txtSsn",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1176: duplicate-target host object dynamic-input-mask assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICINPUTMASK") == "OLDMASKONE",
        "#1176: duplicate-target host object dynamic-input-mask assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_input_mask(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-input-mask-object",
            "--allow-output-object",
            "--dynamic-input-mask", expression,
            "--dynamic-input-mask-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1176: dynamic-input-mask-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICINPUTMASK") == "OLDMASKONE",
        "#1176: dynamic-input-mask-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_current_control_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_current_control_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., 'txtMemo', 'txtNotes')";
    const fs::path dynamic_current_control_path = temp_root / "dynamic_current_control.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(dynamic_current_control_path);
    const auto dynamic_current_control_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_current_control_path.string(),
            "--dynamic-current-control-object",
            "--dynamic-current-control", expression,
            "--dynamic-current-control-target-object-name", "grdOrders",
            "--dynamic-current-control-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_current_control_process.exit_code == 0,
        "#1187: host object dynamic-current-control assignment should exit successfully");
    expect(visual_object_property(dynamic_current_control_path, "one-guid", "DYNAMICCURRENTCONTROL") == expression &&
            visual_object_property(dynamic_current_control_path, "two-guid", "DYNAMICCURRENTCONTROL") == expression &&
            visual_object_property(dynamic_current_control_path, "three-guid", "DYNAMICCURRENTCONTROL") == "THREECURRENT" &&
            visual_object_property(dynamic_current_control_path, "other-guid", "DYNAMICCURRENTCONTROL") == "OTHERCURRENT",
        "#1187: host object dynamic-current-control assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-current-control-object",
            "--dynamic-current-control", expression,
            "--dynamic-current-control-target-unique-id", "one-guid",
            "--dynamic-current-control-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1187: missing-target host object dynamic-current-control assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTONE" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTTWO",
        "#1187: missing-target host object dynamic-current-control assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-current-control-object",
            "--dynamic-current-control", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1187: dynamic-current-control-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTONE",
        "#1187: dynamic-current-control-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-current-control-object",
            "--dynamic-current-control-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1187: dynamic-current-control-object without dynamic-current-control value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTONE",
        "#1187: dynamic-current-control-object without dynamic-current-control value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-current-control-object",
            "--dynamic-current-control", expression,
            "--dynamic-current-control-target-unique-id", "one-guid",
            "--dynamic-current-control-target-object-name", "grdOrders",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1187: duplicate-target host object dynamic-current-control assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTONE",
        "#1187: duplicate-target host object dynamic-current-control assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_current_control(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-current-control-object",
            "--allow-output-object",
            "--dynamic-current-control", expression,
            "--dynamic-current-control-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1187: dynamic-current-control-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICCURRENTCONTROL") == "OLDCURRENTONE",
        "#1187: dynamic-current-control-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
