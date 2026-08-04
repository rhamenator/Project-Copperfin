// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_visual_object_list(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_list_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "outline.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto list_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-list",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(list_process.exit_code == 0,
        "#1418: visual object list JSON should exit successfully for readable assets");
    expect_contains(list_process.stdout_text, "\"visualObjectList\": {",
        "#1418: visual object list JSON should expose a list object");
    expect_contains(list_process.stdout_text, "\"objectCount\": 4",
        "#1418: visual object list JSON should expose object counts");
    expect_contains(list_process.stdout_text, "\"dryRun\": true",
        "#1418: visual object list JSON should remain dry-run");
    expect_contains(list_process.stdout_text, "\"mutatesAsset\": false",
        "#1418: visual object list JSON should remain non-mutating");
    expect_contains(list_process.stdout_text, "\"recordIndex\": 0",
        "#1418: visual object list JSON should expose record indexes");
    expect_contains(list_process.stdout_text, "\"deleted\": false",
        "#1418: visual object list JSON should expose live deletion state");
    expect_contains(list_process.stdout_text, "\"objectName\": \"Page1\"",
        "#1418: visual object list JSON should expose OBJNAME identities");
    expect_contains(list_process.stdout_text, "\"uniqueId\": \"page-guid\"",
        "#1418: visual object list JSON should expose stable unique ids");
    expect_contains(list_process.stdout_text, "\"objectPath\": \"Page1\"",
        "#1418: visual object list JSON should expose root object paths");
    expect_contains(list_process.stdout_text, "\"objectDepth\": 0",
        "#1418: visual object list JSON should expose root object depth");
    expect_contains(list_process.stdout_text, "\"parentRecordIndex\": null",
        "#1418: visual object list JSON should null missing parent record links");
    expect_contains(list_process.stdout_text, "\"ancestorRecordIndexes\": []",
        "#1418: visual object list JSON should expose root ancestor metadata");
    expect_contains(list_process.stdout_text, "\"childCount\": 2",
        "#1418: visual object list JSON should expose child counts");
    expect_contains(list_process.stdout_text, "\"propertyCount\": 10",
        "#1418: visual object list JSON should expose direct plus memo property counts");
    expect_contains(list_process.stdout_text, "\"methodCount\": 1",
        "#1418: visual object list JSON should expose method counts");
    expect_contains(list_process.stdout_text, "\"className\": \"pageframe\"",
        "#1418: visual object list JSON should expose class names");
    expect_contains(list_process.stdout_text, "\"baseclassName\": \"Page\"",
        "#1418: visual object list JSON should expose baseclass names");
    expect_contains(list_process.stdout_text, "\"caption\": \"\\\"Page\\\"\"",
        "#1418: visual object list JSON should expose parsed captions");

    const auto child_begin = list_process.stdout_text.find("\"objectName\": \"cmdSave\"");
    expect(child_begin != std::string::npos,
        "#1418: visual object list JSON should include child controls");
    if (child_begin != std::string::npos) {
        const auto child_json = list_process.stdout_text.substr(child_begin);
        expect_contains(child_json, "\"parentName\": \"Page1\"",
            "#1418: child visual object list JSON should expose parent names");
        expect_contains(child_json, "\"parentRecordIndex\": 0",
            "#1418: child visual object list JSON should expose resolved parent records");
        expect_contains(child_json, "\"ancestorRecordIndexes\": [0]",
            "#1418: child visual object list JSON should expose ancestor records");
        expect_contains(child_json, "\"objectPath\": \"Page1.cmdSave\"",
            "#1418: child visual object list JSON should expose hierarchical paths");
        expect_contains(child_json, "\"objectDepth\": 1",
            "#1418: child visual object list JSON should expose nested depth");
        expect_contains(child_json, "\"siblingIndex\": 0",
            "#1418: child visual object list JSON should expose sibling order");
        expect_contains(child_json, "\"siblingCount\": 2",
            "#1418: child visual object list JSON should expose sibling counts");
        expect_contains(child_json, "\"methodCount\": 2",
            "#1418: child visual object list JSON should expose parsed method counts");
    }

    const auto fallback_begin = list_process.stdout_text.find("\"objectName\": \"fallbackButton\"");
    expect(fallback_begin != std::string::npos,
        "#1418: visual object list JSON should include fallback NAME rows");
    if (fallback_begin != std::string::npos) {
        const auto fallback_entry_begin = list_process.stdout_text.rfind("{", fallback_begin);
        const auto fallback_json = fallback_entry_begin == std::string::npos
            ? list_process.stdout_text.substr(fallback_begin)
            : list_process.stdout_text.substr(fallback_entry_begin);
        expect_contains(fallback_json, "\"deleted\": true",
            "#1418: fallback visual object list JSON should preserve deleted rows");
        expect_contains(fallback_json, "\"uniqueId\": \"fallback-guid\"",
            "#1418: fallback visual object list JSON should preserve unique ids");
        expect_contains(fallback_json, "\"caption\": \"\\\"Fallback\\\"\"",
            "#1418: fallback visual object list JSON should expose captions");
    }

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-list",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1418: visual object list JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectList\": null",
        "#1418: missing-path visual object list JSON should not expose a list object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1418: missing-path visual object list JSON should report parser errors");

    const auto invalid_asset_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-list",
            "--path", (temp_root / "missing.scx").string(),
            "--json"
        },
        temp_root);
    expect(invalid_asset_process.exit_code == 4,
        "#1418: visual object list JSON should reject unreadable assets");
    expect_contains(invalid_asset_process.stdout_text, "\"visualObjectList\": null",
        "#1418: unreadable visual object list JSON should not expose a list object");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-list --path <asset>",
        "#1418: usage text should expose visual object list commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_object_children(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_children_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "children.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto children_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-children",
            "--path", form_path.string(),
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(children_process.exit_code == 0,
        "#1419: visual object children JSON should exit successfully for selected parents");
    expect_contains(children_process.stdout_text, "\"visualObjectChildren\": {",
        "#1419: visual object children JSON should expose a children object");
    expect_contains(children_process.stdout_text, "\"parentRecordIndex\": 0",
        "#1419: visual object children JSON should expose parent record indexes");
    expect_contains(children_process.stdout_text, "\"parentName\": \"Page1\"",
        "#1419: visual object children JSON should expose parent names");
    expect_contains(children_process.stdout_text, "\"childCount\": 2",
        "#1419: visual object children JSON should expose child counts");
    expect_contains(children_process.stdout_text, "\"dryRun\": true",
        "#1419: visual object children JSON should remain dry-run");
    expect_contains(children_process.stdout_text, "\"mutatesAsset\": false",
        "#1419: visual object children JSON should remain non-mutating");
    expect_contains(children_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1419: visual object children JSON should include live immediate children");
    expect_contains(children_process.stdout_text, "\"recordIndex\": 1",
        "#1419: visual object children JSON should expose child record indexes");
    expect_contains(children_process.stdout_text, "\"parentRecordIndex\": 0",
        "#1419: child snapshots should expose resolved parent record links");
    expect_contains(children_process.stdout_text, "\"ancestorRecordIndexes\": [0]",
        "#1419: child snapshots should expose ancestor records");
    expect_contains(children_process.stdout_text, "\"objectPath\": \"Page1.cmdSave\"",
        "#1419: child snapshots should expose hierarchical paths");
    expect_contains(children_process.stdout_text, "\"caption\": \"\\\"Save\\\"\"",
        "#1419: child snapshots should expose parsed captions");

    const auto fallback_begin = children_process.stdout_text.find("\"objectName\": \"fallbackButton\"");
    expect(fallback_begin != std::string::npos,
        "#1419: visual object children JSON should include fallback NAME children");
    if (fallback_begin != std::string::npos) {
        const auto fallback_entry_begin = children_process.stdout_text.rfind("{", fallback_begin);
        const auto fallback_json = fallback_entry_begin == std::string::npos
            ? children_process.stdout_text.substr(fallback_begin)
            : children_process.stdout_text.substr(fallback_entry_begin);
        expect_contains(fallback_json, "\"deleted\": true",
            "#1419: visual object children JSON should preserve deleted child rows");
        expect_contains(fallback_json, "\"uniqueId\": \"fallback-guid\"",
            "#1419: visual object children JSON should expose deleted child unique ids");
    }

    const auto leaf_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-children",
            "--path", form_path.string(),
            "--unique-id", "fallback-guid",
            "--json"
        },
        temp_root);
    expect(leaf_process.exit_code == 0,
        "#1419: visual object children JSON should succeed for childless selected objects");
    expect_contains(leaf_process.stdout_text, "\"parentRecordIndex\": 2",
        "#1419: childless visual object children JSON should expose selected parent record");
    expect_contains(leaf_process.stdout_text, "\"childCount\": 0",
        "#1419: childless visual object children JSON should report zero children");
    expect_contains(leaf_process.stdout_text, "\"children\": [\n    ]",
        "#1419: childless visual object children JSON should expose an empty children array");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-children",
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1419: visual object children JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectChildren\": null",
        "#1419: missing-path visual object children JSON should not expose a children object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1419: missing-path visual object children JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-children",
            "--path", form_path.string(),
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1419: visual object children JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1419: invalid-record visual object children JSON should report parser errors");

    const auto missing_parent_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-children",
            "--path", form_path.string(),
            "--object-name", "missingParent",
            "--json"
        },
        temp_root);
    expect(missing_parent_process.exit_code == 4,
        "#1419: visual object children JSON should reject unresolved parents");
    expect_contains(missing_parent_process.stdout_text, "\"visualObjectChildren\": null",
        "#1419: unresolved visual object children JSON should not expose a children object");
    expect_contains(missing_parent_process.stdout_text, "No visual object with the requested name was found.",
        "#1419: unresolved visual object children JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-children --path <asset>",
        "#1419: usage text should expose visual object children commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_object_descendants(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_descendants_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "descendants.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto descendants_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-descendants",
            "--path", form_path.string(),
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(descendants_process.exit_code == 0,
        "#1420: visual object descendants JSON should exit successfully for selected parents");
    expect_contains(descendants_process.stdout_text, "\"visualObjectDescendants\": {",
        "#1420: visual object descendants JSON should expose a descendants object");
    expect_contains(descendants_process.stdout_text, "\"parentRecordIndex\": 0",
        "#1420: visual object descendants JSON should expose parent record indexes");
    expect_contains(descendants_process.stdout_text, "\"parentName\": \"Page1\"",
        "#1420: visual object descendants JSON should expose parent names");
    expect_contains(descendants_process.stdout_text, "\"descendantCount\": 3",
        "#1420: visual object descendants JSON should expose descendant counts");
    expect_contains(descendants_process.stdout_text, "\"dryRun\": true",
        "#1420: visual object descendants JSON should remain dry-run");
    expect_contains(descendants_process.stdout_text, "\"mutatesAsset\": false",
        "#1420: visual object descendants JSON should remain non-mutating");
    expect_contains(descendants_process.stdout_text, "\"depth\": 1",
        "#1420: visual object descendants JSON should expose immediate child depths");
    expect_contains(descendants_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1420: visual object descendants JSON should include immediate children");
    expect_contains(descendants_process.stdout_text, "\"objectName\": \"fallbackButton\"",
        "#1420: visual object descendants JSON should include deleted fallback children");
    expect_contains(descendants_process.stdout_text, "\"deleted\": true",
        "#1420: visual object descendants JSON should preserve deleted descendant state");
    expect_contains(descendants_process.stdout_text, "\"depth\": 2",
        "#1420: visual object descendants JSON should expose nested descendant depths");
    expect_contains(descendants_process.stdout_text, "\"objectName\": \"lblNested\"",
        "#1420: visual object descendants JSON should include nested descendants");
    expect_contains(descendants_process.stdout_text, "\"ancestorRecordIndexes\": [0, 1]",
        "#1420: nested visual object descendants JSON should expose ancestor chains");
    expect_contains(descendants_process.stdout_text, "\"objectPath\": \"Page1.cmdSave.lblNested\"",
        "#1420: nested visual object descendants JSON should expose hierarchical paths");

    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-descendants",
            "--path", form_path.string(),
            "--unique-id", "fallback-guid",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 0,
        "#1420: visual object descendants JSON should succeed for objects without descendants");
    expect_contains(empty_process.stdout_text, "\"parentRecordIndex\": 2",
        "#1420: empty visual object descendants JSON should expose selected parent record");
    expect_contains(empty_process.stdout_text, "\"descendantCount\": 0",
        "#1420: empty visual object descendants JSON should report zero descendants");
    expect_contains(empty_process.stdout_text, "\"descendants\": [\n    ]",
        "#1420: empty visual object descendants JSON should expose an empty descendants array");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-descendants",
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1420: visual object descendants JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectDescendants\": null",
        "#1420: missing-path visual object descendants JSON should not expose a descendants object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1420: missing-path visual object descendants JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-descendants",
            "--path", form_path.string(),
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1420: visual object descendants JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1420: invalid-record visual object descendants JSON should report parser errors");

    const auto missing_parent_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-descendants",
            "--path", form_path.string(),
            "--object-name", "missingParent",
            "--json"
        },
        temp_root);
    expect(missing_parent_process.exit_code == 4,
        "#1420: visual object descendants JSON should reject unresolved parents");
    expect_contains(missing_parent_process.stdout_text, "\"visualObjectDescendants\": null",
        "#1420: unresolved visual object descendants JSON should not expose a descendants object");
    expect_contains(missing_parent_process.stdout_text, "No visual object with the requested name was found.",
        "#1420: unresolved visual object descendants JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-descendants --path <asset>",
        "#1420: usage text should expose visual object descendants commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_object_ancestors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_ancestors_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "ancestors.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto ancestors_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-ancestors",
            "--path", form_path.string(),
            "--unique-id", "nested-guid",
            "--json"
        },
        temp_root);
    expect(ancestors_process.exit_code == 0,
        "#1421: visual object ancestors JSON should exit successfully for selected objects");
    expect_contains(ancestors_process.stdout_text, "\"visualObjectAncestors\": {",
        "#1421: visual object ancestors JSON should expose an ancestors object");
    expect_contains(ancestors_process.stdout_text, "\"recordIndex\": 3",
        "#1421: visual object ancestors JSON should expose selected object record indexes");
    expect_contains(ancestors_process.stdout_text, "\"ancestorCount\": 2",
        "#1421: visual object ancestors JSON should expose ancestor counts");
    expect_contains(ancestors_process.stdout_text, "\"dryRun\": true",
        "#1421: visual object ancestors JSON should remain dry-run");
    expect_contains(ancestors_process.stdout_text, "\"mutatesAsset\": false",
        "#1421: visual object ancestors JSON should remain non-mutating");
    expect_contains(ancestors_process.stdout_text, "\"depth\": 1",
        "#1421: visual object ancestors JSON should expose immediate parent depths");
    expect_contains(ancestors_process.stdout_text, "\"recordIndex\": 1",
        "#1421: visual object ancestors JSON should include immediate parent records");
    expect_contains(ancestors_process.stdout_text, "\"objectName\": \"cmdSave\"",
        "#1421: visual object ancestors JSON should include immediate parent objects");
    expect_contains(ancestors_process.stdout_text, "\"objectPath\": \"Page1.cmdSave\"",
        "#1421: immediate parent ancestor JSON should expose hierarchical paths");
    expect_contains(ancestors_process.stdout_text, "\"depth\": 2",
        "#1421: visual object ancestors JSON should expose root ancestor depths");
    expect_contains(ancestors_process.stdout_text, "\"recordIndex\": 0",
        "#1421: visual object ancestors JSON should include root ancestor records");
    expect_contains(ancestors_process.stdout_text, "\"objectName\": \"Page1\"",
        "#1421: visual object ancestors JSON should include root ancestor objects");
    expect_contains(ancestors_process.stdout_text, "\"objectPath\": \"Page1\"",
        "#1421: root ancestor JSON should expose root object paths");

    const auto root_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-ancestors",
            "--path", form_path.string(),
            "--unique-id", "page-guid",
            "--json"
        },
        temp_root);
    expect(root_process.exit_code == 0,
        "#1421: visual object ancestors JSON should succeed for root objects");
    expect_contains(root_process.stdout_text, "\"recordIndex\": 0",
        "#1421: root visual object ancestors JSON should expose selected object record");
    expect_contains(root_process.stdout_text, "\"ancestorCount\": 0",
        "#1421: root visual object ancestors JSON should report zero ancestors");
    expect_contains(root_process.stdout_text, "\"ancestors\": [\n    ]",
        "#1421: root visual object ancestors JSON should expose an empty ancestors array");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-ancestors",
            "--unique-id", "nested-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1421: visual object ancestors JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectAncestors\": null",
        "#1421: missing-path visual object ancestors JSON should not expose an ancestors object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1421: missing-path visual object ancestors JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-ancestors",
            "--path", form_path.string(),
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1421: visual object ancestors JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1421: invalid-record visual object ancestors JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-ancestors",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1421: visual object ancestors JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualObjectAncestors\": null",
        "#1421: unresolved visual object ancestors JSON should not expose an ancestors object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1421: unresolved visual object ancestors JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-ancestors --path <asset>",
        "#1421: usage text should expose visual object ancestors commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_visual_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_update_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "object_update_batch.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);

    const auto update_batch_process = run_process_capture(
        studio_host_path,
        {
            "--from-vs",
            "--visual-object-update-batch",
            "--path", form_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "TextBatch",
            "--property-name", "ToolTipText",
            "--property-value", "Hover text",
            "--selected-object-name", "frmCustomer",
            "--property-name", "CAPTION",
            "--property-value", "FormBatch",
            "--json"
        },
        temp_root);
    expect(update_batch_process.exit_code == 0,
        "#1447: visual object update-batch JSON should exit successfully for valid batches");
    expect_contains(update_batch_process.stdout_text, "\"visualObjectUpdateBatch\": {",
        "#1447: visual object update-batch JSON should expose a batch update object");
    expect_contains(update_batch_process.stdout_text, "\"launchedFromVisualStudio\": true",
        "#3985: VS-originated visual object update batches should preserve host provenance in JSON");
    expect_contains(update_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1447: visual object update-batch JSON should expose affected object counts");
    expect_contains(update_batch_process.stdout_text, "\"dryRun\": false",
        "#1447: visual object update-batch JSON should expose committed execution state");
    expect_contains(update_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1447: visual object update-batch JSON should expose mutation state");
    expect_contains(update_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1447: visual object update-batch JSON should expose undo availability");
    expect(visual_object_property(form_path, "existing-textbox-guid", "CAPTION") == "TextBatch" &&
            visual_object_property(form_path, "existing-textbox-guid", "ToolTipText") == "Hover text" &&
            visual_object_property(form_path, "form-guid", "CAPTION") == "FormBatch",
        "#1447: visual object update-batch host command should update multiple selected objects");

    const fs::path later_object_path = temp_root / "object_update_batch_later_object.scx";
    write_synthetic_form_table_for_toolbox_creation(later_object_path);
    const auto later_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", later_object_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "ShouldRollback",
            "--selected-object-name", "missingObject",
            "--property-name", "CAPTION",
            "--property-value", "Noop",
            "--json"
        },
        temp_root);
    expect(later_object_process.exit_code == 4,
        "#1447: visual object update-batch JSON should reject unresolved later objects");
    expect_contains(later_object_process.stdout_text, "\"visualObjectUpdateBatch\": null",
        "#1447: failed visual object update-batch JSON should not expose a batch update object");
    expect_contains(later_object_process.stdout_text, "\"launchedFromVisualStudio\": false",
        "#3985: standalone visual object update batches should retain independent provenance");
    expect_contains(later_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1447: unresolved later object visual object update-batch JSON should report editor errors");
    expect(visual_object_property(later_object_path, "existing-textbox-guid", "CAPTION") == "Existing" &&
            visual_object_property(later_object_path, "form-guid", "CAPTION") == "Customer",
        "#1447: failed visual object update-batch commands should roll back earlier object changes");

    const fs::path later_property_path = temp_root / "object_update_batch_later_property.scx";
    write_synthetic_form_table_for_toolbox_creation(later_property_path);
    const auto later_property_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", later_property_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "ShouldRollback",
            "--selected-unique-id", "form-guid",
            "--property-name", "CAPTION",
            "--property-value", "FormRollback",
            "--property-name", "",
            "--property-value", "Noop",
            "--json"
        },
        temp_root);
    expect(later_property_process.exit_code == 4,
        "#1447: visual object update-batch JSON should reject missing later property names");
    expect_contains(later_property_process.stdout_text, "No property name was provided.",
        "#1447: missing later property visual object update-batch JSON should report editor errors");
    expect(visual_object_property(later_property_path, "existing-textbox-guid", "CAPTION") == "Existing" &&
            visual_object_property(later_property_path, "form-guid", "CAPTION") == "Customer",
        "#1447: failed visual object update-batch commands should roll back later property changes");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--selected-unique-id", "existing-textbox-guid",
            "--property-name", "CAPTION",
            "--property-value", "MissingPath",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1447: visual object update-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectUpdateBatch\": null",
        "#1447: missing-path visual object update-batch JSON should not expose a batch update object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1447: missing-path visual object update-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1447: visual object update-batch JSON should reject empty object batches");
    expect_contains(no_items_process.stdout_text, "No visual object edits were provided.",
        "#1447: empty visual object update-batch JSON should report parser errors");

    const auto property_before_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", form_path.string(),
            "--property-name", "CAPTION",
            "--property-value", "NoObject",
            "--json"
        },
        temp_root);
    expect(property_before_object_process.exit_code == 2,
        "#1447: visual object update-batch JSON should reject properties before selected objects");
    expect_contains(property_before_object_process.stdout_text,
        "Visual object update batch property options require a preceding selected-object selector.",
        "#1447: property-before-object visual object update-batch JSON should report parser errors");

    const auto value_before_property_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", form_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--property-value", "NoProperty",
            "--json"
        },
        temp_root);
    expect(value_before_property_process.exit_code == 2,
        "#1447: visual object update-batch JSON should reject property values before property names");
    expect_contains(value_before_property_process.stdout_text,
        "Visual object update batch property values require a preceding --property-name.",
        "#1447: value-before-property visual object update-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", form_path.string(),
            "--selected-record", "-1",
            "--property-name", "CAPTION",
            "--property-value", "BadRecord",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1447: visual object update-batch JSON should reject invalid selected-record values");
    expect_contains(invalid_record_process.stdout_text, "The --selected-record value must be a non-negative integer.",
        "#1447: invalid-record visual object update-batch JSON should report parser errors");

    const fs::path empty_object_path = temp_root / "object_update_batch_empty_object.scx";
    write_synthetic_form_table_for_toolbox_creation(empty_object_path);
    const auto empty_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-update-batch",
            "--path", empty_object_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(empty_object_process.exit_code == 4,
        "#1447: visual object update-batch JSON should reject per-object empty property lists");
    expect_contains(empty_object_process.stdout_text, "No property changes were provided.",
        "#1447: empty-object visual object update-batch JSON should report editor errors");
    expect(visual_object_property(empty_object_path, "existing-textbox-guid", "CAPTION") == "Existing",
        "#1447: empty-object visual object update-batch commands should not mutate properties");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-update-batch --path <asset>",
        "#1447: usage text should expose visual object update-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_duplicates_visual_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_duplicate_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "duplicate_batch.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const auto duplicate_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", form_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "txtBatchCopy",
            "--new-name", "txtBatchCopy",
            "--new-unique-id", "batch-copy-guid",
            "--selected-object-name", "frmCustomer",
            "--new-object-name", "frmBatchCopy",
            "--new-name", "frmBatchCopy",
            "--new-unique-id", "form-batch-copy-guid",
            "--json"
        },
        temp_root);
    expect(duplicate_batch_process.exit_code == 0,
        "#1448: visual object duplicate-batch JSON should exit successfully for valid batches");
    expect_contains(duplicate_batch_process.stdout_text, "\"visualObjectDuplicateBatch\": {",
        "#1448: visual object duplicate-batch JSON should expose a batch duplicate object");
    expect_contains(duplicate_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1448: visual object duplicate-batch JSON should expose affected object counts");
    expect_contains(duplicate_batch_process.stdout_text, "\"dryRun\": false",
        "#1448: visual object duplicate-batch JSON should expose committed execution state");
    expect_contains(duplicate_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1448: visual object duplicate-batch JSON should expose mutation state");
    expect_contains(duplicate_batch_process.stdout_text, "\"undoAvailable\": false",
        "#1448: visual object duplicate-batch JSON should expose undo availability state");
    expect(visual_object_count(form_path) == 4U &&
            visual_object_exists(form_path, "batch-copy-guid") &&
            visual_object_exists(form_path, "form-batch-copy-guid"),
        "#1448: visual object duplicate-batch host command should append all requested duplicates");

    const fs::path rollback_path = temp_root / "duplicate_batch_rollback.scx";
    write_synthetic_form_table_for_toolbox_creation(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", rollback_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "txtRollbackCopy",
            "--new-name", "txtRollbackCopy",
            "--new-unique-id", "rollback-copy-guid",
            "--selected-object-name", "frmCustomer",
            "--new-object-name", "frmCollisionCopy",
            "--new-name", "frmCollisionCopy",
            "--new-unique-id", "rollback-copy-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1448: visual object duplicate-batch JSON should reject later identity collisions");
    expect_contains(rollback_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
        "#1448: failed visual object duplicate-batch JSON should not expose a batch duplicate object");
    expect_contains(rollback_process.stdout_text, "The requested replacement identity already exists in the asset.",
        "#1448: collision visual object duplicate-batch JSON should report editor errors");
    expect(visual_object_count(rollback_path) == 2U &&
            !visual_object_exists(rollback_path, "rollback-copy-guid"),
        "#1448: failed visual object duplicate-batch commands should roll back earlier duplicates");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "missingPathCopy",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1448: visual object duplicate-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
        "#1448: missing-path visual object duplicate-batch JSON should not expose a batch duplicate object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1448: missing-path visual object duplicate-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1448: visual object duplicate-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No visual object duplicates were provided.",
        "#1448: empty visual object duplicate-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", form_path.string(),
            "--new-object-name", "NoSelectedObject",
            "--selected-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1448: visual object duplicate-batch JSON should reject item options before selected objects");
    expect_contains(option_before_item_process.stdout_text,
        "Visual object duplicate batch item options require a preceding selected-object selector.",
        "#1448: option-before-item visual object duplicate-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", form_path.string(),
            "--selected-record", "-1",
            "--new-object-name", "BadRecordCopy",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1448: visual object duplicate-batch JSON should reject invalid selected-record values");
    expect_contains(invalid_record_process.stdout_text, "The --selected-record value must be a non-negative integer.",
        "#1448: invalid-record visual object duplicate-batch JSON should report parser errors");

    const fs::path missing_object_path = temp_root / "duplicate_batch_missing_object.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_object_path);
    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-duplicate-batch",
            "--path", missing_object_path.string(),
            "--selected-object-name", "missingObject",
            "--new-object-name", "missingObjectCopy",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1448: visual object duplicate-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualObjectDuplicateBatch\": null",
        "#1448: unresolved visual object duplicate-batch JSON should not expose a batch duplicate object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1448: unresolved visual object duplicate-batch JSON should report editor errors");
    expect(visual_object_count(missing_object_path) == 2U,
        "#1448: unresolved visual object duplicate-batch commands should not mutate the asset");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-duplicate-batch --path <asset>",
        "#1448: usage text should expose visual object duplicate-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_renames_visual_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_rename_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "rename_batch.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const auto rename_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", form_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "txtBatch",
            "--new-name", "txtBatch",
            "--new-unique-id", "batch-rename-guid",
            "--selected-object-name", "frmCustomer",
            "--new-object-name", "frmBatch",
            "--new-name", "frmBatch",
            "--new-unique-id", "form-batch-rename-guid",
            "--json"
        },
        temp_root);
    expect(rename_batch_process.exit_code == 0,
        "#1449: visual object rename-batch JSON should exit successfully for valid batches");
    expect_contains(rename_batch_process.stdout_text, "\"visualObjectRenameBatch\": {",
        "#1449: visual object rename-batch JSON should expose a batch rename object");
    expect_contains(rename_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1449: visual object rename-batch JSON should expose affected object counts");
    expect_contains(rename_batch_process.stdout_text, "\"dryRun\": false",
        "#1449: visual object rename-batch JSON should expose committed execution state");
    expect_contains(rename_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1449: visual object rename-batch JSON should expose mutation state");
    expect_contains(rename_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1449: visual object rename-batch JSON should expose undo availability state");
    expect(visual_object_count(form_path) == 2U &&
            visual_object_exists(form_path, "batch-rename-guid") &&
            visual_object_exists(form_path, "form-batch-rename-guid") &&
            !visual_object_exists(form_path, "existing-textbox-guid") &&
            !visual_object_exists(form_path, "form-guid"),
        "#1449: visual object rename-batch host command should update all requested identities");

    const fs::path rollback_path = temp_root / "rename_batch_rollback.scx";
    write_synthetic_form_table_for_toolbox_creation(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", rollback_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "txtRollback",
            "--new-name", "txtRollback",
            "--new-unique-id", "rollback-textbox-guid",
            "--selected-object-name", "frmCustomer",
            "--new-object-name", "frmCollision",
            "--new-name", "frmCollision",
            "--new-unique-id", "rollback-textbox-guid",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1449: visual object rename-batch JSON should reject later identity collisions");
    expect_contains(rollback_process.stdout_text, "\"visualObjectRenameBatch\": null",
        "#1449: failed visual object rename-batch JSON should not expose a batch rename object");
    expect_contains(rollback_process.stdout_text, "The requested identity value already exists in the asset.",
        "#1449: collision visual object rename-batch JSON should report editor errors");
    expect(visual_object_count(rollback_path) == 2U &&
            visual_object_exists(rollback_path, "existing-textbox-guid") &&
            visual_object_exists(rollback_path, "form-guid") &&
            !visual_object_exists(rollback_path, "rollback-textbox-guid"),
        "#1449: failed visual object rename-batch commands should roll back earlier renames");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--selected-unique-id", "existing-textbox-guid",
            "--new-object-name", "missingPathRename",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1449: visual object rename-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectRenameBatch\": null",
        "#1449: missing-path visual object rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1449: missing-path visual object rename-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1449: visual object rename-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No visual object renames were provided.",
        "#1449: empty visual object rename-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", form_path.string(),
            "--new-object-name", "NoSelectedObject",
            "--selected-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1449: visual object rename-batch JSON should reject item options before selected objects");
    expect_contains(option_before_item_process.stdout_text,
        "Visual object rename batch item options require a preceding selected-object selector.",
        "#1449: option-before-item visual object rename-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", form_path.string(),
            "--selected-record", "-1",
            "--new-object-name", "BadRecordRename",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1449: visual object rename-batch JSON should reject invalid selected-record values");
    expect_contains(invalid_record_process.stdout_text, "The --selected-record value must be a non-negative integer.",
        "#1449: invalid-record visual object rename-batch JSON should report parser errors");

    const fs::path missing_object_path = temp_root / "rename_batch_missing_object.scx";
    write_synthetic_form_table_for_toolbox_creation(missing_object_path);
    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", missing_object_path.string(),
            "--selected-object-name", "missingObject",
            "--new-object-name", "missingObjectRename",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1449: visual object rename-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualObjectRenameBatch\": null",
        "#1449: unresolved visual object rename-batch JSON should not expose a batch rename object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1449: unresolved visual object rename-batch JSON should report editor errors");
    expect(visual_object_count(missing_object_path) == 2U &&
            visual_object_exists(missing_object_path, "existing-textbox-guid"),
        "#1449: unresolved visual object rename-batch commands should not mutate the asset");

    const fs::path empty_identity_path = temp_root / "rename_batch_empty_identity.scx";
    write_synthetic_form_table_for_toolbox_creation(empty_identity_path);
    const auto empty_identity_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-rename-batch",
            "--path", empty_identity_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(empty_identity_process.exit_code == 4,
        "#1449: visual object rename-batch JSON should reject empty replacement identities");
    expect_contains(empty_identity_process.stdout_text, "\"visualObjectRenameBatch\": null",
        "#1449: empty-identity visual object rename-batch JSON should not expose a batch rename object");
    expect_contains(empty_identity_process.stdout_text, "No identity fields were provided.",
        "#1449: empty-identity visual object rename-batch JSON should report editor errors");
    expect(visual_object_count(empty_identity_path) == 2U &&
            visual_object_exists(empty_identity_path, "existing-textbox-guid"),
        "#1449: empty-identity visual object rename-batch commands should not mutate the asset");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-rename-batch --path <asset>",
        "#1449: usage text should expose visual object rename-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reparents_visual_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_reparent_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "reparent_batch.scx";
    write_synthetic_form_table_for_object_reparent(form_path);
    const auto reparent_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", form_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--parent-unique-id", "panel-guid",
            "--selected-object-name", "cntPanel",
            "--clear-parent",
            "--json"
        },
        temp_root);
    expect(reparent_batch_process.exit_code == 0,
        "#1445: visual object reparent-batch JSON should exit successfully for valid batches");
    expect_contains(reparent_batch_process.stdout_text, "\"visualObjectReparentBatch\": {",
        "#1445: visual object reparent-batch JSON should expose a batch reparent object");
    expect_contains(reparent_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1445: visual object reparent-batch JSON should expose affected item counts");
    expect_contains(reparent_batch_process.stdout_text, "\"dryRun\": false",
        "#1445: visual object reparent-batch JSON should expose committed execution state");
    expect_contains(reparent_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1445: visual object reparent-batch JSON should expose mutation state");
    expect_contains(reparent_batch_process.stdout_text, "\"undoAvailable\": true",
        "#1445: visual object reparent-batch JSON should expose undo availability");
    expect(visual_object_parent(form_path, "existing-textbox-guid") == "cntPanel" &&
            visual_object_parent(form_path, "panel-guid").empty(),
        "#1445: visual object reparent-batch host command should apply per-item parent selectors");

    const fs::path rollback_path = temp_root / "reparent_batch_rollback.scx";
    write_synthetic_form_table_for_object_reparent(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", rollback_path.string(),
            "--selected-unique-id", "existing-textbox-guid",
            "--parent-unique-id", "panel-guid",
            "--selected-object-name", "cntPanel",
            "--parent-name", "missingParent",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1445: visual object reparent-batch JSON should reject unresolved parent selectors");
    expect_contains(rollback_process.stdout_text, "\"visualObjectReparentBatch\": null",
        "#1445: failed visual object reparent-batch JSON should not expose a batch reparent object");
    expect_contains(rollback_process.stdout_text, "No visual object with the requested name was found.",
        "#1445: missing-parent visual object reparent-batch JSON should report editor errors");
    expect(visual_object_parent(rollback_path, "existing-textbox-guid") == "frmCustomer" &&
            visual_object_parent(rollback_path, "panel-guid") == "frmCustomer",
        "#1445: failed visual object reparent-batch commands should roll back earlier reparent operations");

    const fs::path cycle_path = temp_root / "reparent_batch_cycle.scx";
    write_synthetic_form_table_for_object_reparent(cycle_path);
    const auto cycle_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", cycle_path.string(),
            "--selected-unique-id", "form-guid",
            "--parent-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(cycle_process.exit_code == 4,
        "#1445: visual object reparent-batch JSON should reject descendant-cycle parent selections");
    expect_contains(cycle_process.stdout_text, "A visual object cannot be reparented to one of its descendants.",
        "#1445: descendant-cycle visual object reparent-batch JSON should report editor errors");
    expect(visual_object_parent(cycle_path, "form-guid").empty() &&
            visual_object_parent(cycle_path, "existing-textbox-guid") == "frmCustomer",
        "#1445: descendant-cycle visual object reparent-batch commands should not mutate parent fields");

    const fs::path self_path = temp_root / "reparent_batch_self.scx";
    write_synthetic_form_table_for_object_reparent(self_path);
    const auto self_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", self_path.string(),
            "--selected-unique-id", "panel-guid",
            "--parent-unique-id", "panel-guid",
            "--json"
        },
        temp_root);
    expect(self_process.exit_code == 4,
        "#1445: visual object reparent-batch JSON should reject self-parenting");
    expect_contains(self_process.stdout_text, "A visual object cannot be reparented to itself.",
        "#1445: self-parent visual object reparent-batch JSON should report editor errors");
    expect(visual_object_parent(self_path, "panel-guid") == "frmCustomer",
        "#1445: self-parent visual object reparent-batch commands should not mutate parent fields");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--selected-unique-id", "existing-textbox-guid",
            "--parent-unique-id", "panel-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1445: visual object reparent-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectReparentBatch\": null",
        "#1445: missing-path visual object reparent-batch JSON should not expose a batch reparent object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1445: missing-path visual object reparent-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1445: visual object reparent-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No visual object reparent operations were provided.",
        "#1445: empty visual object reparent-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", form_path.string(),
            "--parent-name", "cntPanel",
            "--selected-unique-id", "existing-textbox-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1445: visual object reparent-batch JSON should reject item options before selected objects");
    expect_contains(option_before_item_process.stdout_text,
        "Visual object reparent batch item options require a preceding selected-object selector.",
        "#1445: option-before-item visual object reparent-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", form_path.string(),
            "--selected-record", "-1",
            "--clear-parent",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1445: visual object reparent-batch JSON should reject invalid selected-record values");
    expect_contains(invalid_record_process.stdout_text, "The --selected-record value must be a non-negative integer.",
        "#1445: invalid-record visual object reparent-batch JSON should report parser errors");

    const fs::path missing_object_path = temp_root / "reparent_batch_missing_object.scx";
    write_synthetic_form_table_for_object_reparent(missing_object_path);
    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reparent-batch",
            "--path", missing_object_path.string(),
            "--selected-object-name", "missingObject",
            "--parent-name", "cntPanel",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1445: visual object reparent-batch JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualObjectReparentBatch\": null",
        "#1445: unresolved visual object reparent-batch JSON should not expose a batch reparent object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1445: unresolved visual object reparent-batch JSON should report editor errors");
    expect(visual_object_parent(missing_object_path, "existing-textbox-guid") == "frmCustomer",
        "#1445: unresolved visual object reparent-batch commands should not mutate parent fields");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-reparent-batch --path <asset>",
        "#1445: usage text should expose visual object reparent-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_reorders_visual_object_batches(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_object_reorder_batch_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path reorder_path = temp_root / "reorder_batch.scx";
    write_synthetic_form_table_for_object_reorder(reorder_path);
    const auto reorder_batch_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", reorder_path.string(),
            "--selected-unique-id", "c-guid",
            "--placement", "front",
            "--selected-object-name", "cmdA",
            "--placement", "after",
            "--target-unique-id", "d-guid",
            "--json"
        },
        temp_root);
    expect(reorder_batch_process.exit_code == 0,
        "#1450: visual object reorder-batch JSON should exit successfully for valid batches");
    expect_contains(reorder_batch_process.stdout_text, "\"visualObjectReorderBatch\": {",
        "#1450: visual object reorder-batch JSON should expose a batch reorder object");
    expect_contains(reorder_batch_process.stdout_text, "\"affectedObjectCount\": 2",
        "#1450: visual object reorder-batch JSON should expose affected object counts");
    expect_contains(reorder_batch_process.stdout_text, "\"dryRun\": false",
        "#1450: visual object reorder-batch JSON should expose committed execution state");
    expect_contains(reorder_batch_process.stdout_text, "\"mutatesAsset\": true",
        "#1450: visual object reorder-batch JSON should expose mutation state");
    expect_contains(reorder_batch_process.stdout_text, "\"undoAvailable\": false",
        "#1450: visual object reorder-batch JSON should expose undo availability state");
    expect(visual_object_order(reorder_path) == "c-guid,b-guid,d-guid,a-guid",
        "#1450: visual object reorder-batch host command should apply all requested reorders");

    const fs::path rollback_path = temp_root / "reorder_batch_rollback.scx";
    write_synthetic_form_table_for_object_reorder(rollback_path);
    const auto rollback_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", rollback_path.string(),
            "--selected-unique-id", "c-guid",
            "--placement", "front",
            "--selected-object-name", "cmdB",
            "--placement", "before",
            "--target-object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(rollback_process.exit_code == 4,
        "#1450: visual object reorder-batch JSON should reject later target failures");
    expect_contains(rollback_process.stdout_text, "\"visualObjectReorderBatch\": null",
        "#1450: failed visual object reorder-batch JSON should not expose a batch reorder object");
    expect_contains(rollback_process.stdout_text, "No visual object with the requested name was found.",
        "#1450: missing-target visual object reorder-batch JSON should report editor errors");
    expect(visual_object_order(rollback_path) == "a-guid,b-guid,c-guid,d-guid",
        "#1450: failed visual object reorder-batch commands should not write earlier in-memory reorders");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--selected-unique-id", "c-guid",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1450: visual object reorder-batch JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualObjectReorderBatch\": null",
        "#1450: missing-path visual object reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1450: missing-path visual object reorder-batch JSON should report parser errors");

    const auto no_items_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", reorder_path.string(),
            "--json"
        },
        temp_root);
    expect(no_items_process.exit_code == 2,
        "#1450: visual object reorder-batch JSON should reject empty batches");
    expect_contains(no_items_process.stdout_text, "No visual object reorders were provided.",
        "#1450: empty visual object reorder-batch JSON should report parser errors");

    const auto option_before_item_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", reorder_path.string(),
            "--placement", "front",
            "--selected-unique-id", "c-guid",
            "--json"
        },
        temp_root);
    expect(option_before_item_process.exit_code == 2,
        "#1450: visual object reorder-batch JSON should reject item options before selected objects");
    expect_contains(option_before_item_process.stdout_text,
        "Visual object reorder batch item options require a preceding selected-object selector.",
        "#1450: option-before-item visual object reorder-batch JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", reorder_path.string(),
            "--selected-record", "-1",
            "--placement", "front",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1450: visual object reorder-batch JSON should reject invalid selected-record values");
    expect_contains(invalid_record_process.stdout_text, "The --selected-record value must be a non-negative integer.",
        "#1450: invalid-record visual object reorder-batch JSON should report parser errors");

    const auto missing_placement_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", reorder_path.string(),
            "--selected-unique-id", "c-guid",
            "--json"
        },
        temp_root);
    expect(missing_placement_process.exit_code == 2,
        "#1450: visual object reorder-batch JSON should reject items without placements");
    expect_contains(missing_placement_process.stdout_text, "No visual object placement was provided.",
        "#1450: missing-placement visual object reorder-batch JSON should report parser errors");

    const fs::path missing_target_path = temp_root / "reorder_batch_missing_target.scx";
    write_synthetic_form_table_for_object_reorder(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", missing_target_path.string(),
            "--selected-unique-id", "c-guid",
            "--placement", "before",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1450: visual object reorder-batch JSON should reject relative placements without targets");
    expect_contains(missing_target_process.stdout_text, "\"visualObjectReorderBatch\": null",
        "#1450: missing-target-selector visual object reorder-batch JSON should not expose a batch reorder object");
    expect_contains(missing_target_process.stdout_text, "No target object selector was provided.",
        "#1450: missing-target-selector visual object reorder-batch JSON should report editor errors");
    expect(visual_object_order(missing_target_path) == "a-guid,b-guid,c-guid,d-guid",
        "#1450: missing-target-selector visual object reorder-batch commands should not mutate order");

    const fs::path self_relative_path = temp_root / "reorder_batch_self_relative.scx";
    write_synthetic_form_table_for_object_reorder(self_relative_path);
    const auto self_relative_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", self_relative_path.string(),
            "--selected-unique-id", "c-guid",
            "--placement", "before",
            "--target-unique-id", "c-guid",
            "--json"
        },
        temp_root);
    expect(self_relative_process.exit_code == 4,
        "#1450: visual object reorder-batch JSON should reject self-relative placements");
    expect_contains(self_relative_process.stdout_text, "A visual object cannot be reordered relative to itself.",
        "#1450: self-relative visual object reorder-batch JSON should report editor errors");
    expect(visual_object_order(self_relative_path) == "a-guid,b-guid,c-guid,d-guid",
        "#1450: self-relative visual object reorder-batch commands should not mutate order");

    const fs::path unsupported_path = temp_root / "reorder_batch_unsupported.scx";
    write_synthetic_form_table_for_object_reorder(unsupported_path);
    const auto unsupported_process = run_process_capture(
        studio_host_path,
        {
            "--visual-object-reorder-batch",
            "--path", unsupported_path.string(),
            "--selected-unique-id", "c-guid",
            "--placement", "sideways",
            "--json"
        },
        temp_root);
    expect(unsupported_process.exit_code == 4,
        "#1450: visual object reorder-batch JSON should reject unsupported placements");
    expect_contains(unsupported_process.stdout_text, "Unsupported visual object placement.",
        "#1450: unsupported-placement visual object reorder-batch JSON should report editor errors");
    expect(visual_object_order(unsupported_path) == "a-guid,b-guid,c-guid,d-guid",
        "#1450: unsupported-placement visual object reorder-batch commands should not mutate order");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-object-reorder-batch --path <asset>",
        "#1450: usage text should expose visual object reorder-batch commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
