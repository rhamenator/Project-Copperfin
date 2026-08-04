// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_table_structure_helpers.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::set_env_value;

std::filesystem::path memo_sidecar_path(std::filesystem::path table_path) {
    table_path.replace_extension(".fpt");
    return table_path;
}

void test_alter_table_drop_and_alter_column_rewrite() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_alter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"ALPHA", "10", "true"}, {"BRAVO", "20", "false"}});
    expect(create_result.ok, "test fixture DBF should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(table_path.string(), 1U, true);
    expect(delete_result.ok, "test fixture should be able to mark BRAVO deleted");

    const fs::path main_path = temp_root / "alter_structure.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "ALTER TABLE '" + table_path.string() + "' DROP COLUMN AGE\n"
        "ALTER TABLE '" + table_path.string() + "' ALTER COLUMN NAME C(12)\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8) NOT NULL DEFAULT 'NEW'\n"
        "INSERT INTO People (NAME, ACTIVE) VALUES ('CHARLIE', .T.)\n"
        "nCount = RECCOUNT()\n"
        "nFields = FCOUNT('People')\n"
        "cField1 = FIELD(1, 'People')\n"
        "cField3 = FIELD(3)\n"
        "cFieldMissing = FIELD(4, 'People')\n"
        "nSizeName = FSIZE('NAME', 'People')\n"
        "nSizeActive = FSIZE(2)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ALTER TABLE DROP/ALTER COLUMN script should complete");
    const auto count = state.globals.find("ncount");
    const auto field_count = state.globals.find("nfields");
    const auto field1 = state.globals.find("cfield1");
    const auto field3 = state.globals.find("cfield3");
    const auto field_missing = state.globals.find("cfieldmissing");
    const auto size_name = state.globals.find("nsizename");
    const auto size_active = state.globals.find("nsizeactive");
    expect(count != state.globals.end(), "ALTER TABLE script should expose record count");
    expect(field_count != state.globals.end(), "ALTER TABLE script should expose field count");
    expect(field1 != state.globals.end(), "FIELD(1, alias) should be captured for local cursor schema");
    expect(field3 != state.globals.end(), "FIELD(3) should be captured for current local cursor schema");
    expect(field_missing != state.globals.end(), "FIELD() beyond schema should be captured as empty");
    expect(size_name != state.globals.end(), "FSIZE(name, alias) should be captured for local cursor schema");
    expect(size_active != state.globals.end(), "FSIZE(index) should be captured for current local cursor schema");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "3", "INSERT after ALTER TABLE should append one row");
    }
    if (field_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_count->second) == "3", "ALTER TABLE should update open cursor field count");
    }
    if (field1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field1->second) == "NAME", "FIELD(1, alias) should return the first field name");
    }
    if (field3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field3->second) == "STATUS", "FIELD(3) should return the current cursor's third field name");
    }
    if (field_missing != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_missing->second).empty(), "FIELD() beyond the schema should return an empty string");
    }
    if (size_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(size_name->second) == "12", "FSIZE(name, alias) should reflect ALTER COLUMN width");
    }
    if (size_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(size_active->second) == "1", "FSIZE(index) should return the current cursor field width");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "ALTER TABLE rewritten DBF should remain readable");
    expect(parse_result.table.fields.size() == 3U, "DROP/ADD COLUMN should rewrite the field set");
    expect(parse_result.table.records.size() == 3U, "ALTER TABLE rewrite should preserve and append rows");
    if (parse_result.table.fields.size() == 3U) {
        expect(parse_result.table.fields[0].name == "NAME", "ALTER COLUMN should keep the target field name");
        expect(parse_result.table.fields[0].length == 12U, "ALTER COLUMN should rewrite field width");
        expect(parse_result.table.fields[1].name == "ACTIVE", "DROP COLUMN should preserve remaining field order");
        expect(parse_result.table.fields[2].name == "STATUS", "ADD COLUMN should append the new field");
    }
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "ALTER rewrite should preserve first row values");
        expect(parse_result.table.records[1].deleted, "ALTER rewrite should preserve deleted flags");
        expect(parse_result.table.records[2].values[0].display_value == "CHARLIE", "INSERT after ALTER should persist appended values");
        expect(parse_result.table.records[2].values[2].display_value == "NEW", "ALTER ADD COLUMN default should apply to later inserts");
    }
    expect(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.alter_table";
    }) == 3, "DROP COLUMN, ALTER COLUMN, and ADD COLUMN should emit runtime.alter_table events");

    fs::remove_all(temp_root, ignored);
}

void test_alter_table_add_column_backfills_existing_rows_with_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_add_default";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"ALPHA", "10"}, {"BRAVO", "20"}});
    expect(create_result.ok, "ALTER TABLE ADD COLUMN default fixture should be created");

    const fs::path main_path = temp_root / "alter_add_default.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8) NOT NULL DEFAULT 'NEW'\n"
        "GO 1\n"
        "cStatus1 = STATUS\n"
        "GO 2\n"
        "cStatus2 = STATUS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ALTER TABLE ADD COLUMN default backfill script should complete");

    const auto status1 = state.globals.find("cstatus1");
    const auto status2 = state.globals.find("cstatus2");
    expect(status1 != state.globals.end(), "ALTER TABLE ADD COLUMN should expose STATUS for row 1");
    expect(status2 != state.globals.end(), "ALTER TABLE ADD COLUMN should expose STATUS for row 2");
    if (status1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(status1->second) == "NEW",
            "ALTER TABLE ADD COLUMN DEFAULT should backfill existing row 1");
    }
    if (status2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(status2->second) == "NEW",
            "ALTER TABLE ADD COLUMN DEFAULT should backfill existing row 2");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "ALTER TABLE ADD COLUMN default-backfilled DBF should remain readable");
    expect(parse_result.table.fields.size() == 3U, "ALTER TABLE ADD COLUMN should append the new field");
    expect(parse_result.table.records.size() == 2U, "ALTER TABLE ADD COLUMN should preserve existing rows");
    if (parse_result.table.records.size() == 2U && parse_result.table.fields.size() == 3U) {
        expect(parse_result.table.records[0].values[2].display_value == "NEW",
            "ALTER TABLE ADD COLUMN DEFAULT should persist the backfill for row 1");
        expect(parse_result.table.records[1].values[2].display_value == "NEW",
            "ALTER TABLE ADD COLUMN DEFAULT should persist the backfill for row 2");
    }

    fs::remove_all(temp_root, ignored);
}

void test_create_table_defaults_and_not_null_constraints() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_defaults";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "defaults.dbf";
    const fs::path main_path = temp_root / "defaults.prg";
    write_text(
        main_path,
        "CREATE TABLE '" + table_path.string() + "' (NAME C(10) NOT NULL DEFAULT 'UNKNOWN', AGE N(3,0) DEFAULT 7, ACTIVE L DEFAULT .T.)\n"
        "INSERT INTO Defaults (NAME) VALUES ('ALPHA')\n"
        "INSERT INTO Defaults (AGE) VALUES (9)\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CREATE TABLE default constraint script should complete");
    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "default insert script should expose record count");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2", "default insert script should append two rows");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "default table should remain readable");
    expect(parse_result.table.records.size() == 2U, "default table should persist two rows");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "explicit NAME should be preserved");
        expect(parse_result.table.records[0].values[1].display_value == "7", "omitted AGE should use DEFAULT");
        expect(parse_result.table.records[0].values[2].display_value == "true", "omitted ACTIVE should use DEFAULT");
        expect(parse_result.table.records[1].values[0].display_value == "UNKNOWN", "NOT NULL NAME should use its DEFAULT when omitted");
        expect(parse_result.table.records[1].values[1].display_value == "9", "explicit AGE should be preserved");
    }

    fs::remove_all(temp_root, ignored);
}

void test_create_table_rejects_duplicate_field_names() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_duplicates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "duplicate_fields.dbf";
    const fs::path main_path = temp_root / "duplicate_fields.prg";
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    write_text(
        main_path,
        "CREATE TABLE '" + table_path.string() + "' (NAME C(10), NAME N(3,0))\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#3678: CREATE TABLE with duplicate field names should fail");
    expect(state.message == english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists"),
           "#3678: CREATE TABLE duplicate field names should surface the standard TargetFieldExists error");
    expect(!fs::exists(table_path), "#3678: failed CREATE TABLE should not leave a partial DBF on disk");

    fs::remove_all(temp_root, ignored);
}

void test_create_table_rejects_overlong_free_table_field_names() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_engine_table_structure_overlong_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "overlong_field.dbf";
    const fs::path main_path = temp_root / "overlong_field.prg";
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    write_text(
        main_path,
        "CREATE TABLE '" + table_path.string() + "' (ABCDEFGHIJK C(1))\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "#4034: CREATE TABLE should reject free-table field names beyond 10 bytes");
    expect(
        state.message == english_catalog.translate(
            "Vfp.DbfTable.Error.FreeTableFieldNameTooLong",
            {{"fieldName", "ABCDEFGHIJK"}, {"maxBytes", "10"}}),
        "#4034: CREATE TABLE should surface the localized field-name byte-limit error");
    expect(!fs::exists(table_path),
           "#4034: rejected CREATE TABLE field names should not leave a DBF on disk");

    fs::remove_all(temp_root, ignored);
}

void test_table_field_dimensions_require_complete_bounded_integers() {
    const std::vector<std::string> malformed_declarations = {
        "VALUE C(10x)",
        "VALUE N(3,0junk)",
        "VALUE C()",
        "VALUE N(3,)",
        "VALUE C(-1)",
        "VALUE C(256)",
        "VALUE N(3,256)",
        "VALUE C(9999999999999999999999999999999999999999)",
        "VALUE C(10)junk",
    };
    for (const std::string& declaration : malformed_declarations) {
        expect(
            !copperfin::runtime::parse_table_field_declaration(declaration).has_value(),
            "#3980: malformed or out-of-range field dimensions should reject the complete declaration: " +
                declaration);
    }

    const auto zero_width = copperfin::runtime::parse_table_field_declaration("VALUE C(0)");
    expect(zero_width.has_value(), "#3980: zero should remain a valid field-dimension token");
    if (zero_width.has_value()) {
        expect(zero_width->descriptor.length == 10U,
               "#3980: zero character width should retain the existing default-width behavior");
    }

    const auto maximum_width = copperfin::runtime::parse_table_field_declaration("VALUE C(255)");
    expect(maximum_width.has_value(), "#3980: the uint8 field-width boundary should remain valid");
    if (maximum_width.has_value()) {
        expect(maximum_width->descriptor.length == 255U,
               "#3980: the maximum supported field width should remain unchanged");
    }

    const auto numeric = copperfin::runtime::parse_table_field_declaration("VALUE N(20,19)");
    expect(numeric.has_value(), "#3980: ordinary complete numeric dimensions should remain valid");
    if (numeric.has_value()) {
        expect(numeric->descriptor.length == 20U && numeric->descriptor.decimal_count == 19U,
               "#3980: valid numeric width and precision should remain unchanged");
    }

    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_field_dimensions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(), "en-US");

    const fs::path rejected_table_path = temp_root / "rejected.dbf";
    const fs::path create_table_path = temp_root / "create_table_rejected.prg";
    write_text(
        create_table_path,
        "CREATE TABLE '" + rejected_table_path.string() + "' (GOOD C(5), BAD C(10x))\nRETURN\n");
    auto create_table_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(create_table_path.string(), temp_root.string()));
    const auto create_table_state =
        create_table_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!create_table_state.completed,
           "#3980: CREATE TABLE should reject an invalid declaration instead of keeping valid siblings");
    expect(
        create_table_state.message ==
            catalog.translate("Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration"),
        "#3980: CREATE TABLE should retain its catalog-routed field-declaration diagnostic");
    expect(!fs::exists(rejected_table_path),
           "#3980: rejected CREATE TABLE declarations should not create a partial DBF");
    expect(std::none_of(create_table_state.events.begin(), create_table_state.events.end(), [](const auto& event) {
        return event.category == "runtime.create_table";
    }), "#3980: rejected CREATE TABLE declarations should not emit a success event");

    const fs::path create_cursor_path = temp_root / "create_cursor_rejected.prg";
    write_text(
        create_cursor_path,
        "CREATE CURSOR BadCursor (BAD N(3,0junk), GOOD C(5))\nRETURN\n");
    auto create_cursor_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(create_cursor_path.string(), temp_root.string()));
    const auto create_cursor_state =
        create_cursor_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!create_cursor_state.completed,
           "#3980: CREATE CURSOR should reject an invalid declaration instead of keeping valid siblings");
    expect(
        create_cursor_state.message ==
            catalog.translate("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration"),
        "#3980: CREATE CURSOR should retain its catalog-routed field-declaration diagnostic");
    expect(std::none_of(create_cursor_state.cursors.begin(), create_cursor_state.cursors.end(), [](const auto& cursor) {
        return cursor.alias == "BadCursor" || cursor.alias == "BADCURSOR";
    }), "#3980: rejected CREATE CURSOR declarations should not materialize a partial cursor");
    expect(std::none_of(create_cursor_state.events.begin(), create_cursor_state.events.end(), [](const auto& event) {
        return event.category == "runtime.create_cursor";
    }), "#3980: rejected CREATE CURSOR declarations should not emit a success event");

    const fs::path existing_table_path = temp_root / "existing.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
    };
    const auto create_result =
        copperfin::vfp::create_dbf_table_file(existing_table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#3980: ALTER TABLE rejection fixture should be created");

    const fs::path alter_add_path = temp_root / "alter_add_rejected.prg";
    write_text(
        alter_add_path,
        "ALTER TABLE '" + existing_table_path.string() + "' ADD COLUMN EXTRA C(10x)\nRETURN\n");
    auto alter_add_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(alter_add_path.string(), temp_root.string()));
    const auto alter_add_state = alter_add_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!alter_add_state.completed, "#3980: ALTER TABLE ADD COLUMN should reject a malformed width");
    expect(
        alter_add_state.message == catalog.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            {{"command", "ALTER TABLE ADD COLUMN"}}),
        "#3980: ALTER TABLE ADD COLUMN should retain its catalog-routed diagnostic");

    const fs::path alter_column_path = temp_root / "alter_column_rejected.prg";
    write_text(
        alter_column_path,
        "ALTER TABLE '" + existing_table_path.string() + "' ALTER COLUMN NAME C(256)\nRETURN\n");
    auto alter_column_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(alter_column_path.string(), temp_root.string()));
    const auto alter_column_state =
        alter_column_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!alter_column_state.completed, "#3980: ALTER TABLE ALTER COLUMN should reject an oversized width");
    expect(
        alter_column_state.message == catalog.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            {{"command", "ALTER TABLE ALTER COLUMN"}}),
        "#3980: ALTER TABLE ALTER COLUMN should retain its catalog-routed diagnostic");

    const auto unchanged_table = copperfin::vfp::parse_dbf_table_from_file(existing_table_path.string(), 10U);
    expect(unchanged_table.ok, "#3980: rejected ALTER TABLE declarations should leave the DBF readable");
    expect(unchanged_table.table.fields.size() == 1U &&
               unchanged_table.table.fields[0].name == "NAME" &&
               unchanged_table.table.fields[0].length == 10U,
           "#3980: rejected ALTER TABLE declarations should leave the schema unchanged");
    expect(unchanged_table.table.records.size() == 1U &&
               unchanged_table.table.records[0].values[0].display_value == "ALPHA",
           "#3980: rejected ALTER TABLE declarations should leave record data unchanged");

    fs::remove_all(temp_root, ignored);
}

void test_create_cursor_uses_temp_backed_local_table_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_create_cursor";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "create_cursor.prg";
    write_text(
        main_path,
        "CREATE CURSOR WorkItems (NAME C(12) NOT NULL DEFAULT 'NEW', AGE N(3,0), ACTIVE L DEFAULT .T.)\n"
        "nFields = FCOUNT('WorkItems')\n"
        "nNameSize = FSIZE('NAME', 'WorkItems')\n"
        "nAFieldCount = AFIELDS(aFields, 'WorkItems')\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'ALPHA', AGE WITH 5\n"
        "INSERT INTO WorkItems (AGE) VALUES (7)\n"
        "nCount = RECCOUNT('WorkItems')\n"
        "cField1 = FIELD(1, 'WorkItems')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CREATE CURSOR local-table script should complete");

    const auto field_count = state.globals.find("nfields");
    const auto name_size = state.globals.find("nnamesize");
    const auto afield_count = state.globals.find("nafieldcount");
    const auto count = state.globals.find("ncount");
    const auto field1 = state.globals.find("cfield1");
    expect(field_count != state.globals.end(), "CREATE CURSOR should expose FCOUNT() for the temp-backed cursor");
    expect(name_size != state.globals.end(), "CREATE CURSOR should expose FSIZE() for the temp-backed cursor");
    expect(afield_count != state.globals.end(), "CREATE CURSOR should expose AFIELDS() for the temp-backed cursor");
    expect(count != state.globals.end(), "CREATE CURSOR should expose RECCOUNT() after local mutations");
    expect(field1 != state.globals.end(), "CREATE CURSOR should expose FIELD() metadata for the temp-backed cursor");
    if (field_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_count->second) == "3", "CREATE CURSOR should preserve the declared field count");
    }
    if (name_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_size->second) == "12", "CREATE CURSOR should preserve the declared field width");
    }
    if (afield_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(afield_count->second) == "3", "AFIELDS() should report the temp-backed cursor schema");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2", "APPEND BLANK and INSERT INTO should mutate the temp-backed cursor");
    }
    if (field1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field1->second) == "NAME", "FIELD() should resolve temp-backed cursor schema names");
    }

    const auto runtime_cursor = std::find_if(state.cursors.begin(), state.cursors.end(), [](const auto& cursor) {
        return uppercase_ascii(cursor.alias) == "WORKITEMS";
    });
    expect(runtime_cursor != state.cursors.end(), "CREATE CURSOR should leave the temp-backed cursor visible in runtime state");
    if (runtime_cursor != state.cursors.end()) {
        expect(runtime_cursor->source_kind == "table", "CREATE CURSOR should open as a normal local table cursor");
        expect(!runtime_cursor->source_path.empty(), "CREATE CURSOR should record a backing DBF path");
        expect(fs::exists(runtime_cursor->source_path), "CREATE CURSOR backing DBF should exist on disk");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(runtime_cursor->source_path, 10U);
        expect(parse_result.ok, "CREATE CURSOR backing DBF should remain readable");
        expect(parse_result.table.records.size() == 2U, "CREATE CURSOR backing DBF should persist appended records");
        if (parse_result.table.records.size() == 2U) {
            expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "REPLACE should persist to the temp-backed cursor DBF");
            expect(parse_result.table.records[0].values[1].display_value == "5", "REPLACE should persist numeric writes to the temp-backed cursor DBF");
            expect(parse_result.table.records[1].values[0].display_value == "NEW", "INSERT INTO should apply CREATE CURSOR defaults through field rules");
            expect(parse_result.table.records[1].values[1].display_value == "7", "INSERT INTO should persist explicit numeric values on the temp-backed cursor");
            expect(parse_result.table.records[1].values[2].display_value == "true", "INSERT INTO should apply logical defaults on the temp-backed cursor");
        }
    }

    expect(std::count_if(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.create_cursor";
    }) == 1, "CREATE CURSOR should emit one runtime.create_cursor event");

    fs::remove_all(temp_root, ignored);
}

void test_create_cursor_name_clause_uses_named_alias() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_create_cursor_name";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "create_cursor_name.prg";
    write_text(
        main_path,
        "CREATE CURSOR DefaultCursor NAME WorkItems (ITEM C(12))\n"
        "cAlias = ALIAS()\n"
        "INSERT INTO WorkItems (ITEM) VALUES ('ALPHA')\n"
        "nCount = RECCOUNT('WorkItems')\n"
        "SELECT WorkItems\n"
        "cSelectedAlias = ALIAS()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "CREATE CURSOR NAME script should complete");
    const auto alias = state.globals.find("calias");
    const auto count = state.globals.find("ncount");
    const auto selected_alias = state.globals.find("cselectedalias");
    expect(alias != state.globals.end(), "CREATE CURSOR NAME should expose the initial alias");
    expect(count != state.globals.end(), "CREATE CURSOR NAME alias should be usable by INSERT and RECCOUNT");
    expect(selected_alias != state.globals.end(), "CREATE CURSOR NAME alias should be selectable");
    if (alias != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(alias->second)) == "WORKITEMS",
               "CREATE CURSOR NAME should select the explicit alias");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1",
               "CREATE CURSOR NAME should make the explicit alias addressable");
    }
    if (selected_alias != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(selected_alias->second)) == "WORKITEMS",
               "SELECT should resolve the CREATE CURSOR NAME alias");
    }
    expect(std::none_of(state.cursors.begin(), state.cursors.end(), [](const auto& cursor) {
        return uppercase_ascii(cursor.alias) == "DEFAULTCURSOR NAME WORKITEMS";
    }), "CREATE CURSOR NAME must not register a combined multi-word alias");

    fs::remove_all(temp_root, ignored);
}

void test_create_cursor_not_null_insert_failure_rolls_back() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_create_cursor_not_null";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "create_cursor_not_null.prg";
    write_text(
        main_path,
        "CREATE CURSOR StrictCursor (NAME C(10) NOT NULL, AGE N(3,0))\n"
        "INSERT INTO StrictCursor (AGE) VALUES (4)\n"
        "nAfter = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "CREATE CURSOR NOT NULL violation should pause the runtime with an error");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "CREATE CURSOR NOT NULL violation should report an error pause");

    const auto runtime_cursor = std::find_if(state.cursors.begin(), state.cursors.end(), [](const auto& cursor) {
        return uppercase_ascii(cursor.alias) == "STRICTCURSOR";
    });
    expect(runtime_cursor != state.cursors.end(), "failing CREATE CURSOR insert should still expose the opened cursor");
    if (runtime_cursor != state.cursors.end()) {
        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(runtime_cursor->source_path, 10U);
        expect(parse_result.ok, "failed CREATE CURSOR insert should leave the temp-backed DBF readable");
        expect(parse_result.table.records.empty(), "failed CREATE CURSOR NOT NULL insert should roll back the appended row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_not_null_insert_failure_rolls_back() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_not_null";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "not_null.dbf";
    const fs::path main_path = temp_root / "not_null.prg";
    write_text(
        main_path,
        "CREATE TABLE '" + table_path.string() + "' (NAME C(10) NOT NULL, AGE N(3,0))\n"
        "INSERT INTO Not_Null (AGE) VALUES (4)\n"
        "nAfter = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "NOT NULL violation should pause the runtime with an error");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "NOT NULL violation should report an error pause");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "failed NOT NULL insert should leave DBF readable");
    expect(parse_result.table.records.empty(), "failed NOT NULL insert should roll back the appended row");

    fs::remove_all(temp_root, ignored);
}

void test_table_structure_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path create_cursor_path = temp_root / "create_cursor_fields_fail.prg";
    write_text(
        create_cursor_path,
        "CREATE CURSOR WorkItems ()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession create_cursor_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(create_cursor_path.string(), temp_root.string()));
    const auto create_cursor_state = create_cursor_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!create_cursor_state.completed, "#2713: qps-ploc CREATE CURSOR with no fields should fail");
    expect(
        create_cursor_state.message ==
            copperfin::localization::pseudo_localize("CREATE CURSOR requires at least one supported field declaration"),
        "#2713: qps-ploc CREATE CURSOR field-declaration error should route through the pseudo-localization transform");

    const fs::path create_table_path = temp_root / "create_table_fields_fail.prg";
    write_text(
        create_table_path,
        "CREATE TABLE '" + (temp_root / "bad.dbf").string() + "' ()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession create_table_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(create_table_path.string(), temp_root.string()));
    const auto create_table_state = create_table_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!create_table_state.completed, "#2713: qps-ploc CREATE TABLE with no fields should fail");
    expect(
        create_table_state.message ==
            copperfin::localization::pseudo_localize("CREATE TABLE requires at least one supported field declaration"),
        "#2713: qps-ploc CREATE TABLE field-declaration error should route through the pseudo-localization transform");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
    };
    const auto create_result =
        copperfin::vfp::create_dbf_table_file((temp_root / "people.dbf").string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2713: ALTER TABLE localization fixture should be created");

    const fs::path alter_table_path = temp_root / "alter_table_action_fail.prg";
    write_text(
        alter_table_path,
        "ALTER TABLE '" + (temp_root / "people.dbf").string() + "' RENAME COLUMN NAME TO TITLE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession alter_table_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(alter_table_path.string(), temp_root.string()));
    const auto alter_table_state = alter_table_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!alter_table_state.completed, "#2713: qps-ploc ALTER TABLE unsupported-action script should fail");
    expect(
        alter_table_state.message ==
            copperfin::localization::pseudo_localize("ALTER TABLE currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only"),
        "#2713: qps-ploc ALTER TABLE action-support error should route through the pseudo-localization transform");

    const fs::path alter_table_add_path = temp_root / "alter_table_add_field_fail.prg";
    write_text(
        alter_table_add_path,
        "ALTER TABLE '" + (temp_root / "people.dbf").string() + "' ADD COLUMN BAD\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession alter_table_add_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(alter_table_add_path.string(), temp_root.string()));
    const auto alter_table_add_state = alter_table_add_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!alter_table_add_state.completed, "#2723: qps-ploc ALTER TABLE ADD COLUMN with an invalid declaration should fail");
    expect(
        alter_table_add_state.message.find("[!! ") == 0U &&
            alter_table_add_state.message.find("ALTER TABLE ADD COLUMN") != std::string::npos &&
            alter_table_add_state.message.find("requires a supported field declaration") == std::string::npos,
        "#2723: qps-ploc ALTER TABLE ADD COLUMN field-declaration error should pseudo-localize prose while preserving command tokens");

    const fs::path alter_table_alter_path = temp_root / "alter_table_alter_field_fail.prg";
    write_text(
        alter_table_alter_path,
        "ALTER TABLE '" + (temp_root / "people.dbf").string() + "' ALTER COLUMN BAD\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession alter_table_alter_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(alter_table_alter_path.string(), temp_root.string()));
    const auto alter_table_alter_state = alter_table_alter_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!alter_table_alter_state.completed, "#2723: qps-ploc ALTER TABLE ALTER COLUMN with an invalid declaration should fail");
    expect(
        alter_table_alter_state.message.find("[!! ") == 0U &&
            alter_table_alter_state.message.find("ALTER TABLE ALTER COLUMN") != std::string::npos &&
            alter_table_alter_state.message.find("requires a supported field declaration") == std::string::npos,
        "#2723: qps-ploc ALTER TABLE ALTER COLUMN field-declaration error should pseudo-localize prose while preserving command tokens");

    fs::remove_all(temp_root, ignored);
}

void test_pack_memo_rewrites_memo_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_pack_memo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "memo.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTES", .type = 'M', .length = 4U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"ALPHA", std::string(900U, 'A')}});
    expect(create_result.ok, "memo fixture should be created");
    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NOTES", "short");
    expect(replace_result.ok, "memo fixture should support replacing a large memo with a small memo");
    const fs::path memo_path = memo_sidecar_path(table_path);
    const auto before_size = fs::file_size(memo_path, ignored);
    expect(before_size > 0U, "memo sidecar should exist before PACK MEMO");

    const fs::path main_path = temp_root / "pack_memo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Memo IN 0\n"
        "PACK MEMO\n"
        "nCount = RECCOUNT()\n"
        "cNotes = NOTES\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PACK MEMO script should complete");
    const auto count = state.globals.find("ncount");
    const auto notes = state.globals.find("cnotes");
    expect(count != state.globals.end(), "PACK MEMO should preserve RECCOUNT()");
    expect(notes != state.globals.end(), "PACK MEMO should keep memo field readable");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1", "PACK MEMO should preserve row count");
    }
    if (notes != state.globals.end()) {
        expect(copperfin::runtime::format_value(notes->second) == "short", "PACK MEMO should preserve current memo value");
    }
    const auto after_size = fs::file_size(memo_path, ignored);
    expect(after_size > 0U, "memo sidecar should exist after PACK MEMO");
    expect(after_size < before_size, "PACK MEMO should compact stale memo blocks");

    fs::remove_all(temp_root, ignored);
}

void test_alter_table_rollback_restores_schema_and_disk_readability() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_alter_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"ALPHA", "10"}, {"BRAVO", "20"}});
    expect(create_result.ok, "ALTER TABLE rollback fixture should be created");

    const fs::path main_path = temp_root / "alter_rollback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO BOTTOM\n"
        "nRecBeforeRollback = RECNO('People')\n"
        "BEGIN TRANSACTION\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8) NOT NULL DEFAULT 'NEW'\n"
        "ROLLBACK\n"
        "nRecAfterRollback = RECNO('People')\n"
        "nFields = FCOUNT('People')\n"
        "cField2 = FIELD(2, 'People')\n"
        "GO TOP\n"
        "cNameAfterRollback = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ALTER TABLE rollback script should complete");

    const auto field_count = state.globals.find("nfields");
    const auto field2 = state.globals.find("cfield2");
    const auto name_after_rollback = state.globals.find("cnameafterrollback");
    const auto rec_before_rollback = state.globals.find("nrecbeforerollback");
    const auto rec_after_rollback = state.globals.find("nrecafterrollback");
    expect(field_count != state.globals.end(), "ALTER TABLE rollback should expose field count");
    expect(field2 != state.globals.end(), "ALTER TABLE rollback should expose second field name");
    expect(name_after_rollback != state.globals.end(), "ALTER TABLE rollback should preserve row readability");
    expect(rec_before_rollback != state.globals.end(), "ALTER TABLE rollback should expose pre-rollback RECNO()");
    expect(rec_after_rollback != state.globals.end(), "ALTER TABLE rollback should expose post-rollback RECNO()");
    if (field_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_count->second) == "2",
               "ROLLBACK should restore the pre-ALTER field count for the open cursor");
    }
    if (field2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(field2->second) == "AGE",
               "ROLLBACK should restore the original field ordering for the open cursor");
    }
    if (name_after_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(name_after_rollback->second) == "ALPHA",
               "ROLLBACK should keep the restored table readable through the open cursor");
    }
    if (rec_before_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before_rollback->second) == "2",
               "ALTER TABLE rollback fixture should start with cursor at bottom record");
    }
    if (rec_after_rollback != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after_rollback->second) == "2",
               "ROLLBACK should preserve open-cursor record position after ALTER TABLE replay");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "ALTER TABLE rollback should leave DBF readable on disk");
    expect(parse_result.table.fields.size() == 2U, "ALTER TABLE rollback should restore the original schema on disk");
    expect(parse_result.table.records.size() == 2U, "ALTER TABLE rollback should preserve original rows on disk");

    fs::remove_all(temp_root, ignored);
}

void test_pack_memo_rollback_restores_original_sidecar_and_readability() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_structure_pack_memo_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "memo.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields = {
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTES", .type = 'M', .length = 4U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        fields,
        {{"ALPHA", std::string(900U, 'A')}});
    expect(create_result.ok, "PACK MEMO rollback fixture should be created");
    const auto replace_result = copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NOTES", "short");
    expect(replace_result.ok, "PACK MEMO rollback fixture should create stale memo blocks");

    const fs::path memo_path = memo_sidecar_path(table_path);
    const auto before_size = fs::file_size(memo_path, ignored);
    expect(before_size > 0U, "memo sidecar should exist before PACK MEMO rollback");

    const fs::path main_path = temp_root / "pack_memo_rollback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Memo IN 0\n"
        "GO 1\n"
        "nRecBefore = RECNO()\n"
        "BEGIN TRANSACTION\n"
        "PACK MEMO\n"
        "ROLLBACK\n"
        "nRecAfter = RECNO()\n"
        "nCount = RECCOUNT()\n"
        "cName = NAME\n"
        "cNotes = NOTES\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PACK MEMO rollback script should complete");

    const auto count = state.globals.find("ncount");
    const auto rec_before = state.globals.find("nrecbefore");
    const auto rec_after = state.globals.find("nrecafter");
    const auto name = state.globals.find("cname");
    const auto notes = state.globals.find("cnotes");
    expect(rec_before != state.globals.end(), "PACK MEMO rollback should expose RECNO() before PACK MEMO");
    expect(rec_after != state.globals.end(), "PACK MEMO rollback should expose RECNO() after ROLLBACK");
    expect(count != state.globals.end(), "PACK MEMO rollback should expose RECCOUNT()");
    expect(name != state.globals.end(), "PACK MEMO rollback should keep current-row NAME readable");
    expect(notes != state.globals.end(), "PACK MEMO rollback should keep memo field readable");
    if (rec_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_before->second) == "1",
               "PACK MEMO rollback fixture should start from row 1");
    }
    if (rec_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(rec_after->second) == "1",
               "ROLLBACK after PACK MEMO should preserve open-cursor record position");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1",
               "PACK MEMO rollback should preserve row count");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ALPHA",
               "PACK MEMO rollback should preserve the current row payload");
    }
    if (notes != state.globals.end()) {
        expect(copperfin::runtime::format_value(notes->second) == "short",
               "PACK MEMO rollback should preserve the current memo value");
    }

    const auto after_size = fs::file_size(memo_path, ignored);
    expect(after_size == before_size,
           "ROLLBACK after PACK MEMO should restore the original memo sidecar bytes");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "PACK MEMO rollback should leave DBF readable on disk");
    expect(parse_result.table.records.size() == 1U, "PACK MEMO rollback should preserve original rows on disk");
    if (parse_result.table.records.size() == 1U) {
        expect(parse_result.table.records[0].values[1].display_value == "short",
               "PACK MEMO rollback should preserve memo payload on disk");
    }

    fs::remove_all(temp_root, ignored);
}

void test_create_table_from_array_uses_vfp_metadata_columns() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_prg_engine_create_table_from_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "from_array.dbf";
    const fs::path minimal_table_path = temp_root / "from_array_minimal.dbf";
    const fs::path main_path = temp_root / "create_from_array.prg";
    write_text(
        main_path,
        "DIM aFields[2, 16]\n"
        "aFields[1, 1] = 'NAME'\n"
        "aFields[1, 2] = 'C'\n"
        "aFields[1, 3] = 12\n"
        "aFields[1, 4] = 0\n"
        "aFields[2, 1] = 'AMOUNT'\n"
        "aFields[2, 2] = 'N'\n"
        "aFields[2, 3] = 10\n"
        "aFields[2, 4] = 2\n"
        "CREATE TABLE '" + table_path.string() + "' FROM ARRAY aFields\n"
        "nFieldCount = FCOUNT()\n"
        "cField1 = FIELD(1)\n"
        "cField2 = FIELD(2)\n"
        "nAmountSize = FSIZE('AMOUNT')\n"
        "DIM aMinimal[1, 4]\n"
        "aMinimal[1, 1] = 'CODE'\n"
        "aMinimal[1, 2] = 'C'\n"
        "aMinimal[1, 3] = 6\n"
        "aMinimal[1, 4] = 0\n"
        "CREATE TABLE '" + minimal_table_path.string() + "' FROM ARRAY aMinimal\n"
        "nMinimalFieldCount = FCOUNT()\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CREATE TABLE FROM ARRAY should complete: " + state.message);
    expect(fs::exists(table_path), "CREATE TABLE FROM ARRAY should create the DBF");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured by CREATE TABLE FROM ARRAY");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " should equal '" + expected + "' for CREATE TABLE FROM ARRAY");
        }
    };
    check("nfieldcount", "2");
    check("cfield1", "NAME");
    check("cfield2", "AMOUNT");
    check("namountsize", "10");
    check("nminimalfieldcount", "1");

    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parsed.ok, "CREATE TABLE FROM ARRAY output should be readable");
    expect(parsed.ok && parsed.table.fields.size() == 2U,
           "CREATE TABLE FROM ARRAY should emit one DBF field per metadata row");
    if (parsed.ok && parsed.table.fields.size() == 2U) {
        expect(parsed.table.fields[0].name == "NAME" && parsed.table.fields[0].type == 'C' &&
                   parsed.table.fields[0].length == 12U,
               "CREATE TABLE FROM ARRAY should preserve character metadata");
        expect(parsed.table.fields[1].name == "AMOUNT" && parsed.table.fields[1].type == 'N' &&
                   parsed.table.fields[1].length == 10U && parsed.table.fields[1].decimal_count == 2U,
               "CREATE TABLE FROM ARRAY should preserve numeric metadata");
    }
    const auto minimal_parsed = copperfin::vfp::parse_dbf_table_from_file(
        minimal_table_path.string(), 10U);
    expect(minimal_parsed.ok && minimal_parsed.table.fields.size() == 1U,
           "CREATE TABLE FROM ARRAY should accept the minimal four-column metadata form");
    if (minimal_parsed.ok && minimal_parsed.table.fields.size() == 1U) {
        expect(minimal_parsed.table.fields[0].name == "CODE" &&
                   minimal_parsed.table.fields[0].type == 'C' &&
                   minimal_parsed.table.fields[0].length == 6U,
               "CREATE TABLE FROM ARRAY should preserve minimal-form field metadata");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_alter_table_drop_and_alter_column_rewrite();
    test_alter_table_add_column_backfills_existing_rows_with_default();
    test_alter_table_rollback_restores_schema_and_disk_readability();
    test_create_table_defaults_and_not_null_constraints();
    test_create_table_rejects_duplicate_field_names();
    test_create_table_rejects_overlong_free_table_field_names();
    test_table_field_dimensions_require_complete_bounded_integers();
    test_create_cursor_uses_temp_backed_local_table_flow();
    test_create_cursor_name_clause_uses_named_alias();
    test_create_cursor_not_null_insert_failure_rolls_back();
    test_not_null_insert_failure_rolls_back();
    test_table_structure_runtime_errors_localize();
    test_pack_memo_rewrites_memo_sidecar();
    test_pack_memo_rollback_restores_original_sidecar_and_readability();
    test_create_table_from_array_uses_vfp_metadata_columns();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
