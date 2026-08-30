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

namespace {

using namespace copperfin::test_support;

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

#include "test_prg_engine_sql_cursors_seek_and_order_filter_deferred.inl"

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
        "SET FILTER TO ID >= 2\n"
        "lFilteredVisible = SEEK('BRAVO', 'sqlcust', 'NAME')\n"
        "nFilteredVisibleRec = RECNO()\n"
        "SET NEAR OFF\n"
        "lFilteredHidden = SEEK('ALPHA', 'sqlcust', 'NAME')\n"
        "nFilteredHiddenRec = RECNO()\n"
        "lFilteredHiddenEof = EOF()\n"
        "SET NEAR ON\n"
        "lFilteredNear = SEEK('ALPHA', 'sqlcust', 'NAME')\n"
        "nFilteredNearRec = RECNO()\n"
        "GO 3\n"
        "nFilteredIndexBefore = RECNO()\n"
        "lFilteredIndexHidden = INDEXSEEK('ALPHA', .F., 'sqlcust', 'NAME')\n"
        "nFilteredIndexAfter = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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
    const auto filtered_visible = state.globals.find("lfilteredvisible");
    const auto filtered_visible_rec = state.globals.find("nfilteredvisiblerec");
    const auto filtered_hidden = state.globals.find("lfilteredhidden");
    const auto filtered_hidden_rec = state.globals.find("nfilteredhiddenrec");
    const auto filtered_hidden_eof = state.globals.find("lfilteredhiddeneof");
    const auto filtered_near = state.globals.find("lfilterednear");
    const auto filtered_near_rec = state.globals.find("nfilterednearrec");
    const auto filtered_index_before = state.globals.find("nfilteredindexbefore");
    const auto filtered_index_hidden = state.globals.find("lfilteredindexhidden");
    const auto filtered_index_after = state.globals.find("nfilteredindexafter");
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
    expect(filtered_visible != state.globals.end(), "filter-visible SQL SEEK() result should be captured");
    expect(filtered_visible_rec != state.globals.end(), "filter-visible SQL SEEK() RECNO() should be captured");
    expect(filtered_hidden != state.globals.end(), "filtered-out SQL SEEK() result should be captured");
    expect(filtered_hidden_rec != state.globals.end(), "filtered-out SQL SEEK() RECNO() should be captured");
    expect(filtered_hidden_eof != state.globals.end(), "filtered-out SQL SEEK() EOF() should be captured");
    expect(filtered_near != state.globals.end(), "filtered SQL SET NEAR result should be captured");
    expect(filtered_near_rec != state.globals.end(), "filtered SQL SET NEAR RECNO() should be captured");
    expect(filtered_index_before != state.globals.end(), "filtered SQL INDEXSEEK() starting RECNO() should be captured");
    expect(filtered_index_hidden != state.globals.end(), "filtered SQL INDEXSEEK() result should be captured");
    expect(filtered_index_after != state.globals.end(), "filtered SQL INDEXSEEK() preserved RECNO() should be captured");
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
    if (filtered_visible != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_visible->second) == "true", "SEEK() should find a filter-visible SQL row");
    }
    if (filtered_visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_visible_rec->second) == "2", "SEEK() should position on the filter-visible SQL row");
    }
    if (filtered_hidden != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_hidden->second) == "false", "SEEK() should not find a filtered-out SQL row");
    }
    if (filtered_hidden_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_hidden_rec->second) == "4", "filtered SQL SEEK() with SET NEAR OFF should move to physical EOF");
    }
    if (filtered_hidden_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_hidden_eof->second) == "true", "filtered SQL SEEK() with SET NEAR OFF should set EOF()");
    }
    if (filtered_near != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_near->second) == "false", "SET NEAR should not turn a filtered SQL key into a hit");
    }
    if (filtered_near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_near_rec->second) == "2", "SET NEAR should select the next filter-visible SQL row");
    }
    if (filtered_index_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_index_before->second) == "3", "filtered SQL INDEXSEEK() should start on the selected row");
    }
    if (filtered_index_hidden != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_index_hidden->second) == "false", "INDEXSEEK() should not report a filtered-out SQL row");
    }
    if (filtered_index_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_index_after->second) == "3", "filtered SQL INDEXSEEK(.F.) should preserve RECNO()");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL SEEK() checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_seek_respects_set_deleted() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_set_deleted";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_set_deleted.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'BRAVO', AMOUNT WITH 40\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'DELTA', AMOUNT WITH 50\n"
        "GO 2\n"
        "DELETE\n"
        "GO 3\n"
        "DELETE\n"
        "SET ORDER TO NAME\n"
        "SET EXACT OFF\n"
        "SET DELETED OFF\n"
        "SEEK 'CHARLIE'\n"
        "lOffDeletedFound = FOUND()\n"
        "nOffDeletedRec = RECNO()\n"
        "SET DELETED ON\n"
        "SET NEAR OFF\n"
        "SEEK 'BRAVO'\n"
        "lDuplicateFound = FOUND()\n"
        "nDuplicateRec = RECNO()\n"
        "SEEK 'BRA'\n"
        "lPrefixDuplicateFound = FOUND()\n"
        "nPrefixDuplicateRec = RECNO()\n"
        "lDeletedOnlyFunction = SEEK('CHARLIE', 'sqlcust', 'NAME')\n"
        "nDeletedOnlyRec = RECNO()\n"
        "lDeletedOnlyEof = EOF()\n"
        "lPrefixDeletedFunction = SEEK('CHAR', 'sqlcust', 'NAME')\n"
        "nPrefixDeletedRec = RECNO()\n"
        "GO TOP\n"
        "nIndexBefore = RECNO()\n"
        "lIndexDeleted = INDEXSEEK('CHARLIE', .F., 'sqlcust', 'NAME')\n"
        "nIndexAfter = RECNO()\n"
        "lIndexMoveDeleted = INDEXSEEK('CHARLIE', .T., 'sqlcust', 'NAME')\n"
        "nIndexMoveAfter = RECNO()\n"
        "SET NEAR ON\n"
        "lIndexMoveNearDeleted = INDEXSEEK('CHARLIE', .T., 'sqlcust', 'NAME')\n"
        "nIndexMoveNearAfter = RECNO()\n"
        "SEEK 'CHARLIE'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'CHAR'\n"
        "lPrefixNearFound = FOUND()\n"
        "nPrefixNearRec = RECNO()\n"
        "SET FILTER TO NAME <> 'DELTA'\n"
        "SEEK 'CHARLIE'\n"
        "lFilteredNearFound = FOUND()\n"
        "nFilteredNearRec = RECNO()\n"
        "lFilteredNearEof = EOF()\n"
        "SET FILTER TO\n"
        "SET ORDER TO NAME DESCENDING\n"
        "SEEK 'CHARLIE'\n"
        "lDescendingNearFound = FOUND()\n"
        "nDescendingNearRec = RECNO()\n"
        "SET DELETED OFF\n"
        "SET NEAR OFF\n"
        "SEEK 'CHARLIE'\n"
        "lOffAgainFound = FOUND()\n"
        "nOffAgainRec = RECNO()\n"
        "lOffAgainDeleted = DELETED()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL cursor SEEK with SET DELETED visibility should complete");
    expect(state.sql_connections.empty(), "SQL cursor SET DELETED seek script should disconnect its SQL handle");

    const auto expect_value = [&](const std::string &name, const std::string &expected, const std::string &message) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), message + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected, message);
        }
    };

    expect_value("loffdeletedfound", "true", "SET DELETED OFF should expose a deleted SQL exact-key match");
    expect_value("noffdeletedrec", "3", "SET DELETED OFF should position on the deleted SQL row");
    expect_value("lduplicatefound", "true", "SET DELETED ON should skip a deleted SQL duplicate and find the live duplicate");
    expect_value("nduplicaterec", "4", "SQL SEEK should position on the first live duplicate");
    expect_value("lprefixduplicatefound", "true", "SQL prefix SEEK should skip a deleted duplicate and find the live duplicate");
    expect_value("nprefixduplicaterec", "4", "SQL prefix SEEK should position on the first live duplicate");
    expect_value("ldeletedonlyfunction", "false", "SQL SEEK() should reject a deleted-only key under SET DELETED ON");
    expect_value("ndeletedonlyrec", "6", "a deleted-only SQL miss with SET NEAR OFF should move to physical EOF");
    expect_value("ldeletedonlyeof", "true", "a deleted-only SQL miss with SET NEAR OFF should set EOF()");
    expect_value("lprefixdeletedfunction", "false", "SQL prefix SEEK() should reject a deleted-only key");
    expect_value("nprefixdeletedrec", "6", "a deleted-only SQL prefix miss should move to physical EOF");
    expect_value("nindexbefore", "1", "SQL INDEXSEEK() pointer-preservation setup should select the first live row");
    expect_value("lindexdeleted", "false", "SQL INDEXSEEK() should reject a deleted-only key");
    expect_value("nindexafter", "1", "SQL INDEXSEEK(.F.) should preserve the pointer after a deleted-only miss");
    expect_value("lindexmovedeleted", "false", "SQL INDEXSEEK(.T.) should reject a deleted-only key with SET NEAR OFF");
    expect_value("nindexmoveafter", "1", "SQL INDEXSEEK(.T.) should preserve the pointer when no match exists");
    expect_value("lindexmoveneardeleted", "false", "SQL INDEXSEEK(.T.) should reject a deleted-only key with SET NEAR ON");
    expect_value("nindexmovenearafter", "1", "SQL INDEXSEEK(.T.) should preserve the pointer on a near miss");
    expect_value("lnearfound", "false", "SQL SET NEAR should keep a deleted exact key as a miss");
    expect_value("nnearrec", "5", "SQL SET NEAR should position on the next live indexed row");
    expect_value("lprefixnearfound", "false", "SQL prefix SET NEAR should keep a deleted-only prefix as a miss");
    expect_value("nprefixnearrec", "5", "SQL prefix SET NEAR should position on the next live indexed row");
    expect_value("lfilterednearfound", "false", "SQL SET FILTER should compose with SET DELETED during near lookup");
    expect_value("nfilterednearrec", "6", "a filter-hidden SQL near candidate should leave the cursor at physical EOF");
    expect_value("lfilteredneareof", "true", "a filter-hidden SQL near candidate should set EOF()");
    expect_value("ldescendingnearfound", "false", "descending SQL SET NEAR should keep a deleted exact key as a miss");
    expect_value("ndescendingnearrec", "4", "descending SQL SET NEAR should skip deleted candidates and choose the live duplicate");
    expect_value("loffagainfound", "true", "SET DELETED OFF should restore SQL deleted-record SEEK access");
    expect_value("noffagainrec", "3", "SET DELETED OFF should restore the deleted SQL exact-key position");
    expect_value("loffagaindeleted", "true", "the restored SQL deleted exact hit should expose DELETED() true");
    expect_value("ldisc", "1", "SQLDISCONNECT should succeed after SET DELETED seek checks");

    fs::remove_all(temp_root, ignored);
}

#include "test_prg_engine_sql_cursors_seek_and_order_temporary_order_normalization.inl"

#include "test_prg_engine_sql_cursors_seek_and_order_derived_string_temporary_order.inl"

#include "test_prg_engine_sql_cursors_seek_and_order_right_str_temporary_order.inl"

#include "test_prg_engine_sql_cursors_seek_and_order_default_padding.inl"
#include "test_prg_engine_sql_cursors_seek_and_order_padr_decimal.inl"
#include "test_prg_engine_sql_cursors_seek_and_order_plain_string_collate.inl"

void test_sql_result_cursor_temporary_order_for_expression_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_for_expression_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_for_expression_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "lSeekFilteredOut = SEEK('ALPHA', 'sqlcust', \"UPPER(NAME) FOR NAME = 'BRAVO'\")\n"
        "nSeekFilteredRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP\n"
        "SET ORDER TO UPPER(NAME) FOR NAME = 'BRAVO'\n"
        "SEEK 'ALPHA'\n"
        "lNearFound = FOUND()\n"
        "nNearRec = RECNO()\n"
        "SEEK 'BRAVO'\n"
        "lVisibleFound = FOUND()\n"
        "nVisibleRec = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL temporary-order FOR-expression parity script should complete");
    expect(state.sql_connections.empty(), "SQL temporary-order FOR-expression parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto seek_filtered_out = state.globals.find("lseekfilteredout");
    const auto seek_filtered_rec = state.globals.find("nseekfilteredrec");
    const auto near_found = state.globals.find("lnearfound");
    const auto near_rec = state.globals.find("nnearrec");
    const auto visible_found = state.globals.find("lvisiblefound");
    const auto visible_rec = state.globals.find("nvisiblerec");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL temporary-order FOR-expression parity");
    expect(seek_filtered_out != state.globals.end(), "SEEK() over a FOR-filtered SQL temporary order should be captured");
    expect(seek_filtered_rec != state.globals.end(), "RECNO() after a filtered-out SQL SEEK() should be captured");
    expect(near_found != state.globals.end(), "FOUND() after SQL SET NEAR plus FOR-filtered SEEK() should be captured");
    expect(near_rec != state.globals.end(), "RECNO() after SQL SET NEAR plus FOR-filtered SEEK() should be captured");
    expect(visible_found != state.globals.end(), "FOUND() after a visible SQL FOR-filtered SEEK() should be captured");
    expect(visible_rec != state.globals.end(), "RECNO() after a visible SQL FOR-filtered SEEK() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL temporary-order FOR-expression parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL temporary-order FOR-expression checks");
    }
    if (seek_filtered_out != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_filtered_out->second) == "false", "SQL temporary-order FOR expressions should filter ALPHA out of the candidate set");
    }
    if (seek_filtered_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_filtered_rec->second) == "4", "filtered-out SQL temporary-order SEEK() without SET NEAR should still land at EOF");
    }
    if (near_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_found->second) == "false", "SET NEAR should still report a miss for a filtered-out SQL temporary-order key");
    }
    if (near_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near_rec->second) == "2", "SET NEAR should move a SQL temporary-order seek to the surviving FOR-filtered row");
    }
    if (visible_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_found->second) == "true", "SQL temporary-order FOR expressions should still allow visible keys");
    }
    if (visible_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(visible_rec->second) == "2", "SQL temporary-order FOR expressions should still position on the surviving visible row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after SQL temporary-order FOR-expression checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_numeric_temporary_order_domain_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_seek_numeric_domain_parity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_seek_numeric_domain_parity.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO AMOUNT\n"
        "SEEK '1'\n"
        "lPrefixFound = FOUND()\n"
        "lPrefixEof = EOF()\n"
        "nPrefixRec = RECNO()\n"
        "SET NEAR ON\n"
        "GO TOP IN sqlcust\n"
        "SEEK '9'\n"
        "lFound = FOUND()\n"
        "nRec = RECNO()\n"
        "nAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL numeric temporary-order key-domain parity script should complete");
    expect(state.sql_connections.empty(), "SQL numeric temporary-order key-domain parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto prefix_found = state.globals.find("lprefixfound");
    const auto prefix_eof = state.globals.find("lprefixeof");
    const auto prefix_rec = state.globals.find("nprefixrec");
    const auto found = state.globals.find("lfound");
    const auto rec = state.globals.find("nrec");
    const auto amount = state.globals.find("namount");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL numeric temporary-order parity");
    expect(prefix_found != state.globals.end(), "FOUND() after numeric SQL exact-seek miss should be captured");
    expect(prefix_eof != state.globals.end(), "EOF() after numeric SQL exact-seek miss should be captured");
    expect(prefix_rec != state.globals.end(), "RECNO() after numeric SQL exact-seek miss should be captured");
    expect(found != state.globals.end(), "FOUND() after numeric SQL temporary-order SEEK() should be captured");
    expect(rec != state.globals.end(), "RECNO() after numeric SQL temporary-order SEEK() should be captured");
    expect(amount != state.globals.end(), "field value after numeric SQL temporary-order SEEK() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL numeric temporary-order parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before numeric SQL temporary-order checks");
    }
    if (prefix_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_found->second) == "false", "numeric SQL temporary-order SEEK() should not treat a numeric prefix as an exact hit when SET EXACT is OFF");
    }
    if (prefix_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_eof->second) == "true", "numeric SQL temporary-order exact miss without SET NEAR should still land at EOF");
    }
    if (prefix_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(prefix_rec->second) == "4", "numeric SQL temporary-order exact miss without SET NEAR should place RECNO() at record_count + 1");
    }
    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "false", "numeric SQL temporary-order SEEK() should still report a miss for a non-existent key");
    }
    if (rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec->second) == "1", "numeric SQL temporary-order SET NEAR should move to the first numeric row instead of lexicographic EOF");
    }
    if (amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(amount->second) == "10", "numeric SQL temporary-order key-domain hints should treat AMOUNT numerically");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should still succeed after numeric SQL temporary-order checks");
    }

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_sql_result_cursor_command_derived_temporary_order_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_derived_orders";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_derived_orders.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO UPPER(LEFT(NAME, 3))\n"
        "SEEK 'CHA'\n"
        "lFoundLeft = FOUND()\n"
        "nRecLeft = RECNO()\n"
        "GO TOP\n"
        "SET ORDER TO UPPER(PADR(NAME, 8, '0'))\n"
        "SEEK 'BRAVO000'\n"
        "lFoundPad = FOUND()\n"
        "nRecPad = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path derived temporary-order parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path derived temporary-order parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto found_left = state.globals.find("lfoundleft");
    const auto rec_left = state.globals.find("nrecleft");
    const auto found_pad = state.globals.find("lfoundpad");
    const auto rec_pad = state.globals.find("nrecpad");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL command-path derived temporary-order parity");
    expect(found_left != state.globals.end(), "LEFT()-derived SQL command SEEK FOUND() should be captured");
    expect(rec_left != state.globals.end(), "LEFT()-derived SQL command SEEK RECNO() should be captured");
    expect(found_pad != state.globals.end(), "PADR()-derived SQL command SEEK FOUND() should be captured");
    expect(rec_pad != state.globals.end(), "PADR()-derived SQL command SEEK RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL command-path derived temporary-order parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL command-path derived-order checks");
    }
    if (found_left != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_left->second) == "true", "command SEEK should match LEFT()-derived temporary SQL order keys");
    }
    if (rec_left != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_left->second) == "3", "LEFT()-derived SQL command SEEK should land on the expected exact match");
    }
    if (found_pad != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_pad->second) == "true", "command SEEK should match PADR()-derived temporary SQL order keys");
    }
    if (rec_pad != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_pad->second) == "2", "PADR()-derived SQL command SEEK should land on the expected exact match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL command-path derived-order checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_command_padl_truncation_temporary_order_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_command_padl_truncation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_command_padl_truncation.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET ORDER TO UPPER(PADL(NAME, 3))\n"
        "SEEK 'LIE'\n"
        "lFoundCmd = FOUND()\n"
        "nRecCmd = RECNO()\n"
        "GO TOP\n"
        "lFoundFn = SEEK('AVO', 'sqlcust', 'UPPER(PADL(NAME, 3))')\n"
        "nRecFn = RECNO()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL command-path truncating PADL() temporary-order parity script should complete");
    expect(state.sql_connections.empty(), "SQL command-path truncating PADL() temporary-order parity script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto found_cmd = state.globals.find("lfoundcmd");
    const auto rec_cmd = state.globals.find("nreccmd");
    const auto found_fn = state.globals.find("lfoundfn");
    const auto rec_fn = state.globals.find("nrecfn");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for SQL truncating PADL() temporary-order parity");
    expect(found_cmd != state.globals.end(), "command SEEK on a truncating PADL() SQL temporary order should be captured");
    expect(rec_cmd != state.globals.end(), "command truncating PADL() SQL RECNO() should be captured");
    expect(found_fn != state.globals.end(), "SEEK() on a truncating PADL() SQL temporary order should be captured");
    expect(rec_fn != state.globals.end(), "SEEK() truncating PADL() SQL RECNO() should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL truncating PADL() temporary-order parity");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before SQL truncating PADL() temporary-order checks");
    }
    if (found_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_cmd->second) == "true", "command SEEK should match truncating PADL()-derived SQL right-edge keys");
    }
    if (rec_cmd != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_cmd->second) == "3", "command SEEK should land on the truncating PADL()-derived CHARLIE SQL match");
    }
    if (found_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_fn->second) == "true", "SEEK() should match truncating PADL()-derived SQL right-edge keys");
    }
    if (rec_fn != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_fn->second) == "2", "SEEK() should land on the truncating PADL()-derived BRAVO SQL match");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL truncating PADL() temporary-order checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_set_exact_seek_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_set_exact";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_set_exact.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust1')\n"
        "SELECT sqlcust1\n"
        "SET ORDER TO NAME\n"
        "lSeekOff = SEEK('BR')\n"
        "nRecOff = RECNO()\n"
        "SET EXACT ON\n"
        "GO TOP\n"
        "lSeekOn = SEEK('BR')\n"
        "lEofOn = EOF()\n"
        "SET DATASESSION TO 2\n"
        "nConn2 = SQLCONNECT('dsn=Northwind')\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from customers', 'sqlcust2')\n"
        "SELECT sqlcust2\n"
        "SET ORDER TO NAME\n"
        "lSeekSession2 = SEEK('BR')\n"
        "lDisc2 = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "GO TOP IN sqlcust1\n"
        "lSeekBack = SEEK('BR')\n"
        "lDisc1 = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL SET EXACT seek parity script should complete");
    expect(state.sql_connections.empty(), "SQL SET EXACT seek parity script should disconnect its SQL handles");

    const auto exec1 = state.globals.find("nexec1");
    const auto seek_off = state.globals.find("lseekoff");
    const auto rec_off = state.globals.find("nrecoff");
    const auto seek_on = state.globals.find("lseekon");
    const auto eof_on = state.globals.find("leofon");
    const auto exec2 = state.globals.find("nexec2");
    const auto seek_session2 = state.globals.find("lseeksession2");
    const auto seek_back = state.globals.find("lseekback");
    const auto disc2 = state.globals.find("ldisc2");
    const auto disc1 = state.globals.find("ldisc1");

    expect(exec1 != state.globals.end(), "First SQLEXEC result should be captured for SQL SET EXACT parity");
    expect(seek_off != state.globals.end(), "SET EXACT OFF SQL seek result should be captured");
    expect(rec_off != state.globals.end(), "SET EXACT OFF SQL RECNO() should be captured");
    expect(seek_on != state.globals.end(), "SET EXACT ON SQL seek result should be captured");
    expect(eof_on != state.globals.end(), "SET EXACT ON SQL EOF() should be captured");
    expect(exec2 != state.globals.end(), "Second-session SQLEXEC result should be captured for SQL SET EXACT parity");
    expect(seek_session2 != state.globals.end(), "second-session SQL seek result should be captured");
    expect(seek_back != state.globals.end(), "restored session-1 SQL seek result should be captured");
    expect(disc2 != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(disc1 != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (exec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec1->second) == "1", "First SQLEXEC should succeed before SQL SET EXACT seek checks");
    }
    if (seek_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_off->second) == "true", "SET EXACT OFF should allow prefix seeks on SQL cursors");
    }
    if (rec_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_off->second) == "2", "SET EXACT OFF SQL seek should land on the matching prefix row");
    }
    if (seek_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_on->second) == "false", "SET EXACT ON should reject prefix seeks on SQL cursors");
    }
    if (eof_on != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_on->second) == "true", "SET EXACT ON failed SQL seek should leave the cursor at EOF");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "Second-session SQLEXEC should succeed before session-isolation checks");
    }
    if (seek_session2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_session2->second) == "true", "SET EXACT should stay session-scoped so a fresh SQL session keeps prefix-seek behavior");
    }
    if (seek_back != state.globals.end()) {
        expect(copperfin::runtime::format_value(seek_back->second) == "false", "restoring the original SQL session should restore its SET EXACT ON seek behavior");
    }
    if (disc2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc2->second) == "1", "session-2 SQLDISCONNECT should succeed after SQL SET EXACT checks");
    }
    if (disc1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc1->second) == "1", "session-1 SQLDISCONNECT should succeed after SQL SET EXACT checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_set_near_is_scoped_per_data_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_set_near_sessions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_set_near_sessions.prg";
    write_text(
        main_path,
        "nConn1 = SQLCONNECT('dsn=Northwind')\n"
        "nExec1 = SQLEXEC(nConn1, 'select * from customers', 'sqlcust1')\n"
        "SELECT sqlcust1\n"
        "SET ORDER TO NAME\n"
        "SET NEAR ON\n"
        "SEEK 'BETA'\n"
        "lNear1Found = FOUND()\n"
        "lNear1Eof = EOF()\n"
        "nNear1Rec = RECNO()\n"
        "SET DATASESSION TO 2\n"
        "nConn2 = SQLCONNECT('dsn=Northwind')\n"
        "nExec2 = SQLEXEC(nConn2, 'select * from customers', 'sqlcust2')\n"
        "SELECT sqlcust2\n"
        "SET ORDER TO NAME\n"
        "SEEK 'BETA'\n"
        "lNear2Found = FOUND()\n"
        "lNear2Eof = EOF()\n"
        "nNear2Rec = RECNO()\n"
        "lDisc2 = SQLDISCONNECT(nConn2)\n"
        "SET DATASESSION TO 1\n"
        "GO TOP IN sqlcust1\n"
        "SEEK 'BETA'\n"
        "lNear1BackFound = FOUND()\n"
        "lNear1BackEof = EOF()\n"
        "nNear1BackRec = RECNO()\n"
        "lDisc1 = SQLDISCONNECT(nConn1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL SET NEAR data-session script should complete");
    expect(state.sql_connections.empty(), "SQL SET NEAR data-session script should disconnect its SQL handles");

    const auto exec1 = state.globals.find("nexec1");
    const auto near1_found = state.globals.find("lnear1found");
    const auto near1_eof = state.globals.find("lnear1eof");
    const auto near1_rec = state.globals.find("nnear1rec");
    const auto exec2 = state.globals.find("nexec2");
    const auto near2_found = state.globals.find("lnear2found");
    const auto near2_eof = state.globals.find("lnear2eof");
    const auto near2_rec = state.globals.find("nnear2rec");
    const auto near1_back_found = state.globals.find("lnear1backfound");
    const auto near1_back_eof = state.globals.find("lnear1backeof");
    const auto near1_back_rec = state.globals.find("nnear1backrec");
    const auto disc2 = state.globals.find("ldisc2");
    const auto disc1 = state.globals.find("ldisc1");

    expect(exec1 != state.globals.end(), "First SQLEXEC result should be captured for SQL SET NEAR session scoping");
    expect(near1_found != state.globals.end(), "session-1 SQL SET NEAR FOUND() should be captured");
    expect(near1_eof != state.globals.end(), "session-1 SQL SET NEAR EOF() should be captured");
    expect(near1_rec != state.globals.end(), "session-1 SQL SET NEAR RECNO() should be captured");
    expect(exec2 != state.globals.end(), "Second-session SQLEXEC result should be captured for SQL SET NEAR session scoping");
    expect(near2_found != state.globals.end(), "session-2 SQL SEEK FOUND() should be captured");
    expect(near2_eof != state.globals.end(), "session-2 SQL SEEK EOF() should be captured");
    expect(near2_rec != state.globals.end(), "session-2 SQL SEEK RECNO() should be captured");
    expect(near1_back_found != state.globals.end(), "restored session-1 SQL SEEK FOUND() should be captured");
    expect(near1_back_eof != state.globals.end(), "restored session-1 SQL SEEK EOF() should be captured");
    expect(near1_back_rec != state.globals.end(), "restored session-1 SQL SEEK RECNO() should be captured");
    expect(disc2 != state.globals.end(), "session-2 SQLDISCONNECT result should be captured");
    expect(disc1 != state.globals.end(), "session-1 SQLDISCONNECT result should be captured");

    if (exec1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec1->second) == "1", "First SQLEXEC should succeed before SQL SET NEAR session checks");
    }
    if (near1_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_found->second) == "false", "SET NEAR ON should still leave SQL FOUND() false on a missed seek");
    }
    if (near1_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_eof->second) == "false", "SET NEAR ON in SQL session 1 should keep the cursor off EOF");
    }
    if (near1_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_rec->second) == "2", "SET NEAR ON in SQL session 1 should move to the nearest ordered row");
    }
    if (exec2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec2->second) == "1", "Second-session SQLEXEC should succeed before SQL SET NEAR isolation checks");
    }
    if (near2_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_found->second) == "false", "a fresh second SQL data session should still report a missed seek");
    }
    if (near2_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_eof->second) == "true", "SET NEAR should not bleed into a fresh second SQL data session");
    }
    if (near2_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near2_rec->second) == "4", "a fresh second SQL data session should keep the default SET NEAR OFF seek position");
    }
    if (near1_back_found != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_found->second) == "false", "restoring SQL session 1 should preserve missed-seek FOUND() behavior");
    }
    if (near1_back_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_eof->second) == "false", "restoring SQL session 1 should restore its SET NEAR ON behavior");
    }
    if (near1_back_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(near1_back_rec->second) == "2", "restoring SQL session 1 should restore its nearest-record seek position");
    }
    if (disc2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc2->second) == "1", "session-2 SQLDISCONNECT should succeed after SQL SET NEAR checks");
    }
    if (disc1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc1->second) == "1", "session-1 SQLDISCONNECT should succeed after SQL SET NEAR checks");
    }

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_sql_result_cursor_go_top_bottom_with_no_visible_records_sets_bof_and_eof() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_filter_hidden_go_flags";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_filter_hidden_go_flags.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "SET FILTER TO .F.\n"
        "GO TOP\n"
        "lBofAfterTop = BOF()\n"
        "lEofAfterTop = EOF()\n"
        "GO BOTTOM\n"
        "lBofAfterBottom = BOF()\n"
        "lEofAfterBottom = EOF()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL GO TOP/BOTTOM with no visible records should complete");
    expect(state.sql_connections.empty(), "SQL GO TOP/BOTTOM hidden-record script should disconnect its SQL handle");

    const auto exec = state.globals.find("nexec");
    const auto bof_after_top = state.globals.find("lbofaftertop");
    const auto eof_after_top = state.globals.find("leofaftertop");
    const auto bof_after_bottom = state.globals.find("lbofafterbottom");
    const auto eof_after_bottom = state.globals.find("leofafterbottom");
    const auto disc = state.globals.find("ldisc");

    expect(exec != state.globals.end(), "SQLEXEC result should be captured for hidden-record SQL GO TOP/BOTTOM checks");
    expect(bof_after_top != state.globals.end(), "hidden-record SQL GO TOP should expose BOF()");
    expect(eof_after_top != state.globals.end(), "hidden-record SQL GO TOP should expose EOF()");
    expect(bof_after_bottom != state.globals.end(), "hidden-record SQL GO BOTTOM should expose BOF()");
    expect(eof_after_bottom != state.globals.end(), "hidden-record SQL GO BOTTOM should expose EOF()");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for hidden-record SQL GO TOP/BOTTOM checks");

    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should succeed before hidden-record SQL GO TOP/BOTTOM checks");
    }
    if (bof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_top->second) == "false",
               "SQL GO TOP with no visible records in a nonempty cursor should leave BOF() false");
    }
    if (eof_after_top != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_top->second) == "true",
               "SQL GO TOP with no visible records should leave EOF() true");
    }
    if (bof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(bof_after_bottom->second) == "true",
               "SQL GO BOTTOM with no visible records should leave BOF() true");
    }
    if (eof_after_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(eof_after_bottom->second) == "true",
               "SQL GO BOTTOM with no visible records should leave EOF() true");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
               "SQLDISCONNECT should succeed after hidden-record SQL GO TOP/BOTTOM checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_macro_fields_and_filter_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_macro_fields_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_macro_fields_filter.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "cFieldsSpec = 'LIKE N*'\n"
        "cFieldsSpecHolder = 'cFieldsSpec'\n"
        "cFieldsSpecDeepHolder = 'cFieldsSpecHolder'\n"
        "SET FIELDS TO &cFieldsSpecDeepHolder\n"
        "DISPLAY IN 'sqlcust'\n"
        "SET FIELDS TO OFF\n"
        "cFilterExpr = 'AMOUNT >= 20'\n"
        "cFilterExprHolder = 'cFilterExpr'\n"
        "cFilterExprDeepHolder = 'cFilterExprHolder'\n"
        "SET FILTER TO &cFilterExprDeepHolder IN sqlcust\n"
        "GO TOP IN sqlcust\n"
        "nFilteredRec = RECNO('sqlcust')\n"
        "cFilteredName = sqlcust.NAME\n"
        "nFilteredAmount = sqlcust.AMOUNT\n"
        "SET FILTER TO IN sqlcust\n"
        "SET FIELDS TO OFF\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL macro fields/filter script should complete");
    expect(state.sql_connections.empty(), "SQL macro fields/filter script should disconnect its SQL handle");

    const auto exec_result = state.globals.find("nexec");
    const auto filtered_rec = state.globals.find("nfilteredrec");
    const auto filtered_name = state.globals.find("cfilteredname");
    const auto filtered_amount = state.globals.find("nfilteredamount");
    const auto disc = state.globals.find("ldisc");
    const copperfin::runtime::RuntimeEvent *display_event = nullptr;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_event = &event;
            break;
        }
    }

    expect(exec_result != state.globals.end(), "SQLEXEC result should be captured for SQL macro fields/filter checks");
    expect(filtered_rec != state.globals.end(), "RECNO(alias) should be captured after SQL macro filter GO TOP");
    expect(filtered_name != state.globals.end(), "filtered NAME should be captured after SQL macro filter GO TOP");
    expect(filtered_amount != state.globals.end(), "filtered AMOUNT should be captured after SQL macro filter GO TOP");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL macro fields/filter checks");
    expect(display_event != nullptr, "DISPLAY IN 'sqlcust' should emit runtime.display for SQL macro fields/filter checks");

    if (exec_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_result->second) == "1", "SQLEXEC should succeed before SQL macro fields/filter checks");
    }
    if (display_event != nullptr) {
        expect(display_event->detail.find("fields=NAME") != std::string::npos, "SET FIELDS TO second-hop &cSpec should drive SQL cursor visible-field metadata");
    }
    if (filtered_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_rec->second) == "2", "SET FILTER TO second-hop &cExpr IN sqlcust should position GO TOP on the first visible SQL row");
    }
    if (filtered_name != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(filtered_name->second)) == "BRAVO", "macro-expanded SQL filter should preserve field lookup on the first visible row");
    }
    if (filtered_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(filtered_amount->second) == "20", "macro-expanded SQL filter should preserve numeric field lookup on the first visible row");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL macro fields/filter checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_result_cursor_macro_for_expression_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_macro_for_expr";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_macro_for_expr.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "cLocateExpr = 'AMOUNT = 30'\n"
        "cLocateExprHolder = 'cLocateExpr'\n"
        "cLocateExprDeepHolder = 'cLocateExprHolder'\n"
        "cDeleteExpr = 'NAME = ''BRAVO''' \n"
        "cDeleteExprHolder = 'cDeleteExpr'\n"
        "cDeleteExprDeepHolder = 'cDeleteExprHolder'\n"
        "LOCATE FOR &cLocateExprDeepHolder IN sqlcust\n"
        "nLocateRec = RECNO('sqlcust')\n"
        "cLocateName = sqlcust.NAME\n"
        "DELETE FOR &cDeleteExprDeepHolder IN sqlcust\n"
        "LOCATE FOR DELETED() IN sqlcust\n"
        "cDeletedName = sqlcust.NAME\n"
        "RECALL FOR &cDeleteExprDeepHolder IN sqlcust\n"
        "LOCATE FOR DELETED() IN sqlcust\n"
        "lDeletedAfterRecall = FOUND()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL macro FOR-expression script should complete");
    expect(state.sql_connections.empty(), "SQL macro FOR-expression script should disconnect its SQL handle");

    const auto exec_result = state.globals.find("nexec");
    const auto locate_rec = state.globals.find("nlocaterec");
    const auto locate_name = state.globals.find("clocatename");
    const auto deleted_name = state.globals.find("cdeletedname");
    const auto deleted_after_recall = state.globals.find("ldeletedafterrecall");
    const auto disc = state.globals.find("ldisc");

    expect(exec_result != state.globals.end(), "SQLEXEC result should be captured for SQL macro FOR-expression checks");
    expect(locate_rec != state.globals.end(), "RECNO(alias) should be captured after LOCATE FOR &expr IN sqlcust");
    expect(locate_name != state.globals.end(), "NAME should be captured after LOCATE FOR &expr IN sqlcust");
    expect(deleted_name != state.globals.end(), "deleted-row NAME should be captured after DELETE FOR &expr IN sqlcust");
    expect(deleted_after_recall != state.globals.end(), "FOUND() should be captured after RECALL FOR &expr IN sqlcust");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured for SQL macro FOR-expression checks");

    if (exec_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec_result->second) == "1", "SQLEXEC should succeed before SQL macro FOR-expression checks");
    }
    if (locate_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(locate_rec->second) == "3", "LOCATE FOR second-hop &cExpr IN sqlcust should position the SQL cursor on the matching row");
    }
    if (locate_name != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(locate_name->second)) == "CHARLIE", "LOCATE FOR second-hop &cExpr IN sqlcust should expose the matching SQL row");
    }
    if (deleted_name != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(deleted_name->second)) == "BRAVO", "DELETE FOR second-hop &cExpr IN sqlcust should tombstone the matching SQL row");
    }
    if (deleted_after_recall != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_after_recall->second) == "false", "RECALL FOR second-hop &cExpr IN sqlcust should clear the tombstone before the final DELETED() locate");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL macro FOR-expression checks");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_set_filter_defers_sql_cursor_evaluation_until_navigation();
    test_sql_result_cursor_seek_parity();
    test_sql_result_cursor_seek_respects_set_deleted();
    test_sql_result_cursor_temporary_order_normalization_parity();
    test_sql_result_cursor_derived_string_temporary_order_parity();
    test_sql_result_cursor_right_and_str_temporary_order_parity();
    test_sql_result_cursor_default_padding_and_str_variant_parity();
    test_sql_result_cursor_padr_default_and_str_decimal_parity();
    test_sql_result_cursor_plain_string_collate_parity();
    test_sql_result_cursor_temporary_order_for_expression_parity();
    test_sql_result_cursor_numeric_temporary_order_domain_parity();
    test_sql_result_cursor_temporary_order_direction_suffix_parity();
    test_sql_result_cursor_command_seek_parity();
    test_sql_result_cursor_command_derived_temporary_order_parity();
    test_sql_result_cursor_command_padl_truncation_temporary_order_parity();
    test_sql_result_cursor_set_exact_seek_parity();
    test_sql_result_cursor_set_near_is_scoped_per_data_session();
    test_sql_result_cursor_command_seek_in_target_parity();
    test_sql_result_cursor_scan_in_target_parity();
    test_sql_result_cursor_order_direction_in_target_parity();
    test_sql_result_cursor_navigation_in_target_parity();
    test_sql_result_cursor_filter_in_target_parity();
    test_sql_result_cursor_go_top_bottom_with_no_visible_records_sets_bof_and_eof();
    test_sql_result_cursor_macro_fields_and_filter_parity();
    test_sql_result_cursor_macro_for_expression_parity();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
