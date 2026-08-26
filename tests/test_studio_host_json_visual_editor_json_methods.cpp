// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#include "test_studio_host_json_visual_editor_json_methods_inspection.inl"

#include "test_studio_host_json_visual_editor_json_methods_update.inl"

#include "test_studio_host_json_visual_editor_json_methods_delete.inl"

void test_studio_host_json_deletes_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_delete_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-delete-batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\nPROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC\r\nFUNCTION Valid\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Paint\r\nTHIS.Refresh()\r\nENDPROC\r\nFUNCTION RefreshValue\r\nRETURN THIS.Caption\r\nENDFUNC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1430: synthetic SCX table for visual method delete batches should be created");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    const auto delete_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--unique-id", "save-guid",
            "--method-name", "LostFocus",
            "--object-name", "txtName",
            "--method-name", "RefreshValue",
            "--record", "2",
            "--json"
        },
        temp_root);
    expect(delete_batch_process.exit_code == 0,
        "#1430: visual method delete-batch JSON should exit successfully for valid batches");
    expect_contains(delete_batch_process.stdout_text, "\"visualMethodDeleteBatch\": {",
        "#1430: visual method delete-batch JSON should expose a batch delete object");
    expect_contains(delete_batch_process.stdout_text, "\"affectedObjectCount\": 3",
        "#1430: visual method delete-batch JSON should expose affected item counts");
    expect_contains(delete_batch_process.stdout_text, "\"dryRun\": false",
        "#1430: visual method delete-batch JSON should expose committed execution state");
    expect_contains(delete_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1430: visual method delete-batch JSON should expose mutation state");
    expect_contains(delete_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1430: visual method delete-batch JSON should expose undo availability");
    auto save_click = method_state("save-guid", "Click");
    auto name_lost_focus = method_state("name-guid", "LostFocus");
    auto status_refresh_value = method_state("status-guid", "RefreshValue");
    auto save_init = method_state("save-guid", "Init");
    auto save_get_caption = method_state("save-guid", "GetCaption");
    auto name_valid = method_state("name-guid", "Valid");
    auto status_paint = method_state("status-guid", "Paint");
    expect(save_click.ok && !save_click.exists &&
            name_lost_focus.ok && !name_lost_focus.exists &&
            status_refresh_value.ok && !status_refresh_value.exists,
        "#1430: visual method delete-batch host command should delete all requested methods");
    expect(save_init.ok && save_init.exists &&
            save_get_caption.ok && save_get_caption.exists &&
            name_valid.ok && name_valid.exists &&
            status_paint.ok && status_paint.exists,
        "#1430: visual method delete-batch host command should preserve unrelated methods");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1430: visual method delete-batch JSON should reject missing methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: failed visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(rollback_process.stdout_text, "The requested method was not found.",
        "#1430: missing-method visual method delete-batch JSON should report editor errors");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#1430: failed visual method delete-batch commands should roll back earlier deletes");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--method-name", "Init",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: missing-path visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1430: missing-path visual method delete-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method deletes were provided.",
        "#1430: empty visual method delete-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Init",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method delete batch item options require a preceding --method-name.",
        "#1430: option-before-item visual method delete-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1430: visual method delete-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1430: invalid-record visual method delete-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-delete-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1430: visual method delete-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodDeleteBatch\": null",
        "#1430: unresolved visual method delete-batch JSON should not expose a batch delete object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1430: unresolved visual method delete-batch JSON should report editor errors");
    save_init = method_state("save-guid", "Init");
    expect(save_init.ok && save_init.exists,
        "#1430: failed visual method delete-batch selection errors should not mutate methods");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-delete-batch --path <asset>",
        "#1430: usage text should expose visual method delete-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_rename_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-rename-batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC\r\nPROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC\r\nFUNCTION Valid\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Paint\r\nTHIS.Refresh()\r\nENDPROC\r\nFUNCTION RefreshValue\r\nRETURN THIS.Caption\r\nENDFUNC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1431: synthetic SCX table for visual method rename batches should be created");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    const auto rename_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--new-method-name", "SaveClick",
            "--unique-id", "save-guid",
            "--method-name", "Valid",
            "--new-method-name", "IsValid",
            "--object-name", "txtName",
            "--method-name", "Paint",
            "--new-method-name", "PaintStatus",
            "--record", "2",
            "--json"
        },
        temp_root);
    expect(rename_batch_process.exit_code == 0,
        "#1431: visual method rename-batch JSON should exit successfully for valid batches");
    expect_contains(rename_batch_process.stdout_text, "\"visualMethodRenameBatch\": {",
        "#1431: visual method rename-batch JSON should expose a batch rename object");
    expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 3",
        "#1431: visual method rename-batch JSON should expose affected item counts");
    expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
        "#1431: visual method rename-batch JSON should expose committed execution state");
    expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1431: visual method rename-batch JSON should expose mutation state");
    expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1431: visual method rename-batch JSON should expose undo availability");
    auto save_click = method_state("save-guid", "Click");
    auto save_save_click = method_state("save-guid", "SaveClick");
    auto name_valid = method_state("name-guid", "Valid");
    auto name_is_valid = method_state("name-guid", "IsValid");
    auto status_paint = method_state("status-guid", "Paint");
    auto status_paint_status = method_state("status-guid", "PaintStatus");
    auto save_init = method_state("save-guid", "Init");
    auto save_get_caption = method_state("save-guid", "GetCaption");
    auto name_lost_focus = method_state("name-guid", "LostFocus");
    auto status_refresh_value = method_state("status-guid", "RefreshValue");
    expect(save_click.ok && !save_click.exists &&
            name_valid.ok && !name_valid.exists &&
            status_paint.ok && !status_paint.exists &&
            save_save_click.ok && save_save_click.exists && save_save_click.method.kind == "procedure" &&
            name_is_valid.ok && name_is_valid.exists && name_is_valid.method.kind == "function" &&
            status_paint_status.ok && status_paint_status.exists && status_paint_status.method.kind == "procedure",
        "#1431: visual method rename-batch host command should rename all requested methods");
    expect(save_init.ok && save_init.exists &&
            save_get_caption.ok && save_get_caption.exists &&
            name_lost_focus.ok && name_lost_focus.exists &&
            status_refresh_value.ok && status_refresh_value.exists,
        "#1431: visual method rename-batch host command should preserve unrelated methods");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--new-method-name", "StartUp",
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--new-method-name", "MissingRenamed",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1431: visual method rename-batch JSON should reject missing methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodRenameBatch\": null",
        "#1431: failed visual method rename-batch JSON should not expose a batch rename object");
    expect_contains(rollback_process.stdout_text, "The requested method was not found.",
        "#1431: missing-method visual method rename-batch JSON should report editor errors");
    save_init = method_state("save-guid", "Init");
    auto save_start_up = method_state("save-guid", "StartUp");
    expect(save_init.ok && save_init.exists &&
            save_start_up.ok && !save_start_up.exists,
        "#1431: failed visual method rename-batch commands should roll back earlier renames");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "GetCaption",
            "--new-method-name", "SaveClick",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1431: visual method rename-batch JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "The requested target method already exists.",
        "#1431: target-collision visual method rename-batch JSON should report editor errors");
    save_get_caption = method_state("save-guid", "GetCaption");
    expect(save_get_caption.ok && save_get_caption.exists,
        "#1431: failed visual method rename-batch target collisions should not mutate methods");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--method-name", "Init",
            "--new-method-name", "StartUp",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1431: visual method rename-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodRenameBatch\": null",
        "#1431: missing-path visual method rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1431: missing-path visual method rename-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1431: visual method rename-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method renames were provided.",
        "#1431: empty visual method rename-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--new-method-name", "StartUp",
            "--method-name", "Init",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1431: visual method rename-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method rename batch item options require a preceding --method-name.",
        "#1431: option-before-item visual method rename-batch JSON should report parser errors");

    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 2,
        "#1431: visual method rename-batch JSON should reject missing target method names");
    expect_contains(missing_target_process.stdout_text, "No target method name was provided.",
        "#1431: missing target visual method rename-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--new-method-name", "StartUp",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1431: visual method rename-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1431: invalid-record visual method rename-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--new-method-name", "StartUp",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1431: visual method rename-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodRenameBatch\": null",
        "#1431: unresolved visual method rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1431: unresolved visual method rename-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-rename-batch --path <asset>",
        "#1431: usage text should expose visual method rename-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-rename.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1426: synthetic SCX table for visual method rename should be created");

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--new-method-name", "DoClick",
            "--json"
        },
        temp_root);
    expect(rename_process.exit_code == 0,
        "#1426: visual method rename JSON should exit successfully for existing methods");
    expect_contains(rename_process.stdout_text, "\"visualMethodRename\": {",
        "#1426: visual method rename JSON should expose a rename object");
    expect_contains(rename_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1426: visual method rename JSON should expose affected object counts");
    expect_contains(rename_process.stdout_text, "\"dryRun\": false",
        "#1426: visual method rename JSON should expose committed execution state");
    expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
        "#1426: visual method rename JSON should expose mutation state");
    expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
        "#1426: visual method rename JSON should expose undo availability");
    auto renamed_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoClick"
    });
    expect(renamed_method.ok && renamed_method.exists &&
            renamed_method.method.kind == "procedure" &&
            renamed_method.method.source_text == "THISFORM.Save()",
        "#1426: visual method rename host command should rename selected methods and preserve bodies");
    auto old_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(old_method.ok && !old_method.exists,
        "#1426: visual method rename host command should remove old method declarations");
    auto can_save_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanSave"
    });
    expect(can_save_method.ok && can_save_method.exists && can_save_method.method.source_text == "RETURN .T.",
        "#1426: visual method rename host command should preserve unrelated methods");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "DoClick",
            "--new-method-name", "CanSave",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1426: visual method rename JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "\"visualMethodRename\": null",
        "#1426: target-collision visual method rename JSON should not expose a rename object");
    expect_contains(collision_process.stdout_text, "The requested target method already exists.",
        "#1426: target-collision visual method rename JSON should report editor errors");
    renamed_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoClick"
    });
    expect(renamed_method.ok && renamed_method.exists,
        "#1426: failed visual method rename commands should not remove source methods");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--unique-id", "save-guid",
            "--method-name", "DoClick",
            "--new-method-name", "Clicked",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1426: visual method rename JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodRename\": null",
        "#1426: missing-path visual method rename JSON should not expose a rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1426: missing-path visual method rename JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--new-method-name", "Clicked",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1426: visual method rename JSON should reject missing source method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1426: missing source method-name visual method rename JSON should report parser errors");

    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "DoClick",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 2,
        "#1426: visual method rename JSON should reject missing target method names");
    expect_contains(missing_target_process.stdout_text, "No target method name was provided.",
        "#1426: missing target method-name visual method rename JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--record", "-1",
            "--method-name", "DoClick",
            "--new-method-name", "Clicked",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1426: visual method rename JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1426: invalid-record visual method rename JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-rename",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--method-name", "DoClick",
            "--new-method-name", "Clicked",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1426: visual method rename JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodRename\": null",
        "#1426: unresolved visual method rename JSON should not expose a rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1426: unresolved visual method rename JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-rename --path <asset>",
        "#1426: usage text should expose visual method rename commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_copies_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_copy_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-copy.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "cmdCancel",
            "cancelButton",
            "cancel-guid",
            "PROCEDURE Cancel\r\nTHISFORM.Cancel()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1427: synthetic SCX table for visual method copy should be created");

    const auto copy_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "CopiedClick",
            "--json"
        },
        temp_root);
    expect(copy_process.exit_code == 0,
        "#1427: visual method copy JSON should exit successfully for existing methods");
    expect_contains(copy_process.stdout_text, "\"visualMethodCopy\": {",
        "#1427: visual method copy JSON should expose a copy object");
    expect_contains(copy_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1427: visual method copy JSON should expose affected object counts");
    expect_contains(copy_process.stdout_text, "\"dryRun\": false",
        "#1427: visual method copy JSON should expose committed execution state");
    expect_contains(copy_process.stdout_text, "\"mutatesAsset\": true",
        "#1427: visual method copy JSON should expose mutation state");
    expect_contains(copy_process.stdout_text, "\"undoAvailable\": true",
        "#1427: visual method copy JSON should expose undo availability");
    auto copied_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "CopiedClick"
    });
    expect(copied_method.ok && copied_method.exists &&
            copied_method.method.kind == "procedure" &&
            copied_method.method.source_text == "THISFORM.Save()",
        "#1427: visual method copy host command should copy source method bodies to target objects");
    auto source_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(source_method.ok && source_method.exists &&
            source_method.method.source_text == "THISFORM.Save()",
        "#1427: visual method copy host command should preserve source methods");

    const auto default_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "CanSave",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(default_name_process.exit_code == 0,
        "#1427: visual method copy JSON should allow target method names to default from the source");
    auto default_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "CanSave"
    });
    expect(default_method.ok && default_method.exists &&
            default_method.method.kind == "function" &&
            default_method.method.source_text == "RETURN .T.",
        "#1427: visual method copy host command should default target method names from source names");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1427: visual method copy JSON should reject target method collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualMethodCopy\": null",
        "#1427: target-collision visual method copy JSON should not expose a copy object");
    expect_contains(collision_process.stdout_text, "The target object already has a method with the requested name.",
        "#1427: target-collision visual method copy JSON should report editor errors");
    auto cancel_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "Cancel"
    });
    expect(cancel_method.ok && cancel_method.exists &&
            cancel_method.method.source_text == "THISFORM.Cancel()",
        "#1427: failed visual method copy commands should not mutate target methods");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1427: visual method copy JSON should allow explicit target replacement");
    cancel_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "Cancel"
    });
    expect(cancel_method.ok && cancel_method.exists &&
            cancel_method.method.source_text == "THISFORM.Save()",
        "#1427: visual method copy host command should replace existing targets when requested");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1427: visual method copy JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodCopy\": null",
        "#1427: missing-path visual method copy JSON should not expose a copy object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1427: missing-path visual method copy JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1427: visual method copy JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1427: missing method-name visual method copy JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-record", "-1",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1427: visual method copy JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1427: invalid-source-record visual method copy JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1427: visual method copy JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1427: invalid replace-existing visual method copy JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy",
            "--path", form_path.string(),
            "--source-object-name", "missingObject",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1427: visual method copy JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodCopy\": null",
        "#1427: unresolved visual method copy JSON should not expose a copy object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1427: unresolved visual method copy JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-copy --path <asset>",
        "#1427: usage text should expose visual method copy commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_copies_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_copy_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-copy-batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC\r\nPROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "cmdCancel",
            "cancelButton",
            "cancel-guid",
            "PROCEDURE Cancel\r\nTHISFORM.Cancel()\r\nENDPROC"
        },
        {
            "cmdOther",
            "otherButton",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1432: synthetic SCX table for visual method copy batches should be created");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    const auto copy_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "CopiedClick",
            "--method-name", "CanSave",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--method-name", "Other",
            "--source-unique-id", "other-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "CopiedOther",
            "--json"
        },
        temp_root);
    expect(copy_batch_process.exit_code == 0,
        "#1432: visual method copy-batch JSON should exit successfully for valid batches");
    expect_contains(copy_batch_process.stdout_text, "\"visualMethodCopyBatch\": {",
        "#1432: visual method copy-batch JSON should expose a batch copy object");
    expect_contains(copy_batch_process.stdout_text, "\"affectedObjectCount\": 3",
        "#1432: visual method copy-batch JSON should expose affected item counts");
    expect_contains(copy_batch_process.stdout_text, "\"dryRun\": false",
        "#1432: visual method copy-batch JSON should expose committed execution state");
    expect_contains(copy_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1432: visual method copy-batch JSON should expose mutation state");
    expect_contains(copy_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1432: visual method copy-batch JSON should expose undo availability");
    auto copied_click = method_state("cancel-guid", "CopiedClick");
    auto copied_can_save = method_state("cancel-guid", "CanSave");
    auto copied_other = method_state("cancel-guid", "CopiedOther");
    auto source_click = method_state("save-guid", "Click");
    auto source_can_save = method_state("save-guid", "CanSave");
    auto source_other = method_state("other-guid", "Other");
    expect(copied_click.ok && copied_click.exists && copied_click.method.source_text == "THISFORM.Save()" &&
            copied_can_save.ok && copied_can_save.exists && copied_can_save.method.kind == "function" &&
            copied_other.ok && copied_other.exists && copied_other.method.source_text == "THISFORM.Other()",
        "#1432: visual method copy-batch host command should copy all requested methods");
    expect(source_click.ok && source_click.exists &&
            source_can_save.ok && source_can_save.exists &&
            source_other.ok && source_other.exists,
        "#1432: visual method copy-batch host command should preserve source methods");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "CopiedInit",
            "--method-name", "MissingMethod",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "CopiedMissing",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1432: visual method copy-batch JSON should reject missing source methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodCopyBatch\": null",
        "#1432: failed visual method copy-batch JSON should not expose a batch copy object");
    expect_contains(rollback_process.stdout_text, "The source method was not found.",
        "#1432: missing-source visual method copy-batch JSON should report editor errors");
    auto copied_init = method_state("cancel-guid", "CopiedInit");
    expect(copied_init.ok && !copied_init.exists,
        "#1432: failed visual method copy-batch commands should roll back earlier copies");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1432: visual method copy-batch JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "The target object already has a method with the requested name.",
        "#1432: target-collision visual method copy-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1432: visual method copy-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodCopyBatch\": null",
        "#1432: missing-path visual method copy-batch JSON should not expose a batch copy object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1432: missing-path visual method copy-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1432: visual method copy-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method copies were provided.",
        "#1432: empty visual method copy-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Init",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1432: visual method copy-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method copy batch item options require a preceding --method-name.",
        "#1432: option-before-item visual method copy-batch JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-record", "-1",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1432: visual method copy-batch JSON should reject invalid source records");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1432: invalid-source-record visual method copy-batch JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1432: visual method copy-batch JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1432: invalid replace-existing visual method copy-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-copy-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-object-name", "missingObject",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1432: visual method copy-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodCopyBatch\": null",
        "#1432: unresolved visual method copy-batch JSON should not expose a batch copy object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1432: unresolved visual method copy-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-copy-batch --path <asset>",
        "#1432: usage text should expose visual method copy-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_move_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-move-batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC\r\nPROCEDURE Init\r\nTHIS.Enabled = .T.\r\nENDPROC"
        },
        {
            "cmdCancel",
            "cancelButton",
            "cancel-guid",
            "PROCEDURE Cancel\r\nTHISFORM.Cancel()\r\nENDPROC"
        },
        {
            "cmdOther",
            "otherButton",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1433: synthetic SCX table for visual method move batches should be created");

    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };

    const auto move_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Click",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "MovedClick",
            "--method-name", "CanSave",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--method-name", "Other",
            "--source-unique-id", "other-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "MovedOther",
            "--json"
        },
        temp_root);
    expect(move_batch_process.exit_code == 0,
        "#1433: visual method move-batch JSON should exit successfully for valid batches");
    expect_contains(move_batch_process.stdout_text, "\"visualMethodMoveBatch\": {",
        "#1433: visual method move-batch JSON should expose a batch move object");
    expect_contains(move_batch_process.stdout_text, "\"affectedObjectCount\": 3",
        "#1433: visual method move-batch JSON should expose affected item counts");
    expect_contains(move_batch_process.stdout_text, "\"dryRun\": false",
        "#1433: visual method move-batch JSON should expose committed execution state");
    expect_contains(move_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1433: visual method move-batch JSON should expose mutation state");
    expect_contains(move_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1433: visual method move-batch JSON should expose undo availability");
    auto moved_click = method_state("cancel-guid", "MovedClick");
    auto moved_can_save = method_state("cancel-guid", "CanSave");
    auto moved_other = method_state("cancel-guid", "MovedOther");
    auto source_click = method_state("save-guid", "Click");
    auto source_can_save = method_state("save-guid", "CanSave");
    auto source_other = method_state("other-guid", "Other");
    auto source_init = method_state("save-guid", "Init");
    expect(moved_click.ok && moved_click.exists && moved_click.method.source_text == "THISFORM.Save()" &&
            moved_can_save.ok && moved_can_save.exists && moved_can_save.method.kind == "function" &&
            moved_other.ok && moved_other.exists && moved_other.method.source_text == "THISFORM.Other()",
        "#1433: visual method move-batch host command should move all requested methods to targets");
    expect(source_click.ok && !source_click.exists &&
            source_can_save.ok && !source_can_save.exists &&
            source_other.ok && !source_other.exists &&
            source_init.ok && source_init.exists,
        "#1433: visual method move-batch host command should remove moved source methods and preserve unrelated sources");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "MovedInit",
            "--method-name", "MissingMethod",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "MovedMissing",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1433: visual method move-batch JSON should reject missing source methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodMoveBatch\": null",
        "#1433: failed visual method move-batch JSON should not expose a batch move object");
    expect_contains(rollback_process.stdout_text, "The source method was not found.",
        "#1433: missing-source visual method move-batch JSON should report editor errors");
    auto moved_init = method_state("cancel-guid", "MovedInit");
    source_init = method_state("save-guid", "Init");
    expect(moved_init.ok && !moved_init.exists &&
            source_init.ok && source_init.exists,
        "#1433: failed visual method move-batch commands should roll back target copies and source deletes");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1433: visual method move-batch JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "The target object already has a method with the requested name.",
        "#1433: target-collision visual method move-batch JSON should report editor errors");
    source_init = method_state("save-guid", "Init");
    expect(source_init.ok && source_init.exists,
        "#1433: failed visual method move-batch target collisions should not delete source methods");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1433: visual method move-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodMoveBatch\": null",
        "#1433: missing-path visual method move-batch JSON should not expose a batch move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1433: missing-path visual method move-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1433: visual method move-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method moves were provided.",
        "#1433: empty visual method move-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Init",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1433: visual method move-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method move batch item options require a preceding --method-name.",
        "#1433: option-before-item visual method move-batch JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-record", "-1",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1433: visual method move-batch JSON should reject invalid source records");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1433: invalid-source-record visual method move-batch JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1433: visual method move-batch JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1433: invalid replace-existing visual method move-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move-batch",
            "--path", form_path.string(),
            "--method-name", "Init",
            "--source-object-name", "missingObject",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1433: visual method move-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodMoveBatch\": null",
        "#1433: unresolved visual method move-batch JSON should not expose a batch move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1433: unresolved visual method move-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-move-batch --path <asset>",
        "#1433: usage text should expose visual method move-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_move_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-move.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION CanSave\r\nRETURN .T.\r\nENDFUNC"
        },
        {
            "cmdCancel",
            "cancelButton",
            "cancel-guid",
            "PROCEDURE Cancel\r\nTHISFORM.Cancel()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1428: synthetic SCX table for visual method move should be created");

    const auto move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "MovedClick",
            "--json"
        },
        temp_root);
    expect(move_process.exit_code == 0,
        "#1428: visual method move JSON should exit successfully for existing methods");
    expect_contains(move_process.stdout_text, "\"visualMethodMove\": {",
        "#1428: visual method move JSON should expose a move object");
    expect_contains(move_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1428: visual method move JSON should expose affected object counts");
    expect_contains(move_process.stdout_text, "\"dryRun\": false",
        "#1428: visual method move JSON should expose committed execution state");
    expect_contains(move_process.stdout_text, "\"mutatesAsset\": true",
        "#1428: visual method move JSON should expose mutation state");
    expect_contains(move_process.stdout_text, "\"undoAvailable\": true",
        "#1428: visual method move JSON should expose undo availability");
    auto moved_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "MovedClick"
    });
    expect(moved_method.ok && moved_method.exists &&
            moved_method.method.kind == "procedure" &&
            moved_method.method.source_text == "THISFORM.Save()",
        "#1428: visual method move host command should move source method bodies to target objects");
    auto source_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Click"
    });
    expect(source_method.ok && !source_method.exists,
        "#1428: visual method move host command should remove moved source methods");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "CanSave",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1428: visual method move JSON should reject target method collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualMethodMove\": null",
        "#1428: target-collision visual method move JSON should not expose a move object");
    expect_contains(collision_process.stdout_text, "The target object already has a method with the requested name.",
        "#1428: target-collision visual method move JSON should report editor errors");
    source_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanSave"
    });
    auto cancel_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "Cancel"
    });
    expect(source_method.ok && source_method.exists &&
            cancel_method.ok && cancel_method.exists &&
            cancel_method.method.source_text == "THISFORM.Cancel()",
        "#1428: failed visual method move commands should not mutate source or target methods");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "CanSave",
            "--target-unique-id", "cancel-guid",
            "--target-method-name", "Cancel",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1428: visual method move JSON should allow explicit target replacement");
    source_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "CanSave"
    });
    cancel_method = copperfin::vfp::query_visual_object_method({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "cancel-guid",
        .method_name = "Cancel"
    });
    expect(source_method.ok && !source_method.exists &&
            cancel_method.ok && cancel_method.exists &&
            cancel_method.method.source_text == "RETURN .T.",
        "#1428: visual method move host command should replace existing targets and remove sources when requested");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--source-unique-id", "save-guid",
            "--method-name", "CanSave",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1428: visual method move JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodMove\": null",
        "#1428: missing-path visual method move JSON should not expose a move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1428: missing-path visual method move JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1428: visual method move JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1428: missing method-name visual method move JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-record", "-1",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1428: visual method move JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1428: invalid-source-record visual method move JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-unique-id", "save-guid",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1428: visual method move JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1428: invalid replace-existing visual method move JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-move",
            "--path", form_path.string(),
            "--source-object-name", "missingObject",
            "--method-name", "Click",
            "--target-unique-id", "cancel-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1428: visual method move JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodMove\": null",
        "#1428: unresolved visual method move JSON should not expose a move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1428: unresolved visual method move JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-move --path <asset>",
        "#1428: usage text should expose visual method move commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_visual_methods(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Alpha\r\nTHISFORM.Alpha()\r\nENDPROC\r\nFUNCTION Bravo\r\nRETURN .T.\r\nENDFUNC\r\nPROCEDURE Charlie\r\nTHISFORM.Charlie()\r\nENDPROC"
        },
        {
            "cmdOther",
            "otherButton",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1429: synthetic SCX table for visual method reorder should be created");

    const auto method_order = [&]() {
        std::vector<std::string> names;
        const auto methods = copperfin::vfp::list_visual_object_methods({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "save-guid"
        });
        if (!methods.ok) {
            return names;
        }
        for (const auto& method : methods.methods) {
            names.push_back(method.method_name);
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names,
                             std::initializer_list<const char*> expected) {
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

    const auto first_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Charlie",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(first_process.exit_code == 0,
        "#1429: visual method reorder JSON should exit successfully for first placement");
    expect_contains(first_process.stdout_text, "\"visualMethodReorder\": {",
        "#1429: visual method reorder JSON should expose a reorder object");
    expect_contains(first_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1429: visual method reorder JSON should expose affected object counts");
    expect_contains(first_process.stdout_text, "\"dryRun\": false",
        "#1429: visual method reorder JSON should expose committed execution state");
    expect_contains(first_process.stdout_text, "\"mutatesAsset\": true",
        "#1429: visual method reorder JSON should expose mutation state");
    expect_contains(first_process.stdout_text, "\"undoAvailable\": true",
        "#1429: visual method reorder JSON should expose undo availability");
    expect(order_is(method_order(), {"Charlie", "Alpha", "Bravo"}),
        "#1429: visual method reorder host command should move methods to the front");

    const auto after_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--placement", "after",
            "--relative-method-name", "Bravo",
            "--json"
        },
        temp_root);
    expect(after_process.exit_code == 0,
        "#1429: visual method reorder JSON should exit successfully for relative placement");
    expect(order_is(method_order(), {"Charlie", "Bravo", "Alpha"}),
        "#1429: visual method reorder host command should support after-relative placement");

    const auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && other_methods.methods.size() == 1U &&
            other_methods.methods.front().method_name == "Other",
        "#1429: visual method reorder host command should preserve unrelated objects");

    const auto missing_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--placement", "before",
            "--json"
        },
        temp_root);
    expect(missing_relative_process.exit_code == 4,
        "#1429: visual method reorder JSON should reject missing relative method names for before placement");
    expect_contains(missing_relative_process.stdout_text, "\"visualMethodReorder\": null",
        "#1429: missing-relative visual method reorder JSON should not expose a reorder object");
    expect_contains(missing_relative_process.stdout_text, "No relative method name was provided.",
        "#1429: missing-relative visual method reorder JSON should report editor errors");
    expect(order_is(method_order(), {"Charlie", "Bravo", "Alpha"}),
        "#1429: failed visual method reorder commands should not mutate method order");

    const auto unknown_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--placement", "middle",
            "--json"
        },
        temp_root);
    expect(unknown_placement_process.exit_code == 4,
        "#1429: visual method reorder JSON should reject unknown placements");
    expect_contains(unknown_placement_process.stdout_text, "Unknown method placement was requested.",
        "#1429: unknown-placement visual method reorder JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1429: visual method reorder JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodReorder\": null",
        "#1429: missing-path visual method reorder JSON should not expose a reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1429: missing-path visual method reorder JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1429: visual method reorder JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1429: missing method-name visual method reorder JSON should report parser errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 2,
        "#1429: visual method reorder JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "No method placement was provided.",
        "#1429: missing placement visual method reorder JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--record", "-1",
            "--method-name", "Alpha",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1429: visual method reorder JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1429: invalid-record visual method reorder JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--method-name", "Alpha",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1429: visual method reorder JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodReorder\": null",
        "#1429: unresolved visual method reorder JSON should not expose a reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1429: unresolved visual method reorder JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-reorder --path <asset>",
        "#1429: usage text should expose visual method reorder commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_visual_method_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_reorder_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-reorder-batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Alpha\r\nTHISFORM.Alpha()\r\nENDPROC\r\nFUNCTION Bravo\r\nRETURN .T.\r\nENDFUNC\r\nPROCEDURE Charlie\r\nTHISFORM.Charlie()\r\nENDPROC"
        },
        {
            "cmdOther",
            "otherButton",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1434: synthetic SCX table for visual method reorder batches should be created");

    const auto method_order = [&]() {
        std::vector<std::string> names;
        const auto methods = copperfin::vfp::list_visual_object_methods({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "save-guid"
        });
        if (!methods.ok) {
            return names;
        }
        for (const auto& method : methods.methods) {
            names.push_back(method.method_name);
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names,
                             std::initializer_list<const char*> expected) {
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

    const auto reorder_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Charlie",
            "--placement", "first",
            "--unique-id", "save-guid",
            "--method-name", "Alpha",
            "--placement", "after",
            "--relative-method-name", "Bravo",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(reorder_batch_process.exit_code == 0,
        "#1434: visual method reorder-batch JSON should exit successfully for valid batches");
    expect_contains(reorder_batch_process.stdout_text, "\"visualMethodReorderBatch\": {",
        "#1434: visual method reorder-batch JSON should expose a batch reorder object");
    expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1434: visual method reorder-batch JSON should expose affected item counts");
    expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
        "#1434: visual method reorder-batch JSON should expose committed execution state");
    expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1434: visual method reorder-batch JSON should expose mutation state");
    expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1434: visual method reorder-batch JSON should expose undo availability");
    expect(order_is(method_order(), {"Charlie", "Bravo", "Alpha"}),
        "#1434: visual method reorder-batch host command should apply ordered batch placements");

    const auto other_methods = copperfin::vfp::list_visual_object_methods({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid"
    });
    expect(other_methods.ok && other_methods.methods.size() == 1U &&
            other_methods.methods.front().method_name == "Other",
        "#1434: visual method reorder-batch host command should preserve unrelated objects");

    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Alpha",
            "--placement", "first",
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--placement", "last",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1434: visual method reorder-batch JSON should reject missing methods");
    expect_contains(rollback_process.stdout_text, "\"visualMethodReorderBatch\": null",
        "#1434: failed visual method reorder-batch JSON should not expose a batch reorder object");
    expect_contains(rollback_process.stdout_text, "The requested method was not found.",
        "#1434: missing-method visual method reorder-batch JSON should report editor errors");
    expect(order_is(method_order(), {"Charlie", "Bravo", "Alpha"}),
        "#1434: failed visual method reorder-batch commands should roll back earlier reorders");

    const auto missing_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Alpha",
            "--placement", "before",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_relative_process.exit_code == 4,
        "#1434: visual method reorder-batch JSON should reject missing relative names for before placement");
    expect_contains(missing_relative_process.stdout_text, "No relative method name was provided.",
        "#1434: missing-relative visual method reorder-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--method-name", "Alpha",
            "--placement", "first",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1434: visual method reorder-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodReorderBatch\": null",
        "#1434: missing-path visual method reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1434: missing-path visual method reorder-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1434: visual method reorder-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No method reorders were provided.",
        "#1434: empty visual method reorder-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--placement", "first",
            "--method-name", "Alpha",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1434: visual method reorder-batch JSON should reject item options before method names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual method reorder batch item options require a preceding --method-name.",
        "#1434: option-before-item visual method reorder-batch JSON should report parser errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Alpha",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 2,
        "#1434: visual method reorder-batch JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "No method placement was provided.",
        "#1434: missing placement visual method reorder-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Alpha",
            "--placement", "first",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1434: visual method reorder-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1434: invalid-record visual method reorder-batch JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-reorder-batch",
            "--path", form_path.string(),
            "--method-name", "Alpha",
            "--placement", "first",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1434: visual method reorder-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodReorderBatch\": null",
        "#1434: unresolved visual method reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1434: unresolved visual method reorder-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-reorder-batch --path <asset>",
        "#1434: usage text should expose visual method reorder-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
