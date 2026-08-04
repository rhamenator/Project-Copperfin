// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_closable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CLOSABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1073: synthetic SCX table for object closable should be created");
}

void test_studio_host_json_assigns_closable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_closable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path closable_path = temp_root / "closable.scx";
    write_synthetic_form_table_for_object_closable(closable_path);
    const auto closable_process = run_process_capture(
        studio_host_path,
        {
            "--path", closable_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-object-name", "frmCustomer",
            "--closable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(closable_process.exit_code == 0,
        "#1073: host object closable assignment should exit successfully");
    expect(visual_object_property(closable_path, "one-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "two-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "three-guid", "CLOSABLE") == "false" &&
            visual_object_property(closable_path, "other-guid", "CLOSABLE") == "true",
        "#1073: host object closable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_closable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--closable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1073: missing-target host object closable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CLOSABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CLOSABLE") == "true",
        "#1073: missing-target host object closable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_closable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--closable-object",
            "--closable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1073: closable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_closable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--closable-object",
            "--closable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1073: closable-object without closable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object without closable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_closable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--closable-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--closable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1073: duplicate-target host object closable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CLOSABLE") == "true",
        "#1073: duplicate-target host object closable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_closable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--closable-object",
            "--dynamic-fore-color-object",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--dynamic-fore-color", "RGB(9,9,9)",
            "--dynamic-fore-color-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1073: closable-object plus dynamic-fore-color-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CLOSABLE") == "true",
        "#1073: closable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_control_box(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "CONTROLBOX", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1074: synthetic SCX table for object control box should be created");
}

void test_studio_host_json_assigns_control_box_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_control_box_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path control_box_path = temp_root / "control_box.scx";
    write_synthetic_form_table_for_object_control_box(control_box_path);
    const auto control_box_process = run_process_capture(
        studio_host_path,
        {
            "--path", control_box_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-object-name", "frmCustomer",
            "--control-box-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(control_box_process.exit_code == 0,
        "#1074: host object control-box assignment should exit successfully");
    expect(visual_object_property(control_box_path, "one-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "two-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "three-guid", "CONTROLBOX") == "false" &&
            visual_object_property(control_box_path, "other-guid", "CONTROLBOX") == "true",
        "#1074: host object control-box assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_control_box(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--control-box-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1074: missing-target host object control-box assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "CONTROLBOX") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "CONTROLBOX") == "true",
        "#1074: missing-target host object control-box assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_control_box(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1074: control-box-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_control_box(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--control-box-object",
            "--control-box-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1074: control-box-object without control-box value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object without control-box value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_control_box(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--control-box-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--control-box-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1074: duplicate-target host object control-box assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: duplicate-target host object control-box assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_control_box(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--control-box-object",
            "--closable-object",
            "--control-box", "false",
            "--control-box-target-unique-id", "one-guid",
            "--closable", "false",
            "--closable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1074: control-box-object plus closable-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "CONTROLBOX") == "true",
        "#1074: control-box-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_auto_center(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "AUTOCENTER", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1078: synthetic SCX table for object auto center should be created");
}

void test_studio_host_json_assigns_desktop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_desktop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path desktop_path = temp_root / "desktop.scx";
    write_synthetic_form_table_for_object_desktop(desktop_path);
    const auto desktop_process = run_process_capture(
        studio_host_path,
        {
            "--path", desktop_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-object-name", "frmCustomer",
            "--desktop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(desktop_process.exit_code == 0,
        "#1147: host object desktop assignment should exit successfully");
    expect(visual_object_property(desktop_path, "one-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "two-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "three-guid", "DESKTOP") == "false" &&
            visual_object_property(desktop_path, "other-guid", "DESKTOP") == "true",
        "#1147: host object desktop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_desktop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--desktop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1147: missing-target host object desktop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DESKTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "DESKTOP") == "true",
        "#1147: missing-target host object desktop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_desktop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1147: desktop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_desktop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--desktop-object",
            "--desktop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1147: desktop-object without desktop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object without desktop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_desktop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--desktop-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--desktop-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1147: duplicate-target host object desktop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DESKTOP") == "true",
        "#1147: duplicate-target host object desktop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_desktop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--desktop-object",
            "--allow-output-object",
            "--desktop", "false",
            "--desktop-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1147: desktop-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DESKTOP") == "true",
        "#1147: desktop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_desktop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DESKTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1147: synthetic SCX table for object desktop should be created");
}

void test_studio_host_json_assigns_key_preview_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_key_preview_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path key_preview_path = temp_root / "key_preview.scx";
    write_synthetic_form_table_for_object_key_preview(key_preview_path);
    const auto key_preview_process = run_process_capture(
        studio_host_path,
        {
            "--path", key_preview_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-object-name", "frmCustomer",
            "--key-preview-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(key_preview_process.exit_code == 0,
        "#1148: host object key-preview assignment should exit successfully");
    expect(visual_object_property(key_preview_path, "one-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "two-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "three-guid", "KEYPREVIEW") == "false" &&
            visual_object_property(key_preview_path, "other-guid", "KEYPREVIEW") == "true",
        "#1148: host object key-preview assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_key_preview(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--key-preview-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1148: missing-target host object key-preview assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "KEYPREVIEW") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "KEYPREVIEW") == "true",
        "#1148: missing-target host object key-preview assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_key_preview(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1148: key-preview-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_key_preview(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--key-preview-object",
            "--key-preview-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1148: key-preview-object without key-preview value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object without key-preview value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_key_preview(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--key-preview-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--key-preview-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1148: duplicate-target host object key-preview assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: duplicate-target host object key-preview assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_key_preview(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--key-preview-object",
            "--allow-output-object",
            "--key-preview", "false",
            "--key-preview-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1148: key-preview-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "KEYPREVIEW") == "true",
        "#1148: key-preview-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_key_preview(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "KEYPREVIEW", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1148: synthetic SCX table for object key preview should be created");
}

void test_studio_host_json_assigns_mac_desktop_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_mac_desktop_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mac_desktop_path = temp_root / "mac_desktop.scx";
    write_synthetic_form_table_for_object_mac_desktop(mac_desktop_path);
    const auto mac_desktop_process = run_process_capture(
        studio_host_path,
        {
            "--path", mac_desktop_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-object-name", "frmCustomer",
            "--mac-desktop-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(mac_desktop_process.exit_code == 0,
        "#1149: host object mac-desktop assignment should exit successfully");
    expect(visual_object_property(mac_desktop_path, "one-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "two-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "three-guid", "MACDESKTOP") == "false" &&
            visual_object_property(mac_desktop_path, "other-guid", "MACDESKTOP") == "true",
        "#1149: host object mac-desktop assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--mac-desktop-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1149: missing-target host object mac-desktop assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MACDESKTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MACDESKTOP") == "true",
        "#1149: missing-target host object mac-desktop assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1149: mac-desktop-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_mac_desktop(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--mac-desktop-object",
            "--mac-desktop-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1149: mac-desktop-object without mac-desktop value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object without mac-desktop value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_mac_desktop(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--mac-desktop-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--mac-desktop-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1149: duplicate-target host object mac-desktop assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: duplicate-target host object mac-desktop assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_mac_desktop(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--mac-desktop-object",
            "--allow-output-object",
            "--mac-desktop", "false",
            "--mac-desktop-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1149: mac-desktop-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MACDESKTOP") == "true",
        "#1149: mac-desktop-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
