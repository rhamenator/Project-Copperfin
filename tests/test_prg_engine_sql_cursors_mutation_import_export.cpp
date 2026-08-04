// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

void test_sql_result_cursor_multi_field_replace_uses_original_values_for_later_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_replace_original_values";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_replace_original_values.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "REPLACE NAME WITH 'Q', AMOUNT WITH LEN(NAME)\n"
        "cAfterName = NAME\n"
        "nAfterAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL multi-field REPLACE original-value script should complete");
    expect(state.sql_connections.empty(), "SQL multi-field REPLACE original-value script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto after_name = state.globals.find("caftername");
    const auto after_amount = state.globals.find("nafteramount");
    const auto disc = state.globals.find("ldisc");
    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL multi-field REPLACE original-value parity");
    expect(after_name != state.globals.end(), "SQL multi-field REPLACE should expose the updated NAME");
    expect(after_amount != state.globals.end(), "SQL multi-field REPLACE should expose the later AMOUNT expression result");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL multi-field REPLACE original-value parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL multi-field REPLACE checks");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Q", "SQL multi-field REPLACE should still update the first assignment");
    }
    if (after_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_amount->second) == "5", "later SQL REPLACE expressions should read the original NAME value before any assignments are applied");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL multi-field REPLACE checks");
    }

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_append_from_json_mutates_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path json_path = temp_root / "new_rows.json";
    const fs::path main_path = temp_root / "sql_append_from_json.prg";

    write_text(json_path.string(),
        "[{\"ID\":\"801\",\"NAME\":\"FOXTROT\",\"AMOUNT\":\"9.00\"},"
        "{\"ID\":\"802\",\"NAME\":\"GOLF\",\"AMOUNT\":\"10.50\"}]");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + json_path.string() + "' TYPE JSON\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE JSON selected SQL result-cursor script should complete: " + state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(), "SQL cursor row count before APPEND FROM JSON should be captured");
    expect(rows_after != state.globals.end(), "SQL cursor row count after APPEND FROM JSON should be captured");
    expect(bottom_id != state.globals.end(), "SQL cursor bottom ID after APPEND FROM JSON should be captured");
    expect(bottom_name != state.globals.end(), "SQL cursor bottom NAME after APPEND FROM JSON should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after APPEND FROM JSON checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
            "selected SQL result cursor should start with seeded row count before APPEND FROM JSON");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "5",
            "APPEND FROM TYPE JSON should add 2 rows to the selected SQL/result cursor");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "802",
            "APPEND FROM TYPE JSON should set last row ID in selected SQL/result cursor");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "GOLF",
            "APPEND FROM TYPE JSON should set last row NAME in selected SQL/result cursor");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after APPEND FROM TYPE JSON checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_csv_mutates_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path csv_path = temp_root / "new_rows.csv";
    const fs::path main_path = temp_root / "sql_append_from_csv.prg";

    write_text(csv_path.string(),
        "ID,NAME,AMOUNT\n"
        "901,HOTEL,11.00\n"
        "902,INDIA,12.50\n");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + csv_path.string() + "' TYPE CSV\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE CSV selected SQL result-cursor script should complete: " + state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(), "SQL cursor row count before APPEND FROM CSV should be captured");
    expect(rows_after != state.globals.end(), "SQL cursor row count after APPEND FROM CSV should be captured");
    expect(bottom_id != state.globals.end(), "SQL cursor bottom ID after APPEND FROM CSV should be captured");
    expect(bottom_name != state.globals.end(), "SQL cursor bottom NAME after APPEND FROM CSV should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after APPEND FROM CSV checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
            "selected SQL result cursor should start with seeded row count before APPEND FROM CSV");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "5",
            "APPEND FROM TYPE CSV should add 2 rows to the selected SQL/result cursor");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "902",
            "APPEND FROM TYPE CSV should set last row ID in selected SQL/result cursor");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "INDIA",
            "APPEND FROM TYPE CSV should set last row NAME in selected SQL/result cursor");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after APPEND FROM TYPE CSV checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_delimited_fields_clause_preserves_typed_order_for_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_delimited_fields_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path delimited_path = temp_root / "new_rows.txt";
    const fs::path main_path = temp_root / "sql_append_from_delimited_fields_order.prg";

    write_text(delimited_path.string(),
        "\"HOTEL\",11.00,901\n"
        "\"INDIA\",12.50,902\n");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + delimited_path.string() +
            "' DELIMITED WITH CHARACTER ',' FIELDS NAME, AMOUNT, ID\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "nBottomAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           "#3692: APPEND FROM DELIMITED with reordered explicit fields should mutate the selected SQL/result cursor: " +
               state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto bottom_amount = state.globals.find("nbottomamount");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(),
           "#3692: selected SQL/result cursor reordered-fields row count before APPEND FROM should be captured");
    expect(rows_after != state.globals.end(),
           "#3692: selected SQL/result cursor reordered-fields row count after APPEND FROM should be captured");
    expect(bottom_id != state.globals.end(),
           "#3692: selected SQL/result cursor reordered-fields bottom ID should be captured");
    expect(bottom_name != state.globals.end(),
           "#3692: selected SQL/result cursor reordered-fields bottom NAME should be captured");
    expect(bottom_amount != state.globals.end(),
           "#3692: selected SQL/result cursor reordered-fields bottom AMOUNT should be captured");
    expect(disc != state.globals.end(),
           "#3692: SQLDISCONNECT result should be captured after reordered-fields APPEND FROM checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
               "#3692: selected SQL/result cursor should start with seeded row count before reordered-fields APPEND FROM");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "5",
               "#3692: reordered-fields APPEND FROM should add two rows to the selected SQL/result cursor");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "902",
               "#3692: reordered-fields APPEND FROM should map the third source column into ID");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "INDIA",
               "#3692: reordered-fields APPEND FROM should map the first source column into NAME");
    }
    if (bottom_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_amount->second) == "12.5",
               "#3692: reordered-fields APPEND FROM should map the second source column into AMOUNT");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
               "#3692: SQLDISCONNECT should succeed after reordered-fields APPEND FROM checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_selected_sql_result_cursor_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path main_path = temp_root / "sql_append_from_localization.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "APPEND FROM '" + (temp_root / "unsupported.xls").string() + "' TYPE XLS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#2709: qps-ploc APPEND FROM TYPE XLS against a selected SQL result cursor should fail");
    expect(
        state.message ==
            copperfin::localization::pseudo_localize("APPEND FROM: selected SQL/result cursor does not support this source type"),
        "#2709: qps-ploc APPEND FROM SQL/result source-type error should route through the pseudo-localization transform");

    write_people_dbf(temp_root / "source.dbf", {{"Alpha", 1}});

    const fs::path fields_main_path = temp_root / "sql_append_from_fields_localization.prg";
    write_text(
        fields_main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "APPEND FROM '" + (temp_root / "source.dbf").string() + "' FIELDS MissingField\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession fields_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(fields_main_path, temp_root));

    const auto fields_state = fields_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!fields_state.completed, "#2709: qps-ploc APPEND FROM selected SQL result cursor with no matching fields should fail");
    expect(
        fields_state.message ==
            copperfin::localization::pseudo_localize("APPEND FROM: no fields match the FIELDS clause"),
        "#2709: qps-ploc APPEND FROM empty-fields error should route through the pseudo-localization transform");

    const fs::path type_open_main_path = temp_root / "sql_append_from_type_open_localization.prg";
    write_text(
        type_open_main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "APPEND FROM '" + (temp_root / "missing.json").string() + "' TYPE JSON\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession type_open_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(type_open_main_path, temp_root));

    const auto type_open_state = type_open_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!type_open_state.completed, "#2710: qps-ploc APPEND FROM TYPE JSON missing-source selected SQL result cursor script should fail");
    expect(
        type_open_state.message.find("[!! ") == 0U &&
            type_open_state.message.find("JSON") != std::string::npos &&
            type_open_state.message.find("unable to open source file") == std::string::npos,
        "#2710: qps-ploc APPEND FROM TYPE open-source error should pseudo-localize prose while preserving the type");

    fs::remove_all(temp_root, ignored);
}

void test_sql_plain_temporary_order_in_target_honors_collate_and_preserves_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_plain_temp_order_collate_in_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_plain_temp_order_collate_in_target.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO()\n"
        "SET ORDER TO NAME IN sqlcust\n"
        "lMachineMiss = SEEK('bravo', 'sqlcust', 'NAME')\n"
        "nCustRecAfterMachine = RECNO('sqlcust')\n"
        "SET COLLATE TO GENERAL\n"
        "GO TOP IN sqlcust\n"
        "lGeneralHit = SEEK('bravo', 'sqlcust', 'NAME')\n"
        "SELECT sqlcust\n"
        "cCustNameAfterGeneral = NAME\n"
        "SELECT sqlother\n"
        "cAliasAfterSeek = ALIAS()\n"
        "nOtherRecAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL plain temporary-order IN-target collate script should complete");
    expect(state.sql_connections.empty(), "SQL plain temporary-order IN-target collate script should disconnect its SQL handle");

    const auto exec_cust = state.globals.find("nexeccust");
    const auto exec_other = state.globals.find("nexecother");
    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto machine_miss = state.globals.find("lmachinemiss");
    const auto cust_rec_after_machine = state.globals.find("ncustrecaftermachine");
    const auto general_hit = state.globals.find("lgeneralhit");
    const auto cust_name_after_general = state.globals.find("ccustnameaftergeneral");
    const auto alias_after_seek = state.globals.find("caliasafterseek");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(exec_cust != state.globals.end(), "First SQLEXEC result should be captured for targeted SQL collate seek parity");
    expect(exec_other != state.globals.end(), "Second SQLEXEC result should be captured for targeted SQL collate seek parity");
    expect(other_rec_before != state.globals.end(), "selected SQL cursor RECNO() before targeted collate seek should be captured");
    expect(machine_miss != state.globals.end(), "MACHINE-collate targeted SQL SEEK() miss should be captured");
    expect(cust_rec_after_machine != state.globals.end(), "target SQL cursor RECNO() after MACHINE-collate seek should be captured");
    expect(general_hit != state.globals.end(), "GENERAL-collate targeted SQL SEEK() hit should be captured");
    expect(cust_name_after_general != state.globals.end(), "target SQL cursor NAME after GENERAL-collate seek should be captured");
    expect(alias_after_seek != state.globals.end(), "ALIAS() after targeted SQL collate seek should be captured");
    expect(other_rec_after != state.globals.end(), "selected SQL cursor RECNO() after targeted collate seek should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for targeted SQL collate seek parity");

    if (exec_cust != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_cust->second) == "1", "First SQLEXEC should succeed before targeted SQL collate seek checks");
    }
    if (exec_other != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_other->second) == "1", "Second SQLEXEC should succeed before targeted SQL collate seek checks");
    }
    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected non-target SQL cursor should begin at bottom");
    }
    if (machine_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(machine_miss->second) == "false", "MACHINE collation should keep plain NAME seek case-sensitive in targeted SQL cursor");
    }
    if (cust_rec_after_machine != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_machine->second) == "4", "MACHINE-collate miss should position the targeted SQL cursor at EOF");
    }
    if (general_hit != state.globals.end()) {
        expect(copperfin::runtime::format_value(general_hit->second) == "true", "GENERAL collation should case-fold plain NAME seek in targeted SQL cursor");
    }
    if (cust_name_after_general != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_name_after_general->second) == "BRAVO", "GENERAL-collate targeted SQL seek should expose the case-folded match row");
    }
    if (alias_after_seek != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_seek->second)) == "SQLOTHER", "targeted SQL SEEK() should preserve the selected non-target alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL SEEK() should preserve the selected non-target SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted SQL collate seek checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_json_for_filters_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_json_for";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path json_path = temp_root / "for_rows.json";
    const fs::path main_path = temp_root / "sql_append_from_json_for.prg";
    write_text(json_path.string(),
        "[{\"ID\":\"811\",\"NAME\":\"JULIET\",\"AMOUNT\":\"13.25\"},"
        "{\"ID\":\"812\",\"NAME\":\"KILO\",\"AMOUNT\":\"9.50\"}]");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + json_path.string() + "' TYPE JSON FOR VAL(AMOUNT) >= 10\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE JSON FOR selected SQL result-cursor script should complete: " + state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(), "selected SQL cursor row count before APPEND FROM TYPE JSON FOR should be captured");
    expect(rows_after != state.globals.end(), "selected SQL cursor row count after APPEND FROM TYPE JSON FOR should be captured");
    expect(bottom_id != state.globals.end(), "selected SQL cursor bottom ID after APPEND FROM TYPE JSON FOR should be captured");
    expect(bottom_name != state.globals.end(), "selected SQL cursor bottom NAME after APPEND FROM TYPE JSON FOR should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after APPEND FROM TYPE JSON FOR checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
               "selected SQL cursor should start with seeded row count before APPEND FROM TYPE JSON FOR");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "4",
               "APPEND FROM TYPE JSON FOR should append only matching rows");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "811",
               "APPEND FROM TYPE JSON FOR should append only the matching ID");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "JULIET",
               "APPEND FROM TYPE JSON FOR should append only the matching row payload");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
               "SQLDISCONNECT should succeed after APPEND FROM TYPE JSON FOR checks");
    }

    fs::remove_all(temp_root, ignored);
}


}  // namespace copperfin::sql_cursor_mutation_tests

