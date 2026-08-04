// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_list_item_id(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LISTITEMID", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1135: synthetic SCX table for object list-item-id should be created");
}

void write_synthetic_form_table_for_object_control_source(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CONTROLSOU", .type = 'C', .length = 70U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtName", "txtName", "one-guid", "customers.name"},
        {"txtCity", "txtCity", "two-guid", "customers.city"},
        {"lblStatus", "lblStatus", "three-guid", "Ready"},
        {"txtOther", "txtOther", "other-guid", "customers.state"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1045: synthetic SCX table for object control source should be created");
}

void write_synthetic_form_table_for_object_current_control(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CURRENTCON", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "txtName"},
        {"frmOrder", "frmOrder", "two-guid", "txtOrderId"},
        {"cntDetails", "cntDetails", "three-guid", "txtDetail"},
        {"frmOther", "frmOther", "other-guid", "txtOther"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1072: synthetic SCX table for object current control should be created");
}

void write_synthetic_form_table_for_object_auto_verb_menu(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "AUTOVERBME", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1145: synthetic SCX table for object auto verb menu should be created");
}

void write_synthetic_form_table_for_object_bind_controls(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BINDCONTRO", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1146: synthetic SCX table for object bind controls should be created");
}

void write_synthetic_form_table_for_object_auto_size(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "AUTOSIZE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1079: synthetic SCX table for object auto size should be created");
}

void write_synthetic_form_table_for_object_auto_release(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "AUTORELEAS", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1080: synthetic SCX table for object auto release should be created");
}

void write_synthetic_form_table_for_object_clip_controls(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CLIPCONTRO", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1083: synthetic SCX table for object clip controls should be created");
}

void test_studio_host_json_assigns_list_item_id_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_list_item_id_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path list_item_id_path = temp_root / "list_item_id.scx";
    write_synthetic_form_table_for_object_list_item_id(list_item_id_path);
    const auto list_item_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", list_item_id_path.string(),
            "--list-item-id-object",
            "--list-item-id", "9",
            "--list-item-id-target-object-name", "cmdSave",
            "--list-item-id-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(list_item_id_process.exit_code == 0,
        "#1135: host object list-item-id assignment should exit successfully");
    expect(visual_object_property(list_item_id_path, "one-guid", "LISTITEMID") == "9" &&
            visual_object_property(list_item_id_path, "two-guid", "LISTITEMID") == "9" &&
            visual_object_property(list_item_id_path, "three-guid", "LISTITEMID") == "2" &&
            visual_object_property(list_item_id_path, "other-guid", "LISTITEMID") == "0",
        "#1135: host object list-item-id assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_list_item_id(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--list-item-id-object",
            "--list-item-id", "2",
            "--list-item-id-target-unique-id", "one-guid",
            "--list-item-id-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1135: missing-target host object list-item-id assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LISTITEMID") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LISTITEMID") == "1",
        "#1135: missing-target host object list-item-id assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_list_item_id(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--list-item-id-object",
            "--list-item-id", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1135: list-item-id-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LISTITEMID") == "0",
        "#1135: list-item-id-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_list_item_id(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--list-item-id-object",
            "--list-item-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1135: list-item-id-object without list-item-id value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LISTITEMID") == "0",
        "#1135: list-item-id-object without list-item-id value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_list_item_id(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--list-item-id-object",
            "--list-item-id", "-1",
            "--list-item-id-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1135: negative list-item-id values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "LISTITEMID") == "0",
        "#1135: negative list-item-id values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_list_item_id(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--list-item-id-object",
            "--list-item-id", "2",
            "--list-item-id-target-unique-id", "one-guid",
            "--list-item-id-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1135: duplicate-target host object list-item-id assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LISTITEMID") == "0",
        "#1135: duplicate-target host object list-item-id assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_list_item_id(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--list-item-id-object",
            "--locked-object",
            "--list-item-id", "2",
            "--list-item-id-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1135: list-item-id-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LISTITEMID") == "0",
        "#1135: list-item-id-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_control_source_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_control_source_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path control_source_path = temp_root / "control_source.scx";
    write_synthetic_form_table_for_object_control_source(control_source_path);
    const auto control_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", control_source_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-object-name", "txtName",
            "--control-source-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(control_source_process.exit_code == 0,
        "#1045: host object control-source assignment should exit successfully");
    expect(visual_object_property(control_source_path, "one-guid", "CONTROLSOURCE") == "ThisForm.Current Customer" &&
            visual_object_property(control_source_path, "two-guid", "CONTROLSOURCE") == "ThisForm.Current Customer" &&
            visual_object_property(control_source_path, "three-guid", "CONTROLSOURCE") == "Ready" &&
            visual_object_property(control_source_path, "other-guid", "CONTROLSOURCE") == "customers.state",
        "#1045: host object control-source assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_control_source(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--control-source-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1045: missing-target host object control-source assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CONTROLSOURCE") == "customers.name" &&
            visual_object_property(missing_target_path, "two-guid", "CONTROLSOURCE") == "customers.city",
        "#1045: missing-target host object control-source assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_control_source(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1045: control-source-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_control_source(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--control-source-object",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1045: control-source-object without control-source value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object without control-source value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_control_source(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--control-source-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--control-source-target-object-name", "txtName",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1045: duplicate-target host object control-source assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: duplicate-target host object control-source assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_control_source(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--control-source-object",
            "--status-bar-text-object",
            "--control-source", "ThisForm.Current Customer",
            "--control-source-target-unique-id", "one-guid",
            "--status-bar-text", "Ready",
            "--status-bar-text-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1045: control-source-object plus status-bar-text-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CONTROLSOURCE") == "customers.name",
        "#1045: control-source-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_current_control_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_current_control_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path current_control_path = temp_root / "current_control.scx";
    write_synthetic_form_table_for_object_current_control(current_control_path);
    const auto current_control_process = run_process_capture(
        studio_host_path,
        {
            "--path", current_control_path.string(),
            "--current-control-object",
            "--current-control", "txtCity",
            "--current-control-target-object-name", "frmCustomer",
            "--current-control-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(current_control_process.exit_code == 0,
        "#1072: host object current-control assignment should exit successfully");
    expect(visual_object_property(current_control_path, "one-guid", "CURRENTCONTROL") == "txtCity" &&
            visual_object_property(current_control_path, "two-guid", "CURRENTCONTROL") == "txtCity" &&
            visual_object_property(current_control_path, "three-guid", "CURRENTCONTROL") == "txtDetail" &&
            visual_object_property(current_control_path, "other-guid", "CURRENTCONTROL") == "txtOther",
        "#1072: host object current-control assignment should assign selected text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_current_control(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--current-control-object",
            "--current-control", "txtCity",
            "--current-control-target-unique-id", "one-guid",
            "--current-control-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1072: missing-target host object current-control assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CURRENTCONTROL") == "txtName" &&
            visual_object_property(missing_target_path, "two-guid", "CURRENTCONTROL") == "txtOrderId",
        "#1072: missing-target host object current-control assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_current_control(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--current-control-object",
            "--current-control", "txtCity",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1072: current-control-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CURRENTCONTROL") == "txtName",
        "#1072: current-control-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_current_control(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--current-control-object",
            "--current-control-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1072: current-control-object without current-control value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CURRENTCONTROL") == "txtName",
        "#1072: current-control-object without current-control value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_current_control(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--current-control-object",
            "--current-control", "txtCity",
            "--current-control-target-unique-id", "one-guid",
            "--current-control-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1072: duplicate-target host object current-control assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CURRENTCONTROL") == "txtName",
        "#1072: duplicate-target host object current-control assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_current_control(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--current-control-object",
            "--control-source-object",
            "--current-control", "txtCity",
            "--current-control-target-unique-id", "one-guid",
            "--control-source", "customers.name",
            "--control-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1072: current-control-object plus control-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CURRENTCONTROL") == "txtName",
        "#1072: current-control-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_bind_controls_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_bind_controls_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path bind_controls_path = temp_root / "bind_controls.scx";
    write_synthetic_form_table_for_object_bind_controls(bind_controls_path);
    const auto bind_controls_process = run_process_capture(
        studio_host_path,
        {
            "--path", bind_controls_path.string(),
            "--bind-controls-object",
            "--bind-controls", "false",
            "--bind-controls-target-object-name", "frmCustomer",
            "--bind-controls-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(bind_controls_process.exit_code == 0,
        "#1146: host object bind-controls assignment should exit successfully");
    expect(visual_object_property(bind_controls_path, "one-guid", "BINDCONTROLS") == "false" &&
            visual_object_property(bind_controls_path, "two-guid", "BINDCONTROLS") == "false" &&
            visual_object_property(bind_controls_path, "three-guid", "BINDCONTROLS") == "false" &&
            visual_object_property(bind_controls_path, "other-guid", "BINDCONTROLS") == "true",
        "#1146: host object bind-controls assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_bind_controls(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--bind-controls-object",
            "--bind-controls", "false",
            "--bind-controls-target-unique-id", "one-guid",
            "--bind-controls-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1146: missing-target host object bind-controls assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BINDCONTROLS") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "BINDCONTROLS") == "true",
        "#1146: missing-target host object bind-controls assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_bind_controls(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--bind-controls-object",
            "--bind-controls", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1146: bind-controls-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BINDCONTROLS") == "true",
        "#1146: bind-controls-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_bind_controls(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--bind-controls-object",
            "--bind-controls-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1146: bind-controls-object without bind-controls value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BINDCONTROLS") == "true",
        "#1146: bind-controls-object without bind-controls value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_bind_controls(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--bind-controls-object",
            "--bind-controls", "false",
            "--bind-controls-target-unique-id", "one-guid",
            "--bind-controls-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1146: duplicate-target host object bind-controls assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BINDCONTROLS") == "true",
        "#1146: duplicate-target host object bind-controls assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_bind_controls(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--bind-controls-object",
            "--allow-output-object",
            "--bind-controls", "false",
            "--bind-controls-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1146: bind-controls-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BINDCONTROLS") == "true",
        "#1146: bind-controls-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_auto_verb_menu_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_auto_verb_menu_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path auto_verb_menu_path = temp_root / "auto_verb_menu.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(auto_verb_menu_path);
    const auto auto_verb_menu_process = run_process_capture(
        studio_host_path,
        {
            "--path", auto_verb_menu_path.string(),
            "--auto-verb-menu-object",
            "--auto-verb-menu", "false",
            "--auto-verb-menu-target-object-name", "frmCustomer",
            "--auto-verb-menu-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(auto_verb_menu_process.exit_code == 0,
        "#1145: host object auto-verb-menu assignment should exit successfully");
    expect(visual_object_property(auto_verb_menu_path, "one-guid", "AUTOVERBMENU") == "false" &&
            visual_object_property(auto_verb_menu_path, "two-guid", "AUTOVERBMENU") == "false" &&
            visual_object_property(auto_verb_menu_path, "three-guid", "AUTOVERBMENU") == "false" &&
            visual_object_property(auto_verb_menu_path, "other-guid", "AUTOVERBMENU") == "true",
        "#1145: host object auto-verb-menu assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--auto-verb-menu-object",
            "--auto-verb-menu", "false",
            "--auto-verb-menu-target-unique-id", "one-guid",
            "--auto-verb-menu-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1145: missing-target host object auto-verb-menu assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "AUTOVERBMENU") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "AUTOVERBMENU") == "true",
        "#1145: missing-target host object auto-verb-menu assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--auto-verb-menu-object",
            "--auto-verb-menu", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1145: auto-verb-menu-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "AUTOVERBMENU") == "true",
        "#1145: auto-verb-menu-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--auto-verb-menu-object",
            "--auto-verb-menu-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1145: auto-verb-menu-object without auto-verb-menu value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "AUTOVERBMENU") == "true",
        "#1145: auto-verb-menu-object without auto-verb-menu value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--auto-verb-menu-object",
            "--auto-verb-menu", "false",
            "--auto-verb-menu-target-unique-id", "one-guid",
            "--auto-verb-menu-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1145: duplicate-target host object auto-verb-menu assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "AUTOVERBMENU") == "true",
        "#1145: duplicate-target host object auto-verb-menu assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_auto_verb_menu(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--auto-verb-menu-object",
            "--allow-output-object",
            "--auto-verb-menu", "false",
            "--auto-verb-menu-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1145: auto-verb-menu-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "AUTOVERBMENU") == "true",
        "#1145: auto-verb-menu-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_auto_size_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_auto_size_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path auto_size_path = temp_root / "auto_size.scx";
    write_synthetic_form_table_for_object_auto_size(auto_size_path);
    const auto auto_size_process = run_process_capture(
        studio_host_path,
        {
            "--path", auto_size_path.string(),
            "--auto-size-object",
            "--auto-size", "false",
            "--auto-size-target-object-name", "frmCustomer",
            "--auto-size-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(auto_size_process.exit_code == 0,
        "#1079: host object auto-size assignment should exit successfully");
    expect(visual_object_property(auto_size_path, "one-guid", "AUTOSIZE") == "false" &&
            visual_object_property(auto_size_path, "two-guid", "AUTOSIZE") == "false" &&
            visual_object_property(auto_size_path, "three-guid", "AUTOSIZE") == "false" &&
            visual_object_property(auto_size_path, "other-guid", "AUTOSIZE") == "true",
        "#1079: host object auto-size assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_auto_size(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--auto-size-object",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--auto-size-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1079: missing-target host object auto-size assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "AUTOSIZE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "AUTOSIZE") == "true",
        "#1079: missing-target host object auto-size assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_auto_size(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--auto-size-object",
            "--auto-size", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1079: auto-size-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "AUTOSIZE") == "true",
        "#1079: auto-size-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_auto_size(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--auto-size-object",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1079: auto-size-object without auto-size value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "AUTOSIZE") == "true",
        "#1079: auto-size-object without auto-size value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_auto_size(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--auto-size-object",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--auto-size-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1079: duplicate-target host object auto-size assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "AUTOSIZE") == "true",
        "#1079: duplicate-target host object auto-size assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_auto_size(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--auto-size-object",
            "--auto-center-object",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1079: auto-size-object plus auto-center-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "AUTOSIZE") == "true",
        "#1079: auto-size-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_auto_release_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_auto_release_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path auto_release_path = temp_root / "auto_release.scx";
    write_synthetic_form_table_for_object_auto_release(auto_release_path);
    const auto auto_release_process = run_process_capture(
        studio_host_path,
        {
            "--path", auto_release_path.string(),
            "--auto-release-object",
            "--auto-release", "false",
            "--auto-release-target-object-name", "frmCustomer",
            "--auto-release-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(auto_release_process.exit_code == 0,
        "#1080: host object auto-release assignment should exit successfully");
    expect(visual_object_property(auto_release_path, "one-guid", "AUTORELEASE") == "false" &&
            visual_object_property(auto_release_path, "two-guid", "AUTORELEASE") == "false" &&
            visual_object_property(auto_release_path, "three-guid", "AUTORELEASE") == "false" &&
            visual_object_property(auto_release_path, "other-guid", "AUTORELEASE") == "true",
        "#1080: host object auto-release assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_auto_release(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--auto-release-object",
            "--auto-release", "false",
            "--auto-release-target-unique-id", "one-guid",
            "--auto-release-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1080: missing-target host object auto-release assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "AUTORELEASE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "AUTORELEASE") == "true",
        "#1080: missing-target host object auto-release assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_auto_release(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--auto-release-object",
            "--auto-release", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1080: auto-release-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "AUTORELEASE") == "true",
        "#1080: auto-release-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_auto_release(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--auto-release-object",
            "--auto-release-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1080: auto-release-object without auto-release value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "AUTORELEASE") == "true",
        "#1080: auto-release-object without auto-release value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_auto_release(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--auto-release-object",
            "--auto-release", "false",
            "--auto-release-target-unique-id", "one-guid",
            "--auto-release-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1080: duplicate-target host object auto-release assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "AUTORELEASE") == "true",
        "#1080: duplicate-target host object auto-release assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_auto_release(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--auto-release-object",
            "--auto-size-object",
            "--auto-release", "false",
            "--auto-release-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1080: auto-release-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "AUTORELEASE") == "true",
        "#1080: auto-release-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_clip_controls_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_clip_controls_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path clip_controls_path = temp_root / "clip_controls.scx";
    write_synthetic_form_table_for_object_clip_controls(clip_controls_path);
    const auto clip_controls_process = run_process_capture(
        studio_host_path,
        {
            "--path", clip_controls_path.string(),
            "--clip-controls-object",
            "--clip-controls", "false",
            "--clip-controls-target-object-name", "frmCustomer",
            "--clip-controls-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(clip_controls_process.exit_code == 0,
        "#1083: host object clip-controls assignment should exit successfully");
    expect(visual_object_property(clip_controls_path, "one-guid", "CLIPCONTROLS") == "false" &&
            visual_object_property(clip_controls_path, "two-guid", "CLIPCONTROLS") == "false" &&
            visual_object_property(clip_controls_path, "three-guid", "CLIPCONTROLS") == "false" &&
            visual_object_property(clip_controls_path, "other-guid", "CLIPCONTROLS") == "true",
        "#1083: host object clip-controls assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_clip_controls(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--clip-controls-object",
            "--clip-controls", "false",
            "--clip-controls-target-unique-id", "one-guid",
            "--clip-controls-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1083: missing-target host object clip-controls assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CLIPCONTROLS") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CLIPCONTROLS") == "true",
        "#1083: missing-target host object clip-controls assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_clip_controls(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--clip-controls-object",
            "--clip-controls", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1083: clip-controls-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CLIPCONTROLS") == "true",
        "#1083: clip-controls-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_clip_controls(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--clip-controls-object",
            "--clip-controls-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1083: clip-controls-object without clip-controls value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CLIPCONTROLS") == "true",
        "#1083: clip-controls-object without clip-controls value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_clip_controls(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--clip-controls-object",
            "--clip-controls", "false",
            "--clip-controls-target-unique-id", "one-guid",
            "--clip-controls-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1083: duplicate-target host object clip-controls assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CLIPCONTROLS") == "true",
        "#1083: duplicate-target host object clip-controls assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_clip_controls(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--clip-controls-object",
            "--auto-size-object",
            "--clip-controls", "false",
            "--clip-controls-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1083: clip-controls-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CLIPCONTROLS") == "true",
        "#1083: clip-controls-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
