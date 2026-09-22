void test_xasset_menu_bootstrap_uses_safe_runtime_symbols() {
    // RQ-CF-XASSET-MENU-NAME-001: filenames are asset identity, not source identifiers.
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_xasset_menu_symbols";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    std::vector<std::string> targets;
    const std::vector<std::string> stems{
        "main menu", "main-menu", "O'Brien", "9first", "caf\xC3\xA9 menu",
        "DEFINE", ".hidden", "a b", "a-b"};
    for (std::size_t index = 0U; index < stems.size(); ++index) {
        const auto& stem = stems[index];
        copperfin::studio::StudioDocumentModel document;
        document.path = copperfin::platform::path_to_utf8_string(
            root / copperfin::platform::path_from_utf8_string(stem + ".mnx"));
        document.kind = copperfin::studio::StudioAssetKind::menu;
        document.table_preview_available = true;
        const auto model = copperfin::runtime::build_xasset_executable_model(document);
        expect(model.ok && model.asset_path == document.path &&
                   model.activation_source_stem == stem,
               "#6459: menu model must retain the original logical path and stem");
        if (!model.ok) {
            continue;
        }
        const std::string prefix = "__cf_menu_";
        expect(model.activation_target.size() == prefix.size() + 64U &&
                   model.activation_target.rfind(prefix, 0U) == 0U &&
                   std::all_of(model.activation_target.begin() + prefix.size(),
                               model.activation_target.end(), [](const char ch) {
                                   return (ch >= '0' && ch <= '9') ||
                                          (ch >= 'a' && ch <= 'f');
                               }),
               "#6459: menu runtime identity must be a bounded VFP identifier");
        targets.push_back(model.activation_target);
        std::string source = copperfin::runtime::build_xasset_bootstrap_source(model, true);
        expect(source.find("DEFINE MENU " + model.activation_target + "\n") != std::string::npos &&
                   source.find("ACTIVATE MENU " + model.activation_target + "\n") != std::string::npos &&
                   source.find("DEACTIVATE MENU " + model.activation_target + "\n") != std::string::npos &&
                   source.find("RELEASE MENU " + model.activation_target + "\n") != std::string::npos,
               "#6459: menu definition, activation, and teardown must share the internal symbol");
        if (index == 0U) {
            source += "PROCEDURE __cf_finish_menu\nCLEAR EVENTS\nENDPROC\n";
        }
        const fs::path program = root / ("menu_" + std::to_string(index) + ".prg");
        write_text(program, source);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(program.string(), root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::event_loop,
               "#6459: generated menu bootstrap must reach the event loop");
        expect(std::any_of(state.events.begin(), state.events.end(), [&](const auto& event) {
            return event.category == "menu.activate" &&
                   event.detail == model.activation_target;
        }), "#6459: menu activation event must use the internal symbol");
        if (index == 0U) {
            expect(session.dispatch_event_handler("__cf_finish_menu"),
                   "#6459: event handler must be able to exit the generated menu loop");
            const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(completed.reason == copperfin::runtime::DebugPauseReason::completed,
                   "#6459: menu bootstrap must finish after CLEAR EVENTS");
            expect(std::any_of(completed.events.begin(), completed.events.end(), [&](const auto& event) {
                return event.category == "menu.deactivate" &&
                       event.detail == model.activation_target;
            }) && std::any_of(completed.events.begin(), completed.events.end(), [&](const auto& event) {
                return event.category == "menu.release" &&
                       event.detail == model.activation_target;
            }), "#6459: menu teardown events must use the same internal symbol");
        }
    }
    expect(targets.size() == stems.size() && targets[0] != targets[1] &&
               targets[7] != targets[8],
           "#6459: distinct unsafe stems must not collide after symbol generation");
    fs::remove_all(root, ignored);
}
