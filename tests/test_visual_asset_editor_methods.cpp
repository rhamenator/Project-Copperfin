// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_query_visual_object_method_reads_one_selected_method() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_query_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_query.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nfUnCtIoN GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#762: method-query fixture should be writable");

    auto query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 0U &&
            !query_result.record_deleted &&
            query_result.method.method_name == "Click" &&
            query_result.method.kind == "procedure" &&
            query_result.method.source_text == "THISFORM.Save()" &&
            query_result.method.source_line_index == 0U &&
            query_result.method.source_memo_block_number != 0U,
        "#762: method query should return one procedure with resolved record and source metadata");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .method_name = "GetCaption"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.method.method_name == "GetCaption" &&
            query_result.method.kind == "function" &&
            query_result.method.source_text == "RETURN THIS.Caption",
        "#762: method query should support object-name selection and function declarations");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .method_name = "LostFocus"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 1U &&
            query_result.method.method_name == "LostFocus" &&
            query_result.method.source_text == "THISFORM.ValidateName()",
        "#762: method query should support direct record-index selection");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Missing"
    });
    expect(query_result.ok && !query_result.exists && query_result.record_index == 0U,
        "#762: missing methods should return a successful not-found result");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#762: method query should not create undo history");

    const fs::path duplicate_path = temp_dir / "method_query_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        }
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#762: duplicate method-query fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "CLICK"
    });
    expect(!query_result.ok, "#762: duplicate matching method names should fail as ambiguous");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "   "
    });
    expect(!query_result.ok, "#762: empty method names should fail explicitly");

    const fs::path no_methods_path = temp_dir / "method_query_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdNoMethods", "no-methods-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#762: missing-METHODS fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = no_methods_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-methods-guid",
        .method_name = "Click"
    });
    expect(!query_result.ok, "#762: missing METHODS fields should fail explicitly");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_method_updates_and_appends_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_edit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_edit.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#747: method-edit fixture should be writable");

    auto update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click",
        .method_kind = "procedure",
        .source_text = "THISFORM.Save(.T.)"
    });
    expect(update_result.ok, "#747: method edits should update existing selected-object methods case-insensitively");
    expect(update_result.affected_object_count == 1U,
        "#1004: successful method update should report one affected object");

    update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "GetCaption",
        .method_kind = "function",
        .source_text = "RETURN THIS.Caption"
    });
    expect(update_result.ok, "#747: method edits should append missing selected-object methods");
    expect(update_result.affected_object_count == 1U,
        "#1004: successful method append should report one affected object");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: updated method fixture should remain readable");
    const auto* click = find_method_snapshot(method_result.methods, "Click");
    const auto* get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save(.T.)",
        "#747: method edits should replace existing method bodies while preserving declaration names");
    expect(get_caption != nullptr && get_caption->kind == "function" && get_caption->source_text == "RETURN THIS.Caption",
        "#747: method edits should append missing methods with requested kind and source");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#747: selected-object method edits should not mutate unrelated object records");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the appended method edit");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the replaced method edit");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: method fixture should remain readable after undo");
    click = find_method_snapshot(method_result.methods, "Click");
    get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save()",
        "#747: undo should restore original method source text");
    expect(get_caption == nullptr,
        "#747: undo should remove methods appended by the edit API");

    fs::remove_all(temp_dir, ignored);
}

void test_delete_visual_object_method_removes_selected_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#748: method-delete fixture should be writable");

    auto delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(delete_result.ok, "#748: method deletes should remove existing selected-object methods case-insensitively");
    expect(delete_result.affected_object_count == 1U,
        "#1004: successful method delete should report one affected object");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#748: method-delete fixture should remain readable after delete");
    expect(find_method_snapshot(method_result.methods, "Click") == nullptr,
        "#748: method deletes should remove the full selected method block");
    expect(find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: method deletes should preserve unrelated methods in the same METHODS memo");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#748: selected-object method deletes should not mutate unrelated object records");

    delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoesNotExist"
    });
    expect(!delete_result.ok, "#748: missing method deletes should fail explicitly");
    expect(delete_result.affected_object_count == 0U,
        "#1004: failed method delete should report zero affected objects");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") == nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: missing method deletes should not mutate the METHODS memo");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#748: undo should restore the deleted method block");
    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") != nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: undo should restore deleted methods while preserving unrelated methods");

    fs::remove_all(temp_dir, ignored);
}

void test_delete_visual_object_methods_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_delete_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_delete_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\n"
            "FUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\n"
            "PROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC\r\n"
            "FUNCTION Valid\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Paint\r\nTHIS.Refresh()\r\nENDPROC\r\n"
            "FUNCTION RefreshValue\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "dupObj",
            "dupName",
            "dup-guid",
            "PROCEDURE Click\r\nWAIT WINDOW \"first\"\r\nENDPROC\r\n"
            "PROCEDURE click\r\nWAIT WINDOW \"second\"\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#776: method-delete-batch fixture should be writable");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    auto batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "click"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .method_name = "LostFocus"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .method_name = "RefreshValue"
            }
        }
    });
    expect(batch_result.ok, "#776: batch method delete should support mixed selectors plus procedure and function deletion");
    expect(batch_result.affected_object_count == 3U,
        "#1004: successful batch method delete should report affected item count");

    auto save_click = method_state("save-guid", "Click");
    auto save_get_caption = method_state("save-guid", "GetCaption");
    auto save_init = method_state("save-guid", "Init");
    auto name_lost_focus = method_state("name-guid", "LostFocus");
    auto name_valid = method_state("name-guid", "Valid");
    auto status_paint = method_state("status-guid", "Paint");
    auto status_refresh_value = method_state("status-guid", "RefreshValue");
    expect(save_click.ok && !save_click.exists &&
            name_lost_focus.ok && !name_lost_focus.exists &&
            status_refresh_value.ok && !status_refresh_value.exists,
        "#776: batch method delete should remove each requested declaration");
    expect(save_get_caption.ok && save_get_caption.exists &&
            save_init.ok && save_init.exists &&
            name_valid.ok && name_valid.exists &&
            status_paint.ok && status_paint.exists,
        "#776: batch method delete should preserve unrelated methods");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#776: successful batch deletes should leave normal visual undo history available");

    batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "MissingMethod"
            }
        }
    });
    expect(!batch_result.ok, "#776: batch method delete should reject missing methods");
    expect(batch_result.affected_object_count == 0U,
        "#1004: failed batch method delete should report zero affected objects");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#776: missing-method failures should roll back earlier method deletes");

    batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = " "
            }
        }
    });
    expect(!batch_result.ok, "#776: batch method delete should reject empty method names");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#776: empty-name failures should roll back earlier method deletes");

    batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .method_name = "Click"
            }
        }
    });
    expect(!batch_result.ok, "#776: batch method delete should reject duplicate matching method names");
    save_init = method_state("save-guid", "Init");
    auto duplicate_click = method_state("dup-guid", "Click");
    expect(save_init.ok && save_init.exists &&
            !duplicate_click.ok,
        "#776: duplicate-method failures should roll back earlier deletes and report ambiguity");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#776: failed batch delete rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = table_path.string(),
        .methods = {}
    });
    expect(!batch_result.ok, "#776: empty batch method delete requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1004: empty batch method delete should report zero affected objects");

    const fs::path unsupported_path = temp_dir / "method_delete_batch_unsupported.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdUnsupported", "unsupported-guid"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#776: unsupported method-delete-batch fixture should be writable");
    batch_result = copperfin::vfp::delete_visual_object_methods({
        .path = unsupported_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "unsupported-guid",
                .method_name = "Click"
            }
        }
    });
    expect(!batch_result.ok, "#776: batch method delete should reject objects without METHODS carriers");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#776: undo should restore each successful batch method delete");
    }

    save_click = method_state("save-guid", "Click");
    name_lost_focus = method_state("name-guid", "LostFocus");
    status_refresh_value = method_state("status-guid", "RefreshValue");
    save_get_caption = method_state("save-guid", "GetCaption");
    save_init = method_state("save-guid", "Init");
    name_valid = method_state("name-guid", "Valid");
    status_paint = method_state("status-guid", "Paint");
    expect(save_click.ok && save_click.exists &&
            name_lost_focus.ok && name_lost_focus.exists &&
            status_refresh_value.ok && status_refresh_value.exists &&
            save_get_caption.ok && save_get_caption.exists &&
            save_init.ok && save_init.exists &&
            name_valid.ok && name_valid.exists &&
            status_paint.ok && status_paint.exists,
        "#776: successful batch delete undo should restore deleted methods and preserve unrelated methods");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_method_updates_declarations() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_rename_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#761: method-rename fixture should be writable");

    auto rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click",
        .new_method_name = "SaveClick"
    });
    expect(rename_result.ok, "#761: method rename should update procedure declarations case-insensitively");
    expect(rename_result.affected_object_count == 1U,
        "#1004: successful method rename should report one affected object");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .method_name = "GetCaption",
        .new_method_name = "BuildCaption"
    });
    expect(rename_result.ok, "#761: method rename should update function declarations by object-name selection");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#761: renamed method fixture should remain readable");
    const auto* save_click = find_method_snapshot(method_result.methods, "SaveClick");
    const auto* build_caption = find_method_snapshot(method_result.methods, "BuildCaption");
    expect(save_click != nullptr &&
            save_click->kind == "procedure" &&
            save_click->source_text == "THISFORM.Save()",
        "#761: procedure rename should preserve kind and body text");
    expect(build_caption != nullptr &&
            build_caption->kind == "function" &&
            build_caption->source_text == "RETURN THIS.Caption",
        "#761: function rename should preserve kind and body text");
    expect(find_method_snapshot(method_result.methods, "Click") == nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") == nullptr,
        "#761: method rename should remove the old declaration names");

    auto other_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid"
    });
    expect(other_result.ok && find_method_snapshot(other_result.methods, "LostFocus") != nullptr,
        "#761: method rename should preserve unrelated object METHODS memos");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "SaveClick",
        .new_method_name = "BuildCaption"
    });
    expect(!rename_result.ok, "#761: method rename should reject target method collisions");
    expect(rename_result.affected_object_count == 0U,
        "#1004: failed method rename should report zero affected objects");

    rename_result = copperfin::vfp::rename_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoesNotExist",
        .new_method_name = "Missing"
    });
    expect(!rename_result.ok, "#761: method rename should reject missing source methods");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "SaveClick") != nullptr &&
            find_method_snapshot(method_result.methods, "BuildCaption") != nullptr,
        "#761: failed method renames should not mutate the METHODS memo");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#761: undo should restore function rename");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#761: undo should restore procedure rename");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") != nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr &&
            find_method_snapshot(method_result.methods, "SaveClick") == nullptr &&
            find_method_snapshot(method_result.methods, "BuildCaption") == nullptr,
        "#761: undo should restore previous METHODS memo declarations");

    fs::remove_all(temp_dir, ignored);
}

void test_rename_visual_object_methods_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_rename_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_rename_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\n"
            "FUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\n"
            "PROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC\r\n"
            "FUNCTION Valid\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Paint\r\nTHIS.Refresh()\r\nENDPROC\r\n"
            "FUNCTION RefreshValue\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "dupObj",
            "dupName",
            "dup-guid",
            "PROCEDURE Click\r\nWAIT WINDOW \"first\"\r\nENDPROC\r\n"
            "PROCEDURE click\r\nWAIT WINDOW \"second\"\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#777: method-rename-batch fixture should be writable");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    auto batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "click",
                .new_method_name = "SaveClick"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .method_name = "Valid",
                .new_method_name = "IsValid"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .method_name = "Paint",
                .new_method_name = "PaintStatus"
            }
        }
    });
    expect(batch_result.ok, "#777: batch method rename should support mixed selectors plus procedure and function renames");
    expect(batch_result.affected_object_count == 3U,
        "#1004: successful batch method rename should report affected item count");

    auto save_click = method_state("save-guid", "Click");
    auto save_save_click = method_state("save-guid", "SaveClick");
    auto name_valid = method_state("name-guid", "Valid");
    auto name_is_valid = method_state("name-guid", "IsValid");
    auto status_paint = method_state("status-guid", "Paint");
    auto status_paint_status = method_state("status-guid", "PaintStatus");
    auto save_get_caption = method_state("save-guid", "GetCaption");
    auto save_init = method_state("save-guid", "Init");
    auto name_lost_focus = method_state("name-guid", "LostFocus");
    auto status_refresh_value = method_state("status-guid", "RefreshValue");
    expect(save_click.ok && !save_click.exists &&
            name_valid.ok && !name_valid.exists &&
            status_paint.ok && !status_paint.exists &&
            save_save_click.ok && save_save_click.exists && save_save_click.method.kind == "procedure" &&
            name_is_valid.ok && name_is_valid.exists && name_is_valid.method.kind == "function" &&
            status_paint_status.ok && status_paint_status.exists && status_paint_status.method.kind == "procedure",
        "#777: batch method rename should rename requested procedure and function declarations");
    expect(save_get_caption.ok && save_get_caption.exists &&
            save_init.ok && save_init.exists &&
            name_lost_focus.ok && name_lost_focus.exists &&
            status_refresh_value.ok && status_refresh_value.exists,
        "#777: batch method rename should preserve unrelated methods");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#777: successful batch renames should leave normal visual undo history available");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init",
                .new_method_name = "StartUp"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "MissingMethod",
                .new_method_name = "MissingRenamed"
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject missing methods");
    expect(batch_result.affected_object_count == 0U,
        "#1004: failed batch method rename should report zero affected objects");
    save_init = method_state("save-guid", "Init");
    auto save_start_up = method_state("save-guid", "StartUp");
    expect(save_init.ok && save_init.exists &&
            save_start_up.ok && !save_start_up.exists,
        "#777: missing-method failures should roll back earlier method renames");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init",
                .new_method_name = "StartUp"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = " ",
                .new_method_name = "EmptySource"
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject empty source method names");
    save_init = method_state("save-guid", "Init");
    save_start_up = method_state("save-guid", "StartUp");
    expect(save_init.ok && save_init.exists &&
            save_start_up.ok && !save_start_up.exists,
        "#777: empty-source failures should roll back earlier method renames");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init",
                .new_method_name = "StartUp"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "GetCaption",
                .new_method_name = " "
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject empty target method names");
    save_init = method_state("save-guid", "Init");
    save_start_up = method_state("save-guid", "StartUp");
    expect(save_init.ok && save_init.exists &&
            save_start_up.ok && !save_start_up.exists,
        "#777: empty-target failures should roll back earlier method renames");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "Init",
                .new_method_name = "StartUp"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .method_name = "Click",
                .new_method_name = "DupClick"
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject duplicate matching method names");
    save_init = method_state("save-guid", "Init");
    save_start_up = method_state("save-guid", "StartUp");
    auto duplicate_click = method_state("dup-guid", "Click");
    expect(save_init.ok && save_init.exists &&
            save_start_up.ok && !save_start_up.exists &&
            !duplicate_click.ok,
        "#777: duplicate-method failures should roll back earlier renames and report ambiguity");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .method_name = "RefreshValue",
                .new_method_name = "StatusRefresh"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .method_name = "GetCaption",
                .new_method_name = "SaveClick"
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject target collisions");
    status_refresh_value = method_state("status-guid", "RefreshValue");
    auto status_status_refresh = method_state("status-guid", "StatusRefresh");
    save_get_caption = method_state("save-guid", "GetCaption");
    expect(status_refresh_value.ok && status_refresh_value.exists &&
            status_status_refresh.ok && !status_status_refresh.exists &&
            save_get_caption.ok && save_get_caption.exists,
        "#777: target-collision failures should roll back earlier method renames");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#777: failed batch rename rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = table_path.string(),
        .methods = {}
    });
    expect(!batch_result.ok, "#777: empty batch method rename requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1004: empty batch method rename should report zero affected objects");

    const fs::path unsupported_path = temp_dir / "method_rename_batch_unsupported.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdUnsupported", "unsupported-guid"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#777: unsupported method-rename-batch fixture should be writable");
    batch_result = copperfin::vfp::rename_visual_object_methods({
        .path = unsupported_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "unsupported-guid",
                .method_name = "Click",
                .new_method_name = "SaveClick"
            }
        }
    });
    expect(!batch_result.ok, "#777: batch method rename should reject objects without METHODS carriers");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#777: undo should restore each successful batch method rename");
    }

    save_click = method_state("save-guid", "Click");
    save_save_click = method_state("save-guid", "SaveClick");
    name_valid = method_state("name-guid", "Valid");
    name_is_valid = method_state("name-guid", "IsValid");
    status_paint = method_state("status-guid", "Paint");
    status_paint_status = method_state("status-guid", "PaintStatus");
    save_get_caption = method_state("save-guid", "GetCaption");
    name_lost_focus = method_state("name-guid", "LostFocus");
    status_refresh_value = method_state("status-guid", "RefreshValue");
    expect(save_click.ok && save_click.exists &&
            save_save_click.ok && !save_save_click.exists &&
            name_valid.ok && name_valid.exists &&
            name_is_valid.ok && !name_is_valid.exists &&
            status_paint.ok && status_paint.exists &&
            status_paint_status.ok && !status_paint_status.exists &&
            save_get_caption.ok && save_get_caption.exists &&
            name_lost_focus.ok && name_lost_focus.exists &&
            status_refresh_value.ok && status_refresh_value.exists,
        "#777: successful batch rename undo should restore original method names and preserve unrelated methods");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_method_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_copy_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_copy.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\nFUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#763: method-copy fixture should be writable");

    auto copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "click",
        .target_record_index = 0U,
        .target_object_name = "txtTarget",
        .target_unique_id = {},
        .target_method_name = {},
        .replace_existing = false
    });
    expect(copy_result.ok, "#763: method copy should copy procedures by UNIQUEID source and object-name target");
    expect(copy_result.affected_object_count == 1U,
        "#1004: successful method copy should report one affected object");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSource",
        .source_unique_id = {},
        .source_method_name = "GetCaption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "CaptionText",
        .replace_existing = false
    });
    expect(copy_result.ok, "#763: method copy should support record-index targets and target method renames");

    auto target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(target_methods.ok, "#763: target method fixture should remain readable after copies");
    const auto* copied_click = find_method_snapshot(target_methods.methods, "Click");
    const auto* copied_caption = find_method_snapshot(target_methods.methods, "CaptionText");
    const auto* existing = find_method_snapshot(target_methods.methods, "Existing");
    expect(copied_click != nullptr &&
            copied_click->kind == "procedure" &&
            copied_click->source_text == "THISFORM.Save()",
        "#763: copied procedures should preserve source body and kind");
    expect(copied_caption != nullptr &&
            copied_caption->kind == "function" &&
            copied_caption->source_text == "RETURN THIS.Caption",
        "#763: copied functions should append using the source kind and requested target name");
    expect(existing != nullptr && existing->source_text == "THISFORM.Old()",
        "#763: method copy should preserve unrelated target methods");

    auto source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") != nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr,
        "#763: method copy should not mutate source methods");

    auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && find_method_snapshot(other_methods.methods, "Other") != nullptr,
        "#763: method copy should preserve unrelated object methods");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject target collisions without replacement");
    expect(copy_result.affected_object_count == 0U,
        "#1004: failed method copy should report zero affected objects");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "Refresh",
        .replace_existing = true
    });
    expect(copy_result.ok, "#763: method copy should allow explicit replacement");

    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(refresh != nullptr &&
            refresh->kind == "function" &&
            refresh->source_text == "THISFORM.Save()",
        "#763: replacing existing target methods should preserve the target declaration kind");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Missing",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Missing",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject missing source methods");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "EmptySource",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject empty source method names");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = " ",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject empty requested target method names");

    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "source-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: same-object method copy should reject implicit overwrite without replacement");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should restore replaced target methods");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should remove copied renamed function methods");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#763: undo should remove copied procedure methods");

    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(target_methods.ok &&
            find_method_snapshot(target_methods.methods, "Click") == nullptr &&
            find_method_snapshot(target_methods.methods, "CaptionText") == nullptr &&
            refresh != nullptr &&
            refresh->source_text == "RETURN .F.",
        "#763: undo should restore the target METHODS memo to its original method set");

    const fs::path duplicate_path = temp_dir / "method_copy_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        },
        {"txtTarget", "targetBox", "target-guid", ""}
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#763: duplicate method-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = duplicate_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "dup-guid",
        .source_method_name = "CLICK",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject duplicate source declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_copy_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdSource", "source-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#763: missing-METHODS method-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_method({
        .path = no_methods_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!copy_result.ok, "#763: method copy should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_methods_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_copy_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_copy_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\n"
            "FUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\n"
            "FUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        },
        {
            "dupObj",
            "dupName",
            "dup-guid",
            "PROCEDURE Click\r\nWAIT WINDOW \"first\"\r\nENDPROC\r\n"
            "PROCEDURE click\r\nWAIT WINDOW \"second\"\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#778: method-copy-batch fixture should be writable");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    auto batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "click",
                .target_record_index = 0U,
                .target_object_name = "txtTarget",
                .target_unique_id = {},
                .target_method_name = {},
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = "cmdSource",
                .source_unique_id = {},
                .source_method_name = "GetCaption",
                .target_record_index = 1U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_method_name = "CaptionText",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "other-guid",
                .source_method_name = "Other",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "OtherCopy",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Refresh",
                .replace_existing = true
            }
        }
    });
    expect(batch_result.ok, "#778: batch method copy should support mixed selectors, procedure/function copy, target renames, and replacement");
    expect(batch_result.affected_object_count == 4U,
        "#1004: successful batch method copy should report affected item count");

    auto target_click = method_state("target-guid", "Click");
    auto target_caption_text = method_state("target-guid", "CaptionText");
    auto target_other_copy = method_state("target-guid", "OtherCopy");
    auto target_refresh = method_state("target-guid", "Refresh");
    auto target_existing = method_state("target-guid", "Existing");
    auto source_click = method_state("source-guid", "Click");
    auto source_get_caption = method_state("source-guid", "GetCaption");
    auto other_method = method_state("other-guid", "Other");
    expect(target_click.ok && target_click.exists && target_click.method.kind == "procedure" &&
            target_caption_text.ok && target_caption_text.exists && target_caption_text.method.kind == "function" &&
            target_other_copy.ok && target_other_copy.exists && target_other_copy.method.source_text == "THISFORM.Other()" &&
            target_refresh.ok && target_refresh.exists && target_refresh.method.kind == "function" &&
            target_refresh.method.source_text == "THISFORM.Save()" &&
            target_existing.ok && target_existing.exists,
        "#778: batch method copy should persist copied targets and preserve target declaration kind on replacement");
    expect(source_click.ok && source_click.exists &&
            source_get_caption.ok && source_get_caption.exists &&
            other_method.ok && other_method.exists,
        "#778: batch method copy should preserve source methods");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#778: successful batch copies should leave normal visual undo history available");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "GetCaption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Click",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject target collisions");
    expect(batch_result.affected_object_count == 0U,
        "#1004: failed batch method copy should report zero affected objects");
    auto target_temp_caption = method_state("target-guid", "TempCaption");
    expect(target_temp_caption.ok && !target_temp_caption.exists,
        "#778: target-collision failures should roll back earlier method copy targets");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "GetCaption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Missing",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "MissingCopy",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject missing source methods");
    target_temp_caption = method_state("target-guid", "TempCaption");
    expect(target_temp_caption.ok && !target_temp_caption.exists,
        "#778: missing-source failures should roll back earlier method copy targets");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "GetCaption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = " ",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "EmptySource",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject empty source method names");
    target_temp_caption = method_state("target-guid", "TempCaption");
    expect(target_temp_caption.ok && !target_temp_caption.exists,
        "#778: empty-source failures should roll back earlier method copy targets");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "GetCaption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = " ",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject empty target method names");
    target_temp_caption = method_state("target-guid", "TempCaption");
    expect(target_temp_caption.ok && !target_temp_caption.exists,
        "#778: empty-target failures should roll back earlier method copy targets");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "GetCaption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "dup-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "DupClick",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject duplicate source methods");
    target_temp_caption = method_state("target-guid", "TempCaption");
    auto duplicate_click = method_state("dup-guid", "Click");
    expect(target_temp_caption.ok && !target_temp_caption.exists &&
            !duplicate_click.ok,
        "#778: duplicate-source failures should roll back earlier copy targets and report ambiguity");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#778: failed batch copy rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = table_path.string(),
        .methods = {}
    });
    expect(!batch_result.ok, "#778: empty batch method copy requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1004: empty batch method copy should report zero affected objects");

    const fs::path unsupported_path = temp_dir / "method_copy_batch_unsupported.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdUnsupported", "unsupported-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#778: unsupported method-copy-batch fixture should be writable");
    batch_result = copperfin::vfp::copy_visual_object_methods({
        .path = unsupported_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "unsupported-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Click",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#778: batch method copy should reject objects without METHODS carriers");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#778: undo should restore each successful batch method copy target");
    }

    target_click = method_state("target-guid", "Click");
    target_caption_text = method_state("target-guid", "CaptionText");
    target_other_copy = method_state("target-guid", "OtherCopy");
    target_refresh = method_state("target-guid", "Refresh");
    target_existing = method_state("target-guid", "Existing");
    source_click = method_state("source-guid", "Click");
    source_get_caption = method_state("source-guid", "GetCaption");
    other_method = method_state("other-guid", "Other");
    expect(target_click.ok && !target_click.exists &&
            target_caption_text.ok && !target_caption_text.exists &&
            target_other_copy.ok && !target_other_copy.exists &&
            target_refresh.ok && target_refresh.exists && target_refresh.method.source_text == "RETURN .F." &&
            target_existing.ok && target_existing.exists &&
            source_click.ok && source_click.exists &&
            source_get_caption.ok && source_get_caption.exists &&
            other_method.ok && other_method.exists,
        "#778: successful batch copy undo should restore original target state and preserve sources");

    fs::remove_all(temp_dir, ignored);
}

void test_move_visual_object_method_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_move_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_move.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\nFUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#764: method-move fixture should be writable");

    auto move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "click",
        .target_record_index = 0U,
        .target_object_name = "txtTarget",
        .target_unique_id = {},
        .target_method_name = {},
        .replace_existing = false
    });
    expect(move_result.ok, "#764: method move should move procedures by UNIQUEID source and object-name target");
    expect(move_result.affected_object_count == 1U,
        "#1004: successful method move should report one affected object");

    auto source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    auto target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") == nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr,
        "#764: method move should delete only the moved source method");
    const auto* moved_click = target_methods.ok ? find_method_snapshot(target_methods.methods, "Click") : nullptr;
    expect(moved_click != nullptr &&
            moved_click->kind == "procedure" &&
            moved_click->source_text == "THISFORM.Save()" &&
            find_method_snapshot(target_methods.methods, "Existing") != nullptr,
        "#764: method move should append the moved procedure while preserving target methods");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSource",
        .source_unique_id = {},
        .source_method_name = "GetCaption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "CaptionText",
        .replace_existing = false
    });
    expect(move_result.ok, "#764: method move should support record-index targets and target method renames");

    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* moved_caption = target_methods.ok ? find_method_snapshot(target_methods.methods, "CaptionText") : nullptr;
    expect(source_methods.ok && source_methods.methods.empty(),
        "#764: moving the final source method should leave the source METHODS memo empty");
    expect(moved_caption != nullptr &&
            moved_caption->kind == "function" &&
            moved_caption->source_text == "RETURN THIS.Caption",
        "#764: renamed function moves should preserve source body and function kind");

    auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && find_method_snapshot(other_methods.methods, "Other") != nullptr,
        "#764: method move should preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore the moved function source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should remove the moved function target copy");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore the moved procedure source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should remove the moved procedure target copy");

    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    expect(source_methods.ok &&
            find_method_snapshot(source_methods.methods, "Click") != nullptr &&
            find_method_snapshot(source_methods.methods, "GetCaption") != nullptr &&
            target_methods.ok &&
            find_method_snapshot(target_methods.methods, "Click") == nullptr &&
            find_method_snapshot(target_methods.methods, "CaptionText") == nullptr,
        "#764: undo should restore source and target METHODS memos after moves");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Existing",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject target collisions without replacement");
    expect(move_result.affected_object_count == 0U,
        "#1004: failed method move should report zero affected objects");
    source_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    expect(source_methods.ok && find_method_snapshot(source_methods.methods, "Click") != nullptr,
        "#764: failed target copies should leave the source method intact");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_method_name = "Refresh",
        .replace_existing = true
    });
    expect(move_result.ok, "#764: method move should allow explicit replacement");
    target_methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid"
    });
    const auto* refresh = target_methods.ok ? find_method_snapshot(target_methods.methods, "Refresh") : nullptr;
    expect(refresh != nullptr &&
            refresh->kind == "function" &&
            refresh->source_text == "THISFORM.Save()",
        "#764: replacing target methods during move should preserve target declaration kind");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore replacement source deletion");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#764: undo should restore replacement target body");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "source-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject same-source implicit overwrites");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "EmptySource",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject empty source method names");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = " ",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject empty requested target method names");

    move_result = copperfin::vfp::move_visual_object_method({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Missing",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = "Missing",
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject missing source methods");

    const fs::path duplicate_path = temp_dir / "method_move_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        },
        {"txtTarget", "targetBox", "target-guid", ""}
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#764: duplicate method-move fixture should be writable");
    move_result = copperfin::vfp::move_visual_object_method({
        .path = duplicate_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "dup-guid",
        .source_method_name = "CLICK",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject duplicate source declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_move_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdSource", "source-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#764: missing-METHODS method-move fixture should be writable");
    move_result = copperfin::vfp::move_visual_object_method({
        .path = no_methods_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_method_name = "Click",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_method_name = {},
        .replace_existing = false
    });
    expect(!move_result.ok, "#764: method move should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_move_visual_object_methods_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_move_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_move_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\n"
            "FUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\n"
            "PROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC\r\n"
            "PROCEDURE Late\r\nTHISFORM.Late()\r\nENDPROC"
        },
        {
            "txtTarget",
            "targetBox",
            "target-guid",
            "PROCEDURE Existing\r\nTHISFORM.Old()\r\nENDPROC\r\n"
            "FUNCTION Refresh\r\nRETURN .F.\r\nENDFUNC"
        },
        {
            "lblOther",
            "otherLabel",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC\r\n"
            "FUNCTION OtherValue\r\nRETURN 42\r\nENDFUNC"
        },
        {
            "dupObj",
            "dupName",
            "dup-guid",
            "PROCEDURE Click\r\nWAIT WINDOW \"first\"\r\nENDPROC\r\n"
            "PROCEDURE click\r\nWAIT WINDOW \"second\"\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#779: method-move-batch fixture should be writable");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    auto batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "click",
                .target_record_index = 0U,
                .target_object_name = "txtTarget",
                .target_unique_id = {},
                .target_method_name = {},
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = "cmdSource",
                .source_unique_id = {},
                .source_method_name = "GetCaption",
                .target_record_index = 1U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_method_name = "CaptionText",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "other-guid",
                .source_method_name = "Other",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "OtherMoved",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Init",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Refresh",
                .replace_existing = true
            }
        }
    });
    expect(batch_result.ok, "#779: batch method move should support mixed selectors, procedure/function move, target renames, and replacement");
    expect(batch_result.affected_object_count == 4U,
        "#1004: successful batch method move should report affected item count");

    auto source_click = method_state("source-guid", "Click");
    auto source_get_caption = method_state("source-guid", "GetCaption");
    auto source_init = method_state("source-guid", "Init");
    auto source_late = method_state("source-guid", "Late");
    auto other_method = method_state("other-guid", "Other");
    auto other_value = method_state("other-guid", "OtherValue");
    auto target_click = method_state("target-guid", "Click");
    auto target_caption_text = method_state("target-guid", "CaptionText");
    auto target_other_moved = method_state("target-guid", "OtherMoved");
    auto target_refresh = method_state("target-guid", "Refresh");
    auto target_existing = method_state("target-guid", "Existing");
    expect(source_click.ok && !source_click.exists &&
            source_get_caption.ok && !source_get_caption.exists &&
            source_init.ok && !source_init.exists &&
            other_method.ok && !other_method.exists,
        "#779: batch method move should delete moved source declarations");
    expect(target_click.ok && target_click.exists && target_click.method.kind == "procedure" &&
            target_caption_text.ok && target_caption_text.exists && target_caption_text.method.kind == "function" &&
            target_other_moved.ok && target_other_moved.exists && target_other_moved.method.source_text == "THISFORM.Other()" &&
            target_refresh.ok && target_refresh.exists && target_refresh.method.kind == "function" &&
            target_refresh.method.source_text == "THIS.Enabled = .T." &&
            target_existing.ok && target_existing.exists,
        "#779: batch method move should persist targets and preserve target declaration kind on replacement");
    expect(source_late.ok && source_late.exists &&
            other_value.ok && other_value.exists,
        "#779: batch method move should preserve unrelated source methods");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#779: successful batch moves should leave normal visual undo history available");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempLate",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Click",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject target collisions");
    expect(batch_result.affected_object_count == 0U,
        "#1004: failed batch method move should report zero affected objects");
    auto target_temp_late = method_state("target-guid", "TempLate");
    source_late = method_state("source-guid", "Late");
    expect(target_temp_late.ok && !target_temp_late.exists &&
            source_late.ok && source_late.exists,
        "#779: target-collision failures should roll back earlier move targets and source deletes");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempLate",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Missing",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "MissingMove",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject missing source methods");
    target_temp_late = method_state("target-guid", "TempLate");
    source_late = method_state("source-guid", "Late");
    expect(target_temp_late.ok && !target_temp_late.exists &&
            source_late.ok && source_late.exists,
        "#779: missing-source failures should roll back earlier move targets and source deletes");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempLate",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = " ",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "EmptySource",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject empty source method names");
    target_temp_late = method_state("target-guid", "TempLate");
    source_late = method_state("source-guid", "Late");
    expect(target_temp_late.ok && !target_temp_late.exists &&
            source_late.ok && source_late.exists,
        "#779: empty-source failures should roll back earlier move targets and source deletes");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempLate",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = " ",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject empty target method names");
    target_temp_late = method_state("target-guid", "TempLate");
    source_late = method_state("source-guid", "Late");
    expect(target_temp_late.ok && !target_temp_late.exists &&
            source_late.ok && source_late.exists,
        "#779: empty-target failures should roll back earlier move targets and source deletes");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "TempLate",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "dup-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "DupClick",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject duplicate source methods");
    target_temp_late = method_state("target-guid", "TempLate");
    source_late = method_state("source-guid", "Late");
    auto duplicate_click = method_state("dup-guid", "Click");
    expect(target_temp_late.ok && !target_temp_late.exists &&
            source_late.ok && source_late.exists &&
            !duplicate_click.ok,
        "#779: duplicate-source failures should roll back earlier move targets and source deletes");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "source-guid",
                .source_method_name = "Late",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "source-guid",
                .target_method_name = {},
                .replace_existing = true
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject same-object same-method moves");
    source_late = method_state("source-guid", "Late");
    expect(source_late.ok && source_late.exists,
        "#779: self-move failures should preserve the source method");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#779: failed batch move rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = table_path.string(),
        .methods = {}
    });
    expect(!batch_result.ok, "#779: empty batch method move requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1004: empty batch method move should report zero affected objects");

    const fs::path unsupported_path = temp_dir / "method_move_batch_unsupported.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdUnsupported", "unsupported-guid"},
        {"txtTarget", "target-guid"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#779: unsupported method-move-batch fixture should be writable");
    batch_result = copperfin::vfp::move_visual_object_methods({
        .path = unsupported_path.string(),
        .methods = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "unsupported-guid",
                .source_method_name = "Click",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "target-guid",
                .target_method_name = "Click",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#779: batch method move should reject objects without METHODS carriers");

    for (int index = 0; index < 8; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#779: undo should restore each copy/delete step from successful batch method moves");
    }

    source_click = method_state("source-guid", "Click");
    source_get_caption = method_state("source-guid", "GetCaption");
    source_init = method_state("source-guid", "Init");
    source_late = method_state("source-guid", "Late");
    other_method = method_state("other-guid", "Other");
    other_value = method_state("other-guid", "OtherValue");
    target_click = method_state("target-guid", "Click");
    target_caption_text = method_state("target-guid", "CaptionText");
    target_other_moved = method_state("target-guid", "OtherMoved");
    target_refresh = method_state("target-guid", "Refresh");
    target_existing = method_state("target-guid", "Existing");
    expect(source_click.ok && source_click.exists &&
            source_get_caption.ok && source_get_caption.exists &&
            source_init.ok && source_init.exists &&
            source_late.ok && source_late.exists &&
            other_method.ok && other_method.exists &&
            other_value.ok && other_value.exists &&
            target_click.ok && !target_click.exists &&
            target_caption_text.ok && !target_caption_text.exists &&
            target_other_moved.ok && !target_other_moved.exists &&
            target_refresh.ok && target_refresh.exists && target_refresh.method.source_text == "RETURN .F." &&
            target_existing.ok && target_existing.exists,
        "#779: successful batch move undo should restore original source and target method state");

    fs::remove_all(temp_dir, ignored);
}

#include "test_visual_asset_editor_method_reorder_contracts.inl"

}  // namespace cf_test_visual_asset_editor
