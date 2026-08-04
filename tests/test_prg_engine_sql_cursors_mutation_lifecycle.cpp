// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

namespace copperfin::sql_cursor_mutation_tests
{

using namespace copperfin::test_support;
using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

void test_sql_result_cursors_are_isolated_by_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_datasession";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_datasession.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust')\n"
        "nArea1 = SELECT('sqlcust')\n"
        "SET DATASESSION TO 2\n"
        "lUsedSession2Before = USED('sqlcust')\n"
        "nAreaSession2Before = SELECT('sqlcust')\n"
        "nExecCrossSession = SQLEXEC(nConn1, 'select * from orders', 'sqlcust2')\n"
        "cExecCrossSessionMessage = MESSAGE()\n"
        "lDisconnectSession2BeforeConnect = SQLDISCONNECT(nConn1)\n"
        "cDisconnectCrossSessionMessage = MESSAGE()\n"
        "nConn2 = SQLCONNECT('dsn=SessionTwo')\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from orders', 'sqlother')\n"
        "lUsedSession2After = USED('sqlother')\n"
        "nAreaSession2After = SELECT('sqlother')\n"
        "lDisconnectSession2Own = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "lUsedSession1Back = USED('sqlcust')\n"
        "nAreaSession1Back = SELECT('sqlcust')\n"
        "lUsedSession1Other = USED('sqlother')\n"
        "nConn1Again = SQLCONNECT('dsn=NorthwindAgain')\n"
        "lDisconnectSession1Again = SQLDISCONNECT(nConn1Again)\n"
        "lDisconnectSession1Own = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL data-session isolation script should complete");
    expect(state.work_area.data_session == 1, "SQL data-session isolation script should restore data session 1");
    expect(state.sql_connections.empty(), "all session-local SQL connections should be disconnected by the end of the script");

    const auto area1 = state.globals.find("narea1");
    const auto used_session2_before = state.globals.find("lusedsession2before");
    const auto area_session2_before = state.globals.find("nareasession2before");
    const auto exec_cross_session = state.globals.find("nexeccrosssession");
    const auto exec_cross_session_message = state.globals.find("cexeccrosssessionmessage");
    const auto disconnect_session2_before_connect = state.globals.find("ldisconnectsession2beforeconnect");
    const auto disconnect_cross_session_message = state.globals.find("cdisconnectcrosssessionmessage");
    const auto conn2 = state.globals.find("nconn2");
    const auto exec2 = state.globals.find("nexec2");
    const auto used_session2_after = state.globals.find("lusedsession2after");
    const auto area_session2_after = state.globals.find("nareasession2after");
    const auto disconnect_session2_own = state.globals.find("ldisconnectsession2own");
    const auto used_session1_back = state.globals.find("lusedsession1back");
    const auto area_session1_back = state.globals.find("nareasession1back");
    const auto used_session1_other = state.globals.find("lusedsession1other");
    const auto conn1_again = state.globals.find("nconn1again");
    const auto disconnect_session1_again = state.globals.find("ldisconnectsession1again");
    const auto disconnect_session1_own = state.globals.find("ldisconnectsession1own");

    expect(area1 != state.globals.end(), "session-1 SQL cursor area should be captured");
    expect(used_session2_before != state.globals.end(), "session-2 preexisting SQL cursor visibility should be captured");
    expect(area_session2_before != state.globals.end(), "session-2 preexisting SQL cursor area should be captured");
    expect(exec_cross_session != state.globals.end(), "cross-session SQLEXEC result should be captured");
    expect(exec_cross_session_message != state.globals.end(), "cross-session SQLEXEC message should be captured");
    expect(disconnect_session2_before_connect != state.globals.end(), "cross-session SQLDISCONNECT before a local connect should be captured");
    expect(disconnect_cross_session_message != state.globals.end(), "cross-session SQLDISCONNECT message should be captured");
    expect(conn2 != state.globals.end(), "session-2 SQLCONNECT handle should be captured");
    expect(exec2 != state.globals.end(), "session-2 SQLEXEC result should be captured");
    expect(used_session2_after != state.globals.end(), "session-2 SQL cursor visibility should be captured");
    expect(area_session2_after != state.globals.end(), "session-2 SQL cursor area should be captured");
    expect(disconnect_session2_own != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(used_session1_back != state.globals.end(), "restored session-1 SQL cursor visibility should be captured");
    expect(area_session1_back != state.globals.end(), "restored session-1 SQL cursor area should be captured");
    expect(used_session1_other != state.globals.end(), "restored session-1 visibility for session-2 alias should be captured");
    expect(conn1_again != state.globals.end(), "restored session-1 SQLCONNECT handle should be captured");
    expect(disconnect_session1_again != state.globals.end(), "restored session-1 second SQLDISCONNECT result should be captured");
    expect(disconnect_session1_own != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (area1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(area1->second) == "1", "session 1 should materialize its SQL cursor in work area 1");
    }
    if (used_session2_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session2_before->second) == "false", "switching to a fresh data session should hide session-1 SQL cursors");
    }
    if (area_session2_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session2_before->second) == "0", "SELECT('alias') should not resolve a SQL cursor from another data session");
    }
    if (exec_cross_session != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cross_session->second) == "-1", "SQLEXEC should reject a SQL handle from another data session");
    }
    if (exec_cross_session_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cross_session_message->second) == "SQL handle not found: 1",
            "cross-session SQLEXEC missing-handle message should route through the default locale catalog");
    }
    if (disconnect_session2_before_connect != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session2_before_connect->second) == "-1", "SQLDISCONNECT should reject a SQL handle from another data session before the session creates its own handle");
    }
    if (disconnect_cross_session_message != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_cross_session_message->second) == "SQL handle not found: 1",
            "cross-session SQLDISCONNECT missing-handle message should route through the default locale catalog");
    }
    if (conn2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn2->second) == "1", "the first SQLCONNECT handle in a fresh data session should restart at 1");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "session 2 should still be able to create its own SQL cursor");
    }
    if (used_session2_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session2_after->second) == "true", "session 2 should see its own SQL cursor");
    }
    if (area_session2_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session2_after->second) == "1", "session 2 should resolve its own SQL cursor area");
    }
    if (disconnect_session2_own != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session2_own->second) == "1", "session 2 should disconnect its own SQL handle");
    }
    if (used_session1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session1_back->second) == "true", "restoring session 1 should restore its SQL cursor visibility");
    }
    if (area_session1_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(area_session1_back->second) == "1", "restoring session 1 should restore its SQL cursor work area");
    }
    if (used_session1_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_session1_other->second) == "false", "session-2 SQL aliases should stay hidden after restoring session 1");
    }
    if (conn1_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(conn1_again->second) == "2", "restoring session 1 should resume that session's SQLCONNECT handle numbering");
    }
    if (disconnect_session1_again != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session1_again->second) == "1", "session 1 should disconnect its later SQL handle after restoring the session");
    }
    if (disconnect_session1_own != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session1_own->second) == "1", "session 1 should disconnect its own SQL handle");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_auto_allocation_tracks_session_selection_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_selection_flow";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_selection_flow.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "SELECT 0\n"
        "nSession1SelectedBefore = SELECT()\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust1')\n"
        "nSession1Area = SELECT('sqlcust1')\n"
        "nSession1SelectedAfter = SELECT()\n"
        "SET DATASESSION TO 2\n"
        "nConn2 = SQLCONNECT('dsn=SessionTwo')\n"
        "SELECT 0\n"
        "SELECT 0\n"
        "nSession2SelectedBefore = SELECT()\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from orders', 'sqlcust2')\n"
        "nSession2Area = SELECT('sqlcust2')\n"
        "nSession2SelectedAfter = SELECT()\n"
        "lDisc2 = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "nSession1AreaBack = SELECT('sqlcust1')\n"
        "lDisc1 = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL selection-flow script should complete");
    expect(state.work_area.data_session == 1, "SQL selection-flow script should restore data session 1");
    expect(state.sql_connections.empty(), "SQL selection-flow script should disconnect all session-local handles");

    const auto session1_selected_before = state.globals.find("nsession1selectedbefore");
    const auto exec1 = state.globals.find("nexec1");
    const auto session1_area = state.globals.find("nsession1area");
    const auto session1_selected_after = state.globals.find("nsession1selectedafter");
    const auto session2_selected_before = state.globals.find("nsession2selectedbefore");
    const auto exec2 = state.globals.find("nexec2");
    const auto session2_area = state.globals.find("nsession2area");
    const auto session2_selected_after = state.globals.find("nsession2selectedafter");
    const auto session1_area_back = state.globals.find("nsession1areaback");
    const auto disc2 = state.globals.find("ldisc2");
    const auto disc1 = state.globals.find("ldisc1");

    expect(session1_selected_before != state.globals.end(), "session-1 selected area before SQLEXEC should be captured");
    expect(exec1 != state.globals.end(), "session-1 SQLEXEC result should be captured");
    expect(session1_area != state.globals.end(), "session-1 SQL cursor area should be captured");
    expect(session1_selected_after != state.globals.end(), "session-1 selected area after SQLEXEC should be captured");
    expect(session2_selected_before != state.globals.end(), "session-2 selected area before SQLEXEC should be captured");
    expect(exec2 != state.globals.end(), "session-2 SQLEXEC result should be captured");
    expect(session2_area != state.globals.end(), "session-2 SQL cursor area should be captured");
    expect(session2_selected_after != state.globals.end(), "session-2 selected area after SQLEXEC should be captured");
    expect(session1_area_back != state.globals.end(), "session-1 SQL cursor area after restoring the session should be captured");
    expect(disc2 != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(disc1 != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (session1_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_before->second) == "1", "session 1 should auto-select work area 1 before its first SQLEXEC");
    }
    if (exec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec1->second) == "1", "session 1 SQLEXEC should succeed");
    }
    if (session1_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area->second) == "1", "session 1 SQLEXEC should reuse the selected empty work area");
    }
    if (session1_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_selected_after->second) == "1", "session 1 should keep the SQL cursor on its selected work area");
    }
    if (session2_selected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_before->second) == "2", "session 2 should preserve its own current SELECT 0 flow before SQLEXEC");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "session 2 SQLEXEC should succeed");
    }
    if (session2_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_area->second) == "2", "session 2 SQLEXEC should reuse that session's selected empty work area");
    }
    if (session2_selected_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_selected_after->second) == "2", "session 2 should keep its SQL cursor on the selected work area");
    }
    if (session1_area_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(session1_area_back->second) == "1", "restoring session 1 should keep its SQL cursor bound to session 1's selection flow");
    }
    if (disc2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc2->second) == "1", "session 2 should disconnect its own SQL handle");
    }
    if (disc1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc1->second) == "1", "session 1 should disconnect its own SQL handle");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursors_and_ole_actions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sqlcursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sqlcursor.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "cAlias = ALIAS()\n"
        "nCount = RECCOUNT()\n"
        "nRec = RECNO()\n"
        "oExcel = CREATEOBJECT('Excel.Application')\n"
        "oExcel.Visible = .T.\n"
        "cVisible = oExcel.Visible\n"
        "oBook = oExcel.Workbooks.Add()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL cursor/OLE script should complete");
    expect(!state.cursors.empty(), "SQLEXEC should materialize a result cursor");
    expect(state.ole_objects.size() == 1U, "CREATEOBJECT and follow-on automation should track one OLE object");

    const auto alias = state.globals.find("calias");
    const auto count = state.globals.find("ncount");
    const auto rec = state.globals.find("nrec");
    const auto visible = state.globals.find("cvisible");
    const auto book = state.globals.find("obook");

    expect(alias != state.globals.end(), "ALIAS() for SQL cursor should be captured");
    expect(count != state.globals.end(), "RECCOUNT() for SQL cursor should be captured");
    expect(rec != state.globals.end(), "RECNO() for SQL cursor should be captured");
    expect(visible != state.globals.end(), "OLE property reads should flow back into VFP code");
    expect(book != state.globals.end(), "OLE method calls should return a placeholder value");

    if (alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(alias->second) == "sqlcust", "SQLEXEC cursor alias should be selectable like a normal work area");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "3", "synthetic SQL result cursors should expose row counts");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "synthetic SQL result cursors should begin on record 1");
    }
    if (visible != state.globals.end()) {
        expect(!copperfin::runtime::format_value(visible->second).empty(), "OLE property access should produce a debuggable value");
    }
    if (book != state.globals.end()) {
        expect(!copperfin::runtime::format_value(book->second).empty(), "OLE method invocation should return a placeholder object/value");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "sql.cursor"; }),
        "SQLEXEC with a cursor alias should emit a sql.cursor event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.set"; }),
        "OLE property assignments should emit ole.set events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "ole.invoke"; }),
        "OLE method calls should emit ole.invoke events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_mutation_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutation_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutation_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cFound = NAME\n"
        "nFoundAmount = AMOUNT\n"
        "REPLACE AMOUNT WITH 21, NAME WITH 'BRAVOX'\n"
        "cAfterReplace = NAME\n"
        "nAfterReplace = AMOUNT\n"
        "nBeforeAppend = RECCOUNT('sqlcust')\n"
        "APPEND BLANK\n"
        "nAfterAppend = RECCOUNT('sqlcust')\n"
        "nRecAfterAppend = RECNO()\n"
        "lAppendDeleted = DELETED()\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "nBeforeTargetedAppend = RECCOUNT('sqlcust')\n"
        "APPEND BLANK IN sqlcust\n"
        "nAfterTargetedAppend = RECCOUNT('sqlcust')\n"
        "cAliasAfterTargetedAppend = ALIAS()\n"
        "SELECT sqlcust\n"
        "nTargetedRecAfterAppend = RECNO()\n"
        "lTargetedAppendDeleted = DELETED()\n"
        "REPLACE NAME WITH 'DELTA', AMOUNT WITH 40\n"
        "cAppendedName = NAME\n"
        "nAppendedAmount = AMOUNT\n"
        "SET ORDER TO NAME\n"
        "lSeekDelta = SEEK('DELTA')\n"
        "nSeekRec = RECNO()\n"
        "cSeekName = NAME\n"
        "DELETE\n"
        "lDeleted = DELETED()\n"
        "RECALL\n"
        "lRecalled = DELETED()\n"
        "DELETE FOR AMOUNT = 30\n"
        "LOCATE FOR DELETED()\n"
        "cDeletedName = NAME\n"
        "nDeletedAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL mutation parity script should complete");
    expect(state.sql_connections.empty(), "SQL mutation parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto found = state.globals.find("cfound");
    const auto found_amount = state.globals.find("nfoundamount");
    const auto after_replace = state.globals.find("cafterreplace");
    const auto after_replace_amount = state.globals.find("nafterreplace");
    const auto before_append = state.globals.find("nbeforeappend");
    const auto after_append = state.globals.find("nafterappend");
    const auto rec_after_append = state.globals.find("nrecafterappend");
    const auto append_deleted = state.globals.find("lappenddeleted");
    const auto exec_other = state.globals.find("nexecother");
    const auto before_targeted_append = state.globals.find("nbeforetargetedappend");
    const auto after_targeted_append = state.globals.find("naftertargetedappend");
    const auto alias_after_targeted_append = state.globals.find("caliasaftertargetedappend");
    const auto targeted_rec_after_append = state.globals.find("ntargetedrecafterappend");
    const auto targeted_append_deleted = state.globals.find("ltargetedappenddeleted");
    const auto appended_name = state.globals.find("cappendedname");
    const auto appended_amount = state.globals.find("nappendedamount");
    const auto seek_delta = state.globals.find("lseekdelta");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto seek_name = state.globals.find("cseekname");
    const auto deleted = state.globals.find("ldeleted");
    const auto recalled = state.globals.find("lrecalled");
    const auto deleted_name = state.globals.find("cdeletedname");
    const auto deleted_amount = state.globals.find("ndeletedamount");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL mutation parity");
    expect(found != state.globals.end(), "LOCATE on a SQL cursor should expose the matching NAME");
    expect(found_amount != state.globals.end(), "LOCATE on a SQL cursor should expose the matching AMOUNT");
    expect(after_replace != state.globals.end(), "REPLACE on a SQL cursor should expose the updated NAME");
    expect(after_replace_amount != state.globals.end(), "REPLACE on a SQL cursor should expose the updated AMOUNT");
    expect(before_append != state.globals.end(), "RECCOUNT() before SQL APPEND BLANK should be captured");
    expect(after_append != state.globals.end(), "RECCOUNT() after SQL APPEND BLANK should be captured");
    expect(rec_after_append != state.globals.end(), "RECNO() after SQL APPEND BLANK should be captured");
    expect(append_deleted != state.globals.end(), "DELETED() after SQL APPEND BLANK should be captured");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for targeted SQL APPEND BLANK");
    expect(before_targeted_append != state.globals.end(), "RECCOUNT() before targeted SQL APPEND BLANK should be captured");
    expect(after_targeted_append != state.globals.end(), "RECCOUNT() after targeted SQL APPEND BLANK should be captured");
    expect(alias_after_targeted_append != state.globals.end(), "ALIAS() after targeted SQL APPEND BLANK should be captured");
    expect(targeted_rec_after_append != state.globals.end(), "RECNO() after targeted SQL APPEND BLANK should be captured");
    expect(targeted_append_deleted != state.globals.end(), "DELETED() after targeted SQL APPEND BLANK should be captured");
    expect(appended_name != state.globals.end(), "REPLACE after SQL APPEND BLANK should expose the appended NAME");
    expect(appended_amount != state.globals.end(), "REPLACE after SQL APPEND BLANK should expose the appended AMOUNT");
    expect(seek_delta != state.globals.end(), "SEEK after SQL APPEND BLANK should expose whether the appended row is indexed");
    expect(seek_rec != state.globals.end(), "RECNO() after SQL SEEK should be captured");
    expect(seek_name != state.globals.end(), "SEEK after SQL APPEND BLANK should expose the matching NAME");
    expect(deleted != state.globals.end(), "DELETE on a SQL cursor should expose DELETED()");
    expect(recalled != state.globals.end(), "RECALL on a SQL cursor should expose DELETED()");
    expect(deleted_name != state.globals.end(), "DELETE FOR on a SQL cursor should expose the tombstoned NAME");
    expect(deleted_amount != state.globals.end(), "DELETE FOR on a SQL cursor should expose the tombstoned AMOUNT");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL mutation parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL mutation checks");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "BRAVO", "LOCATE should position the matching synthetic SQL row before mutation");
    }
    if (found_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_amount->second) == "20", "field resolution should expose the original SQL row values before mutation");
    }
    if (after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_replace->second) == "BRAVOX", "REPLACE should update synthetic SQL character fields in place");
    }
    if (after_replace_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_replace_amount->second) == "21", "REPLACE should update synthetic SQL numeric fields in place");
    }
    if (before_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_append->second) == "3", "synthetic SQL result cursors should start with three rows in this fixture");
    }
    if (after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_append->second) == "4", "APPEND BLANK should add a new synthetic SQL row");
    }
    if (rec_after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after_append->second) == "4", "APPEND BLANK should move the SQL cursor pointer to the appended row");
    }
    if (append_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(append_deleted->second) == "false", "APPEND BLANK should create a non-deleted synthetic SQL row");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL APPEND BLANK checks");
    }
    if (before_targeted_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_targeted_append->second) == "4", "targeted SQL APPEND BLANK should start from the prior appended row count");
    }
    if (after_targeted_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_targeted_append->second) == "5", "targeted SQL APPEND BLANK should append to the requested non-selected SQL cursor");
    }
    if (alias_after_targeted_append != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_targeted_append->second)) == "SQLOTHER", "targeted SQL APPEND BLANK should preserve the current selected alias");
    }
    if (targeted_rec_after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(targeted_rec_after_append->second) == "5", "targeted SQL APPEND BLANK should move the targeted SQL cursor pointer to the appended row");
    }
    if (targeted_append_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(targeted_append_deleted->second) == "false", "targeted SQL APPEND BLANK should create a non-deleted row on the targeted cursor");
    }
    if (appended_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(appended_name->second) == "DELTA", "REPLACE after APPEND BLANK should update the appended SQL row");
    }
    if (appended_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(appended_amount->second) == "40", "REPLACE after APPEND BLANK should update numeric fields on the appended SQL row");
    }
    if (seek_delta != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_delta->second) == "true", "SEEK should find SQL rows appended and mutated in memory");
    }
    if (seek_rec != state.globals.end()) {
        expect(
            targeted_rec_after_append != state.globals.end() &&
                copperfin::runtime::format_value(seek_rec->second) == copperfin::runtime::format_value(targeted_rec_after_append->second),
            "SEEK should position to the SQL row appended by the targeted APPEND BLANK");
    }
    if (seek_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_name->second) == "DELTA", "SEEK should expose the appended SQL row values after in-memory mutation");
    }
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true", "DELETE should tombstone the current synthetic SQL row");
    }
    if (recalled != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled->second) == "false", "RECALL should clear the synthetic SQL tombstone flag");
    }
    if (deleted_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_name->second) == "CHARLIE", "DELETE FOR should tombstone the matching synthetic SQL row");
    }
    if (deleted_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_amount->second) == "30", "DELETE FOR should preserve field lookup on the tombstoned synthetic SQL row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL mutation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.append_blank"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "SQL mutation commands should emit the same runtime events as local mutation commands");

    fs::remove_all(temp_root, ignored);
}

}  // namespace copperfin::sql_cursor_mutation_tests

