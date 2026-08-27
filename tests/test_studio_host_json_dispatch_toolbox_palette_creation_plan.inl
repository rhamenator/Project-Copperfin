void test_studio_host_json_plans_toolbox_object_creation_from_palette_dispatch(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_toolbox_create_from_dispatch_plan_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path form_path = temp_root / "customer.scx";
    write_synthetic_form_table_for_toolbox_creation(form_path);
    const std::size_t before_count = visual_object_count(form_path);

    const auto plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--record", "0",
            "--object-name", "frmCustomer",
            "--unique-id", "form-guid",
            "--create-unique-id", "dispatch-textbox-guid",
            "--field-value", "CAPTION=Dispatch Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(plan_process.exit_code == 0,
        "#1261: toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create plans");
    expect_contains(plan_process.stdout_text, "\"toolboxItemId\": \"textbox\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose selected toolbox items");
    expect_contains(plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose descriptor class names");
    expect_contains(plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#1261: toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(plan_process.stdout_text, "\"toolboxContext\": \"form\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should resolve visual-object form contexts");
    expect_contains(plan_process.stdout_text, "\"targetRecordIndex\": 2",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose planned target indexes");
    expect_contains(plan_process.stdout_text, "\"objectName\": \"txt2\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose generated object names");
    expect_contains(plan_process.stdout_text, "\"uniqueId\": \"dispatch-textbox-guid\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create unique-id overrides");
    expect_contains(plan_process.stdout_text, "\"parentName\": \"frmCustomer\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should default parents from selected objects");
    expect_contains(plan_process.stdout_text, "\"propertyName\": \"CAPTION\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose caller field values");
    expect_contains(plan_process.stdout_text, "\"propertyValue\": \"Dispatch Plan\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose caller field-value payloads");
    expect_contains(plan_process.stdout_text, "\"dryRun\": true",
        "#1261: toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(plan_process.stdout_text, "\"mutatesAsset\": false",
        "#1261: toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect(visual_object_count(form_path) == before_count,
        "#1261: toolbox-create-from-dispatch-plan host command should not mutate the visual asset");

    const auto override_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "commandbutton",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--create-object-name", "cmdLaunch",
            "--create-parent-name", "cntToolbar",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(override_process.exit_code == 0,
        "#1261: toolbox-create-from-dispatch-plan JSON should accept create override fields");
    expect_contains(override_process.stdout_text, "\"objectName\": \"cmdLaunch\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create object-name overrides");
    expect_contains(override_process.stdout_text, "\"parentName\": \"cntToolbar\"",
        "#1261: toolbox-create-from-dispatch-plan JSON should expose create parent overrides");
    expect(visual_object_count(form_path) == before_count,
        "#1261: toolbox-create-from-dispatch-plan override commands should not mutate the visual asset");

    const auto report_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "label",
            "--selection-context", "report_expression",
            "--create-unique-id", "dispatch-report-plan-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Report Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(report_plan_process.exit_code == 0,
        "#2142: report toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(report_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose stable create plans");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2142: report toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(report_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(report_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose label plans");
    expect_contains(report_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose generated label names");
    expect_contains(report_plan_process.stdout_text, "\"uniqueId\": \"dispatch-report-plan-guid\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label unique ids");
    expect_contains(report_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label parent overrides");
    expect_contains(report_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Report Plan\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should expose report label field values");
    expect_contains(report_plan_process.stdout_text, "\"dryRun\": true",
        "#2142: report toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(report_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2142: report toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(report_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2142: report toolbox-create-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2142: report toolbox-create-from-dispatch-plan host command should not mutate assets");

    const auto label_plan_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "label",
            "--selection-context", "label_expression",
            "--create-unique-id", "dispatch-label-plan-guid",
            "--create-parent-name", "DetailBand",
            "--field-value", "CAPTION=Dispatch Label Plan",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(label_plan_process.exit_code == 0,
        "#2139: label toolbox-create-from-dispatch-plan JSON command should exit successfully");
    expect_contains(label_plan_process.stdout_text, "\"toolboxCreatePlan\": {",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose stable create plans");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContextProvided\": true",
        "#2139: label toolbox-create-from-dispatch-plan JSON should use dispatch toolbox contexts");
    expect_contains(label_plan_process.stdout_text, "\"toolboxContext\": \"report\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should resolve report contexts");
    expect_contains(label_plan_process.stdout_text, "\"toolboxItemId\": \"label\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label plans");
    expect_contains(label_plan_process.stdout_text, "\"objectName\": \"lbl1\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose generated label names");
    expect_contains(label_plan_process.stdout_text, "\"uniqueId\": \"dispatch-label-plan-guid\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label unique ids");
    expect_contains(label_plan_process.stdout_text, "\"parentName\": \"DetailBand\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label parent overrides");
    expect_contains(label_plan_process.stdout_text, "\"propertyValue\": \"Dispatch Label Plan\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should expose label field values");
    expect_contains(label_plan_process.stdout_text, "\"dryRun\": true",
        "#2139: label toolbox-create-from-dispatch-plan JSON should remain a create plan");
    expect_contains(label_plan_process.stdout_text, "\"mutatesAsset\": false",
        "#2139: label toolbox-create-from-dispatch-plan JSON should remain non-mutating");
    expect_not_contains(label_plan_process.stdout_text, "\"className\": \"TextBox\"",
        "#2139: label toolbox-create-from-dispatch-plan JSON should exclude form-only textbox plans");
    expect(visual_object_count(form_path) == before_count,
        "#2139: label toolbox-create-from-dispatch-plan host command should not mutate assets");

    const auto non_admitted_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--object-name", "frmCustomer",
            "--json"
        },
        temp_root);
    expect(non_admitted_process.exit_code == 4,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject non-admitted palette dispatches");
    expect_contains(non_admitted_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1261: failed toolbox-create-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(non_admitted_process.stdout_text,
        "A toolbox dispatch request requires an admitted non-dry-run invocation.",
        "#1261: non-admitted toolbox-create-from-dispatch-plan JSON should report dispatch errors");
    expect(visual_object_count(form_path) == before_count,
        "#1261: non-admitted toolbox-create-from-dispatch-plan commands should not mutate the visual asset");

    const auto unavailable_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "report_expression",
            "--object-name", "rptCustomer",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(unavailable_process.exit_code == 4,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject unavailable dispatch items");
    expect_contains(unavailable_process.stdout_text, "\"toolboxCreatePlan\": null",
        "#1261: unavailable toolbox-create-from-dispatch-plan JSON should not expose stale plans");
    expect_contains(unavailable_process.stdout_text,
        "The requested toolbox item is not available in the admitted toolbox dispatch.",
        "#1261: unavailable toolbox-create-from-dispatch-plan JSON should report availability errors");
    expect(visual_object_count(form_path) == before_count,
        "#1261: unavailable toolbox-create-from-dispatch-plan commands should not mutate the visual asset");

    const auto missing_context_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--admit-palette-invocation", "true",
            "--json"
        },
        temp_root);
    expect(missing_context_process.exit_code == 2,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject missing selection contexts");
    expect_contains(missing_context_process.stdout_text, "No selection context was provided.",
        "#1261: missing-context toolbox-create-from-dispatch-plan JSON should report parser errors");

    const auto invalid_admission_process = run_process_capture(
        studio_host_path,
        {
            "--path", form_path.string(),
            "--toolbox-create-from-dispatch-plan", "textbox",
            "--selection-context", "visual_object",
            "--admit-palette-invocation", "maybe",
            "--json"
        },
        temp_root);
    expect(invalid_admission_process.exit_code == 2,
        "#1261: toolbox-create-from-dispatch-plan JSON should reject invalid admission tokens");
    expect_contains(invalid_admission_process.stdout_text,
        "The --admit-palette-invocation value must be true or false.",
        "#1261: invalid-admission toolbox-create-from-dispatch-plan JSON should report parser errors");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
