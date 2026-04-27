#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "NOT NULL violation should pause the runtime with an error");
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "NOT NULL violation should report an error pause");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "failed NOT NULL insert should leave DBF readable");
    expect(parse_result.table.records.empty(), "failed NOT NULL insert should roll back the appended row");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

}  // namespace

int main() {
    test_alter_table_drop_and_alter_column_rewrite();
    test_create_table_defaults_and_not_null_constraints();
    test_create_cursor_uses_temp_backed_local_table_flow();
    test_create_cursor_not_null_insert_failure_rolls_back();
    test_not_null_insert_failure_rolls_back();
    test_pack_memo_rewrites_memo_sidecar();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return 1;
    }
    std::cout << "All tests passed.\n";
    return 0;
}
