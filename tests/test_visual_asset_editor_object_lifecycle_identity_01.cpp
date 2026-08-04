// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
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


}  // namespace cf_test_visual_asset_editor
