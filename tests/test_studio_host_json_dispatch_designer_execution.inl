void test_studio_host_json_exposes_designer_execution(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_designer_execution_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto visual_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--path", "forms/customer.scx",
            "--record", "1",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--symbol", "Click",
            "--line", "12",
            "--column", "4",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(visual_process.exit_code == 0,
        "#1325: designer execution JSON should accept admitted visual-object contexts");
    expect_contains(visual_process.stdout_text, "\"designerExecution\": {",
        "#1325: designer execution JSON should expose an execution object");
    expect_contains(visual_process.stdout_text, "\"selectionContext\": \"visual_object\"",
        "#1325: designer execution JSON should expose selected Studio contexts");
    expect_contains(visual_process.stdout_text, "\"assetPath\": \"forms/customer.scx\"",
        "#1325: designer execution JSON should preserve asset paths");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1325: designer execution JSON should expose execution admission");
    expect_contains(visual_process.stdout_text, "\"executed\": true",
        "#1325: designer execution JSON should mark aggregate execution complete");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1325: admitted designer execution JSON should not be dry-run");
    expect_contains(visual_process.stdout_text, "\"executionCount\": ",
        "#1325: designer execution JSON should expose execution counts");
    expect_contains(visual_process.stdout_text, "\"errorCount\": 0",
        "#1325: admitted designer execution JSON should expose zero execution errors");
    expect_contains(visual_process.stdout_text, "\"executionReadySelectionContexts\": [\"visual_object\"]",
        "#1402: admitted designer execution JSON should summarize execution-ready selected contexts");
    expect_contains(visual_process.stdout_text, "\"executionBlockedSelectionContexts\": []",
        "#1402: admitted designer execution JSON should expose empty blocked selected contexts");
    expect_contains(visual_process.stdout_text, "\"executionBlockedErrors\": []",
        "#1402: admitted designer execution JSON should expose empty blocked execution errors");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutionCount\": 5",
        "#1342: admitted designer execution JSON should expose editor action execution counts");
    expect_contains(visual_process.stdout_text, "\"builderExecutionCount\": 3",
        "#1342: admitted designer execution JSON should expose builder execution counts");
    expect_contains(visual_process.stdout_text, "\"toolboxExecutionCount\": 1",
        "#1342: admitted designer execution JSON should expose toolbox execution counts");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: admitted designer execution JSON should expose editor action executed counts");
    expect_contains(visual_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: admitted designer execution JSON should expose builder executed counts");
    expect_contains(visual_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: admitted designer execution JSON should expose toolbox executed counts");
    expect_contains(visual_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero editor action errors");
    expect_contains(visual_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero builder errors");
    expect_contains(visual_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: admitted designer execution JSON should expose zero toolbox errors");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: admitted designer execution JSON should expose no failed editor actions");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: admitted designer execution JSON should expose no failed editor action command tokens");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: admitted designer execution JSON should expose no failed editor action executed commands");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: admitted designer execution JSON should expose no failed editor action exit codes");
    expect_contains(visual_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: admitted designer execution JSON should expose no failed editor action errors");
    expect_contains(visual_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: admitted designer execution JSON should expose no failed builders");
    expect_contains(visual_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: admitted designer execution JSON should expose no failed builder command tokens");
    expect_contains(visual_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: admitted designer execution JSON should expose no failed builder executed commands");
    expect_contains(visual_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: admitted designer execution JSON should expose no failed builder exit codes");
    expect_contains(visual_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: admitted designer execution JSON should expose no failed builder errors");
    expect_contains(visual_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: admitted designer execution JSON should expose successful toolbox summary state");
    expect_contains(visual_process.stdout_text, "\"toolboxExitCode\": 0",
        "#1355: admitted designer execution JSON should expose successful toolbox exit codes");
    expect_contains(visual_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: admitted designer execution JSON should expose empty toolbox error summaries");
    expect_contains(visual_process.stdout_text,
        "\"editorActionLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose editor launch commands");
    expect_contains(visual_process.stdout_text,
        "\"builderLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose builder launch commands");
    expect_contains(visual_process.stdout_text,
        "\"toolboxLaunchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1325: designer execution JSON should expose toolbox launch commands");
    expect_contains(visual_process.stdout_text, "\"editorActionExecutions\": [",
        "#1325: designer execution JSON should expose editor execution results");
    expect_contains(visual_process.stdout_text, "\"actionId\": \"edit-visual-method\"",
        "#1337: designer execution JSON should expose editor child action identities");
    const auto editor_child_begin = visual_process.stdout_text.find("\"actionId\": \"edit-visual-method\"");
    expect(editor_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose an editor child entry for target metadata checks");
    if (editor_child_begin != std::string::npos) {
        const auto editor_child_json = visual_process.stdout_text.substr(editor_child_begin, 900);
        expect_contains(editor_child_json, "\"label\": \"Edit Method\"",
            "#1351: aggregate editor child JSON should expose action labels");
        expect_contains(editor_child_json, "\"kind\": \"source_editor\"",
            "#1351: aggregate editor child JSON should expose action kinds");
        expect_contains(editor_child_json,
            "\"description\": \"Open the selected visual object's PROCEDURE/FUNCTION source in a method editor.\"",
            "#1352: aggregate editor child JSON should expose action descriptions");
        expect_contains(editor_child_json, "\"targetSurface\": \"method-editor\"",
            "#1349: aggregate editor child JSON should expose target surfaces");
        expect_contains(editor_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate editor child JSON should expose asset paths");
        expect_contains(editor_child_json, "\"recordIndex\": 1",
            "#1349: aggregate editor child JSON should expose record indexes");
        expect_contains(editor_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate editor child JSON should expose object names");
        expect_contains(editor_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate editor child JSON should expose unique ids");
        expect_contains(editor_child_json, "\"symbol\": \"Click\"",
            "#1349: aggregate editor child JSON should expose symbols");
        expect_contains(editor_child_json, "\"line\": 12",
            "#1349: aggregate editor child JSON should expose source lines");
        expect_contains(editor_child_json, "\"column\": 4",
            "#1349: aggregate editor child JSON should expose source columns");
    }
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.method_editor.open\"",
        "#1337: designer execution JSON should expose editor child command tokens");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.method_editor.open\"",
        "#1338: designer execution JSON should expose editor child dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"launchCommand\": \"" COPPERFIN_TEST_SUCCESS_COMMAND "\"",
        "#1348: admitted designer execution JSON should expose child launch commands");
    expect_contains(visual_process.stdout_text, "\"executionAdmitted\": true",
        "#1339: designer execution JSON should expose child execution admission state");
    expect_contains(visual_process.stdout_text, "\"launched\": true",
        "#1339: designer execution JSON should expose child launch state");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.method_editor.open"}),
        "#1335: designer execution JSON should expose editor child executed commands");
    expect_contains(visual_process.stdout_text, "\"builderExecutions\": [",
        "#1325: designer execution JSON should expose builder execution results");
    expect_contains(visual_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1337: designer execution JSON should expose builder child identities");
    const auto builder_child_begin = visual_process.stdout_text.find("\"builderId\": \"form-builder\"");
    expect(builder_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose a builder child entry for target metadata checks");
    if (builder_child_begin != std::string::npos) {
        const auto builder_child_json = visual_process.stdout_text.substr(builder_child_begin, 700);
        expect_contains(builder_child_json, "\"title\": \"Form Builder\"",
            "#1351: aggregate builder child JSON should expose builder titles");
        expect_contains(builder_child_json, "\"kind\": \"builder\"",
            "#1351: aggregate builder child JSON should expose builder kinds");
        expect_contains(builder_child_json, "\"vfp9Equivalent\": \"builder.app form builder\"",
            "#1352: aggregate builder child JSON should expose VFP equivalent metadata");
        expect_contains(builder_child_json, "\"vfp9EquivalentDisplay\": \"builder.app form builder\"",
            "#4303: aggregate builder child JSON should expose localized VFP equivalent display metadata");
        expect_contains(builder_child_json, "\"copperfinComponent\": \"cf_form_surface\"",
            "#1352: aggregate builder child JSON should expose Copperfin component metadata");
        expect_contains(builder_child_json,
            "\"description\": \"Configure form-level data, layout, and generated method defaults.\"",
            "#1352: aggregate builder child JSON should expose builder descriptions");
        expect_contains(builder_child_json, "\"entryPoint\": \"cf_builders.form_builder\"",
            "#1349: aggregate builder child JSON should expose entry points");
        expect_contains(builder_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate builder child JSON should expose asset paths");
        expect_contains(builder_child_json, "\"recordIndex\": 1",
            "#1349: aggregate builder child JSON should expose record indexes");
        expect_contains(builder_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate builder child JSON should expose object names");
        expect_contains(builder_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate builder child JSON should expose unique ids");
    }
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1337: designer execution JSON should expose builder child command tokens");
    expect_contains(visual_process.stdout_text, "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1338: designer execution JSON should expose builder child dispatch arguments");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1335: designer execution JSON should expose builder child executed commands");
    expect_contains(visual_process.stdout_text, "\"toolboxExecution\": {",
        "#1325: designer execution JSON should expose toolbox execution results");
    const auto toolbox_child_begin = visual_process.stdout_text.find("\"toolboxExecution\": {");
    expect(toolbox_child_begin != std::string::npos,
        "#1349: designer execution JSON should expose a toolbox child entry for target metadata checks");
    if (toolbox_child_begin != std::string::npos) {
        const auto toolbox_child_json = visual_process.stdout_text.substr(toolbox_child_begin, 1600);
        expect_contains(toolbox_child_json, "\"assetPath\": \"forms/customer.scx\"",
            "#1349: aggregate toolbox child JSON should expose asset paths");
        expect_contains(toolbox_child_json, "\"recordIndex\": 1",
            "#1349: aggregate toolbox child JSON should expose record indexes");
        expect_contains(toolbox_child_json, "\"objectName\": \"frmCustomer\"",
            "#1349: aggregate toolbox child JSON should expose object names");
        expect_contains(toolbox_child_json, "\"uniqueId\": \"form-guid\"",
            "#1349: aggregate toolbox child JSON should expose unique ids");
        expect_contains(toolbox_child_json, "\"items\": [",
            "#1350: aggregate toolbox child JSON should expose toolbox item descriptors");
        expect_contains(toolbox_child_json, "\"id\": \"textbox\"",
            "#1350: aggregate toolbox child JSON should include form-safe TextBox items");
        expect_contains(toolbox_child_json, "\"baseClass\": \"TextBox\"",
            "#1350: aggregate toolbox child JSON should expose toolbox item base classes");
    }
    expect_contains(visual_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1337: designer execution JSON should expose toolbox child contexts");
    expect_contains(visual_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1337: designer execution JSON should expose toolbox child command tokens");
    expect_contains(visual_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1338: designer execution JSON should expose toolbox child dispatch arguments");
    expect_contains(visual_process.stdout_text, "\"dryRun\": false",
        "#1339: designer execution JSON should expose child dry-run state");
    expect_contains(visual_process.stdout_text, "\"mutatesAsset\": false",
        "#1339: designer execution JSON should expose child mutation state");
    expect_contains(visual_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_SUCCESS_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1335: designer execution JSON should expose toolbox child executed commands");

    const auto missing_builder_command_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-builder-invocations", "true",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(missing_builder_command_process.exit_code == 2,
        "#1325: designer execution JSON should reject missing required child launch commands");
    expect_contains(missing_builder_command_process.stdout_text,
        "No designer builder launch command was provided.",
        "#1325: missing designer builder launch command JSON should report parser errors");

    const auto dry_run_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-designer-execution", "true",
            "--json"
        },
        temp_root);
    expect(dry_run_process.exit_code == 4,
        "#1325: designer execution JSON should reject aggregate dry-run dispatches");
    expect_contains(dry_run_process.stdout_text, "\"designerExecution\": null",
        "#1325: dry-run designer execution JSON should not expose a result object");
    expect_contains(dry_run_process.stdout_text,
        "A designer dispatch execution request requires at least one admitted dispatch.",
        "#1325: dry-run designer execution JSON should report aggregate dispatch preflight errors");

    const auto unadmitted_execution_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "false",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(unadmitted_execution_process.exit_code == 4,
        "#1325: designer execution JSON should require explicit aggregate execution admission");
    expect_contains(unadmitted_execution_process.stdout_text,
        "A designer dispatch execution request requires explicit execution admission.",
        "#1325: unadmitted designer execution JSON should report aggregate execution admission errors");
    expect_contains(unadmitted_execution_process.stdout_text, "\"executed\": false",
        "#1325: unadmitted designer execution JSON should not mark execution complete");

    const auto failed_builder_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_builder_process.exit_code == 4,
        "#1325: designer execution JSON should fail when child executions fail");
    expect_contains(failed_builder_process.stdout_text,
        "Designer builder launch command returned a non-zero exit code.",
        "#1325: failed designer child execution JSON should expose child errors");
    expect_contains(failed_builder_process.stdout_text, "\"errorCount\": ",
        "#1325: failed designer execution JSON should expose execution error counts");
    expect_contains(failed_builder_process.stdout_text, "\"executionReadySelectionContexts\": []",
        "#1402: failed builder execution JSON should expose empty execution-ready selected contexts");
    expect_contains(failed_builder_process.stdout_text, "\"executionBlockedSelectionContexts\": [\"visual_object\"]",
        "#1402: failed builder execution JSON should summarize execution-blocked selected contexts");
    expect_contains(failed_builder_process.stdout_text,
        "\"executionBlockedErrors\": [\"Designer builder launch command returned a non-zero exit code.\"]",
        "#1402: failed builder execution JSON should summarize blocked selected-context errors");
    expect_contains(failed_builder_process.stdout_text, "\"executed\": false",
        "#1325: failed designer execution JSON should not mark aggregate execution complete");
    expect_contains(failed_builder_process.stdout_text, "\"builderId\": \"form-builder\"",
        "#1340: failed designer execution JSON should preserve planned builder identity");
    expect_contains(failed_builder_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: failed builder execution JSON should preserve editor action executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"builderExecutedCount\": 0",
        "#1344: failed builder execution JSON should expose zero builder executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: failed builder execution JSON should preserve toolbox executed counts");
    expect_contains(failed_builder_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: failed builder execution JSON should preserve zero editor action errors");
    expect_contains(failed_builder_process.stdout_text, "\"builderErrorCount\": 3",
        "#1343: failed builder execution JSON should expose builder error counts");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: failed builder execution JSON should preserve zero toolbox errors");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderIds\": [\"form-builder\", \"control-builder\", \"grid-builder\"]",
        "#1345: failed builder execution JSON should summarize failed builder ids");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderCommandTokens\": [\"studio.builder.invoke\"",
        "#1353: failed builder execution JSON should summarize failed builder command tokens");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderExecutedCommands\": [\"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1354: failed builder execution JSON should summarize failed builder executed commands");
    expect_contains(failed_builder_process.stdout_text, "\"failedBuilderExitCodes\": [1, 1, 1]",
        "#1355: failed builder execution JSON should summarize failed builder exit codes");
    expect_contains(failed_builder_process.stdout_text,
        "\"failedBuilderErrors\": [\"Designer builder launch command returned a non-zero exit code.\"",
        "#1346: failed builder execution JSON should summarize failed builder errors");
    expect_contains(failed_builder_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate builder execution JSON should expose child launch commands");
    expect_contains(failed_builder_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate builder execution JSON should report normalized child exit codes");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: failed builder execution JSON should preserve no failed editor actions");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: failed builder execution JSON should preserve no failed editor action command tokens");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: failed builder execution JSON should preserve no failed editor action executed commands");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: failed builder execution JSON should preserve no failed editor action exit codes");
    expect_contains(failed_builder_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: failed builder execution JSON should preserve no failed editor action errors");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: failed builder execution JSON should preserve successful toolbox summary state");
    expect_contains(failed_builder_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: failed builder execution JSON should preserve empty toolbox error summaries");
    expect_contains(failed_builder_process.stdout_text, "\"commandToken\": \"studio.builder.invoke\"",
        "#1340: failed designer execution JSON should preserve planned builder command token");
    expect_contains(failed_builder_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.builder.invoke\"",
        "#1340: failed designer execution JSON should preserve planned builder dispatch arguments");
    expect_contains(failed_builder_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.builder.invoke"}),
        "#1340: failed designer execution JSON should preserve the failed builder command");

    const auto failed_editor_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_editor_process.exit_code == 4,
        "#1341: designer execution JSON should fail when editor action executions fail");
    expect_contains(failed_editor_process.stdout_text,
        "Designer editor action launch command returned a non-zero exit code.",
        "#1341: failed editor action execution JSON should expose child errors");
    expect_contains(failed_editor_process.stdout_text, "\"actionId\": \"show-property-grid\"",
        "#1341: failed editor action execution JSON should preserve planned editor action identity");
    expect_contains(failed_editor_process.stdout_text, "\"editorActionExecutedCount\": 0",
        "#1344: failed editor action execution JSON should expose zero editor action executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: failed editor action execution JSON should preserve builder executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxExecutedCount\": 1",
        "#1344: failed editor action execution JSON should preserve toolbox executed counts");
    expect_contains(failed_editor_process.stdout_text, "\"editorActionErrorCount\": 5",
        "#1343: failed editor action execution JSON should expose editor action error counts");
    expect_contains(failed_editor_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: failed editor action execution JSON should preserve zero builder errors");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxErrorCount\": 0",
        "#1343: failed editor action execution JSON should preserve zero toolbox errors");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionIds\": [\"show-property-grid\", \"edit-visual-method\"",
        "#1345: failed editor action execution JSON should summarize failed editor action ids");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionCommandTokens\": [\"studio.property_grid.show\", \"studio.method_editor.open\"",
        "#1353: failed editor action execution JSON should summarize failed editor action command tokens");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionExecutedCommands\": [\"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.property_grid.show"}),
        "#1354: failed editor action execution JSON should summarize failed editor action executed commands");
    expect_contains(failed_editor_process.stdout_text, "\"failedEditorActionExitCodes\": [1, 1, 1, 1, 1]",
        "#1355: failed editor action execution JSON should summarize failed editor action exit codes");
    expect_contains(failed_editor_process.stdout_text,
        "\"failedEditorActionErrors\": [\"Designer editor action launch command returned a non-zero exit code.\"",
        "#1346: failed editor action execution JSON should summarize failed editor action errors");
    expect_contains(failed_editor_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate editor action execution JSON should expose child launch commands");
    expect_contains(failed_editor_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate editor action execution JSON should report normalized child exit codes");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: failed editor action execution JSON should preserve no failed builders");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: failed editor action execution JSON should preserve no failed builder command tokens");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: failed editor action execution JSON should preserve no failed builder executed commands");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: failed editor action execution JSON should preserve no failed builder exit codes");
    expect_contains(failed_editor_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: failed editor action execution JSON should preserve no failed builder errors");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxFailed\": false",
        "#1345: failed editor action execution JSON should preserve successful toolbox summary state");
    expect_contains(failed_editor_process.stdout_text, "\"toolboxError\": \"\"",
        "#1346: failed editor action execution JSON should preserve empty toolbox error summaries");
    expect_contains(failed_editor_process.stdout_text, "\"commandToken\": \"studio.property_grid.show\"",
        "#1341: failed editor action execution JSON should preserve planned editor action command token");
    expect_contains(failed_editor_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.property_grid.show\"",
        "#1341: failed editor action execution JSON should preserve planned editor action dispatch arguments");
    expect_contains(failed_editor_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.property_grid.show"}),
        "#1341: failed editor action execution JSON should preserve the failed editor action command");

    const auto failed_toolbox_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "visual_object",
            "--admit-editor-invocations", "true",
            "--admit-builder-invocations", "true",
            "--admit-toolbox-invocation", "true",
            "--admit-designer-execution", "true",
            "--editor-action-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--builder-launch-command", COPPERFIN_TEST_SUCCESS_COMMAND,
            "--toolbox-launch-command", COPPERFIN_TEST_FAILURE_COMMAND,
            "--json"
        },
        temp_root);
    expect(failed_toolbox_process.exit_code == 4,
        "#1341: designer execution JSON should fail when toolbox execution fails");
    expect_contains(failed_toolbox_process.stdout_text,
        "Designer toolbox launch command returned a non-zero exit code.",
        "#1341: failed toolbox execution JSON should expose child errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionExecutionCount\": 5",
        "#1342: failed toolbox execution JSON should preserve editor action execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderExecutionCount\": 3",
        "#1342: failed toolbox execution JSON should preserve builder execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExecutionCount\": 1",
        "#1342: failed toolbox execution JSON should preserve toolbox execution counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionExecutedCount\": 5",
        "#1344: failed toolbox execution JSON should preserve editor action executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderExecutedCount\": 3",
        "#1344: failed toolbox execution JSON should preserve builder executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExecutedCount\": 0",
        "#1344: failed toolbox execution JSON should expose zero toolbox executed counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"editorActionErrorCount\": 0",
        "#1343: failed toolbox execution JSON should preserve zero editor action errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"builderErrorCount\": 0",
        "#1343: failed toolbox execution JSON should preserve zero builder errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxErrorCount\": 1",
        "#1343: failed toolbox execution JSON should expose toolbox error counts");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionIds\": []",
        "#1345: failed toolbox execution JSON should preserve no failed editor actions");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionCommandTokens\": []",
        "#1353: failed toolbox execution JSON should preserve no failed editor action command tokens");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionExecutedCommands\": []",
        "#1354: failed toolbox execution JSON should preserve no failed editor action executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionExitCodes\": []",
        "#1355: failed toolbox execution JSON should preserve no failed editor action exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedEditorActionErrors\": []",
        "#1346: failed toolbox execution JSON should preserve no failed editor action errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderIds\": []",
        "#1345: failed toolbox execution JSON should preserve no failed builders");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderCommandTokens\": []",
        "#1353: failed toolbox execution JSON should preserve no failed builder command tokens");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderExecutedCommands\": []",
        "#1354: failed toolbox execution JSON should preserve no failed builder executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderExitCodes\": []",
        "#1355: failed toolbox execution JSON should preserve no failed builder exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"failedBuilderErrors\": []",
        "#1346: failed toolbox execution JSON should preserve no failed builder errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxFailed\": true",
        "#1345: failed toolbox execution JSON should summarize failed toolbox state");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"toolboxError\": \"Designer toolbox launch command returned a non-zero exit code.\"",
        "#1346: failed toolbox execution JSON should summarize failed toolbox errors");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxCommandToken\": \"studio.toolbox.palette.invoke\"",
        "#1353: failed toolbox execution JSON should summarize toolbox command tokens");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"toolboxExecutedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1354: failed toolbox execution JSON should summarize toolbox executed commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxExitCode\": 1",
        "#1355: failed toolbox execution JSON should summarize toolbox exit codes");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"launchCommand\": \"" COPPERFIN_TEST_FAILURE_COMMAND "\"",
        "#1348: failed aggregate toolbox execution JSON should expose child launch commands");
    expect_contains(failed_toolbox_process.stdout_text, "\"observedExitCode\": 1",
        "#1347: failed aggregate toolbox execution JSON should report normalized child exit codes");
    expect_contains(failed_toolbox_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox context");
    expect_contains(failed_toolbox_process.stdout_text, "\"commandToken\": \"studio.toolbox.palette.invoke\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox command token");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"dispatchArguments\": [\"--command-token\", \"studio.toolbox.palette.invoke\"",
        "#1341: failed toolbox execution JSON should preserve planned toolbox dispatch arguments");
    expect_contains(failed_toolbox_process.stdout_text,
        "\"executedCommand\": \"" + expected_json_shell_command(
            COPPERFIN_TEST_FAILURE_COMMAND,
            {"--command-token", "studio.toolbox.palette.invoke"}),
        "#1341: failed toolbox execution JSON should preserve the failed toolbox command");

    const auto unknown_context_process = run_process_capture(
        studio_host_path,
        {
            "--designer-execute",
            "--selection-context", "unknown",
            "--json"
        },
        temp_root);
    expect(unknown_context_process.exit_code == 2,
        "#1325: designer execution JSON should reject unknown selection contexts");
    expect_contains(unknown_context_process.stdout_text, "Unknown selection context token: unknown",
        "#1325: unknown designer execution context JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
