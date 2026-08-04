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


void test_sqlprimarykeys_and_sqlforeignkeys_metadata_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_key_metadata";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_key_metadata.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nPrimary = SQLPRIMARYKEYS(nConn, 'ORD*', 'sqlpk')\n"
        "nPkRows = RECCOUNT('sqlpk')\n"
        "nPkFields = FCOUNT('sqlpk')\n"
        "cPkField4 = FIELD(4, 'sqlpk')\n"
        "nForeign = SQLFOREIGNKEYS(nConn, 'ORD*', 'sqlfk')\n"
        "nFkRows = RECCOUNT('sqlfk')\n"
        "nFkFields = FCOUNT('sqlfk')\n"
        "cFkField8 = FIELD(8, 'sqlfk')\n"
        "nMissingPk = SQLPRIMARYKEYS(nConn, 'DOES_NOT_EXIST', 'missingpk')\n"
        "lMissingPkUsed = USED('missingpk')\n"
        "nMissingPkRows = RECCOUNT('missingpk')\n"
        "nMissingForeign = SQLFOREIGNKEYS(nConn, 'DOES_NOT_EXIST', 'missingfk')\n"
        "lMissingFkUsed = USED('missingfk')\n"
        "nMissingFkRows = RECCOUNT('missingfk')\n"
        "cActionAfterKeys = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL key metadata cursor script should complete");

    const auto primary = state.globals.find("nprimary");
    const auto pk_rows = state.globals.find("npkrows");
    const auto pk_fields = state.globals.find("npkfields");
    const auto pk_field4 = state.globals.find("cpkfield4");
    const auto foreign = state.globals.find("nforeign");
    const auto fk_rows = state.globals.find("nfkrows");
    const auto fk_fields = state.globals.find("nfkfields");
    const auto fk_field8 = state.globals.find("cfkfield8");
    const auto missing_pk = state.globals.find("nmissingpk");
    const auto missing_pk_used = state.globals.find("lmissingpkused");
    const auto missing_pk_rows = state.globals.find("nmissingpkrows");
    const auto missing_foreign = state.globals.find("nmissingforeign");
    const auto missing_fk_used = state.globals.find("lmissingfkused");
    const auto missing_fk_rows = state.globals.find("nmissingfkrows");
    const auto action_after_keys = state.globals.find("cactionafterkeys");
    const auto disc = state.globals.find("ldisc");

    expect(primary != state.globals.end(), "SQLPRIMARYKEYS result should be captured");
    expect(pk_rows != state.globals.end(), "SQLPRIMARYKEYS row count should be captured");
    expect(pk_fields != state.globals.end(), "SQLPRIMARYKEYS field count should be captured");
    expect(pk_field4 != state.globals.end(), "FIELD(index, alias) should be captured for SQLPRIMARYKEYS metadata cursors");
    expect(foreign != state.globals.end(), "SQLFOREIGNKEYS result should be captured");
    expect(fk_rows != state.globals.end(), "SQLFOREIGNKEYS row count should be captured");
    expect(fk_fields != state.globals.end(), "SQLFOREIGNKEYS field count should be captured");
    expect(fk_field8 != state.globals.end(), "FIELD(index, alias) should be captured for SQLFOREIGNKEYS metadata cursors");
    expect(missing_pk != state.globals.end(), "missing-table SQLPRIMARYKEYS result should be captured");
    expect(missing_pk_used != state.globals.end(), "missing-table SQLPRIMARYKEYS cursor visibility should be captured");
    expect(missing_pk_rows != state.globals.end(), "missing-table SQLPRIMARYKEYS row count should be captured");
    expect(missing_foreign != state.globals.end(), "missing-table SQLFOREIGNKEYS result should be captured");
    expect(missing_fk_used != state.globals.end(), "missing-table SQLFOREIGNKEYS cursor visibility should be captured");
    expect(missing_fk_rows != state.globals.end(), "missing-table SQLFOREIGNKEYS row count should be captured");
    expect(action_after_keys != state.globals.end(), "last SQL action should be captured after key metadata helpers");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after key metadata helper checks");

    if (primary != state.globals.end()) {
        expect(copperfin::runtime::format_value(primary->second) == "1", "SQLPRIMARYKEYS should succeed for a valid SQL handle");
    }
    if (pk_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(pk_rows->second) == "1", "SQLPRIMARYKEYS should expose one synthetic primary-key row for ORDERS");
    }
    if (pk_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(pk_fields->second) == "6", "SQLPRIMARYKEYS should expose the expected metadata schema");
    }
    if (pk_field4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(pk_field4->second) == "COLUMN_NAME", "SQLPRIMARYKEYS metadata cursor should expose column-name metadata");
    }
    if (foreign != state.globals.end()) {
        expect(copperfin::runtime::format_value(foreign->second) == "1", "SQLFOREIGNKEYS should succeed for a valid SQL handle");
    }
    if (fk_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(fk_rows->second) == "1", "SQLFOREIGNKEYS should expose one synthetic relationship row for ORDERS");
    }
    if (fk_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fk_fields->second) == "14", "SQLFOREIGNKEYS should expose the expected metadata schema");
    }
    if (fk_field8 != state.globals.end()) {
        expect(copperfin::runtime::format_value(fk_field8->second) == "FKCOLUMN_NAME", "SQLFOREIGNKEYS metadata cursor should expose foreign-column metadata");
    }
    if (missing_pk != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_pk->second) == "1", "missing-table SQLPRIMARYKEYS should still succeed with an empty metadata cursor");
    }
    if (missing_pk_used != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_pk_used->second) == "true", "missing-table SQLPRIMARYKEYS should materialize an empty cursor");
    }
    if (missing_pk_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_pk_rows->second) == "0", "missing-table SQLPRIMARYKEYS should produce zero metadata rows");
    }
    if (missing_foreign != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_foreign->second) == "1", "missing-table SQLFOREIGNKEYS should still succeed with an empty metadata cursor");
    }
    if (missing_fk_used != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_fk_used->second) == "true", "missing-table SQLFOREIGNKEYS should materialize an empty cursor");
    }
    if (missing_fk_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_fk_rows->second) == "0", "missing-table SQLFOREIGNKEYS should produce zero metadata rows");
    }
    if (action_after_keys != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_keys->second) == "foreignkeys", "key metadata helper batch should leave the connection last-action metadata on foreignkeys");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after key metadata helper checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "sql.primarykeys" && event.detail.rfind("handle 1:", 0) == 0;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "sql.foreignkeys" && event.detail.rfind("handle 1:", 0) == 0;
        }),
        "key metadata helper batch should emit sql.primarykeys and sql.foreignkeys events");

    fs::remove_all(temp_root, ignored);
}

void test_sqldatabases_metadata_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_databases";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_databases.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nDatabases = SQLDATABASES(nConn, 'sqldbs')\n"
        "nDbRows = RECCOUNT('sqldbs')\n"
        "nDbFields = FCOUNT('sqldbs')\n"
        "cDbField1 = FIELD(1, 'sqldbs')\n"
        "cDbField2 = FIELD(2, 'sqldbs')\n"
        "cActionAfterDatabases = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL databases metadata script should complete");

    const auto databases = state.globals.find("ndatabases");
    const auto db_rows = state.globals.find("ndbrows");
    const auto db_fields = state.globals.find("ndbfields");
    const auto db_field1 = state.globals.find("cdbfield1");
    const auto db_field2 = state.globals.find("cdbfield2");
    const auto action_after_databases = state.globals.find("cactionafterdatabases");
    const auto disc = state.globals.find("ldisc");

    expect(databases != state.globals.end(), "SQLDATABASES result should be captured");
    expect(db_rows != state.globals.end(), "SQLDATABASES row count should be captured");
    expect(db_fields != state.globals.end(), "SQLDATABASES field count should be captured");
    expect(db_field1 != state.globals.end(), "FIELD(index, alias) should be captured for SQLDATABASES metadata cursors");
    expect(db_field2 != state.globals.end(), "FIELD(index, alias) should be captured for SQLDATABASES metadata cursors");
    expect(action_after_databases != state.globals.end(), "last SQL action should be captured after SQLDATABASES");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after SQLDATABASES checks");

    if (databases != state.globals.end()) {
        expect(copperfin::runtime::format_value(databases->second) == "1", "SQLDATABASES should succeed for a valid SQL handle");
    }
    if (db_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(db_rows->second) == "2", "SQLDATABASES should materialize the synthetic catalog list");
    }
    if (db_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(db_fields->second) == "2", "SQLDATABASES should expose the expected metadata schema");
    }
    if (db_field1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(db_field1->second) == "DATABASE_NAME", "SQLDATABASES metadata cursor should expose the database-name column");
    }
    if (db_field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(db_field2->second) == "REMARKS", "SQLDATABASES metadata cursor should expose the remarks column");
    }
    if (action_after_databases != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_databases->second) == "databases", "SQLDATABASES should update the connection last-action metadata");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQLDATABASES checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "sql.databases" && event.detail.rfind("handle 1", 0) == 0;
        }),
        "SQLDATABASES should emit a sql.databases event");

    fs::remove_all(temp_root, ignored);
}

void test_sqltables_and_sqlcolumns_metadata_cursors() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_metadata";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_metadata.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "nTablesAll = SQLTABLES(nConn, '', 'sqltabs')\n"
        "nTableRows = RECCOUNT('sqltabs')\n"
        "cTableField3 = FIELD(3, 'sqltabs')\n"
        "nTablesView = SQLTABLES(nConn, \"'VIEW'\", 'sqlviews')\n"
        "nViewRows = RECCOUNT('sqlviews')\n"
        "cViewField4 = FIELD(4, 'sqlviews')\n"
        "nColsFox = SQLCOLUMNS(nConn, 'CUSTOMERS', 'FOXPRO', 'sqlcolsfox')\n"
        "nFoxRows = RECCOUNT('sqlcolsfox')\n"
        "nFoxFields = FCOUNT('sqlcolsfox')\n"
        "cFoxField1 = FIELD(1, 'sqlcolsfox')\n"
        "cFoxField2 = FIELD(2, 'sqlcolsfox')\n"
        "nColsNative = SQLCOLUMNS(nConn, 'ORD*', 'NATIVE', 'sqlcolsnative')\n"
        "nNativeRows = RECCOUNT('sqlcolsnative')\n"
        "nNativeFields = FCOUNT('sqlcolsnative')\n"
        "cNativeField4 = FIELD(4, 'sqlcolsnative')\n"
        "cNativeField5 = FIELD(5, 'sqlcolsnative')\n"
        "nMissingFox = SQLCOLUMNS(nConn, 'DOES_NOT_EXIST', 'FOXPRO', 'missingfox')\n"
        "lMissingFoxUsed = USED('missingfox')\n"
        "nMissingNative = SQLCOLUMNS(nConn, 'DOES_NOT_EXIST', 'NATIVE', 'missingnative')\n"
        "lMissingNativeUsed = USED('missingnative')\n"
        "nMissingNativeRows = RECCOUNT('missingnative')\n"
        "nMissingNativeFields = FCOUNT('missingnative')\n"
        "cActionAfterColumns = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL metadata cursor script should complete");

    const auto tables_all = state.globals.find("ntablesall");
    const auto table_rows = state.globals.find("ntablerows");
    const auto table_field3 = state.globals.find("ctablefield3");
    const auto tables_view = state.globals.find("ntablesview");
    const auto view_rows = state.globals.find("nviewrows");
    const auto view_field4 = state.globals.find("cviewfield4");
    const auto cols_fox = state.globals.find("ncolsfox");
    const auto fox_rows = state.globals.find("nfoxrows");
    const auto fox_fields = state.globals.find("nfoxfields");
    const auto fox_field1 = state.globals.find("cfoxfield1");
    const auto fox_field2 = state.globals.find("cfoxfield2");
    const auto cols_native = state.globals.find("ncolsnative");
    const auto native_rows = state.globals.find("nnativerows");
    const auto native_fields = state.globals.find("nnativefields");
    const auto native_field4 = state.globals.find("cnativefield4");
    const auto native_field5 = state.globals.find("cnativefield5");
    const auto missing_fox = state.globals.find("nmissingfox");
    const auto missing_fox_used = state.globals.find("lmissingfoxused");
    const auto missing_native = state.globals.find("nmissingnative");
    const auto missing_native_used = state.globals.find("lmissingnativeused");
    const auto missing_native_rows = state.globals.find("nmissingnativerows");
    const auto missing_native_fields = state.globals.find("nmissingnativefields");
    const auto action_after_columns = state.globals.find("cactionaftercolumns");
    const auto disc = state.globals.find("ldisc");

    expect(tables_all != state.globals.end(), "SQLTABLES result should be captured for all-table metadata");
    expect(table_rows != state.globals.end(), "SQLTABLES row count should be captured");
    expect(table_field3 != state.globals.end(), "FIELD(index, alias) should be captured for SQLTABLES metadata cursors");
    expect(tables_view != state.globals.end(), "filtered SQLTABLES result should be captured");
    expect(view_rows != state.globals.end(), "filtered SQLTABLES row count should be captured");
    expect(view_field4 != state.globals.end(), "FIELD(index, alias) should be captured for filtered SQLTABLES metadata cursors");
    expect(cols_fox != state.globals.end(), "FOXPRO SQLCOLUMNS result should be captured");
    expect(fox_rows != state.globals.end(), "FOXPRO SQLCOLUMNS row count should be captured");
    expect(fox_fields != state.globals.end(), "FOXPRO SQLCOLUMNS field count should be captured");
    expect(fox_field1 != state.globals.end(), "FIELD(index, alias) should be captured for FOXPRO SQLCOLUMNS metadata cursors");
    expect(fox_field2 != state.globals.end(), "FIELD(index, alias) should be captured for FOXPRO SQLCOLUMNS metadata cursors");
    expect(cols_native != state.globals.end(), "NATIVE SQLCOLUMNS result should be captured");
    expect(native_rows != state.globals.end(), "NATIVE SQLCOLUMNS row count should be captured");
    expect(native_fields != state.globals.end(), "NATIVE SQLCOLUMNS field count should be captured");
    expect(native_field4 != state.globals.end(), "FIELD(index, alias) should be captured for NATIVE SQLCOLUMNS metadata cursors");
    expect(native_field5 != state.globals.end(), "FIELD(index, alias) should be captured for NATIVE SQLCOLUMNS metadata cursors");
    expect(missing_fox != state.globals.end(), "missing-table FOXPRO SQLCOLUMNS result should be captured");
    expect(missing_fox_used != state.globals.end(), "missing-table FOXPRO SQLCOLUMNS cursor visibility should be captured");
    expect(missing_native != state.globals.end(), "missing-table NATIVE SQLCOLUMNS result should be captured");
    expect(missing_native_used != state.globals.end(), "missing-table NATIVE SQLCOLUMNS cursor visibility should be captured");
    expect(missing_native_rows != state.globals.end(), "missing-table NATIVE SQLCOLUMNS row count should be captured");
    expect(missing_native_fields != state.globals.end(), "missing-table NATIVE SQLCOLUMNS field count should be captured");
    expect(action_after_columns != state.globals.end(), "last SQL action should be captured after metadata helpers");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after metadata helper checks");

    if (tables_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(tables_all->second) == "1", "SQLTABLES should succeed for a valid SQL handle");
    }
    if (table_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(table_rows->second) == "4", "SQLTABLES should materialize all synthetic catalog rows when no type filter is supplied");
    }
    if (table_field3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(table_field3->second) == "TABLE_NAME", "SQLTABLES metadata cursor should expose the expected schema");
    }
    if (tables_view != state.globals.end()) {
        expect(copperfin::runtime::format_value(tables_view->second) == "1", "filtered SQLTABLES should succeed for a valid SQL handle");
    }
    if (view_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(view_rows->second) == "1", "SQLTABLES type filtering should narrow the synthetic catalog rows");
    }
    if (view_field4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(view_field4->second) == "TABLE_TYPE", "filtered SQLTABLES should preserve the expected metadata schema");
    }
    if (cols_fox != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols_fox->second) == "1", "FOXPRO SQLCOLUMNS should succeed for a known synthetic table");
    }
    if (fox_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(fox_rows->second) == "3", "FOXPRO SQLCOLUMNS should expose one row per synthetic source column");
    }
    if (fox_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fox_fields->second) == "4", "FOXPRO SQLCOLUMNS should expose the FoxPro-style metadata schema");
    }
    if (fox_field1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(fox_field1->second) == "FIELD_NAME", "FOXPRO SQLCOLUMNS should expose the FoxPro-style metadata schema");
    }
    if (fox_field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(fox_field2->second) == "FIELD_TYPE", "FOXPRO SQLCOLUMNS should expose FoxPro-style metadata columns");
    }
    if (cols_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols_native->second) == "1", "NATIVE SQLCOLUMNS should succeed for wildcard-matched synthetic tables");
    }
    if (native_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(native_rows->second) == "3", "NATIVE SQLCOLUMNS should expose the synthetic native metadata rows");
    }
    if (native_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(native_fields->second) == "8", "NATIVE SQLCOLUMNS should expose the native-style metadata schema");
    }
    if (native_field4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(native_field4->second) == "COLUMN_NAME", "NATIVE SQLCOLUMNS should expose native-style metadata schema");
    }
    if (native_field5 != state.globals.end()) {
        expect(copperfin::runtime::format_value(native_field5->second) == "TYPE_NAME", "NATIVE SQLCOLUMNS should expose native-style type metadata columns");
    }
    if (missing_fox != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_fox->second) == "0", "missing-table FOXPRO SQLCOLUMNS should return false-like 0 in the first-pass runtime");
    }
    if (missing_fox_used != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_fox_used->second) == "false", "missing-table FOXPRO SQLCOLUMNS should not materialize a cursor");
    }
    if (missing_native != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_native->second) == "1", "missing-table NATIVE SQLCOLUMNS should still succeed with an empty metadata cursor");
    }
    if (missing_native_used != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_native_used->second) == "true", "missing-table NATIVE SQLCOLUMNS should materialize an empty cursor");
    }
    if (missing_native_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_native_rows->second) == "0", "missing-table NATIVE SQLCOLUMNS should produce zero metadata rows");
    }
    if (missing_native_fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing_native_fields->second) == "8", "missing-table NATIVE SQLCOLUMNS should preserve the native metadata schema");
    }
    if (action_after_columns != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_columns->second) == "columns", "metadata helper batch should leave the connection last-action metadata on columns");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after metadata helper checks");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "sql.tables" && event.detail.rfind("handle 1:", 0) == 0;
        }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "sql.columns" && event.detail.rfind("handle 1:", 0) == 0;
        }),
        "metadata helper batch should emit sql.tables and sql.columns events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_connection_transaction_and_cancel_helpers() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_txn_helpers";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_txn_helpers.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Driver=ODBC Driver 18 for SQL Server;Server=Northwind')\n"
        "nInsert = SQLEXEC(nConn, 'insert into customers values (1)')\n"
        "lDirtyAfterInsert = SQLGETPROP(nConn, 'TransactionDirty')\n"
        "nCancel = SQLCANCEL(nConn)\n"
        "lCancelRequested = SQLGETPROP(nConn, 'CancelRequested')\n"
        "cActionAfterCancel = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "nCommit = SQLCOMMIT(nConn)\n"
        "lDirtyAfterCommit = SQLGETPROP(nConn, 'TransactionDirty')\n"
        "lCancelAfterCommit = SQLGETPROP(nConn, 'CancelRequested')\n"
        "cActionAfterCommit = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "nUpdate = SQLEXEC(nConn, 'update customers set id = 2 where id = 1')\n"
        "nRollback = SQLROLLBACK(nConn)\n"
        "lDirtyAfterRollback = SQLGETPROP(nConn, 'TransactionDirty')\n"
        "cActionAfterRollback = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "SET DATASESSION TO 2\n"
        "nCrossSessionCommit = SQLCOMMIT(nConn)\n"
        "SET DATASESSION TO 1\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL transaction/cancel helper script should complete");

    const auto insert = state.globals.find("ninsert");
    const auto dirty_after_insert = state.globals.find("ldirtyafterinsert");
    const auto cancel = state.globals.find("ncancel");
    const auto cancel_requested = state.globals.find("lcancelrequested");
    const auto action_after_cancel = state.globals.find("cactionaftercancel");
    const auto commit = state.globals.find("ncommit");
    const auto dirty_after_commit = state.globals.find("ldirtyaftercommit");
    const auto cancel_after_commit = state.globals.find("lcancelaftercommit");
    const auto action_after_commit = state.globals.find("cactionaftercommit");
    const auto update = state.globals.find("nupdate");
    const auto rollback = state.globals.find("nrollback");
    const auto dirty_after_rollback = state.globals.find("ldirtyafterrollback");
    const auto action_after_rollback = state.globals.find("cactionafterrollback");
    const auto cross_session_commit = state.globals.find("ncrosssessioncommit");
    const auto disc = state.globals.find("ldisc");

    expect(insert != state.globals.end(), "SQLEXEC insert result should be captured for SQL transaction helper parity");
    expect(dirty_after_insert != state.globals.end(), "transaction-dirty property should be captured after DML");
    expect(cancel != state.globals.end(), "SQLCANCEL result should be captured");
    expect(cancel_requested != state.globals.end(), "cancel-requested property should be captured after SQLCANCEL");
    expect(action_after_cancel != state.globals.end(), "last SQL action should be captured after SQLCANCEL");
    expect(commit != state.globals.end(), "SQLCOMMIT result should be captured");
    expect(dirty_after_commit != state.globals.end(), "transaction-dirty property should be captured after SQLCOMMIT");
    expect(cancel_after_commit != state.globals.end(), "cancel-requested property should be captured after SQLCOMMIT");
    expect(action_after_commit != state.globals.end(), "last SQL action should be captured after SQLCOMMIT");
    expect(update != state.globals.end(), "SQLEXEC update result should be captured for SQL rollback parity");
    expect(rollback != state.globals.end(), "SQLROLLBACK result should be captured");
    expect(dirty_after_rollback != state.globals.end(), "transaction-dirty property should be captured after SQLROLLBACK");
    expect(action_after_rollback != state.globals.end(), "last SQL action should be captured after SQLROLLBACK");
    expect(cross_session_commit != state.globals.end(), "cross-session SQLCOMMIT result should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after SQL helper checks");

    if (insert != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert->second) == "1", "SQLEXEC insert should succeed before SQL transaction helper checks");
    }
    if (dirty_after_insert != state.globals.end()) {
        expect(copperfin::runtime::format_value(dirty_after_insert->second) == "true", "DML SQLEXEC should mark the SQL connection transaction-dirty");
    }
    if (cancel != state.globals.end()) {
        expect(copperfin::runtime::format_value(cancel->second) == "1", "SQLCANCEL should succeed for a valid session-local SQL handle");
    }
    if (cancel_requested != state.globals.end()) {
        expect(copperfin::runtime::format_value(cancel_requested->second) == "true", "SQLCANCEL should mark the connection cancel-requested flag");
    }
    if (action_after_cancel != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_cancel->second) == "cancel", "SQLCANCEL should update the connection last-action metadata");
    }
    if (commit != state.globals.end()) {
        expect(copperfin::runtime::format_value(commit->second) == "1", "SQLCOMMIT should succeed for a valid session-local SQL handle");
    }
    if (dirty_after_commit != state.globals.end()) {
        expect(copperfin::runtime::format_value(dirty_after_commit->second) == "false", "SQLCOMMIT should clear the transaction-dirty flag");
    }
    if (cancel_after_commit != state.globals.end()) {
        expect(copperfin::runtime::format_value(cancel_after_commit->second) == "false", "SQLCOMMIT should clear the cancel-requested flag");
    }
    if (action_after_commit != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_commit->second) == "commit", "SQLCOMMIT should update the connection last-action metadata");
    }
    if (update != state.globals.end()) {
        expect(copperfin::runtime::format_value(update->second) == "1", "SQLEXEC update should succeed before SQLROLLBACK checks");
    }
    if (rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(rollback->second) == "1", "SQLROLLBACK should succeed for a valid session-local SQL handle");
    }
    if (dirty_after_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(dirty_after_rollback->second) == "false", "SQLROLLBACK should clear the transaction-dirty flag");
    }
    if (action_after_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_rollback->second) == "rollback", "SQLROLLBACK should update the connection last-action metadata");
    }
    if (cross_session_commit != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_session_commit->second) == "-1", "SQLCOMMIT should reject SQL handles from another data session");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after SQL helper checks");
    }

    expect(
        has_runtime_event(state.events, "sql.cancel", "handle 1") &&
        has_runtime_event(state.events, "sql.commit", "handle 1") &&
        has_runtime_event(state.events, "sql.rollback", "handle 1"),
        "SQL helper batch should emit sql.cancel/sql.commit/sql.rollback events");

    fs::remove_all(temp_root, ignored);
}

void test_sql_connection_property_breadth() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_property_breadth";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "sql_property_breadth.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('dsn=Northwind')\n"
        "lConnectedBefore = SQLGETPROP(nConn, 'Connected')\n"
        "nHandleProp = SQLGETPROP(nConn, 'ConnectHandle')\n"
        "cConnectString = SQLGETPROP(nConn, 'ConnectString')\n"
        "cCatalogBefore = SQLGETPROP(nConn, 'CurrentCatalog')\n"
        "nSetAsync = SQLSETPROP(nConn, 'Asynchronous', .T.)\n"
        "nSetBatch = SQLSETPROP(nConn, 'BatchMode', 2)\n"
        "nSetWarnings = SQLSETPROP(nConn, 'DispWarnings', .F.)\n"
        "nSetDispLogin = SQLSETPROP(nConn, 'DispLogin', .T.)\n"
        "nSetTransactions = SQLSETPROP(nConn, 'Transactions', .F.)\n"
        "nSetWaitTime = SQLSETPROP(nConn, 'WaitTime', 9)\n"
        "nSetPacketSize = SQLSETPROP(nConn, 'PacketSize', 8192)\n"
        "nSetConnectName = SQLSETPROP(nConn, 'ConnectName', 'Northwind Session')\n"
        "nSetCatalog = SQLSETPROP(nConn, 'CurrentCatalog', 'archive')\n"
        "nExec = SQLEXEC(nConn, 'select * from customers', 'sqlpropcur')\n"
        "cLastCursorAlias = SQLGETPROP(nConn, 'LastCursorAlias')\n"
        "nLastResultCount = SQLGETPROP(nConn, 'LastResultCount')\n"
        "lAsync = SQLGETPROP(nConn, 'Asynchronous')\n"
        "nBatch = SQLGETPROP(nConn, 'BatchMode')\n"
        "lWarnings = SQLGETPROP(nConn, 'DispWarnings')\n"
        "lDispLogin = SQLGETPROP(nConn, 'DispLogin')\n"
        "lTransactions = SQLGETPROP(nConn, 'Transactions')\n"
        "nWaitTime = SQLGETPROP(nConn, 'WaitTime')\n"
        "nPacketSize = SQLGETPROP(nConn, 'PacketSize')\n"
        "cConnectName = SQLGETPROP(nConn, 'ConnectName')\n"
        "cCatalogAfter = SQLGETPROP(nConn, 'CurrentCatalog')\n"
        "cActionAfterExec = SQLGETPROP(nConn, 'LastSqlAction')\n"
        "SET DATASESSION TO 2\n"
        "nCrossSessionConnected = SQLGETPROP(nConn, 'Connected')\n"
        "SET DATASESSION TO 1\n"
        "lDisc = SQLDISCONNECT(nConn)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL connection property breadth script should complete");

    const auto connected_before = state.globals.find("lconnectedbefore");
    const auto handle_prop = state.globals.find("nhandleprop");
    const auto connect_string = state.globals.find("cconnectstring");
    const auto catalog_before = state.globals.find("ccatalogbefore");
    const auto set_async = state.globals.find("nsetasync");
    const auto set_batch = state.globals.find("nsetbatch");
    const auto set_warnings = state.globals.find("nsetwarnings");
    const auto set_disp_login = state.globals.find("nsetdisplogin");
    const auto set_transactions = state.globals.find("nsettransactions");
    const auto set_wait_time = state.globals.find("nsetwaittime");
    const auto set_packet_size = state.globals.find("nsetpacketsize");
    const auto set_connect_name = state.globals.find("nsetconnectname");
    const auto set_catalog = state.globals.find("nsetcatalog");
    const auto exec = state.globals.find("nexec");
    const auto last_cursor_alias = state.globals.find("clastcursoralias");
    const auto last_result_count = state.globals.find("nlastresultcount");
    const auto async = state.globals.find("lasync");
    const auto batch = state.globals.find("nbatch");
    const auto warnings = state.globals.find("lwarnings");
    const auto disp_login = state.globals.find("ldisplogin");
    const auto transactions = state.globals.find("ltransactions");
    const auto wait_time = state.globals.find("nwaittime");
    const auto packet_size = state.globals.find("npacketsize");
    const auto connect_name = state.globals.find("cconnectname");
    const auto catalog_after = state.globals.find("ccatalogafter");
    const auto action_after_exec = state.globals.find("cactionafterexec");
    const auto cross_session_connected = state.globals.find("ncrosssessionconnected");
    const auto disc = state.globals.find("ldisc");

    expect(connected_before != state.globals.end(), "Connected property should be captured");
    expect(handle_prop != state.globals.end(), "ConnectHandle property should be captured");
    expect(connect_string != state.globals.end(), "ConnectString property should be captured");
    expect(catalog_before != state.globals.end(), "CurrentCatalog should be captured before updates");
    expect(set_async != state.globals.end(), "Asynchronous SQLSETPROP result should be captured");
    expect(set_batch != state.globals.end(), "BatchMode SQLSETPROP result should be captured");
    expect(set_warnings != state.globals.end(), "DispWarnings SQLSETPROP result should be captured");
    expect(set_disp_login != state.globals.end(), "DispLogin SQLSETPROP result should be captured");
    expect(set_transactions != state.globals.end(), "Transactions SQLSETPROP result should be captured");
    expect(set_wait_time != state.globals.end(), "WaitTime SQLSETPROP result should be captured");
    expect(set_packet_size != state.globals.end(), "PacketSize SQLSETPROP result should be captured");
    expect(set_connect_name != state.globals.end(), "ConnectName SQLSETPROP result should be captured");
    expect(set_catalog != state.globals.end(), "CurrentCatalog SQLSETPROP result should be captured");
    expect(exec != state.globals.end(), "SQLEXEC result should be captured for property breadth flow");
    expect(last_cursor_alias != state.globals.end(), "LastCursorAlias property should be captured");
    expect(last_result_count != state.globals.end(), "LastResultCount property should be captured");
    expect(async != state.globals.end(), "Asynchronous property should round-trip");
    expect(batch != state.globals.end(), "BatchMode property should round-trip");
    expect(warnings != state.globals.end(), "DispWarnings property should round-trip");
    expect(disp_login != state.globals.end(), "DispLogin property should round-trip");
    expect(transactions != state.globals.end(), "Transactions property should round-trip");
    expect(wait_time != state.globals.end(), "WaitTime property should round-trip");
    expect(packet_size != state.globals.end(), "PacketSize property should round-trip");
    expect(connect_name != state.globals.end(), "ConnectName property should round-trip");
    expect(catalog_after != state.globals.end(), "CurrentCatalog should be captured after updates");
    expect(action_after_exec != state.globals.end(), "LastSqlAction should be captured after SQLEXEC");
    expect(cross_session_connected != state.globals.end(), "cross-session SQLGETPROP result should be captured");
    expect(disc != state.globals.end(), "SQLDISCONNECT result should be captured after property breadth checks");

    if (connected_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(connected_before->second) == "true", "Connected should report true for a live session-local handle");
    }
    if (handle_prop != state.globals.end()) {
        expect(copperfin::runtime::format_value(handle_prop->second) == "1", "ConnectHandle should expose the SQL handle number");
    }
    if (connect_string != state.globals.end()) {
        expect(copperfin::runtime::format_value(connect_string->second) == "dsn=Northwind", "ConnectString should preserve the original target");
    }
    if (catalog_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(catalog_before->second) == "NORTHWIND", "CurrentCatalog should infer the synthetic primary catalog from the connect string");
    }
    if (set_async != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_async->second) == "1", "SQLSETPROP should accept Asynchronous updates");
    }
    if (set_batch != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_batch->second) == "1", "SQLSETPROP should accept BatchMode updates");
    }
    if (set_warnings != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_warnings->second) == "1", "SQLSETPROP should accept DispWarnings updates");
    }
    if (set_disp_login != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_disp_login->second) == "1", "SQLSETPROP should accept DispLogin updates");
    }
    if (set_transactions != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_transactions->second) == "1", "SQLSETPROP should accept Transactions updates");
    }
    if (set_wait_time != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_wait_time->second) == "1", "SQLSETPROP should accept WaitTime updates");
    }
    if (set_packet_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_packet_size->second) == "1", "SQLSETPROP should accept PacketSize updates");
    }
    if (set_connect_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_connect_name->second) == "1", "SQLSETPROP should accept ConnectName updates");
    }
    if (set_catalog != state.globals.end()) {
        expect(copperfin::runtime::format_value(set_catalog->second) == "1", "SQLSETPROP should accept CurrentCatalog updates");
    }
    if (exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(exec->second) == "1", "SQLEXEC should still succeed after property updates");
    }
    if (last_cursor_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_cursor_alias->second) == "sqlpropcur", "LastCursorAlias should track the most recently materialized SQL cursor");
    }
    if (last_result_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(last_result_count->second) == "3", "LastResultCount should expose the most recent SQLEXEC row count");
    }
    if (async != state.globals.end()) {
        expect(copperfin::runtime::format_value(async->second) == "true", "Asynchronous should round-trip as a boolean property");
    }
    if (batch != state.globals.end()) {
        expect(copperfin::runtime::format_value(batch->second) == "2", "BatchMode should round-trip as a numeric property");
    }
    if (warnings != state.globals.end()) {
        expect(copperfin::runtime::format_value(warnings->second) == "false", "DispWarnings should round-trip as a boolean property");
    }
    if (disp_login != state.globals.end()) {
        expect(copperfin::runtime::format_value(disp_login->second) == "true", "DispLogin should round-trip as a boolean property");
    }
    if (transactions != state.globals.end()) {
        expect(copperfin::runtime::format_value(transactions->second) == "false", "Transactions should round-trip as a boolean property");
    }
    if (wait_time != state.globals.end()) {
        expect(copperfin::runtime::format_value(wait_time->second) == "9", "WaitTime should round-trip as a numeric property");
    }
    if (packet_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(packet_size->second) == "8192", "PacketSize should round-trip as a numeric property");
    }
    if (connect_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(connect_name->second) == "Northwind Session", "ConnectName should round-trip as a string property");
    }
    if (catalog_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(catalog_after->second) == "ARCHIVE", "CurrentCatalog should normalize to uppercase on update");
    }
    if (action_after_exec != state.globals.end()) {
        expect(copperfin::runtime::format_value(action_after_exec->second) == "exec", "LastSqlAction should still reflect SQLEXEC after property updates");
    }
    if (cross_session_connected != state.globals.end()) {
        expect(copperfin::runtime::format_value(cross_session_connected->second) == "-1", "SQLGETPROP should reject SQL handles from another data session");
    }
    if (disc != state.globals.end()) {
        expect(copperfin::runtime::format_value(disc->second) == "1", "SQLDISCONNECT should succeed after property breadth checks");
    }

    expect(
        has_runtime_event(state.events, "sql.setprop", "handle 1: asynchronous=true") &&
        has_runtime_event(state.events, "sql.setprop", "handle 1: batchmode=2") &&
        has_runtime_event(state.events, "sql.setprop", "handle 1: displogin=true") &&
        has_runtime_event(state.events, "sql.setprop", "handle 1: transactions=false") &&
        has_runtime_event(state.events, "sql.setprop", "handle 1: packetsize=8192") &&
        has_runtime_event(state.events, "sql.setprop", "handle 1: currentcatalog=ARCHIVE"),
        "SQL property breadth flow should emit sql.setprop events for the new connection properties");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

}  // namespace

int main() {
    test_sqlprimarykeys_and_sqlforeignkeys_metadata_cursors();
    test_sqldatabases_metadata_cursor();
    test_sqltables_and_sqlcolumns_metadata_cursors();
    test_sql_connection_transaction_and_cancel_helpers();
    test_sql_connection_property_breadth();
    test_sql_result_cursor_read_only_parity();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
