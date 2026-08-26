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

#include "test_studio_host_json_visual_editor_json_properties_move.inl"
#include "test_studio_host_json_visual_editor_json_properties_rename.inl"
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
