// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_drag_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DRAGMODE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1104: synthetic SCX table for object drag-mode should be created");
}

void write_synthetic_form_table_for_object_ole_drag_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "OLEDRAGMOD", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1105: synthetic SCX table for object OLE drag-mode should be created");
}

void write_synthetic_form_table_for_object_ole_drop_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "OLEDROPMOD", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1106: synthetic SCX table for object OLE drop-mode should be created");
}

void write_synthetic_form_table_for_object_ole_drop_effects(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "OLEDROPEFF", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1107: synthetic SCX table for object OLE drop-effects should be created");
}

void write_synthetic_form_table_for_object_ole_drop_text_insertion(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "OLEDROPTEX", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1108: synthetic SCX table for object OLE drop text-insertion should be created");
}

void write_synthetic_form_table_for_object_buffer_mode(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BUFFERMODE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1117: synthetic SCX table for object buffer-mode should be created");
}

void write_synthetic_form_table_for_object_buffer_mode_override(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "BUFFERMODE", .type = 'N', .length = 3U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "one-guid", "0"},
        {"cmdCancel", "cmdCancel", "two-guid", "1"},
        {"lblStatus", "lblStatus", "three-guid", "2"},
        {"cmdOther", "cmdOther", "other-guid", "0"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1118: synthetic SCX table for object buffer-mode-override should be created");
}

void test_studio_host_json_assigns_drag_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_drag_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path drag_mode_path = temp_root / "drag_mode.scx";
    write_synthetic_form_table_for_object_drag_mode(drag_mode_path);
    const auto drag_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", drag_mode_path.string(),
            "--drag-mode-object",
            "--drag-mode", "3",
            "--drag-mode-target-object-name", "cmdSave",
            "--drag-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(drag_mode_process.exit_code == 0,
        "#1104: host object drag-mode assignment should exit successfully");
    expect(visual_object_property(drag_mode_path, "one-guid", "DRAGMODE") == "3" &&
            visual_object_property(drag_mode_path, "two-guid", "DRAGMODE") == "3" &&
            visual_object_property(drag_mode_path, "three-guid", "DRAGMODE") == "2" &&
            visual_object_property(drag_mode_path, "other-guid", "DRAGMODE") == "0",
        "#1104: host object drag-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_drag_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--drag-mode-object",
            "--drag-mode", "2",
            "--drag-mode-target-unique-id", "one-guid",
            "--drag-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1104: missing-target host object drag-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DRAGMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "DRAGMODE") == "1",
        "#1104: missing-target host object drag-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_drag_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--drag-mode-object",
            "--drag-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1104: drag-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DRAGMODE") == "0",
        "#1104: drag-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_drag_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--drag-mode-object",
            "--drag-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1104: drag-mode-object without drag-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DRAGMODE") == "0",
        "#1104: drag-mode-object without drag-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_drag_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--drag-mode-object",
            "--drag-mode", "-1",
            "--drag-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1104: negative drag-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "DRAGMODE") == "0",
        "#1104: negative drag-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_drag_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--drag-mode-object",
            "--drag-mode", "2",
            "--drag-mode-target-unique-id", "one-guid",
            "--drag-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1104: duplicate-target host object drag-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DRAGMODE") == "0",
        "#1104: duplicate-target host object drag-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_drag_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--drag-mode-object",
            "--locked-object",
            "--drag-mode", "2",
            "--drag-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1104: drag-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DRAGMODE") == "0",
        "#1104: drag-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_ole_drag_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ole_drag_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path ole_drag_mode_path = temp_root / "ole_drag_mode.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(ole_drag_mode_path);
    const auto ole_drag_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", ole_drag_mode_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode", "3",
            "--ole-drag-mode-target-object-name", "cmdSave",
            "--ole-drag-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(ole_drag_mode_process.exit_code == 0,
        "#1105: host object OLE drag-mode assignment should exit successfully");
    expect(visual_object_property(ole_drag_mode_path, "one-guid", "OLEDRAGMODE") == "3" &&
            visual_object_property(ole_drag_mode_path, "two-guid", "OLEDRAGMODE") == "3" &&
            visual_object_property(ole_drag_mode_path, "three-guid", "OLEDRAGMODE") == "2" &&
            visual_object_property(ole_drag_mode_path, "other-guid", "OLEDRAGMODE") == "0",
        "#1105: host object OLE drag-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode", "2",
            "--ole-drag-mode-target-unique-id", "one-guid",
            "--ole-drag-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1105: missing-target host object OLE drag-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "OLEDRAGMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "OLEDRAGMODE") == "1",
        "#1105: missing-target host object OLE drag-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1105: OLE drag-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "OLEDRAGMODE") == "0",
        "#1105: OLE drag-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1105: OLE drag-mode-object without OLE drag-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "OLEDRAGMODE") == "0",
        "#1105: OLE drag-mode-object without OLE drag-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode", "-1",
            "--ole-drag-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1105: negative OLE drag-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "OLEDRAGMODE") == "0",
        "#1105: negative OLE drag-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--ole-drag-mode-object",
            "--ole-drag-mode", "2",
            "--ole-drag-mode-target-unique-id", "one-guid",
            "--ole-drag-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1105: duplicate-target host object OLE drag-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "OLEDRAGMODE") == "0",
        "#1105: duplicate-target host object OLE drag-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ole_drag_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ole-drag-mode-object",
            "--locked-object",
            "--ole-drag-mode", "2",
            "--ole-drag-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1105: OLE drag-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "OLEDRAGMODE") == "0",
        "#1105: OLE drag-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_ole_drop_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ole_drop_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path ole_drop_mode_path = temp_root / "ole_drop_mode.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(ole_drop_mode_path);
    const auto ole_drop_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", ole_drop_mode_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode", "3",
            "--ole-drop-mode-target-object-name", "cmdSave",
            "--ole-drop-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(ole_drop_mode_process.exit_code == 0,
        "#1106: host object OLE drop-mode assignment should exit successfully");
    expect(visual_object_property(ole_drop_mode_path, "one-guid", "OLEDROPMODE") == "3" &&
            visual_object_property(ole_drop_mode_path, "two-guid", "OLEDROPMODE") == "3" &&
            visual_object_property(ole_drop_mode_path, "three-guid", "OLEDROPMODE") == "2" &&
            visual_object_property(ole_drop_mode_path, "other-guid", "OLEDROPMODE") == "0",
        "#1106: host object OLE drop-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode", "2",
            "--ole-drop-mode-target-unique-id", "one-guid",
            "--ole-drop-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1106: missing-target host object OLE drop-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "OLEDROPMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "OLEDROPMODE") == "1",
        "#1106: missing-target host object OLE drop-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1106: OLE drop-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "OLEDROPMODE") == "0",
        "#1106: OLE drop-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1106: OLE drop-mode-object without OLE drop-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "OLEDROPMODE") == "0",
        "#1106: OLE drop-mode-object without OLE drop-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode", "-1",
            "--ole-drop-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1106: negative OLE drop-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "OLEDROPMODE") == "0",
        "#1106: negative OLE drop-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--ole-drop-mode-object",
            "--ole-drop-mode", "2",
            "--ole-drop-mode-target-unique-id", "one-guid",
            "--ole-drop-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1106: duplicate-target host object OLE drop-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "OLEDROPMODE") == "0",
        "#1106: duplicate-target host object OLE drop-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ole_drop_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ole-drop-mode-object",
            "--locked-object",
            "--ole-drop-mode", "2",
            "--ole-drop-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1106: OLE drop-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "OLEDROPMODE") == "0",
        "#1106: OLE drop-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_ole_drop_effects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ole_drop_effects_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path ole_drop_effects_path = temp_root / "ole_drop_effects.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(ole_drop_effects_path);
    const auto ole_drop_effects_process = run_process_capture(
        studio_host_path,
        {
            "--path", ole_drop_effects_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects", "3",
            "--ole-drop-effects-target-object-name", "cmdSave",
            "--ole-drop-effects-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(ole_drop_effects_process.exit_code == 0,
        "#1107: host object OLE drop-effects assignment should exit successfully");
    expect(visual_object_property(ole_drop_effects_path, "one-guid", "OLEDROPEFFECTS") == "3" &&
            visual_object_property(ole_drop_effects_path, "two-guid", "OLEDROPEFFECTS") == "3" &&
            visual_object_property(ole_drop_effects_path, "three-guid", "OLEDROPEFFECTS") == "2" &&
            visual_object_property(ole_drop_effects_path, "other-guid", "OLEDROPEFFECTS") == "0",
        "#1107: host object OLE drop-effects assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects", "2",
            "--ole-drop-effects-target-unique-id", "one-guid",
            "--ole-drop-effects-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1107: missing-target host object OLE drop-effects assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "OLEDROPEFFECTS") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "OLEDROPEFFECTS") == "1",
        "#1107: missing-target host object OLE drop-effects assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1107: OLE drop-effects-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "OLEDROPEFFECTS") == "0",
        "#1107: OLE drop-effects-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1107: OLE drop-effects-object without OLE drop-effects value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "OLEDROPEFFECTS") == "0",
        "#1107: OLE drop-effects-object without OLE drop-effects value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects", "-1",
            "--ole-drop-effects-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1107: negative OLE drop-effects values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "OLEDROPEFFECTS") == "0",
        "#1107: negative OLE drop-effects values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--ole-drop-effects-object",
            "--ole-drop-effects", "2",
            "--ole-drop-effects-target-unique-id", "one-guid",
            "--ole-drop-effects-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1107: duplicate-target host object OLE drop-effects assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "OLEDROPEFFECTS") == "0",
        "#1107: duplicate-target host object OLE drop-effects assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ole_drop_effects(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ole-drop-effects-object",
            "--locked-object",
            "--ole-drop-effects", "2",
            "--ole-drop-effects-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1107: OLE drop-effects-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "OLEDROPEFFECTS") == "0",
        "#1107: OLE drop-effects-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_ole_drop_text_insertion_by_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_ole_drop_text_insertion_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path ole_drop_text_insertion_path = temp_root / "ole_drop_text_insertion.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(ole_drop_text_insertion_path);
    const auto ole_drop_text_insertion_process = run_process_capture(
        studio_host_path,
        {
            "--path", ole_drop_text_insertion_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "3",
            "--ole-drop-text-insertion-target-object-name", "cmdSave",
            "--ole-drop-text-insertion-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(ole_drop_text_insertion_process.exit_code == 0,
        "#1108: host object OLE drop text-insertion assignment should exit successfully");
    expect(visual_object_property(ole_drop_text_insertion_path, "one-guid", "OLEDROPTEXTINSERTION") == "3" &&
            visual_object_property(ole_drop_text_insertion_path, "two-guid", "OLEDROPTEXTINSERTION") == "3" &&
            visual_object_property(ole_drop_text_insertion_path, "three-guid", "OLEDROPTEXTINSERTION") == "2" &&
            visual_object_property(ole_drop_text_insertion_path, "other-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: host object OLE drop text-insertion assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "2",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--ole-drop-text-insertion-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1108: missing-target host object OLE drop text-insertion assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "OLEDROPTEXTINSERTION") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "OLEDROPTEXTINSERTION") == "1",
        "#1108: missing-target host object OLE drop text-insertion assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1108: OLE drop text-insertion-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: OLE drop text-insertion-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1108: OLE drop text-insertion-object without OLE drop text-insertion value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: OLE drop text-insertion-object without OLE drop text-insertion value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "-1",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1108: negative OLE drop text-insertion values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: negative OLE drop text-insertion values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--ole-drop-text-insertion-object",
            "--ole-drop-text-insertion", "2",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--ole-drop-text-insertion-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1108: duplicate-target host object OLE drop text-insertion assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: duplicate-target host object OLE drop text-insertion assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ole_drop_text_insertion(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ole-drop-text-insertion-object",
            "--locked-object",
            "--ole-drop-text-insertion", "2",
            "--ole-drop-text-insertion-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1108: OLE drop text-insertion-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "OLEDROPTEXTINSERTION") == "0",
        "#1108: OLE drop text-insertion-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_buffer_mode_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_buffer_mode_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path buffer_mode_path = temp_root / "buffer_mode.scx";
    write_synthetic_form_table_for_object_buffer_mode(buffer_mode_path);
    const auto buffer_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", buffer_mode_path.string(),
            "--buffer-mode-object",
            "--buffer-mode", "9",
            "--buffer-mode-target-object-name", "cmdSave",
            "--buffer-mode-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(buffer_mode_process.exit_code == 0,
        "#1117: host object buffer-mode assignment should exit successfully");
    expect(visual_object_property(buffer_mode_path, "one-guid", "BUFFERMODE") == "9" &&
            visual_object_property(buffer_mode_path, "two-guid", "BUFFERMODE") == "9" &&
            visual_object_property(buffer_mode_path, "three-guid", "BUFFERMODE") == "2" &&
            visual_object_property(buffer_mode_path, "other-guid", "BUFFERMODE") == "0",
        "#1117: host object buffer-mode assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_buffer_mode(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--buffer-mode-object",
            "--buffer-mode", "2",
            "--buffer-mode-target-unique-id", "one-guid",
            "--buffer-mode-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1117: missing-target host object buffer-mode assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BUFFERMODE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BUFFERMODE") == "1",
        "#1117: missing-target host object buffer-mode assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_buffer_mode(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--buffer-mode-object",
            "--buffer-mode", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1117: buffer-mode-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BUFFERMODE") == "0",
        "#1117: buffer-mode-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_buffer_mode(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--buffer-mode-object",
            "--buffer-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1117: buffer-mode-object without buffer-mode value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BUFFERMODE") == "0",
        "#1117: buffer-mode-object without buffer-mode value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_buffer_mode(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--buffer-mode-object",
            "--buffer-mode", "-1",
            "--buffer-mode-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1117: negative buffer-mode values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BUFFERMODE") == "0",
        "#1117: negative buffer-mode values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_buffer_mode(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--buffer-mode-object",
            "--buffer-mode", "2",
            "--buffer-mode-target-unique-id", "one-guid",
            "--buffer-mode-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1117: duplicate-target host object buffer-mode assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BUFFERMODE") == "0",
        "#1117: duplicate-target host object buffer-mode assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_buffer_mode(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--buffer-mode-object",
            "--locked-object",
            "--buffer-mode", "2",
            "--buffer-mode-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1117: buffer-mode-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BUFFERMODE") == "0",
        "#1117: buffer-mode-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_buffer_mode_override_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_buffer_mode_override_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path buffer_mode_override_path = temp_root / "buffer_mode_override.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(buffer_mode_override_path);
    const auto buffer_mode_override_process = run_process_capture(
        studio_host_path,
        {
            "--path", buffer_mode_override_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override", "9",
            "--buffer-mode-override-target-object-name", "cmdSave",
            "--buffer-mode-override-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(buffer_mode_override_process.exit_code == 0,
        "#1118: host object buffer-mode-override assignment should exit successfully");
    expect(visual_object_property(buffer_mode_override_path, "one-guid", "BUFFERMODEOVERRIDE") == "9" &&
            visual_object_property(buffer_mode_override_path, "two-guid", "BUFFERMODEOVERRIDE") == "9" &&
            visual_object_property(buffer_mode_override_path, "three-guid", "BUFFERMODEOVERRIDE") == "2" &&
            visual_object_property(buffer_mode_override_path, "other-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: host object buffer-mode-override assignment should assign selected values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override", "2",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--buffer-mode-override-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1118: missing-target host object buffer-mode-override assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BUFFERMODEOVERRIDE") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "BUFFERMODEOVERRIDE") == "1",
        "#1118: missing-target host object buffer-mode-override assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override", "2",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1118: buffer-mode-override-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: buffer-mode-override-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1118: buffer-mode-override-object without buffer-mode-override value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: buffer-mode-override-object without buffer-mode-override value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override", "-1",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1118: negative buffer-mode-override values should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: negative buffer-mode-override values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--buffer-mode-override-object",
            "--buffer-mode-override", "2",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--buffer-mode-override-target-object-name", "cmdSave",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1118: duplicate-target host object buffer-mode-override assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: duplicate-target host object buffer-mode-override assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_buffer_mode_override(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--buffer-mode-override-object",
            "--locked-object",
            "--buffer-mode-override", "2",
            "--buffer-mode-override-target-unique-id", "one-guid",
            "--locked", "true",
            "--locked-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1118: buffer-mode-override-object plus locked-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BUFFERMODEOVERRIDE") == "0",
        "#1118: buffer-mode-override-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
