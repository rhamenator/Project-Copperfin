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

#include "test_studio_host_json_object_lifecycle_selection_toolbox_batches.inl"

#include "test_studio_host_json_object_lifecycle_selection_toolbox_objects.inl"
#include "test_studio_host_json_object_lifecycle_toolbox_objects.inl"
#endif

#if !defined(COPPERFIN_OBJECT_LIFECYCLE_ONLY_TOOLBOX_CREATE) && \
    !defined(COPPERFIN_OBJECT_LIFECYCLE_SKIP_STABLE_SELECTOR_MUTATION)
#include "test_studio_host_json_object_lifecycle_stable_selector_property.inl"

#include "test_studio_host_json_object_lifecycle_stable_selector_property_rename.inl"

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

#include "test_studio_host_json_object_lifecycle_stable_selector_ungroup.inl"

#endif
}  // namespace cf_test_studio_host_json
