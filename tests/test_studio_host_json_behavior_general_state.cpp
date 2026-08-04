// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_tab_order(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TABINDEX", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "10"},
        {"cmdTwo", "cmdTwo", "two-guid", "20"},
        {"cmdThree", "cmdThree", "three-guid", "30"},
        {"cmdOther", "cmdOther", "other-guid", "99"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1036: synthetic SCX table for object tab order should be created");
}

void write_synthetic_form_table_for_object_tab_stop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TABSTOP", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1037: synthetic SCX table for object tab stop should be created");
}

void write_synthetic_form_table_for_object_visibility(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "VISIBLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1038: synthetic SCX table for object visibility should be created");
}

void write_synthetic_form_table_for_object_enabled(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ENABLED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", ".T."},
        {"cmdTwo", "cmdTwo", "two-guid", ".T."},
        {"cmdThree", "cmdThree", "three-guid", ".F."},
        {"cmdOther", "cmdOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1039: synthetic SCX table for object enabled state should be created");
}

void write_synthetic_form_table_for_object_read_only(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "READONLY", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "txtOne", "one-guid", ".F."},
        {"txtTwo", "txtTwo", "two-guid", ".F."},
        {"txtThree", "txtThree", "three-guid", ".T."},
        {"txtOther", "txtOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1040: synthetic SCX table for object read-only state should be created");
}

void write_synthetic_form_table_for_object_locked(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKED", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "txtOne", "one-guid", ".F."},
        {"txtTwo", "txtTwo", "two-guid", ".F."},
        {"txtThree", "txtThree", "three-guid", ".T."},
        {"txtOther", "txtOther", "other-guid", ".F."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1041: synthetic SCX table for object locked state should be created");
}

void write_synthetic_form_table_for_object_tab_orientation(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TABORIENTA", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1139: synthetic SCX table for object tab-orientation should be created");
}

void write_synthetic_form_table_for_object_display_orientation(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DISPLAYORI", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1140: synthetic SCX table for object display-orientation should be created");
}

void write_synthetic_form_table_for_object_help_context_id(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HELPCONTEX", .type = 'N', .length = 6U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1141: synthetic SCX table for object help-context-id should be created");
}

void write_synthetic_form_table_for_object_tooltip_text(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "TOOLTIPTEX", .type = 'C', .length = 64U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "Save"},
        {"cmdCancel", "cmdCancel", "two-guid", "Cancel"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"cmdOther", "cmdOther", "other-guid", "Other"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1043: synthetic SCX table for object tooltip text should be created");
}

void write_synthetic_form_table_for_object_style(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "STYLE", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "cboCustomer", "one-guid", "0"},
        {"lstOrders", "lstOrders", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "0"},
        {"cboOther", "cboOther", "other-guid", "2"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1052: synthetic SCX table for object style should be created");
}

void write_synthetic_form_table_for_object_allow_output(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALLOWOUTPU", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1075: synthetic SCX table for object allow output should be created");
}

void write_synthetic_form_table_for_object_add_line_feeds(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ADDLINEFEE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1095: synthetic SCX table for object add line feeds should be created");
}

void test_studio_host_json_assigns_tab_order_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tab_order_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tab_order_path = temp_root / "tab_order.scx";
    write_synthetic_form_table_for_object_tab_order(tab_order_path);
    const auto tab_order_process = run_process_capture(
        studio_host_path,
        {
            "--path", tab_order_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "5",
            "--tab-order-target-unique-id", "two-guid",
            "--tab-order-target-object-name", "cmdOne",
            "--tab-order-target-unique-id", "three-guid",
            "--json"
        },
        temp_root);
    expect(tab_order_process.exit_code == 0,
        "#1036: host object tab-order assignment should exit successfully");
    expect(visual_object_property(tab_order_path, "two-guid", "TABINDEX") == "5" &&
            visual_object_property(tab_order_path, "one-guid", "TABINDEX") == "6" &&
            visual_object_property(tab_order_path, "three-guid", "TABINDEX") == "7" &&
            visual_object_property(tab_order_path, "other-guid", "TABINDEX") == "99",
        "#1036: host object tab-order assignment should assign sequential indexes and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tab_order(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--tab-order-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1036: missing-target host object tab-order assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TABINDEX") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "TABINDEX") == "20",
        "#1036: missing-target host object tab-order assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tab_order(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "0",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1036: tab-order-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TABINDEX") == "10",
        "#1036: tab-order-object without target selectors should not mutate the asset");

    const fs::path negative_start_path = temp_root / "negative_start.scx";
    write_synthetic_form_table_for_object_tab_order(negative_start_path);
    const auto negative_start_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_start_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "-1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_start_process.exit_code == 2,
        "#1036: negative-start host object tab-order assignment should fail during launch parsing");
    expect(visual_object_property(negative_start_path, "one-guid", "TABINDEX") == "10",
        "#1036: negative-start host object tab-order assignment should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tab_order(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tab-order-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--tab-order-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1036: duplicate-target host object tab-order assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TABINDEX") == "10",
        "#1036: duplicate-target host object tab-order assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tab_order(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tab-order-object",
            "--nudge-object",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1036: tab-order-object plus nudge-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TABINDEX") == "10",
        "#1036: tab-order-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_tab_stop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tab_stop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tab_stop_path = temp_root / "tab_stop.scx";
    write_synthetic_form_table_for_object_tab_stop(tab_stop_path);
    const auto tab_stop_process = run_process_capture(
        studio_host_path,
        {
            "--path", tab_stop_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-object-name", "cmdOne",
            "--tab-stop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(tab_stop_process.exit_code == 0,
        "#1037: host object tab-stop assignment should exit successfully");
    expect(visual_object_property(tab_stop_path, "one-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "two-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "three-guid", "TABSTOP") == ".F." &&
            visual_object_property(tab_stop_path, "other-guid", "TABSTOP") == ".T.",
        "#1037: host object tab-stop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--tab-stop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1037: missing-target host object tab-stop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TABSTOP") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "TABSTOP") == ".T.",
        "#1037: missing-target host object tab-stop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tab-stop-object",
            "--tab-stop", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1037: tab-stop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_tab_stop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--tab-stop-object",
            "--tab-stop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1037: tab-stop-object without tab-stop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object without tab-stop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tab_stop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tab-stop-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--tab-stop-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1037: duplicate-target host object tab-stop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: duplicate-target host object tab-stop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tab_stop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tab-stop-object",
            "--tab-order-object",
            "--tab-stop", "false",
            "--tab-stop-target-unique-id", "one-guid",
            "--starting-tab-index", "1",
            "--tab-order-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1037: tab-stop-object plus tab-order-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TABSTOP") == ".T.",
        "#1037: tab-stop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_visibility_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visibility_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path visibility_path = temp_root / "visibility.scx";
    write_synthetic_form_table_for_object_visibility(visibility_path);
    const auto visibility_process = run_process_capture(
        studio_host_path,
        {
            "--path", visibility_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-object-name", "cmdOne",
            "--visibility-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(visibility_process.exit_code == 0,
        "#1038: host object visibility assignment should exit successfully");
    expect_contains(visibility_process.stdout_text, "\"dryRun\": false",
        "#4394: visibility-object success should expose mutation dry-run metadata");
    expect_contains(visibility_process.stdout_text, "\"mutatesAsset\": true",
        "#4394: visibility-object success should expose mutation metadata");
    expect(visual_object_property(visibility_path, "one-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "two-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "three-guid", "VISIBLE") == ".F." &&
            visual_object_property(visibility_path, "other-guid", "VISIBLE") == ".T.",
        "#1038: host object visibility assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_visibility(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--visibility-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1038: missing-target host object visibility assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "VISIBLE") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "VISIBLE") == ".T.",
        "#1038: missing-target host object visibility assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_visibility(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--visibility-object",
            "--visible", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1038: visibility-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_visibility(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--visibility-object",
            "--visibility-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1038: visibility-object without visible value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object without visible value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_visibility(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--visibility-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--visibility-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1038: duplicate-target host object visibility assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: duplicate-target host object visibility assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_visibility(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--visibility-object",
            "--tab-stop-object",
            "--visible", "false",
            "--visibility-target-unique-id", "one-guid",
            "--tab-stop", "true",
            "--tab-stop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1038: visibility-object plus tab-stop-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "VISIBLE") == ".T.",
        "#1038: visibility-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_enabled_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_enabled_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path enabled_path = temp_root / "enabled.scx";
    write_synthetic_form_table_for_object_enabled(enabled_path);
    const auto enabled_process = run_process_capture(
        studio_host_path,
        {
            "--path", enabled_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-object-name", "cmdOne",
            "--enabled-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(enabled_process.exit_code == 0,
        "#1039: host object enabled assignment should exit successfully");
    expect(visual_object_property(enabled_path, "one-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "two-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "three-guid", "ENABLED") == ".F." &&
            visual_object_property(enabled_path, "other-guid", "ENABLED") == ".T.",
        "#1039: host object enabled assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_enabled(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--enabled-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1039: missing-target host object enabled assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ENABLED") == ".T." &&
            visual_object_property(missing_target_path, "two-guid", "ENABLED") == ".T.",
        "#1039: missing-target host object enabled assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_enabled(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--enabled-object",
            "--enabled", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1039: enabled-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_enabled(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--enabled-object",
            "--enabled-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1039: enabled-object without enabled value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object without enabled value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_enabled(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--enabled-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--enabled-target-object-name", "cmdOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1039: duplicate-target host object enabled assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ENABLED") == ".T.",
        "#1039: duplicate-target host object enabled assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_enabled(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--enabled-object",
            "--visibility-object",
            "--enabled", "false",
            "--enabled-target-unique-id", "one-guid",
            "--visible", "true",
            "--visibility-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1039: enabled-object plus visibility-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ENABLED") == ".T.",
        "#1039: enabled-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_read_only_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_read_only_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path read_only_path = temp_root / "read_only.scx";
    write_synthetic_form_table_for_object_read_only(read_only_path);
    const auto read_only_process = run_process_capture(
        studio_host_path,
        {
            "--path", read_only_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-object-name", "txtOne",
            "--read-only-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(read_only_process.exit_code == 0,
        "#1040: host object read-only assignment should exit successfully");
    expect(visual_object_property(read_only_path, "one-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "two-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "three-guid", "READONLY") == ".T." &&
            visual_object_property(read_only_path, "other-guid", "READONLY") == ".F.",
        "#1040: host object read-only assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_read_only(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--read-only-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1040: missing-target host object read-only assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "READONLY") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "READONLY") == ".F.",
        "#1040: missing-target host object read-only assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_read_only(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1040: read-only-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_read_only(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--read-only-object",
            "--read-only-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1040: read-only-object without read-only value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object without read-only value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_read_only(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--read-only-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--read-only-target-object-name", "txtOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1040: duplicate-target host object read-only assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "READONLY") == ".F.",
        "#1040: duplicate-target host object read-only assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_read_only(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--read-only-object",
            "--enabled-object",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--enabled", "true",
            "--enabled-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1040: read-only-object plus enabled-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "READONLY") == ".F.",
        "#1040: read-only-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_locked_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_locked_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path locked_path = temp_root / "locked.scx";
    write_synthetic_form_table_for_object_locked(locked_path);
    const auto locked_process = run_process_capture(
        studio_host_path,
        {
            "--path", locked_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-object-name", "txtOne",
            "--locked-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(locked_process.exit_code == 0,
        "#1041: host object locked assignment should exit successfully");
    expect(visual_object_property(locked_path, "one-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "two-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "three-guid", "LOCKED") == ".T." &&
            visual_object_property(locked_path, "other-guid", "LOCKED") == ".F.",
        "#1041: host object locked assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_locked(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--locked-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1041: missing-target host object locked assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKED") == ".F." &&
            visual_object_property(missing_target_path, "two-guid", "LOCKED") == ".F.",
        "#1041: missing-target host object locked assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_locked(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--locked-object",
            "--locked", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1041: locked-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_locked(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--locked-object",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1041: locked-object without locked value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object without locked value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_locked(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--locked-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--locked-target-object-name", "txtOne",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1041: duplicate-target host object locked assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKED") == ".F.",
        "#1041: duplicate-target host object locked assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_locked(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--locked-object",
            "--read-only-object",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--object-read-only", "true",
            "--read-only-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1041: locked-object plus read-only-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKED") == ".F.",
        "#1041: locked-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_tab_orientation_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tab_orientation_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tab_orientation_path = temp_root / "tab_orientation.scx";
    write_synthetic_form_table_for_object_tab_orientation(tab_orientation_path);
    const auto tab_orientation_process = run_process_capture(
        studio_host_path,
        {
            "--path", tab_orientation_path.string(),
            "--tab-orientation-object",
            "--tab-orientation", "9",
            "--tab-orientation-target-object-name", "cmdSave",
            "--tab-orientation-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(tab_orientation_process.exit_code == 0,
        "#1139: host object tab-orientation assignment should exit successfully");
    expect(visual_object_property(tab_orientation_path, "one-guid", "TABORIENTATION") == "9" &&
            visual_object_property(tab_orientation_path, "two-guid", "TABORIENTATION") == "9" &&
            visual_object_property(tab_orientation_path, "three-guid", "TABORIENTATION") == "2" &&
            visual_object_property(tab_orientation_path, "other-guid", "TABORIENTATION") == "0",
        "#1139: host object tab-orientation assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tab_orientation(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tab-orientation-object",
            "--tab-orientation", "2",
            "--tab-orientation-target-unique-id", "one-guid",
            "--tab-orientation-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1139: missing-target host object tab-orientation assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TABORIENTATION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "TABORIENTATION") == "1",
        "#1139: missing-target host object tab-orientation assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tab_orientation(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tab-orientation-object",
            "--tab-orientation", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1139: tab-orientation-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TABORIENTATION") == "0",
        "#1139: tab-orientation-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_tab_orientation(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--tab-orientation-object",
            "--tab-orientation-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1139: tab-orientation-object without tab-orientation value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TABORIENTATION") == "0",
        "#1139: tab-orientation-object without tab-orientation value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_tab_orientation(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--tab-orientation-object",
            "--tab-orientation", "-1",
            "--tab-orientation-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1139: negative tab-orientation values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "TABORIENTATION") == "0",
        "#1139: negative tab-orientation values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tab_orientation(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tab-orientation-object",
            "--tab-orientation", "2",
            "--tab-orientation-target-unique-id", "one-guid",
            "--tab-orientation-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1139: duplicate-target host object tab-orientation assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TABORIENTATION") == "0",
        "#1139: duplicate-target host object tab-orientation assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tab_orientation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tab-orientation-object",
            "--locked-object",
            "--tab-orientation", "2",
            "--tab-orientation-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1139: tab-orientation-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TABORIENTATION") == "0",
        "#1139: tab-orientation-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_display_orientation_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_display_orientation_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path display_orientation_path = temp_root / "display_orientation.scx";
    write_synthetic_form_table_for_object_display_orientation(display_orientation_path);
    const auto display_orientation_process = run_process_capture(
        studio_host_path,
        {
            "--path", display_orientation_path.string(),
            "--display-orientation-object",
            "--display-orientation", "9",
            "--display-orientation-target-object-name", "cmdSave",
            "--display-orientation-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(display_orientation_process.exit_code == 0,
        "#1140: host object display-orientation assignment should exit successfully");
    expect(visual_object_property(display_orientation_path, "one-guid", "DISPLAYORIENTATION") == "9" &&
            visual_object_property(display_orientation_path, "two-guid", "DISPLAYORIENTATION") == "9" &&
            visual_object_property(display_orientation_path, "three-guid", "DISPLAYORIENTATION") == "2" &&
            visual_object_property(display_orientation_path, "other-guid", "DISPLAYORIENTATION") == "0",
        "#1140: host object display-orientation assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_display_orientation(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--display-orientation-object",
            "--display-orientation", "2",
            "--display-orientation-target-unique-id", "one-guid",
            "--display-orientation-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1140: missing-target host object display-orientation assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISPLAYORIENTATION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DISPLAYORIENTATION") == "1",
        "#1140: missing-target host object display-orientation assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_display_orientation(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--display-orientation-object",
            "--display-orientation", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1140: display-orientation-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISPLAYORIENTATION") == "0",
        "#1140: display-orientation-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_display_orientation(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--display-orientation-object",
            "--display-orientation-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1140: display-orientation-object without display-orientation value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISPLAYORIENTATION") == "0",
        "#1140: display-orientation-object without display-orientation value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_display_orientation(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--display-orientation-object",
            "--display-orientation", "-1",
            "--display-orientation-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1140: negative display-orientation values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DISPLAYORIENTATION") == "0",
        "#1140: negative display-orientation values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_display_orientation(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--display-orientation-object",
            "--display-orientation", "2",
            "--display-orientation-target-unique-id", "one-guid",
            "--display-orientation-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1140: duplicate-target host object display-orientation assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISPLAYORIENTATION") == "0",
        "#1140: duplicate-target host object display-orientation assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_display_orientation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--display-orientation-object",
            "--locked-object",
            "--display-orientation", "2",
            "--display-orientation-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1140: display-orientation-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISPLAYORIENTATION") == "0",
        "#1140: display-orientation-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_help_context_id_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_help_context_id_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path help_context_id_path = temp_root / "help_context_id.scx";
    write_synthetic_form_table_for_object_help_context_id(help_context_id_path);
    const auto help_context_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", help_context_id_path.string(),
            "--help-context-id-object",
            "--help-context-id", "900",
            "--help-context-id-target-object-name", "cmdSave",
            "--help-context-id-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(help_context_id_process.exit_code == 0,
        "#1141: host object help-context-id assignment should exit successfully");
    expect(visual_object_property(help_context_id_path, "one-guid", "HELPCONTEXTID") == "900" &&
            visual_object_property(help_context_id_path, "two-guid", "HELPCONTEXTID") == "900" &&
            visual_object_property(help_context_id_path, "three-guid", "HELPCONTEXTID") == "2" &&
            visual_object_property(help_context_id_path, "other-guid", "HELPCONTEXTID") == "0",
        "#1141: host object help-context-id assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_help_context_id(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--help-context-id-object",
            "--help-context-id", "2",
            "--help-context-id-target-unique-id", "one-guid",
            "--help-context-id-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1141: missing-target host object help-context-id assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HELPCONTEXTID") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "HELPCONTEXTID") == "1",
        "#1141: missing-target host object help-context-id assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_help_context_id(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--help-context-id-object",
            "--help-context-id", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1141: help-context-id-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "HELPCONTEXTID") == "0",
        "#1141: help-context-id-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_help_context_id(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--help-context-id-object",
            "--help-context-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1141: help-context-id-object without help-context-id value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "HELPCONTEXTID") == "0",
        "#1141: help-context-id-object without help-context-id value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_help_context_id(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--help-context-id-object",
            "--help-context-id", "-1",
            "--help-context-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1141: negative help-context-id values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "HELPCONTEXTID") == "0",
        "#1141: negative help-context-id values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_help_context_id(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--help-context-id-object",
            "--help-context-id", "2",
            "--help-context-id-target-unique-id", "one-guid",
            "--help-context-id-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1141: duplicate-target host object help-context-id assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "HELPCONTEXTID") == "0",
        "#1141: duplicate-target host object help-context-id assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_help_context_id(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--help-context-id-object",
            "--locked-object",
            "--help-context-id", "2",
            "--help-context-id-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1141: help-context-id-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HELPCONTEXTID") == "0",
        "#1141: help-context-id-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_tooltip_text_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_tooltip_text_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path tooltip_text_path = temp_root / "tooltip_text.scx";
    write_synthetic_form_table_for_object_tooltip_text(tooltip_text_path);
    const auto tooltip_text_process = run_process_capture(
        studio_host_path,
        {
            "--path", tooltip_text_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-object-name", "cmdSave",
            "--tooltip-text-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(tooltip_text_process.exit_code == 0,
        "#1043: host object tooltip text assignment should exit successfully");
    expect(visual_object_property(tooltip_text_path, "one-guid", "TOOLTIPTEXT") == "Save this customer" &&
            visual_object_property(tooltip_text_path, "two-guid", "TOOLTIPTEXT") == "Save this customer" &&
            visual_object_property(tooltip_text_path, "three-guid", "TOOLTIPTEXT") == "Ready" &&
            visual_object_property(tooltip_text_path, "other-guid", "TOOLTIPTEXT") == "Other",
        "#1043: host object tooltip text assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--tooltip-text-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1043: missing-target host object tooltip text assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "TOOLTIPTEXT") == "Save" &&
            visual_object_property(missing_target_path, "two-guid", "TOOLTIPTEXT") == "Cancel",
        "#1043: missing-target host object tooltip text assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1043: tooltip-text-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_tooltip_text(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--tooltip-text-object",
            "--tooltip-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1043: tooltip-text-object without tooltip text value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object without tooltip text value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_tooltip_text(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--tooltip-text-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--tooltip-text-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1043: duplicate-target host object tooltip text assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: duplicate-target host object tooltip text assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_tooltip_text(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--tooltip-text-object",
            "--caption-object",
            "--tooltip-text", "Save this customer",
            "--tooltip-text-target-unique-id", "one-guid",
            "--caption", "Save Customer",
            "--caption-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1043: tooltip-text-object plus caption-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "TOOLTIPTEXT") == "Save",
        "#1043: tooltip-text-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_style_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_style_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path style_path = temp_root / "style.scx";
    write_synthetic_form_table_for_object_style(style_path);
    const auto style_process = run_process_capture(
        studio_host_path,
        {
            "--path", style_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-object-name", "cboCustomer",
            "--style-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(style_process.exit_code == 0,
        "#1052: host object style assignment should exit successfully");
    expect(visual_object_property(style_path, "one-guid", "STYLE") == "2" &&
            visual_object_property(style_path, "two-guid", "STYLE") == "2" &&
            visual_object_property(style_path, "three-guid", "STYLE") == "0" &&
            visual_object_property(style_path, "other-guid", "STYLE") == "2",
        "#1052: host object style assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_style(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--style-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1052: missing-target host object style assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "STYLE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "STYLE") == "1",
        "#1052: missing-target host object style assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_style(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--style-object",
            "--style", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1052: style-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "STYLE") == "0",
        "#1052: style-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_style(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--style-object",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1052: style-object without style value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "STYLE") == "0",
        "#1052: style-object without style value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_style(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--style-object",
            "--style", "-1",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1052: negative style values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "STYLE") == "0",
        "#1052: negative style values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_style(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--style-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--style-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1052: duplicate-target host object style assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "STYLE") == "0",
        "#1052: duplicate-target host object style assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_style(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--style-object",
            "--column-count-object",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1052: style-object plus column-count-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "STYLE") == "0",
        "#1052: style-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_allow_output_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_allow_output_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path allow_output_path = temp_root / "allow_output.scx";
    write_synthetic_form_table_for_object_allow_output(allow_output_path);
    const auto allow_output_process = run_process_capture(
        studio_host_path,
        {
            "--path", allow_output_path.string(),
            "--allow-output-object",
            "--allow-output", "false",
            "--allow-output-target-object-name", "frmCustomer",
            "--allow-output-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(allow_output_process.exit_code == 0,
        "#1075: host object allow-output assignment should exit successfully");
    expect(visual_object_property(allow_output_path, "one-guid", "ALLOWOUTPUT") == "false" &&
            visual_object_property(allow_output_path, "two-guid", "ALLOWOUTPUT") == "false" &&
            visual_object_property(allow_output_path, "three-guid", "ALLOWOUTPUT") == "false" &&
            visual_object_property(allow_output_path, "other-guid", "ALLOWOUTPUT") == "true",
        "#1075: host object allow-output assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_allow_output(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--allow-output-object",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--allow-output-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1075: missing-target host object allow-output assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALLOWOUTPUT") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALLOWOUTPUT") == "true",
        "#1075: missing-target host object allow-output assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_allow_output(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--allow-output-object",
            "--allow-output", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1075: allow-output-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALLOWOUTPUT") == "true",
        "#1075: allow-output-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_allow_output(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--allow-output-object",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1075: allow-output-object without allow-output value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALLOWOUTPUT") == "true",
        "#1075: allow-output-object without allow-output value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_allow_output(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--allow-output-object",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--allow-output-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1075: duplicate-target host object allow-output assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALLOWOUTPUT") == "true",
        "#1075: duplicate-target host object allow-output assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_allow_output(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--allow-output-object",
            "--control-box-object",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1075: allow-output-object plus control-box-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALLOWOUTPUT") == "true",
        "#1075: allow-output-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_add_line_feeds_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_add_line_feeds_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path add_line_feeds_path = temp_root / "add_line_feeds.scx";
    write_synthetic_form_table_for_object_add_line_feeds(add_line_feeds_path);
    const auto add_line_feeds_process = run_process_capture(
        studio_host_path,
        {
            "--path", add_line_feeds_path.string(),
            "--add-line-feeds-object",
            "--add-line-feeds", "false",
            "--add-line-feeds-target-object-name", "frmCustomer",
            "--add-line-feeds-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(add_line_feeds_process.exit_code == 0,
        "#1095: host object add-line-feeds assignment should exit successfully");
    expect(visual_object_property(add_line_feeds_path, "one-guid", "ADDLINEFEEDS") == "false" &&
            visual_object_property(add_line_feeds_path, "two-guid", "ADDLINEFEEDS") == "false" &&
            visual_object_property(add_line_feeds_path, "three-guid", "ADDLINEFEEDS") == "false" &&
            visual_object_property(add_line_feeds_path, "other-guid", "ADDLINEFEEDS") == "true",
        "#1095: host object add-line-feeds assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_add_line_feeds(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--add-line-feeds-object",
            "--add-line-feeds", "false",
            "--add-line-feeds-target-unique-id", "one-guid",
            "--add-line-feeds-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1095: missing-target host object add-line-feeds assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ADDLINEFEEDS") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ADDLINEFEEDS") == "true",
        "#1095: missing-target host object add-line-feeds assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_add_line_feeds(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--add-line-feeds-object",
            "--add-line-feeds", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1095: add-line-feeds-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ADDLINEFEEDS") == "true",
        "#1095: add-line-feeds-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_add_line_feeds(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--add-line-feeds-object",
            "--add-line-feeds-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1095: add-line-feeds-object without add-line-feeds value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ADDLINEFEEDS") == "true",
        "#1095: add-line-feeds-object without add-line-feeds value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_add_line_feeds(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--add-line-feeds-object",
            "--add-line-feeds", "false",
            "--add-line-feeds-target-unique-id", "one-guid",
            "--add-line-feeds-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1095: duplicate-target host object add-line-feeds assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ADDLINEFEEDS") == "true",
        "#1095: duplicate-target host object add-line-feeds assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_add_line_feeds(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--add-line-feeds-object",
            "--auto-size-object",
            "--add-line-feeds", "false",
            "--add-line-feeds-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1095: add-line-feeds-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ADDLINEFEEDS") == "true",
        "#1095: add-line-feeds-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
