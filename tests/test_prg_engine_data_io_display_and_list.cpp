// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_prg_engine_data_io_support.h"

namespace cf_test_prg_engine_data_io {
void test_browse_emits_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_browse";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});
    write_people_dbf(temp_root / "other.dbf", {{"Carol", 55}});

    const fs::path main_path = temp_root / "browse.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "USE '" + (temp_root / "other.dbf").string() + "' ALIAS other AGAIN IN 0\n"
        "SELECT people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "BROWSE\n"
        "BROWSE IN other FIELDS AGE FOR AGE > 40\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "BROWSE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> browse_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.browse") {
            browse_events.push_back(event);
        }
    }

    expect(browse_events.size() == 2U, "BROWSE commands should emit two runtime.browse events");
    if (browse_events.size() >= 2U) {
        expect(browse_events[0].detail.find("people@") != std::string::npos,
            "default BROWSE should target the selected cursor");
        expect(browse_events[0].detail.find("fields=NAME") != std::string::npos,
            "default BROWSE should honor SET FIELDS state");
        expect(browse_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "default BROWSE should surface the current cursor filter");

        expect(browse_events[1].detail.find("other@") != std::string::npos,
            "targeted BROWSE IN should surface the requested cursor");
        expect(browse_events[1].detail.find("fields=AGE") != std::string::npos,
            "BROWSE FIELDS should override SET FIELDS for the event payload");
        expect(browse_events[1].detail.find("for=AGE > 40") != std::string::npos,
            "BROWSE FOR should surface the inline filter expression");
    }

    fs::remove_all(temp_root, ignored);
}

void test_browse_like_and_except_field_filters_surface_event_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_browse_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "BROWSE LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "browse_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "SET FIELDS TO LIKE N*\n"
        "BROWSE\n"
        "BROWSE FIELDS EXCEPT NOTE FOR AGE >= 25\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "BROWSE LIKE/EXCEPT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> browse_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.browse") {
            browse_events.push_back(event);
        }
    }

    expect(browse_events.size() == 2U, "BROWSE LIKE/EXCEPT commands should emit two runtime.browse events");
    if (browse_events.size() >= 2U) {
        expect(browse_events[0].detail.find("fields=NAME,NOTE") != std::string::npos,
            "SET FIELDS TO LIKE N* should surface NAME and NOTE in browse metadata");
        expect(browse_events[1].detail.find("fields=NAME,AGE") != std::string::npos,
            "BROWSE FIELDS EXCEPT NOTE should exclude NOTE in browse metadata");
        expect(browse_events[1].detail.find("for=AGE >= 25") != std::string::npos,
            "BROWSE FIELDS EXCEPT NOTE should preserve the FOR clause in metadata");
    }

    fs::remove_all(temp_root, ignored);
}

void test_browse_nowait_remains_a_clause_boundary() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_browse_nowait";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});

    const fs::path main_path = temp_root / "browse_nowait.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "BROWSE FIELDS NAME, AGE NOWAIT IN people\n"
        "BROWSE IN people NOWAIT FIELDS NAME, AGE\n"
        "BROWSE IN people FIELDS NAME, AGE FOR AGE > 25 NOWAIT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "BROWSE NOWAIT clause-boundary script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> browse_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.browse") {
            browse_events.push_back(event);
        }
    }

    expect(browse_events.size() == 3U, "BROWSE NOWAIT commands should emit three runtime.browse events");
    for (const auto &event : browse_events) {
        expect(event.detail.find("people@") != std::string::npos,
               "NOWAIT should not become part of the BROWSE IN target");
        expect(event.detail.find("fields=NAME,AGE") != std::string::npos,
               "NOWAIT should not become part of the final BROWSE FIELDS entry");
    }
    if (browse_events.size() >= 3U) {
        expect(browse_events[2].detail.find("for=AGE > 25") != std::string::npos,
               "NOWAIT should not become part of the BROWSE FOR expression");
        expect(browse_events[2].detail.find("for=AGE > 25 NOWAIT") == std::string::npos,
               "BROWSE FOR metadata should exclude the independent NOWAIT clause");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_structure_emits_runtime_display_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "display_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "DISPLAY STRUCTURE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY STRUCTURE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY STRUCTURE command should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=STRUCTURE") != std::string::npos,
            "DISPLAY STRUCTURE event should report mode=STRUCTURE");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY STRUCTURE should surface the selected cursor");
        expect(display_events[0].detail.find("field_count=2") != std::string::npos,
            "DISPLAY STRUCTURE should surface the schema field count");
        expect(display_events[0].detail.find("schema_fields=NAME,AGE") != std::string::npos,
            "DISPLAY STRUCTURE should surface schema field names");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_status_surfaces_session_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "display_status_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "DISPLAY STATUS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY STATUS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY STATUS should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=STATUS") != std::string::npos,
            "DISPLAY STATUS event should report mode=STATUS");
        expect(display_events[0].detail.find("datasession=1") != std::string::npos,
            "DISPLAY STATUS should report the current data session");
        expect(display_events[0].detail.find("open_cursors=1") != std::string::npos,
            "DISPLAY STATUS should report open cursor count");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY STATUS should surface the selected cursor");
        expect(display_events[0].detail.find("fields=NAME") != std::string::npos,
            "DISPLAY STATUS should honor current visible-field metadata");
        expect(display_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "DISPLAY STATUS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_memory_surfaces_visible_variable_and_array_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_memory";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "display_memory_test.prg";
    write_text(
        main_path,
        "PUBLIC cPublic\n"
        "cPublic = \"pub\"\n"
        "cGlobal = \"glob\"\n"
        "PRIVATE cPrivate\n"
        "cPrivate = \"priv\"\n"
        "LOCAL nLocal\n"
        "nLocal = 42\n"
        "LOCAL cPublic\n"
        "cPublic = \"localpub\"\n"
        "DIMENSION aMemory(2,2)\n"
        "aMemory[1,1] = \"x\"\n"
        "oThing = CREATEOBJECT('Empty')\n"
        "DISPLAY MEMORY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY MEMORY script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY MEMORY should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=MEMORY") != std::string::npos,
            "DISPLAY MEMORY event should report mode=MEMORY");
        expect(display_events[0].detail.find("memvar_count=5") != std::string::npos,
            "DISPLAY MEMORY should report visible memory-variable count");
        expect(display_events[0].detail.find("public_count=0") != std::string::npos,
            "DISPLAY MEMORY should report public variable count");
        expect(display_events[0].detail.find("private_count=1") != std::string::npos,
            "DISPLAY MEMORY should report private variable count");
        expect(display_events[0].detail.find("local_count=2") != std::string::npos,
            "DISPLAY MEMORY should report local variable count");
        expect(display_events[0].detail.find("global_count=2") != std::string::npos,
            "DISPLAY MEMORY should report ordinary global variable count");
        expect(display_events[0].detail.find("array_count=1") != std::string::npos,
            "DISPLAY MEMORY should report visible runtime array count");
        expect(display_events[0].detail.find("cpublic{local:C=localpub}") != std::string::npos,
            "DISPLAY MEMORY should include the visible shadowing local value");
        expect(display_events[0].detail.find("cprivate{private:C=priv}") != std::string::npos,
            "DISPLAY MEMORY should include private variable scope/type/value detail");
        expect(display_events[0].detail.find("nlocal{local:N=42}") != std::string::npos,
            "DISPLAY MEMORY should include local variable scope/type/value detail");
        expect(display_events[0].detail.find("cglobal{global:C=glob}") != std::string::npos,
            "DISPLAY MEMORY should include global variable scope/type/value detail");
        expect(display_events[0].detail.find("othing{global:O=<object:Empty props=0>}") != std::string::npos,
            "DISPLAY MEMORY should include object scope/type/detail");
        expect(display_events[0].detail.find("shadowed=cpublic{public:C=pub}") != std::string::npos,
            "DISPLAY MEMORY should include shadowed public bindings");
        expect(display_events[0].detail.find("amemory{global:A=2x2}") != std::string::npos,
            "DISPLAY MEMORY should include runtime array scope and dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_memory_hides_internal_application_surfaces() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_memory_internal_surfaces";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "display_memory_internal_surfaces.prg";
    write_text(
        main_path,
        "cCaption = _SCREEN.Caption\n"
        "DISPLAY MEMORY\n"
        "LIST MEMORY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY MEMORY internal-surface script should complete");

    const auto caption = state.globals.find("ccaption");
    expect(caption != state.globals.end() &&
               copperfin::runtime::format_value(caption->second) == "Microsoft Visual FoxPro",
           "internal application surface property access should remain available");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }
    expect(display_events.size() == 1U,
           "internal-surface DISPLAY MEMORY should emit one runtime.display event");
    if (display_events.size() == 1U) {
        expect(display_events[0].detail.find("memvar_count=1") != std::string::npos,
               "internal application surfaces should not count as memory variables");
        expect(display_events[0].detail.find("global_count=1") != std::string::npos,
               "internal application surfaces should not count as ordinary globals");
        expect(display_events[0].detail.find("_screen") == std::string::npos &&
                   display_events[0].detail.find("_vfp") == std::string::npos &&
                   display_events[0].detail.find("application") == std::string::npos,
               "internal application surface bindings should not be listed");
    }

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }
    expect(list_events.size() == 1U,
           "internal-surface LIST MEMORY should emit one runtime.list event");
    if (list_events.size() == 1U) {
        expect(list_events[0].detail.find("memvar_count=1") != std::string::npos,
               "LIST MEMORY should not count internal application surfaces");
        expect(list_events[0].detail.find("global_count=1") != std::string::npos,
               "LIST MEMORY should not count internal application globals");
        expect(list_events[0].detail.find("_screen") == std::string::npos &&
                   list_events[0].detail.find("_vfp") == std::string::npos &&
                   list_events[0].detail.find("application") == std::string::npos,
               "LIST MEMORY should not list internal application surface bindings");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_records_surfaces_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_records";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "DISPLAY RECORDS fixture should be created");

    const fs::path main_path = temp_root / "display_records.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO LIKE N*\n"
        "DISPLAY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY RECORDS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY RECORDS should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=RECORDS") != std::string::npos,
            "DISPLAY RECORDS event should report mode=RECORDS");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY RECORDS event should surface the selected cursor");
        expect(display_events[0].detail.find("fields=NAME,NOTE") != std::string::npos,
            "DISPLAY RECORDS should honor SET FIELDS LIKE metadata");
        expect(display_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "DISPLAY RECORDS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_and_list_records_surface_resolved_in_target_detail() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_list_target_detail";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 22}});

    const fs::path main_path = temp_root / "display_list_target_detail.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "cAlias = 'people'\n"
        "cAliasHolder = 'cAlias'\n"
        "cAliasDeepHolder = 'cAliasHolder'\n"
        "DISPLAY IN &cAliasDeepHolder FIELDS NAME FOR AGE >= 25 WHILE AGE < 40\n"
        "LIST IN &cAliasDeepHolder FIELDS EXCEPT AGE FOR AGE >= 20 WHILE AGE < 40\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY/LIST target detail script should complete");

    const copperfin::runtime::RuntimeEvent *display_event = nullptr;
    const copperfin::runtime::RuntimeEvent *list_event = nullptr;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_event = &event;
        } else if (event.category == "runtime.list") {
            list_event = &event;
        }
    }

    expect(display_event != nullptr, "DISPLAY target detail script should emit runtime.display");
    expect(list_event != nullptr, "LIST target detail script should emit runtime.list");
    if (display_event != nullptr) {
        expect(display_event->detail.find("target=&cAliasDeepHolder") != std::string::npos,
            "DISPLAY RECORDS should retain the raw IN target expression");
        expect(display_event->detail.find("target_resolved=people") != std::string::npos,
            "DISPLAY RECORDS should surface the second-hop resolved IN target");
        expect(display_event->detail.find("fields=NAME") != std::string::npos,
            "DISPLAY RECORDS should preserve inline field metadata with macro IN");
        expect(display_event->detail.find("for=AGE >= 25") != std::string::npos,
            "DISPLAY RECORDS should preserve the FOR clause with macro IN");
        expect(display_event->detail.find("while=AGE < 40") != std::string::npos,
            "DISPLAY RECORDS should preserve the WHILE clause with macro IN");
    }
    if (list_event != nullptr) {
        expect(list_event->detail.find("target=&cAliasDeepHolder") != std::string::npos,
            "LIST RECORDS should retain the raw IN target expression");
        expect(list_event->detail.find("target_resolved=people") != std::string::npos,
            "LIST RECORDS should surface the second-hop resolved IN target");
        expect(list_event->detail.find("fields=NAME") != std::string::npos,
            "LIST RECORDS should preserve inline field metadata with macro IN");
        expect(list_event->detail.find("for=AGE >= 20") != std::string::npos,
            "LIST RECORDS should preserve the FOR clause with macro IN");
        expect(list_event->detail.find("while=AGE < 40") != std::string::npos,
            "LIST RECORDS should preserve the WHILE clause with macro IN");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_and_list_structure_surface_target_detail() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_list_structure_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "display_list_structure_target.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people IN 0\n"
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS other AGAIN IN 0\n"
        "cAlias = 'other'\n"
        "cAliasHolder = 'cAlias'\n"
        "cAliasDeepHolder = 'cAliasHolder'\n"
        "DISPLAY STRUCTURE IN &cAliasDeepHolder\n"
        "LIST STRUCTURE IN &cAliasDeepHolder\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY/LIST STRUCTURE target-detail script should complete");

    const copperfin::runtime::RuntimeEvent *display_event = nullptr;
    const copperfin::runtime::RuntimeEvent *list_event = nullptr;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_event = &event;
        } else if (event.category == "runtime.list") {
            list_event = &event;
        }
    }

    expect(display_event != nullptr, "DISPLAY STRUCTURE target-detail script should emit runtime.display");
    expect(list_event != nullptr, "LIST STRUCTURE target-detail script should emit runtime.list");
    if (display_event != nullptr) {
        expect(display_event->detail.find("mode=STRUCTURE") != std::string::npos,
            "DISPLAY STRUCTURE target-detail event should report mode=STRUCTURE");
        expect(display_event->detail.find("other@") != std::string::npos,
            "DISPLAY STRUCTURE should surface the targeted cursor");
        expect(display_event->detail.find("target=&cAliasDeepHolder") != std::string::npos,
            "DISPLAY STRUCTURE should retain the raw IN target expression");
        expect(display_event->detail.find("target_resolved=other") != std::string::npos,
            "DISPLAY STRUCTURE should surface the resolved IN target");
    }
    if (list_event != nullptr) {
        expect(list_event->detail.find("mode=STRUCTURE") != std::string::npos,
            "LIST STRUCTURE target-detail event should report mode=STRUCTURE");
        expect(list_event->detail.find("other@") != std::string::npos,
            "LIST STRUCTURE should surface the targeted cursor");
        expect(list_event->detail.find("target=&cAliasDeepHolder") != std::string::npos,
            "LIST STRUCTURE should retain the raw IN target expression");
        expect(list_event->detail.find("target_resolved=other") != std::string::npos,
            "LIST STRUCTURE should surface the resolved IN target");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_status_emits_runtime_list_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "list_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "LIST STATUS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST STATUS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST STATUS command should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=STATUS") != std::string::npos,
            "LIST STATUS event should report mode=STATUS");
        expect(list_events[0].detail.find("datasession=1") != std::string::npos,
            "LIST STATUS should report the current data session");
        expect(list_events[0].detail.find("open_cursors=1") != std::string::npos,
            "LIST STATUS should report open cursor count");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST STATUS should surface the selected cursor");
        expect(list_events[0].detail.find("fields=NAME") != std::string::npos,
            "LIST STATUS should honor current visible-field metadata");
        expect(list_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "LIST STATUS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_memory_surfaces_visible_variable_and_array_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_memory";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "list_memory_test.prg";
    write_text(
        main_path,
        "PUBLIC cPublic\n"
        "cPublic = \"pub\"\n"
        "cGlobal = \"glob\"\n"
        "PRIVATE cPrivate\n"
        "cPrivate = \"priv\"\n"
        "LOCAL nLocal\n"
        "nLocal = 42\n"
        "LOCAL cPublic\n"
        "cPublic = \"localpub\"\n"
        "DIMENSION aMemory(2,2)\n"
        "aMemory[1,1] = \"x\"\n"
        "oThing = CREATEOBJECT('Empty')\n"
        "LIST MEMORY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST MEMORY script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST MEMORY should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=MEMORY") != std::string::npos,
            "LIST MEMORY event should report mode=MEMORY");
        expect(list_events[0].detail.find("memvar_count=5") != std::string::npos,
            "LIST MEMORY should report visible memory-variable count");
        expect(list_events[0].detail.find("public_count=0") != std::string::npos,
            "LIST MEMORY should report public variable count");
        expect(list_events[0].detail.find("private_count=1") != std::string::npos,
            "LIST MEMORY should report private variable count");
        expect(list_events[0].detail.find("local_count=2") != std::string::npos,
            "LIST MEMORY should report local variable count");
        expect(list_events[0].detail.find("global_count=2") != std::string::npos,
            "LIST MEMORY should report ordinary global variable count");
        expect(list_events[0].detail.find("array_count=1") != std::string::npos,
            "LIST MEMORY should report visible runtime array count");
        expect(list_events[0].detail.find("cpublic{local:C=localpub}") != std::string::npos,
            "LIST MEMORY should include the visible shadowing local value");
        expect(list_events[0].detail.find("cprivate{private:C=priv}") != std::string::npos,
            "LIST MEMORY should include private variable scope/type/value detail");
        expect(list_events[0].detail.find("nlocal{local:N=42}") != std::string::npos,
            "LIST MEMORY should include local variable scope/type/value detail");
        expect(list_events[0].detail.find("cglobal{global:C=glob}") != std::string::npos,
            "LIST MEMORY should include global variable scope/type/value detail");
        expect(list_events[0].detail.find("othing{global:O=<object:Empty props=0>}") != std::string::npos,
            "LIST MEMORY should include object scope/type/detail");
        expect(list_events[0].detail.find("shadowed=cpublic{public:C=pub}") != std::string::npos,
            "LIST MEMORY should include shadowed public bindings");
        expect(list_events[0].detail.find("amemory{global:A=2x2}") != std::string::npos,
            "LIST MEMORY should include runtime array scope and dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_list_memory_like_except_filter_applies_to_output() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_memory_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "display_memory_filter_test.prg";
    write_text(
        main_path,
        "cAlpha = 'alpha'\n"
        "cBeta = 'beta'\n"
        "nGamma = 3\n"
        "DIMENSION aAlpha(2)\n"
        "aAlpha[1] = 1\n"
        "DIMENSION aBeta(3)\n"
        "aBeta[1] = 2\n"
        "* LIKE c* should show cAlpha and cBeta but not nGamma\n"
        "DISPLAY MEMORY LIKE c*\n"
        "* EXCEPT c* should show nGamma but not cAlpha or cBeta\n"
        "LIST MEMORY EXCEPT c*\n"
        "* LIKE *alpha should include calpha and aalpha but not cbeta or abeta\n"
        "DISPLAY MEMORY LIKE *alpha\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#392: DISPLAY/LIST MEMORY LIKE/EXCEPT filter script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") { display_events.push_back(event); }
        if (event.category == "runtime.list") { list_events.push_back(event); }
    }

    expect(display_events.size() == 2U, "#392: should emit two runtime.display events");
    expect(list_events.size() == 1U, "#392: should emit one runtime.list event");

    if (display_events.size() >= 1U) {
        const auto &d0 = display_events[0].detail;
        expect(d0.find("clause=LIKE c*") != std::string::npos,
               "#392: DISPLAY MEMORY LIKE c* event should record the filter clause");
        expect(d0.find("calpha{global:C=alpha}") != std::string::npos,
               "#392: LIKE c* should include calpha");
        expect(d0.find("cbeta{global:C=beta}") != std::string::npos,
               "#392: LIKE c* should include cbeta");
        expect(d0.find("ngamma") == std::string::npos,
               "#392: LIKE c* should exclude ngamma");
        expect(d0.find("aalpha") == std::string::npos,
               "#392: LIKE c* should exclude arrays not matching the pattern");
    }
    if (list_events.size() >= 1U) {
        const auto &l0 = list_events[0].detail;
        expect(l0.find("clause=EXCEPT c*") != std::string::npos,
               "#392: LIST MEMORY EXCEPT c* event should record the filter clause");
        expect(l0.find("ngamma{global:N=3}") != std::string::npos,
               "#392: EXCEPT c* should include ngamma");
        expect(l0.find("calpha") == std::string::npos,
               "#392: EXCEPT c* should exclude calpha");
        expect(l0.find("cbeta") == std::string::npos,
               "#392: EXCEPT c* should exclude cbeta");
    }
    if (display_events.size() >= 2U) {
        const auto &d1 = display_events[1].detail;
        expect(d1.find("clause=LIKE *alpha") != std::string::npos,
               "#392: DISPLAY MEMORY LIKE *alpha event should record the filter clause");
        expect(d1.find("aalpha{global:A=2x1}") != std::string::npos,
               "#392: LIKE *alpha should include aalpha array");
        expect(d1.find("calpha{global:C=alpha}") != std::string::npos,
               "#392: LIKE *alpha should include calpha variable");
        expect(d1.find("abeta") == std::string::npos,
               "#392: LIKE *alpha should exclude abeta array");
        expect(d1.find("cbeta") == std::string::npos,
               "#392: LIKE *alpha should exclude cbeta variable");
        expect(d1.find("ngamma") == std::string::npos,
               "#392: LIKE *alpha should exclude ngamma variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_structure_surfaces_selected_cursor_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_structure";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "list_structure_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "LIST STRUCTURE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST STRUCTURE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST STRUCTURE should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=STRUCTURE") != std::string::npos,
            "LIST STRUCTURE event should report mode=STRUCTURE");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST STRUCTURE should surface the selected cursor");
        expect(list_events[0].detail.find("field_count=2") != std::string::npos,
            "LIST STRUCTURE should surface the schema field count");
        expect(list_events[0].detail.find("schema_fields=NAME,AGE") != std::string::npos,
            "LIST STRUCTURE should surface schema field names");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_records_surfaces_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_records";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "LIST RECORDS fixture should be created");

    const fs::path main_path = temp_root / "list_records.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "LIST FIELDS EXCEPT NOTE FOR AGE >= 25\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST RECORDS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST RECORDS should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=RECORDS") != std::string::npos,
            "LIST RECORDS event should report mode=RECORDS");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST RECORDS event should surface the selected cursor");
        expect(list_events[0].detail.find("fields=NAME,AGE") != std::string::npos,
            "LIST RECORDS should honor inline FIELDS EXCEPT metadata");
        expect(list_events[0].detail.find("for=AGE >= 25") != std::string::npos,
            "LIST RECORDS should surface the FOR clause");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io
