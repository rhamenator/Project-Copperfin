// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <system_error>
#include <vector>

namespace copperfin::table_mutation_tests
{

using namespace copperfin::test_support;

void test_insert_into_and_delete_from_local_table() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_insert_delete_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(other_path, {{"OTHER", 1}});

    const fs::path main_path = temp_root / "insert_delete_from.prg";
    write_text(
        main_path,
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "SELECT Other\n"
        "cTarget = 'People'\n"
        "INSERT INTO cTarget (AGE, NAME) VALUES (44, 'DELTA')\n"
        "INSERT INTO cTarget VALUES ('ECHO', 55)\n"
        "DELETE FROM cTarget WHERE AGE = 20\n"
        "cSelectedAlias = ALIAS()\n"
        "nPeopleCount = RECCOUNT('People')\n"
        "nOtherRec = RECNO('Other')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INSERT INTO / DELETE FROM script should complete");

    const auto selected_alias = state.globals.find("cselectedalias");
    const auto people_count = state.globals.find("npeoplecount");
    const auto other_rec = state.globals.find("notherrec");
    expect(selected_alias != state.globals.end(), "INSERT/DELETE FROM should preserve selected alias");
    expect(people_count != state.globals.end(), "INSERT INTO should expose updated target RECCOUNT()");
    expect(other_rec != state.globals.end(), "INSERT/DELETE FROM should preserve selected cursor position");
    if (selected_alias != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(selected_alias->second)) == "OTHER", "INSERT/DELETE FROM should not select the target cursor");
    }
    if (people_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(people_count->second) == "5", "INSERT INTO should append two local records");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "1", "INSERT/DELETE FROM should leave the selected cursor pointer alone");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.insert_into";
    }), "INSERT INTO should emit a runtime.insert_into event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.delete_from";
    }), "DELETE FROM should emit a runtime.delete_from event");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(people_path.string(), 10U);
    expect(parse_result.ok, "INSERT INTO / DELETE FROM should leave the DBF readable");
    expect(parse_result.table.records.size() == 5U, "INSERT INTO should persist appended records");
    if (parse_result.table.records.size() == 5U) {
        expect(parse_result.table.records[1].deleted, "DELETE FROM WHERE should tombstone the matching persisted record");
        expect(parse_result.table.records[3].values[0].display_value == "DELTA", "INSERT INTO field list should map NAME by field name");
        expect(parse_result.table.records[3].values[1].display_value == "44", "INSERT INTO field list should map AGE by field name");
        expect(parse_result.table.records[4].values[0].display_value == "ECHO", "INSERT INTO without field list should map schema field order");
        expect(parse_result.table.records[4].values[1].display_value == "55", "INSERT INTO without field list should persist numeric schema order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_insert_into_select_materializes_filtered_ordered_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_insert_select";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const fs::path target_path = temp_root / "target.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(source_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    write_people_dbf(target_path, {{"TARGET", 1}});
    write_people_dbf(other_path, {{"OTHER", 2}});

    const fs::path main_path = temp_root / "insert_select.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "GO 2 IN Source\n"
        "USE '" + target_path.string() + "' ALIAS Target IN 0\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "SELECT Other\n"
        "INSERT INTO Target (NAME, AGE) SELECT NAME, AGE FROM Source WHERE AGE >= 20 ORDER BY AGE DESC\n"
        "INSERT INTO Target SELECT NAME, AGE FROM Source WHERE AGE = 10\n"
        "nSourceRec = RECNO('Source')\n"
        "INSERT INTO Source (NAME, AGE) SELECT NAME, AGE FROM Source WHERE AGE = 10\n"
        "nSourceCount = RECCOUNT('Source')\n"
        "cSelectedAlias = ALIAS()\n"
        "nOtherRec = RECNO('Other')\n"
        "nTargetCount = RECCOUNT('Target')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);

    expect(state.completed, "INSERT INTO SELECT script should complete: " + state.message);
    const auto selected_alias = state.globals.find("cselectedalias");
    const auto source_rec = state.globals.find("nsourcerec");
    const auto source_count = state.globals.find("nsourcecount");
    const auto other_rec = state.globals.find("notherrec");
    const auto target_count = state.globals.find("ntargetcount");
    expect(selected_alias != state.globals.end(), "INSERT INTO SELECT should preserve the selected alias");
    expect(source_rec != state.globals.end(), "INSERT INTO SELECT should expose the restored source record pointer");
    expect(source_count != state.globals.end(), "self INSERT INTO SELECT should expose the finite source record count");
    expect(other_rec != state.globals.end(), "INSERT INTO SELECT should expose the selected cursor pointer");
    expect(target_count != state.globals.end(), "INSERT INTO SELECT should expose the target record count");
    if (selected_alias != state.globals.end()) {
        expect(uppercase_ascii(copperfin::runtime::format_value(selected_alias->second)) == "OTHER",
               "INSERT INTO SELECT should not change the selected work area");
    }
    if (source_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(source_rec->second) == "2",
               "INSERT INTO SELECT should restore the source record pointer after materialization");
    }
    if (source_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(source_count->second) == "4",
               "self INSERT INTO SELECT should materialize before appending instead of chasing new rows");
    }
    if (other_rec != state.globals.end()) {
        expect(copperfin::runtime::format_value(other_rec->second) == "1",
               "INSERT INTO SELECT should preserve the selected cursor pointer");
    }
    if (target_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_count->second) == "4",
               "INSERT INTO SELECT should append rows with explicit and schema-order target mapping");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(target_path.string(), 5U);
    expect(parse_result.ok, "INSERT INTO SELECT should leave the target DBF readable");
    expect(parse_result.table.records.size() == 4U,
           "INSERT INTO SELECT should persist the original and three selected rows");
    if (parse_result.table.records.size() == 4U) {
        expect(parse_result.table.records[1].values[0].display_value == "CHARLIE",
               "INSERT INTO SELECT should honor descending query order for the first appended row");
        expect(parse_result.table.records[1].values[1].display_value == "30",
               "INSERT INTO SELECT should preserve the first appended numeric value");
        expect(parse_result.table.records[2].values[0].display_value == "BRAVO",
               "INSERT INTO SELECT should append the remaining filtered row");
        expect(parse_result.table.records[2].values[1].display_value == "20",
               "INSERT INTO SELECT should preserve the remaining numeric value");
        expect(parse_result.table.records[3].values[0].display_value == "ALPHA",
               "INSERT INTO SELECT without a target field list should use target schema order");
        expect(parse_result.table.records[3].values[1].display_value == "10",
               "INSERT INTO SELECT schema-order mapping should preserve the numeric value");
    }
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.insert_into";
    }), "INSERT INTO SELECT should emit runtime.insert_into after the batch succeeds");

    fs::remove_all(temp_root, ignored);
}

void test_insert_into_select_rolls_back_the_whole_failed_batch() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_insert_select_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const fs::path target_path = temp_root / "target.dbf";
    write_people_dbf(source_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_people_dbf(target_path, {{"TARGET", 1}});
    const auto original_size = fs::file_size(target_path);

    const fs::path main_path = temp_root / "insert_select_rollback.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "USE '" + target_path.string() + "' ALIAS Target IN 0\n"
        "INSERT INTO Target (NAME, AGE) SELECT IIF(AGE = 20, 'THIS-NAME-IS-TOO-LONG', NAME), AGE FROM Source ORDER BY AGE\n"
        "nAfterError = RECCOUNT('Target')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "failed INSERT INTO SELECT should pause with an error");
    expect(state.location.line == 3U,
           "failed INSERT INTO SELECT should highlight the batch statement");
    expect(state.message.find("too large") != std::string::npos,
           "failed INSERT INTO SELECT should preserve the target field-write error");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after failed INSERT INTO SELECT should keep the session alive");
    const auto after_error = state.globals.find("naftererror");
    expect(after_error != state.globals.end(),
           "failed INSERT INTO SELECT should expose the restored target record count");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "1",
               "failed INSERT INTO SELECT should restore the in-memory target record count");
    }

    expect(fs::file_size(target_path) == original_size,
           "failed INSERT INTO SELECT should restore the original DBF size");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(target_path.string(), 5U);
    expect(parse_result.ok, "failed INSERT INTO SELECT rollback should leave the target DBF readable");
    expect(parse_result.table.records.size() == 1U,
           "failed INSERT INTO SELECT should roll back rows appended earlier in the batch");
    if (parse_result.table.records.size() == 1U) {
        expect(parse_result.table.records[0].values[0].display_value == "TARGET",
               "failed INSERT INTO SELECT should preserve the original target row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_insert_into_rolls_back_failed_local_append() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_insert_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    const auto original_size = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "insert_rollback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "INSERT INTO People (NAME, AGE) VALUES ('THIS-NAME-IS-TOO-LONG', 77)\n"
        "nAfterError = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "failed INSERT INTO should pause with an error");
    expect(state.location.line == 2U, "failed INSERT INTO should highlight the INSERT statement");
    expect(
        state.message.find("too large") != std::string::npos,
        "failed INSERT INTO should preserve the field-write error message");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after failed INSERT INTO should keep the session alive");
    const auto after_error = state.globals.find("naftererror");
    expect(after_error != state.globals.end(), "post-error RECCOUNT() should be captured");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "2", "failed INSERT INTO should roll back the appended row");
    }

    expect(std::filesystem::file_size(table_path) == original_size, "failed INSERT INTO should restore the original DBF size");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "failed INSERT INTO rollback should leave the DBF readable");
    expect(parse_result.table.records.size() == 2U, "failed INSERT INTO rollback should preserve the original record count");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "failed INSERT INTO rollback should preserve row 1");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO", "failed INSERT INTO rollback should preserve row 2");
    }

    fs::remove_all(temp_root, ignored);
}

void test_indexed_table_mutation_succeeds_for_structural_indexes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_indexed_mutation_guard";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const fs::path cdx_path = temp_root / "people.cdx";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_synthetic_cdx(cdx_path, "NAME", "UPPER(NAME)");

    const auto original_table_bytes = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "indexed_mutation_guard.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'CHARLIE', AGE WITH 30\n"
        "xAfterMutation = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "indexed-table mutation should complete without runtime faults");

    const auto after_mutation = state.globals.find("xaftermutation");
    expect(after_mutation != state.globals.end(), "indexed-table mutation should continue through follow-on statements");
    if (after_mutation != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_mutation->second) == "3", "indexed-table APPEND BLANK should increase RECCOUNT");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_blank";
    }), "successful indexed-table APPEND BLANK should emit a runtime append event");
    expect(std::filesystem::file_size(table_path) > original_table_bytes, "successful indexed-table APPEND BLANK should increase DBF size");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "indexed-table mutation should keep the DBF readable");
    expect(parse_result.table.records.size() == 3U, "indexed-table mutation should persist appended rows");
    if (parse_result.table.records.size() == 3U) {
        expect(parse_result.table.records[2].values[0].display_value == "CHARLIE", "indexed-table mutation should persist REPLACE on appended row");
        expect(parse_result.table.records[2].values[1].display_value == "30", "indexed-table mutation should persist numeric REPLACE values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_blank_supports_opaque_field_layouts_at_runtime() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_blank_unsupported_layout";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "unsupported.dbf";
    std::vector<std::uint8_t> table_bytes(65U + 9U + 1U, 0U);
    table_bytes[0] = 0x30U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 65U);
    write_le_u16(table_bytes, 10U, 9U);
    const char value_field[] = "VALUE";
    for (std::size_t index = 0; index < 5U; ++index) {
        table_bytes[32U + index] = static_cast<std::uint8_t>(value_field[index]);
    }
    table_bytes[32U + 11U] = static_cast<std::uint8_t>('W');
    write_le_u32(table_bytes, 32U + 12U, 1U);
    table_bytes[32U + 16U] = 8U;
    table_bytes[64U] = 0x0DU;
    table_bytes[65U] = 0x20U;
    for (std::size_t index = 0; index < 8U; ++index) {
        table_bytes[66U + index] = static_cast<std::uint8_t>('0' + index);
    }
    table_bytes.back() = 0x1AU;
    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    const fs::path main_path = temp_root / "append_blank_opaque.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Weird IN 0\n"
        "REPLACE VALUE WITH '0x4142434445464748'\n"
        "GO 1\n"
        "cUpdatedOpaque = VALUE\n"
        "APPEND BLANK\n"
        "GO 2\n"
        "cOpaqueValue = VALUE\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "opaque-layout APPEND BLANK script should complete: " + state.message);
    const auto updated_opaque = state.globals.find("cupdatedopaque");
    const auto opaque_value = state.globals.find("copaquevalue");
    const auto count = state.globals.find("ncount");
    expect(updated_opaque != state.globals.end(), "opaque-layout REPLACE should expose the updated opaque field");
    expect(opaque_value != state.globals.end(), "opaque-layout APPEND BLANK should expose the appended opaque field");
    expect(count != state.globals.end(), "opaque-layout APPEND BLANK should expose the grown record count");
    if (updated_opaque != state.globals.end()) {
        expect(copperfin::runtime::format_value(updated_opaque->second) == "0x4142434445464748",
            "runtime REPLACE should support opaque field hex payloads");
    }
    if (opaque_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(opaque_value->second) == "0x0000000000000000",
            "runtime APPEND BLANK should initialize opaque fields to zero bytes");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "runtime APPEND BLANK should grow the table record count");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "opaque-layout APPEND BLANK should leave the DBF readable");
    expect(parse_result.table.records.size() == 2U, "opaque-layout APPEND BLANK should append a row");
    if (parse_result.ok && parse_result.table.records.size() == 2U && !parse_result.table.records[0U].values.empty()) {
        expect(parse_result.table.records[0U].values[0U].display_value == "0x4142434445464748",
            "opaque-layout REPLACE should persist zero-free opaque bytes");
    }
    if (parse_result.ok && parse_result.table.records.size() == 2U && !parse_result.table.records[1U].values.empty()) {
        expect(parse_result.table.records[1U].values[0U].display_value == "0x0000000000000000",
            "opaque-layout APPEND BLANK should persist zero-filled opaque bytes");
    }

    fs::remove_all(temp_root, ignored);
}

void test_update_command_sets_scoped_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_update_command";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "40"},
        {"Bob", "50"},
        {"Alice", "60"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "UPDATE command DBF fixture should be created");

    const fs::path main_path = temp_root / "update_command.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "UPDATE People SET AGE = AGE + 1 WHERE NAME = 'Alice'\n"
        "UPDATE SET AGE = AGE + 10 WHERE NAME = 'Bob'\n"
        "UPDATE IN People SET AGE = AGE + 1 WHERE NAME = 'Alice'\n"
        "GO 1\n"
        "nFirstAge = AGE\n"
        "GO 2\n"
        "nSecondAge = AGE\n"
        "GO 3\n"
        "nThirdAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UPDATE command script should complete");
    expect(has_runtime_event(state.events, "runtime.update", "UPDATE People SET AGE = AGE + 1 WHERE NAME = 'Alice'"),
        "UPDATE should emit a runtime.update event");

    const auto first_age = state.globals.find("nfirstage");
    const auto second_age = state.globals.find("nsecondage");
    const auto third_age = state.globals.find("nthirdage");
    expect(first_age != state.globals.end(), "UPDATE script should capture first AGE");
    expect(second_age != state.globals.end(), "UPDATE script should capture second AGE");
    expect(third_age != state.globals.end(), "UPDATE script should capture third AGE");
    if (first_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_age->second) == "42",
            "UPDATE alias and UPDATE IN should both update first matching record");
    }
    if (second_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_age->second) == "60",
            "UPDATE SET without explicit alias should target the selected cursor");
    }
    if (third_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(third_age->second) == "62",
            "UPDATE alias and UPDATE IN should both update later matching records");
    }

    fs::remove_all(temp_root, ignored);
}

void test_update_and_delete_accept_in_subquery_predicates() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_mutation_subquery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path target_path = temp_root / "people.dbf";
    const fs::path eligible_path = temp_root / "eligible.dbf";
    write_people_dbf(target_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});
    write_people_dbf(eligible_path, {{"THIRTY", 30}, {"FORTY", 40}});

    const fs::path main_path = temp_root / "mutation_subquery.prg";
    write_text(
        main_path,
        "USE '" + target_path.string() + "' ALIAS People IN 0\n"
        "USE '" + eligible_path.string() + "' ALIAS Eligible IN 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.RowSourceType = 3\n"
        "oList.RowSource = \"SELECT AGE FROM Eligible WHERE AGE >= 30\"\n"
        "oList.Requery()\n"
        "nEligible = oList.ListCount\n"
        "cEligibleFirst = oList.List(1, 1)\n"
        "UPDATE People SET NAME = 'MATCH' WHERE AGE IN (SELECT AGE FROM Eligible WHERE AGE >= 30)\n"
        "DELETE FROM People WHERE AGE IN (SELECT AGE FROM Eligible WHERE AGE = 40)\n"
        "SELECT People\n"
        "GO 3\n"
        "cThirdName = NAME\n"
        "GO 4\n"
        "cFourthName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, std::string("mutation subquery script should complete: ") + state.message);

    const auto third_name = state.globals.find("cthirdname");
    const auto fourth_name = state.globals.find("cfourthname");
    const auto eligible_count = state.globals.find("neligible");
    expect(eligible_count != state.globals.end(), "mutation subquery should materialize the lookup query");
    if (eligible_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(eligible_count->second) == "2",
               "mutation subquery lookup query should return eligible rows; got " +
                   copperfin::runtime::format_value(eligible_count->second));
    }
    const auto eligible_first = state.globals.find("celigiblefirst");
    expect(eligible_first != state.globals.end(), "mutation subquery should expose the first lookup value");
    if (eligible_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(eligible_first->second) == "30",
               "mutation subquery first lookup value should be 30; got " +
                   copperfin::runtime::format_value(eligible_first->second));
    }
    expect(third_name != state.globals.end(), "UPDATE subquery should capture the third record");
    expect(fourth_name != state.globals.end(), "DELETE subquery should capture the fourth record");
    if (third_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(third_name->second) == "MATCH",
               "UPDATE WHERE IN (SELECT ...) should update matching values; got " +
                   copperfin::runtime::format_value(third_name->second));
    }
    if (fourth_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(fourth_name->second) == "MATCH",
               "DELETE FROM WHERE IN (SELECT ...) should run after UPDATE; got " +
                   copperfin::runtime::format_value(fourth_name->second));
    }

    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(target_path.string(), 10U);
    expect(parsed.ok, "mutation subquery fixture should remain readable");
    if (parsed.ok && parsed.table.records.size() == 4U) {
        expect(parsed.table.records[2].values[0].display_value == "MATCH",
               "UPDATE subquery should persist the third record change");
        expect(parsed.table.records[3].deleted,
               "DELETE FROM subquery should persist the fourth record tombstone");
    }

    fs::remove_all(temp_root, ignored);
}

void test_update_and_delete_accept_not_in_subquery_predicates() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_mutation_not_in_subquery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path target_path = temp_root / "people.dbf";
    const fs::path eligible_path = temp_root / "eligible.dbf";
    write_people_dbf(target_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}, {"DELTA", 40}});
    write_people_dbf(eligible_path, {{"THIRTY", 30}, {"FORTY", 40}});

    const fs::path main_path = temp_root / "mutation_not_in_subquery.prg";
    write_text(
        main_path,
        "USE '" + target_path.string() + "' ALIAS People IN 0\n"
        "USE '" + eligible_path.string() + "' ALIAS Eligible IN 0\n"
        "UPDATE People SET NAME = 'OUTSIDE' WHERE AGE NOT IN (SELECT AGE FROM Eligible WHERE AGE >= 30)\n"
        "UPDATE People SET NAME = 'NULLLEFT' WHERE .NULL. NOT IN (SELECT AGE FROM Eligible)\n"
        "UPDATE People SET NAME = 'NULLRESULT' WHERE AGE NOT IN (SELECT .NULL. FROM Eligible)\n"
        "DELETE FROM People WHERE AGE NOT IN (SELECT AGE FROM Eligible WHERE AGE >= 30)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, std::string("NOT IN mutation subquery script should complete: ") + state.message);

    const auto parsed = copperfin::vfp::parse_dbf_table_from_file(target_path.string(), 10U);
    expect(parsed.ok, "NOT IN mutation fixture should remain readable");
    if (parsed.ok && parsed.table.records.size() == 4U) {
        expect(parsed.table.records[0].values[0].display_value == "OUTSIDE",
               "UPDATE NOT IN should update a row absent from the subquery result");
        expect(parsed.table.records[1].values[0].display_value == "OUTSIDE",
               "UPDATE NOT IN should update every absent matching row");
        expect(parsed.table.records[2].values[0].display_value == "CHARLIE",
               "UPDATE NOT IN should preserve a row present in the subquery result");
        expect(parsed.table.records[3].values[0].display_value == "DELTA",
               "UPDATE NOT IN should preserve every row present in the subquery result");
        expect(parsed.table.records[0].deleted && parsed.table.records[1].deleted,
               "DELETE NOT IN should tombstone rows absent from the subquery result");
        expect(!parsed.table.records[2].deleted && !parsed.table.records[3].deleted,
               "DELETE NOT IN should preserve rows present in the subquery result");
    }

    fs::remove_all(temp_root, ignored);
}

void test_sql_style_for_clauses_accept_macro_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sql_style_for_macro";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alpha", "10"},
        {"Bravo", "20"},
        {"Charlie", "30"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SQL-style FOR macro DBF fixture should be created");

    const fs::path main_path = temp_root / "sql_style_for_macro.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "cDeleteExpr = \"AGE = 20\"\n"
        "cUpdateExpr = \"AGE = 30\"\n"
        "cDeleteExprHolder = 'cDeleteExpr'\n"
        "cDeleteExprDeepHolder = 'cDeleteExprHolder'\n"
        "cUpdateExprHolder = 'cUpdateExpr'\n"
        "cUpdateExprDeepHolder = 'cUpdateExprHolder'\n"
        "DELETE FROM People FOR &cDeleteExprDeepHolder\n"
        "UPDATE People SET NAME = 'Thirty' FOR &cUpdateExprDeepHolder\n"
        "GO 2\n"
        "lDeleted = DELETED()\n"
        "GO 3\n"
        "cThirdName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SQL-style FOR macro script should complete: " + state.message);

    const auto deleted = state.globals.find("ldeleted");
    const auto third_name = state.globals.find("cthirdname");
    expect(deleted != state.globals.end(), "DELETE FROM ... FOR &expr should expose DELETED() result");
    expect(third_name != state.globals.end(), "UPDATE ... FOR &expr should expose updated NAME");
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true",
            "DELETE FROM ... FOR second-hop &expr should evaluate the macro-expanded filter expression");
    }
    if (third_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(third_name->second) == "Thirty",
            "UPDATE ... FOR second-hop &expr should evaluate the macro-expanded filter expression");
    }

    expect(has_runtime_event(state.events, "runtime.delete_from", "People WHERE &cDeleteExprDeepHolder"),
        "DELETE FROM with second-hop macro FOR should emit a runtime.delete_from event");
    expect(has_runtime_event(state.events, "runtime.update", "UPDATE People SET NAME = 'Thirty' FOR &cUpdateExprDeepHolder"),
        "UPDATE with second-hop macro FOR should emit a runtime.update event");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "SQL-style FOR macro commands should leave the DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 3U) {
        expect(parse_result.table.records[1].deleted,
            "DELETE FROM ... FOR &expr should tombstone the matching record");
        expect(parse_result.table.records[2].values[0U].display_value == "Thirty",
            "UPDATE ... FOR &expr should persist the updated record value");
    }

    fs::remove_all(temp_root, ignored);
}

} // namespace copperfin::table_mutation_tests
