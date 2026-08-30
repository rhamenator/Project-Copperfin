void test_report_form_to_file_renders_without_event_loop_pause() {
    namespace fs = std::filesystem;
    const fs::path report_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\invoice.frx)";
    if (!fs::exists(report_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_render";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "invoice_render.txt";
    const fs::path main_path = temp_root / "report_render.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' TO FILE '" + output_path.string() + "'\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPORT FORM TO FILE should complete without entering a preview event loop");
    expect(!state.waiting_for_events, "REPORT FORM TO FILE should not leave the runtime waiting_for_events");
    expect(fs::exists(output_path), "REPORT FORM TO FILE should materialize an output artifact");

    const auto x_value = state.globals.find("x");
    expect(x_value != state.globals.end(), "statements after REPORT FORM TO FILE should continue executing");
    if (x_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(x_value->second) == "2", "REPORT FORM TO FILE should not block follow-on statements");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "report.render";
    }), "REPORT FORM TO FILE should emit a report.render event");

    fs::remove_all(temp_root, ignored);
}

void test_sys2040_report_status_tracks_preview_and_output() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sys2040_report_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path preview_asset = temp_root / "status_preview.frx";
    const fs::path preview_program = temp_root / "status_preview.prg";
    write_synthetic_report_surface(preview_asset);
    write_text(
        preview_program,
        "REPORT FORM '" + preview_asset.string() + "' PREVIEW\n"
        "nAfterPreview = SYS(2040)\n"
        "RETURN\n"
        "PROCEDURE closepreview\n"
        "CLEAR EVENTS\n"
        "RETURN\n"
        "ENDPROC\n");

    auto preview_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(preview_program.string(), temp_root.string()));
    const auto preview_state = preview_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(preview_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2040) preview fixture should pause in the report event loop");
    const auto preview_status = preview_session.evaluate_watch_expression("SYS(2040)");
    expect(preview_status.ok && copperfin::runtime::format_value(preview_status.value) == "1",
           "SYS(2040) should report preview status while REPORT FORM PREVIEW is active");
    const auto preview_interrupted = preview_session.evaluate_watch_expression("SYS(2024)");
    expect(preview_interrupted.ok && copperfin::runtime::format_value(preview_interrupted.value) == "N",
           "SYS(2024) should report an uninterrupted active preview as character N");
    expect(preview_session.dispatch_event_handler("closepreview"),
           "SYS(2040) preview fixture should dispatch its cleanup handler");
    const auto after_preview_state = preview_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(after_preview_state.completed,
           "SYS(2040) preview fixture should complete after CLEAR EVENTS");
    const auto after_preview = after_preview_state.globals.find("nafterpreview");
    expect(after_preview != after_preview_state.globals.end() &&
               copperfin::runtime::format_value(after_preview->second) == "0",
           "SYS(2040) should reset to idle after preview cleanup");

    const fs::path interrupted_program = temp_root / "status_interrupted.prg";
    write_text(
        interrupted_program,
        "REPORT FORM '" + preview_asset.string() + "' PREVIEW\n"
        "RETURN\n"
        "PROCEDURE interruptpreview\n"
        "CANCEL\n"
        "RETURN\n"
        "ENDPROC\n");
    auto interrupted_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(interrupted_program.string(), temp_root.string()));
    const auto interrupted_preview = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(interrupted_preview.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2024) interruption fixture should enter the report event loop");
    expect(interrupted_session.dispatch_event_handler("interruptpreview"),
           "SYS(2024) interruption fixture should dispatch its cancellation handler");
    const auto interrupted_step_state = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::step_into);
    expect(interrupted_step_state.reason == copperfin::runtime::DebugPauseReason::step,
           "SYS(2024) interruption should stop after executing the cancellation handler");
    const auto interrupted_public = interrupted_session.evaluate_watch_expression("SYS(2024)");
    expect(interrupted_public.ok &&
               interrupted_public.value.kind == copperfin::runtime::PrgValueKind::string &&
               copperfin::runtime::format_value(interrupted_public.value) == "Y",
           "SYS(2024) should return public character Y after CANCEL interrupts an active preview");
    const auto interrupted_event_state = interrupted_session.run(
        copperfin::runtime::DebugResumeAction::continue_run);
    expect(interrupted_event_state.reason == copperfin::runtime::DebugPauseReason::event_loop,
           "SYS(2024) interruption should restore the existing event-loop contract after handler dispatch");
    expect(std::any_of(
               interrupted_event_state.events.begin(),
               interrupted_event_state.events.end(),
               [](const copperfin::runtime::RuntimeEvent& event) {
                   return event.category == "report.interrupted" && event.detail == "Y";
               }),
           "SYS(2024) should record Y when CANCEL interrupts an active preview");

    const fs::path table_path = temp_root / "status_people.dbf";
    const fs::path output_path = temp_root / "status_output.txt";
    const fs::path output_program = temp_root / "status_output.prg";
    write_named_age_dbf(table_path, {{"Alice", 20}});
    write_synthetic_report_status_surface(temp_root / "status_output.frx", "SYS(2024)");
    write_text(
        output_program,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPORT FORM '" + (temp_root / "status_output.frx").string() + "' TO FILE '" + output_path.string() + "'\n"
        "RETURN\n");

    auto output_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(output_program.string(), temp_root.string()));
    const auto output_state = output_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(output_state.completed, "SYS(2040) output fixture should complete");
    const std::string output_text = read_text(output_path);
    expect(output_text.find("object_exprs=1:N") != std::string::npos,
           "SYS(2024) should report an uninterrupted output as character N");

    fs::remove_all(temp_root, ignored);
}

void test_label_form_to_file_renders_without_event_loop_pause() {
    namespace fs = std::filesystem;
    const fs::path label_path = R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports\cust.lbx)";
    if (!fs::exists(label_path)) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_label_render";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path output_path = temp_root / "cust_render.txt";
    const fs::path main_path = temp_root / "label_render.prg";
    write_text(
        main_path,
        "LABEL FORM '" + label_path.string() + "' TO FILE '" + output_path.string() + "'\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LABEL FORM TO FILE should complete without entering a preview event loop");
    expect(!state.waiting_for_events, "LABEL FORM TO FILE should not leave the runtime waiting_for_events");
    expect(fs::exists(output_path), "LABEL FORM TO FILE should materialize an output artifact");

    const auto x_value = state.globals.find("x");
    expect(x_value != state.globals.end(), "statements after LABEL FORM TO FILE should continue executing");
    if (x_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(x_value->second) == "2", "LABEL FORM TO FILE should not block follow-on statements");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "label.render";
    }), "LABEL FORM TO FILE should emit a label.render event");

    fs::remove_all(temp_root, ignored);
}

void test_report_and_label_to_file_emit_filtered_data_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_filtered_rows";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_named_age_dbf(table_path, {{"Alice", 20}, {"Bob", 30}, {"Cara", 32}, {"Dana", 40}});

    const auto run_case = [&](const std::string& command, const fs::path& asset_path, const fs::path& output_path) {
        write_synthetic_report_surface(asset_path);

        const fs::path main_path = temp_root / (asset_path.stem().string() + ".prg");
        write_text(
            main_path,
            "USE '" + table_path.string() + "' ALIAS People IN 0\n"
            "SET FILTER TO AGE >= 20\n"
            + command + " '" + asset_path.string() + "' TO FILE '" + output_path.string() + "' FOR AGE <> 30 WHILE AGE < 35\n"
            "x = 2\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, command + " TO FILE with FOR/WHILE should complete");
        expect(fs::exists(output_path), command + " TO FILE with FOR/WHILE should materialize an output artifact");

        const std::string output_text = read_text(output_path);
        expect(output_text.find("sections=1") != std::string::npos,
               command + " TO FILE should preserve shared layout section metadata");
        expect(output_text.find("cursor=People") != std::string::npos,
               command + " TO FILE should identify the active work area");
        expect(output_text.find("set_filter=AGE >= 20") != std::string::npos,
               command + " TO FILE should surface the active cursor filter");
        expect(output_text.find("for=AGE <> 30") != std::string::npos,
               command + " TO FILE should surface the parsed FOR clause");
        expect(output_text.find("while=AGE < 35") != std::string::npos,
               command + " TO FILE should surface the parsed WHILE clause");
        expect(output_text.find("rows=2") != std::string::npos,
               command + " TO FILE should render only the qualifying rows");
        expect(output_text.find("row[1]=NAME=Alice|AGE=20") != std::string::npos,
               command + " TO FILE should render the first qualifying record");
        expect(output_text.find("row[1]=NAME=Alice|AGE=20|object_exprs=1:Alice") != std::string::npos,
               command + " TO FILE should evaluate the layout object expression for the first qualifying record");
        expect(output_text.find("row[3]=NAME=Cara|AGE=32") != std::string::npos,
               command + " TO FILE should render later qualifying records before the WHILE boundary");
        expect(output_text.find("row[3]=NAME=Cara|AGE=32|object_exprs=1:Cara") != std::string::npos,
               command + " TO FILE should evaluate the layout object expression for later qualifying records");
        expect(output_text.find("Bob") == std::string::npos,
               command + " TO FILE should exclude rows filtered by FOR");
        expect(output_text.find("Dana") == std::string::npos,
               command + " TO FILE should stop before rows outside the WHILE boundary");

        const auto x_value = state.globals.find("x");
        expect(x_value != state.globals.end(), command + " TO FILE should continue executing follow-on statements");
        if (x_value != state.globals.end()) {
            expect(copperfin::runtime::format_value(x_value->second) == "2",
                   command + " TO FILE should not block follow-on statements");
        }
    };

    run_case("REPORT FORM", temp_root / "filtered_report.frx", temp_root / "filtered_report.txt");
    run_case("LABEL FORM", temp_root / "filtered_label.lbx", temp_root / "filtered_label.txt");

    fs::remove_all(temp_root, ignored);
}

void test_report_form_missing_asset_uses_localized_error() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_report_missing_asset";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "missing_invoice.frx";
    const fs::path main_path = temp_root / "report_missing_asset.prg";
    write_text(
        main_path,
        "REPORT FORM '" + report_path.string() + "' PREVIEW\n"
        "x = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "missing report asset should pause with an error");
    expect(
        state.message == active_runtime_text(
            "Runtime.Prg.ReportAsset.Error.ResolveFailed",
            {{"path", report_path.string()}}),
        "missing report asset error should route through the active locale catalog");

    const auto x_value = state.globals.find("x");
    expect(x_value == state.globals.end(), "statements after missing report asset should not execute");

    fs::remove_all(temp_root, ignored);
}
