// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_form_table_for_deleted_states(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "cmdSave", "save-guid"},
        {"txtName", "txtName", "name-guid"},
        {"lblStatus", "lblStatus", "status-guid"},
        {"dupControl", "dupOne", "dup-one-guid"},
        {"dupControl", "dupTwo", "dup-two-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1201: synthetic SCX table for deleted-states should be created");
}

void mark_deleted_for_deleted_states_fixture(const std::filesystem::path& form_path,
                                             const std::string& unique_id,
                                             bool deleted,
                                             const char* message) {
    const auto delete_result = copperfin::vfp::set_visual_object_deleted_state({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = unique_id,
        .deleted = deleted
    });
    expect(delete_result.ok, message);
}

void write_synthetic_form_table_for_subtree_deleted_state(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cntMain", "cntMain", "container-guid", "", ""},
        {"cmdSave", "cmdSave", "save-guid", "cntMain", ""},
        {"txtName", "txtName", "name-guid", "cntMain", ""},
        {"lblNested", "lblNested", "nested-guid", "txtName", ""},
        {"cmdOther", "cmdOther", "other-guid", "", ""},
        {"", "", "nameless-guid", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1202: synthetic SCX table for subtree deleted-state should be created");
}

std::filesystem::path write_synthetic_form_table_for_property_rename(
    const std::filesystem::path& temp_root,
    const std::string& file_name) {
    const std::filesystem::path form_path = temp_root / file_name;
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmCustomer", "TextBox", "TextBox",
            "ControlSource = \"customer.name\"\r\nLeft = 12\r\n"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1022: synthetic SCX table for property rename should be created");
    return form_path;
}

void write_synthetic_form_table_for_object_reparent(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cntPanel", "cntPanel", "panel-guid", "frmCustomer", "Container", "Container", ""},
        {"txt1", "txt1", "existing-textbox-guid", "frmCustomer", "TextBox", "TextBox", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1027: synthetic SCX table for object reparent should be created");
}

void write_synthetic_form_table_for_object_reorder(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdA", "cmdA", "a-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdB", "cmdB", "b-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdC", "cmdC", "c-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"cmdD", "cmdD", "d-guid", "frmCustomer", "CommandButton", "CommandButton", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1028: synthetic SCX table for object reorder should be created");
}

void write_synthetic_form_table_for_object_group(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cmdSave", "cmdSave", "save-guid", "frmCustomer", "CommandButton", "CommandButton", ""},
        {"txtName", "txtName", "name-guid", "frmCustomer", "TextBox", "TextBox", ""},
        {"lblStatus", "lblStatus", "status-guid", "frmCustomer", "Label", "Label", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1030: synthetic SCX table for object group should be created");
}

void write_synthetic_form_table_for_object_delete_mark(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "DELETEMARK", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "one-guid", ".T."},
        {"frmOrder", "frmOrder", "two-guid", ".T."},
        {"cntDetails", "cntDetails", "three-guid", ".F."},
        {"frmOther", "frmOther", "other-guid", ".T."}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1087: synthetic SCX table for object delete mark should be created");
}

void write_synthetic_form_table_for_object_ungroup(const std::filesystem::path& form_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "NAME", .type = 'C', .length = 24U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U},
        {.name = "PARENT", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"frmCustomer", "frmCustomer", "form-guid", "", "Form", "Form", ""},
        {"cntGroup", "cntGroup", "group-guid", "frmCustomer", "Container", "Container", ""},
        {"txtName", "txtName", "name-guid", "cntGroup", "TextBox", "TextBox", ""},
        {"cmdSave", "cmdSave", "save-guid", "cntGroup", "CommandButton", "CommandButton", ""},
        {"cntRoot", "cntRoot", "root-group-guid", "", "Container", "Container", ""},
        {"txtRoot", "txtRoot", "root-child-guid", "cntRoot", "TextBox", "TextBox", ""},
        {"cntEmpty", "cntEmpty", "empty-guid", "frmCustomer", "Container", "Container", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(form_path.string(), fields, records);
    expect(create_result.ok, "#1029: synthetic SCX table for object ungroup should be created");
}

#if !defined(COPPERFIN_OBJECT_LIFECYCLE_SKIP_TOOLBOX_CREATE)
void test_studio_host_json_creates_toolbox_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "first-created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Created",
            "--toolbox-item", "textbox",
            "--unique-id", "second-created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Created",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdCreateBatch",
            "--unique-id", "created-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Create Batch",
            "--json"
        },
        temp_root);
    expect(create_process.exit_code == 0,
        "#1248: toolbox-create-batch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"toolboxCreateBatch\": {",
        "#1248: toolbox-create-batch JSON should expose a batch result object");
    expect_contains(create_process.stdout_text, "\"ok\": true",
        "#1248: toolbox-create-batch JSON should expose successful mutation state");
    expect_contains(create_process.stdout_text, "\"recordIndexes\": [2, 3, 4]",
        "#1248: toolbox-create-batch JSON should expose created record indexes in append order");
    expect_contains(create_process.stdout_text, "\"createdObjects\": [",
        "#1248: toolbox-create-batch JSON should expose created object metadata");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1248: toolbox-create-batch JSON should expose first generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1248: toolbox-create-batch JSON should reserve generated names across the batch");
    expect_contains(create_process.stdout_text, "\"objectName\": \"cmdCreateBatch\"",
        "#1248: toolbox-create-batch JSON should preserve explicit names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"first-created-textbox-guid\"",
        "#1248: toolbox-create-batch JSON should expose first unique ids");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"created-command-guid\"",
        "#1248: toolbox-create-batch JSON should expose later unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1248: toolbox-create-batch JSON should expose created parent names");
    expect_contains(create_process.stdout_text,
        "\"createdObjectNames\": [\"txt2\", \"txt3\", \"cmdCreateBatch\"]",
        "#1383: toolbox-create-batch JSON should summarize created object names");
    expect_contains(create_process.stdout_text,
        "\"createdUniqueIds\": [\"first-created-textbox-guid\", \"second-created-textbox-guid\", \"created-command-guid\"]",
        "#1383: toolbox-create-batch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1383: successful toolbox-create-batch JSON should summarize empty create errors");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#1248: toolbox-create-batch host command should mutate the visual asset exactly once per item");

    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "first-created-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "second-created-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-command-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Created" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Created" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Create Batch",
        "#1248: toolbox-create-batch host command should persist per-item direct fields");

    const std::size_t report_batch_before_count = visual_object_count(form_path);
    const auto report_batch_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--toolbox-context", "report",
            "--toolbox-item", "label",
            "--unique-id", "direct-report-batch-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Direct Report Batch",
            "--json"
        },
        temp_root);
    expect(report_batch_process.exit_code == 0,
        "#2099: report toolbox-create-batch JSON command should exit successfully");
    expect_contains(report_batch_process.stdout_text, "\"toolboxCreateBatch\": {",
        "#2099: report toolbox-create-batch JSON should expose a batch result object");
    expect_contains(report_batch_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2099: report toolbox-create-batch JSON should expose generated label names");
    expect_contains(report_batch_process.stdout_text, "\"uniqueId\": \"direct-report-batch-label-guid\"",
        "#2099: report toolbox-create-batch JSON should expose label unique ids");
    expect_contains(report_batch_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2099: report toolbox-create-batch JSON should preserve label parent overrides");
    expect_contains(report_batch_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#2099: report toolbox-create-batch JSON should summarize created report object names");
    expect_contains(report_batch_process.stdout_text, "\"createdUniqueIds\": [\"direct-report-batch-label-guid\"]",
        "#2099: report toolbox-create-batch JSON should summarize created report unique ids");
    expect_contains(report_batch_process.stdout_text, "\"createErrors\": []",
        "#2099: report toolbox-create-batch JSON should summarize empty create errors");
    expect_not_contains(report_batch_process.stdout_text, "\"className\": \"TextBox\"",
        "#2099: report toolbox-create-batch JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == report_batch_before_count + 1U,
        "#2099: report toolbox-create-batch host command should mutate the asset exactly once");

    const auto report_batch_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "direct-report-batch-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_batch_caption.ok && report_batch_caption.exists &&
            report_batch_caption.value == "Direct Report Batch",
        "#2099: report toolbox-create-batch host command should persist caller fields");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--object-name", "dupHostName",
            "--unique-id", "dup-host-guid-1",
            "--toolbox-item", "commandbutton",
            "--object-name", "dupHostName",
            "--unique-id", "dup-host-guid-2",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1248: toolbox-create-batch JSON should reject duplicate planned identities");
    expect_contains(duplicate_process.stdout_text, "The requested toolbox object identity already exists in the asset.",
        "#1248: duplicate toolbox-create-batch identities should report runtime errors");
    expect_contains(duplicate_process.stdout_text, "\"recordIndexes\": []",
        "#1248: failed toolbox-create-batch JSON should not expose stale record indexes");
    expect_contains(duplicate_process.stdout_text, "\"createdObjects\": [\n    ]",
        "#1248: failed toolbox-create-batch JSON should not expose stale created objects");
    expect_contains(duplicate_process.stdout_text, "\"createdObjectNames\": []",
        "#1383: failed toolbox-create-batch JSON should summarize no created object names");
    expect_contains(duplicate_process.stdout_text, "\"createdUniqueIds\": []",
        "#1383: failed toolbox-create-batch JSON should summarize no created unique ids");
    expect_contains(duplicate_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox object identity already exists in the asset.\"",
        "#1383: failed toolbox-create-batch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1248: duplicate-rejected toolbox-create-batch commands should not partially mutate");

    const auto invalid_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--toolbox-context", "form",
            "--toolbox-item", "textbox",
            "--unique-id", "valid-before-invalid-host-guid",
            "--toolbox-item", "commandbutton",
            "--unique-id", "invalid-field-host-guid",
            "--field-value", "UNKNOWN=value",
            "--json"
        },
        temp_root);
    expect(invalid_field_process.exit_code == 4,
        "#1248: toolbox-create-batch JSON should reject invalid fields without partial mutation");
    expect_contains(invalid_field_process.stdout_text, "The requested field was not found in the asset.",
        "#1248: invalid toolbox-create-batch fields should report lower-layer errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1248: lower-layer toolbox-create-batch failures should not partially mutate");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1248: toolbox-create-batch JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Toolbox batch item options require a preceding --toolbox-item.",
        "#1248: orphan toolbox-create-batch item options should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1248: parser-rejected toolbox-create-batch commands should not mutate");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-batch",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1248: toolbox-create-batch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1248: malformed toolbox-create-batch field values should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1248: malformed toolbox-create-batch commands should not mutate");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_selection_toolbox_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-first-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=First Selection Batch",
            "--toolbox-item", "commandbutton",
            "--object-name", "cmdSelectionBatchHost",
            "--unique-id", "selection-batch-host-command-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Run Selection Batch",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-second-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Second Selection Batch",
            "--json"
        },
        temp_root);
    expect(create_process.exit_code == 0,
        "#1311: selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1311: successful selection-toolbox-create-batch JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#1311: selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1311: selection-toolbox-create-batch JSON should expose selected contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1311: selection-toolbox-create-batch JSON should expose resolved toolbox contexts");
    expect_contains(create_process.stdout_text, "\"launchPlanOk\": true",
        "#1311: selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(create_process.stdout_text, "\"batchPlanOk\": true",
        "#1311: selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1311: selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"recordIndexes\": [2, 3, 4]",
        "#1311: selection-toolbox-create-batch JSON should expose created record indexes");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1311: selection-toolbox-create-batch JSON should expose first generated names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"cmdSelectionBatchHost\"",
        "#1311: selection-toolbox-create-batch JSON should expose explicit names");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt3\"",
        "#1311: selection-toolbox-create-batch JSON should reserve generated names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-batch-host-first-guid\"",
        "#1311: selection-toolbox-create-batch JSON should expose first unique ids");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-batch-host-command-guid\"",
        "#1311: selection-toolbox-create-batch JSON should expose later unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1311: selection-toolbox-create-batch JSON should expose created parents");
    expect_contains(create_process.stdout_text,
        "\"createdObjectNames\": [\"txt2\", \"cmdSelectionBatchHost\", \"txt3\"]",
        "#1383: selection-toolbox-create-batch JSON should summarize created object names");
    expect_contains(create_process.stdout_text,
        "\"createdUniqueIds\": [\"selection-batch-host-first-guid\", \"selection-batch-host-command-guid\", \"selection-batch-host-second-guid\"]",
        "#1383: selection-toolbox-create-batch JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1383: successful selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"className\": \"TextBox\"",
        "#1311: selection-toolbox-create-batch JSON should expose descriptor class names");
    expect_contains(create_process.stdout_text, "\"propertyValue\": \"First Selection Batch\"",
        "#1311: selection-toolbox-create-batch JSON should expose per-item field values");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1311: selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1311: selection-toolbox-create-batch JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#1311: selection-toolbox-create-batch host command should mutate once per accepted item");

    const auto first_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-first-guid",
        .property_name = "CAPTION"
    });
    const auto command_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-command-guid",
        .property_name = "CAPTION"
    });
    const auto second_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-second-guid",
        .property_name = "CAPTION"
    });
    expect(first_caption.ok && first_caption.exists && first_caption.value == "First Selection Batch" &&
            command_caption.ok && command_caption.exists && command_caption.value == "Run Selection Batch" &&
            second_caption.ok && second_caption.exists && second_caption.value == "Second Selection Batch",
        "#1311: selection-toolbox-create-batch host command should persist caller fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "report_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-host-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection Batch",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1311: report selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#2116: report selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1311: report selection-toolbox-create-batch JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1311: report selection-toolbox-create-batch JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"batchPlanOk\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2116: report selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1311: report selection-toolbox-create-batch JSON should expose generated labels");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-batch-host-report-label-guid\"",
        "#1311: report selection-toolbox-create-batch JSON should expose label unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2116: report selection-toolbox-create-batch JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#1383: report selection-toolbox-create-batch JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"selection-batch-host-report-label-guid\"]",
        "#1383: report selection-toolbox-create-batch JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2116: report selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection Batch\"",
        "#2116: report selection-toolbox-create-batch JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2116: report selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2116: report selection-toolbox-create-batch JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1311: report selection-toolbox-create-batch JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count + 4U,
        "#2116: report selection-toolbox-create-batch host command should mutate the asset exactly once");

    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-host-report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Selection Batch",
        "#2116: report selection-toolbox-create-batch host command should persist report label captions");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "label_expression",
            "--toolbox-item", "label",
            "--unique-id", "selection-batch-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection Batch",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2085: label selection-toolbox-create-batch JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreateBatch\": {",
        "#2130: label selection-toolbox-create-batch JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2085: label selection-toolbox-create-batch JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2085: label selection-toolbox-create-batch JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"batchPlanOk\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose nested batch-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2130: label selection-toolbox-create-batch JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2085: label selection-toolbox-create-batch JSON should expose generated labels");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-batch-label-guid\"",
        "#2085: label selection-toolbox-create-batch JSON should expose label unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2130: label selection-toolbox-create-batch JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2085: label selection-toolbox-create-batch JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"selection-batch-label-guid\"]",
        "#2085: label selection-toolbox-create-batch JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2130: label selection-toolbox-create-batch JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection Batch\"",
        "#2130: label selection-toolbox-create-batch JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2130: label selection-toolbox-create-batch JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2130: label selection-toolbox-create-batch JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2085: label selection-toolbox-create-batch JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count + 5U,
        "#2130: label selection-toolbox-create-batch host command should mutate the asset exactly once");

    const auto label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-batch-label-guid",
        .property_name = "CAPTION"
    });
    expect(label_caption.ok && label_caption.exists && label_caption.value == "Label Selection Batch",
        "#2130: label selection-toolbox-create-batch host command should persist label captions");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "report_expression",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1311: selection-toolbox-create-batch JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"status\": \"error\"",
        "#1311: unavailable selection-toolbox-create-batch JSON should report error status");
    expect_contains(unavailable_process.stdout_text, "\"batchPlanOk\": false",
        "#1311: unavailable selection-toolbox-create-batch JSON should expose failed batch plans");
    expect_contains(unavailable_process.stdout_text, "\"recordIndexes\": []",
        "#1311: unavailable selection-toolbox-create-batch JSON should avoid stale record indexes");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1311: unavailable selection-toolbox-create-batch JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1383: unavailable selection-toolbox-create-batch JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: unavailable selection-toolbox-create-batch commands should not mutate assets");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "menu_item",
            "--toolbox-item", "textbox",
            "--unique-id", "selection-batch-host-menu-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1311: selection-toolbox-create-batch JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1311: unsupported selection-toolbox-create-batch JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1311: unsupported selection-toolbox-create-batch JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object batch creation plan request requires a toolbox palette.",
        "#1311: unsupported selection-toolbox-create-batch JSON should report palette errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: unsupported selection-toolbox-create-batch commands should not mutate assets");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1311: missing path selection-toolbox-create-batch JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--toolbox-item", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1311: missing selection selection-toolbox-create-batch JSON should report parser errors");

    const auto empty_items_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(empty_items_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject empty item lists");
    expect_contains(empty_items_process.stdout_text, "No toolbox item ids were provided.",
        "#1311: empty selection-toolbox-create-batch item lists should report parser errors");

    const auto orphan_item_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--parent-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(orphan_item_option_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject item options before items");
    expect_contains(orphan_item_option_process.stdout_text,
        "Selection toolbox batch create item options require a preceding --toolbox-item.",
        "#1311: orphan selection-toolbox-create-batch item options should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-item", "textbox",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1311: malformed selection-toolbox-create-batch JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create-batch",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1311: selection-toolbox-create-batch JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create-batch option: --toolbox-context",
        "#1311: unknown option selection-toolbox-create-batch JSON should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1311: parser-rejected selection-toolbox-create-batch commands should not mutate assets");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_selection_toolbox_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selection_toolbox_create_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--unique-id", "selection-created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Selection Created",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    if (create_process.exit_code != 0) {
        std::cerr << "studio host selection-toolbox-create stdout:\n" << create_process.stdout_text << "\n";
        std::cerr << "studio host selection-toolbox-create stderr:\n" << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(create_process.exit_code == 0,
        "#1309: selection-toolbox-create JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
        "#1309: successful selection-toolbox-create JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#1309: selection-toolbox-create JSON should expose a stable result object");
    expect_contains(create_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1309: selection-toolbox-create JSON should expose selected contexts");
    expect_contains(create_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1309: selection-toolbox-create JSON should expose resolved toolbox contexts");
    expect_contains(create_process.stdout_text, "\"launchPlanOk\": true",
        "#1309: selection-toolbox-create JSON should expose launch state");
    expect_contains(create_process.stdout_text, "\"createPlanOk\": true",
        "#1309: selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(create_process.stdout_text, "\"createResult\": {",
        "#1309: selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
        "#1309: selection-toolbox-create JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1309: selection-toolbox-create JSON should expose generated object names");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"selection-created-textbox-guid\"",
        "#1309: selection-toolbox-create JSON should expose created unique ids");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1309: selection-toolbox-create JSON should expose created parents");
    expect_contains(create_process.stdout_text, "\"createdObjectNames\": [\"txt2\"]",
        "#1382: selection-toolbox-create JSON should summarize created object names");
    expect_contains(create_process.stdout_text, "\"createdUniqueIds\": [\"selection-created-textbox-guid\"]",
        "#1382: selection-toolbox-create JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
        "#1382: successful selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(create_process.stdout_text, "\"className\": \"TextBox\"",
        "#1309: selection-toolbox-create JSON should expose descriptor metadata");
    expect_contains(create_process.stdout_text, "\"propertyValue\": \"Selection Created\"",
        "#1309: selection-toolbox-create JSON should expose caller direct fields");
    expect_contains(create_process.stdout_text, "\"dryRun\": false",
        "#1309: selection-toolbox-create JSON should expose execution state");
    expect_contains(create_process.stdout_text, "\"mutatesAsset\": true",
        "#1309: selection-toolbox-create JSON should expose mutation state");
    expect(visual_object_count(form_path) == before_count + 1U,
        "#1309: selection-toolbox-create host command should mutate the asset exactly once");

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .property_name = "CAPTION"
    });
    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(caption.ok && caption.exists && caption.value == "Selection Created" &&
            control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1309: selection-toolbox-create host command should persist caller fields");

    const auto report_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "label",
            "--selection-context", "report_expression",
            "--unique-id", "selection-report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Report Selection",
            "--json"
        },
        temp_root);
    expect(report_process.exit_code == 0,
        "#1309: report selection-toolbox-create JSON command should exit successfully");
    expect_contains(report_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#2115: report selection-toolbox-create JSON should expose a stable result object");
    expect_contains(report_process.stdout_text, "\"selectionContext\": \"report_expression\"",
        "#1309: report selection-toolbox-create JSON should expose report selections");
    expect_contains(report_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#1309: report selection-toolbox-create JSON should resolve report contexts");
    expect_contains(report_process.stdout_text, "\"launchPlanOk\": true",
        "#2115: report selection-toolbox-create JSON should expose launch state");
    expect_contains(report_process.stdout_text, "\"createPlanOk\": true",
        "#2115: report selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(report_process.stdout_text, "\"createResult\": {",
        "#2115: report selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(report_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#1309: report selection-toolbox-create JSON should expose generated label names");
    expect_contains(report_process.stdout_text, "\"uniqueId\": \"selection-report-label-guid\"",
        "#1309: report selection-toolbox-create JSON should expose report label unique ids");
    expect_contains(report_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2115: report selection-toolbox-create JSON should preserve report parent payloads");
    expect_contains(report_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
        "#1382: report selection-toolbox-create JSON should summarize created report object names");
    expect_contains(report_process.stdout_text, "\"createdUniqueIds\": [\"selection-report-label-guid\"]",
        "#1382: report selection-toolbox-create JSON should summarize created report unique ids");
    expect_contains(report_process.stdout_text, "\"createErrors\": []",
        "#2115: report selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(report_process.stdout_text, "\"propertyValue\": \"Report Selection\"",
        "#2115: report selection-toolbox-create JSON should expose caller report fields");
    expect_contains(report_process.stdout_text, "\"dryRun\": false",
        "#2115: report selection-toolbox-create JSON should expose execution state");
    expect_contains(report_process.stdout_text, "\"mutatesAsset\": true",
        "#2115: report selection-toolbox-create JSON should expose mutation state");
    expect_not_contains(report_process.stdout_text, "\"className\": \"TextBox\"",
        "#1309: report selection-toolbox-create JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 2U,
        "#1309: report selection-toolbox-create host command should mutate the asset exactly once");

    const auto report_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_caption.ok && report_caption.exists && report_caption.value == "Report Selection",
        "#2115: report selection-toolbox-create host command should persist report label captions");

    const auto label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "label",
            "--selection-context", "label_expression",
            "--unique-id", "selection-label-expression-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Label Selection",
            "--json"
        },
        temp_root);
    expect(label_process.exit_code == 0,
        "#2084: label selection-toolbox-create JSON command should exit successfully");
    expect_contains(label_process.stdout_text, "\"selectionToolboxCreate\": {",
        "#2129: label selection-toolbox-create JSON should expose a stable result object");
    expect_contains(label_process.stdout_text, "\"selectionContext\": \"label_expression\"",
        "#2084: label selection-toolbox-create JSON should expose label selections");
    expect_contains(label_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2084: label selection-toolbox-create JSON should resolve report contexts");
    expect_contains(label_process.stdout_text, "\"launchPlanOk\": true",
        "#2129: label selection-toolbox-create JSON should expose launch state");
    expect_contains(label_process.stdout_text, "\"createPlanOk\": true",
        "#2129: label selection-toolbox-create JSON should expose nested create-plan state");
    expect_contains(label_process.stdout_text, "\"createResult\": {",
        "#2129: label selection-toolbox-create JSON should expose lower-level create results");
    expect_contains(label_process.stdout_text, "\"objectName\": \"lbl2\"",
        "#2084: label selection-toolbox-create JSON should expose generated label names");
    expect_contains(label_process.stdout_text, "\"uniqueId\": \"selection-label-expression-label-guid\"",
        "#2084: label selection-toolbox-create JSON should expose label unique ids");
    expect_contains(label_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2129: label selection-toolbox-create JSON should preserve label parent payloads");
    expect_contains(label_process.stdout_text, "\"createdObjectNames\": [\"lbl2\"]",
        "#2084: label selection-toolbox-create JSON should summarize created label object names");
    expect_contains(label_process.stdout_text, "\"createdUniqueIds\": [\"selection-label-expression-label-guid\"]",
        "#2084: label selection-toolbox-create JSON should summarize created label unique ids");
    expect_contains(label_process.stdout_text, "\"createErrors\": []",
        "#2129: label selection-toolbox-create JSON should summarize empty create errors");
    expect_contains(label_process.stdout_text, "\"propertyValue\": \"Label Selection\"",
        "#2129: label selection-toolbox-create JSON should expose caller label fields");
    expect_contains(label_process.stdout_text, "\"dryRun\": false",
        "#2129: label selection-toolbox-create JSON should expose execution state");
    expect_contains(label_process.stdout_text, "\"mutatesAsset\": true",
        "#2129: label selection-toolbox-create JSON should expose mutation state");
    expect_not_contains(label_process.stdout_text, "\"className\": \"TextBox\"",
        "#2084: label selection-toolbox-create JSON should exclude form-only textbox metadata");
    expect(visual_object_count(form_path) == before_count + 3U,
        "#2129: label selection-toolbox-create host command should mutate the asset exactly once");

    const auto label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "selection-label-expression-label-guid",
        .property_name = "CAPTION"
    });
    expect(label_caption.ok && label_caption.exists && label_caption.value == "Label Selection",
        "#2129: label selection-toolbox-create host command should persist label captions");

    const std::size_t committed_count = visual_object_count(form_path);
    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "report_expression",
            "--unique-id", "selection-report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1309: selection-toolbox-create JSON should reject unavailable selected-context items");
    expect_contains(unavailable_process.stdout_text, "\"status\": \"error\"",
        "#1309: unavailable selection-toolbox-create JSON should report error status");
    expect_contains(unavailable_process.stdout_text, "\"createPlanOk\": false",
        "#1309: unavailable selection-toolbox-create JSON should expose failed create-plan state");
    expect_contains(unavailable_process.stdout_text, "\"createResult\": {",
        "#1309: unavailable selection-toolbox-create JSON should expose clean create-result state");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the requested designer context.",
        "#1309: unavailable selection-toolbox-create JSON should report planner errors");
    expect_contains(unavailable_process.stdout_text, "\"objectName\": \"\"",
        "#1309: unavailable selection-toolbox-create JSON should avoid stale object names");
    expect_contains(unavailable_process.stdout_text, "\"createdObjectNames\": []",
        "#1382: unavailable selection-toolbox-create JSON should summarize no created object names");
    expect_contains(unavailable_process.stdout_text, "\"createdUniqueIds\": []",
        "#1382: unavailable selection-toolbox-create JSON should summarize no created unique ids");
    expect_contains(unavailable_process.stdout_text,
        "\"createErrors\": [\"The requested toolbox item is not available in the requested designer context.\"",
        "#1382: unavailable selection-toolbox-create JSON should summarize create errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: unavailable selection-toolbox-create commands should not mutate the asset");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "menu_item",
            "--unique-id", "selection-menu-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1309: selection-toolbox-create JSON should reject unsupported selections");
    expect_contains(unsupported_process.stdout_text, "\"selectionContext\": \"menu_item\"",
        "#1309: unsupported selection-toolbox-create JSON should preserve selected contexts");
    expect_contains(unsupported_process.stdout_text, "\"launchPlanOk\": false",
        "#1309: unsupported selection-toolbox-create JSON should expose launch failures");
    expect_contains(unsupported_process.stdout_text,
        "A selection-context toolbox object creation plan request requires a toolbox palette.",
        "#1309: unsupported selection-toolbox-create JSON should report palette errors");
    expect_contains(unsupported_process.stdout_text, "\"objectName\": \"\"",
        "#1309: unsupported selection-toolbox-create JSON should avoid stale object names");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: unsupported selection-toolbox-create commands should not mutate the asset");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing paths");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1309: missing path selection-toolbox-create JSON should report parser errors");

    const auto missing_item_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create",
            "--selection-context", "visual_object",
            "--json"
        },
        temp_root);
    expect(missing_item_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing item values");
    expect_contains(missing_item_process.stdout_text, "Missing value for --selection-toolbox-create.",
        "#1309: missing item selection-toolbox-create JSON should report parser errors");

    const auto missing_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--json"
        },
        temp_root);
    expect(missing_selection_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject missing selections");
    expect_contains(missing_selection_process.stdout_text, "No selection context was provided.",
        "#1309: missing selection selection-toolbox-create JSON should report parser errors");

    const auto unknown_selection_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "unknown_context",
            "--json"
        },
        temp_root);
    expect(unknown_selection_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject unknown selections");
    expect_contains(unknown_selection_process.stdout_text, "Unknown selection context token: unknown_context",
        "#1309: unknown selection selection-toolbox-create JSON should report parser errors");

    const auto malformed_field_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--field-value", "BROKEN",
            "--json"
        },
        temp_root);
    expect(malformed_field_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject malformed field values");
    expect_contains(malformed_field_process.stdout_text, "Toolbox field values must use name=value syntax.",
        "#1309: malformed selection-toolbox-create JSON should report parser errors");

    const auto unknown_option_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--selection-toolbox-create", "textbox",
            "--selection-context", "visual_object",
            "--toolbox-context", "form",
            "--json"
        },
        temp_root);
    expect(unknown_option_process.exit_code == 2,
        "#1309: selection-toolbox-create JSON should reject unknown options");
    expect_contains(unknown_option_process.stdout_text,
        "Unknown selection-toolbox-create option: --toolbox-context",
        "#1309: unknown option selection-toolbox-create JSON should report parser errors");
    expect(visual_object_count(form_path) == committed_count,
        "#1309: parser-rejected selection-toolbox-create commands should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_creates_toolbox_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto create_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--unique-id", "created-textbox-guid",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Customer",
            "--field-value", "PROPERTIES=ControlSource = \"customer.name\"",
            "--json"
        },
        temp_root);

    if (create_process.exit_code != 0) {
        std::cerr << "studio host toolbox-create stdout:\n" << create_process.stdout_text << "\n";
        std::cerr << "studio host toolbox-create stderr:\n" << create_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(create_process.exit_code == 0, "#1018: toolbox-create JSON command should exit successfully");
    expect_contains(create_process.stdout_text, "\"status\": \"ok\"",
                    "#1018: successful toolbox-create JSON should report ok status");
    expect_contains(create_process.stdout_text, "\"toolboxCreate\": {",
                    "#1018: toolbox-create JSON should use a stable result object");
    expect_contains(create_process.stdout_text, "\"ok\": true",
                    "#1018: toolbox-create JSON should expose result success");
    expect_contains(create_process.stdout_text, "\"recordIndex\": 2",
                    "#1018: toolbox-create JSON should expose appended record index");
    expect_contains(create_process.stdout_text, "\"objectName\": \"txt2\"",
                    "#1018: toolbox-create JSON should expose generated object name");
    expect_contains(create_process.stdout_text, "\"uniqueId\": \"created-textbox-guid\"",
                    "#1018: toolbox-create JSON should expose created unique id");
    expect_contains(create_process.stdout_text, "\"parentName\": \"frmCustomer\"",
                    "#1018: toolbox-create JSON should expose created parent name");
    expect_contains(create_process.stdout_text, "\"createdObjectNames\": [\"txt2\"]",
                    "#1382: toolbox-create JSON should summarize created object names");
    expect_contains(create_process.stdout_text, "\"createdUniqueIds\": [\"created-textbox-guid\"]",
                    "#1382: toolbox-create JSON should summarize created unique ids");
    expect_contains(create_process.stdout_text, "\"createErrors\": []",
                    "#1382: successful toolbox-create JSON should summarize empty create errors");

    const auto caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "CAPTION"
    });
    expect(caption.ok && caption.exists && caption.value == "Customer",
        "#1018: toolbox-create host command should propagate extra direct fields");

    const auto control_source = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "created-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(control_source.ok && control_source.exists && control_source.value == "\"customer.name\"",
        "#1018: toolbox-create host command should propagate extra memo fields");

    const std::size_t object_count_before_failure = visual_object_count(form_path);
    const auto failure_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "missing-toolbox-item",
            "--unique-id", "should-not-exist",
            "--parent-name", "frmCustomer",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(failure_process.exit_code == 4, "#1018: unknown toolbox ids should return a command failure exit code");
    expect_contains(failure_process.stdout_text, "\"status\": \"error\"",
                    "#1018: failed toolbox-create JSON should report error status");
    expect_contains(failure_process.stdout_text, "\"ok\": false",
                    "#1018: failed toolbox-create JSON should expose result failure");
    expect_contains(failure_process.stdout_text, "\"error\": \"The requested toolbox item was not found.\"",
                    "#1018: failed toolbox-create JSON should expose clean error text");
    expect_contains(failure_process.stdout_text, "\"objectName\": \"\"",
                    "#1018: failed toolbox-create JSON should not report stale object names");
    expect_contains(failure_process.stdout_text, "\"createdObjectNames\": []",
                    "#1382: failed toolbox-create JSON should summarize no created object names");
    expect_contains(failure_process.stdout_text, "\"createdUniqueIds\": []",
                    "#1382: failed toolbox-create JSON should summarize no created unique ids");
    expect_contains(failure_process.stdout_text,
                    "\"createErrors\": [\"The requested toolbox item was not found.\"",
                    "#1382: failed toolbox-create JSON should summarize create errors");
    expect(visual_object_count(form_path) == object_count_before_failure,
        "#1018: failed toolbox-create host commands should not mutate the asset");

    const auto report_label_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "label",
            "--toolbox-context", "report",
            "--unique-id", "report-label-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Total",
            "--json"
        },
        temp_root);

    expect(report_label_process.exit_code == 0,
        "#1019: report-compatible toolbox items should create through host JSON when report context is requested");
    expect_contains(report_label_process.stdout_text, "\"objectName\": \"lbl1\"",
                    "#1019: report-compatible toolbox creates should expose generated label names");
    expect_contains(report_label_process.stdout_text, "\"uniqueId\": \"report-label-guid\"",
                    "#1019: report-compatible toolbox creates should expose created unique ids");
    expect_contains(report_label_process.stdout_text, "\"parentName\": \"DetailBand\"",
                    "#2104: report-compatible toolbox creates should preserve report label parents");
    expect_contains(report_label_process.stdout_text, "\"createdObjectNames\": [\"lbl1\"]",
                    "#1382: report toolbox-create JSON should summarize created report object names");
    expect_contains(report_label_process.stdout_text, "\"createdUniqueIds\": [\"report-label-guid\"]",
                    "#1382: report toolbox-create JSON should summarize created report unique ids");
    expect_not_contains(report_label_process.stdout_text, "\"className\": \"TextBox\"",
                    "#2104: report toolbox-create JSON should exclude form-only TextBox metadata");
    expect(visual_object_count(form_path) == object_count_before_failure + 1U,
        "#2104: report toolbox-create host command should mutate the asset exactly once");

    const auto report_label_caption = copperfin::vfp::query_visual_object_property({
        .path = form_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "report-label-guid",
        .property_name = "CAPTION"
    });
    expect(report_label_caption.ok && report_label_caption.exists && report_label_caption.value == "Total",
        "#2104: report toolbox-create host command should persist caller report label fields");

    const std::size_t object_count_before_context_failure = visual_object_count(form_path);
    const auto report_textbox_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create", "textbox",
            "--toolbox-context", "report",
            "--unique-id", "report-textbox-guid",
            "--parent-name", "DetailBand",
            "--field-value", "CAPTION=Should Not Exist",
            "--json"
        },
        temp_root);

    expect(report_textbox_process.exit_code == 4,
        "#1019: report-incompatible toolbox items should fail through host JSON when report context is requested");
    expect_contains(report_textbox_process.stdout_text, "\"status\": \"error\"",
                    "#1019: context-filtered toolbox failures should report JSON error status");
    expect_contains(
        report_textbox_process.stdout_text,
        "\"error\": \"The requested toolbox item is not available in the requested designer context.\"",
        "#1019: context-filtered toolbox failures should expose clean error text");
    expect(visual_object_count(form_path) == object_count_before_context_failure,
        "#1019: context-filtered toolbox failures should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION)
void test_studio_host_json_sets_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    auto caption_value = [&]() {
        return copperfin::vfp::query_visual_object_property({
            .path = form_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "existing-textbox-guid",
            .property_name = "CAPTION"
        });
    };

    const auto record_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--record", "1",
            "--property-name", "CAPTION",
            "--property-value", "RecordTarget",
            "--json"
        },
        temp_root);
    expect(record_process.exit_code == 0,
        "#1020: record-index host property edits should remain compatible");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "RecordTarget",
        "#1020: record-index host property edits should update the selected record");

    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--property-value", "NameTarget",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1020: object-name host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "NameTarget",
        "#1020: object-name host property edits should update the named object");

    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "UniqueTarget",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1020: unique-id host property edits should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: unique-id host property edits should update the stable selected object");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "ShouldNotWrite",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1020: missing object-name host property edits should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "UniqueTarget",
        "#1020: missing object-name host property edits should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = write_synthetic_form_table_for_property_rename(temp_root, "object_name.scx");
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--rename-property",
            "--object-name", "txt1",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1022: object-name host property renames should exit successfully");
    auto renamed_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: object-name host property renames should create the target memo-backed property");
    auto source_property = copperfin::vfp::query_visual_object_property({
        .path = object_name_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && !source_property.exists,
        "#1022: object-name host property renames should remove the source memo-backed property");

    const fs::path unique_id_path = write_synthetic_form_table_for_property_rename(temp_root, "unique_id.scx");
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--rename-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1022: unique-id host property renames should exit successfully");
    renamed_property = copperfin::vfp::query_visual_object_property({
        .path = unique_id_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "InputSource"
    });
    expect(renamed_property.ok && renamed_property.exists && renamed_property.value == "\"customer.name\"",
        "#1022: unique-id host property renames should create the target memo-backed property");

    const fs::path missing_path = write_synthetic_form_table_for_property_rename(temp_root, "missing.scx");
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--rename-property",
            "--object-name", "missingObject",
            "--property-name", "ControlSource",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1022: missing object-name host property renames should return command failure");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = missing_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: missing object-name host property renames should not mutate the asset");

    const fs::path ambiguous_path = write_synthetic_form_table_for_property_rename(temp_root, "ambiguous.scx");
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--set-property",
            "--rename-property",
            "--property-name", "ControlSource",
            "--property-value", "Ambiguous",
            "--new-property-name", "InputSource",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1022: ambiguous set/rename property requests should fail during launch parsing");
    source_property = copperfin::vfp::query_visual_object_property({
        .path = ambiguous_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "existing-textbox-guid",
        .property_name = "ControlSource"
    });
    expect(source_property.ok && source_property.exists && source_property.value == "\"customer.name\"",
        "#1022: ambiguous set/rename property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_deleted_states_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_deleted_states_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path batch_path = temp_root / "batch.scx";
    write_synthetic_form_table_for_deleted_states(batch_path);
    mark_deleted_for_deleted_states_fixture(batch_path, "name-guid", true,
        "#1201: deleted-states fixture should support initially deleted targets");
    const auto batch_process = run_process_capture(
        studio_host_path,
        {
            "--from-vs",
            "--path", batch_path.string(),
            "--deleted-states",
            "--deleted-state-target-object-name", "cmdSave",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "name-guid",
            "--deleted-state", "false",
            "--json"
        },
        temp_root);
    expect(batch_process.exit_code == 0,
        "#1201: host deleted-states batch should exit successfully");
    expect_contains(batch_process.stdout_text, "\"dryRun\": false",
        "#4394: deleted-states success should expose mutation dry-run metadata");
    expect_contains(batch_process.stdout_text, "\"mutatesAsset\": true",
        "#4394: deleted-states success should expose mutation metadata");
    expect_contains(batch_process.stdout_text, "\"launchedFromVisualStudio\": true",
        "#3985: VS-originated deleted-state batches should preserve host provenance in JSON");
    expect(visual_object_deleted(batch_path, "save-guid") &&
            !visual_object_deleted(batch_path, "name-guid") &&
            !visual_object_deleted(batch_path, "status-guid") &&
            !visual_object_deleted(batch_path, "dup-one-guid") &&
            !visual_object_deleted(batch_path, "dup-two-guid"),
        "#1201: host deleted-states batch should delete, restore, and preserve unrelated records");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_deleted_states(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "save-guid",
            "--deleted-state", "true",
            "--deleted-state-target-unique-id", "missing-guid",
            "--deleted-state", "true",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1201: missing-target host deleted-states batch should return command failure");
    expect(!visual_object_deleted(missing_path, "save-guid") &&
            !visual_object_deleted(missing_path, "name-guid") &&
            !visual_object_deleted(missing_path, "status-guid"),
        "#1201: failed host deleted-states batch should roll back earlier target mutations");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_deleted_states(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--deleted-states",
            "--deleted-state-target-object-name", "dupControl",
            "--deleted-state", "true",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1201: ambiguous-target host deleted-states batch should return command failure");
    expect(!visual_object_deleted(duplicate_path, "dup-one-guid") &&
            !visual_object_deleted(duplicate_path, "dup-two-guid"),
        "#1201: ambiguous host deleted-states batch should not mutate duplicate candidates");

    const fs::path missing_state_path = temp_root / "missing_state.scx";
    write_synthetic_form_table_for_deleted_states(missing_state_path);
    const auto missing_state_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_state_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_state_process.exit_code == 2,
        "#1201: deleted-states item without state should fail during launch parsing");
    expect(!visual_object_deleted(missing_state_path, "save-guid"),
        "#1201: deleted-states missing-state parse failure should not mutate the asset");

    const fs::path empty_path = temp_root / "empty.scx";
    write_synthetic_form_table_for_deleted_states(empty_path);
    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_path.string(),
            "--deleted-states",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 2,
        "#1201: deleted-states without targets should fail during launch parsing");
    expect(!visual_object_deleted(empty_path, "save-guid"),
        "#1201: deleted-states empty parse failure should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_deleted_states(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--deleted-states",
            "--deleted-state-target-unique-id", "save-guid",
            "--deleted-state", "true",
            "--delete-object",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1201: deleted-states plus delete-object should fail during launch parsing");
    expect(!visual_object_deleted(ambiguous_path, "save-guid"),
        "#1201: deleted-states ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_applies_subtree_deleted_state_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_subtree_deleted_state_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_subtree_deleted_state(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--unique-id", "container-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1202: host subtree deleted-state by unique id should exit successfully");
    expect(visual_object_deleted(unique_id_path, "container-guid") &&
            visual_object_deleted(unique_id_path, "save-guid") &&
            visual_object_deleted(unique_id_path, "name-guid") &&
            visual_object_deleted(unique_id_path, "nested-guid") &&
            !visual_object_deleted(unique_id_path, "other-guid"),
        "#1202: host subtree deleted-state should delete root and descendants while preserving unrelated records");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "false",
            "--object-name", "cntMain",
            "--json"
        },
        temp_root);
    expect(restore_process.exit_code == 0,
        "#1202: host subtree restore by object name should exit successfully");
    expect(!visual_object_deleted(unique_id_path, "container-guid") &&
            !visual_object_deleted(unique_id_path, "save-guid") &&
            !visual_object_deleted(unique_id_path, "name-guid") &&
            !visual_object_deleted(unique_id_path, "nested-guid") &&
            !visual_object_deleted(unique_id_path, "other-guid"),
        "#1202: host subtree restore should restore root and descendants while preserving unrelated records");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_subtree_deleted_state(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1202: missing-root host subtree deleted-state should return command failure");
    expect(!visual_object_deleted(missing_path, "container-guid") &&
            !visual_object_deleted(missing_path, "save-guid") &&
            !visual_object_deleted(missing_path, "name-guid") &&
            !visual_object_deleted(missing_path, "nested-guid") &&
            !visual_object_deleted(missing_path, "other-guid"),
        "#1202: missing-root host subtree deleted-state should not mutate the asset");

    const fs::path nameless_path = temp_root / "nameless.scx";
    write_synthetic_form_table_for_subtree_deleted_state(nameless_path);
    const auto nameless_process = run_process_capture(
        studio_host_path,
        {
            "--path", nameless_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--unique-id", "nameless-guid",
            "--json"
        },
        temp_root);
    expect(nameless_process.exit_code == 4,
        "#1202: nameless-root host subtree deleted-state should return command failure");
    expect(!visual_object_deleted(nameless_path, "nameless-guid") &&
            !visual_object_deleted(nameless_path, "container-guid"),
        "#1202: nameless-root host subtree deleted-state should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_subtree_deleted_state(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1202: subtree deleted-state without root selectors should fail during launch parsing");
    expect(!visual_object_deleted(missing_selector_path, "container-guid"),
        "#1202: subtree deleted-state missing-selector parse failure should not mutate the asset");

    const fs::path invalid_value_path = temp_root / "invalid_value.scx";
    write_synthetic_form_table_for_subtree_deleted_state(invalid_value_path);
    const auto invalid_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", invalid_value_path.string(),
            "--subtree-deleted-state",
            "--unique-id", "container-guid",
            "--subtree-deleted", "sometimes",
            "--json"
        },
        temp_root);
    expect(invalid_value_process.exit_code == 2,
        "#1202: subtree deleted-state invalid logical values should fail during launch parsing");
    expect(!visual_object_deleted(invalid_value_path, "container-guid"),
        "#1202: subtree deleted-state invalid-value parse failure should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_subtree_deleted_state(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--subtree-deleted-state",
            "--subtree-deleted", "true",
            "--unique-id", "container-guid",
            "--delete-object",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1202: subtree deleted-state plus delete-object should fail during launch parsing");
    expect(!visual_object_deleted(ambiguous_path, "container-guid") &&
            !visual_object_deleted(ambiguous_path, "save-guid"),
        "#1202: subtree deleted-state ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_STABLE_SELECTOR_MUTATION) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_SKIP_OBJECT_ACTION_LIFECYCLE)
void test_studio_host_json_duplicates_visual_object_subtrees(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_subtree_duplicate_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path subtree_path = temp_root / "subtree_duplicate.scx";
    write_synthetic_form_table_for_subtree_deleted_state(subtree_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", subtree_path.string(),
            "--unique-id", "container-guid",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntCopy",
            "--new-name", "cntCopy",
            "--new-unique-id", "container-copy-guid",
            "--replacement-source-unique-id", "save-guid",
            "--new-object-name", "cmdSaveCopy",
            "--new-name", "cmdSaveCopy",
            "--new-unique-id", "save-copy-guid",
            "--replacement-source-unique-id", "name-guid",
            "--new-object-name", "txtNameCopy",
            "--new-name", "txtNameCopy",
            "--new-unique-id", "name-copy-guid",
            "--replacement-source-unique-id", "nested-guid",
            "--new-object-name", "lblNestedCopy",
            "--new-name", "lblNestedCopy",
            "--new-unique-id", "nested-copy-guid",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 0,
        "#1451: visual object duplicate-subtree JSON should exit successfully for valid replacement maps");
    expect_contains(duplicate_process.stdout_text, "\"visualObjectDuplicateSubtree\": {",
        "#1451: visual object duplicate-subtree JSON should expose a subtree duplicate object");
    expect_contains(duplicate_process.stdout_text, "\"rootRecordIndex\": 6",
        "#1451: visual object duplicate-subtree JSON should expose copied root record indexes");
    expect_contains(duplicate_process.stdout_text, "\"copiedCount\": 4",
        "#1451: visual object duplicate-subtree JSON should expose copied counts");
    expect_contains(duplicate_process.stdout_text, "\"affectedObjectCount\": 4",
        "#1451: visual object duplicate-subtree JSON should expose affected object counts");
    expect_contains(duplicate_process.stdout_text, "\"rootObjectName\": \"cntCopy\"",
        "#1451: visual object duplicate-subtree JSON should expose copied root object names");
    expect_contains(duplicate_process.stdout_text, "\"rootUniqueId\": \"container-copy-guid\"",
        "#1451: visual object duplicate-subtree JSON should expose copied root unique ids");
    expect_contains(duplicate_process.stdout_text, "\"rootParentName\": \"\"",
        "#1451: visual object duplicate-subtree JSON should expose copied root parent names");
    expect_contains(duplicate_process.stdout_text, "\"dryRun\": false",
        "#1451: visual object duplicate-subtree JSON should expose committed execution state");
    expect_contains(duplicate_process.stdout_text, "\"mutatesAsset\": true",
        "#1451: visual object duplicate-subtree JSON should expose mutation state");
    expect_contains(duplicate_process.stdout_text, "\"undoAvailable\": false",
        "#1451: visual object duplicate-subtree JSON should expose undo availability state");
    expect(visual_object_count(subtree_path) == 10U &&
            visual_object_exists(subtree_path, "container-copy-guid") &&
            visual_object_exists(subtree_path, "save-copy-guid") &&
            visual_object_exists(subtree_path, "name-copy-guid") &&
            visual_object_exists(subtree_path, "nested-copy-guid"),
        "#1451: visual object duplicate-subtree host command should append root and descendants");
    expect(visual_object_parent(subtree_path, "save-copy-guid") == "cntCopy" &&
            visual_object_parent(subtree_path, "name-copy-guid") == "cntCopy" &&
            visual_object_parent(subtree_path, "nested-copy-guid") == "txtNameCopy",
        "#1451: visual object duplicate-subtree host command should rewrite copied child parents");

    const fs::path collision_path = temp_root / "subtree_duplicate_collision.scx";
    write_synthetic_form_table_for_subtree_deleted_state(collision_path);
    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", collision_path.string(),
            "--object-name", "cntMain",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntCollision",
            "--new-name", "cntCollision",
            "--new-unique-id", "other-guid",
            "--replacement-source-unique-id", "save-guid",
            "--new-object-name", "cmdCollision",
            "--new-name", "cmdCollision",
            "--new-unique-id", "collision-save-guid",
            "--replacement-source-unique-id", "name-guid",
            "--new-object-name", "txtCollision",
            "--new-name", "txtCollision",
            "--new-unique-id", "collision-name-guid",
            "--replacement-source-unique-id", "nested-guid",
            "--new-object-name", "lblCollision",
            "--new-name", "lblCollision",
            "--new-unique-id", "collision-nested-guid",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1451: visual object duplicate-subtree JSON should reject replacement identity collisions");
    expect_contains(collision_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
        "#1451: failed visual object duplicate-subtree JSON should not expose a subtree duplicate object");
    expect_contains(collision_process.stdout_text, "The requested replacement identity already exists in the asset.",
        "#1451: collision visual object duplicate-subtree JSON should report editor errors");
    expect(visual_object_count(collision_path) == 6U,
        "#1451: collision visual object duplicate-subtree commands should not mutate the asset");

    const fs::path missing_replacement_path = temp_root / "subtree_duplicate_missing_replacement.scx";
    write_synthetic_form_table_for_subtree_deleted_state(missing_replacement_path);
    const auto missing_replacement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", missing_replacement_path.string(),
            "--unique-id", "container-guid",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntIncomplete",
            "--new-name", "cntIncomplete",
            "--new-unique-id", "container-incomplete-guid",
            "--json"
        },
        temp_root);
    expect(missing_replacement_process.exit_code == 4,
        "#1451: visual object duplicate-subtree JSON should reject missing descendant replacement maps");
    expect_contains(missing_replacement_process.stdout_text, "Missing or ambiguous subtree replacement identity.",
        "#1451: missing-replacement visual object duplicate-subtree JSON should report editor errors");
    expect(visual_object_count(missing_replacement_path) == 6U,
        "#1451: missing-replacement visual object duplicate-subtree commands should not mutate the asset");

    const fs::path missing_data_path = temp_root / "subtree_duplicate_missing_data.scx";
    write_synthetic_form_table_for_subtree_deleted_state(missing_data_path);
    const auto missing_data_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", missing_data_path.string(),
            "--unique-id", "container-guid",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntMissingData",
            "--new-name", "cntMissingData",
            "--new-unique-id", "container-missing-data-guid",
            "--replacement-source-unique-id", "save-guid",
            "--new-object-name", "cmdMissingData",
            "--new-name", "cmdMissingData",
            "--new-unique-id", "save-missing-data-guid",
            "--replacement-source-unique-id", "name-guid",
            "--new-object-name", "txtMissingData",
            "--new-unique-id", "name-missing-data-guid",
            "--replacement-source-unique-id", "nested-guid",
            "--new-object-name", "lblMissingData",
            "--new-name", "lblMissingData",
            "--new-unique-id", "nested-missing-data-guid",
            "--json"
        },
        temp_root);
    expect(missing_data_process.exit_code == 4,
        "#1451: visual object duplicate-subtree JSON should reject incomplete replacement identity data");
    expect_contains(missing_data_process.stdout_text, "Missing subtree replacement identity data.",
        "#1451: missing-data visual object duplicate-subtree JSON should report editor errors");
    expect(visual_object_count(missing_data_path) == 6U,
        "#1451: missing-data visual object duplicate-subtree commands should not mutate the asset");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--unique-id", "container-guid",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntMissingPath",
            "--new-name", "cntMissingPath",
            "--new-unique-id", "container-missing-path-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1451: visual object duplicate-subtree JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectDuplicateSubtree\": null",
        "#1451: missing-path visual object duplicate-subtree JSON should not expose a subtree duplicate object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1451: missing-path visual object duplicate-subtree JSON should report parser errors");

    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", subtree_path.string(),
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntMissingSelector",
            "--new-name", "cntMissingSelector",
            "--new-unique-id", "container-missing-selector-guid",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1451: visual object duplicate-subtree JSON should reject missing root selectors");
    expect_contains(missing_selector_process.stdout_text, "No root object selector was provided.",
        "#1451: missing-root-selector visual object duplicate-subtree JSON should report parser errors");

    const auto no_replacements_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", subtree_path.string(),
            "--unique-id", "container-guid",
            "--json"
        },
        temp_root);
    expect(no_replacements_process.exit_code == 2,
        "#1451: visual object duplicate-subtree JSON should reject empty replacement lists");
    expect_contains(no_replacements_process.stdout_text, "No subtree replacement identities were provided.",
        "#1451: empty visual object duplicate-subtree JSON should report parser errors");

    const auto option_before_replacement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", subtree_path.string(),
            "--unique-id", "container-guid",
            "--new-object-name", "NoReplacementSource",
            "--replacement-source-unique-id", "container-guid",
            "--json"
        },
        temp_root);
    expect(option_before_replacement_process.exit_code == 2,
        "#1451: visual object duplicate-subtree JSON should reject replacement options before source ids");
    expect_contains(option_before_replacement_process.stdout_text,
        "Subtree duplicate replacement options require a preceding --replacement-source-unique-id.",
        "#1451: option-before-replacement visual object duplicate-subtree JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-subtree",
            "--path", subtree_path.string(),
            "--record", "-1",
            "--replacement-source-unique-id", "container-guid",
            "--new-object-name", "cntBadRecord",
            "--new-name", "cntBadRecord",
            "--new-unique-id", "container-bad-record-guid",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1451: visual object duplicate-subtree JSON should reject invalid record selectors");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1451: invalid-record visual object duplicate-subtree JSON should report parser errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-duplicate-subtree --path <asset>",
        "#1451: usage text should expose visual object duplicate-subtree commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_delete_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--delete-object",
            "--object-name", "txt1",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1023: object-name host object deletes should exit successfully");
    expect(visual_object_deleted(object_name_path, "existing-textbox-guid"),
        "#1023: object-name host object deletes should mark the targeted object deleted");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--delete-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1023: unique-id host object deletes should exit successfully");
    expect(visual_object_deleted(unique_id_path, "existing-textbox-guid"),
        "#1023: unique-id host object deletes should mark the targeted object deleted");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--delete-object",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1023: missing object-name host object deletes should return command failure");
    expect(!visual_object_deleted(missing_path, "existing-textbox-guid"),
        "#1023: missing object-name host object deletes should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--delete-object",
            "--clear-property",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1023: delete-object plus property command requests should fail during launch parsing");
    expect(!visual_object_deleted(ambiguous_path, "existing-textbox-guid"),
        "#1023: delete-object/property ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_restore_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    delete_existing_textbox(object_name_path,
        "#1024: restore-object object-name fixture should start with a deleted target");
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--restore-object",
            "--object-name", "txt1",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1024: object-name host object restores should exit successfully");
    expect(!visual_object_deleted(object_name_path, "existing-textbox-guid"),
        "#1024: object-name host object restores should clear the targeted object's deleted state");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    delete_existing_textbox(unique_id_path,
        "#1024: restore-object unique-id fixture should start with a deleted target");
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--restore-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1024: unique-id host object restores should exit successfully");
    expect(!visual_object_deleted(unique_id_path, "existing-textbox-guid"),
        "#1024: unique-id host object restores should clear the targeted object's deleted state");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    delete_existing_textbox(missing_path,
        "#1024: restore-object missing-object fixture should start with a deleted target");
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--restore-object",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1024: missing object-name host object restores should return command failure");
    expect(visual_object_deleted(missing_path, "existing-textbox-guid"),
        "#1024: missing object-name host object restores should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    delete_existing_textbox(ambiguous_path,
        "#1024: restore-object ambiguity fixture should start with a deleted target");
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--delete-object",
            "--restore-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1024: delete-object plus restore-object requests should fail during launch parsing");
    expect(visual_object_deleted(ambiguous_path, "existing-textbox-guid"),
        "#1024: delete-object/restore-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_duplicate_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--duplicate-object",
            "--object-name", "txt1",
            "--new-object-name", "txtCopy",
            "--new-name", "txtCopy",
            "--new-unique-id", "copied-textbox-guid",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1025: object-name host object duplicates should exit successfully");
    expect(visual_object_count(object_name_path) == 3U,
        "#1025: object-name host object duplicates should append one visual object");
    expect(visual_object_exists(object_name_path, "copied-textbox-guid"),
        "#1025: object-name host object duplicates should use replacement unique ids");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--duplicate-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "txtCopyById",
            "--new-name", "txtCopyById",
            "--new-unique-id", "copied-by-id-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1025: unique-id host object duplicates should exit successfully");
    expect(visual_object_count(unique_id_path) == 3U,
        "#1025: unique-id host object duplicates should append one visual object");
    expect(visual_object_exists(unique_id_path, "copied-by-id-guid"),
        "#1025: unique-id host object duplicates should use replacement unique ids");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--duplicate-object",
            "--object-name", "missingObject",
            "--new-object-name", "missingCopy",
            "--new-name", "missingCopy",
            "--new-unique-id", "missing-copy-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1025: missing object-name host object duplicates should return command failure");
    expect(visual_object_count(missing_path) == 2U,
        "#1025: missing object-name host object duplicates should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--duplicate-object",
            "--delete-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "ambiguousCopy",
            "--new-name", "ambiguousCopy",
            "--new-unique-id", "ambiguous-copy-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1025: duplicate-object plus delete-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 2U,
        "#1025: duplicate-object/delete-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_rename_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_toolbox_creation(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--rename-object",
            "--object-name", "txt1",
            "--new-object-name", "txtCustomer",
            "--new-name", "txtCustomer",
            "--new-unique-id", "customer-textbox-guid",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1026: object-name host object renames should exit successfully");
    expect(visual_object_count(object_name_path) == 2U,
        "#1026: object-name host object renames should not append visual objects");
    expect(visual_object_exists(object_name_path, "customer-textbox-guid") &&
            !visual_object_exists(object_name_path, "existing-textbox-guid"),
        "#1026: object-name host object renames should replace the target identity");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_toolbox_creation(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "txtCustomerById",
            "--new-name", "txtCustomerById",
            "--new-unique-id", "customer-by-id-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1026: unique-id host object renames should exit successfully");
    expect(visual_object_count(unique_id_path) == 2U,
        "#1026: unique-id host object renames should not append visual objects");
    expect(visual_object_exists(unique_id_path, "customer-by-id-guid") &&
            !visual_object_exists(unique_id_path, "existing-textbox-guid"),
        "#1026: unique-id host object renames should replace the target identity");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--rename-object",
            "--object-name", "missingObject",
            "--new-object-name", "missingRename",
            "--new-name", "missingRename",
            "--new-unique-id", "missing-rename-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1026: missing object-name host object renames should return command failure");
    expect(visual_object_count(missing_path) == 2U &&
            visual_object_exists(missing_path, "existing-textbox-guid"),
        "#1026: missing object-name host object renames should not mutate the asset");

    const fs::path empty_identity_path = temp_root / "empty_identity.scx";
    write_synthetic_form_table_for_toolbox_creation(empty_identity_path);
    const auto empty_identity_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_identity_path.string(),
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(empty_identity_process.exit_code == 2,
        "#1026: rename-object without replacement identity fields should fail during launch parsing");
    expect(visual_object_count(empty_identity_path) == 2U &&
            visual_object_exists(empty_identity_path, "existing-textbox-guid"),
        "#1026: rename-object empty-identity failures should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_toolbox_creation(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--rename-object",
            "--duplicate-object",
            "--unique-id", "existing-textbox-guid",
            "--new-object-name", "ambiguousRename",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1026: rename-object plus duplicate-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 2U &&
            visual_object_exists(ambiguous_path, "existing-textbox-guid"),
        "#1026: rename-object/duplicate-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_STABLE_SELECTOR_MUTATION) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_OBJECT_ACTION_LIFECYCLE) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_SKIP_REMAINING_OBJECT_ACTIONS)
void test_studio_host_json_reparents_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_reparent_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_object_reparent(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--reparent-object",
            "--object-name", "txt1",
            "--parent-name", "cntPanel",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1027: object-name host object reparent should exit successfully");
    expect(visual_object_parent(object_name_path, "existing-textbox-guid") == "cntPanel",
        "#1027: object-name host object reparent should update the target parent");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_object_reparent(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-unique-id", "panel-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1027: unique-id host object reparent should exit successfully");
    expect(visual_object_parent(unique_id_path, "existing-textbox-guid") == "cntPanel",
        "#1027: unique-id host object reparent should update the target parent");

    const fs::path clear_parent_path = temp_root / "clear_parent.scx";
    write_synthetic_form_table_for_object_reparent(clear_parent_path);
    const auto clear_parent_process = run_process_capture(
        studio_host_path,
        {
            "--path", clear_parent_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--clear-parent",
            "--json"
        },
        temp_root);
    expect(clear_parent_process.exit_code == 0,
        "#1027: clear-parent host object reparent should exit successfully");
    expect(visual_object_parent(clear_parent_path, "existing-textbox-guid").empty(),
        "#1027: clear-parent host object reparent should clear the target parent");

    const fs::path missing_source_path = temp_root / "missing_source.scx";
    write_synthetic_form_table_for_object_reparent(missing_source_path);
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_source_path.string(),
            "--reparent-object",
            "--object-name", "missingObject",
            "--parent-name", "cntPanel",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1027: missing source host object reparent should return command failure");
    expect(visual_object_parent(missing_source_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: missing source host object reparent should not mutate the asset");

    const fs::path missing_parent_path = temp_root / "missing_parent.scx";
    write_synthetic_form_table_for_object_reparent(missing_parent_path);
    const auto missing_parent_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_parent_path.string(),
            "--reparent-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-name", "missingParent",
            "--json"
        },
        temp_root);
    expect(missing_parent_process.exit_code == 4,
        "#1027: missing parent host object reparent should return command failure");
    expect(visual_object_parent(missing_parent_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: missing parent host object reparent should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_reparent(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--reparent-object",
            "--rename-object",
            "--unique-id", "existing-textbox-guid",
            "--parent-name", "cntPanel",
            "--new-object-name", "txtCustomer",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1027: reparent-object plus rename-object requests should fail during launch parsing");
    expect(visual_object_parent(ambiguous_path, "existing-textbox-guid") == "frmCustomer",
        "#1027: reparent-object/rename-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_reorder_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path reorder_path = temp_root / "reorder.scx";
    write_synthetic_form_table_for_object_reorder(reorder_path);

    const auto front_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "c-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(front_process.exit_code == 0,
        "#1028: front host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,b-guid,d-guid",
        "#1028: front host object reorder should move the selected object to the front");

    const auto back_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--object-name", "cmdA",
            "--placement", "back",
            "--json"
        },
        temp_root);
    expect(back_process.exit_code == 0,
        "#1028: back host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,b-guid,d-guid,a-guid",
        "#1028: back host object reorder should move the selected object to the back");

    const auto before_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "a-guid",
            "--placement", "before",
            "--target-object-name", "cmdB",
            "--json"
        },
        temp_root);
    expect(before_process.exit_code == 0,
        "#1028: before host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,b-guid,d-guid",
        "#1028: before host object reorder should move the selected object before the target object name");

    const auto after_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "after",
            "--target-unique-id", "d-guid",
            "--json"
        },
        temp_root);
    expect(after_process.exit_code == 0,
        "#1028: after host object reorder should exit successfully");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: after host object reorder should move the selected object after the target unique id");

    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "before",
            "--target-object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1028: missing target host object reorder should return command failure");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: missing target host object reorder should not mutate order");

    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--unique-id", "b-guid",
            "--placement", "sideways",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1028: unsupported placement host object reorder should return command failure");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: unsupported placement host object reorder should not mutate order");

    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", reorder_path.string(),
            "--reorder-object",
            "--reparent-object",
            "--unique-id", "b-guid",
            "--placement", "front",
            "--parent-name", "cmdA",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1028: reorder-object plus reparent-object requests should fail during launch parsing");
    expect(visual_object_order(reorder_path) == "c-guid,a-guid,d-guid,b-guid",
        "#1028: reorder-object/reparent-object ambiguity should not mutate order");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_groups_objects_by_stable_child_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_group_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path group_path = temp_root / "group.scx";
    write_synthetic_form_table_for_object_group(group_path);
    const auto group_process = run_process_capture(
        studio_host_path,
        {
            "--path", group_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--field-value", "NAME=cntGroup",
            "--field-value", "UNIQUEID=group-guid",
            "--field-value", "PARENT=frmCustomer",
            "--field-value", "CLASS=Container",
            "--field-value", "BASECLASS=Container",
            "--field-value", "PROPERTIES=Caption = \"Group\"",
            "--group-child-object-name", "cmdSave",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(group_process.exit_code == 0,
        "#1030: host object group should exit successfully");
    expect(visual_object_count(group_path) == 5U &&
            visual_object_parent(group_path, "save-guid") == "cntGroup" &&
            visual_object_parent(group_path, "name-guid") == "cntGroup" &&
            visual_object_parent(group_path, "status-guid") == "frmCustomer",
        "#1030: host object group should append a container and reparent only selected children");

    const fs::path missing_child_path = temp_root / "missing_child.scx";
    write_synthetic_form_table_for_object_group(missing_child_path);
    const auto missing_child_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_child_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--field-value", "UNIQUEID=group-guid",
            "--group-child-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_child_process.exit_code == 4,
        "#1030: missing child host object group should return command failure");
    expect(visual_object_count(missing_child_path) == 4U &&
            visual_object_parent(missing_child_path, "save-guid") == "frmCustomer" &&
            visual_object_parent(missing_child_path, "name-guid") == "frmCustomer",
        "#1030: missing child host object group should not mutate the asset");

    const fs::path missing_field_values_path = temp_root / "missing_field_values.scx";
    write_synthetic_form_table_for_object_group(missing_field_values_path);
    const auto missing_field_values_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_field_values_path.string(),
            "--group-object",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(missing_field_values_process.exit_code == 2,
        "#1030: group-object without container field values should fail during launch parsing");
    expect(visual_object_count(missing_field_values_path) == 4U &&
            visual_object_parent(missing_field_values_path, "name-guid") == "frmCustomer",
        "#1030: group-object without container field values should not mutate the asset");

    const fs::path missing_children_path = temp_root / "missing_children.scx";
    write_synthetic_form_table_for_object_group(missing_children_path);
    const auto missing_children_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_children_path.string(),
            "--group-object",
            "--field-value", "OBJNAME=cntGroup",
            "--json"
        },
        temp_root);
    expect(missing_children_process.exit_code == 2,
        "#1030: group-object without child selectors should fail during launch parsing");
    expect(visual_object_count(missing_children_path) == 4U &&
            visual_object_parent(missing_children_path, "save-guid") == "frmCustomer",
        "#1030: group-object without child selectors should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_group(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--group-object",
            "--ungroup-object",
            "--field-value", "OBJNAME=cntGroup",
            "--group-child-unique-id", "name-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1030: group-object plus ungroup-object requests should fail during launch parsing");
    expect(visual_object_count(ambiguous_path) == 4U &&
            visual_object_parent(ambiguous_path, "name-guid") == "frmCustomer",
        "#1030: group-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_delete_mark_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_delete_mark_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path delete_mark_path = temp_root / "delete_mark.scx";
    write_synthetic_form_table_for_object_delete_mark(delete_mark_path);
    const auto delete_mark_process = run_process_capture(
        studio_host_path,
        {
            "--path", delete_mark_path.string(),
            "--delete-mark-object",
            "--delete-mark", "false",
            "--delete-mark-target-object-name", "frmCustomer",
            "--delete-mark-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(delete_mark_process.exit_code == 0,
        "#1087: host object delete-mark assignment should exit successfully");
    expect(visual_object_property(delete_mark_path, "one-guid", "DELETEMARK") == "false" &&
            visual_object_property(delete_mark_path, "two-guid", "DELETEMARK") == "false" &&
            visual_object_property(delete_mark_path, "three-guid", "DELETEMARK") == "false" &&
            visual_object_property(delete_mark_path, "other-guid", "DELETEMARK") == "true",
        "#1087: host object delete-mark assignment should assign selected logical state and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_delete_mark(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--delete-mark-object",
            "--delete-mark", "false",
            "--delete-mark-target-unique-id", "one-guid",
            "--delete-mark-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1087: missing-target host object delete-mark assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DELETEMARK") == "true" &&
            visual_object_property(missing_target_path, "two-guid", "DELETEMARK") == "true",
        "#1087: missing-target host object delete-mark assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_delete_mark(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--delete-mark-object",
            "--delete-mark", "false",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1087: delete-mark-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DELETEMARK") == "true",
        "#1087: delete-mark-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_delete_mark(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--delete-mark-object",
            "--delete-mark-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1087: delete-mark-object without delete-mark value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DELETEMARK") == "true",
        "#1087: delete-mark-object without delete-mark value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_delete_mark(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--delete-mark-object",
            "--delete-mark", "false",
            "--delete-mark-target-unique-id", "one-guid",
            "--delete-mark-target-object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1087: duplicate-target host object delete-mark assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DELETEMARK") == "true",
        "#1087: duplicate-target host object delete-mark assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_delete_mark(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--delete-mark-object",
            "--auto-size-object",
            "--delete-mark", "false",
            "--delete-mark-target-unique-id", "one-guid",
            "--auto-size", "false",
            "--auto-size-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1087: delete-mark-object plus auto-size-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DELETEMARK") == "true",
        "#1087: delete-mark-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_ungroups_objects_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_ungroup_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path object_name_path = temp_root / "object_name.scx";
    write_synthetic_form_table_for_object_ungroup(object_name_path);
    const auto object_name_process = run_process_capture(
        studio_host_path,
        {
            "--path", object_name_path.string(),
            "--ungroup-object",
            "--object-name", "cntGroup",
            "--json"
        },
        temp_root);
    expect(object_name_process.exit_code == 0,
        "#1029: object-name host object ungroup should exit successfully");
    expect(visual_object_is_deleted(object_name_path, "group-guid") &&
            visual_object_parent(object_name_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(object_name_path, "save-guid") == "frmCustomer",
        "#1029: object-name host object ungroup should move children to the container parent and delete the container");

    const fs::path unique_id_path = temp_root / "unique_id.scx";
    write_synthetic_form_table_for_object_ungroup(unique_id_path);
    const auto unique_id_process = run_process_capture(
        studio_host_path,
        {
            "--path", unique_id_path.string(),
            "--ungroup-object",
            "--unique-id", "group-guid",
            "--json"
        },
        temp_root);
    expect(unique_id_process.exit_code == 0,
        "#1029: unique-id host object ungroup should exit successfully");
    expect(visual_object_is_deleted(unique_id_path, "group-guid") &&
            visual_object_parent(unique_id_path, "name-guid") == "frmCustomer" &&
            visual_object_parent(unique_id_path, "save-guid") == "frmCustomer",
        "#1029: unique-id host object ungroup should move children to the container parent and delete the container");

    const fs::path root_path = temp_root / "root.scx";
    write_synthetic_form_table_for_object_ungroup(root_path);
    const auto root_process = run_process_capture(
        studio_host_path,
        {
            "--path", root_path.string(),
            "--ungroup-object",
            "--unique-id", "root-group-guid",
            "--json"
        },
        temp_root);
    expect(root_process.exit_code == 0,
        "#1029: root-level host object ungroup should exit successfully");
    expect(visual_object_is_deleted(root_path, "root-group-guid") &&
            visual_object_parent(root_path, "root-child-guid").empty(),
        "#1029: root-level host object ungroup should clear child parents and delete the container");

    const fs::path empty_path = temp_root / "empty.scx";
    write_synthetic_form_table_for_object_ungroup(empty_path);
    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--path", empty_path.string(),
            "--ungroup-object",
            "--unique-id", "empty-guid",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 4,
        "#1029: empty-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(empty_path, "empty-guid") &&
            visual_object_parent(empty_path, "name-guid") == "cntGroup",
        "#1029: empty-container host object ungroup should not mutate the asset");

    const fs::path missing_path = temp_root / "missing.scx";
    write_synthetic_form_table_for_object_ungroup(missing_path);
    const auto missing_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_path.string(),
            "--ungroup-object",
            "--unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_process.exit_code == 4,
        "#1029: missing-container host object ungroup should return command failure");
    expect(!visual_object_is_deleted(missing_path, "group-guid") &&
            visual_object_parent(missing_path, "name-guid") == "cntGroup",
        "#1029: missing-container host object ungroup should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_ungroup(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--ungroup-object",
            "--reorder-object",
            "--unique-id", "group-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1029: ungroup-object plus reorder-object requests should fail during launch parsing");
    expect(!visual_object_is_deleted(ambiguous_path, "group-guid") &&
            visual_object_parent(ambiguous_path, "name-guid") == "cntGroup",
        "#1029: ungroup-object/reorder-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif
}  // namespace cf_test_studio_host_json
