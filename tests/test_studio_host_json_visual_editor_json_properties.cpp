// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_visual_property_filter(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_filter_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path direct_path = temp_root / "direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);

    const auto direct_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-filter",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-filter-text", "CAPTION",
            "--json"
        },
        temp_root);
    expect(direct_process.exit_code == 0,
        "#1415: visual property filter JSON should exit successfully for matching direct fields");
    expect_contains(direct_process.stdout_text, "\"visualPropertyFilter\": {",
        "#1415: visual property filter JSON should expose a filter object");
    expect_contains(direct_process.stdout_text, "\"recordIndex\": 1",
        "#1415: visual property filter JSON should expose resolved record indexes");
    expect_contains(direct_process.stdout_text, "\"recordDeleted\": false",
        "#1415: visual property filter JSON should expose selected-record deleted state");
    expect_contains(direct_process.stdout_text, "\"searchText\": \"CAPTION\"",
        "#1415: visual property filter JSON should expose search text");
    expect_contains(direct_process.stdout_text, "\"propertyCount\": 1",
        "#1415: visual property filter JSON should expose filtered property counts");
    expect_contains(direct_process.stdout_text, "\"dryRun\": true",
        "#1415: visual property filter JSON should remain dry-run");
    expect_contains(direct_process.stdout_text, "\"mutatesAsset\": false",
        "#1415: visual property filter JSON should remain non-mutating");
    expect_contains(direct_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1415: visual property filter JSON should include matching property names");
    expect_contains(direct_process.stdout_text, "\"value\": \"Existing\"",
        "#1415: visual property filter JSON should include matching property values");
    expect_contains(direct_process.stdout_text, "\"directField\": true",
        "#1415: visual property filter JSON should identify direct DBF fields");
    expect_contains(direct_process.stdout_text, "\"fieldType\": \"C\"",
        "#1415: visual property filter JSON should expose direct DBF field types");
    expect_contains(direct_process.stdout_text, "\"sourceLineIndex\": null",
        "#1415: visual property filter JSON should null direct-field source lines");

    const fs::path memo_path = write_synthetic_form_table_for_property_rename(temp_root, "memo.scx");
    const auto memo_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-filter",
            "--path", memo_path.string(),
            "--object-name", "txt1",
            "--property-filter-text", "customer.name",
            "--json"
        },
        temp_root);
    expect(memo_process.exit_code == 0,
        "#1415: visual property filter JSON should exit successfully for matching memo properties");
    expect_contains(memo_process.stdout_text, "\"propertyName\": \"ControlSource\"",
        "#1415: visual property filter JSON should include memo-backed property names");
    expect_contains(memo_process.stdout_text, "\"value\": \"\\\"customer.name\\\"\"",
        "#1415: visual property filter JSON should include memo-backed property values");
    expect_contains(memo_process.stdout_text, "\"directField\": false",
        "#1415: visual property filter JSON should identify memo-backed properties");
    expect_contains(memo_process.stdout_text, "\"fieldType\": null",
        "#1415: visual property filter JSON should null memo-backed field types");
    expect_contains(memo_process.stdout_text, "\"sourceLineIndex\": 0",
        "#1415: visual property filter JSON should expose memo-backed source lines");

    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-filter",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-filter-text", "does-not-match",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 0,
        "#1415: visual property filter JSON should succeed for empty result sets");
    expect_contains(empty_process.stdout_text, "\"propertyCount\": 0",
        "#1415: visual property filter JSON should report zero matches");
    expect_contains(empty_process.stdout_text, "\"properties\": [\n    ]",
        "#1415: visual property filter JSON should expose an empty property array");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-filter",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1415: visual property filter JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyFilter\": null",
        "#1415: missing-path visual property filter JSON should not expose a filter object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1415: missing-path visual property filter JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-filter",
            "--path", direct_path.string(),
            "--object-name", "missingObject",
            "--property-filter-text", "Caption",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1415: visual property filter JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyFilter\": null",
        "#1415: unresolved visual property filter JSON should not expose a filter object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1415: unresolved visual property filter JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-filter --path <asset>",
        "#1415: usage text should expose visual property filter commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_property_query(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_query_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path direct_path = temp_root / "direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);

    const auto direct_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(direct_process.exit_code == 0,
        "#1416: visual property query JSON should exit successfully for direct fields");
    expect_contains(direct_process.stdout_text, "\"visualPropertyQuery\": {",
        "#1416: visual property query JSON should expose a query object");
    expect_contains(direct_process.stdout_text, "\"exists\": true",
        "#1416: visual property query JSON should report existing direct fields");
    expect_contains(direct_process.stdout_text, "\"directField\": true",
        "#1416: visual property query JSON should identify direct DBF fields");
    expect_contains(direct_process.stdout_text, "\"recordIndex\": 1",
        "#1416: visual property query JSON should expose resolved record indexes");
    expect_contains(direct_process.stdout_text, "\"recordDeleted\": false",
        "#1416: visual property query JSON should expose selected-record deleted state");
    expect_contains(direct_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1416: visual property query JSON should expose resolved direct property names");
    expect_contains(direct_process.stdout_text, "\"value\": \"Existing\"",
        "#1416: visual property query JSON should expose direct property values");

    const fs::path memo_path = write_synthetic_form_table_for_property_rename(temp_root, "memo_query.scx");
    const auto memo_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", memo_path.string(),
            "--object-name", "txt1",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(memo_process.exit_code == 0,
        "#1416: visual property query JSON should exit successfully for memo-backed properties");
    expect_contains(memo_process.stdout_text, "\"exists\": true",
        "#1416: visual property query JSON should report existing memo-backed properties");
    expect_contains(memo_process.stdout_text, "\"directField\": false",
        "#1416: visual property query JSON should identify memo-backed properties");
    expect_contains(memo_process.stdout_text, "\"propertyName\": \"ControlSource\"",
        "#1416: visual property query JSON should expose resolved memo property names");
    expect_contains(memo_process.stdout_text, "\"value\": \"\\\"customer.name\\\"\"",
        "#1416: visual property query JSON should expose memo-backed property values");

    const auto missing_property_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--json"
        },
        temp_root);
    expect(missing_property_process.exit_code == 0,
        "#1416: visual property query JSON should succeed for missing properties on resolved objects");
    expect_contains(missing_property_process.stdout_text, "\"exists\": false",
        "#1416: visual property query JSON should report missing property existence");
    expect_contains(missing_property_process.stdout_text, "\"propertyName\": \"MissingProperty\"",
        "#1416: visual property query JSON should preserve requested missing property names");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1416: visual property query JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyQuery\": null",
        "#1416: missing-path visual property query JSON should not expose a query object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1416: missing-path visual property query JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", direct_path.string(),
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1416: visual property query JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1416: missing property-name visual property query JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-query",
            "--path", direct_path.string(),
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1416: visual property query JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyQuery\": null",
        "#1416: unresolved visual property query JSON should not expose a query object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1416: unresolved visual property query JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-query --path <asset>",
        "#1416: usage text should expose visual property query commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#include "test_studio_host_json_visual_editor_json_properties_clear_copy.inl"

void test_studio_host_json_moves_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_move_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "move.scx");

    const auto move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(move_process.exit_code == 0,
        "#1439: visual property move JSON should exit successfully for memo-backed properties");
    expect_contains(move_process.stdout_text, "\"visualPropertyMove\": {",
        "#1439: visual property move JSON should expose a move object");
    expect_contains(move_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1439: visual property move JSON should expose affected object counts");
    expect_contains(move_process.stdout_text, "\"dryRun\": false",
        "#1439: visual property move JSON should expose committed execution state");
    expect_contains(move_process.stdout_text, "\"mutatesAsset\": true",
        "#1439: visual property move JSON should expose mutation state");
    expect_contains(move_process.stdout_text, "\"undoAvailable\": true",
        "#1439: visual property move JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1439: visual property move host command should move values, clear sources, and preserve unrelated properties");

    const fs::path default_path = write_synthetic_form_table_for_property_rename(temp_root, "move_default.scx");
    const auto default_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", default_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(default_name_process.exit_code == 0,
        "#1439: visual property move JSON should default target property names from source properties");
    expect(visual_object_property(default_path, "form-guid", "Left") == "12" &&
            visual_object_property(default_path, "existing-textbox-guid", "Left").empty(),
        "#1439: visual property move host command should default target property names and clear sources");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1439: visual property move JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: target-collision visual property move JSON should not expose a move object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1439: target-collision visual property move JSON should report editor errors");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1439: failed visual property move commands should not mutate source or target properties");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1439: visual property move JSON should allow explicit replacement");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1439: visual property move host command should replace targets and clear sources when requested");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "move_self.scx");
    const auto self_move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", self_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_move_process.exit_code == 4,
        "#1439: visual property move JSON should reject self-moves");
    expect_contains(self_move_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: self-move visual property move JSON should not expose a move object");
    expect_contains(self_move_process.stdout_text, "The source property cannot be moved onto itself.",
        "#1439: self-move visual property move JSON should report editor errors");
    expect(visual_object_property(self_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1439: self-move visual property move commands should not mutate source properties");

    const fs::path missing_source_path = write_synthetic_form_table_for_property_rename(temp_root, "move_missing.scx");
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--target-unique-id", "form-guid",
            "--target-property-name", "CopiedMissing",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1439: visual property move JSON should reject missing source properties");
    expect_contains(missing_source_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: missing-source visual property move JSON should not expose a move object");
    expect_contains(missing_source_process.stdout_text, "The source property was not found.",
        "#1439: missing-source visual property move JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1439: visual property move JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: missing-path visual property move JSON should not expose a move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1439: missing-path visual property move JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1439: visual property move JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1439: missing property-name visual property move JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-record", "-1",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1439: invalid-source-record visual property move JSON should report parser errors");

    const auto invalid_target_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_target_record_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid target record values");
    expect_contains(invalid_target_record_process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#1439: invalid-target-record visual property move JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1439: visual property move JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1439: invalid replace-existing visual property move JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move",
            "--path", missing_source_path.string(),
            "--source-object-name", "missingObject",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1439: visual property move JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyMove\": null",
        "#1439: unresolved visual property move JSON should not expose a move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1439: unresolved visual property move JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-move --path <asset>",
        "#1439: usage text should expose visual property move commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_move_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch.scx");

    const auto move_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormLeft",
            "--json"
        },
        temp_root);
    expect(move_batch_process.exit_code == 0,
        "#1440: visual property move-batch JSON should exit successfully for valid batches");
    expect_contains(move_batch_process.stdout_text, "\"visualPropertyMoveBatch\": {",
        "#1440: visual property move-batch JSON should expose a batch move object");
    expect_contains(move_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1440: visual property move-batch JSON should expose affected item counts");
    expect_contains(move_batch_process.stdout_text, "\"dryRun\": false",
        "#1440: visual property move-batch JSON should expose committed execution state");
    expect_contains(move_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1440: visual property move-batch JSON should expose mutation state");
    expect_contains(move_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1440: visual property move-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "form-guid", "FormLeft") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1440: visual property move-batch host command should move all requested properties and clear sources");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "MovedControlSource",
            "--property-name", "MissingProperty",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "MovedMissing",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject missing source properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: failed visual property move-batch JSON should not expose a batch move object");
    expect_contains(rollback_process.stdout_text, "The source property was not found.",
        "#1440: missing-source visual property move-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "form-guid", "MovedControlSource").empty() &&
            visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1440: failed visual property move-batch commands should roll back earlier moves");

    const fs::path collision_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_collision.scx");
    const auto seed_result = copperfin::vfp::copy_visual_object_property({
        .path = collision_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "existing-textbox-guid",
        .source_property_name = "ControlSource",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "form-guid",
        .target_property_name = "FormControlSource",
        .replace_existing = false
    });
    expect(seed_result.ok,
        "#1440: synthetic visual property move-batch collision fixture should seed target properties");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", collision_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject target collisions by default");
    expect_contains(collision_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: target-collision visual property move-batch JSON should not expose a batch move object");
    expect_contains(collision_process.stdout_text, "The target object already has the requested property.",
        "#1440: target-collision visual property move-batch JSON should report editor errors");
    expect(visual_object_property(collision_path, "form-guid", "FormControlSource") == "\"customer.name\"" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left") == "12",
        "#1440: failed visual property move-batch collision commands should not mutate source or target properties");

    const auto replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", collision_path.string(),
            "--property-name", "Left",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--target-property-name", "FormControlSource",
            "--replace-existing", "true",
            "--json"
        },
        temp_root);
    expect(replace_process.exit_code == 0,
        "#1440: visual property move-batch JSON should allow explicit replacement");
    expect(visual_object_property(collision_path, "form-guid", "FormControlSource") == "12" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left").empty(),
        "#1440: visual property move-batch host command should replace targets and clear sources when requested");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "move_batch_self.scx");
    const auto self_move_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", self_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_move_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject self-moves");
    expect_contains(self_move_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: self-move visual property move-batch JSON should not expose a batch move object");
    expect_contains(self_move_process.stdout_text, "The source property cannot be moved onto itself.",
        "#1440: self-move visual property move-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: missing-path visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1440: missing-path visual property move-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property moves were provided.",
        "#1440: empty visual property move-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--source-unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property move batch item options require a preceding --property-name.",
        "#1440: option-before-item visual property move-batch JSON should report parser errors");

    const auto invalid_source_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-record", "-1",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(invalid_source_record_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid source record values");
    expect_contains(invalid_source_record_process.stdout_text,
        "The --source-record value must be a non-negative integer.",
        "#1440: invalid-source-record visual property move-batch JSON should report parser errors");

    const auto invalid_target_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_target_record_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid target record values");
    expect_contains(invalid_target_record_process.stdout_text,
        "The --target-record value must be a non-negative integer.",
        "#1440: invalid-target-record visual property move-batch JSON should report parser errors");

    const auto invalid_replace_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--replace-existing", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_replace_process.exit_code == 2,
        "#1440: visual property move-batch JSON should reject invalid replace-existing values");
    expect_contains(invalid_replace_process.stdout_text,
        "The --replace-existing value must be true or false.",
        "#1440: invalid replace-existing visual property move-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--source-unique-id", "existing-textbox-guid",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: missing property-name visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1440: missing property-name visual property move-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-move-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--source-object-name", "missingObject",
            "--target-unique-id", "form-guid",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1440: visual property move-batch JSON should reject unresolved source objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyMoveBatch\": null",
        "#1440: unresolved visual property move-batch JSON should not expose a batch move object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1440: unresolved visual property move-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-move-batch --path <asset>",
        "#1440: usage text should expose visual property move-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_rename_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "rename.scx");

    const auto rename_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(rename_process.exit_code == 0,
        "#1441: visual property rename JSON should exit successfully for memo-backed properties");
    expect_contains(rename_process.stdout_text, "\"visualPropertyRename\": {",
        "#1441: visual property rename JSON should expose a rename object");
    expect_contains(rename_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1441: visual property rename JSON should expose affected object counts");
    expect_contains(rename_process.stdout_text, "\"dryRun\": false",
        "#1441: visual property rename JSON should expose committed execution state");
    expect_contains(rename_process.stdout_text, "\"mutatesAsset\": true",
        "#1441: visual property rename JSON should expose mutation state");
    expect_contains(rename_process.stdout_text, "\"undoAvailable\": true",
        "#1441: visual property rename JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1441: visual property rename host command should rename values and preserve unrelated properties");

    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1441: visual property rename JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: target-collision visual property rename JSON should not expose a rename object");
    expect_contains(collision_process.stdout_text, "The target property already exists in the selected object.",
        "#1441: target-collision visual property rename JSON should report editor errors");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12",
        "#1441: failed visual property rename commands should not mutate source or target properties");

    const fs::path same_name_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_same.scx");
    const auto same_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", same_name_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "controlsource",
            "--json"
        },
        temp_root);
    expect(same_name_process.exit_code == 4,
        "#1441: visual property rename JSON should reject same-name renames");
    expect_contains(same_name_process.stdout_text, "The source property cannot be renamed to itself.",
        "#1441: same-name visual property rename JSON should report editor errors");
    expect(visual_object_property(same_name_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1441: same-name visual property rename commands should not mutate source properties");

    const fs::path missing_source_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_missing.scx");
    const auto missing_source_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--new-property-name", "RenamedMissing",
            "--json"
        },
        temp_root);
    expect(missing_source_process.exit_code == 4,
        "#1441: visual property rename JSON should reject missing source properties");
    expect_contains(missing_source_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: missing-source visual property rename JSON should not expose a rename object");
    expect_contains(missing_source_process.stdout_text, "The source property was not found.",
        "#1441: missing-source visual property rename JSON should report editor errors");

    const fs::path direct_path = temp_root / "rename_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--new-property-name", "DisplayCaption",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1441: visual property rename JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: direct-field visual property rename JSON should not expose a rename object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
        "#1441: direct-field visual property rename JSON should report editor errors");
    expect(visual_object_property(direct_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1441: direct-field visual property rename commands should not mutate direct fields");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: missing-path visual property rename JSON should not expose a rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1441: missing-path visual property rename JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1441: missing property-name visual property rename JSON should report parser errors");

    const auto missing_new_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(missing_new_property_name_process.exit_code == 2,
        "#1441: visual property rename JSON should reject missing target property names");
    expect_contains(missing_new_property_name_process.stdout_text, "No target property name was provided.",
        "#1441: missing target property-name visual property rename JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--record", "-1",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1441: visual property rename JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1441: invalid-record visual property rename JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename",
            "--path", missing_source_path.string(),
            "--object-name", "missingObject",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1441: visual property rename JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyRename\": null",
        "#1441: unresolved visual property rename JSON should not expose a rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1441: unresolved visual property rename JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-rename --path <asset>",
        "#1441: usage text should expose visual property rename commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_rename_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch.scx");

    const auto rename_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--new-property-name", "DisplayLeft",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rename_batch_process.exit_code == 0,
        "#1442: visual property rename-batch JSON should exit successfully for valid batches");
    expect_contains(rename_batch_process.stdout_text, "\"visualPropertyRenameBatch\": {",
        "#1442: visual property rename-batch JSON should expose a batch rename object");
    expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1442: visual property rename-batch JSON should expose affected item counts");
    expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
        "#1442: visual property rename-batch JSON should expose committed execution state");
    expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1442: visual property rename-batch JSON should expose mutation state");
    expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1442: visual property rename-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "BoundControlSource") == "\"customer.name\"" &&
            visual_object_property(form_path, "existing-textbox-guid", "DisplayLeft") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource").empty() &&
            visual_object_property(form_path, "existing-textbox-guid", "Left").empty(),
        "#1442: visual property rename-batch host command should rename all requested properties");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", rollback_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "MissingProperty",
            "--new-property-name", "RenamedMissing",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing source properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: failed visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(rollback_process.stdout_text, "The source property was not found.",
        "#1442: missing-source visual property rename-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "existing-textbox-guid", "BoundControlSource").empty() &&
            visual_object_property(rollback_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1442: failed visual property rename-batch commands should roll back earlier renames");

    const fs::path collision_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_collision.scx");
    const auto collision_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", collision_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(collision_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject target collisions");
    expect_contains(collision_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: target-collision visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(collision_process.stdout_text, "The target property already exists in the selected object.",
        "#1442: target-collision visual property rename-batch JSON should report editor errors");
    expect(visual_object_property(collision_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"" &&
            visual_object_property(collision_path, "existing-textbox-guid", "Left") == "12",
        "#1442: failed visual property rename-batch collision commands should not mutate source or target properties");

    const fs::path direct_path = temp_root / "rename_batch_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", direct_path.string(),
            "--property-name", "CAPTION",
            "--new-property-name", "DisplayCaption",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: direct-field visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be renamed per object.",
        "#1442: direct-field visual property rename-batch JSON should report editor errors");

    const fs::path same_name_path = write_synthetic_form_table_for_property_rename(temp_root, "rename_batch_same.scx");
    const auto same_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", same_name_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "controlsource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(same_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject same-name renames");
    expect_contains(same_name_process.stdout_text, "The source property cannot be renamed to itself.",
        "#1442: same-name visual property rename-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing-path visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1442: missing-path visual property rename-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property renames were provided.",
        "#1442: empty visual property rename-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--new-property-name", "BoundControlSource",
            "--property-name", "ControlSource",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property rename batch item options require a preceding --property-name.",
        "#1442: option-before-item visual property rename-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1442: visual property rename-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1442: invalid-record visual property rename-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--new-property-name", "BoundControlSource",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing property-name visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1442: missing property-name visual property rename-batch JSON should report editor errors");

    const auto missing_new_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_new_property_name_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject missing target property names");
    expect_contains(missing_new_property_name_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: missing target property-name visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_new_property_name_process.stdout_text, "No target property name was provided.",
        "#1442: missing target property-name visual property rename-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-rename-batch",
            "--path", form_path.string(),
            "--property-name", "ControlSource",
            "--new-property-name", "BoundControlSource",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1442: visual property rename-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyRenameBatch\": null",
        "#1442: unresolved visual property rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1442: unresolved visual property rename-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-rename-batch --path <asset>",
        "#1442: usage text should expose visual property rename-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_visual_properties(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_reorder_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder.scx");

    const auto reorder_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(reorder_process.exit_code == 0,
        "#1443: visual property reorder JSON should exit successfully for memo-backed properties");
    expect_contains(reorder_process.stdout_text, "\"visualPropertyReorder\": {",
        "#1443: visual property reorder JSON should expose a reorder object");
    expect_contains(reorder_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1443: visual property reorder JSON should expose affected object counts");
    expect_contains(reorder_process.stdout_text, "\"dryRun\": false",
        "#1443: visual property reorder JSON should expose committed execution state");
    expect_contains(reorder_process.stdout_text, "\"mutatesAsset\": true",
        "#1443: visual property reorder JSON should expose mutation state");
    expect_contains(reorder_process.stdout_text, "\"undoAvailable\": true",
        "#1443: visual property reorder JSON should expose undo availability");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "Left,ControlSource" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1443: visual property reorder host command should reorder properties and preserve values");

    const auto relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--json"
        },
        temp_root);
    expect(relative_process.exit_code == 0,
        "#1443: visual property reorder JSON should support relative placements");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1443: visual property reorder host command should apply relative placements");

    const auto missing_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "after",
            "--relative-property-name", "MissingProperty",
            "--json"
        },
        temp_root);
    expect(missing_relative_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject missing relative properties");
    expect_contains(missing_relative_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: missing-relative visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_relative_process.stdout_text, "The relative property was not found.",
        "#1443: missing-relative visual property reorder JSON should report editor errors");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1443: failed visual property reorder commands should not mutate property order");

    const auto self_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--json"
        },
        temp_root);
    expect(self_relative_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject self-relative placement");
    expect_contains(self_relative_process.stdout_text, "The source property cannot be positioned relative to itself.",
        "#1443: self-relative visual property reorder JSON should report editor errors");

    const fs::path direct_path = temp_root / "reorder_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", direct_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: direct-field visual property reorder JSON should not expose a reorder object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
        "#1443: direct-field visual property reorder JSON should report editor errors");
    expect(visual_object_property(direct_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1443: direct-field visual property reorder commands should not mutate direct fields");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: missing-path visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1443: missing-path visual property reorder JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1443: missing property-name visual property reorder JSON should report parser errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "No property placement was provided.",
        "#1443: missing-placement visual property reorder JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--record", "-1",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1443: visual property reorder JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1443: invalid-record visual property reorder JSON should report parser errors");

    const auto unknown_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "middle",
            "--json"
        },
        temp_root);
    expect(unknown_placement_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject unknown placements");
    expect_contains(unknown_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1443: unknown-placement visual property reorder JSON should report editor errors");

    const auto missing_relative_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "Left",
            "--placement", "before",
            "--json"
        },
        temp_root);
    expect(missing_relative_name_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject missing relative names");
    expect_contains(missing_relative_name_process.stdout_text, "No relative property name was provided.",
        "#1443: missing-relative-name visual property reorder JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--property-name", "Left",
            "--placement", "first",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1443: visual property reorder JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyReorder\": null",
        "#1443: unresolved visual property reorder JSON should not expose a reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1443: unresolved visual property reorder JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-reorder --path <asset>",
        "#1443: usage text should expose visual property reorder commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_reorder_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch.scx");

    const auto reorder_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "after",
            "--relative-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(reorder_batch_process.exit_code == 0,
        "#1444: visual property reorder-batch JSON should exit successfully for valid batches");
    expect_contains(reorder_batch_process.stdout_text, "\"visualPropertyReorderBatch\": {",
        "#1444: visual property reorder-batch JSON should expose a batch reorder object");
    expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1444: visual property reorder-batch JSON should expose affected item counts");
    expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
        "#1444: visual property reorder-batch JSON should expose committed execution state");
    expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1444: visual property reorder-batch JSON should expose mutation state");
    expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1444: visual property reorder-batch JSON should expose undo availability");
    expect(visual_object_property_order(form_path, "existing-textbox-guid") == "Left,ControlSource" &&
            visual_object_property(form_path, "existing-textbox-guid", "Left") == "12" &&
            visual_object_property(form_path, "existing-textbox-guid", "ControlSource") == "\"customer.name\"",
        "#1444: visual property reorder-batch host command should reorder requested properties and preserve values");

    const fs::path rollback_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch_rollback.scx");
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", rollback_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "ControlSource",
            "--placement", "after",
            "--relative-property-name", "MissingProperty",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing relative properties");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: failed visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(rollback_process.stdout_text, "The relative property was not found.",
        "#1444: missing-relative visual property reorder-batch JSON should report editor errors");
    expect(visual_object_property_order(rollback_path, "existing-textbox-guid") == "ControlSource,Left",
        "#1444: failed visual property reorder-batch commands should roll back earlier reorders");

    const fs::path direct_path = temp_root / "reorder_batch_direct.scx";
    write_synthetic_form_table_for_toolbox_creation(direct_path);
    const auto direct_field_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", direct_path.string(),
            "--property-name", "CAPTION",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(direct_field_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject direct DBF-backed fields");
    expect_contains(direct_field_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: direct-field visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(direct_field_process.stdout_text, "Direct DBF-backed fields cannot be reordered per object.",
        "#1444: direct-field visual property reorder-batch JSON should report editor errors");

    const fs::path self_path = write_synthetic_form_table_for_property_rename(temp_root, "reorder_batch_self.scx");
    const auto self_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", self_path.string(),
            "--property-name", "Left",
            "--placement", "before",
            "--relative-property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(self_relative_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject self-relative placement");
    expect_contains(self_relative_process.stdout_text, "The source property cannot be positioned relative to itself.",
        "#1444: self-relative visual property reorder-batch JSON should report editor errors");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--property-name", "Left",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing-path visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1444: missing-path visual property reorder-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property reorders were provided.",
        "#1444: empty visual property reorder-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--placement", "first",
            "--property-name", "Left",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property reorder batch item options require a preceding --property-name.",
        "#1444: option-before-item visual property reorder-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1444: visual property reorder-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1444: invalid-record visual property reorder-batch JSON should report parser errors");

    const auto missing_property_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "",
            "--placement", "first",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_property_name_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing property names");
    expect_contains(missing_property_name_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing property-name visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_property_name_process.stdout_text, "No property name was provided.",
        "#1444: missing property-name visual property reorder-batch JSON should report editor errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing placements");
    expect_contains(missing_placement_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: missing-placement visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1444: missing-placement visual property reorder-batch JSON should report editor errors");

    const auto missing_relative_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "before",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_relative_name_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject missing relative names");
    expect_contains(missing_relative_name_process.stdout_text, "No relative property name was provided.",
        "#1444: missing-relative-name visual property reorder-batch JSON should report editor errors");

    const auto unknown_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "middle",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(unknown_placement_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject unknown placements");
    expect_contains(unknown_placement_process.stdout_text, "Unknown property placement was requested.",
        "#1444: unknown-placement visual property reorder-batch JSON should report editor errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-reorder-batch",
            "--path", form_path.string(),
            "--property-name", "Left",
            "--placement", "first",
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1444: visual property reorder-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyReorderBatch\": null",
        "#1444: unresolved visual property reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1444: unresolved visual property reorder-batch JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-reorder-batch --path <asset>",
        "#1444: usage text should expose visual property reorder-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_property_list(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_list_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = write_synthetic_form_table_for_property_rename(temp_root, "list.scx");
    const auto list_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-list",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(list_process.exit_code == 0,
        "#1417: visual property list JSON should exit successfully for resolved objects");
    expect_contains(list_process.stdout_text, "\"visualPropertyList\": {",
        "#1417: visual property list JSON should expose a list object");
    expect_contains(list_process.stdout_text, "\"recordIndex\": 1",
        "#1417: visual property list JSON should expose resolved record indexes");
    expect_contains(list_process.stdout_text, "\"recordDeleted\": false",
        "#1417: visual property list JSON should expose selected-record deleted state");
    expect_contains(list_process.stdout_text, "\"propertyCount\": ",
        "#1417: visual property list JSON should expose property counts");
    expect_contains(list_process.stdout_text, "\"dryRun\": true",
        "#1417: visual property list JSON should remain dry-run");
    expect_contains(list_process.stdout_text, "\"mutatesAsset\": false",
        "#1417: visual property list JSON should remain non-mutating");
    expect_contains(list_process.stdout_text, "\"propertyName\": \"OBJNAME\"",
        "#1417: visual property list JSON should include direct DBF field properties");
    expect_contains(list_process.stdout_text, "\"directField\": true",
        "#1417: visual property list JSON should identify direct DBF fields");
    expect_contains(list_process.stdout_text, "\"fieldType\": \"C\"",
        "#1417: visual property list JSON should expose direct DBF field types");
    expect_contains(list_process.stdout_text, "\"sourceLineIndex\": null",
        "#1417: visual property list JSON should null direct-field source lines");
    expect_contains(list_process.stdout_text, "\"propertyName\": \"ControlSource\"",
        "#1417: visual property list JSON should include memo-backed property names");
    expect_contains(list_process.stdout_text, "\"value\": \"\\\"customer.name\\\"\"",
        "#1417: visual property list JSON should include memo-backed property values");
    expect_contains(list_process.stdout_text, "\"directField\": false",
        "#1417: visual property list JSON should identify memo-backed properties");
    expect_contains(list_process.stdout_text, "\"fieldType\": null",
        "#1417: visual property list JSON should null memo-backed field types");
    expect_contains(list_process.stdout_text, "\"sourceLineIndex\": 0",
        "#1417: visual property list JSON should expose memo-backed source lines");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-list",
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1417: visual property list JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyList\": null",
        "#1417: missing-path visual property list JSON should not expose a list object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1417: missing-path visual property list JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-list",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1417: visual property list JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyList\": null",
        "#1417: unresolved visual property list JSON should not expose a list object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1417: unresolved visual property list JSON should report editor errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-list",
            "--path", form_path.string(),
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1417: visual property list JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1417: invalid-record visual property list JSON should report parser errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-list --path <asset>",
        "#1417: usage text should expose visual property list commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_visual_property_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_property_update_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "update_batch.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto update_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BatchCaption",
            "--property-name", "ToolTipText",
            "--property-value", "Hover text",
            "--json"
        },
        temp_root);
    expect(update_batch_process.exit_code == 0,
        "#1446: visual property update-batch JSON should exit successfully for valid batches");
    expect_contains(update_batch_process.stdout_text, "\"visualPropertyUpdateBatch\": {",
        "#1446: visual property update-batch JSON should expose a batch update object");
    expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 1",
        "#1446: visual property update-batch JSON should expose affected object counts");
    expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
        "#1446: visual property update-batch JSON should expose committed execution state");
    expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1446: visual property update-batch JSON should expose mutation state");
    expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1446: visual property update-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "CAPTION") == "BatchCaption" &&
            visual_object_property(form_path, "existing-textbox-guid", "ToolTipText") == "Hover text",
        "#1446: visual property update-batch host command should update direct and memo-backed properties");

    const fs::path rollback_path = temp_root / "update_batch_rollback.scx";
    write_synthetic_form_table_for_toolbox_creation(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", rollback_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "ShouldRollback",
            "--property-name", "",
            "--property-value", "Noop",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1446: visual property update-batch JSON should reject missing property names");
    expect_contains(rollback_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: failed visual property update-batch JSON should not expose a batch update object");
    expect_contains(rollback_process.stdout_text, "No property name was provided.",
        "#1446: missing-property visual property update-batch JSON should report editor errors");
    expect(visual_object_property(rollback_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1446: failed visual property update-batch commands should roll back earlier property changes");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "MissingPath",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: missing-path visual property update-batch JSON should not expose a batch update object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1446: missing-path visual property update-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No property changes were provided.",
        "#1446: empty visual property update-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-value", "NoProperty",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject item options before property names");
    expect_contains(option_before_item_process.stdout_text,
        "Visual property update batch item options require a preceding --property-name.",
        "#1446: option-before-item visual property update-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", form_path.string(),
            "--record", "-1",
            "--property-name", "CAPTION",
            "--property-value", "BadRecord",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1446: visual property update-batch JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1446: invalid-record visual property update-batch JSON should report parser errors");

    const fs::path missing_object_path = temp_root / "update_batch_missing_object.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_object_path);
    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-property-update-batch",
            "--path", missing_object_path.string(),
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "ShouldNotWrite",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1446: visual property update-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualPropertyUpdateBatch\": null",
        "#1446: unresolved visual property update-batch JSON should not expose a batch update object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1446: unresolved visual property update-batch JSON should report editor errors");
    expect(visual_object_property(missing_object_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1446: unresolved visual property update-batch commands should not mutate properties");

    const fs::path alias_path = temp_root / "update_batch_alias.scx";
    write_synthetic_form_table_for_toolbox_creation(alias_path);
    const auto alias_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-property-update-batch",
            "--path", alias_path.string(),
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "AliasCaption",
            "--json"
        },
        temp_root);
    expect(alias_process.exit_code == 0,
        "#1446: visual object property update-batch alias should remain accepted");
    expect(visual_object_property(alias_path, "existing-textbox-guid", "CAPTION") == "AliasCaption",
        "#1446: visual object property update-batch alias should update selected properties");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-property-update-batch --path <asset>",
        "#1446: usage text should expose visual property update-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_properties_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_property_clear_json_tests";
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

    const auto object_name_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "txt1",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(object_name_clear_process.exit_code == 0,
        "#1021: object-name host property clears should exit successfully");
    auto caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: object-name host property clears should empty direct-field properties");

    const auto set_before_unique_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeUniqueClear",
            "--json"
        },
        temp_root);
    expect(set_before_unique_clear.exit_code == 0,
        "#1021: clear-property setup should be able to restore a direct-field value");

    const auto unique_id_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(unique_id_clear_process.exit_code == 0,
        "#1021: unique-id host property clears should exit successfully");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value.empty(),
        "#1021: unique-id host property clears should empty direct-field properties");

    const auto set_before_missing_clear = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "BeforeMissingClear",
            "--json"
        },
        temp_root);
    expect(set_before_missing_clear.exit_code == 0,
        "#1021: missing-clear setup should be able to restore a direct-field value");

    const auto missing_clear_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--clear-property",
            "--object-name", "missingObject",
            "--property-name", "CAPTION",
            "--json"
        },
        temp_root);
    expect(missing_clear_process.exit_code == 4,
        "#1021: missing object-name host property clears should return command failure");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: missing object-name host property clears should not mutate the asset");

    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--set-property",
            "--clear-property",
            "--property-name", "CAPTION",
            "--property-value", "Ambiguous",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1021: ambiguous set/clear property requests should fail during launch parsing");
    caption = caption_value();
    expect(caption.ok && caption.exists && caption.value == "BeforeMissingClear",
        "#1021: ambiguous set/clear property requests should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
