// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_snap_visual_objects_to_grid_by_axis() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_snap_grid_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "snap_grid.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "13.2", "24.9"},
        {"cmdTwo", "twoButton", "two-guid", "36", "51"},
        {"cmdOther", "otherButton", "other-guid", "77", "88"},
        {"cmdBad", "badButton", "bad-guid", "bad", "12"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#791: snap-grid fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#791: snap-grid fixture property should be readable");
        return result.value;
    };
    const auto coordinate_state = [&]() {
        return property_value("one-guid", "HPOS") + "," +
            property_value("one-guid", "VPOS") + "," +
            property_value("two-guid", "HPOS") + "," +
            property_value("two-guid", "VPOS") + "," +
            property_value("other-guid", "HPOS") + "," +
            property_value("other-guid", "VPOS");
    };

    auto snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .mode = "horizontal",
        .grid_width = 10.0,
        .grid_height = 0.0
    });
    expect(snap_result.ok, "#791: horizontal snap should support mixed selectors");
    expect(snap_result.affected_object_count == 2U,
        "#999: successful snap-to-grid should report affected object count");
    expect(property_value("one-guid", "HPOS") == "10" &&
            property_value("two-guid", "HPOS") == "40" &&
            property_value("one-guid", "VPOS") == "24.9" &&
            property_value("other-guid", "HPOS") == "77",
        "#791: horizontal snap should round HPOS and preserve VPOS plus unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#791: first horizontal snap write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#791: second horizontal snap write should remain undo-backed");
    expect(property_value("one-guid", "HPOS") == "13.2" &&
            property_value("two-guid", "HPOS") == "36",
        "#791: snap-grid undo should restore original horizontal coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "vertical",
        .grid_width = 0.0,
        .grid_height = 25.0
    });
    expect(snap_result.ok, "#791: vertical snap should support record-index and UNIQUEID selectors");
    expect(property_value("one-guid", "VPOS") == "25" &&
            property_value("two-guid", "VPOS") == "50" &&
            property_value("one-guid", "HPOS") == "13.2",
        "#791: vertical snap should round VPOS and preserve HPOS");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .mode = "both",
        .grid_width = 10.0,
        .grid_height = 25.0
    });
    expect(snap_result.ok, "#791: both-axis snap should support HPOS and VPOS together");
    expect(property_value("one-guid", "HPOS") == "10" &&
            property_value("one-guid", "VPOS") == "25" &&
            property_value("two-guid", "HPOS") == "40" &&
            property_value("two-guid", "VPOS") == "50" &&
            property_value("other-guid", "HPOS") == "77" &&
            property_value("other-guid", "VPOS") == "88",
        "#791: both-axis snap should round both coordinates and preserve unrelated objects");

    const std::string committed_state = coordinate_state();
    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {},
        .mode = "both",
        .grid_width = 10.0,
        .grid_height = 10.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject empty selections");
    expect(snap_result.affected_object_count == 0U,
        "#999: failed snap-to-grid should report zero affected objects");
    expect(coordinate_state() == committed_state,
        "#791: empty-selection failures should not mutate coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .mode = "horizontal",
        .grid_width = 10.0,
        .grid_height = 0.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject missing selected objects");
    expect(coordinate_state() == committed_state,
        "#791: missing-object failures should not mutate coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "bad-guid"}
        },
        .mode = "horizontal",
        .grid_width = 10.0,
        .grid_height = 0.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject non-numeric coordinates");
    expect(coordinate_state() == committed_state,
        "#791: non-numeric coordinate failures should not mutate coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "diagonal",
        .grid_width = 10.0,
        .grid_height = 10.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject unsupported modes");
    expect(coordinate_state() == committed_state,
        "#791: unsupported-mode failures should not mutate coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "horizontal",
        .grid_width = 0.0,
        .grid_height = 10.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject non-positive horizontal increments");
    expect(coordinate_state() == committed_state,
        "#791: invalid-horizontal-grid failures should not mutate coordinates");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "vertical",
        .grid_width = 10.0,
        .grid_height = -1.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject non-positive vertical increments");
    expect(coordinate_state() == committed_state,
        "#791: invalid-vertical-grid failures should not mutate coordinates");

    const fs::path incomplete_path = temp_dir / "missing_snap_coordinate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid", "12"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#791: missing-coordinate fixture should be writable");

    snap_result = copperfin::vfp::snap_visual_objects_to_grid({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .mode = "vertical",
        .grid_width = 10.0,
        .grid_height = 10.0
    });
    expect(!snap_result.ok, "#791: snap-grid should reject missing coordinate fields");

    fs::remove_all(temp_dir, ignored);
}


void test_nudge_visual_objects_by_delta() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_nudge_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "nudge.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdOne", "oneButton", "one-guid", "10", "20"},
        {"cmdTwo", "twoButton", "two-guid", "33.5", "44.5"},
        {"cmdOther", "otherButton", "other-guid", "77", "88"},
        {"cmdBad", "badButton", "bad-guid", "bad", "12"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#792: nudge fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#792: nudge fixture property should be readable");
        return result.value;
    };
    const auto coordinate_state = [&]() {
        return property_value("one-guid", "HPOS") + "," +
            property_value("one-guid", "VPOS") + "," +
            property_value("two-guid", "HPOS") + "," +
            property_value("two-guid", "VPOS") + "," +
            property_value("other-guid", "HPOS") + "," +
            property_value("other-guid", "VPOS");
    };

    auto nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cmdOne", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .mode = "horizontal",
        .delta_hpos = 5.0,
        .delta_vpos = 0.0
    });
    expect(nudge_result.ok, "#792: horizontal nudge should support mixed selectors");
    expect(nudge_result.affected_object_count == 2U,
        "#999: successful nudge should report affected object count");
    expect(property_value("one-guid", "HPOS") == "15" &&
            property_value("two-guid", "HPOS") == "38.5" &&
            property_value("one-guid", "VPOS") == "20" &&
            property_value("other-guid", "HPOS") == "77",
        "#792: horizontal nudge should move HPOS and preserve VPOS plus unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#792: first horizontal nudge write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#792: second horizontal nudge write should remain undo-backed");
    expect(property_value("one-guid", "HPOS") == "10" &&
            property_value("two-guid", "HPOS") == "33.5",
        "#792: nudge undo should restore original horizontal coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "vertical",
        .delta_hpos = 0.0,
        .delta_vpos = -4.5
    });
    expect(nudge_result.ok, "#792: vertical nudge should support record-index and UNIQUEID selectors");
    expect(property_value("one-guid", "VPOS") == "15.5" &&
            property_value("two-guid", "VPOS") == "40" &&
            property_value("one-guid", "HPOS") == "10",
        "#792: vertical nudge should move VPOS and preserve HPOS");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        .mode = "both",
        .delta_hpos = -2.5,
        .delta_vpos = 10.0
    });
    expect(nudge_result.ok, "#792: both-axis nudge should support HPOS and VPOS together");
    expect(property_value("one-guid", "HPOS") == "7.5" &&
            property_value("one-guid", "VPOS") == "25.5" &&
            property_value("two-guid", "HPOS") == "31" &&
            property_value("two-guid", "VPOS") == "50" &&
            property_value("other-guid", "HPOS") == "77" &&
            property_value("other-guid", "VPOS") == "88",
        "#792: both-axis nudge should move both coordinates and preserve unrelated objects");

    const std::string committed_state = coordinate_state();
    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {},
        .mode = "both",
        .delta_hpos = 1.0,
        .delta_vpos = 1.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject empty selections");
    expect(nudge_result.affected_object_count == 0U,
        "#999: failed nudge should report zero affected objects");
    expect(coordinate_state() == committed_state,
        "#792: empty-selection failures should not mutate coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .mode = "horizontal",
        .delta_hpos = 1.0,
        .delta_vpos = 0.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject missing selected objects");
    expect(coordinate_state() == committed_state,
        "#792: missing-object failures should not mutate coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "bad-guid"}
        },
        .mode = "horizontal",
        .delta_hpos = 1.0,
        .delta_vpos = 0.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject non-numeric coordinates");
    expect(coordinate_state() == committed_state,
        "#792: non-numeric coordinate failures should not mutate coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "diagonal",
        .delta_hpos = 1.0,
        .delta_vpos = 1.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject unsupported modes");
    expect(coordinate_state() == committed_state,
        "#792: unsupported-mode failures should not mutate coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "horizontal",
        .delta_hpos = 0.0,
        .delta_vpos = 1.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject zero horizontal movement");
    expect(coordinate_state() == committed_state,
        "#792: zero-horizontal-delta failures should not mutate coordinates");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        .mode = "both",
        .delta_hpos = 1.0,
        .delta_vpos = 0.0
    });
    expect(!nudge_result.ok, "#792: both-axis nudge should reject zero vertical movement");
    expect(coordinate_state() == committed_state,
        "#792: zero-vertical-delta failures should not mutate coordinates");

    const fs::path incomplete_path = temp_dir / "missing_nudge_coordinate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid", "12"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#792: missing-coordinate fixture should be writable");

    nudge_result = copperfin::vfp::nudge_visual_objects({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .mode = "vertical",
        .delta_hpos = 0.0,
        .delta_vpos = 1.0
    });
    expect(!nudge_result.ok, "#792: nudge should reject missing coordinate fields");

    fs::remove_all(temp_dir, ignored);
}


}  // namespace cf_test_visual_asset_editor
