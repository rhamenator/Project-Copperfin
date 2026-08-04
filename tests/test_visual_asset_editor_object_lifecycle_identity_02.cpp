// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
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
