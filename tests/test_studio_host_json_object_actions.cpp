// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

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

}  // namespace cf_test_studio_host_json
