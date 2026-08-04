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

void test_sql_result_cursor_backward_navigation_in_target_parity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_backward_navigation_in_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_backward_navigation_in_target.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO BOTTOM\n"
        "nOtherRecBefore = RECNO('sqlother')\n"
        "GO BOTTOM IN sqlcust\n"
        "nCustRecBottom = RECNO('sqlcust')\n"
        "SKIP -1 IN sqlcust\n"
        "nCustRecBackOne = RECNO('sqlcust')\n"
        "GO TOP IN sqlcust\n"
        "SKIP -1 IN sqlcust\n"
        "lCustBof = BOF('sqlcust')\n"
        "nCustRecAtBof = RECNO('sqlcust')\n"
        "GO BOTTOM IN sqlcust\n"
        "SKIP 99 IN sqlcust\n"
        "lCustEof = EOF('sqlcust')\n"
        "nCustRecAtEof = RECNO('sqlcust')\n"
        "cAliasAfter = ALIAS()\n"
        "nOtherRecAfter = RECNO('sqlother')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL backward navigation IN-target parity script should complete");
    expect(state.sql_connections.empty(), "SQL backward navigation IN-target parity script should disconnect its SQL handle");

    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_bottom = state.globals.find("ncustrecbottom");
    const auto cust_rec_back_one = state.globals.find("ncustrecbackone");
    const auto cust_bof = state.globals.find("lcustbof");
    const auto cust_rec_at_bof = state.globals.find("ncustrecatbof");
    const auto cust_eof = state.globals.find("lcusteof");
    const auto cust_rec_at_eof = state.globals.find("ncustrecateof");
    const auto alias_after = state.globals.find("caliasafter");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto disc = state.globals.find("ldisc");

    expect(other_rec_before != state.globals.end(), "selected SQL cursor RECNO() before targeted backward navigation should be captured");
    expect(cust_rec_bottom != state.globals.end(), "target SQL cursor RECNO() after GO BOTTOM IN should be captured");
    expect(cust_rec_back_one != state.globals.end(), "target SQL cursor RECNO() after SKIP -1 IN should be captured");
    expect(cust_bof != state.globals.end(), "target SQL cursor BOF() after targeted backward navigation should be captured");
    expect(cust_rec_at_bof != state.globals.end(), "target SQL cursor RECNO() at BOF should be captured");
    expect(cust_eof != state.globals.end(), "target SQL cursor EOF() after targeted forward overflow should be captured");
    expect(cust_rec_at_eof != state.globals.end(), "target SQL cursor RECNO() at EOF should be captured");
    expect(alias_after != state.globals.end(), "selected alias after targeted backward navigation should be captured");
    expect(other_rec_after != state.globals.end(), "selected SQL cursor RECNO() after targeted backward navigation should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after targeted backward navigation checks");

    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "3", "selected SQL cursor should start at bottom before targeted backward navigation");
    }
    if (cust_rec_bottom != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_bottom->second) == "3", "GO BOTTOM IN should move the targeted SQL cursor to its last record");
    }
    if (cust_rec_back_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_back_one->second) == "2", "SKIP -1 IN should move the targeted SQL cursor backward by one visible row");
    }
    if (cust_bof != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_bof->second) == "true", "SKIP -1 IN from the first SQL row should move the targeted SQL cursor to BOF");
    }
    if (cust_rec_at_bof != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_at_bof->second) == "1", "RECNO() at targeted SQL BOF should remain at the first record number");
    }
    if (cust_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_eof->second) == "true", "SKIP past the end IN should move the targeted SQL cursor to EOF");
    }
    if (cust_rec_at_eof != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_at_eof->second) == "4", "RECNO() at targeted SQL EOF should be record_count + 1");
    }
    if (alias_after != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after->second)) == "SQLOTHER", "targeted SQL backward navigation should preserve the selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "3", "targeted SQL backward navigation should preserve the selected SQL cursor pointer");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after targeted backward navigation checks");
    }

    expect(
        has_runtime_event(state.events, "runtime.go", "BOTTOM") &&
        has_runtime_event(state.events, "runtime.skip", "-1") &&
        has_runtime_event(state.events, "runtime.skip", "99"),
        "targeted SQL backward navigation flow should emit runtime.go and runtime.skip events");

    fs::remove_all(temp_root, ignored);
}

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_copy_to_exports_selected_sql_result_cursor_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_copy_to_export";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path export_path = temp_root / "sql_export.dbf";
    const fs::path main_path = temp_root / "sql_copy_to_export.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "GO 2\n"
        "nRecBefore = RECNO()\n"
        "COPY TO '" + export_path.string() + "'\n"
        "nRecAfter = RECNO()\n"
        "cAliasAfterCopy = ALIAS()\n"
        "USE '" + export_path.string() + "' ALIAS local IN 0\n"
        "nRows = RECCOUNT('local')\n"
        "GO 2 IN local\n"
        "cName2 = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO selected SQL result-cursor export script should complete: " + state.message);

    const auto rec_before = state.globals.find("nrecbefore");
    const auto rec_after = state.globals.find("nrecafter");
    const auto alias_after_copy = state.globals.find("caliasaftercopy");
    const auto rows = state.globals.find("nrows");
    const auto name2 = state.globals.find("cname2");
    const auto disc = state.globals.find("ldisc");

    expect(rec_before != state.globals.end(), "RECNO() before SQL COPY TO should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after SQL COPY TO should be captured");
    expect(alias_after_copy != state.globals.end(), "selected alias after SQL COPY TO should be captured");
    expect(rows != state.globals.end(), "exported local row count should be captured");
    expect(name2 != state.globals.end(), "exported local second-row NAME should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after SQL COPY TO export checks");

    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "2",
            "selected SQL cursor should start on row 2 before COPY TO");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2",
            "COPY TO should preserve the selected SQL cursor pointer");
    }
    if (alias_after_copy != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_copy->second)) == "SQLCUST",
            "COPY TO should preserve the selected SQL alias before opening other cursors");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "3",
            "COPY TO should materialize all selected SQL result-cursor rows into DBF output");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "BRAVO",
            "COPY TO should preserve SQL result-cursor row values in DBF output");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after SQL COPY TO export checks");
    }

    expect(fs::exists(export_path), "COPY TO should create a DBF export file for selected SQL result cursors");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(export_path.string(), 10U);
    expect(parse_result.ok, "SQL result-cursor COPY TO output should be readable as DBF");
    if (parse_result.ok) {
        expect(parse_result.table.records.size() == 3U,
            "SQL result-cursor COPY TO output should persist all rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_json_exports_selected_sql_result_cursor_and_preserves_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_copy_to_json_export";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path export_path = temp_root / "sql_export.json";
    const fs::path main_path = temp_root / "sql_copy_to_json_export.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "GO 2\n"
        "nRecBefore = RECNO()\n"
        "COPY TO '" + export_path.string() + "' TYPE JSON FIELDS NAME, AMOUNT\n"
        "nRecAfter = RECNO()\n"
        "cAliasAfterCopy = ALIAS()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    const auto state = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()))
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE JSON selected SQL result-cursor export script should complete: " + state.message);

    const auto rec_before = state.globals.find("nrecbefore");
    const auto rec_after = state.globals.find("nrecafter");
    const auto alias_after_copy = state.globals.find("caliasaftercopy");
    const auto disc = state.globals.find("ldisc");
    expect(rec_before != state.globals.end(), "RECNO() before SQL COPY TO TYPE JSON should be captured");
    expect(rec_after != state.globals.end(), "RECNO() after SQL COPY TO TYPE JSON should be captured");
    expect(alias_after_copy != state.globals.end(), "selected alias after SQL COPY TO TYPE JSON should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after SQL COPY TO TYPE JSON checks");

    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "2",
            "selected SQL cursor should start on row 2 before COPY TO TYPE JSON");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "2",
            "COPY TO TYPE JSON should preserve the selected SQL cursor pointer");
    }
    if (alias_after_copy != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_copy->second)) == "SQLCUST",
            "COPY TO TYPE JSON should preserve the selected SQL alias");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after SQL COPY TO TYPE JSON checks");
    }

    expect(fs::exists(export_path), "COPY TO TYPE JSON should create a JSON export file for selected SQL result cursors");
    std::ifstream input(export_path);
    std::string json_text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    expect(json_text.find("\"NAME\"") != std::string::npos,
        "COPY TO TYPE JSON export should include NAME field entries");
    expect(json_text.find("\"AMOUNT\"") != std::string::npos,
        "COPY TO TYPE JSON export should include AMOUNT field entries");
    expect(json_text.find("\"ID\"") == std::string::npos,
        "COPY TO TYPE JSON with FIELDS NAME, AMOUNT should omit ID entries");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_dbf_mutates_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_export";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source_rows.dbf";
    const fs::path main_path = temp_root / "sql_append_from_selected_cursor.prg";

    const auto source_write = copperfin::vfp::create_dbf_table_file(
        source_path.string(),
        {
            copperfin::vfp::DbfFieldDescriptor{.name = "ID", .type = 'N', .length = 6U, .decimal_count = 0U},
            copperfin::vfp::DbfFieldDescriptor{.name = "NAME", .type = 'C', .length = 20U, .decimal_count = 0U},
            copperfin::vfp::DbfFieldDescriptor{.name = "AMOUNT", .type = 'N', .length = 10U, .decimal_count = 2U},
        },
        {
            {"501", "DELTA", "5"},
            {"502", "ECHO", "6"},
        });
    expect(source_write.ok, "source DBF fixture for SQL APPEND FROM should be created successfully");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "nBottomAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM selected SQL result-cursor script should complete: " + state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto bottom_amount = state.globals.find("nbottomamount");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(), "selected SQL cursor row count before APPEND FROM should be captured");
    expect(rows_after != state.globals.end(), "selected SQL cursor row count after APPEND FROM should be captured");
    expect(bottom_id != state.globals.end(), "selected SQL cursor bottom ID after APPEND FROM should be captured");
    expect(bottom_name != state.globals.end(), "selected SQL cursor bottom NAME after APPEND FROM should be captured");
    expect(bottom_amount != state.globals.end(), "selected SQL cursor bottom AMOUNT after APPEND FROM should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after APPEND FROM selected SQL cursor checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
            "selected SQL result cursor should start with seeded row count before APPEND FROM");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "5",
            "APPEND FROM should add DBF source rows into selected SQL/result cursor");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "502",
            "APPEND FROM should append DBF values into selected SQL/result cursor ID field");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "ECHO",
            "APPEND FROM should append DBF values into selected SQL/result cursor NAME field");
    }
    if (bottom_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_amount->second) == "6",
            "APPEND FROM should append DBF values into selected SQL/result cursor AMOUNT field");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after APPEND FROM selected SQL cursor checks");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_structure_to_exports_sql_metadata_cursor_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_copy_structure_metadata";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path export_path = temp_root / "sql_meta_schema.dbf";
    const fs::path main_path = temp_root / "sql_copy_structure_metadata.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nMissingPk = SQLPRIMARYKEYS(nConn, 'DOES_NOT_EXIST', 'missingpk')\n"
        "SELECT missingpk\n"
        "cLogicalField2 = FIELD(2, 'missingpk')\n"
        "COPY STRUCTURE TO '" + export_path.string() + "'\n"
        "cAliasAfterCopy = ALIAS()\n"
        "USE '" + export_path.string() + "' ALIAS local IN 0\n"
        "nRows = RECCOUNT('local')\n"
        "nFields = FCOUNT('local')\n"
        "cField2 = FIELD(2, 'local')\n"
        "cField4 = FIELD(4, 'local')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY STRUCTURE TO SQL metadata cursor script should complete: " + state.message);

    const auto alias_after_copy = state.globals.find("caliasaftercopy");
    const auto rows = state.globals.find("nrows");
    const auto fields = state.globals.find("nfields");
    const auto logical_field2 = state.globals.find("clogicalfield2");
    const auto field2 = state.globals.find("cfield2");
    const auto field4 = state.globals.find("cfield4");
    const auto disc = state.globals.find("ldisc");

    expect(alias_after_copy != state.globals.end(), "selected alias after SQL metadata COPY STRUCTURE TO should be captured");
    expect(rows != state.globals.end(), "schema-exported local row count should be captured");
    expect(fields != state.globals.end(), "schema-exported local field count should be captured");
    expect(logical_field2 != state.globals.end(), "logical SQL metadata FIELD(2) should be captured");
    expect(field2 != state.globals.end(), "schema-exported FIELD(2) should be captured");
    expect(field4 != state.globals.end(), "schema-exported FIELD(4) should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after SQL metadata schema export checks");

    if (alias_after_copy != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after_copy->second)) == "MISSINGPK",
            "COPY STRUCTURE TO should preserve selected SQL metadata alias before opening other cursors");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "0",
            "COPY STRUCTURE TO should export schema-only DBF with zero rows from SQL metadata cursor");
    }
    if (fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields->second) == "6",
            "COPY STRUCTURE TO should export all SQL metadata cursor fields");
    }
    if (logical_field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(logical_field2->second) == "TABLE_SCHEM",
            "DBF export should not change the SQL metadata cursor's logical field names");
    }
    if (field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field2->second) == "TABLE_SCHE",
            "COPY STRUCTURE TO should map long logical SQL metadata names to 10-byte physical names");
    }
    if (field4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field4->second) == "COLUMN_NAM",
            "COPY STRUCTURE TO should preserve SQL metadata field order while mapping physical names");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after SQL metadata schema export checks");
    }

    expect(fs::exists(export_path), "COPY STRUCTURE TO should create DBF output for SQL metadata cursors");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(export_path.string(), 10U);
    expect(parse_result.ok, "SQL metadata COPY STRUCTURE TO output should be readable as DBF");
    if (parse_result.ok) {
        expect(parse_result.table.records.empty(),
            "SQL metadata COPY STRUCTURE TO output should be schema-only");
        expect(parse_result.table.fields.size() == 6U,
            "SQL metadata COPY STRUCTURE TO output should keep metadata field count");
        for (const auto &field : parse_result.table.fields) {
            expect(field.name.size() <= 10U,
                "SQL metadata COPY STRUCTURE TO output should use valid free-table physical field names");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_dbf_for_filters_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_append_from_for_filter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source_rows.dbf";
    const fs::path main_path = temp_root / "sql_append_from_for_filter.prg";

    const auto source_write = copperfin::vfp::create_dbf_table_file(
        source_path.string(),
        {
            copperfin::vfp::DbfFieldDescriptor{.name = "ID", .type = 'N', .length = 6U, .decimal_count = 0U},
            copperfin::vfp::DbfFieldDescriptor{.name = "NAME", .type = 'C', .length = 20U, .decimal_count = 0U},
            copperfin::vfp::DbfFieldDescriptor{.name = "AMOUNT", .type = 'N', .length = 10U, .decimal_count = 2U},
        },
        {
            {"701", "DELTA", "7"},
            {"702", "ECHO", "8"},
        });
    expect(source_write.ok, "source DBF fixture for SQL APPEND FROM FOR filter should be created successfully");

    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "nRowsBefore = RECCOUNT()\n"
        "APPEND FROM '" + source_path.string() + "' FOR NAME == 'DELTA'\n"
        "nRowsAfter = RECCOUNT()\n"
        "GO BOTTOM\n"
        "nBottomId = ID\n"
        "cBottomName = NAME\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM FOR selected SQL result-cursor script should complete: " + state.message);

    const auto rows_before = state.globals.find("nrowsbefore");
    const auto rows_after = state.globals.find("nrowsafter");
    const auto bottom_id = state.globals.find("nbottomid");
    const auto bottom_name = state.globals.find("cbottomname");
    const auto disc = state.globals.find("ldisc");

    expect(rows_before != state.globals.end(), "selected SQL cursor row count before APPEND FROM FOR should be captured");
    expect(rows_after != state.globals.end(), "selected SQL cursor row count after APPEND FROM FOR should be captured");
    expect(bottom_id != state.globals.end(), "selected SQL cursor bottom ID after APPEND FROM FOR should be captured");
    expect(bottom_name != state.globals.end(), "selected SQL cursor bottom NAME after APPEND FROM FOR should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after APPEND FROM FOR checks");

    if (rows_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_before->second) == "3",
            "selected SQL result cursor should start with seeded row count before APPEND FROM FOR");
    }
    if (rows_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_after->second) == "4",
            "APPEND FROM FOR should append only records matching the FOR expression");
    }
    if (bottom_id != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_id->second) == "701",
            "APPEND FROM FOR should append only matching DBF source rows into selected SQL/result cursor ID field");
    }
    if (bottom_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(bottom_name->second) == "DELTA",
            "APPEND FROM FOR should append only matching DBF source rows into selected SQL/result cursor NAME field");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after APPEND FROM FOR checks");
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
        "SET DELETED ON\n"
        "RECALL FOR NAME = 'BRAVO'\n"
        "SET DELETED OFF\n"
        "GO 2\n"
        "lRecalledBravo = DELETED()\n"
        "nCount = RECCOUNT('sqlcust')\n"
        "GO BOTTOM\n"
        "cLastName = NAME\n"
        "nLastAmount = AMOUNT\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_delete_all_and_recall_all_affect_whole_selected_sql_result_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_delete_recall_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_delete_recall_all.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "SELECT sqlcust\n"
        "GO 2\n"
        "DELETE ALL\n"
        "GO 1\n"
        "lDeleted1 = DELETED()\n"
        "GO 2\n"
        "lDeleted2 = DELETED()\n"
        "GO 3\n"
        "lDeleted3 = DELETED()\n"
        "RECALL ALL\n"
        "GO 1\n"
        "lRecalled1 = DELETED()\n"
        "GO 2\n"
        "lRecalled2 = DELETED()\n"
        "GO 3\n"
        "lRecalled3 = DELETED()\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3683: DELETE ALL / RECALL ALL selected SQL/result cursor script should complete");

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), "#3683: " + name + " should be captured");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   "#3683: " + name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("ldeleted1", "true");
    check("ldeleted2", "true");
    check("ldeleted3", "true");
    check("lrecalled1", "false");
    check("lrecalled2", "false");
    check("lrecalled3", "false");
    check("ldisc", "1");

    fs::remove_all(temp_root, ignored);
}

void test_targeted_sql_result_cursor_mutations_preserve_selected_alias_and_pointer() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_mutations_in_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_mutations_in_target.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nExecCust = SQLEXEC(nConn, 'select * from customers', 'sqlcust')\n"
        "nExecOther = SQLEXEC(nConn, 'select * from customers', 'sqlother')\n"
        "SELECT sqlother\n"
        "GO 2\n"
        "nOtherRecBefore = RECNO('sqlother')\n"
        "GO BOTTOM IN sqlcust\n"
        "nCustRecBefore = RECNO('sqlcust')\n"
        "APPEND BLANK IN sqlcust\n"
        "nCustRecAfterAppend = RECNO('sqlcust')\n"
        "REPLACE NAME WITH 'DELTA', AMOUNT WITH 99 IN sqlcust\n"
        "DELETE FOR NAME = 'BRAVO' IN sqlcust\n"
        "RECALL FOR NAME = 'BRAVO' IN sqlcust\n"
        "cAliasAfter = ALIAS()\n"
        "nOtherRecAfter = RECNO('sqlother')\n"
        "nCustCount = RECCOUNT('sqlcust')\n"
        "SELECT sqlcust\n"
        "GO 2\n"
        "lBravoDeleted = DELETED()\n"
        "GO BOTTOM\n"
        "cLastName = NAME\n"
        "nLastAmount = AMOUNT\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "targeted SQL mutation isolation script should complete: " + state.message);
    expect(state.sql_connections.empty(), "targeted SQL mutation isolation script should disconnect its SQL handle");

    const auto other_rec_before = state.globals.find("notherrecbefore");
    const auto cust_rec_before = state.globals.find("ncustrecbefore");
    const auto cust_rec_after_append = state.globals.find("ncustrecafterappend");
    const auto alias_after = state.globals.find("caliasafter");
    const auto other_rec_after = state.globals.find("notherrecafter");
    const auto cust_count = state.globals.find("ncustcount");
    const auto bravo_deleted = state.globals.find("lbravodeleted");
    const auto last_name = state.globals.find("clastname");
    const auto last_amount = state.globals.find("nlastamount");
    const auto disc = state.globals.find("ldisc");

    expect(other_rec_before != state.globals.end(), "selected SQL cursor RECNO() before targeted mutation should be captured");
    expect(cust_rec_before != state.globals.end(), "target SQL cursor RECNO() before targeted append should be captured");
    expect(cust_rec_after_append != state.globals.end(), "target SQL cursor RECNO() after targeted append should be captured");
    expect(alias_after != state.globals.end(), "selected alias after targeted SQL mutations should be captured");
    expect(other_rec_after != state.globals.end(), "selected SQL cursor RECNO() after targeted SQL mutations should be captured");
    expect(cust_count != state.globals.end(), "target SQL cursor RECCOUNT() after targeted mutations should be captured");
    expect(bravo_deleted != state.globals.end(), "target SQL cursor DELETED() state after targeted recall should be captured");
    expect(last_name != state.globals.end(), "target SQL cursor appended NAME after targeted mutations should be captured");
    expect(last_amount != state.globals.end(), "target SQL cursor appended AMOUNT after targeted mutations should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after targeted SQL mutation checks");

    if (other_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_before->second) == "2",
            "selected SQL cursor should start on row 2 before targeted SQL mutations");
    }
    if (cust_rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_before->second) == "3",
            "GO BOTTOM IN should move the targeted SQL cursor to its last row before targeted append");
    }
    if (cust_rec_after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_rec_after_append->second) == "4",
            "APPEND BLANK IN should advance only the targeted SQL cursor to the appended row");
    }
    if (alias_after != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias_after->second)) == "SQLOTHER",
            "targeted SQL mutations should preserve the selected alias");
    }
    if (other_rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec_after->second) == "2",
            "targeted SQL mutations should preserve the selected SQL cursor pointer");
    }
    if (cust_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(cust_count->second) == "4",
            "targeted SQL APPEND BLANK should grow only the targeted SQL cursor record count");
    }
    if (bravo_deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(bravo_deleted->second) == "false",
            "targeted SQL RECALL should clear the deletion flag on the targeted result cursor row");
    }
    if (last_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_name->second) == "DELTA",
            "targeted SQL REPLACE should persist appended NAME values on the targeted result cursor");
    }
    if (last_amount != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_amount->second) == "99",
            "targeted SQL REPLACE should persist appended AMOUNT values on the targeted result cursor");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1",
            "SQLDISCONNECT should succeed after targeted SQL mutation checks");
    }

    expect(
        has_runtime_event(state.events, "runtime.append_blank", "sqlcust") &&
        has_runtime_event(state.events, "runtime.replace", "NAME WITH 'DELTA', AMOUNT WITH 99") &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "targeted SQL mutation flow should emit append/replace/delete/recall runtime events");

    fs::remove_all(temp_root, ignored);
}

}  // namespace copperfin::sql_cursor_mutation_tests

