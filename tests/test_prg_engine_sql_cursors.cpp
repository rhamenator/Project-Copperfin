#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_cursor_identity_functions_for_sql_result_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cursor_identity_sql";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "identity_sql.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "lUsed = USED('sqlcust')\n"
        "cDbf = DBF('sqlcust')\n"
        "nFields = FCOUNT('sqlcust')\n"
        "cField2 = FIELD(2, 'sqlcust')\n"
        "nSizeAmount = FSIZE('AMOUNT', 'sqlcust')\n"
        "nSizeName = FSIZE(2, 'sqlcust')\n"
        "nAFieldCount = AFIELDS(aSqlFields, 'sqlcust')\n"
        "cAField2 = aSqlFields[2,1]\n"
        "nAField2Size = aSqlFields[2,3]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL cursor identity script should complete");

    const auto used = state.globals.find("lused");
    const auto dbf = state.globals.find("cdbf");
    const auto fields = state.globals.find("nfields");
    const auto field2 = state.globals.find("cfield2");
    const auto size_amount = state.globals.find("nsizeamount");
    const auto size_name = state.globals.find("nsizename");
    const auto afield_count = state.globals.find("nafieldcount");
    const auto afield2 = state.globals.find("cafield2");
    const auto afield2_size = state.globals.find("nafield2size");

    expect(used != state.globals.end(), "USED('sqlcust') should be captured for the SQL cursor");
    expect(dbf != state.globals.end(), "DBF('sqlcust') should be captured for the SQL cursor");
    expect(fields != state.globals.end(), "FCOUNT('sqlcust') should be captured for the SQL cursor");
    expect(field2 != state.globals.end(), "FIELD(index, alias) should be captured for the SQL cursor");
    expect(size_amount != state.globals.end(), "FSIZE(name, alias) should be captured for the SQL cursor");
    expect(size_name != state.globals.end(), "FSIZE(index, alias) should be captured for the SQL cursor");
    expect(afield_count != state.globals.end(), "AFIELDS(array, alias) should be captured for the SQL cursor");
    expect(afield2 != state.globals.end(), "AFIELDS should populate SQL cursor field names");
    expect(afield2_size != state.globals.end(), "AFIELDS should populate SQL cursor field widths");

    if (used != state.globals.end()) {
        expect(copperfin::runtime::format_value(used->second) == "true", "USED('sqlcust') should report true for a materialized SQL cursor");
    }
    if (dbf != state.globals.end()) {
        expect(copperfin::runtime::format_value(dbf->second) == "sqlcust", "DBF('sqlcust') should expose the runtime identity for a SQL cursor");
    }
    if (fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields->second) == "3", "FCOUNT('sqlcust') should expose the synthetic SQL cursor schema");
    }
    if (field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field2->second) == "NAME", "FIELD(index, alias) should expose synthetic SQL field order");
    }
    if (size_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(size_amount->second) == "18", "FSIZE(name, alias) should expose synthetic SQL numeric width");
    }
    if (size_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(size_name->second) == "32", "FSIZE(index, alias) should expose synthetic SQL character width");
    }
    if (afield_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(afield_count->second) == "3", "AFIELDS(array, alias) should expose synthetic SQL field count");
    }
    if (afield2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(afield2->second) == "NAME", "AFIELDS should expose synthetic SQL field order");
    }
    if (afield2_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(afield2_size->second) == "32", "AFIELDS should expose synthetic SQL field width");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_mutation_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutations.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "GO BOTTOM\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'DELTA', AMOUNT WITH 99\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "GO 2\n"
        "lDeletedBravo = DELETED()\n"
        "RECALL FOR NAME = 'BRAVO'\n"
        "GO 2\n"
        "lRecalledBravo = DELETED()\n"
        "nCount = RECCOUNT('sqlcust')\n"
        "GO BOTTOM\n"
        "cLastName = NAME\n"
        "nLastAmount = AMOUNT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL mutation script should complete");

    const auto deleted_bravo = state.globals.find("ldeletedbravo");
    const auto recalled_bravo = state.globals.find("lrecalledbravo");
    const auto count = state.globals.find("ncount");
    const auto last_name = state.globals.find("clastname");
    const auto last_amount = state.globals.find("nlastamount");

    expect(deleted_bravo != state.globals.end(), "DELETE FOR over SQL cursor should expose DELETED() state");
    expect(recalled_bravo != state.globals.end(), "RECALL FOR over SQL cursor should expose DELETED() state");
    expect(count != state.globals.end(), "SQL mutation flow should expose RECCOUNT() after APPEND BLANK");
    expect(last_name != state.globals.end(), "SQL mutation flow should expose appended-row NAME values");
    expect(last_amount != state.globals.end(), "SQL mutation flow should expose appended-row numeric values");

    if (deleted_bravo != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_bravo->second) == "true", "DELETE FOR should tombstone matching SQL cursor rows");
    }
    if (recalled_bravo != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled_bravo->second) == "false", "RECALL FOR should clear SQL cursor tombstones");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "4", "APPEND BLANK should grow synthetic SQL cursor record count");
    }
    if (last_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_name->second) == "DELTA", "REPLACE should persist appended SQL cursor character values");
    }
    if (last_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_amount->second) == "99", "REPLACE should persist appended SQL cursor numeric values");
    }

    expect(
        has_runtime_event(state.events, "runtime.append_blank", "sqlcust") &&
        has_runtime_event(state.events, "runtime.replace", "NAME WITH 'DELTA', AMOUNT WITH 99") &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "SQL mutation commands should emit append/replace/delete/recall runtime events");

    fs::remove_all(temp_root, ignored);
}

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
        "lDisconnectSession2BeforeConnect = SQLDISCONNECT(nConn1)\n"
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL data-session isolation script should complete");
    expect(state.work_area.data_session == 1, "SQL data-session isolation script should restore data session 1");
    expect(state.sql_connections.empty(), "all session-local SQL connections should be disconnected by the end of the script");

    const auto area1 = state.globals.find("narea1");
    const auto used_session2_before = state.globals.find("lusedsession2before");
    const auto area_session2_before = state.globals.find("nareasession2before");
    const auto exec_cross_session = state.globals.find("nexeccrosssession");
    const auto disconnect_session2_before_connect = state.globals.find("ldisconnectsession2beforeconnect");
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
    expect(disconnect_session2_before_connect != state.globals.end(), "cross-session SQLDISCONNECT before a local connect should be captured");
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
    if (disconnect_session2_before_connect != state.globals.end()) {
        expect(copperfin::runtime::format_value(disconnect_session2_before_connect->second) == "-1", "SQLDISCONNECT should reject a SQL handle from another data session before the session creates its own handle");
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

void test_sql_result_cursor_read_only_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_read_only_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_read_only_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET FILTER TO ID >= 2\n"
        "GO TOP\n"
        "cTopName = NAME\n"
        "LOCATE FOR AMOUNT = 20\n"
        "lFound = FOUND()\n"
        "nLocateRec = RECNO()\n"
        "cLocateName = NAME\n"
        "nCountVisible = COUNT()\n"
        "nSumVisible = SUM(AMOUNT)\n"
        "CALCULATE COUNT() TO nCalcCount, SUM(AMOUNT) TO nCalcSum\n"
        "nCountAlias = COUNT(ID >= 2, 'sqlcust')\n"
        "nSumAlias = SUM(AMOUNT, ID >= 2, 'sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL read-only parity script should complete");
    expect(state.sql_connections.empty(), "SQL read-only parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto top_name = state.globals.find("ctopname");
    const auto found = state.globals.find("lfound");
    const auto locate_rec = state.globals.find("nlocaterec");
    const auto locate_name = state.globals.find("clocatename");
    const auto count_visible = state.globals.find("ncountvisible");
    const auto sum_visible = state.globals.find("nsumvisible");
    const auto calc_count = state.globals.find("ncalccount");
    const auto calc_sum = state.globals.find("ncalcsum");
    const auto count_alias = state.globals.find("ncountalias");
    const auto sum_alias = state.globals.find("nsumalias");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for the SQL read-only parity script");
    expect(top_name != state.globals.end(), "filtered GO TOP should expose the current SQL cursor row");
    expect(found != state.globals.end(), "LOCATE on a SQL cursor should expose FOUND()");
    expect(locate_rec != state.globals.end(), "LOCATE on a SQL cursor should expose RECNO()");
    expect(locate_name != state.globals.end(), "LOCATE on a SQL cursor should expose field values");
    expect(count_visible != state.globals.end(), "COUNT() should work against a filtered SQL cursor");
    expect(sum_visible != state.globals.end(), "SUM() should work against a filtered SQL cursor");
    expect(calc_count != state.globals.end(), "CALCULATE COUNT() should work against a SQL cursor");
    expect(calc_sum != state.globals.end(), "CALCULATE SUM() should work against a SQL cursor");
    expect(count_alias != state.globals.end(), "COUNT(..., alias) should target a SQL cursor by alias");
    expect(sum_alias != state.globals.end(), "SUM(..., alias) should target a SQL cursor by alias");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for the SQL read-only parity script");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before read-only SQL cursor checks");
    }
    if (top_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(top_name->second) == "BRAVO", "SET FILTER plus GO TOP should position a SQL cursor on the first visible synthetic row");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "true", "LOCATE should succeed on a SQL cursor when the synthetic row matches");
    }
    if (locate_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_rec->second) == "2", "LOCATE should leave the SQL cursor on the matching synthetic row");
    }
    if (locate_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_name->second) == "BRAVO", "SQL cursor field lookup should flow through the located synthetic row");
    }
    if (count_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_visible->second) == "2", "COUNT() should respect active SQL cursor filters");
    }
    if (sum_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_visible->second) == "50", "SUM() should aggregate visible synthetic SQL rows");
    }
    if (calc_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(calc_count->second) == "2", "CALCULATE COUNT() should respect active SQL cursor filters");
    }
    if (calc_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(calc_sum->second) == "50", "CALCULATE SUM() should aggregate visible synthetic SQL rows");
    }
    if (count_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_alias->second) == "2", "COUNT(condition, alias) should resolve the SQL cursor by alias");
    }
    if (sum_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(sum_alias->second) == "50", "SUM(value, condition, alias) should resolve the SQL cursor by alias");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after read-only SQL cursor operations");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter"; }),
        "SQL cursor filter changes should emit runtime.filter events");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }),
        "SQL cursor LOCATE should emit runtime.locate events");
    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.calculate"; }),
        "SQL cursor CALCULATE should emit runtime.calculate events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_seek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "lSeekName = SEEK('BRAVO', 'sqlcust', 'NAME')\n"
        "nSeekRec = RECNO()\n"
        "GO TOP\n"
        "lIndexNoMove = INDEXSEEK('CHARLIE', .F., 'sqlcust', 'NAME')\n"
        "nAfterNoMove = RECNO()\n"
        "lIndexMove = INDEXSEEK('CHARLIE', .T., 'sqlcust', 'NAME')\n"
        "nAfterMove = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "lSeekNear = SEEK('BETA', 'sqlcust', 'NAME')\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL seek parity script should complete");
    expect(state.sql_connections.empty(), "SQL seek parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_name = state.globals.find("lseekname");
    const auto seek_rec = state.globals.find("nseekrec");
    const auto index_no_move = state.globals.find("lindexnomove");
    const auto after_no_move = state.globals.find("nafternomove");
    const auto index_move = state.globals.find("lindexmove");
    const auto after_move = state.globals.find("naftermove");
    const auto seek_near = state.globals.find("lseeknear");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto order_after = state.globals.find("corderafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL seek parity");
    expect(seek_name != state.globals.end(), "SEEK() on a SQL cursor should be captured");
    expect(seek_rec != state.globals.end(), "RECNO() after SQL SEEK() should be captured");
    expect(index_no_move != state.globals.end(), "INDEXSEEK(.F.) on a SQL cursor should be captured");
    expect(after_no_move != state.globals.end(), "RECNO() after SQL INDEXSEEK(.F.) should be captured");
    expect(index_move != state.globals.end(), "INDEXSEEK(.T.) on a SQL cursor should be captured");
    expect(after_move != state.globals.end(), "RECNO() after SQL INDEXSEEK(.T.) should be captured");
    expect(seek_near != state.globals.end(), "SEEK() miss with SET NEAR on a SQL cursor should be captured");
    expect(near_found != state.globals.end(), "FOUND() after SQL SEEK() miss should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SQL SEEK() miss should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SQL SEEK()/INDEXSEEK() probes should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL seek parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL SEEK checks");
    }
    if (seek_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_name->second) == "true", "SEEK() should find a matching synthetic SQL row by one-off order expression");
    }
    if (seek_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_rec->second) == "2", "SEEK() should position the SQL cursor on the matching synthetic row");
    }
    if (index_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_no_move->second) == "true", "INDEXSEEK(.F.) should report SQL cursor matches");
    }
    if (after_no_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_no_move->second) == "1", "INDEXSEEK(.F.) should not move the SQL cursor pointer");
    }
    if (index_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move->second) == "true", "INDEXSEEK(.T.) should report SQL cursor matches");
    }
    if (after_move != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_move->second) == "3", "INDEXSEEK(.T.) should move the SQL cursor pointer to the matching row");
    }
    if (seek_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_near->second) == "false", "SEEK() should report a miss for an in-between SQL key");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "FOUND() should stay false after a SQL SEEK() miss");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR should position a SQL cursor to the next matching synthetic row");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "one-off SQL SEEK()/INDEXSEEK() order expressions should not permanently change ORDER()");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL SEEK() checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_temporary_order_normalization_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_normalization_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_normalization_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "lSeekFn = SEEK('bravo', 'sqlcust', 'UPPER(NAME)')\n"
        "nSeekFnRec = RECNO()\n"
        "GO TOP\n"
        "SET ORDER TO UPPER(NAME)\n"
        "SEEK 'charlie'\n"
        "lFoundCmd = FOUND()\n"
        "nSeekCmdRec = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL temporary-order normalization parity script should complete");
    expect(state.sql_connections.empty(), "SQL temporary-order normalization parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_fn = state.globals.find("lseekfn");
    const auto seek_fn_rec = state.globals.find("nseekfnrec");
    const auto found_cmd = state.globals.find("lfoundcmd");
    const auto seek_cmd_rec = state.globals.find("nseekcmdrec");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL temporary-order normalization parity");
    expect(seek_fn != state.globals.end(), "SEEK() with UPPER(NAME) should be captured for a SQL cursor");
    expect(seek_fn_rec != state.globals.end(), "RECNO() after SQL SEEK() with UPPER(NAME) should be captured");
    expect(found_cmd != state.globals.end(), "FOUND() after command SEEK with SQL UPPER(NAME) order should be captured");
    expect(seek_cmd_rec != state.globals.end(), "RECNO() after command SEEK with SQL UPPER(NAME) order should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL temporary-order normalization parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL temporary-order normalization checks");
    }
    if (seek_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn->second) == "true", "SEEK() should case-fold search keys for SQL UPPER(NAME) temporary orders");
    }
    if (seek_fn_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_fn_rec->second) == "2", "SEEK() should position the SQL cursor on the normalized match");
    }
    if (found_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_cmd->second) == "true", "command SEEK should case-fold search keys for SQL UPPER(NAME) orders");
    }
    if (seek_cmd_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_cmd_rec->second) == "3", "command SEEK should position the SQL cursor on the normalized match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL temporary-order normalization checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]") != std::string::npos;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" &&
                event.detail.find("UPPER(NAME) [norm=upper, coll=case-folded]: charlie -> found") != std::string::npos;
        }),
        "SQL temporary-order normalization should emit runtime.order and runtime.seek metadata");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_temporary_order_direction_suffix_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_direction_suffix_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_direction_suffix_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET NEAR ON\n"
        "lSeekNearDesc = SEEK('beta', 'sqlcust', 'UPPER(NAME) DESCENDING')\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "lIndexMoveDesc = INDEXSEEK('beta', .T., 'sqlcust', 'UPPER(NAME) DESCENDING')\n"
        "nIndexRec = RECNO()\n"
        "cOrderAfter = ORDER()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL temporary-order direction-suffix parity script should complete");
    expect(state.sql_connections.empty(), "SQL temporary-order direction-suffix parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_near_desc = state.globals.find("lseekneardesc");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto index_move_desc = state.globals.find("lindexmovedesc");
    const auto index_rec = state.globals.find("nindexrec");
    const auto order_after = state.globals.find("corderafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL temporary-order direction suffix parity");
    expect(seek_near_desc != state.globals.end(), "SEEK() with UPPER(NAME) DESCENDING should be captured for a SQL cursor");
    expect(near_found != state.globals.end(), "FOUND() after SQL descending SEEK() miss should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SQL descending SEEK() miss should be captured");
    expect(index_move_desc != state.globals.end(), "INDEXSEEK(.T.) with UPPER(NAME) DESCENDING should be captured");
    expect(index_rec != state.globals.end(), "RECNO() after SQL descending INDEXSEEK(.T.) should be captured");
    expect(order_after != state.globals.end(), "ORDER() after SQL descending temporary-order probes should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL temporary-order direction suffix parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL temporary-order direction suffix checks");
    }
    if (seek_near_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_near_desc->second) == "false", "descending SQL SEEK() should still report a miss for an in-between key");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "FOUND() should stay false after a descending SQL SEEK() miss");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "1", "descending SQL SEEK() should near-position to the next row in descending order after case-folding");
    }
    if (index_move_desc != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_move_desc->second) == "false", "descending SQL INDEXSEEK(.T.) should still report a miss for an in-between key");
    }
    if (index_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_rec->second) == "1", "descending SQL INDEXSEEK(.T.) should move to the descending near-match row after case-folding");
    }
    if (order_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(order_after->second).empty(), "one-off SQL descending temporary-order probes should not permanently change ORDER()");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL temporary-order direction suffix checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_command_seek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_seek_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_seek_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO NAME\n"
        "cOrder = ORDER()\n"
        "SEEK 'CHARLIE'\n"
        "lFoundExact = FOUND()\n"
        "nRecExact = RECNO()\n"
        "SET NEAR ON\n"
        "SEEK 'BETA'\n"
        "lFoundNear = FOUND()\n"
        "nRecNear = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path seek parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path seek parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto order = state.globals.find("corder");
    const auto found_exact = state.globals.find("lfoundexact");
    const auto rec_exact = state.globals.find("nrecexact");
    const auto found_near = state.globals.find("lfoundnear");
    const auto rec_near = state.globals.find("nrecnear");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL command-path seek parity");
    expect(order != state.globals.end(), "ORDER() after SQL SET ORDER should be captured");
    expect(found_exact != state.globals.end(), "FOUND() after SQL command SEEK exact match should be captured");
    expect(rec_exact != state.globals.end(), "RECNO() after SQL command SEEK exact match should be captured");
    expect(found_near != state.globals.end(), "FOUND() after SQL command SEEK miss should be captured");
    expect(rec_near != state.globals.end(), "RECNO() after SQL command SEEK miss should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL command-path seek parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL command-path seek checks");
    }
    if (order != state.globals.end()) {
        expect(copperfin::runtime::format_value(order->second) == "NAME", "SET ORDER TO NAME should establish a synthetic SQL order expression");
    }
    if (found_exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_exact->second) == "true", "command SEEK should find the synthetic SQL row");
    }
    if (rec_exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_exact->second) == "3", "command SEEK should position the SQL cursor on the matching row");
    }
    if (found_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_near->second) == "false", "command SEEK should report a miss for an in-between SQL key");
    }
    if (rec_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_near->second) == "2", "SET NEAR plus command SEEK should position the SQL cursor to the next synthetic row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL command-path SEEK checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" && event.detail == "NAME";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" && event.detail.find("NAME: BETA -> not found") != std::string::npos;
        }),
        "SQL command-path SET ORDER and SEEK should emit runtime.order and runtime.seek events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_command_seek_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_seek_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_seek_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO NAME IN sqlcust\n"
        "cCustOrder = ORDER('sqlcust')\n"
        "cOtherOrder = ORDER('sqlother')\n"
        "SEEK 'CHARLIE' IN sqlcust\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "SELECT sqlcust\n"
        "nCustRecAfterSeek = RECNO()\n"
        "cCustNameAfterSeek = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path SEEK IN target parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path SEEK IN target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_order = state.globals.find("ccustorder");
    const auto other_order = state.globals.find("cotherorder");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_rec_after_seek = state.globals.find("ncustrecafterseek");
    const auto cust_name_after_seek = state.globals.find("ccustnameafterseek");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL command-path SEEK IN target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL command-path SEEK IN target parity");
    expect(other_rec_before != state.globals.end(), "RECNO() before targeted SQL SEEK should be captured");
    expect(cust_order != state.globals.end(), "ORDER('sqlcust') after targeted SQL SET ORDER should be captured");
    expect(other_order != state.globals.end(), "ORDER('sqlother') after targeted SQL SET ORDER should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted SQL SEEK should be captured");
    expect(other_rec_after != state.globals.end(), "RECNO() on selected SQL cursor after targeted SEEK should be captured");
    expect(cust_rec_after_seek != state.globals.end(), "RECNO() on targeted SQL cursor after targeted SEEK should be captured");
    expect(cust_name_after_seek != state.globals.end(), "NAME on targeted SQL cursor after targeted SEEK should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL command-path SEEK IN target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL SEEK checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL SEEK checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "1", "selected SQL cursor should begin at first record");
    }
    if (cust_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_order->second) == "NAME", "SET ORDER TO ... IN sqlcust should affect the targeted SQL cursor");
    }
    if (other_order != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_order->second).empty(), "SET ORDER TO ... IN sqlcust should not alter the selected non-target SQL cursor");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "SQLOTHER", "SEEK ... IN sqlcust should preserve the current selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "1", "SEEK ... IN sqlcust should not move the selected non-target SQL cursor pointer");
    }
    if (cust_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_seek->second) == "3", "SEEK ... IN sqlcust should move the targeted SQL cursor pointer to the match");
    }
    if (cust_name_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_name_after_seek->second) == "CHARLIE", "SEEK ... IN sqlcust should expose the targeted SQL row values");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL SEEK checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order" && event.detail == "NAME";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek" && event.detail.find("NAME: CHARLIE -> found") != std::string::npos;
        }),
        "SQL command-path SET ORDER ... IN and SEEK ... IN should emit runtime.order and runtime.seek events for targeted SQL cursors");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_scan_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_scan_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_scan_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "nScanHits = 0\n"
        "SCAN FOR AMOUNT >= 20 IN sqlcust\n"
        "    nScanHits = nScanHits + 1\n"
        "ENDSCAN\n"
        "cAliasAfterScan = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nCustRecAfter = RECNO('sqlcust')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL SCAN IN target parity script should complete");
    expect(state.sql_connections.empty(), "SQL SCAN IN target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto scan_hits = state.globals.find("nscanhits");
    const auto alias_after_scan = state.globals.find("caliasafterscan");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_rec_after = state.globals.find("ncustrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL SCAN IN target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL SCAN IN target parity");
    expect(other_rec_before != state.globals.end(), "selected SQL cursor RECNO() before targeted SCAN should be captured");
    expect(cust_rec_before != state.globals.end(), "target SQL cursor RECNO() before targeted SCAN should be captured");
    expect(scan_hits != state.globals.end(), "SCAN FOR ... IN sqlcust hit count should be captured");
    expect(alias_after_scan != state.globals.end(), "ALIAS() after targeted SQL SCAN should be captured");
    expect(other_rec_after != state.globals.end(), "selected SQL cursor RECNO() after targeted SCAN should be captured");
    expect(cust_rec_after != state.globals.end(), "target SQL cursor RECNO() after targeted SCAN should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL SCAN IN target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL SCAN checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL SCAN checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should be on the bottom record before targeted SCAN");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "1", "target SQL cursor should start on its first record before targeted SCAN");
    }
    if (scan_hits != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_hits->second) == "2", "SCAN FOR ... IN sqlcust should iterate matching rows on the targeted SQL cursor");
    }
    if (alias_after_scan != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_scan->second)) == "SQLOTHER", "SCAN ... IN sqlcust should preserve the currently selected SQL alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "SCAN ... IN sqlcust should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after->second) == "4", "targeted SQL SCAN should leave the targeted cursor just past the last record");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL SCAN checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.scan" && event.detail == "AMOUNT >= 20";
        }),
        "SCAN ... IN sqlcust should emit a runtime.scan event with the targeted filter expression");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_order_direction_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_order_direction_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_order_direction_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO NAME IN sqlcust DESCENDING\n"
        "SET NEAR ON\n"
        "SEEK 'BETA' IN sqlcust\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nCustRecAfterSeek = RECNO('sqlcust')\n"
        "SET NEAR OFF\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL order direction IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL order direction IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_rec_after_seek = state.globals.find("ncustrecafterseek");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL order direction IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL order direction IN-target parity");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted descending seek should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted descending seek should be captured");
    expect(other_rec_after != state.globals.end(), "Selected SQL cursor RECNO() after targeted descending seek should be captured");
    expect(cust_rec_after_seek != state.globals.end(), "Target SQL cursor RECNO() after targeted descending seek should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL order direction IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted descending seek checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted descending seek checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted descending seek");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "SQLOTHER", "SEEK ... IN should preserve the selected SQL alias with descending order");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "SEEK ... IN should preserve selected SQL cursor pointer with descending order");
    }
    if (cust_rec_after_seek != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_seek->second) == "1", "descending SET ORDER ... IN plus SET NEAR should position targeted SQL cursor on descending near-match record");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted descending seek checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.order";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.seek";
        }),
        "targeted descending SQL order/seek flow should emit runtime.order and runtime.seek events");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

void test_sql_result_cursor_sql_style_mutation_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_style_mutation_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_style_mutation_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "nBeforeTarget = RECCOUNT('sqlcust')\n"
        "INSERT INTO sqlcust (ID, NAME, AMOUNT) VALUES (4, 'DELTA', 44)\n"
        "INSERT INTO sqlcust VALUES (5, 'ECHO', 55)\n"
        "DELETE FROM sqlcust WHERE NAME = 'BRAVO'\n"
        "cAliasAfter = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "nAfterTarget = RECCOUNT('sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'DELTA'\n"
        "nDeltaId = ID\n"
        "cDeltaName = NAME\n"
        "nDeltaAmount = AMOUNT\n"
        "lDeltaDeleted = DELETED()\n"
        "LOCATE FOR NAME = 'ECHO'\n"
        "nEchoId = ID\n"
        "cEchoName = NAME\n"
        "nEchoAmount = AMOUNT\n"
        "lEchoDeleted = DELETED()\n"
        "GO 2\n"
        "cBravoName = NAME\n"
        "lBravoDeleted = DELETED()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL-style mutation parity script should complete");
    expect(state.sql_connections.empty(), "SQL-style mutation parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto before_target = state.globals.find("nbeforetarget");
    const auto alias_after = state.globals.find("caliasafter");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto after_target = state.globals.find("naftertarget");
    const auto delta_id = state.globals.find("ndeltaid");
    const auto delta_name = state.globals.find("cdeltaname");
    const auto delta_amount = state.globals.find("ndeltaamount");
    const auto delta_deleted = state.globals.find("ldeltadeleted");
    const auto echo_id = state.globals.find("nechoid");
    const auto echo_name = state.globals.find("cechoname");
    const auto echo_amount = state.globals.find("nechoamount");
    const auto echo_deleted = state.globals.find("lechodeleted");
    const auto bravo_name = state.globals.find("cbravoname");
    const auto bravo_deleted = state.globals.find("lbravodeleted");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL-style mutation parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL-style mutation parity");
    expect(alias_before != state.globals.end(), "Selected alias before SQL-style mutation should be captured");
    expect(other_rec_before != state.globals.end(), "Selected cursor pointer before SQL-style mutation should be captured");
    expect(before_target != state.globals.end(), "Target SQL cursor RECCOUNT() before INSERT INTO should be captured");
    expect(alias_after != state.globals.end(), "Selected alias after SQL-style mutation should be captured");
    expect(other_rec_after != state.globals.end(), "Selected cursor pointer after SQL-style mutation should be captured");
    expect(after_target != state.globals.end(), "Target SQL cursor RECCOUNT() after INSERT INTO should be captured");
    expect(delta_id != state.globals.end(), "field-list INSERT INTO should expose appended SQL ID");
    expect(delta_name != state.globals.end(), "field-list INSERT INTO should expose appended SQL NAME");
    expect(delta_amount != state.globals.end(), "field-list INSERT INTO should expose appended SQL AMOUNT");
    expect(delta_deleted != state.globals.end(), "field-list INSERT INTO should create a live SQL row");
    expect(echo_id != state.globals.end(), "schema-order INSERT INTO should expose appended SQL ID");
    expect(echo_name != state.globals.end(), "schema-order INSERT INTO should expose appended SQL NAME");
    expect(echo_amount != state.globals.end(), "schema-order INSERT INTO should expose appended SQL AMOUNT");
    expect(echo_deleted != state.globals.end(), "schema-order INSERT INTO should create a live SQL row");
    expect(bravo_name != state.globals.end(), "DELETE FROM should leave the matched SQL row readable");
    expect(bravo_deleted != state.globals.end(), "DELETE FROM should expose the matched SQL tombstone state");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL-style mutation parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before SQL-style mutation checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before SQL-style mutation checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should start on sqlother before SQL-style mutations");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before SQL-style mutations");
    }
    if (before_target != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_target->second) == "3", "synthetic SQL result cursor should start with three rows before SQL-style inserts");
    }
    if (alias_after != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after->second)) == "SQLOTHER", "INSERT INTO / DELETE FROM should preserve the selected SQL alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "INSERT INTO / DELETE FROM should preserve the selected SQL cursor pointer");
    }
    if (after_target != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_target->second) == "5", "two SQL-style INSERT INTO commands should append two synthetic SQL rows");
    }
    if (delta_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(delta_id->second) == "4", "field-list INSERT INTO should map SQL ID by field name");
    }
    if (delta_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(delta_name->second) == "DELTA", "field-list INSERT INTO should map SQL NAME by field name");
    }
    if (delta_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(delta_amount->second) == "44", "field-list INSERT INTO should map SQL AMOUNT by field name");
    }
    if (delta_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(delta_deleted->second) == "false", "field-list INSERT INTO should append a non-deleted SQL row");
    }
    if (echo_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(echo_id->second) == "5", "schema-order INSERT INTO should map SQL ID by schema order");
    }
    if (echo_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(echo_name->second) == "ECHO", "schema-order INSERT INTO should map SQL NAME by schema order");
    }
    if (echo_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(echo_amount->second) == "55", "schema-order INSERT INTO should map SQL AMOUNT by schema order");
    }
    if (echo_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(echo_deleted->second) == "false", "schema-order INSERT INTO should append a non-deleted SQL row");
    }
    if (bravo_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bravo_name->second) == "BRAVO", "DELETE FROM should match the requested synthetic SQL row");
    }
    if (bravo_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(bravo_deleted->second) == "true", "DELETE FROM should tombstone the matching synthetic SQL row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL-style mutation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.insert_into";
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.delete_from";
        }),
        "SQL-style mutation commands should emit INSERT INTO and DELETE FROM runtime events for synthetic SQL cursors");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_mutation_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutation_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutation_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "GO BOTTOM IN sqlcust\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "REPLACE NAME WITH 'CHARLIEX' IN sqlcust\n"
        "cAliasAfterReplace = ALIAS()\n"
        "nOtherRecAfterReplace = RECNO()\n"
        "nCustRecAfterReplace = RECNO('sqlcust')\n"
        "DELETE FOR NAME = 'BRAVO' IN sqlcust\n"
        "cAliasAfterDelete = ALIAS()\n"
        "nOtherRecAfterDelete = RECNO()\n"
        "nCustRecAfterDelete = RECNO('sqlcust')\n"
        "RECALL FOR NAME = 'BRAVO' IN sqlcust\n"
        "cAliasAfterRecall = ALIAS()\n"
        "nOtherRecAfterRecall = RECNO()\n"
        "nCustRecAfterRecall = RECNO('sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'CHARLIEX'\n"
        "cTargetReplacedName = NAME\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "lTargetBravoDeleted = DELETED()\n"
        "SELECT sqlother\n"
        "cAliasFinal = ALIAS()\n"
        "nOtherRecFinal = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL mutation IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL mutation IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto alias_after_replace = state.globals.find("caliasafterreplace");
    const auto other_rec_after_replace = state.globals.find("notherrecafterreplace");
    const auto cust_rec_after_replace = state.globals.find("ncustrecafterreplace");
    const auto alias_after_delete = state.globals.find("caliasafterdelete");
    const auto other_rec_after_delete = state.globals.find("notherrecafterdelete");
    const auto cust_rec_after_delete = state.globals.find("ncustrecafterdelete");
    const auto alias_after_recall = state.globals.find("caliasafterrecall");
    const auto other_rec_after_recall = state.globals.find("notherrecafterrecall");
    const auto cust_rec_after_recall = state.globals.find("ncustrecafterrecall");
    const auto target_replaced_name = state.globals.find("ctargetreplacedname");
    const auto target_bravo_deleted = state.globals.find("ltargetbravodeleted");
    const auto alias_final = state.globals.find("caliasfinal");
    const auto other_rec_final = state.globals.find("notherrecfinal");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL mutation IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL mutation IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL mutation commands should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted mutations should be captured");
    expect(cust_rec_before != state.globals.end(), "Target SQL cursor RECNO() before targeted mutations should be captured");
    expect(alias_after_replace != state.globals.end(), "Selected alias after REPLACE IN should be captured");
    expect(other_rec_after_replace != state.globals.end(), "Selected SQL cursor RECNO() after REPLACE IN should be captured");
    expect(cust_rec_after_replace != state.globals.end(), "Target SQL cursor RECNO() after REPLACE IN should be captured");
    expect(alias_after_delete != state.globals.end(), "Selected alias after DELETE FOR ... IN should be captured");
    expect(other_rec_after_delete != state.globals.end(), "Selected SQL cursor RECNO() after DELETE FOR ... IN should be captured");
    expect(cust_rec_after_delete != state.globals.end(), "Target SQL cursor RECNO() after DELETE FOR ... IN should be captured");
    expect(alias_after_recall != state.globals.end(), "Selected alias after RECALL FOR ... IN should be captured");
    expect(other_rec_after_recall != state.globals.end(), "Selected SQL cursor RECNO() after RECALL FOR ... IN should be captured");
    expect(cust_rec_after_recall != state.globals.end(), "Target SQL cursor RECNO() after RECALL FOR ... IN should be captured");
    expect(target_replaced_name != state.globals.end(), "Target SQL cursor REPLACE IN field update should be captured");
    expect(target_bravo_deleted != state.globals.end(), "Target SQL cursor DELETE/RECALL IN state should be captured");
    expect(alias_final != state.globals.end(), "Selected alias after targeted SQL mutation verification should be captured");
    expect(other_rec_final != state.globals.end(), "Selected SQL cursor RECNO() final position should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL mutation IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL mutation checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL mutation checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should start on sqlother before targeted mutations");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted mutations");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "3", "target SQL cursor should be positioned at bottom before REPLACE IN");
    }
    if (alias_after_replace != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_replace->second)) == "SQLOTHER", "REPLACE IN should preserve the selected SQL alias");
    }
    if (other_rec_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_replace->second) == "3", "REPLACE IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_replace != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_replace->second) == "3", "REPLACE IN should keep the targeted SQL cursor pointer on the current record");
    }
    if (alias_after_delete != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_delete->second)) == "SQLOTHER", "DELETE FOR ... IN should preserve the selected SQL alias");
    }
    if (other_rec_after_delete != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_delete->second) == "3", "DELETE FOR ... IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_delete != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_delete->second) == "3", "DELETE FOR ... IN should restore the targeted SQL cursor pointer");
    }
    if (alias_after_recall != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_recall->second)) == "SQLOTHER", "RECALL FOR ... IN should preserve the selected SQL alias");
    }
    if (other_rec_after_recall != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after_recall->second) == "3", "RECALL FOR ... IN should preserve the selected SQL cursor pointer");
    }
    if (cust_rec_after_recall != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_recall->second) == "3", "RECALL FOR ... IN should restore the targeted SQL cursor pointer");
    }
    if (target_replaced_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_replaced_name->second) == "CHARLIEX", "REPLACE IN should update the targeted SQL row fields");
    }
    if (target_bravo_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_bravo_deleted->second) == "false", "DELETE FOR ... IN followed by RECALL FOR ... IN should leave the targeted SQL row recalled");
    }
    if (alias_final != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_final->second)) == "SQLOTHER", "selected SQL alias should remain on sqlother at the end of targeted mutation checks");
    }
    if (other_rec_final != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_final->second) == "3", "selected SQL cursor pointer should remain unchanged at the end of targeted mutation checks");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL mutation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "targeted SQL mutation commands should emit runtime.replace, runtime.delete, and runtime.recall events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_navigation_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_navigation_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_navigation_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterGoTop = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkip = RECNO('sqlcust')\n"
        "LOCATE FOR AMOUNT = 30 IN sqlcust\n"
        "nCustRecAfterLocate = RECNO('sqlcust')\n"
        "GO 99 IN sqlcust\n"
        "nCustRecAfterGoEdge = RECNO('sqlcust')\n"
        "cAliasAfterCommands = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL navigation IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL navigation IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto cust_rec_after_go_top = state.globals.find("ncustrecaftergotop");
    const auto cust_rec_after_skip = state.globals.find("ncustrecafterskip");
    const auto cust_rec_after_locate = state.globals.find("ncustrecafterlocate");
    const auto cust_rec_after_go_edge = state.globals.find("ncustrecaftergoedge");
    const auto alias_after_commands = state.globals.find("caliasaftercommands");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL navigation IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL navigation IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL navigation commands should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted navigation should be captured");
    expect(cust_rec_before != state.globals.end(), "Target SQL cursor RECNO() before targeted navigation should be captured");
    expect(cust_rec_after_go_top != state.globals.end(), "Target SQL cursor RECNO() after GO TOP IN should be captured");
    expect(cust_rec_after_skip != state.globals.end(), "Target SQL cursor RECNO() after SKIP IN should be captured");
    expect(cust_rec_after_locate != state.globals.end(), "Target SQL cursor RECNO() after LOCATE IN should be captured");
    expect(cust_rec_after_go_edge != state.globals.end(), "Target SQL cursor RECNO() after GO edge IN should be captured");
    expect(alias_after_commands != state.globals.end(), "Selected alias after targeted SQL navigation commands should be captured");
    expect(other_rec_after != state.globals.end(), "Selected SQL cursor RECNO() after targeted navigation should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL navigation IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL navigation checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL navigation checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should start on sqlother before targeted navigation");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted navigation");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "1", "target SQL cursor should start at first record before targeted navigation");
    }
    if (cust_rec_after_go_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_top->second) == "1", "GO TOP IN should reposition the targeted SQL cursor to first record");
    }
    if (cust_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip->second) == "2", "SKIP IN should move the targeted SQL cursor pointer");
    }
    if (cust_rec_after_locate != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_locate->second) == "3", "LOCATE ... IN should position the targeted SQL cursor on the match");
    }
    if (cust_rec_after_go_edge != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_edge->second) == "4", "GO 99 IN should move targeted SQL cursor to EOF position");
    }
    if (alias_after_commands != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_commands->second)) == "SQLOTHER", "targeted SQL navigation commands should preserve the selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL navigation commands should preserve the selected SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL navigation checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.go"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.skip"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }),
        "targeted SQL GO/SKIP/LOCATE commands should emit runtime navigation events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_filter_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_filter_in_target_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_filter_in_target_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "cAliasBefore = ALIAS()\n"
        "nOtherRecBefore = RECNO()\n"
        "SET FILTER TO AMOUNT >= 20 IN sqlcust\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterGoTop = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkip = RECNO('sqlcust')\n"
        "SKIP 1 IN sqlcust\n"
        "nCustRecAfterSkipEdge = RECNO('sqlcust')\n"
        "SET FILTER TO IN sqlcust\n"
        "GO TOP IN sqlcust\n"
        "nCustRecAfterFilterOff = RECNO('sqlcust')\n"
        "cAliasAfter = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL filter IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL filter IN-target parity script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto alias_before = state.globals.find("caliasbefore");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_after_go_top = state.globals.find("ncustrecaftergotop");
    const auto cust_rec_after_skip = state.globals.find("ncustrecafterskip");
    const auto cust_rec_after_skip_edge = state.globals.find("ncustrecafterskipedge");
    const auto cust_rec_after_filter_off = state.globals.find("ncustrecafterfilteroff");
    const auto alias_after = state.globals.find("caliasafter");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for SQL filter IN-target parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for SQL filter IN-target parity");
    expect(alias_before != state.globals.end(), "Selected alias before targeted SQL filter flow should be captured");
    expect(other_rec_before != state.globals.end(), "Selected SQL cursor RECNO() before targeted SQL filter flow should be captured");
    expect(cust_rec_after_go_top != state.globals.end(), "Target SQL cursor RECNO() after GO TOP IN with filter should be captured");
    expect(cust_rec_after_skip != state.globals.end(), "Target SQL cursor RECNO() after SKIP IN with filter should be captured");
    expect(cust_rec_after_skip_edge != state.globals.end(), "Target SQL cursor RECNO() after filtered SKIP edge should be captured");
    expect(cust_rec_after_filter_off != state.globals.end(), "Target SQL cursor RECNO() after clearing filter should be captured");
    expect(alias_after != state.globals.end(), "Selected alias after targeted SQL filter flow should be captured");
    expect(other_rec_after != state.globals.end(), "Selected SQL cursor RECNO() after targeted SQL filter flow should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL filter IN-target parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL filter checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL filter checks");
    }
    if (alias_before != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_before->second)) == "SQLOTHER", "selected SQL alias should remain on sqlother before targeted filter flow");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted filter flow");
    }
    if (cust_rec_after_go_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_go_top->second) == "2", "GO TOP IN should honor targeted SQL filter visibility");
    }
    if (cust_rec_after_skip != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip->second) == "3", "SKIP IN should move across filtered visible SQL rows");
    }
    if (cust_rec_after_skip_edge != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_skip_edge->second) == "4", "filtered SKIP IN edge should move targeted SQL cursor to EOF position");
    }
    if (cust_rec_after_filter_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_filter_off->second) == "1", "clearing targeted SQL filter should restore full GO TOP visibility");
    }
    if (alias_after != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after->second)) == "SQLOTHER", "targeted SQL filter flow should preserve selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL filter flow should preserve selected SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL filter checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.filter"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.go"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.skip"; }),
        "targeted SQL SET FILTER/GO/SKIP flow should emit runtime.filter and navigation events");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_cursor_identity_functions_for_sql_result_cursors();
    test_sql_result_cursor_mutation_commands();
    test_sql_result_cursors_are_isolated_by_data_session();
    test_sql_result_cursor_auto_allocation_tracks_session_selection_flow();
    test_sql_result_cursors_and_ole_actions();
    test_sql_result_cursor_read_only_parity();
    test_sql_result_cursor_seek_parity();
    test_sql_result_cursor_temporary_order_normalization_parity();
    test_sql_result_cursor_temporary_order_direction_suffix_parity();
    test_sql_result_cursor_command_seek_parity();
    test_sql_result_cursor_command_seek_in_target_parity();
    test_sql_result_cursor_scan_in_target_parity();
    test_sql_result_cursor_order_direction_in_target_parity();
    test_sql_result_cursor_mutation_parity();
    test_sql_result_cursor_sql_style_mutation_parity();
    test_sql_result_cursor_mutation_in_target_parity();
    test_sql_result_cursor_navigation_in_target_parity();
    test_sql_result_cursor_filter_in_target_parity();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
