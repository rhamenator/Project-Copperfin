// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_object_align(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdAnchor", "cmdAnchor", "anchor-guid", "10", "20", "100", "50"},
        {"txtName", "txtName", "name-guid", "1", "2", "30", "10"},
        {"lblStatus", "lblStatus", "status-guid", "5", "6", "20", "25"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1031: synthetic SCX table for object alignment should be created");
}

void write_synthetic_form_table_for_object_distribute(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdLeft", "cmdLeft", "left-guid", "10", "10", "20", "10"},
        {"cmdMiddle", "cmdMiddle", "middle-guid", "90", "50", "20", "10"},
        {"cmdRight", "cmdRight", "right-guid", "110", "90", "20", "10"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1033: synthetic SCX table for object distribution should be created");
}

void write_synthetic_form_table_for_object_snap(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "13.2", "24.9"},
        {"cmdTwo", "cmdTwo", "two-guid", "36", "51"},
        {"cmdOther", "cmdOther", "other-guid", "77", "88"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1034: synthetic SCX table for object snap should be created");
}

void write_synthetic_form_table_for_object_nudge(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "cmdOne", "one-guid", "10", "20"},
        {"cmdTwo", "cmdTwo", "two-guid", "33.5", "44.5"},
        {"cmdOther", "cmdOther", "other-guid", "77", "88"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1035: synthetic SCX table for object nudge should be created");
}

void write_synthetic_form_table_for_object_dynamic_alignment(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DYNAMICALI", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtNotes", "txtNotes", "one-guid", "OLDALIGNONE"},
        {"txtMemo", "txtMemo", "two-guid", "OLDALIGNTWO"},
        {"cntDetails", "cntDetails", "three-guid", "THREEALIGN"},
        {"txtOther", "txtOther", "other-guid", "OTHERALIGN"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1186: synthetic SCX table for object dynamic alignment should be created");
}

void test_studio_host_json_aligns_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_align_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path align_path = temp_root / "align.scx";
    write_synthetic_form_table_for_object_align(align_path);
    const auto align_process = run_process_capture(
        studio_host_path,
        {
            "--path", align_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-object-name", "txtName",
            "--align-target-unique-id", "status-guid",
            "--json"
        },
        temp_root);
    expect(align_process.exit_code == 0,
        "#1031: host object alignment should exit successfully");
    expect(visual_object_property(align_path, "name-guid", "HPOS") == "10" &&
            visual_object_property(align_path, "status-guid", "HPOS") == "10" &&
            visual_object_property(align_path, "name-guid", "VPOS") == "2",
        "#1031: host object alignment should align selected objects and preserve unrelated geometry");

    const fs::path missing_anchor_path = temp_root / "missing_anchor.scx";
    write_synthetic_form_table_for_object_align(missing_anchor_path);
    const auto missing_anchor_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_anchor_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "missing-anchor",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_anchor_process.exit_code == 4,
        "#1031: missing-anchor host object alignment should return command failure");
    expect(visual_object_property(missing_anchor_path, "name-guid", "HPOS") == "1" &&
            visual_object_property(missing_anchor_path, "name-guid", "VPOS") == "2",
        "#1031: missing-anchor host object alignment should not mutate the asset");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_align(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "missing-target",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1031: missing-target host object alignment should return command failure");
    expect(visual_object_property(missing_target_path, "name-guid", "HPOS") == "1" &&
            visual_object_property(missing_target_path, "status-guid", "HPOS") == "5",
        "#1031: missing-target host object alignment should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_align(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--align-object",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1031: align-object without alignment mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "name-guid", "HPOS") == "1",
        "#1031: align-object without alignment mode should not mutate the asset");

    const fs::path missing_targets_path = temp_root / "missing_targets.scx";
    write_synthetic_form_table_for_object_align(missing_targets_path);
    const auto missing_targets_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_targets_path.string(),
            "--align-object",
            "--alignment-mode", "left",
            "--anchor-object-name", "cmdAnchor",
            "--json"
        },
        temp_root);
    expect(missing_targets_process.exit_code == 2,
        "#1031: align-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_targets_path, "name-guid", "HPOS") == "1",
        "#1031: align-object without target selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_align(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--align-object",
            "--group-object",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--align-target-unique-id", "name-guid",
            "--field-value", "OBJNAME=cntGroup",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1031: align-object plus group-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "name-guid", "HPOS") == "1",
        "#1031: align-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_resizes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_resize_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path resize_path = temp_root / "resize.scx";
    write_synthetic_form_table_for_object_align(resize_path);
    const auto resize_process = run_process_capture(
        studio_host_path,
        {
            "--path", resize_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-object-name", "txtName",
            "--resize-target-unique-id", "status-guid",
            "--json"
        },
        temp_root);
    expect(resize_process.exit_code == 0,
        "#1032: host object resize should exit successfully");
    expect(visual_object_property(resize_path, "name-guid", "WIDTH") == "100" &&
            visual_object_property(resize_path, "status-guid", "WIDTH") == "100" &&
            visual_object_property(resize_path, "name-guid", "HEIGHT") == "10",
        "#1032: host object resize should resize selected objects and preserve unrelated geometry");

    const fs::path missing_anchor_path = temp_root / "missing_anchor.scx";
    write_synthetic_form_table_for_object_align(missing_anchor_path);
    const auto missing_anchor_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_anchor_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "missing-anchor",
            "--resize-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_anchor_process.exit_code == 4,
        "#1032: missing-anchor host object resize should return command failure");
    expect(visual_object_property(missing_anchor_path, "name-guid", "WIDTH") == "30" &&
            visual_object_property(missing_anchor_path, "name-guid", "HEIGHT") == "10",
        "#1032: missing-anchor host object resize should not mutate the asset");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_align(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "missing-target",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1032: missing-target host object resize should return command failure");
    expect(visual_object_property(missing_target_path, "name-guid", "WIDTH") == "30" &&
            visual_object_property(missing_target_path, "status-guid", "WIDTH") == "20",
        "#1032: missing-target host object resize should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_align(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--resize-object",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1032: resize-object without resize mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object without resize mode should not mutate the asset");

    const fs::path missing_targets_path = temp_root / "missing_targets.scx";
    write_synthetic_form_table_for_object_align(missing_targets_path);
    const auto missing_targets_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_targets_path.string(),
            "--resize-object",
            "--resize-mode", "width",
            "--anchor-object-name", "cmdAnchor",
            "--json"
        },
        temp_root);
    expect(missing_targets_process.exit_code == 2,
        "#1032: resize-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_targets_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object without target selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_align(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--resize-object",
            "--align-object",
            "--resize-mode", "width",
            "--alignment-mode", "left",
            "--anchor-unique-id", "anchor-guid",
            "--resize-target-unique-id", "name-guid",
            "--align-target-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1032: resize-object plus align-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "name-guid", "WIDTH") == "30",
        "#1032: resize-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_distributes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_distribute_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path distribute_path = temp_root / "distribute.scx";
    write_synthetic_form_table_for_object_distribute(distribute_path);
    const auto distribute_process = run_process_capture(
        studio_host_path,
        {
            "--path", distribute_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-object-name", "cmdLeft",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-object-name", "cmdRight",
            "--json"
        },
        temp_root);
    expect(distribute_process.exit_code == 0,
        "#1033: host object distribution should exit successfully");
    expect(visual_object_property(distribute_path, "left-guid", "HPOS") == "10" &&
            visual_object_property(distribute_path, "middle-guid", "HPOS") == "60" &&
            visual_object_property(distribute_path, "right-guid", "HPOS") == "110",
        "#1033: host object distribution should evenly position the middle object");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_distribute(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "missing-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1033: missing-target host object distribution should return command failure");
    expect(visual_object_property(missing_target_path, "middle-guid", "HPOS") == "90",
        "#1033: missing-target host object distribution should not mutate the asset");

    const fs::path too_few_path = temp_root / "too_few.scx";
    write_synthetic_form_table_for_object_distribute(too_few_path);
    const auto too_few_process = run_process_capture(
        studio_host_path,
        {
            "--path", too_few_path.string(),
            "--distribute-object",
            "--distribution-mode", "horizontal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(too_few_process.exit_code == 4,
        "#1033: too-few-target host object distribution should return command failure");
    expect(visual_object_property(too_few_path, "middle-guid", "HPOS") == "90",
        "#1033: too-few-target host object distribution should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_distribute(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--distribute-object",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1033: distribute-object without distribution mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "middle-guid", "HPOS") == "90",
        "#1033: distribute-object without distribution mode should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_distribute(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--distribute-object",
            "--distribution-mode", "diagonal",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1033: unsupported-mode host object distribution should return command failure");
    expect(visual_object_property(unsupported_mode_path, "middle-guid", "HPOS") == "90",
        "#1033: unsupported-mode host object distribution should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_distribute(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--distribute-object",
            "--resize-object",
            "--distribution-mode", "horizontal",
            "--resize-mode", "width",
            "--anchor-unique-id", "left-guid",
            "--distribute-target-unique-id", "left-guid",
            "--distribute-target-unique-id", "middle-guid",
            "--distribute-target-unique-id", "right-guid",
            "--resize-target-unique-id", "middle-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1033: distribute-object plus resize-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "middle-guid", "HPOS") == "90",
        "#1033: distribute-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_snaps_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_snap_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path snap_path = temp_root / "snap.scx";
    write_synthetic_form_table_for_object_snap(snap_path);
    const auto snap_process = run_process_capture(
        studio_host_path,
        {
            "--path", snap_path.string(),
            "--snap-object",
            "--snap-mode", "both",
            "--grid-width", "10",
            "--grid-height", "25",
            "--snap-target-object-name", "cmdOne",
            "--snap-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(snap_process.exit_code == 0,
        "#1034: host object snap should exit successfully");
    expect(visual_object_property(snap_path, "one-guid", "HPOS") == "10" &&
            visual_object_property(snap_path, "one-guid", "VPOS") == "25" &&
            visual_object_property(snap_path, "two-guid", "HPOS") == "40" &&
            visual_object_property(snap_path, "two-guid", "VPOS") == "50" &&
            visual_object_property(snap_path, "other-guid", "HPOS") == "77",
        "#1034: host object snap should round selected coordinates and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_snap(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--snap-object",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--snap-target-unique-id", "one-guid",
            "--snap-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1034: missing-target host object snap should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HPOS") == "13.2" &&
            visual_object_property(missing_target_path, "two-guid", "HPOS") == "36",
        "#1034: missing-target host object snap should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_snap(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--snap-object",
            "--grid-width", "10",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1034: snap-object without snap mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "one-guid", "HPOS") == "13.2",
        "#1034: snap-object without snap mode should not mutate the asset");

    const fs::path invalid_grid_path = temp_root / "invalid_grid.scx";
    write_synthetic_form_table_for_object_snap(invalid_grid_path);
    const auto invalid_grid_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_grid_path.string(),
            "--snap-object",
            "--snap-mode", "horizontal",
            "--grid-width", "0",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(invalid_grid_process.exit_code == 4,
        "#1034: invalid-grid host object snap should return command failure");
    expect(visual_object_property(invalid_grid_path, "one-guid", "HPOS") == "13.2",
        "#1034: invalid-grid host object snap should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_snap(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--snap-object",
            "--snap-mode", "diagonal",
            "--grid-width", "10",
            "--grid-height", "25",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1034: unsupported-mode host object snap should return command failure");
    expect(visual_object_property(unsupported_mode_path, "one-guid", "HPOS") == "13.2",
        "#1034: unsupported-mode host object snap should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_snap(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--snap-object",
            "--distribute-object",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--distribution-mode", "horizontal",
            "--snap-target-unique-id", "one-guid",
            "--distribute-target-unique-id", "one-guid",
            "--distribute-target-unique-id", "two-guid",
            "--distribute-target-unique-id", "other-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1034: snap-object plus distribute-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HPOS") == "13.2",
        "#1034: snap-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_nudges_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_nudge_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path nudge_path = temp_root / "nudge.scx";
    write_synthetic_form_table_for_object_nudge(nudge_path);
    const auto nudge_process = run_process_capture(
        studio_host_path,
        {
            "--path", nudge_path.string(),
            "--nudge-object",
            "--nudge-mode", "both",
            "--delta-hpos", "5",
            "--delta-vpos", "-2.5",
            "--nudge-target-object-name", "cmdOne",
            "--nudge-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(nudge_process.exit_code == 0,
        "#1035: host object nudge should exit successfully");
    expect(visual_object_property(nudge_path, "one-guid", "HPOS") == "15" &&
            visual_object_property(nudge_path, "one-guid", "VPOS") == "17.5" &&
            visual_object_property(nudge_path, "two-guid", "HPOS") == "38.5" &&
            visual_object_property(nudge_path, "two-guid", "VPOS") == "42" &&
            visual_object_property(nudge_path, "other-guid", "HPOS") == "77",
        "#1035: host object nudge should move selected coordinates and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_nudge(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--nudge-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--nudge-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1035: missing-target host object nudge should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "HPOS") == "10" &&
            visual_object_property(missing_target_path, "two-guid", "HPOS") == "33.5",
        "#1035: missing-target host object nudge should not mutate the asset");

    const fs::path missing_mode_path = temp_root / "missing_mode.scx";
    write_synthetic_form_table_for_object_nudge(missing_mode_path);
    const auto missing_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_mode_path.string(),
            "--nudge-object",
            "--delta-hpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_mode_process.exit_code == 2,
        "#1035: nudge-object without nudge mode should fail during launch parsing");
    expect(visual_object_property(missing_mode_path, "one-guid", "HPOS") == "10",
        "#1035: nudge-object without nudge mode should not mutate the asset");

    const fs::path zero_delta_path = temp_root / "zero_delta.scx";
    write_synthetic_form_table_for_object_nudge(zero_delta_path);
    const auto zero_delta_process = run_process_capture(
        studio_host_path,
        {
            "--path", zero_delta_path.string(),
            "--nudge-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "0",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(zero_delta_process.exit_code == 4,
        "#1035: zero-delta host object nudge should return command failure");
    expect(visual_object_property(zero_delta_path, "one-guid", "HPOS") == "10",
        "#1035: zero-delta host object nudge should not mutate the asset");

    const fs::path unsupported_mode_path = temp_root / "unsupported_mode.scx";
    write_synthetic_form_table_for_object_nudge(unsupported_mode_path);
    const auto unsupported_mode_process = run_process_capture(
        studio_host_path,
        {
            "--path", unsupported_mode_path.string(),
            "--nudge-object",
            "--nudge-mode", "diagonal",
            "--delta-hpos", "1",
            "--delta-vpos", "1",
            "--nudge-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_mode_process.exit_code == 4,
        "#1035: unsupported-mode host object nudge should return command failure");
    expect(visual_object_property(unsupported_mode_path, "one-guid", "HPOS") == "10",
        "#1035: unsupported-mode host object nudge should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_nudge(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--nudge-object",
            "--snap-object",
            "--nudge-mode", "horizontal",
            "--delta-hpos", "1",
            "--snap-mode", "horizontal",
            "--grid-width", "10",
            "--nudge-target-unique-id", "one-guid",
            "--snap-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1035: nudge-object plus snap-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "HPOS") == "10",
        "#1035: nudge-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_dynamic_alignment_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_dynamic_alignment_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string expression = "IIF(.T., 2, 0)";
    const fs::path dynamic_alignment_path = temp_root / "dynamic_alignment.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(dynamic_alignment_path);
    const auto dynamic_alignment_process = run_process_capture(
        studio_host_path,
        {
            "--path", dynamic_alignment_path.string(),
            "--dynamic-alignment-object",
            "--dynamic-alignment", expression,
            "--dynamic-alignment-target-object-name", "txtNotes",
            "--dynamic-alignment-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(dynamic_alignment_process.exit_code == 0,
        "#1186: host object dynamic-alignment assignment should exit successfully");
    expect(visual_object_property(dynamic_alignment_path, "one-guid", "DYNAMICALIGNMENT") == expression &&
            visual_object_property(dynamic_alignment_path, "two-guid", "DYNAMICALIGNMENT") == expression &&
            visual_object_property(dynamic_alignment_path, "three-guid", "DYNAMICALIGNMENT") == "THREEALIGN" &&
            visual_object_property(dynamic_alignment_path, "other-guid", "DYNAMICALIGNMENT") == "OTHERALIGN",
        "#1186: host object dynamic-alignment assignment should assign raw expression text and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--dynamic-alignment-object",
            "--dynamic-alignment", expression,
            "--dynamic-alignment-target-unique-id", "one-guid",
            "--dynamic-alignment-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1186: missing-target host object dynamic-alignment assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DYNAMICALIGNMENT") == "OLDALIGNONE" &&
            visual_object_property(missing_target_path, "two-guid", "DYNAMICALIGNMENT") == "OLDALIGNTWO",
        "#1186: missing-target host object dynamic-alignment assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--dynamic-alignment-object",
            "--dynamic-alignment", expression,
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1186: dynamic-alignment-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DYNAMICALIGNMENT") == "OLDALIGNONE",
        "#1186: dynamic-alignment-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--dynamic-alignment-object",
            "--dynamic-alignment-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1186: dynamic-alignment-object without dynamic-alignment value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DYNAMICALIGNMENT") == "OLDALIGNONE",
        "#1186: dynamic-alignment-object without dynamic-alignment value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--dynamic-alignment-object",
            "--dynamic-alignment", expression,
            "--dynamic-alignment-target-unique-id", "one-guid",
            "--dynamic-alignment-target-object-name", "txtNotes",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1186: duplicate-target host object dynamic-alignment assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DYNAMICALIGNMENT") == "OLDALIGNONE",
        "#1186: duplicate-target host object dynamic-alignment assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_dynamic_alignment(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--dynamic-alignment-object",
            "--allow-output-object",
            "--dynamic-alignment", expression,
            "--dynamic-alignment-target-unique-id", "one-guid",
            "--allow-output", "false",
            "--allow-output-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1186: dynamic-alignment-object plus allow-output-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DYNAMICALIGNMENT") == "OLDALIGNONE",
        "#1186: dynamic-alignment-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_layout_actions <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_aligns_objects_by_stable_selectors(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_resizes_objects_by_stable_selectors(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_distributes_objects_by_stable_selectors(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_snaps_objects_by_stable_selectors(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_nudges_objects_by_stable_selectors(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_assigns_dynamic_alignment_by_stable_selectors(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
