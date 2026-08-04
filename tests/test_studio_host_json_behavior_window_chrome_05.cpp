// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_max_left(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXLEFT", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1153: synthetic SCX table for object max left should be created");
}

void test_studio_host_json_assigns_max_top_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_max_top_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path max_top_path = temp_root / "max_top.scx";
    write_synthetic_form_table_for_object_max_top(max_top_path);
    const auto max_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", max_top_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-object-name", "frmCustomer",
            "--max-top-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(max_top_process.exit_code == 0,
        "#1154: host object max-top assignment should exit successfully");
    expect(visual_object_property(max_top_path, "one-guid", "MAXTOP") == "640" &&
            visual_object_property(max_top_path, "two-guid", "MAXTOP") == "640" &&
            visual_object_property(max_top_path, "three-guid", "MAXTOP") == "300" &&
            visual_object_property(max_top_path, "other-guid", "MAXTOP") == "400",
        "#1154: host object max-top assignment should assign selected numeric value and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_max_top(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--max-top-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1154: missing-target host object max-top assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "MAXTOP") == "100" &&
            visual_object_property(missing_target_path, "two-guid", "MAXTOP") == "200",
        "#1154: missing-target host object max-top assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_max_top(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1154: max-top-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_max_top(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--max-top-object",
            "--max-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1154: max-top-object without max-top value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object without max-top value should not mutate the asset");

    const fs::path negative_value_path = temp_root / "negative_value.scx";
    write_synthetic_form_table_for_object_max_top(negative_value_path);
    const auto negative_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_value_path.string(),
            "--max-top-object",
            "--max-top", "-1",
            "--max-top-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_value_process.exit_code == 2,
        "#1154: max-top-object with negative value should fail during launch parsing");
    expect(visual_object_property(negative_value_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object with negative value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_max_top(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--max-top-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--max-top-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1154: duplicate-target host object max-top assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "MAXTOP") == "100",
        "#1154: duplicate-target host object max-top assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_max_top(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--max-top-object",
            "--allow-output-object",
            "--max-top", "640",
            "--max-top-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1154: max-top-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "MAXTOP") == "100",
        "#1154: max-top-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_max_top(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "MAXTOP", .type = 'N', .length = 10U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", "100"},
        {"frmOrder", "frmOrder", "two-guid", "200"},
        {"cntDetails", "cntDetails", "three-guid", "300"},
        {"frmOther", "frmOther", "other-guid", "400"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1154: synthetic SCX table for object max top should be created");
}

void test_studio_host_json_assigns_auto_center_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_auto_center_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path auto_center_path = temp_root / "auto_center.scx";
    write_synthetic_form_table_for_object_auto_center(auto_center_path);
    const auto auto_center_process = run_process_capture(
        studio_host_path,
        {
            "--path", auto_center_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-object-name", "frmCustomer",
            "--auto-center-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(auto_center_process.exit_code == 0,
        "#1078: host object auto-center assignment should exit successfully");
    expect(visual_object_property(auto_center_path, "one-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "two-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "three-guid", "AUTOCENTER") == "false" &&
            visual_object_property(auto_center_path, "other-guid", "AUTOCENTER") == "true",
        "#1078: host object auto-center assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_auto_center(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--auto-center-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1078: missing-target host object auto-center assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "AUTOCENTER") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "AUTOCENTER") == "true",
        "#1078: missing-target host object auto-center assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_auto_center(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1078: auto-center-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_auto_center(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--auto-center-object",
            "--auto-center-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1078: auto-center-object without auto-center value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object without auto-center value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_auto_center(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--auto-center-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--auto-center-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1078: duplicate-target host object auto-center assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: duplicate-target host object auto-center assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_auto_center(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--auto-center-object",
            "--allow-output-object",
            "--auto-center", "false",
            "--auto-center-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1078: auto-center-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "AUTOCENTER") == "true",
        "#1078: auto-center-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_dockable(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DOCKABLE", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1082: synthetic SCX table for object dockable should be created");
}

void test_studio_host_json_assigns_dockable_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dockable_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path dockable_path = temp_root / "dockable.scx";
    write_synthetic_form_table_for_object_dockable(dockable_path);
    const auto dockable_process = run_process_capture(
        studio_host_path,
        {
            "--path", dockable_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-object-name", "frmCustomer",
            "--dockable-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dockable_process.exit_code == 0,
        "#1082: host object dockable assignment should exit successfully");
    expect(visual_object_property(dockable_path, "one-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "two-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "three-guid", "DOCKABLE") == "false" &&
            visual_object_property(dockable_path, "other-guid", "DOCKABLE") == "true",
        "#1082: host object dockable assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dockable(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--dockable-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1082: missing-target host object dockable assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DOCKABLE") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "DOCKABLE") == "true",
        "#1082: missing-target host object dockable assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dockable(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1082: dockable-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dockable(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dockable-object",
            "--dockable-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1082: dockable-object without dockable value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object without dockable value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dockable(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dockable-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--dockable-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1082: duplicate-target host object dockable assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DOCKABLE") == "true",
        "#1082: duplicate-target host object dockable assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dockable(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dockable-object",
            "--auto-size-object",
            "--dockable", "false",
            "--dockable-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1082: dockable-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DOCKABLE") == "true",
        "#1082: dockable-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_lock_screen(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "LOCKSCREEN", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1085: synthetic SCX table for object lock screen should be created");
}

void test_studio_host_json_assigns_lock_screen_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_lock_screen_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path lock_screen_path = temp_root / "lock_screen.scx";
    write_synthetic_form_table_for_object_lock_screen(lock_screen_path);
    const auto lock_screen_process = run_process_capture(
        studio_host_path,
        {
            "--path", lock_screen_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-object-name", "frmCustomer",
            "--lock-screen-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(lock_screen_process.exit_code == 0,
        "#1085: host object lock-screen assignment should exit successfully");
    expect(visual_object_property(lock_screen_path, "one-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "two-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "three-guid", "LOCKSCREEN") == "false" &&
            visual_object_property(lock_screen_path, "other-guid", "LOCKSCREEN") == "true",
        "#1085: host object lock-screen assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--lock-screen-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1085: missing-target host object lock-screen assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LOCKSCREEN") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "LOCKSCREEN") == "true",
        "#1085: missing-target host object lock-screen assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1085: lock-screen-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_lock_screen(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--lock-screen-object",
            "--lock-screen-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1085: lock-screen-object without lock-screen value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object without lock-screen value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_lock_screen(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--lock-screen-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--lock-screen-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1085: duplicate-target host object lock-screen assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: duplicate-target host object lock-screen assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_lock_screen(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--lock-screen-object",
            "--auto-size-object",
            "--lock-screen", "false",
            "--lock-screen-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1085: lock-screen-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LOCKSCREEN") == "true",
        "#1085: lock-screen-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void write_synthetic_form_table_for_object_split_bar(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "SPLITBAR", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1089: synthetic SCX table for object split bar should be created");
}

void test_studio_host_json_assigns_split_bar_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_split_bar_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path split_bar_path = temp_root / "split_bar.scx";
    write_synthetic_form_table_for_object_split_bar(split_bar_path);
    const auto split_bar_process = run_process_capture(
        studio_host_path,
        {
            "--path", split_bar_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-object-name", "frmCustomer",
            "--split-bar-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(split_bar_process.exit_code == 0,
        "#1089: host object split-bar assignment should exit successfully");
    expect(visual_object_property(split_bar_path, "one-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "two-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "three-guid", "SPLITBAR") == "false" &&
            visual_object_property(split_bar_path, "other-guid", "SPLITBAR") == "true",
        "#1089: host object split-bar assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_split_bar(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--split-bar-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1089: missing-target host object split-bar assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "SPLITBAR") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "SPLITBAR") == "true",
        "#1089: missing-target host object split-bar assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_split_bar(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1089: split-bar-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_split_bar(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--split-bar-object",
            "--split-bar-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1089: split-bar-object without split-bar value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object without split-bar value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_split_bar(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--split-bar-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--split-bar-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1089: duplicate-target host object split-bar assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "SPLITBAR") == "true",
        "#1089: duplicate-target host object split-bar assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_split_bar(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--split-bar-object",
            "--auto-size-object",
            "--split-bar", "false",
            "--split-bar-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1089: split-bar-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "SPLITBAR") == "true",
        "#1089: split-bar-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
