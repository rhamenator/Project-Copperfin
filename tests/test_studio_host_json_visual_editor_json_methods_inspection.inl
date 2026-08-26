void test_studio_host_json_exposes_visual_method_list(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_list_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "methods.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto list_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-list",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(list_process.exit_code == 0,
        "#1422: visual method list JSON should exit successfully for resolved objects");
    expect_contains(list_process.stdout_text, "\"visualMethodList\": {",
        "#1422: visual method list JSON should expose a list object");
    expect_contains(list_process.stdout_text, "\"recordIndex\": 1",
        "#1422: visual method list JSON should expose resolved record indexes");
    expect_contains(list_process.stdout_text, "\"recordDeleted\": false",
        "#1422: visual method list JSON should expose selected-record deleted state");
    expect_contains(list_process.stdout_text, "\"methodCount\": 2",
        "#1422: visual method list JSON should expose parsed method counts");
    expect_contains(list_process.stdout_text, "\"dryRun\": true",
        "#1422: visual method list JSON should remain dry-run");
    expect_contains(list_process.stdout_text, "\"mutatesAsset\": false",
        "#1422: visual method list JSON should remain non-mutating");
    expect_contains(list_process.stdout_text, "\"methodName\": \"Click\"",
        "#1422: visual method list JSON should include procedure names");
    expect_contains(list_process.stdout_text, "\"kind\": \"procedure\"",
        "#1422: visual method list JSON should identify procedures");
    expect_contains(list_process.stdout_text, "\"sourceText\": \"RETURN\"",
        "#1422: visual method list JSON should expose procedure source text");
    expect_contains(list_process.stdout_text, "\"sourceLineIndex\": 0",
        "#1422: visual method list JSON should expose procedure declaration lines");
    expect_contains(list_process.stdout_text, "\"sourceMemoBlockNumber\": ",
        "#1422: visual method list JSON should expose source memo block metadata");
    expect_contains(list_process.stdout_text, "\"methodName\": \"CanSave\"",
        "#1422: visual method list JSON should include function names");
    expect_contains(list_process.stdout_text, "\"kind\": \"function\"",
        "#1422: visual method list JSON should identify functions");
    expect_contains(list_process.stdout_text, "\"sourceText\": \"RETURN .T.\"",
        "#1422: visual method list JSON should expose function source text");
    expect_contains(list_process.stdout_text, "\"sourceLineIndex\": 2",
        "#1422: visual method list JSON should expose later method declaration lines");

    const auto empty_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-list",
            "--path", form_path.string(),
            "--unique-id", "fallback-guid",
            "--json"
        },
        temp_root);
    expect(empty_process.exit_code == 0,
        "#1422: visual method list JSON should succeed for objects without methods");
    expect_contains(empty_process.stdout_text, "\"recordIndex\": 2",
        "#1422: empty visual method list JSON should expose selected object records");
    expect_contains(empty_process.stdout_text, "\"recordDeleted\": true",
        "#1422: empty visual method list JSON should preserve deleted selected-record state");
    expect_contains(empty_process.stdout_text, "\"methodCount\": 0",
        "#1422: empty visual method list JSON should report zero methods");
    expect_contains(empty_process.stdout_text, "\"methods\": [\n    ]",
        "#1422: empty visual method list JSON should expose an empty methods array");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-list",
            "--unique-id", "save-guid",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1422: visual method list JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodList\": null",
        "#1422: missing-path visual method list JSON should not expose a list object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1422: missing-path visual method list JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-list",
            "--path", form_path.string(),
            "--record", "-1",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1422: visual method list JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1422: invalid-record visual method list JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-list",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1422: visual method list JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodList\": null",
        "#1422: unresolved visual method list JSON should not expose a list object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1422: unresolved visual method list JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-list --path <asset>",
        "#1422: usage text should expose visual method list commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_visual_method_query(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_visual_method_query_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "method-query.scx";
    write_synthetic_form_table_for_visual_object_list(form_path);

    const auto query_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "cansave",
            "--json"
        },
        temp_root);
    expect(query_process.exit_code == 0,
        "#1423: visual method query JSON should exit successfully for existing methods");
    expect_contains(query_process.stdout_text, "\"visualMethodQuery\": {",
        "#1423: visual method query JSON should expose a query object");
    expect_contains(query_process.stdout_text, "\"exists\": true",
        "#1423: visual method query JSON should report existing methods");
    expect_contains(query_process.stdout_text, "\"recordIndex\": 1",
        "#1423: visual method query JSON should expose resolved record indexes");
    expect_contains(query_process.stdout_text, "\"recordDeleted\": false",
        "#1423: visual method query JSON should expose selected-record deleted state");
    expect_contains(query_process.stdout_text, "\"dryRun\": true",
        "#1423: visual method query JSON should remain dry-run");
    expect_contains(query_process.stdout_text, "\"mutatesAsset\": false",
        "#1423: visual method query JSON should remain non-mutating");
    expect_contains(query_process.stdout_text, "\"methodName\": \"CanSave\"",
        "#1423: visual method query JSON should expose resolved method names");
    expect_contains(query_process.stdout_text, "\"kind\": \"function\"",
        "#1423: visual method query JSON should expose method declaration kind");
    expect_contains(query_process.stdout_text, "\"sourceText\": \"RETURN .T.\"",
        "#1423: visual method query JSON should expose method source text");
    expect_contains(query_process.stdout_text, "\"sourceLineIndex\": 2",
        "#1423: visual method query JSON should expose method source-line metadata");
    expect_contains(query_process.stdout_text, "\"sourceMemoBlockNumber\": ",
        "#1423: visual method query JSON should expose method memo-block metadata");

    const auto missing_method_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--path", form_path.string(),
            "--unique-id", "save-guid",
            "--method-name", "MissingMethod",
            "--json"
        },
        temp_root);
    expect(missing_method_process.exit_code == 0,
        "#1423: visual method query JSON should succeed for absent methods");
    expect_contains(missing_method_process.stdout_text, "\"exists\": false",
        "#1423: absent visual method query JSON should report missing methods");
    expect_contains(missing_method_process.stdout_text, "\"recordIndex\": 1",
        "#1423: absent visual method query JSON should still expose selected records");
    expect_contains(missing_method_process.stdout_text, "\"method\": null",
        "#1423: absent visual method query JSON should null method snapshots");

    const auto missing_path_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--unique-id", "save-guid",
            "--method-name", "Click",
            "--json"
        },
        temp_root);
    expect(missing_path_process.exit_code == 2,
        "#1423: visual method query JSON should reject missing asset paths");
    expect_contains(missing_path_process.stdout_text, "\"visualMethodQuery\": null",
        "#1423: missing-path visual method query JSON should not expose a query object");
    expect_contains(missing_path_process.stdout_text, "No asset path was provided.",
        "#1423: missing-path visual method query JSON should report parser errors");

    const auto missing_method_name_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--path", form_path.string(),
            "--json"
        },
        temp_root);
    expect(missing_method_name_process.exit_code == 2,
        "#1423: visual method query JSON should reject missing method names");
    expect_contains(missing_method_name_process.stdout_text, "No method name was provided.",
        "#1423: missing method-name visual method query JSON should report parser errors");

    const auto invalid_record_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--path", form_path.string(),
            "--record", "-1",
            "--method-name", "Click",
            "--json"
        },
        temp_root);
    expect(invalid_record_process.exit_code == 2,
        "#1423: visual method query JSON should reject invalid record values");
    expect_contains(invalid_record_process.stdout_text, "The --record value must be a non-negative integer.",
        "#1423: invalid-record visual method query JSON should report parser errors");

    const auto missing_object_process = run_process_capture(
        studio_host_path,
        {
            "--visual-method-query",
            "--path", form_path.string(),
            "--object-name", "missingObject",
            "--method-name", "Click",
            "--json"
        },
        temp_root);
    expect(missing_object_process.exit_code == 4,
        "#1423: visual method query JSON should reject unresolved selected objects");
    expect_contains(missing_object_process.stdout_text, "\"visualMethodQuery\": null",
        "#1423: unresolved visual method query JSON should not expose a query object");
    expect_contains(missing_object_process.stdout_text, "No visual object with the requested name was found.",
        "#1423: unresolved visual method query JSON should report editor errors");

    const auto usage_process = run_process_capture(studio_host_path, {}, temp_root);
    expect_contains(usage_process.stdout_text, "--visual-method-query --path <asset>",
        "#1423: usage text should expose visual method query commands");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
