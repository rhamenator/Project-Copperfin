// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_visual_asset_editor_support.h"
#include "../src/vfp/visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_visual_geometry_parsing_uses_invariant_decimal_text() {
    const auto fractional = copperfin::vfp::parse_visual_geometry_number("6666.667");
    expect(fractional.has_value() && std::abs(*fractional - 6666.667) < 0.000001,
           "FRX/LBX native geometry should parse invariant fractional decimal text");

    const auto leading_plus = copperfin::vfp::parse_visual_geometry_number("+1.25");
    expect(leading_plus.has_value() && std::abs(*leading_plus - 1.25) < 0.000001,
           "FRX/LBX native geometry should preserve an optional leading plus");

    expect(!copperfin::vfp::parse_visual_geometry_number("6666,667").has_value() &&
               !copperfin::vfp::parse_visual_geometry_number("6666.667 trailing").has_value(),
           "FRX/LBX native geometry should reject culture-specific separators and trailing input");
}

void test_align_visual_objects_to_anchor_geometry() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_alignment_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "alignment.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdAnchor", "anchorButton", "anchor-guid", "10", "20", "100", "50"},
        {"txtName", "nameBox", "name-guid", "1", "2", "30", "10"},
        {"lblStatus", "statusLabel", "status-guid", "5", "6", "20", "25"},
        {"badGeometry", "badGeometry", "bad-guid", "bad", "8", "20", "10"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#786: alignment fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#786: alignment fixture property should be readable");
        return result.value;
    };

    const auto geometry_state = [&]() {
        return property_value("name-guid", "HPOS") + "," +
            property_value("name-guid", "VPOS") + "," +
            property_value("status-guid", "HPOS") + "," +
            property_value("status-guid", "VPOS");
    };

    auto align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = "txtName", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "left"
    });
    expect(align_result.ok, "#786: align should support left alignment with mixed selectors");
    expect(align_result.affected_object_count == 2U,
        "#999: successful alignment should report affected object count");
    expect(property_value("name-guid", "HPOS") == "10" &&
            property_value("status-guid", "HPOS") == "10" &&
            property_value("name-guid", "VPOS") == "2",
        "#786: left alignment should update HPOS and preserve unrelated VPOS fields");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#786: first successful alignment write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#786: second successful alignment write should remain undo-backed");
    expect(geometry_state() == "1,2,5,6",
        "#786: alignment undo should restore original geometry");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = "cmdAnchor",
        .anchor_unique_id = {},
        .objects = {
            {.record_index = 0U, .object_name = "txtName", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "right"
    });
    expect(align_result.ok, "#786: align should support right alignment by object-name anchor");
    expect(property_value("name-guid", "HPOS") == "80" &&
            property_value("status-guid", "HPOS") == "90",
        "#786: right alignment should account for each selected object width");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 2U, .object_name = {}, .unique_id = {}}
        },
        .mode = "top"
    });
    expect(align_result.ok, "#786: align should support top alignment by record-index targets");
    expect(property_value("name-guid", "VPOS") == "20" &&
            property_value("status-guid", "VPOS") == "20",
        "#786: top alignment should copy anchor VPOS");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "bottom"
    });
    expect(align_result.ok, "#786: align should support bottom alignment");
    expect(property_value("name-guid", "VPOS") == "60" &&
            property_value("status-guid", "VPOS") == "45",
        "#786: bottom alignment should account for each selected object height");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "horizontal-center"
    });
    expect(align_result.ok, "#786: align should support horizontal-center alignment");
    expect(property_value("name-guid", "HPOS") == "45" &&
            property_value("status-guid", "HPOS") == "50",
        "#786: horizontal-center alignment should center each selected object against anchor width");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "vertical-center"
    });
    expect(align_result.ok, "#786: align should support vertical-center alignment");
    expect(property_value("name-guid", "VPOS") == "40" &&
            property_value("status-guid", "VPOS") == "32.5",
        "#786: vertical-center alignment should center each selected object against anchor height");

    const std::string committed_state = geometry_state();
    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .mode = "left"
    });
    expect(!align_result.ok, "#786: align should reject missing selected objects");
    expect(align_result.affected_object_count == 0U,
        "#999: failed alignment should report zero affected objects");
    expect(geometry_state() == committed_state,
        "#786: missing-target alignment failures should leave prior geometry unchanged");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "missing-anchor",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "left"
    });
    expect(!align_result.ok, "#786: align should reject missing anchors");
    expect(geometry_state() == committed_state,
        "#786: missing-anchor failures should leave prior geometry unchanged");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "bad-guid"}
        },
        .mode = "left"
    });
    expect(!align_result.ok, "#786: align should reject non-numeric geometry");
    expect(geometry_state() == committed_state,
        "#786: non-numeric geometry failures should leave prior geometry unchanged");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "diagonal"
    });
    expect(!align_result.ok, "#786: align should reject unsupported alignment modes");
    expect(geometry_state() == committed_state,
        "#786: unsupported-mode failures should leave prior geometry unchanged");

    align_result = copperfin::vfp::align_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {},
        .mode = "left"
    });
    expect(!align_result.ok, "#786: align should reject empty target selections");
    expect(geometry_state() == committed_state,
        "#786: empty-target failures should leave prior geometry unchanged");

    const fs::path incomplete_path = temp_dir / "missing_geometry.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdAnchor", "anchor-guid", "10", "20"},
        {"txtName", "name-guid", "1", "2"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#786: missing-geometry fixture should be writable");

    align_result = copperfin::vfp::align_visual_objects({
        .path = incomplete_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "left"
    });
    expect(!align_result.ok, "#786: align should reject missing geometry fields");

    fs::remove_all(temp_dir, ignored);
}


void test_resize_visual_objects_to_anchor_geometry() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_resize_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "resize.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U},
        {.name = "HEIGHT", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdAnchor", "anchorButton", "anchor-guid", "10", "20", "100", "50"},
        {"txtName", "nameBox", "name-guid", "1", "2", "30", "10"},
        {"lblStatus", "statusLabel", "status-guid", "5", "6", "20", "25"},
        {"badGeometry", "badGeometry", "bad-guid", "7", "8", "bad", "10"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#787: resize fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#787: resize fixture property should be readable");
        return result.value;
    };

    const auto size_state = [&]() {
        return property_value("name-guid", "WIDTH") + "," +
            property_value("name-guid", "HEIGHT") + "," +
            property_value("status-guid", "WIDTH") + "," +
            property_value("status-guid", "HEIGHT");
    };

    auto resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = "txtName", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "width"
    });
    expect(resize_result.ok, "#787: resize should support width mode with mixed selectors");
    expect(resize_result.affected_object_count == 2U,
        "#999: successful resize should report affected object count");
    expect(property_value("name-guid", "WIDTH") == "100" &&
            property_value("status-guid", "WIDTH") == "100" &&
            property_value("name-guid", "HEIGHT") == "10",
        "#787: width resize should copy anchor WIDTH and preserve HEIGHT");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#787: first successful resize write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#787: second successful resize write should remain undo-backed");
    expect(size_state() == "30,10,20,25",
        "#787: resize undo should restore original sizes");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = "cmdAnchor",
        .anchor_unique_id = {},
        .objects = {
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 2U, .object_name = {}, .unique_id = {}}
        },
        .mode = "height"
    });
    expect(resize_result.ok, "#787: resize should support height mode by object-name anchor and record-index targets");
    expect(property_value("name-guid", "HEIGHT") == "50" &&
            property_value("status-guid", "HEIGHT") == "50",
        "#787: height resize should copy anchor HEIGHT");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "status-guid"}
        },
        .mode = "size"
    });
    expect(resize_result.ok, "#787: resize should support size mode");
    expect(size_state() == "100,50,100,50",
        "#787: size resize should copy both anchor WIDTH and HEIGHT");

    const std::string committed_state = size_state();
    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .mode = "width"
    });
    expect(!resize_result.ok, "#787: resize should reject missing selected objects");
    expect(size_state() == committed_state,
        "#787: missing-target resize failures should leave prior sizes unchanged");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "missing-anchor",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "width"
    });
    expect(!resize_result.ok, "#787: resize should reject missing anchors");
    expect(size_state() == committed_state,
        "#787: missing-anchor failures should leave prior sizes unchanged");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "bad-guid"}
        },
        .mode = "width"
    });
    expect(!resize_result.ok, "#787: resize should reject non-numeric geometry");
    expect(size_state() == committed_state,
        "#787: non-numeric resize failures should leave prior sizes unchanged");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "diagonal"
    });
    expect(!resize_result.ok, "#787: resize should reject unsupported resize modes");
    expect(size_state() == committed_state,
        "#787: unsupported-mode resize failures should leave prior sizes unchanged");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = table_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {},
        .mode = "width"
    });
    expect(!resize_result.ok, "#787: resize should reject empty target selections");
    expect(size_state() == committed_state,
        "#787: empty-target resize failures should leave prior sizes unchanged");

    const fs::path incomplete_path = temp_dir / "missing_resize_geometry.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "WIDTH", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdAnchor", "anchor-guid", "10", "20", "100"},
        {"txtName", "name-guid", "1", "2", "30"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#787: missing-resize-geometry fixture should be writable");

    resize_result = copperfin::vfp::resize_visual_objects({
        .path = incomplete_path.string(),
        .anchor_record_index = 0U,
        .anchor_object_name = {},
        .anchor_unique_id = "anchor-guid",
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        },
        .mode = "height"
    });
    expect(!resize_result.ok, "#787: resize should reject missing geometry fields");

    fs::remove_all(temp_dir, ignored);
}


void test_distribute_visual_objects_evenly_by_axis() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_distribute_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "distribute.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdLeft", "leftButton", "left-guid", "0", "0"},
        {"cmdMidA", "midAButton", "mida-guid", "70", "90"},
        {"cmdMidB", "midBButton", "midb-guid", "10", "30"},
        {"cmdRight", "rightButton", "right-guid", "100", "120"},
        {"cmdBad", "badButton", "bad-guid", "bad", "10"},
        {"cmdSameA", "sameAButton", "samea-guid", "5", "5"},
        {"cmdSameB", "sameBButton", "sameb-guid", "5", "5"},
        {"cmdSameC", "sameCButton", "samec-guid", "5", "5"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#790: distribute fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#790: distribute fixture property should be readable");
        return result.value;
    };
    const auto coordinate_state = [&]() {
        return property_value("left-guid", "HPOS") + "," +
            property_value("mida-guid", "HPOS") + "," +
            property_value("midb-guid", "HPOS") + "," +
            property_value("right-guid", "HPOS") + "," +
            property_value("left-guid", "VPOS") + "," +
            property_value("mida-guid", "VPOS") + "," +
            property_value("midb-guid", "VPOS") + "," +
            property_value("right-guid", "VPOS");
    };

    auto distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"},
            {.record_index = 0U, .object_name = "cmdLeft", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "midb-guid"}
        },
        .mode = "horizontal"
    });
    expect(distribute_result.ok,
        "#790: horizontal distribution should support mixed selectors and unsorted inputs");
    expect(distribute_result.affected_object_count == 2U,
        "#999: successful distribution should report affected interior object count");
    expect(property_value("left-guid", "HPOS") == "0" &&
            property_value("midb-guid", "HPOS") == "33.333" &&
            property_value("mida-guid", "HPOS") == "66.667" &&
            property_value("right-guid", "HPOS") == "100",
        "#790: horizontal distribution should space interior HPOS values between endpoints");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#790: first distribution write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#790: second distribution write should remain undo-backed");
    expect(property_value("mida-guid", "HPOS") == "70" &&
            property_value("midb-guid", "HPOS") == "10",
        "#790: distribution undo should restore original horizontal coordinates");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"},
            {.record_index = 0U, .object_name = "cmdLeft", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "midb-guid"}
        },
        .mode = "vertical"
    });
    expect(distribute_result.ok, "#790: vertical distribution should support VPOS");
    expect(property_value("left-guid", "VPOS") == "0" &&
            property_value("midb-guid", "VPOS") == "40" &&
            property_value("mida-guid", "VPOS") == "80" &&
            property_value("right-guid", "VPOS") == "120",
        "#790: vertical distribution should space interior VPOS values between endpoints");

    const std::string committed_state = coordinate_state();
    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "left-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"}
        },
        .mode = "horizontal"
    });
    expect(!distribute_result.ok, "#790: distribution should reject fewer than three selected objects");
    expect(distribute_result.affected_object_count == 0U,
        "#999: failed distribution should report zero affected objects");
    expect(coordinate_state() == committed_state,
        "#790: too-few-object failures should not mutate coordinates");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "left-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"}
        },
        .mode = "horizontal"
    });
    expect(!distribute_result.ok, "#790: distribution should reject missing selected objects");
    expect(coordinate_state() == committed_state,
        "#790: missing-object failures should not mutate coordinates");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "left-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "bad-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"}
        },
        .mode = "horizontal"
    });
    expect(!distribute_result.ok, "#790: distribution should reject non-numeric coordinates");
    expect(coordinate_state() == committed_state,
        "#790: non-numeric coordinate failures should not mutate coordinates");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "samea-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "sameb-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "samec-guid"}
        },
        .mode = "horizontal"
    });
    expect(!distribute_result.ok, "#790: distribution should reject duplicate endpoint coordinates");
    expect(coordinate_state() == committed_state,
        "#790: duplicate-endpoint failures should not mutate coordinates");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "left-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "midb-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "right-guid"}
        },
        .mode = "diagonal"
    });
    expect(!distribute_result.ok, "#790: distribution should reject unsupported modes");
    expect(coordinate_state() == committed_state,
        "#790: unsupported-mode failures should not mutate coordinates");

    const fs::path incomplete_path = temp_dir / "missing_distribute_coordinate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cmdA", "a-guid", "0"},
        {"cmdB", "b-guid", "50"},
        {"cmdC", "c-guid", "100"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#790: missing-coordinate fixture should be writable");

    distribute_result = copperfin::vfp::distribute_visual_objects({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "b-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "c-guid"}
        },
        .mode = "vertical"
    });
    expect(!distribute_result.ok, "#790: distribution should reject missing coordinate fields");

    fs::remove_all(temp_dir, ignored);
}


}  // namespace cf_test_visual_asset_editor
