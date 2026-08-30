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
