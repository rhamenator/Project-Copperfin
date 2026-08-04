// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_prg_engine_data_io_support.h"

namespace cf_test_prg_engine_data_io {
void test_m_dot_namespace_shares_bare_memory_variable_binding() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_m_dot_namespace";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "m_dot.prg";
    write_text(
        main_path,
        "m.customer = 'Alice'\n"
        "from_bare = customer\n"
        "M.customer = 'Bob'\n"
        "from_prefixed = m.customer\n"
        "customer = 'Carol'\n"
        "from_m_after_bare = m.customer\n"
        "PUBLIC m.public_name\n"
        "m.public_name = 'Public'\n"
        "from_public_bare = public_name\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "m. namespace script should complete");

    const auto customer = state.globals.find("customer");
    const auto prefixed_customer = state.globals.find("m.customer");
    const auto from_bare = state.globals.find("from_bare");
    const auto from_prefixed = state.globals.find("from_prefixed");
    const auto from_m_after_bare = state.globals.find("from_m_after_bare");
    const auto from_public_bare = state.globals.find("from_public_bare");

    expect(customer != state.globals.end(), "m.customer should be stored under the bare customer binding");
    expect(prefixed_customer == state.globals.end(), "m.customer should not create a separate prefixed global binding");
    expect(from_bare != state.globals.end(), "bare reads should see m.customer assignments");
    expect(from_prefixed != state.globals.end(), "m. reads should see bare memory-variable storage");
    expect(from_m_after_bare != state.globals.end(), "m. reads should see later bare assignments");
    expect(from_public_bare != state.globals.end(), "PUBLIC m.name should declare the bare memory-variable binding");

    if (customer != state.globals.end()) {
        expect(copperfin::runtime::format_value(customer->second) == "Carol",
            "bare assignment should update the shared m.customer binding");
    }
    if (from_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_bare->second) == "Alice",
            "bare reads should resolve the initial m.customer value");
    }
    if (from_prefixed != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_prefixed->second) == "Bob",
            "m.customer reads should resolve the updated shared value");
    }
    if (from_m_after_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_m_after_bare->second) == "Carol",
            "m.customer should resolve a value assigned through the bare name");
    }
    if (from_public_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_public_bare->second) == "Public",
            "PUBLIC m.public_name should be readable through public_name");
    }

    fs::remove_all(temp_root, ignored);
}

void test_edit_command_emits_runtime_edit_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_edit_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "edit_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO LIKE N*\n"
        "EDIT\n"
        "EDIT MEMO notes\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "EDIT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> edit_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.edit") {
            edit_events.push_back(event);
        }
    }

    expect(edit_events.size() == 2U, "EDIT commands should emit two runtime.edit events");
    if (edit_events.size() >= 2U) {
        expect(edit_events[0].detail.find("people@") != std::string::npos,
            "EDIT should surface the selected cursor");
        expect(edit_events[0].detail.find("fields=NAME") != std::string::npos,
            "EDIT should honor SET FIELDS metadata");
        expect(edit_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "EDIT should surface the current filter");
        expect(edit_events[1].detail.find("memo=notes") != std::string::npos,
            "EDIT MEMO should record the memo field name");
    }

    fs::remove_all(temp_root, ignored);
}

void test_change_command_emits_runtime_change_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_change_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "change_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "CHANGE\n"
        "CHANGE FIELD NAME,AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CHANGE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> change_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.change") {
            change_events.push_back(event);
        }
    }

    expect(change_events.size() == 2U, "CHANGE commands should emit two runtime.change events");
    if (change_events.size() >= 2U) {
        expect(change_events[0].detail.find("people@") != std::string::npos,
            "CHANGE should surface the selected cursor");
        expect(change_events[0].detail.find("fields=NAME,AGE") != std::string::npos,
            "CHANGE without an explicit field list should surface the effective visible fields");
        expect(change_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "CHANGE should surface the current filter");
        expect(change_events[1].detail.find("fields=NAME,AGE") != std::string::npos,
            "CHANGE FIELD should record the field list");
    }

    fs::remove_all(temp_root, ignored);
}

void test_input_command_emits_runtime_input_event_with_prompt() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_input_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "input_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "INPUT \"Enter a value: \" TO myvar\n"
        "cAfterInput = myvar\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INPUT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> input_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.input") {
            input_events.push_back(event);
        }
    }

    expect(input_events.size() == 1U, "INPUT command should emit one runtime.input event");
    const auto after_input = state.globals.find("cafterinput");
    if (input_events.size() >= 1U) {
        expect(input_events[0].detail.find("people@") != std::string::npos,
            "INPUT should surface the selected cursor");
        expect(input_events[0].detail.find("fields=NAME") != std::string::npos,
            "INPUT should honor SET FIELDS metadata");
        expect(input_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "INPUT should surface the current filter");
        expect(input_events[0].detail.find("prompt=") != std::string::npos,
            "INPUT event should include the prompt field");
        expect(input_events[0].detail.find("target=myvar") != std::string::npos,
            "INPUT event should include the target variable name");
        expect(input_events[0].detail.find("result=''") != std::string::npos,
            "INPUT event should surface the deterministic headless result");
    }
    expect(after_input != state.globals.end(), "INPUT should assign a deterministic headless result to the target");
    if (after_input != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_input->second).empty(),
            "INPUT should assign an empty-string headless result to the target variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_accept_command_emits_runtime_accept_event_with_prompt() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_accept_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "accept_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "ACCEPT \"Enter your name: \" TO username\n"
        "cAfterAccept = username\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ACCEPT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> accept_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.accept") {
            accept_events.push_back(event);
        }
    }

    expect(accept_events.size() == 1U, "ACCEPT command should emit one runtime.accept event");
    const auto after_accept = state.globals.find("cafteraccept");
    if (accept_events.size() >= 1U) {
        expect(accept_events[0].detail.find("people@") != std::string::npos,
            "ACCEPT should surface the selected cursor");
        expect(accept_events[0].detail.find("fields=NAME") != std::string::npos,
            "ACCEPT should honor SET FIELDS metadata");
        expect(accept_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "ACCEPT should surface the current filter");
        expect(accept_events[0].detail.find("prompt=") != std::string::npos,
            "ACCEPT event should include the prompt field");
        expect(accept_events[0].detail.find("target=username") != std::string::npos,
            "ACCEPT event should include the target variable name");
        expect(accept_events[0].detail.find("result=''") != std::string::npos,
            "ACCEPT event should surface the deterministic headless result");
    }
    expect(after_accept != state.globals.end(), "ACCEPT should assign a deterministic headless result to the target");
    if (after_accept != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_accept->second).empty(),
            "ACCEPT should assign an empty-string headless result to the target variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_input_accept_commands_surface_macro_prompt_and_target_detail() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_input_accept_macro_detail";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "input_accept_macro_detail.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "cInputPrompt = 'Enter code:'\n"
        "cInputTarget = 'cInputValue'\n"
        "cInputTargetHolder = 'cInputTarget'\n"
        "cInputTargetDeepHolder = 'cInputTargetHolder'\n"
        "cAcceptPrompt = 'Enter name:'\n"
        "cAcceptTarget = 'cAcceptValue'\n"
        "cAcceptTargetHolder = 'cAcceptTarget'\n"
        "cAcceptTargetDeepHolder = 'cAcceptTargetHolder'\n"
        "INPUT cInputPrompt TO &cInputTargetDeepHolder\n"
        "ACCEPT cAcceptPrompt TO &cAcceptTargetDeepHolder\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INPUT/ACCEPT macro detail script should complete");

    const copperfin::runtime::RuntimeEvent *input_event = nullptr;
    const copperfin::runtime::RuntimeEvent *accept_event = nullptr;
    for (const auto &event : state.events) {
        if (event.category == "runtime.input") {
            input_event = &event;
        } else if (event.category == "runtime.accept") {
            accept_event = &event;
        }
    }

    expect(input_event != nullptr, "INPUT macro detail script should emit runtime.input");
    expect(accept_event != nullptr, "ACCEPT macro detail script should emit runtime.accept");
    if (input_event != nullptr) {
        expect(input_event->detail.find("prompt=Enter code:") != std::string::npos,
            "INPUT should surface the resolved prompt text");
        expect(input_event->detail.find("prompt_expr=cInputPrompt") != std::string::npos,
            "INPUT should preserve the source prompt expression");
        expect(input_event->detail.find("target=&cInputTargetDeepHolder") != std::string::npos,
            "INPUT should retain the raw TO target expression");
        expect(input_event->detail.find("target_resolved=cInputValue") != std::string::npos,
            "INPUT should surface the second-hop resolved TO target name");
    }
    if (accept_event != nullptr) {
        expect(accept_event->detail.find("prompt=Enter name:") != std::string::npos,
            "ACCEPT should surface the resolved prompt text");
        expect(accept_event->detail.find("prompt_expr=cAcceptPrompt") != std::string::npos,
            "ACCEPT should preserve the source prompt expression");
        expect(accept_event->detail.find("target=&cAcceptTargetDeepHolder") != std::string::npos,
            "ACCEPT should retain the raw TO target expression");
        expect(accept_event->detail.find("target_resolved=cAcceptValue") != std::string::npos,
            "ACCEPT should surface the second-hop resolved TO target name");
    }

    const auto input_value = state.globals.find("cinputvalue");
    const auto accept_value = state.globals.find("cacceptvalue");
    expect(input_value != state.globals.end(), "INPUT macro target should be assigned");
    expect(accept_value != state.globals.end(), "ACCEPT macro target should be assigned");

    fs::remove_all(temp_root, ignored);
}

void test_input_accept_to_local_targets_stay_local_in_routine_scope() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_input_accept_local_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "input_accept_local_scope.prg";
    write_text(
        main_path,
        "PUBLIC input_local_type, accept_local_type, input_local_value, accept_local_value\n"
        "input_local_type = 'unset'\n"
        "accept_local_type = 'unset'\n"
        "DO run_dialogs\n"
        "after_input_global_type = TYPE('cInputLocal')\n"
        "after_accept_global_type = TYPE('cAcceptLocal')\n"
        "RETURN\n"
        "PROCEDURE run_dialogs\n"
        "LOCAL cInputLocal, cAcceptLocal\n"
        "PUBLIC input_local_type, accept_local_type, input_local_value, accept_local_value\n"
        "INPUT \"Enter input:\" TO cInputLocal\n"
        "ACCEPT \"Enter accept:\" TO cAcceptLocal\n"
        "input_local_type = TYPE('cInputLocal')\n"
        "accept_local_type = TYPE('cAcceptLocal')\n"
        "input_local_value = cInputLocal\n"
        "accept_local_value = cAcceptLocal\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(
                           make_runtime_session_options(main_path.string(), temp_root.string()))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "INPUT/ACCEPT LOCAL-scope script should complete");

    const auto input_local_type = state.globals.find("input_local_type");
    const auto accept_local_type = state.globals.find("accept_local_type");
    const auto input_local_value = state.globals.find("input_local_value");
    const auto accept_local_value = state.globals.find("accept_local_value");
    const auto after_input_global_type = state.globals.find("after_input_global_type");
    const auto after_accept_global_type = state.globals.find("after_accept_global_type");

    expect(input_local_type != state.globals.end(), "LOCAL INPUT type capture should exist");
    expect(accept_local_type != state.globals.end(), "LOCAL ACCEPT type capture should exist");
    expect(input_local_value != state.globals.end(), "LOCAL INPUT value capture should exist");
    expect(accept_local_value != state.globals.end(), "LOCAL ACCEPT value capture should exist");
    expect(after_input_global_type != state.globals.end(), "post-routine INPUT global TYPE() should exist");
    expect(after_accept_global_type != state.globals.end(), "post-routine ACCEPT global TYPE() should exist");

    if (input_local_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(input_local_type->second) == "C",
            "INPUT TO LOCAL target should stay character-typed inside routine scope");
    }
    if (accept_local_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(accept_local_type->second) == "C",
            "ACCEPT TO LOCAL target should stay character-typed inside routine scope");
    }
    if (input_local_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(input_local_value->second).empty(),
            "INPUT TO LOCAL target should receive deterministic empty-string headless result");
    }
    if (accept_local_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(accept_local_value->second).empty(),
            "ACCEPT TO LOCAL target should receive deterministic empty-string headless result");
    }
    if (after_input_global_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_input_global_type->second) == "U",
            "INPUT TO LOCAL target should not leak a global binding after routine return");
    }
    if (after_accept_global_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_accept_global_type->second) == "U",
            "ACCEPT TO LOCAL target should not leak a global binding after routine return");
    }

    fs::remove_all(temp_root, ignored);
}

void test_getfile_command_emits_runtime_getfile_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_getfile_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "getfile_test.prg";
    write_text(
        main_path,
        "GETFILE PROMPT \"Pick a file\" TITLE \"Open\" DEFAULT \"./data\" FILTER \"*.dbf\" TO lcPath\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GETFILE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.getfile") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "GETFILE command should emit one runtime.getfile event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "GETFILE event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "GETFILE event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "GETFILE event should include default detail");
        expect(dialog_events[0].detail.find("filter=") != std::string::npos,
            "GETFILE event should include filter detail");
        expect(dialog_events[0].detail.find("target=lcPath") != std::string::npos,
            "GETFILE event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "GETFILE event should include deterministic empty-string result detail");
    }

    const auto path_value = state.globals.find("lcpath");
    expect(path_value != state.globals.end(), "GETFILE TO target should be assigned");
    if (path_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_value->second).empty(),
            "GETFILE TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_putfile_command_emits_runtime_putfile_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_putfile_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "putfile_test.prg";
    write_text(
        main_path,
        "PUTFILE PROMPT \"Save as\" TITLE \"Save\" DEFAULT \"./output\" FILTER \"*.txt\" TO lcSavePath\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PUTFILE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.putfile") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "PUTFILE command should emit one runtime.putfile event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "PUTFILE event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "PUTFILE event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "PUTFILE event should include default detail");
        expect(dialog_events[0].detail.find("filter=") != std::string::npos,
            "PUTFILE event should include filter detail");
        expect(dialog_events[0].detail.find("target=lcSavePath") != std::string::npos,
            "PUTFILE event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "PUTFILE event should include deterministic empty-string result detail");
    }

    const auto save_path_value = state.globals.find("lcsavepath");
    expect(save_path_value != state.globals.end(), "PUTFILE TO target should be assigned");
    if (save_path_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(save_path_value->second).empty(),
            "PUTFILE TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_getdir_command_emits_runtime_getdir_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_getdir_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "getdir_test.prg";
    write_text(
        main_path,
        "GETDIR PROMPT \"Choose folder\" TITLE \"Browse\" DEFAULT \"./workspace\" TO lcDir\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GETDIR script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.getdir") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "GETDIR command should emit one runtime.getdir event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "GETDIR event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "GETDIR event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "GETDIR event should include default detail");
        expect(dialog_events[0].detail.find("target=lcDir") != std::string::npos,
            "GETDIR event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "GETDIR event should include deterministic empty-string result detail");
    }

    const auto dir_value = state.globals.find("lcdir");
    expect(dir_value != state.globals.end(), "GETDIR TO target should be assigned");
    if (dir_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(dir_value->second).empty(),
            "GETDIR TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_inputbox_command_emits_runtime_inputbox_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_inputbox_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "inputbox_test.prg";
    write_text(
        main_path,
        "INPUTBOX PROMPT \"Enter value\" TITLE \"Question\" DEFAULT \"42\" TO lcAnswer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INPUTBOX script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.inputbox") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "INPUTBOX command should emit one runtime.inputbox event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "INPUTBOX event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "INPUTBOX event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "INPUTBOX event should include default detail");
        expect(dialog_events[0].detail.find("target=lcAnswer") != std::string::npos,
            "INPUTBOX event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "INPUTBOX event should include deterministic empty-string result detail");
    }

    const auto answer_value = state.globals.find("lcanswer");
    expect(answer_value != state.globals.end(), "INPUTBOX TO target should be assigned");
    if (answer_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(answer_value->second).empty(),
            "INPUTBOX TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dialog_commands_parenthesized_forms_assign_targets_and_extract_positional_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dialog_parenthesized";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "dialog_parenthesized_test.prg";
    write_text(
        main_path,
        "LOCAL lcLocalFile\n"
        "GETFILE(\"*.dbf\", \"Pick a file\", \"Open\", \"./data\") TO lcLocalFile\n"
        "cLocalFileResult = lcLocalFile\n"
        "PUTFILE(\"*.txt\", \"Save as\", \"Save\", \"./out\") TO cPut\n"
        "GETDIR(\"./workspace\", \"Choose folder\", \"Browse\") TO m.cMemDir\n"
        "INPUTBOX(\"Enter value\", \"Question\", \"42\") TO cAnswer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "parenthesized dialog script should complete");

    const auto local_file_result = state.globals.find("clocalfileresult");
    const auto put_result = state.globals.find("cput");
    const auto mem_dir_result = state.globals.find("cmemdir");
    const auto answer_result = state.globals.find("canswer");
    expect(local_file_result != state.globals.end(), "GETFILE TO LOCAL target should be assignable and readable in-scope");
    expect(put_result != state.globals.end(), "PUTFILE TO target should be assigned");
    expect(mem_dir_result != state.globals.end(), "GETDIR TO m.<var> target should be assigned through memory-variable path");
    expect(answer_result != state.globals.end(), "INPUTBOX TO target should be assigned");

    if (local_file_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(local_file_result->second).empty(),
            "GETFILE parenthesized TO LOCAL target should default to empty string");
    }
    if (put_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(put_result->second).empty(),
            "PUTFILE parenthesized TO target should default to empty string");
    }
    if (mem_dir_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(mem_dir_result->second).empty(),
            "GETDIR parenthesized TO m.<var> target should default to empty string");
    }
    if (answer_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(answer_result->second).empty(),
            "INPUTBOX parenthesized TO target should default to empty string");
    }

    const auto find_event = [&](const std::string& category) -> const copperfin::runtime::RuntimeEvent* {
        for (const auto& event : state.events) {
            if (event.category == category) {
                return &event;
            }
        }
        return nullptr;
    };

    const auto* getfile_event = find_event("runtime.getfile");
    const auto* putfile_event = find_event("runtime.putfile");
    const auto* getdir_event = find_event("runtime.getdir");
    const auto* inputbox_event = find_event("runtime.inputbox");
    expect(getfile_event != nullptr, "GETFILE parenthesized call should emit runtime.getfile event");
    expect(putfile_event != nullptr, "PUTFILE parenthesized call should emit runtime.putfile event");
    expect(getdir_event != nullptr, "GETDIR parenthesized call should emit runtime.getdir event");
    expect(inputbox_event != nullptr, "INPUTBOX parenthesized call should emit runtime.inputbox event");

    if (getfile_event != nullptr) {
        expect(getfile_event->detail.find("filter=\"*.dbf\"") != std::string::npos,
            "GETFILE positional extraction should map first argument to filter");
        expect(getfile_event->detail.find("prompt=\"Pick a file\"") != std::string::npos,
            "GETFILE positional extraction should map second argument to prompt");
        expect(getfile_event->detail.find("title=\"Open\"") != std::string::npos,
            "GETFILE positional extraction should map third argument to title");
        expect(getfile_event->detail.find("default=\"./data\"") != std::string::npos,
            "GETFILE positional extraction should map fourth argument to default");
        expect(getfile_event->detail.find("target=lcLocalFile") != std::string::npos,
            "GETFILE parenthesized event should retain TO target detail");
    }
    if (putfile_event != nullptr) {
        expect(putfile_event->detail.find("filter=\"*.txt\"") != std::string::npos,
            "PUTFILE positional extraction should map first argument to filter");
        expect(putfile_event->detail.find("prompt=\"Save as\"") != std::string::npos,
            "PUTFILE positional extraction should map second argument to prompt");
        expect(putfile_event->detail.find("title=\"Save\"") != std::string::npos,
            "PUTFILE positional extraction should map third argument to title");
        expect(putfile_event->detail.find("default=\"./out\"") != std::string::npos,
            "PUTFILE positional extraction should map fourth argument to default");
        expect(putfile_event->detail.find("target=cPut") != std::string::npos,
            "PUTFILE parenthesized event should retain TO target detail");
    }
    if (getdir_event != nullptr) {
        expect(getdir_event->detail.find("default=\"./workspace\"") != std::string::npos,
            "GETDIR positional extraction should map first argument to default");
        expect(getdir_event->detail.find("prompt=\"Choose folder\"") != std::string::npos,
            "GETDIR positional extraction should map second argument to prompt");
        expect(getdir_event->detail.find("title=\"Browse\"") != std::string::npos,
            "GETDIR positional extraction should map third argument to title");
        expect(getdir_event->detail.find("target=m.cMemDir") != std::string::npos,
            "GETDIR parenthesized event should retain TO target detail");
    }
    if (inputbox_event != nullptr) {
        expect(inputbox_event->detail.find("prompt=\"Enter value\"") != std::string::npos,
            "INPUTBOX positional extraction should map first argument to prompt");
        expect(inputbox_event->detail.find("title=\"Question\"") != std::string::npos,
            "INPUTBOX positional extraction should map second argument to title");
        expect(inputbox_event->detail.find("default=\"42\"") != std::string::npos,
            "INPUTBOX positional extraction should map third argument to default");
        expect(inputbox_event->detail.find("target=cAnswer") != std::string::npos,
            "INPUTBOX parenthesized event should retain TO target detail");
    }

    fs::remove_all(temp_root, ignored);
}

void test_wait_window_command_emits_runtime_wait_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_wait_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "wait_test.prg";
    write_text(
        main_path,
        "cPrompt = \"Please wait...\"\n"
        "nDelay = 5\n"
        "cWaitTarget = \"m.wResult\"\n"
        "cWaitTargetHolder = \"cWaitTarget\"\n"
        "cWaitTargetDeepHolder = \"cWaitTargetHolder\"\n"
        "WAIT WINDOW cPrompt TIMEOUT nDelay TO &cWaitTargetDeepHolder NOWAIT NOCLEAR\n"
        "cAfterWait = m.wResult\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WAIT WINDOW script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> wait_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.wait") {
            wait_events.push_back(event);
        }
    }

    expect(wait_events.size() == 1U, "WAIT WINDOW command should emit one runtime.wait event");
    const auto after_wait = state.globals.find("cafterwait");
    if (wait_events.size() >= 1U) {
        expect(wait_events[0].detail.find("mode=WINDOW") != std::string::npos,
            "WAIT WINDOW event should report mode=WINDOW");
        expect(wait_events[0].detail.find("prompt=Please wait...") != std::string::npos,
            "WAIT WINDOW event should surface the resolved prompt text");
        expect(wait_events[0].detail.find("prompt_expr=cPrompt") != std::string::npos,
            "WAIT WINDOW event should preserve the source prompt expression");
        expect(wait_events[0].detail.find("timeout=5") != std::string::npos,
            "WAIT WINDOW event should surface the resolved timeout expression");
        expect(wait_events[0].detail.find("timeout_expr=nDelay") != std::string::npos,
            "WAIT WINDOW event should preserve the source timeout expression");
        expect(wait_events[0].detail.find("flag=NOWAIT") != std::string::npos,
            "WAIT WINDOW event should surface the NOWAIT flag");
        expect(wait_events[0].detail.find("flag=NOCLEAR") != std::string::npos,
            "WAIT WINDOW event should surface the NOCLEAR flag");
        expect(wait_events[0].detail.find("target=&cWaitTargetDeepHolder") != std::string::npos,
            "WAIT WINDOW event should include the raw TO target expression");
        expect(wait_events[0].detail.find("target_resolved=m.wResult") != std::string::npos,
            "WAIT WINDOW event should include the second-hop resolved TO target");
        expect(wait_events[0].detail.find("result=''") != std::string::npos,
            "WAIT WINDOW event should surface the deterministic headless result");
    }
    expect(after_wait != state.globals.end(), "WAIT WINDOW TO target should be assigned");
    if (after_wait != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_wait->second).empty(),
            "WAIT WINDOW TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_wait_clear_command_emits_runtime_wait_clear_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_wait_clear_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "wait_clear_test.prg";
    write_text(
        main_path,
        "WAIT CLEAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WAIT CLEAR script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> wait_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.wait") {
            wait_events.push_back(event);
        }
    }

    expect(wait_events.size() == 1U, "WAIT CLEAR command should emit one runtime.wait event");
    if (wait_events.size() >= 1U) {
        expect(wait_events[0].detail.find("mode=CLEAR") != std::string::npos,
            "WAIT CLEAR event should report mode=CLEAR");
    }

    fs::remove_all(temp_root, ignored);
}

void test_keyboard_command_emits_runtime_keyboard_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_keyboard_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "keyboard_test.prg";
    write_text(
        main_path,
        "cKeys = \"ABC\"\n"
        "KEYBOARD cKeys PLAIN CLEAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "KEYBOARD script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> kb_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.keyboard") {
            kb_events.push_back(event);
        }
    }

    expect(kb_events.size() == 1U, "KEYBOARD command should emit one runtime.keyboard event");
    if (kb_events.size() >= 1U) {
        expect(kb_events[0].detail.find("keys=ABC") != std::string::npos,
            "KEYBOARD event should include the resolved keys payload");
        expect(kb_events[0].detail.find("keys_expr=cKeys") != std::string::npos,
            "KEYBOARD event should preserve the source key expression");
        expect(kb_events[0].detail.find("flag=PLAIN") != std::string::npos,
            "KEYBOARD event should include the PLAIN flag");
        expect(kb_events[0].detail.find("flag=CLEAR") != std::string::npos,
            "KEYBOARD event should include the CLEAR flag");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io
