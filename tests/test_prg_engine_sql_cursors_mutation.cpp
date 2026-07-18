// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

}  // namespace

int main() {
    test_sql_result_cursor_backward_navigation_in_target_parity();
    test_cursor_identity_functions_for_sql_result_cursors();
    test_copy_to_exports_selected_sql_result_cursor_rows();
    test_copy_to_type_json_exports_selected_sql_result_cursor_and_preserves_selection();
    test_append_from_dbf_mutates_selected_sql_result_cursor();
    test_copy_structure_to_exports_sql_metadata_cursor_schema();
    test_append_from_dbf_for_filters_selected_sql_result_cursor();
    test_sql_result_cursor_mutation_commands();
    test_delete_all_and_recall_all_affect_whole_selected_sql_result_cursor();
    test_targeted_sql_result_cursor_mutations_preserve_selected_alias_and_pointer();
    test_sql_result_cursors_are_isolated_by_data_session();
    test_sql_result_cursor_auto_allocation_tracks_session_selection_flow();
    test_sql_result_cursors_and_ole_actions();
    test_sql_result_cursor_mutation_parity();
    test_sql_result_cursor_multi_field_replace_uses_original_values_for_later_expressions();
    test_sql_result_cursor_sql_style_mutation_parity();
    test_sql_result_cursor_mutation_in_target_parity();
    test_append_from_json_mutates_selected_sql_result_cursor();
    test_append_from_csv_mutates_selected_sql_result_cursor();
    test_append_from_delimited_fields_clause_preserves_typed_order_for_selected_sql_result_cursor();
    test_append_from_selected_sql_result_cursor_runtime_errors_localize();
    test_sql_plain_temporary_order_in_target_honors_collate_and_preserves_selection();
    test_append_from_json_for_filters_selected_sql_result_cursor();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
