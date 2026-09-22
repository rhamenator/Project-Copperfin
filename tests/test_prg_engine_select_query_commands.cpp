#include "test_prg_engine_control_flow_support.h"

namespace cf_test_prg_engine_control_flow {

void test_select_query_into_array_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_select_query_into_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path search_path = temp_root / "searchrows.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> search_fields{
        {.name = "REFTYPE", .type = 'C', .length = 1U},
        {.name = "INACTIVE", .type = 'L', .length = 1U}};
    const auto search_result = copperfin::vfp::create_dbf_table_file(
        search_path.string(), search_fields, {{"S", "F"}, {"S", "T"}, {"P", "F"}});
    expect(search_result.ok, "direct SELECT search DBF fixture should be created");

    const fs::path other_path = temp_root / "otherrows.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> other_fields{
        {.name = "MARKER", .type = 'C', .length = 8U}};
    const auto other_result = copperfin::vfp::create_dbf_table_file(
        other_path.string(), other_fields, {{"before"}});
    expect(other_result.ok, "direct SELECT other DBF fixture should be created");

    const fs::path main_path = temp_root / "select_query.prg";
    write_text(
        main_path,
        "USE '" + search_path.string() + "' ALIAS SearchRows IN 0\n"
        "USE '" + other_path.string() + "' ALIAS OtherRows AGAIN IN 0\n"
        "SELECT OtherRows\n"
        "cBefore = ALIAS()\n"
        "SELECT CNT(*) FROM SearchRows WHERE RefType == 'S' AND !Inactive INTO ARRAY aSearchCnt\n"
        "cAfter = ALIAS()\n"
        "nSearchCount = aSearchCnt[1]\n"
        "nTally = _TALLY\n"
        "oHost = CREATEOBJECT('QueryHost')\n"
        "oHost.RefTable = '" + search_path.string() + "'\n"
        "nObjectCount = oHost.CountSearch()\n"
        "RETURN\n"
        "DEFINE CLASS QueryHost AS Session\n"
        "    RefTable = 'SearchRows'\n"
        "    FUNCTION CountSearch\n"
        "        LOCAL ARRAY aCounts[1]\n"
        "        SELECT CNT(*) FROM (THIS.RefTable) WHERE RefType == 'S' AND !Inactive INTO ARRAY aCounts\n"
        "        RETURN IIF(_TALLY > 0, aCounts[1], -1)\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "direct SELECT INTO ARRAY query script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("cbefore", "OtherRows");
    check("cafter", "OtherRows");
    check("nsearchcount", "1");
    check("ntally", "1");
    check("nobjectcount", "1");

    const auto query_event = std::find_if(
        state.events.begin(),
        state.events.end(),
        [](const auto &event) { return event.category == "runtime.select_query"; });
    expect(query_event != state.events.end(), "direct SELECT INTO ARRAY should emit query metadata");
    if (query_event != state.events.end()) {
        expect(query_event->detail == "aSearchCnt",
               "direct SELECT INTO ARRAY metadata should identify the target array");
    }

    fs::remove_all(temp_root, ignored);
}

// #6251: a WHERE-clause callback can close the SELECT's own source cursor
// during row materialization. Verifies the command fails catchably instead
// of continuing through freed CursorState -- USED() confirms genuine
// closure and _TALLY is not published as if the query had succeeded.
void test_select_where_expression_closing_source_cursor_fails_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_select_where_closes_source";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "select_where_closes_source.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT CNT(*) FROM People INTO ARRAY prior\n"
        "beforeTally = _TALLY\n"
        "DIMENSION result[1]\n"
        "result[1] = 'SENTINEL'\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SELECT NAME FROM People WHERE dropcursor() INTO ARRAY result\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "afterTally = _TALLY\n"
        "resultPreserved = result[1] == 'SENTINEL' AND ALEN(result, 1) == 1\n"
        "lStillOpen = USED('People')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION dropcursor\n"
        "USE IN People\n"
        "RETURN .T.\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6251: SELECT whose WHERE clause closes its own source cursor should complete without "
           "crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6251: SELECT must raise a catchable error instead of continuing through the closed source");

    const auto still_open_it = state.globals.find("lstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6251: dropcursor()'s USE IN People should genuinely have closed People");

    const auto before_tally_it = state.globals.find("beforetally");
    const auto after_tally_it = state.globals.find("aftertally");
    expect(before_tally_it != state.globals.end() &&
               copperfin::runtime::format_value(before_tally_it->second) == "1" &&
               after_tally_it != state.globals.end() &&
               copperfin::runtime::format_value(after_tally_it->second) == "1",
           "#6251: failed SELECT must preserve the preceding query's system _TALLY");
    const auto result_it = state.globals.find("resultpreserved");
    expect(result_it != state.globals.end() && result_it->second.boolean_value,
           "#6251: failed SELECT must preserve the target array sentinel");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6251: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

// A stored SET FILTER callback runs inside the visibility check, before
// WHERE. Its false result must not hide a cursor closure from SELECT.
void test_select_filter_closing_source_cursor_fails_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_select_filter_closes_source";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});
    const fs::path main_path = temp_root / "select_filter_closes_source.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT CNT(*) FROM People INTO ARRAY prior\n"
        "beforeTally = _TALLY\n"
        "DIMENSION result[1]\n"
        "result[1] = 'SENTINEL'\n"
        "SET FILTER TO dropcursor() IN People\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SELECT NAME FROM People INTO ARRAY result\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "afterTally = _TALLY\n"
        "resultPreserved = result[1] == 'SENTINEL' AND ALEN(result, 1) == 1\n"
        "lStillOpen = USED('People')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION dropcursor\n"
        "USE IN People\n"
        "RETURN .F.\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6251: SELECT filter callback closure should fail catchably: " + state.message);
    const auto error_it = state.globals.find("lerrorcaught");
    expect(error_it != state.globals.end() && error_it->second.boolean_value,
           "#6251: SELECT must detect a cursor closed by its stored visibility filter");
    const auto open_it = state.globals.find("lstillopen");
    expect(open_it != state.globals.end() && !open_it->second.boolean_value,
           "#6251: visibility filter must genuinely close the source cursor");
    const auto before_it = state.globals.find("beforetally");
    const auto after_it = state.globals.find("aftertally");
    expect(before_it != state.globals.end() &&
               copperfin::runtime::format_value(before_it->second) == "1" &&
               after_it != state.globals.end() &&
               copperfin::runtime::format_value(after_it->second) == "1",
           "#6251: visibility-filter closure must preserve system _TALLY");
    const auto result_it = state.globals.find("resultpreserved");
    expect(result_it != state.globals.end() && result_it->second.boolean_value,
           "#6251: visibility-filter closure must preserve the target array");
    expect(state.globals.find("after") != state.globals.end(),
           "#6251: script execution must continue after the caught filter error");
    fs::remove_all(temp_root, ignored);
}

// #6251: a projection expression can independently close the source cursor,
// distinct from the WHERE-clause trigger above.
void test_select_projection_expression_closing_source_cursor_fails_catchably() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_select_projection_closes_source";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "select_projection_closes_source.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "SELECT CNT(*) FROM People INTO ARRAY prior\n"
        "beforeTally = _TALLY\n"
        "DIMENSION result[1]\n"
        "result[1] = 'SENTINEL'\n"
        "lErrorCaught = .F.\n"
        "TRY\n"
        "    SELECT droptext() AS changed FROM People INTO ARRAY result\n"
        "CATCH TO oErr\n"
        "    lErrorCaught = .T.\n"
        "ENDTRY\n"
        "afterTally = _TALLY\n"
        "resultPreserved = result[1] == 'SENTINEL' AND ALEN(result, 1) == 1\n"
        "lStillOpen = USED('People')\n"
        "after = 1\n"
        "RETURN\n"
        "FUNCTION droptext\n"
        "IF RECNO('People') == 1\n"
        "    RETURN 'FIRST'\n"
        "ENDIF\n"
        "USE IN People\n"
        "RETURN 'CHANGED'\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed,
           "#6251: SELECT whose projection expression closes its own source cursor should complete "
           "without crashing: " + state.message);

    const auto error_caught_it = state.globals.find("lerrorcaught");
    expect(error_caught_it != state.globals.end() && error_caught_it->second.boolean_value,
           "#6251: SELECT must raise a catchable error instead of continuing through the closed source");

    const auto still_open_it = state.globals.find("lstillopen");
    expect(still_open_it != state.globals.end() && !still_open_it->second.boolean_value,
           "#6251: droptext()'s USE IN People should genuinely have closed People");

    const auto before_tally_it = state.globals.find("beforetally");
    const auto after_tally_it = state.globals.find("aftertally");
    expect(before_tally_it != state.globals.end() &&
               copperfin::runtime::format_value(before_tally_it->second) == "1" &&
               after_tally_it != state.globals.end() &&
               copperfin::runtime::format_value(after_tally_it->second) == "1",
           "#6251: projection failure after one row must preserve system _TALLY");
    const auto result_it = state.globals.find("resultpreserved");
    expect(result_it != state.globals.end() && result_it->second.boolean_value,
           "#6251: projection failure after one row must preserve the target array sentinel");

    const auto after_it = state.globals.find("after");
    expect(after_it != state.globals.end(), "#6251: script execution should continue after the caught error");
    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow

namespace cf_test_prg_engine_control_flow {

void test_create_table_free_dynamic_target() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_create_table_free_dynamic_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "dynamic_ref";
    const fs::path main_path = temp_root / "create_table_free.prg";
    write_text(
        main_path,
        "cTable = '" + table_path.string() + "'\n"
        "CREATE TABLE (m.cTable) FREE (Marker C(8), Inactive L, Timestamp T NULL)\n"
        "INSERT INTO (m.cTable) (Marker, Inactive, Timestamp) VALUES ('one', .F., DATETIME())\n"
        "lCreated = FILE(cTable + '.dbf')\n"
        "cAliasCreated = ALIAS()\n"
        "nFieldCount = FCOUNT()\n"
        "nRecordCount = RECCOUNT()\n"
        "cTableNoFree = '" + (temp_root / "dynamic_ref_nofree").string() + "'\n"
        "CREATE TABLE (m.cTableNoFree) (Marker C(8), Timestamp T NULL)\n"
        "lCreatedNoFree = FILE(cTableNoFree + '.dbf')\n"
        "cAliasNoFree = ALIAS()\n"
        "nFieldCountNoFree = FCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "dynamic CREATE TABLE FREE script should complete: " + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("lcreated", "true");
    check("caliascreated", "dynamic_ref");
    check("nfieldcount", "3");
    check("nrecordcount", "1");
    check("lcreatednofree", "true");
    check("caliasnofree", "dynamic_ref_nofree");
    check("nfieldcountnofree", "2");
    const auto table_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string() + ".dbf", 5U);
    expect(table_result.ok, "dynamic CREATE TABLE FREE output should remain readable: " + table_result.error);
    if (table_result.ok && !table_result.table.records.empty()) {
        const auto timestamp = std::find_if(
            table_result.table.records.front().values.begin(),
            table_result.table.records.front().values.end(),
            [](const auto &field) { return field.field_name == "Timestamp"; });
        expect(timestamp != table_result.table.records.front().values.end(),
               "dynamic INSERT should persist the Timestamp field");
        if (timestamp != table_result.table.records.front().values.end()) {
            expect(timestamp->display_value.rfind("julian:", 0U) == 0U &&
                       timestamp->display_value != "julian:0 millis:0",
                   "dynamic INSERT should serialize DATETIME() to DBF DateTime storage");
        }
    }
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto &event) {
        return event.category == "runtime.create_table";
    }), "dynamic CREATE TABLE FREE should emit table metadata");

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_control_flow
