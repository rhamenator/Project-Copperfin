// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
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


}  // namespace cf_test_visual_asset_editor
