#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
const copperfin::vfp::VisualObjectPropertySnapshot* find_property_snapshot(
    const std::vector<copperfin::vfp::VisualObjectPropertySnapshot>& properties,
    const std::string& property_name) {
    const auto value = std::find_if(properties.begin(), properties.end(), [&](const auto& candidate) {
        return candidate.property_name == property_name;
    });
    return value == properties.end() ? nullptr : &(*value);
}

const copperfin::vfp::VisualObjectMethodSnapshot* find_method_snapshot(
    const std::vector<copperfin::vfp::VisualObjectMethodSnapshot>& methods,
    const std::string& method_name) {
    const auto value = std::find_if(methods.begin(), methods.end(), [&](const auto& candidate) {
        return candidate.method_name == method_name;
    });
    return value == methods.end() ? nullptr : &(*value);
}

void test_rename_visual_object_memo_property_updates_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#769: property-rename fixture should be writable");

    auto rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "caption",
        .new_property_name = "DisplayCaption"
    });
    expect(rename_result.ok, "#769: property rename should support UNIQUEID selection and case-insensitive source matching");
    expect(rename_result.affected_object_count == 1U,
        "#1005: successful property rename should report one affected object");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "Top",
        .new_property_name = "TopOffset"
    });
    expect(rename_result.ok, "#769: property rename should support object-name selection");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Left",
        .new_property_name = "LeftOffset"
    });
    expect(rename_result.ok, "#769: property rename should support record-index selection");

    auto display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    auto old_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    auto top_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "TopOffset"
    });
    auto unrelated_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Caption"
    });
    expect(display_caption.ok && display_caption.exists && display_caption.value == "\"Save\"" &&
            old_caption.ok && !old_caption.exists &&
            left_offset.ok && left_offset.exists && left_offset.value == "10" &&
            top_offset.ok && top_offset.exists && top_offset.value == "30" &&
            unrelated_caption.ok && unrelated_caption.exists && unrelated_caption.value == "\"Name\"",
        "#769: property rename should preserve values and unrelated assignments while removing old assignment names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS",
        .new_property_name = "HPosition"
    });
    expect(!rename_result.ok, "#769: property rename should reject direct DBF-backed fields");
    expect(rename_result.affected_object_count == 0U,
        "#1005: failed property rename should report zero affected objects");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "MissingProp",
        .new_property_name = "MissingRenamed"
    });
    expect(!rename_result.ok, "#769: property rename should reject missing source properties");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = "LeftOffset"
    });
    expect(!rename_result.ok, "#769: property rename should reject target-name collisions");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = "displaycaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject same-name renames case-insensitively");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " ",
        .new_property_name = "EmptySource"
    });
    expect(!rename_result.ok, "#769: property rename should reject empty source names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption",
        .new_property_name = " "
    });
    expect(!rename_result.ok, "#769: property rename should reject empty target names");

    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "caption",
        .new_property_name = "DuplicateCaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject duplicate source assignments");

    display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    auto duplicate_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "DuplicateCaption"
    });
    expect(display_caption.ok && display_caption.exists && display_caption.value == "\"Save\"" &&
            left_offset.ok && left_offset.exists && left_offset.value == "10" &&
            duplicate_caption.ok && !duplicate_caption.exists,
        "#769: rejected property renames should not mutate selected object properties");

    const fs::path no_properties_path = temp_dir / "property_rename_missing_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> no_properties_records{
        {"cmdSave", "save-guid", "111"}
    };
    const auto no_properties_create = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(no_properties_create.ok, "#769: missing-PROPERTIES fixture should be writable");
    rename_result = copperfin::vfp::rename_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .new_property_name = "DisplayCaption"
    });
    expect(!rename_result.ok, "#769: property rename should reject objects without PROPERTIES memo fields");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#769: undo should restore each successful memo property rename");
    }

    display_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "DisplayCaption"
    });
    old_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    left_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "LeftOffset"
    });
    top_offset = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "TopOffset"
    });
    auto original_top = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Top"
    });
    expect(display_caption.ok && !display_caption.exists &&
            old_caption.ok && old_caption.exists && old_caption.value == "\"Save\"" &&
            left_offset.ok && !left_offset.exists &&
            top_offset.ok && !top_offset.exists &&
            original_top.ok && original_top.exists && original_top.value == "30",
        "#769: undo should restore original memo property names and values");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_memo_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_rename_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_rename_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#772: property-rename-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "caption",
                .new_property_name = "DisplayCaption"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .property_name = "Top",
                .new_property_name = "TopOffset"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .property_name = "Caption",
                .new_property_name = "StatusCaption"
            }
        }
    });
    expect(batch_result.ok, "#772: batch property renames should support mixed selectors");
    expect(batch_result.affected_object_count == 3U,
        "#1005: successful batch property rename should report affected item count");

    auto display_caption = property_state("save-guid", "DisplayCaption");
    auto old_caption = property_state("save-guid", "Caption");
    auto top_offset = property_state("name-guid", "TopOffset");
    auto status_caption = property_state("status-guid", "StatusCaption");
    auto save_left = property_state("save-guid", "Left");
    expect(display_caption.ok && display_caption.exists && display_caption.value == "\"Save\"" &&
            old_caption.ok && !old_caption.exists &&
            top_offset.ok && top_offset.exists && top_offset.value == "30" &&
            status_caption.ok && status_caption.exists && status_caption.value == "\"Status\"" &&
            save_left.ok && save_left.exists && save_left.value == "10",
        "#772: batch property renames should preserve values, target names, and unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#772: successful batch renames should leave normal visual undo history available");

    batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "Left",
                .new_property_name = "LeftOffset"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "HPOS",
                .new_property_name = "HPosition"
            }
        }
    });
    expect(!batch_result.ok, "#772: batch property renames should reject direct DBF-backed fields");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property rename should report zero affected objects");
    save_left = property_state("save-guid", "Left");
    auto left_offset = property_state("save-guid", "LeftOffset");
    expect(save_left.ok && save_left.exists && save_left.value == "10" &&
            left_offset.ok && !left_offset.exists,
        "#772: direct-field failures should roll back earlier memo renames");

    batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Left",
                .new_property_name = "StatusLeft"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "DisplayCaption",
                .new_property_name = "Left"
            }
        }
    });
    expect(!batch_result.ok, "#772: batch property renames should reject target collisions");
    auto status_left = property_state("status-guid", "Left");
    auto status_left_renamed = property_state("status-guid", "StatusLeft");
    expect(status_left.ok && status_left.exists && status_left.value == "50" &&
            status_left_renamed.ok && !status_left_renamed.exists,
        "#772: target-collision failures should roll back earlier memo renames");

    batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Left",
                .new_property_name = "StatusLeft"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = " ",
                .new_property_name = "EmptySource"
            }
        }
    });
    expect(!batch_result.ok, "#772: batch property renames should reject empty source names");
    status_left = property_state("status-guid", "Left");
    expect(status_left.ok && status_left.exists && status_left.value == "50",
        "#772: empty-source failures should roll back earlier memo renames");

    batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Left",
                .new_property_name = "StatusLeft"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "DisplayCaption",
                .new_property_name = " "
            }
        }
    });
    expect(!batch_result.ok, "#772: batch property renames should reject empty target names");
    status_left = property_state("status-guid", "Left");
    expect(status_left.ok && status_left.exists && status_left.value == "50",
        "#772: empty-target failures should roll back earlier memo renames");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#772: failed batch rename rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::rename_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#772: empty batch rename requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property rename should report zero affected objects");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#772: undo should restore each successful batch property rename");
    }

    display_caption = property_state("save-guid", "DisplayCaption");
    old_caption = property_state("save-guid", "Caption");
    top_offset = property_state("name-guid", "TopOffset");
    auto original_top = property_state("name-guid", "Top");
    status_caption = property_state("status-guid", "StatusCaption");
    auto original_status_caption = property_state("status-guid", "Caption");
    expect(display_caption.ok && !display_caption.exists &&
            old_caption.ok && old_caption.exists && old_caption.value == "\"Save\"" &&
            top_offset.ok && !top_offset.exists &&
            original_top.ok && original_top.exists && original_top.value == "30" &&
            status_caption.ok && !status_caption.exists &&
            original_status_caption.ok && original_status_caption.exists && original_status_caption.value == "\"Status\"",
        "#772: successful batch rename undo should restore original memo property names and values");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_memo_properties_within_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\nTop = 20\r\nWidth = 80\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\n"},
        {"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#770: property-reorder fixture should be writable");

    const auto memo_property_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto properties = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!properties.ok) {
            return names;
        }
        for (const auto& property : properties.properties) {
            if (!property.direct_field) {
                names.push_back(property.property_name);
            }
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "width",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support UNIQUEID selection and first placement");
    expect(reorder_result.affected_object_count == 1U,
        "#1005: successful property reorder should report one affected object");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}),
        "#770: first placement should move the requested memo property to the start");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .property_name = "WIDTH",
        .placement = "last",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support object-name selection and last placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: last placement should move the requested memo property to the end");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "Top"
    });
    expect(reorder_result.ok, "#770: property reorder should support record-index selection and after placement");
    expect(order_is(memo_property_order("save-guid"), {"Left", "Top", "Caption", "Width"}),
        "#770: after placement should move the requested memo property after the relative property");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "Left"
    });
    expect(reorder_result.ok, "#770: property reorder should support before placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: before placement should move the requested memo property before the relative property");

    auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto left = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Left"
    });
    auto width = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Width"
    });
    expect(caption.ok && caption.exists && caption.value == "\"Save\"" &&
            left.ok && left.exists && left.value == "10" &&
            width.ok && width.exists && width.value == "80",
        "#770: property reorder should preserve memo property names and values");
    expect(order_is(memo_property_order("name-guid"), {"Caption", "Left"}),
        "#770: property reorder should preserve unrelated object PROPERTIES memos");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed source fields");
    expect(reorder_result.affected_object_count == 0U,
        "#1005: failed property reorder should report zero affected objects");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "HPOS"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed relative fields");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Missing",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing source properties");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing relative properties for before placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject self-relative before/after placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "middle",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject unknown placements");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " ",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject empty source names");

    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: failed property reorders should not mutate the PROPERTIES memo");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#770: undo should restore each successful property reorder");
    }
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: undo should restore original memo property ordering");

    const fs::path duplicate_path = temp_dir / "property_reorder_duplicate.scx";
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        {{"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}});
    expect(duplicate_create_result.ok, "#770: duplicate property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "CAPTION",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate source assignments as ambiguous");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "Anchor",
        .placement = "before",
        .relative_property_name = "Caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate relative assignments as ambiguous");

    const fs::path no_properties_path = temp_dir / "property_reorder_no_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const auto no_properties_create_result = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        {{"cmdNoProps", "no-props-guid"}});
    expect(no_properties_create_result.ok, "#770: missing-PROPERTIES property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing PROPERTIES fields");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_memo_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_reorder_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_reorder_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\nTop = 20\r\nWidth = 80\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\nTop = 40\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "Caption = \"Status\"\r\nLeft = 50\r\nWidth = 90\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#773: property-reorder-batch fixture should be writable");

    const auto memo_property_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto properties = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!properties.ok) {
            return names;
        }
        for (const auto& property : properties.properties) {
            if (!property.direct_field) {
                names.push_back(property.property_name);
            }
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };
    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .property_name = "Caption",
                .placement = "after",
                .relative_property_name = "Left"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .property_name = "Left",
                .placement = "before",
                .relative_property_name = "Caption"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "last",
                .relative_property_name = {}
            }
        }
    });
    expect(batch_result.ok, "#773: batch property reorder should support mixed selectors and all placements");
    expect(batch_result.affected_object_count == 4U,
        "#1005: successful batch property reorder should report affected item count");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}) &&
            order_is(memo_property_order("name-guid"), {"Left", "Caption", "Top"}) &&
            order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: batch property reorder should persist expected memo assignment ordering");

    const auto caption = property_state("save-guid", "Caption");
    const auto width = property_state("save-guid", "Width");
    expect(caption.ok && caption.exists && caption.value == "\"Save\"" &&
            width.ok && width.exists && width.value == "80",
        "#773: batch property reorder should preserve assignment names and values");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#773: successful batch reorders should leave normal visual undo history available");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "Top",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "HPOS",
                .placement = "first",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject direct DBF-backed fields");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property reorder should report zero affected objects");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}),
        "#773: direct-field failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .property_name = "Top",
                .placement = "before",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject missing relative names");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: missing-relative failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = " ",
                .placement = "last",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject empty property names");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: empty-name failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "Top",
                .placement = "middle",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject unknown placements");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: unknown-placement failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "after",
                .relative_property_name = "width"
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject self-relative placement");

    const fs::path duplicate_path = temp_dir / "property_reorder_batch_duplicate.scx";
    const auto duplicate_create = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        {{"dupObj", "dupName", "dup-guid", "111", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}});
    expect(duplicate_create.ok, "#773: duplicate property-reorder-batch fixture should be writable");
    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = duplicate_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .property_name = "Anchor",
                .placement = "before",
                .relative_property_name = "Caption"
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject duplicate relative assignments");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#773: failed batch reorder rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#773: empty batch reorder requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property reorder should report zero affected objects");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#773: undo should restore each successful batch property reorder");
    }
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}) &&
            order_is(memo_property_order("name-guid"), {"Caption", "Left", "Top"}) &&
            order_is(memo_property_order("status-guid"), {"Caption", "Left", "Width"}),
        "#773: successful batch reorder undo should restore original memo property order");

    fs::remove_all(temp_dir, ignored);
}

void test_duplicate_visual_object_appends_identity_safe_copy() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_duplicate_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "duplicate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\nLeft = 12\r\n",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#749: duplicate fixture should be writable");

    auto duplicate_result = copperfin::vfp::duplicate_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .new_object_name = "cmdSaveCopy",
        .new_name = "saveButtonCopy",
        .new_unique_id = "save-copy-guid"
    });
    expect(duplicate_result.ok && duplicate_result.record_index == 2U,
        "#749: selected-object duplication should append a live copy at the next record index");
    expect(duplicate_result.object_name == "cmdSaveCopy" &&
            duplicate_result.unique_id == "save-copy-guid" &&
            duplicate_result.parent_name == "frmMain",
        "#993: selected-object duplication should report duplicated object identity metadata");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#749: duplication should append exactly one visual object row");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdSave" &&
                list_result.objects[0].unique_id == "save-guid",
            "#749: duplication should preserve the original selected object");
        expect(!list_result.objects[2].deleted &&
                list_result.objects[2].object_name == "cmdSaveCopy" &&
                list_result.objects[2].unique_id == "save-copy-guid" &&
                list_result.objects[2].parent_name == "frmMain" &&
                list_result.objects[2].class_name == "commandbutton" &&
                list_result.objects[2].baseclass_name == "commandbutton" &&
                list_result.objects[2].caption == "\"Save\"",
            "#749: duplicated visual objects should expose replacement identity and preserved metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "12",
        "#749: duplicated visual objects should preserve memo-backed properties");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#749: duplicated visual objects should preserve METHODS memo content");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#749: duplicate fixture should support marking an existing object deleted");

    duplicate_result = copperfin::vfp::duplicate_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .new_object_name = "cmdOther",
        .new_name = "otherButton",
        .new_unique_id = "name-guid"
    });
    expect(!duplicate_result.ok,
        "#749: duplicate identity checks should reject collisions with deleted records");
    expect(duplicate_result.object_name.empty() &&
            duplicate_result.unique_id.empty() &&
            duplicate_result.parent_name.empty(),
        "#993: failed duplicate requests should not report stale identity metadata after collisions");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U && list_result.objects[1].deleted,
        "#749: failed duplicate requests should not mutate object count or deleted flags");

    duplicate_result = copperfin::vfp::duplicate_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .new_object_name = "txtNameCopy",
        .new_name = "nameBoxCopy",
        .new_unique_id = "name-copy-guid"
    });
    expect(duplicate_result.ok && duplicate_result.record_index == 3U,
        "#1784: duplicating a deleted visual object should append the copied record");
    expect(duplicate_result.object_name == "txtNameCopy" &&
            duplicate_result.unique_id == "name-copy-guid" &&
            duplicate_result.parent_name == "frmMain",
        "#1784: deleted visual object duplicate should report replacement identity metadata");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 4U &&
            list_result.objects[3].deleted &&
            list_result.objects[3].object_name == "txtNameCopy" &&
            list_result.objects[3].unique_id == "name-copy-guid",
        "#1784: deleted visual object duplicate should preserve deleted state and replacement identity");

    fs::remove_all(temp_dir, ignored);
}

void test_duplicate_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_duplicate_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch_duplicate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\nLeft = 12\r\n",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "frmMain",
            "label",
            "label",
            "Caption = \"Status\"\r\nLeft = 36\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#785: batch duplicate fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#785: batch duplicate fixture should support deleted-row collision setup");

    const auto object_count = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#785: batch-duplicate fixture should remain listable");
        return list_result.objects.size();
    };

    auto batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = "cmdSave",
                .unique_id = {},
                .new_object_name = "cmdSaveCopy",
                .new_name = "saveButtonCopy",
                .new_unique_id = "save-copy-guid"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .new_object_name = "lblStatusCopy",
                .new_name = "statusLabelCopy",
                .new_unique_id = "status-copy-guid"
            }
        }
    });
    expect(batch_result.ok && batch_result.record_indexes.size() == 2U &&
            batch_result.record_indexes[0] == 3U && batch_result.record_indexes[1] == 4U,
        "#785: batch duplicate should append each copy and return duplicate record indexes");
    expect(batch_result.duplicated_objects.size() == 2U &&
            batch_result.duplicated_objects[0].record_index == 3U &&
            batch_result.duplicated_objects[0].object_name == "cmdSaveCopy" &&
            batch_result.duplicated_objects[0].unique_id == "save-copy-guid" &&
            batch_result.duplicated_objects[0].parent_name == "frmMain" &&
            batch_result.duplicated_objects[1].record_index == 4U &&
            batch_result.duplicated_objects[1].object_name == "lblStatusCopy" &&
            batch_result.duplicated_objects[1].unique_id == "status-copy-guid" &&
            batch_result.duplicated_objects[1].parent_name == "frmMain",
        "#994: batch duplicate should report duplicated object identity metadata in append order");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 5U,
        "#785: batch duplicate should append all requested copies");
    if (list_result.ok && list_result.objects.size() == 5U) {
        expect(list_result.objects[1].deleted && list_result.objects[1].unique_id == "name-guid",
            "#785: batch duplicate should preserve existing deleted flags");
        expect(!list_result.objects[3].deleted &&
                list_result.objects[3].object_name == "cmdSaveCopy" &&
                list_result.objects[3].unique_id == "save-copy-guid" &&
                list_result.objects[3].parent_name == "frmMain" &&
                list_result.objects[3].class_name == "commandbutton" &&
                list_result.objects[3].caption == "\"Save\"",
            "#785: first batch duplicate should expose replacement identity and copied metadata");
        expect(!list_result.objects[4].deleted &&
                list_result.objects[4].object_name == "lblStatusCopy" &&
                list_result.objects[4].unique_id == "status-copy-guid" &&
                list_result.objects[4].caption == "\"Status\"",
            "#785: second batch duplicate should expose replacement identity and copied metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "12",
        "#785: batch duplicate should preserve memo-backed properties");
    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#785: batch duplicate should preserve METHODS memo content");

    const auto committed_count = object_count();
    batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .new_object_name = "cmdTemp",
                .new_name = "tempButton",
                .new_unique_id = "temp-guid"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .new_object_name = "lblTemp",
                .new_name = "tempLabel",
                .new_unique_id = "name-guid"
            }
        }
    });
    expect(!batch_result.ok && batch_result.record_indexes.empty(),
        "#785: batch duplicate should reject identity collisions with deleted rows");
    expect(batch_result.duplicated_objects.empty(),
        "#994: failed batch duplicate should not report stale identity metadata after collisions");
    expect(object_count() == committed_count,
        "#785: deleted-row collision failures should roll back earlier duplicate rows");

    batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .new_object_name = "cmdTemp",
                .new_name = "tempButton",
                .new_unique_id = "temp-guid"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .new_object_name = "lblTemp",
                .new_name = "tempLabel",
                .new_unique_id = "temp-guid"
            }
        }
    });
    expect(!batch_result.ok, "#785: batch duplicate should reject within-batch identity collisions");
    expect(object_count() == committed_count,
        "#785: within-batch collision failures should roll back earlier duplicate rows");

    batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .new_object_name = "cmdTemp",
                .new_name = "tempButton",
                .new_unique_id = "temp-guid"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .new_object_name = "missingCopy",
                .new_name = "missingCopy",
                .new_unique_id = "missing-copy-guid"
            }
        }
    });
    expect(!batch_result.ok, "#785: batch duplicate should reject missing source selectors");
    expect(object_count() == committed_count,
        "#785: missing-source failures should roll back earlier duplicate rows");

    batch_result = copperfin::vfp::duplicate_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#785: batch duplicate should reject empty batch requests");
    expect(object_count() == committed_count,
        "#785: empty-batch failures should not append rows");

    fs::remove_all(temp_dir, ignored);
}

void test_create_visual_object_appends_toolbox_field_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\n",
            ""
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#750: create fixture should be writable");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#750: create fixture should support deleted-row preservation setup");

    auto create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkActive"},
            {.property_name = "NAME", .property_value = "activeCheck"},
            {.property_name = "UNIQUEID", .property_value = "active-guid"},
            {.property_name = "PARENT", .property_value = "frmMain"},
            {.property_name = "CLASS", .property_value = "checkbox"},
            {.property_name = "BASECLASS", .property_value = "checkbox"},
            {.property_name = "PROPERTIES", .property_value = "Caption = \"Active\"\r\nLeft = 24\r\n"},
            {.property_name = "METHODS", .property_value = "PROCEDURE Click\r\nTHIS.Value = !THIS.Value\r\nENDPROC"}
        }
    });
    expect(create_object_result.ok && create_object_result.record_index == 2U,
        "#750: toolbox creates should append a live object row at the next record index");
    expect(create_object_result.object_name == "chkActive" &&
            create_object_result.unique_id == "active-guid" &&
            create_object_result.parent_name == "frmMain",
        "#991: toolbox creates should report created object identity metadata");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#750: toolbox creates should append exactly one object");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdSave" &&
                list_result.objects[0].unique_id == "save-guid",
            "#750: toolbox creates should preserve existing live records");
        expect(list_result.objects[1].deleted && list_result.objects[1].unique_id == "name-guid",
            "#750: toolbox creates should preserve existing deleted-row flags");
        expect(!list_result.objects[2].deleted &&
                list_result.objects[2].object_name == "chkActive" &&
                list_result.objects[2].unique_id == "active-guid" &&
                list_result.objects[2].parent_name == "frmMain" &&
                list_result.objects[2].class_name == "checkbox" &&
                list_result.objects[2].baseclass_name == "checkbox" &&
                list_result.objects[2].caption == "\"Active\"",
            "#750: created objects should expose initialized identity, hierarchy, class, and caption metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "24",
        "#750: toolbox creates should initialize memo-backed properties");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#750: toolbox creates should initialize METHODS memo content");

    create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkOther"},
            {.property_name = "NAME", .property_value = "otherCheck"},
            {.property_name = "UNIQUEID", .property_value = "name-guid"}
        }
    });
    expect(!create_object_result.ok,
        "#750: toolbox creates should reject identity collisions with deleted rows");
    expect(create_object_result.object_name.empty() &&
            create_object_result.unique_id.empty() &&
            create_object_result.parent_name.empty(),
        "#991: failed toolbox creates should not report stale identity metadata after collisions");

    create_object_result = copperfin::vfp::create_visual_object({
        .path = table_path.string(),
        .field_values = {
            {.property_name = "OBJNAME", .property_value = "chkOther"},
            {.property_name = "UNKNOWN", .property_value = "value"}
        }
    });
    expect(!create_object_result.ok,
        "#750: toolbox creates should reject unknown requested fields");
    expect(create_object_result.object_name.empty() &&
            create_object_result.unique_id.empty() &&
            create_object_result.parent_name.empty(),
        "#991: failed toolbox creates should not report stale identity metadata after invalid fields");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U && list_result.objects[1].deleted,
        "#750: failed toolbox creates should not mutate object count or deleted flags");

    fs::remove_all(temp_dir, ignored);
}

void test_create_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_create_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch_create.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "frmMain",
            "commandbutton",
            "commandbutton",
            "Caption = \"Save\"\r\n",
            ""
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "frmMain",
            "textbox",
            "textbox",
            "Caption = \"Name\"\r\n",
            ""
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#784: batch create fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#784: batch create fixture should support deleted-row collision setup");

    const auto object_count = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#784: batch-create fixture should remain listable");
        return list_result.objects.size();
    };

    auto batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "chkActive"},
                    {.property_name = "NAME", .property_value = "activeCheck"},
                    {.property_name = "UNIQUEID", .property_value = "active-guid"},
                    {.property_name = "PARENT", .property_value = "frmMain"},
                    {.property_name = "CLASS", .property_value = "checkbox"},
                    {.property_name = "BASECLASS", .property_value = "checkbox"},
                    {.property_name = "PROPERTIES", .property_value = "Caption = \"Active\"\r\nLeft = 24\r\n"},
                    {.property_name = "METHODS", .property_value = "PROCEDURE Click\r\nTHIS.Value = !THIS.Value\r\nENDPROC"}
                }
            },
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "lblState"},
                    {.property_name = "NAME", .property_value = "stateLabel"},
                    {.property_name = "UNIQUEID", .property_value = "state-guid"},
                    {.property_name = "PARENT", .property_value = "frmMain"},
                    {.property_name = "CLASS", .property_value = "label"},
                    {.property_name = "BASECLASS", .property_value = "label"},
                    {.property_name = "PROPERTIES", .property_value = "Caption = \"State\"\r\nLeft = 48\r\n"}
                }
            }
        }
    });
    expect(batch_result.ok && batch_result.record_indexes.size() == 2U &&
            batch_result.record_indexes[0] == 2U && batch_result.record_indexes[1] == 3U,
        "#784: batch creates should append each object and return created record indexes");
    expect(batch_result.created_objects.size() == 2U &&
            batch_result.created_objects[0].record_index == 2U &&
            batch_result.created_objects[0].object_name == "chkActive" &&
            batch_result.created_objects[0].unique_id == "active-guid" &&
            batch_result.created_objects[0].parent_name == "frmMain" &&
            batch_result.created_objects[1].record_index == 3U &&
            batch_result.created_objects[1].object_name == "lblState" &&
            batch_result.created_objects[1].unique_id == "state-guid" &&
            batch_result.created_objects[1].parent_name == "frmMain",
        "#992: batch creates should report created object identity metadata in append order");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 4U,
        "#784: batch creates should append all requested objects");
    if (list_result.ok && list_result.objects.size() == 4U) {
        expect(list_result.objects[1].deleted && list_result.objects[1].unique_id == "name-guid",
            "#784: batch creates should preserve existing deleted-row flags");
        expect(!list_result.objects[2].deleted &&
                list_result.objects[2].object_name == "chkActive" &&
                list_result.objects[2].unique_id == "active-guid" &&
                list_result.objects[2].caption == "\"Active\"",
            "#784: first batch-created object should expose initialized metadata");
        expect(!list_result.objects[3].deleted &&
                list_result.objects[3].object_name == "lblState" &&
                list_result.objects[3].unique_id == "state-guid" &&
                list_result.objects[3].caption == "\"State\"",
            "#784: second batch-created object should expose initialized metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid",
        .property_name = "Left"
    });
    expect(property_result.ok && property_result.exists && property_result.value == "24",
        "#784: batch creates should initialize memo-backed properties");
    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "active-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#784: batch creates should initialize METHODS memo content");

    const auto committed_count = object_count();
    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp"},
                    {.property_name = "NAME", .property_value = "tempButton"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            },
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "txtClone"},
                    {.property_name = "NAME", .property_value = "cloneBox"},
                    {.property_name = "UNIQUEID", .property_value = "name-guid"}
                }
            }
        }
    });
    expect(!batch_result.ok && batch_result.record_indexes.empty(),
        "#784: batch creates should reject identity collisions with deleted rows");
    expect(batch_result.created_objects.empty(),
        "#992: failed batch creates should not report stale identity metadata after collisions");
    expect(object_count() == committed_count,
        "#784: deleted-row collision failures should not append partial rows");

    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp"},
                    {.property_name = "NAME", .property_value = "tempButton"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            },
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp2"},
                    {.property_name = "NAME", .property_value = "tempButton2"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            }
        }
    });
    expect(!batch_result.ok, "#784: batch creates should reject within-batch identity collisions");
    expect(object_count() == committed_count,
        "#784: within-batch collision failures should not append partial rows");

    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            },
            {
                .field_values = {
                    {.property_name = "UNKNOWN", .property_value = "value"}
                }
            }
        }
    });
    expect(!batch_result.ok, "#784: batch creates should reject unknown fields");
    expect(batch_result.created_objects.empty(),
        "#992: failed batch creates should not report stale identity metadata after invalid fields");
    expect(object_count() == committed_count,
        "#784: unknown-field failures should not append partial rows");

    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            },
            {
                .field_values = {
                    {.property_name = "  ", .property_value = "value"}
                }
            }
        }
    });
    expect(!batch_result.ok, "#784: batch creates should reject empty field names");
    expect(object_count() == committed_count,
        "#784: empty-field-name failures should not append partial rows");

    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .field_values = {
                    {.property_name = "OBJNAME", .property_value = "cmdTemp"},
                    {.property_name = "UNIQUEID", .property_value = "temp-guid"}
                }
            },
            {
                .field_values = {}
            }
        }
    });
    expect(!batch_result.ok, "#784: batch creates should reject empty item field sets");
    expect(object_count() == committed_count,
        "#784: empty-item failures should not append partial rows");

    batch_result = copperfin::vfp::create_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#784: batch creates should reject empty batch requests");
    expect(object_count() == committed_count,
        "#784: empty-batch failures should not append rows");

    fs::remove_all(temp_dir, ignored);
}

void test_reparent_visual_object_updates_container_parent() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reparent_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reparent.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", ""},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", ""},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#751: reparent fixture should be writable");

    auto reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID source and object-name parent selection");
    expect(reparent_result.affected_object_count == 1U,
        "#1006: successful reparent should report one affected object");

    auto parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "cntMain",
        "#751: reparent should write the resolved parent object name");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: reparent should preserve unrelated object parent fields");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#751: reparent should route through visual property undo");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: undo should restore the previous parent");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .parent_object_name = "cntMain",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support object-name source selection");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "form-guid",
        .clear_parent = false
    });
    expect(reparent_result.ok, "#751: reparent should support UNIQUEID parent selection");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value == "frmMain",
        "#751: UNIQUEID parent selection should write the target object's OBJNAME");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = {},
        .clear_parent = true
    });
    expect(reparent_result.ok, "#751: reparent should support clearing parent for root-level placement");
    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: clear-parent reparent should blank the parent field");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = {},
        .parent_unique_id = "save-guid",
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject self-parenting");
    expect(reparent_result.affected_object_count == 0U,
        "#1006: failed reparent should report zero affected objects");

    reparent_result = copperfin::vfp::reparent_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .parent_object_name = "missingParent",
        .parent_unique_id = {},
        .clear_parent = false
    });
    expect(!reparent_result.ok, "#751: reparent should reject missing parent selectors");

    parent_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "PARENT"
    });
    expect(parent_result.ok && parent_result.value.empty(),
        "#751: failed reparent requests should not mutate the selected object's parent");

    fs::remove_all(temp_dir, ignored);
}

void test_reparent_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reparent_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reparent_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", ""},
        {"cntAlt", "altContainer", "alt-guid", "frmMain", "container", "container", ""},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", ""},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", ""},
        {"lblStatus", "statusLabel", "status-guid", "cntMain", "label", "label", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#781: reparent-batch fixture should be writable");

    const auto parent_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "PARENT"
        });
    };

    auto batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntMain",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = "alt-guid",
                .clear_parent = false
            },
            {
                .record_index = 5U,
                .object_name = {},
                .unique_id = {},
                .parent_object_name = {},
                .parent_unique_id = {},
                .clear_parent = true
            }
        }
    });
    expect(batch_result.ok, "#781: batch reparent should support mixed selectors, parent names, parent UNIQUEIDs, and clear-parent operations");
    expect(batch_result.affected_object_count == 3U,
        "#1006: successful batch reparent should report affected item count");

    auto save_parent = parent_state("save-guid");
    auto name_parent = parent_state("name-guid");
    auto status_parent = parent_state("status-guid");
    auto container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            name_parent.ok && name_parent.value == "cntAlt" &&
            status_parent.ok && status_parent.value.empty() &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: batch reparent should persist requested parents while preserving unrelated parent fields");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#781: successful batch reparents should leave normal visual undo history available");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .parent_object_name = "missingParent",
                .parent_unique_id = {},
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject missing parent selectors");
    expect(batch_result.affected_object_count == 0U,
        "#1006: failed batch reparent should report zero affected objects");
    save_parent = parent_state("save-guid");
    name_parent = parent_state("name-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            name_parent.ok && name_parent.value == "cntAlt",
        "#781: missing-parent failures should roll back earlier reparent writes");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .parent_object_name = "cntMain",
                .parent_unique_id = {},
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject missing source selectors");
    save_parent = parent_state("save-guid");
    expect(save_parent.ok && save_parent.value == "cntMain",
        "#781: missing-source failures should roll back earlier reparent writes");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .parent_object_name = "cntAlt",
                .parent_unique_id = {},
                .clear_parent = false
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "container-guid",
                .parent_object_name = {},
                .parent_unique_id = "container-guid",
                .clear_parent = false
            }
        }
    });
    expect(!batch_result.ok, "#781: batch reparent should reject self-parenting");
    save_parent = parent_state("save-guid");
    container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "cntMain" &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: self-parent failures should roll back earlier reparent writes");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#781: failed batch reparent rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::reparent_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#781: empty batch reparent requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1006: empty batch reparent should report zero affected objects");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#781: undo should restore each successful batch reparent write");
    }

    save_parent = parent_state("save-guid");
    name_parent = parent_state("name-guid");
    status_parent = parent_state("status-guid");
    container_parent = parent_state("container-guid");
    expect(save_parent.ok && save_parent.value == "frmMain" &&
            name_parent.ok && name_parent.value == "frmMain" &&
            status_parent.ok && status_parent.value == "cntMain" &&
            container_parent.ok && container_parent.value == "frmMain",
        "#781: successful batch reparent undo should restore original parent state");

    fs::remove_all(temp_dir, ignored);
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

void test_group_visual_objects_creates_container_and_rolls_back_failures() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_group_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "group.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n"},
        {"cmdSave", "saveButton", "save-guid", "frmMain", "commandbutton", "commandbutton", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "frmMain", "textbox", "textbox", "Caption = \"Name\"\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "frmMain", "label", "label", "Caption = \"Status\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#788: grouping fixture should be writable");

    const auto object_count = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#788: grouping fixture should remain listable");
        return list_result.objects.size();
    };
    const auto parent_value = [&](const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "PARENT"
        });
        expect(result.ok && result.exists, "#788: grouping parent property should be readable");
        return result.value;
    };

    auto group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {
            {.property_name = "OBJNAME", .property_value = "cntGroup"},
            {.property_name = "NAME", .property_value = "groupContainer"},
            {.property_name = "UNIQUEID", .property_value = "group-guid"},
            {.property_name = "PARENT", .property_value = "frmMain"},
            {.property_name = "CLASS", .property_value = "container"},
            {.property_name = "BASECLASS", .property_value = "container"},
            {.property_name = "PROPERTIES", .property_value = "Caption = \"Group\"\r\n"}
        },
        .objects = {
            {.record_index = 0U, .object_name = "cmdSave", .unique_id = {}},
            {.record_index = 0U, .object_name = {}, .unique_id = "name-guid"}
        }
    });
    expect(group_result.ok && group_result.container_record_index == 4U && group_result.child_count == 2U,
        "#989: grouping should append a container and report the grouped child count");
    expect(group_result.container_object_name == "cntGroup" &&
            group_result.container_unique_id == "group-guid" &&
            group_result.container_parent_name == "frmMain",
        "#996: grouping should report created container identity metadata");
    expect(object_count() == 5U,
        "#788: grouping should append exactly one group container");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 5U,
        "#788: grouped asset should remain listable");
    if (list_result.ok && list_result.objects.size() == 5U) {
        expect(list_result.objects[4].object_name == "cntGroup" &&
                list_result.objects[4].unique_id == "group-guid" &&
                list_result.objects[4].parent_name == "frmMain" &&
                list_result.objects[4].class_name == "container" &&
                list_result.objects[4].caption == "\"Group\"",
            "#788: grouping should expose the appended container metadata");
    }
    expect(parent_value("save-guid") == "cntGroup" &&
            parent_value("name-guid") == "cntGroup" &&
            parent_value("status-guid") == "frmMain",
        "#788: grouping should reparent selected objects and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#788: first successful grouping reparent should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#788: second successful grouping reparent should remain undo-backed");
    expect(parent_value("save-guid") == "frmMain" &&
            parent_value("name-guid") == "frmMain" &&
            object_count() == 5U,
        "#788: grouping undo should restore parents while leaving the created container row");

    const auto committed_count = object_count();
    group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {
            {.property_name = "OBJNAME", .property_value = "cntTemp"},
            {.property_name = "NAME", .property_value = "tempContainer"},
            {.property_name = "UNIQUEID", .property_value = "temp-guid"},
            {.property_name = "PARENT", .property_value = "frmMain"}
        },
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "save-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        }
    });
    expect(!group_result.ok && group_result.child_count == 0U,
        "#989: grouping should reject missing selected objects with zero grouped child count");
    expect(group_result.container_object_name.empty() &&
            group_result.container_unique_id.empty() &&
            group_result.container_parent_name.empty(),
        "#996: failed grouping should not report stale container identity metadata");
    expect(object_count() == committed_count &&
            parent_value("save-guid") == "frmMain" &&
            parent_value("name-guid") == "frmMain",
        "#788: missing-selection grouping failures should remove the container and roll back parents");

    group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {
            {.property_name = "OBJNAME", .property_value = "cntNoName"},
            {.property_name = "UNIQUEID", .property_value = "group-guid"}
        },
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "save-guid"}
        }
    });
    expect(!group_result.ok && group_result.child_count == 0U,
        "#989: grouping should reject invalid container identities with zero grouped child count");
    expect(object_count() == committed_count,
        "#788: invalid-container failures should not append rows");

    group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {
            {.property_name = "CLASS", .property_value = "container"},
            {.property_name = "UNIQUEID", .property_value = "nameless-guid"}
        },
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "save-guid"}
        }
    });
    expect(!group_result.ok && group_result.child_count == 0U,
        "#989: grouping should reject containers without OBJNAME or fallback NAME with zero grouped child count");
    expect(object_count() == committed_count,
        "#788: nameless-container failures should remove the created row");

    group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {},
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "save-guid"}
        }
    });
    expect(!group_result.ok && group_result.child_count == 0U,
        "#989: grouping should reject empty container field values with zero grouped child count");
    expect(object_count() == committed_count,
        "#788: empty-container-field failures should not append rows");

    group_result = copperfin::vfp::group_visual_objects({
        .path = table_path.string(),
        .container_field_values = {
            {.property_name = "OBJNAME", .property_value = "cntEmpty"},
            {.property_name = "UNIQUEID", .property_value = "empty-guid"}
        },
        .objects = {}
    });
    expect(!group_result.ok && group_result.child_count == 0U,
        "#989: grouping should reject empty selections with zero grouped child count");
    expect(object_count() == committed_count,
        "#788: empty-selection failures should not append rows");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_updates_identity_safely() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "frmMain"},
        {"txtName", "nameBox", "name-guid", "frmMain"},
        {"oldDeleted", "deletedName", "deleted-guid", "frmMain"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#754: rename fixture should be writable");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "deleted-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#754: rename fixture should support deleted-row collision setup");

    auto rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .update_object_name = true,
        .new_object_name = "cmdCommit",
        .update_name = true,
        .new_name = "commitButton",
        .update_unique_id = true,
        .new_unique_id = "commit-guid"
    });
    expect(rename_result.ok, "#754: rename should update selected object identity fields together");
    expect(rename_result.affected_object_count == 1U,
        "#1006: successful object rename should report one affected object");

    auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U,
        "#754: renamed visual asset should remain listable");
    if (list_result.ok && list_result.objects.size() == 3U) {
        expect(list_result.objects[0].object_name == "cmdCommit" &&
                list_result.objects[0].unique_id == "commit-guid",
            "#754: rename should expose updated OBJNAME and UNIQUEID");
        expect(list_result.objects[1].object_name == "txtName" &&
                list_result.objects[1].unique_id == "name-guid",
            "#754: rename should preserve unrelated object identity");
        expect(list_result.objects[2].deleted && list_result.objects[2].unique_id == "deleted-guid",
            "#754: rename should preserve deleted-row identity metadata");
    }

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "commit-guid",
        .property_name = "NAME"
    });
    expect(property_result.ok && property_result.value == "commitButton",
        "#754: rename should update NAME and keep UNIQUEID selection usable");

    rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .update_object_name = false,
        .new_object_name = {},
        .update_name = false,
        .new_name = {},
        .update_unique_id = true,
        .new_unique_id = "deleted-guid"
    });
    expect(!rename_result.ok,
        "#754: rename should reject identity collisions with deleted rows");
    expect(rename_result.affected_object_count == 0U,
        "#1006: failed object rename should report zero affected objects");

    rename_result = copperfin::vfp::rename_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .update_object_name = false,
        .new_object_name = {},
        .update_name = false,
        .new_name = {},
        .update_unique_id = false,
        .new_unique_id = {}
    });
    expect(!rename_result.ok, "#754: empty rename requests should fail explicitly");

    property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdCommit",
        .unique_id = {},
        .property_name = "UNIQUEID"
    });
    expect(property_result.ok && property_result.value == "commit-guid",
        "#754: failed rename requests should not mutate selected identity fields");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route UNIQUEID through existing undo");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route NAME through existing undo");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#754: rename should route OBJNAME through existing undo");

    list_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(list_result.ok && list_result.objects.size() == 3U &&
            list_result.objects[0].object_name == "cmdSave" &&
            list_result.objects[0].unique_id == "save-guid",
        "#754: undo should restore renamed identity fields");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_rename_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rename_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PARENT", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "frmMain"},
        {"txtName", "nameBox", "name-guid", "frmMain"},
        {"lblStatus", "statusLabel", "status-guid", "frmMain"},
        {"oldDeleted", "deletedName", "deleted-guid", "frmMain"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#782: rename-batch fixture should be writable");

    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "deleted-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#782: rename-batch fixture should support deleted-row collision setup");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#782: identity property should remain queryable");
        return result.value;
    };

    auto batch_result = copperfin::vfp::rename_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .update_object_name = true,
                .new_object_name = "cmdCommit",
                .update_name = true,
                .new_name = "commitButton",
                .update_unique_id = true,
                .new_unique_id = "commit-guid"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .update_object_name = false,
                .new_object_name = {},
                .update_name = true,
                .new_name = "nameEntry",
                .update_unique_id = true,
                .new_unique_id = "name-entry-guid"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .update_object_name = true,
                .new_object_name = "lblState",
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            }
        }
    });
    expect(batch_result.ok, "#782: batch rename should support mixed selectors and OBJNAME/NAME/UNIQUEID updates");
    expect(batch_result.affected_object_count == 3U,
        "#1006: successful batch object rename should report affected item count");
    expect(property_value("commit-guid", "OBJNAME") == "cmdCommit" &&
            property_value("commit-guid", "NAME") == "commitButton" &&
            property_value("name-entry-guid", "NAME") == "nameEntry" &&
            property_value("status-guid", "OBJNAME") == "lblState",
        "#782: batch rename should persist requested identity updates");
    expect(property_value("deleted-guid", "OBJNAME") == "oldDeleted",
        "#782: batch rename should preserve unrelated deleted-row identity");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#782: successful batch renames should leave normal visual undo history available");

    batch_result = copperfin::vfp::rename_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "commit-guid",
                .update_object_name = false,
                .new_object_name = {},
                .update_name = true,
                .new_name = "temporaryCommit",
                .update_unique_id = false,
                .new_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-entry-guid",
                .update_object_name = false,
                .new_object_name = {},
                .update_name = false,
                .new_name = {},
                .update_unique_id = true,
                .new_unique_id = "deleted-guid"
            }
        }
    });
    expect(!batch_result.ok, "#782: batch rename should reject identity collisions with deleted rows");
    expect(property_value("commit-guid", "NAME") == "commitButton" &&
            property_value("name-entry-guid", "UNIQUEID") == "name-entry-guid",
        "#782: collision failures should roll back earlier identity writes");

    batch_result = copperfin::vfp::rename_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "commit-guid",
                .update_object_name = false,
                .new_object_name = {},
                .update_name = true,
                .new_name = "temporaryCommit",
                .update_unique_id = false,
                .new_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .update_object_name = true,
                .new_object_name = "missingObj",
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            }
        }
    });
    expect(!batch_result.ok, "#782: batch rename should reject missing source selectors");
    expect(property_value("commit-guid", "NAME") == "commitButton",
        "#782: missing-source failures should roll back earlier identity writes");

    batch_result = copperfin::vfp::rename_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "commit-guid",
                .update_object_name = false,
                .new_object_name = {},
                .update_name = true,
                .new_name = "temporaryCommit",
                .update_unique_id = false,
                .new_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-entry-guid",
                .update_object_name = false,
                .new_object_name = {},
                .update_name = false,
                .new_name = {},
                .update_unique_id = false,
                .new_unique_id = {}
            }
        }
    });
    expect(!batch_result.ok, "#782: batch rename should reject items without requested identity fields");
    expect(property_value("commit-guid", "NAME") == "commitButton",
        "#782: empty-item failures should roll back earlier identity writes");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#782: failed batch rename rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::rename_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#782: empty batch rename requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1006: empty batch object rename should report zero affected objects");

    for (int index = 0; index < 6; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#782: undo should restore each successful batch identity write");
    }

    expect(property_value("save-guid", "OBJNAME") == "cmdSave" &&
            property_value("save-guid", "NAME") == "saveButton" &&
            property_value("name-guid", "NAME") == "nameBox" &&
            property_value("status-guid", "OBJNAME") == "lblStatus" &&
            property_value("deleted-guid", "OBJNAME") == "oldDeleted",
        "#782: successful batch rename undo should restore original identity state");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_updates_z_order() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdA", "buttonA", "a-guid", "Caption = \"A\"\r\n"},
        {"cmdB", "buttonB", "b-guid", "Caption = \"B\"\r\n"},
        {"cmdC", "buttonC", "c-guid", "Caption = \"C\"\r\n"},
        {"cmdD", "buttonD", "d-guid", "Caption = \"D\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#755: reorder fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#755: reorder fixture should support deleted-row preservation setup");

    const auto order_string = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#755: reordered visual asset should remain listable");
        std::string value;
        for (const auto& object : list_result.objects) {
            if (!value.empty()) {
                value += ",";
            }
            value += object.unique_id;
            if (object.deleted) {
                value += "*";
            }
        }
        return value;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .placement = "front",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support front placement by UNIQUEID");
    expect(reorder_result.affected_object_count == 1U,
        "#1006: successful object reorder should report one affected object");
    expect(order_string() == "c-guid*,a-guid,b-guid,d-guid",
        "#755: front placement should move the selected record to the front and preserve deleted flags");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdA",
        .unique_id = {},
        .placement = "back",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support back placement by object name after indexes change");
    expect(order_string() == "c-guid*,b-guid,d-guid,a-guid",
        "#755: back placement should move the selected record to the back");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .placement = "before",
        .target_object_name = "cmdB",
        .target_unique_id = {}
    });
    expect(reorder_result.ok, "#755: reorder should support before-target placement by object-name target");
    expect(order_string() == "c-guid*,a-guid,b-guid,d-guid",
        "#755: before placement should insert the selected record before the resolved target");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "after",
        .target_object_name = {},
        .target_unique_id = "d-guid"
    });
    expect(reorder_result.ok, "#755: reorder should support after-target placement by UNIQUEID target");
    expect(order_string() == "c-guid*,a-guid,d-guid,b-guid",
        "#755: after placement should insert the selected record after the resolved target");

    auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .property_name = "Caption"
    });
    expect(property_result.ok && property_result.value == "\"A\"",
        "#755: reorder should preserve memo-backed field values");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "before",
        .target_object_name = {},
        .target_unique_id = "b-guid"
    });
    expect(!reorder_result.ok, "#755: reorder should reject self-targeted relative moves");
    expect(reorder_result.affected_object_count == 0U,
        "#1006: failed object reorder should report zero affected objects");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "after",
        .target_object_name = "missingObject",
        .target_unique_id = {}
    });
    expect(!reorder_result.ok, "#755: reorder should reject missing target selectors");

    reorder_result = copperfin::vfp::reorder_visual_object({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "b-guid",
        .placement = "sideways",
        .target_object_name = {},
        .target_unique_id = {}
    });
    expect(!reorder_result.ok, "#755: reorder should reject unsupported placements");
    expect(order_string() == "c-guid*,a-guid,d-guid,b-guid",
        "#755: failed reorder requests should not mutate record order");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_objects_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdA", "buttonA", "a-guid", "Caption = \"A\"\r\n"},
        {"cmdB", "buttonB", "b-guid", "Caption = \"B\"\r\n"},
        {"cmdC", "buttonC", "c-guid", "Caption = \"C\"\r\n"},
        {"cmdD", "buttonD", "d-guid", "Caption = \"D\"\r\n"},
        {"cmdE", "buttonE", "e-guid", "Caption = \"E\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#783: batch reorder fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "c-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#783: batch reorder fixture should support deleted-row preservation setup");

    const auto order_string = [&]() {
        const auto list_result = copperfin::vfp::list_visual_objects(table_path.string());
        expect(list_result.ok, "#783: batch-reordered visual asset should remain listable");
        std::string value;
        for (const auto& object : list_result.objects) {
            if (!value.empty()) {
                value += ",";
            }
            value += object.unique_id;
            if (object.deleted) {
                value += "*";
            }
        }
        return value;
    };

    const std::string original_order = order_string();
    auto batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#783: batch reorder should reject empty operation sets");
    expect(batch_result.affected_object_count == 0U,
        "#1006: empty batch object reorder should report zero affected objects");
    expect(order_string() == original_order, "#783: empty batch failures should not mutate record order");

    batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "d-guid",
                .placement = "front",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = "cmdA",
                .unique_id = {},
                .placement = "back",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 3U,
                .object_name = {},
                .unique_id = {},
                .placement = "before",
                .target_object_name = {},
                .target_unique_id = "c-guid"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "after",
                .target_object_name = "cmdA",
                .target_unique_id = {}
            }
        }
    });
    expect(batch_result.ok,
        "#783: batch reorder should support mixed source selectors and front/back/before/after placements");
    expect(batch_result.affected_object_count == 4U,
        "#1006: successful batch object reorder should report affected item count");
    expect(order_string() == "d-guid,e-guid,c-guid*,a-guid,b-guid",
        "#783: batch reorder should apply operations against the evolving row order and preserve deleted flags");

    const auto property_result = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "a-guid",
        .property_name = "Caption"
    });
    expect(property_result.ok && property_result.value == "\"A\"",
        "#783: batch reorder should preserve memo-backed field values");

    const std::string committed_order = order_string();
    batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "front",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "e-guid",
                .placement = "after",
                .target_object_name = "missingObject",
                .target_unique_id = {}
            }
        }
    });
    expect(!batch_result.ok, "#783: batch reorder should reject missing target selectors");
    expect(batch_result.affected_object_count == 0U,
        "#1006: failed batch object reorder should report zero affected objects");
    expect(order_string() == committed_order,
        "#783: missing-target failures should roll back earlier batch order changes");

    batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "front",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .placement = "back",
                .target_object_name = {},
                .target_unique_id = {}
            }
        }
    });
    expect(!batch_result.ok, "#783: batch reorder should reject missing source selectors");
    expect(order_string() == committed_order,
        "#783: missing-source failures should roll back earlier batch order changes");

    batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "front",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "before",
                .target_object_name = {},
                .target_unique_id = "b-guid"
            }
        }
    });
    expect(!batch_result.ok, "#783: batch reorder should reject self-relative moves");
    expect(order_string() == committed_order,
        "#783: self-relative failures should roll back earlier batch order changes");

    batch_result = copperfin::vfp::reorder_visual_objects({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "b-guid",
                .placement = "front",
                .target_object_name = {},
                .target_unique_id = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "a-guid",
                .placement = "sideways",
                .target_object_name = {},
                .target_unique_id = {}
            }
        }
    });
    expect(!batch_result.ok, "#783: batch reorder should reject unsupported placements");
    expect(order_string() == committed_order,
        "#783: unsupported-placement failures should roll back earlier batch order changes");

    fs::remove_all(temp_dir, ignored);
}

void test_duplicate_visual_object_subtree_rewrites_copied_parents() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_subtree_duplicate_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "subtree_duplicate.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 20U},
        {.name = "BASECLASS", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmMain", "mainForm", "form-guid", "", "form", "form", "Caption = \"Main\"\r\n", ""},
        {"cntMain", "mainContainer", "container-guid", "frmMain", "container", "container", "Caption = \"Container\"\r\n", ""},
        {"cmdSave", "saveButton", "save-guid", "cntMain", "commandbutton", "commandbutton", "Caption = \"Save\"\r\nLeft = 10\r\n", "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"},
        {"txtName", "nameBox", "name-guid", "cntMain", "textbox", "textbox", "Caption = \"Name\"\r\n", ""},
        {"lblNested", "nestedLabel", "nested-guid", "txtName", "label", "label", "Caption = \"Nested\"\r\n", ""},
        {"cmdOther", "otherButton", "other-guid", "", "commandbutton", "commandbutton", "", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#760: subtree duplicate fixture should be writable");
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .deleted = true
    });
    expect(delete_result.ok, "#760: subtree duplicate fixture should support deleted descendant setup");

    auto duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntCopy", .new_name = "mainContainerCopy", .new_unique_id = "container-copy-guid"},
            {.source_unique_id = "save-guid", .new_object_name = "cmdSaveCopy", .new_name = "saveButtonCopy", .new_unique_id = "save-copy-guid"},
            {.source_unique_id = "name-guid", .new_object_name = "txtNameCopy", .new_name = "nameBoxCopy", .new_unique_id = "name-copy-guid"},
            {.source_unique_id = "nested-guid", .new_object_name = "lblNestedCopy", .new_name = "nestedLabelCopy", .new_unique_id = "nested-copy-guid"}
        }
    });
    expect(duplicate_result.ok &&
            duplicate_result.root_record_index == 6U &&
            duplicate_result.copied_count == 4U,
        "#760: subtree duplicate should append root and descendants in pre-order");
    expect(duplicate_result.root_object_name == "cntCopy" &&
            duplicate_result.root_unique_id == "container-copy-guid" &&
            duplicate_result.root_parent_name == "frmMain",
        "#995: subtree duplicate should report copied root identity metadata");

    auto objects_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(objects_result.ok && objects_result.objects.size() == 10U,
        "#760: subtree duplicate should append the copied subtree without removing source rows");
    const auto find_object = [&](const std::string& unique_id) {
        return std::find_if(
            objects_result.objects.begin(),
            objects_result.objects.end(),
            [&](const copperfin::vfp::VisualObjectSnapshot& object) {
                return object.unique_id == unique_id;
            });
    };
    if (objects_result.ok) {
        const auto copied_root = find_object("container-copy-guid");
        const auto copied_save = find_object("save-copy-guid");
        const auto copied_name = find_object("name-copy-guid");
        const auto copied_nested = find_object("nested-copy-guid");
        expect(copied_root != objects_result.objects.end() &&
                copied_root->object_name == "cntCopy" &&
                copied_root->parent_name == "frmMain" &&
                !copied_root->deleted,
            "#760: subtree duplicate should preserve root parent and replacement identity");
        expect(copied_save != objects_result.objects.end() &&
                copied_save->parent_name == "cntCopy" &&
                copied_save->caption == "\"Save\"",
            "#760: subtree duplicate should rewrite copied child parent names and preserve memo properties");
        expect(copied_name != objects_result.objects.end() &&
                copied_name->parent_name == "cntCopy" &&
                copied_name->deleted,
            "#760: subtree duplicate should preserve deleted state for copied descendants");
        expect(copied_nested != objects_result.objects.end() &&
                copied_nested->parent_name == "txtNameCopy",
            "#760: subtree duplicate should rewrite grandchild parent names to copied parent identities");
        expect(find_object("container-guid") != objects_result.objects.end() &&
                find_object("other-guid") != objects_result.objects.end(),
            "#760: subtree duplicate should preserve source and unrelated rows");
    }

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-copy-guid"
    });
    expect(method_result.ok && find_method_snapshot(method_result.methods, "Click") != nullptr,
        "#760: subtree duplicate should preserve copied METHODS memo content");

    const auto object_count_after_success = objects_result.objects.size();
    duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cntMain",
        .unique_id = {},
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntCollision", .new_name = "collisionContainer", .new_unique_id = "container-copy-guid"},
            {.source_unique_id = "save-guid", .new_object_name = "cmdCollision", .new_name = "collisionButton", .new_unique_id = "collision-save-guid"},
            {.source_unique_id = "name-guid", .new_object_name = "txtCollision", .new_name = "collisionName", .new_unique_id = "collision-name-guid"},
            {.source_unique_id = "nested-guid", .new_object_name = "lblCollision", .new_name = "collisionNested", .new_unique_id = "collision-nested-guid"}
        }
    });
    expect(!duplicate_result.ok,
        "#760: subtree duplicate should reject replacement identities colliding with existing rows");
    expect(duplicate_result.root_object_name.empty() &&
            duplicate_result.root_unique_id.empty() &&
            duplicate_result.root_parent_name.empty(),
        "#995: failed subtree duplicate should not report stale copied root identity metadata after collisions");

    duplicate_result = copperfin::vfp::duplicate_visual_object_subtree({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "container-guid",
        .replacements = {
            {.source_unique_id = "container-guid", .new_object_name = "cntIncomplete", .new_name = "incompleteContainer", .new_unique_id = "incomplete-container-guid"}
        }
    });
    expect(!duplicate_result.ok,
        "#760: subtree duplicate should reject missing replacement identity data for copied descendants");

    objects_result = copperfin::vfp::list_visual_objects(table_path.string());
    expect(objects_result.ok && objects_result.objects.size() == object_count_after_success,
        "#760: failed subtree duplicate requests should not mutate object count");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
