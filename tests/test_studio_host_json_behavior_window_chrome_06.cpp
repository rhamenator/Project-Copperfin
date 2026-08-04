// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_panel_link(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PANELLINK", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1091: synthetic SCX table for object panel link should be created");
}

void test_studio_host_json_assigns_panel_link_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_panel_link_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path panel_link_path = temp_root / "panel_link.scx";
    write_synthetic_form_table_for_object_panel_link(panel_link_path);
    const auto panel_link_process = run_process_capture(
        studio_host_path,
        {
            "--path", panel_link_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-object-name", "frmCustomer",
            "--panel-link-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(panel_link_process.exit_code == 0,
        "#1091: host object panel-link assignment should exit successfully");
    expect(visual_object_property(panel_link_path, "one-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "two-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "three-guid", "PANELLINK") == "false" &&
            visual_object_property(panel_link_path, "other-guid", "PANELLINK") == "true",
        "#1091: host object panel-link assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_panel_link(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--panel-link-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1091: missing-target host object panel-link assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "PANELLINK") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "PANELLINK") == "true",
        "#1091: missing-target host object panel-link assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_panel_link(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1091: panel-link-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_panel_link(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--panel-link-object",
            "--panel-link-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1091: panel-link-object without panel-link value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object without panel-link value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_panel_link(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--panel-link-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--panel-link-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1091: duplicate-target host object panel-link assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "PANELLINK") == "true",
        "#1091: duplicate-target host object panel-link assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_panel_link(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--panel-link-object",
            "--auto-size-object",
            "--panel-link", "false",
            "--panel-link-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1091: panel-link-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "PANELLINK") == "true",
        "#1091: panel-link-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_resizable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "RESIZABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1094: synthetic SCX table for object resizable should be created");
}

void test_studio_host_json_assigns_resizable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_resizable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path resizable_path = temp_root / "resizable.scx";
    write_synthetic_form_table_for_object_resizable(resizable_path);
    const auto resizable_process = run_process_capture(
        studio_host_path,
        {
            "--path", resizable_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-object-name", "frmCustomer",
            "--resizable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(resizable_process.exit_code == 0,
        "#1094: host object resizable assignment should exit successfully");
    expect(visual_object_property(resizable_path, "one-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "two-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "three-guid", "RESIZABLE") == "false" &&
            visual_object_property(resizable_path, "other-guid", "RESIZABLE") == "true",
        "#1094: host object resizable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_resizable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--resizable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1094: missing-target host object resizable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "RESIZABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "RESIZABLE") == "true",
        "#1094: missing-target host object resizable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_resizable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1094: resizable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_resizable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--resizable-object",
            "--resizable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1094: resizable-object without resizable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object without resizable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_resizable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--resizable-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--resizable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1094: duplicate-target host object resizable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "RESIZABLE") == "true",
        "#1094: duplicate-target host object resizable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_resizable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--resizable-object",
            "--auto-size-object",
            "--resizable", "false",
            "--resizable-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1094: resizable-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "RESIZABLE") == "true",
        "#1094: resizable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_always_on_top(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALWAYSONTO", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1096: synthetic SCX table for object always on top should be created");
}

void test_studio_host_json_assigns_always_on_top_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_always_on_top_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path always_on_top_path = temp_root / "always_on_top.scx";
    write_synthetic_form_table_for_object_always_on_top(always_on_top_path);
    const auto always_on_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", always_on_top_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-object-name", "frmCustomer",
            "--always-on-top-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(always_on_top_process.exit_code == 0,
        "#1096: host object always-on-top assignment should exit successfully");
    expect(visual_object_property(always_on_top_path, "one-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "two-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "three-guid", "ALWAYSONTOP") == "false" &&
            visual_object_property(always_on_top_path, "other-guid", "ALWAYSONTOP") == "true",
        "#1096: host object always-on-top assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--always-on-top-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1096: missing-target host object always-on-top assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALWAYSONTOP") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALWAYSONTOP") == "true",
        "#1096: missing-target host object always-on-top assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1096: always-on-top-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_always_on_top(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--always-on-top-object",
            "--always-on-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1096: always-on-top-object without always-on-top value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object without always-on-top value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_always_on_top(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--always-on-top-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--always-on-top-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1096: duplicate-target host object always-on-top assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: duplicate-target host object always-on-top assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_always_on_top(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--always-on-top-object",
            "--auto-size-object",
            "--always-on-top", "false",
            "--always-on-top-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1096: always-on-top-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALWAYSONTOP") == "true",
        "#1096: always-on-top-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_always_on_bottom(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "ALWAYSONBO", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1097: synthetic SCX table for object always on bottom should be created");
}

void test_studio_host_json_assigns_always_on_bottom_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_always_on_bottom_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path always_on_bottom_path = temp_root / "always_on_bottom.scx";
    write_synthetic_form_table_for_object_always_on_bottom(always_on_bottom_path);
    const auto always_on_bottom_process = run_process_capture(
        studio_host_path,
        {
            "--path", always_on_bottom_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-object-name", "frmCustomer",
            "--always-on-bottom-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(always_on_bottom_process.exit_code == 0,
        "#1097: host object always-on-bottom assignment should exit successfully");
    expect(visual_object_property(always_on_bottom_path, "one-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "two-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "three-guid", "ALWAYSONBOTTOM") == "false" &&
            visual_object_property(always_on_bottom_path, "other-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: host object always-on-bottom assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--always-on-bottom-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1097: missing-target host object always-on-bottom assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ALWAYSONBOTTOM") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: missing-target host object always-on-bottom assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1097: always-on-bottom-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_always_on_bottom(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1097: always-on-bottom-object without always-on-bottom value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object without always-on-bottom value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_always_on_bottom(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--always-on-bottom-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--always-on-bottom-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1097: duplicate-target host object always-on-bottom assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: duplicate-target host object always-on-bottom assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_always_on_bottom(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--always-on-bottom-object",
            "--auto-size-object",
            "--always-on-bottom", "false",
            "--always-on-bottom-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1097: always-on-bottom-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ALWAYSONBOTTOM") == "true",
        "#1097: always-on-bottom-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
