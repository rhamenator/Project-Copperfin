// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
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


}  // namespace cf_test_visual_asset_editor
