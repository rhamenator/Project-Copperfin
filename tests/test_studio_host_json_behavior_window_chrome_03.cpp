// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_mac_desktop(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MACDESKTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1149: synthetic SCX table for object mac desktop should be created");
}

void test_studio_host_json_assigns_max_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_button_path = temp_root / "max_button.scx";
    write_synthetic_form_table_for_object_max_button(max_button_path);
    const auto max_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_button_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-object-name", "frmCustomer",
            "--max-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_button_process.exit_code == 0,
        "#1150: host object max-button assignment should exit successfully");
    expect(visual_object_property(max_button_path, "one-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "two-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "three-guid", "MAXBUTTON") == "false" &&
            visual_object_property(max_button_path, "other-guid", "MAXBUTTON") == "true",
        "#1150: host object max-button assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--max-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1150: missing-target host object max-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXBUTTON") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MAXBUTTON") == "true",
        "#1150: missing-target host object max-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1150: max-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-button-object",
            "--max-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1150: max-button-object without max-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object without max-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-button-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--max-button-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1150: duplicate-target host object max-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: duplicate-target host object max-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-button-object",
            "--allow-output-object",
            "--max-button", "false",
            "--max-button-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1150: max-button-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXBUTTON") == "true",
        "#1150: max-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_max_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXBUTTON", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1150: synthetic SCX table for object max button should be created");
}

void test_studio_host_json_assigns_min_button_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_button_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_button_path = temp_root / "min_button.scx";
    write_synthetic_form_table_for_object_min_button(min_button_path);
    const auto min_button_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_button_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-object-name", "frmCustomer",
            "--min-button-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_button_process.exit_code == 0,
        "#1155: host object min-button assignment should exit successfully");
    expect(visual_object_property(min_button_path, "one-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "two-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "three-guid", "MINBUTTON") == "false" &&
            visual_object_property(min_button_path, "other-guid", "MINBUTTON") == "true",
        "#1155: host object min-button assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_button(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--min-button-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1155: missing-target host object min-button assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINBUTTON") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "MINBUTTON") == "true",
        "#1155: missing-target host object min-button assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_button(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1155: min-button-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_button(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-button-object",
            "--min-button-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1155: min-button-object without min-button value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object without min-button value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_button(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-button-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--min-button-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1155: duplicate-target host object min-button assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINBUTTON") == "true",
        "#1155: duplicate-target host object min-button assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_button(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-button-object",
            "--allow-output-object",
            "--min-button", "false",
            "--min-button-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1155: min-button-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINBUTTON") == "true",
        "#1155: min-button-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_min_button(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINBUTTON", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1155: synthetic SCX table for object min button should be created");
}

void test_studio_host_json_assigns_min_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_height_path = temp_root / "min_height.scx";
    write_synthetic_form_table_for_object_min_height(min_height_path);
    const auto min_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_height_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-object-name", "frmCustomer",
            "--min-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_height_process.exit_code == 0,
        "#1156: host object min-height assignment should exit successfully");
    expect(visual_object_property(min_height_path, "one-guid", "MINHEIGHT") == "640" &&
            visual_object_property(min_height_path, "two-guid", "MINHEIGHT") == "640" &&
            visual_object_property(min_height_path, "three-guid", "MINHEIGHT") == "300" &&
            visual_object_property(min_height_path, "other-guid", "MINHEIGHT") == "400",
        "#1156: host object min-height assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--min-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1156: missing-target host object min-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINHEIGHT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MINHEIGHT") == "200",
        "#1156: missing-target host object min-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1156: min-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-height-object",
            "--min-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1156: min-height-object without min-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object without min-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_min_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--min-height-object",
            "--min-height", "-1",
            "--min-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1156: min-height-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-height-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--min-height-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1156: duplicate-target host object min-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: duplicate-target host object min-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-height-object",
            "--allow-output-object",
            "--min-height", "640",
            "--min-height-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1156: min-height-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINHEIGHT") == "100",
        "#1156: min-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_min_height(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINHEIGHT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1156: synthetic SCX table for object min height should be created");
}

void test_studio_host_json_assigns_min_width_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_min_width_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path min_width_path = temp_root / "min_width.scx";
    write_synthetic_form_table_for_object_min_width(min_width_path);
    const auto min_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", min_width_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-object-name", "frmCustomer",
            "--min-width-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(min_width_process.exit_code == 0,
        "#1157: host object min-width assignment should exit successfully");
    expect(visual_object_property(min_width_path, "one-guid", "MINWIDTH") == "640" &&
            visual_object_property(min_width_path, "two-guid", "MINWIDTH") == "640" &&
            visual_object_property(min_width_path, "three-guid", "MINWIDTH") == "300" &&
            visual_object_property(min_width_path, "other-guid", "MINWIDTH") == "400",
        "#1157: host object min-width assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_min_width(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--min-width-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1157: missing-target host object min-width assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MINWIDTH") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MINWIDTH") == "200",
        "#1157: missing-target host object min-width assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_min_width(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1157: min-width-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_min_width(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--min-width-object",
            "--min-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1157: min-width-object without min-width value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object without min-width value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_min_width(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--min-width-object",
            "--min-width", "-1",
            "--min-width-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1157: min-width-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_min_width(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--min-width-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--min-width-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1157: duplicate-target host object min-width assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MINWIDTH") == "100",
        "#1157: duplicate-target host object min-width assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_min_width(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--min-width-object",
            "--allow-output-object",
            "--min-width", "640",
            "--min-width-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1157: min-width-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MINWIDTH") == "100",
        "#1157: min-width-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_min_width(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MINWIDTH", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1157: synthetic SCX table for object min width should be created");
}

void test_studio_host_json_assigns_max_height_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_height_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_height_path = temp_root / "max_height.scx";
    write_synthetic_form_table_for_object_max_height(max_height_path);
    const auto max_height_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_height_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-object-name", "frmCustomer",
            "--max-height-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_height_process.exit_code == 0,
        "#1151: host object max-height assignment should exit successfully");
    expect(visual_object_property(max_height_path, "one-guid", "MAXHEIGHT") == "640" &&
            visual_object_property(max_height_path, "two-guid", "MAXHEIGHT") == "640" &&
            visual_object_property(max_height_path, "three-guid", "MAXHEIGHT") == "300" &&
            visual_object_property(max_height_path, "other-guid", "MAXHEIGHT") == "400",
        "#1151: host object max-height assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_height(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--max-height-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1151: missing-target host object max-height assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXHEIGHT") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXHEIGHT") == "200",
        "#1151: missing-target host object max-height assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_height(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1151: max-height-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_height(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-height-object",
            "--max-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1151: max-height-object without max-height value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object without max-height value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_height(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-height-object",
            "--max-height", "-1",
            "--max-height-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1151: max-height-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_height(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-height-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--max-height-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1151: duplicate-target host object max-height assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: duplicate-target host object max-height assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_height(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-height-object",
            "--allow-output-object",
            "--max-height", "640",
            "--max-height-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1151: max-height-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXHEIGHT") == "100",
        "#1151: max-height-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
