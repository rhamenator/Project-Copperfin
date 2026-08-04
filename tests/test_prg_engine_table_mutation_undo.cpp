// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

void test_undo_reverts_latest_replacement_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_latest";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_latest.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'CHANGED'\n"
        "UNDO\n"
        "GO 1\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest replace script should complete");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit a runtime.command_undo event");

    const auto name = state.globals.find("cname");
    expect(name != state.globals.end(), "UNDO should expose restored NAME");
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ALPHA",
            "UNDO should restore the pre-command NAME value");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "UNDO latest should leave DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 1U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
            "UNDO latest should persist original first-row NAME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_command_undo_query_reports_available_label_after_bulk_operation() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_query";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_query.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'CHANGED'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "command undo query script should complete");
    expect(session.can_undo_command(), "command undo query should report an available undo");
    const std::string undo_label = session.command_undo_label();
    expect(undo_label.find("REPLACE") != std::string::npos,
        "command undo query should expose the last undoable command label (got '" + undo_label + "')");

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_append_blank() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_append_blank";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_append_blank.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "APPEND BLANK\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest APPEND BLANK script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_blank" && event.detail == "People";
    }), "APPEND BLANK should emit runtime.append_blank");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "UNDO should expose RECCOUNT after rollback");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "UNDO after APPEND BLANK should restore the original row count");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "APPEND BLANK + UNDO should keep DBF readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
            "UNDO after APPEND BLANK should preserve first row NAME");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO",
            "UNDO after APPEND BLANK should preserve second row NAME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_delete_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_delete";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_delete.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 1\n"
        "DELETE\n"
        "UNDO\n"
        "nDeleted = DELETED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest DELETE script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.delete" && event.detail == "People";
    }), "DELETE should emit runtime.delete");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto deleted = state.globals.find("ndeleted");
    expect(deleted != state.globals.end(), "UNDO after DELETE should expose DELETED()");
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "false",
            "UNDO after DELETE should clear the deleted flag");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "DELETE + UNDO should keep DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 1U) {
        expect(!parse_result.table.records[0].deleted, "UNDO should restore record 1 deletion state");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_update_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_update";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_update.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UPDATE SET AGE = AGE + 10\n"
        "UNDO\n"
        "GO 1\n"
        "nAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest UPDATE script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.update";
    }), "UPDATE should emit runtime.update");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto age = state.globals.find("nage");
    expect(age != state.globals.end(), "UNDO should expose restored AGE");
    if (age != state.globals.end()) {
        expect(copperfin::runtime::format_value(age->second) == "10",
            "UNDO after UPDATE should restore original AGE");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "UPDATE + UNDO should keep DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 1U) {
        expect(parse_result.table.records[0].values[1].display_value == "10",
            "UNDO should persist reverted AGE for first row");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_insert_into_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_insert_into";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_insert_into.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "INSERT INTO People (NAME, AGE) VALUES ('DELTA', 40)\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest INSERT INTO script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.insert_into";
    }), "INSERT INTO should emit runtime.insert_into");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "UNDO after INSERT INTO should expose RECCOUNT");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "UNDO after INSERT INTO should restore original row count");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "INSERT INTO + UNDO should keep DBF readable");
    expect(parse_result.table.records.size() == 2U,
        "UNDO after INSERT INTO should remove the appended row");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[1].values[1].display_value == "20",
            "UNDO after INSERT INTO should preserve second row AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_create_table_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_create_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "created_from_undo.dbf";
    const fs::path main_path = temp_root / "undo_create_table.prg";
    write_text(
        main_path,
        "CREATE TABLE '" + table_path.string() + "' (NAME C(10), AGE N(3))\n"
        "UNDO\n"
        "lUsed = USED('created_from_undo')\n"
        "lExists = FILE('" + table_path.string() + "')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest CREATE TABLE script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.create_table";
    }), "CREATE TABLE should emit runtime.create_table");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto used = state.globals.find("lused");
    const auto exists = state.globals.find("lexists");
    expect(used != state.globals.end(), "UNDO create-table script should capture USED() state");
    expect(exists != state.globals.end(), "UNDO create-table script should capture FILE() state");
    if (used != state.globals.end()) {
        expect(copperfin::runtime::format_value(used->second) == "false", "UNDO should close the created table alias");
    }
    if (exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(exists->second) == "false", "UNDO should remove the created table file");
    }
    expect(!fs::exists(table_path), "UNDO should remove CREATE TABLE output file from disk");

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_alter_table_command() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_alter_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_alter_table.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8) DEFAULT 'NEW'\n"
        "UNDO\n"
        "nFields = FCOUNT('People')\n"
        "GO 1\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest ALTER TABLE script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.alter_table";
    }), "ALTER TABLE should emit a runtime.alter_table event");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto fields = state.globals.find("nfields");
    const auto name = state.globals.find("cname");
    expect(fields != state.globals.end(), "UNDO ALTER TABLE should expose restored field count");
    expect(name != state.globals.end(), "UNDO ALTER TABLE should expose existing row data");
    if (fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields->second) == "2",
            "UNDO should restore original field count");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ALPHA",
            "UNDO should preserve existing row data");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "UNDO ALTER TABLE should keep DBF readable");
    expect(parse_result.table.fields.size() == 2U, "UNDO should restore the original schema");
    expect(parse_result.table.records.size() == 2U, "UNDO ALTER TABLE should preserve record count");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_rolls_back_failed_multi_row_write() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_write_failure";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    const auto original_size = fs::file_size(table_path);

    const fs::path main_path = temp_root / "append_from_array_write_failure.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "DIMENSION aRows[2,2]\n"
        "aRows[1,1] = 'GAMMA'\n"
        "aRows[1,2] = 30\n"
        "aRows[2,1] = 'THIS-NAME-IS-TOO-LONG'\n"
        "aRows[2,2] = 40\n"
        "APPEND FROM ARRAY aRows FIELDS NAME, AGE\n"
        "nAfterError = RECCOUNT()\n"
        "nRecAfterError = RECNO()\n"
        "cNameAfterError = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "failed APPEND FROM ARRAY field write should pause with an error");
    expect(state.location.line == 8U,
           "failed APPEND FROM ARRAY field write should highlight the command");
    expect(state.message.find("too large") != std::string::npos,
           "failed APPEND FROM ARRAY should preserve the DBF writer diagnostic");
    expect(!session.can_undo_command(),
           "failed APPEND FROM ARRAY should roll back instead of committing an undo entry");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after failed APPEND FROM ARRAY should keep the session alive");
    const auto count = state.globals.find("naftererror");
    const auto recno = state.globals.find("nrecaftererror");
    const auto name = state.globals.find("cnameaftererror");
    expect(count != state.globals.end(), "failed APPEND FROM ARRAY should expose restored record count");
    expect(recno != state.globals.end(), "failed APPEND FROM ARRAY should expose restored record pointer");
    expect(name != state.globals.end(), "failed APPEND FROM ARRAY should expose the original current row");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
               "failed APPEND FROM ARRAY should restore the in-memory record count");
    }
    if (recno != state.globals.end()) {
        expect(copperfin::runtime::format_value(recno->second) == "2",
               "failed APPEND FROM ARRAY should restore the pre-command record pointer");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "BRAVO",
               "failed APPEND FROM ARRAY should restore the original current row");
    }

    expect(fs::file_size(table_path) == original_size,
           "failed APPEND FROM ARRAY should restore the original DBF size");
    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "failed APPEND FROM ARRAY rollback should leave the DBF readable");
    expect(parse_result.table.records.size() == 2U,
           "failed APPEND FROM ARRAY should remove every row appended before the failure");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
               "failed APPEND FROM ARRAY should preserve the first original row");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO",
               "failed APPEND FROM ARRAY should preserve the second original row");
    }
    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_from_array";
    }), "failed APPEND FROM ARRAY should not emit a success event");

    fs::remove_all(temp_root, ignored);
}

void test_undo_reverts_latest_append_from_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_append_from_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_append_from_array.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "DIMENSION aRows[2,2]\n"
        "aRows[1,1] = 'GAMMA'\n"
        "aRows[1,2] = 30\n"
        "aRows[2,1] = 'DELTA'\n"
        "aRows[2,2] = 40\n"
        "APPEND FROM ARRAY aRows\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO latest APPEND FROM ARRAY script should complete");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.append_from_array";
    }), "APPEND FROM ARRAY should emit a runtime.append_from_array event");
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO should emit runtime.command_undo");

    const auto count = state.globals.find("ncount");
    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    expect(count != state.globals.end(), "UNDO should expose RECCOUNT after bulk append rollback");
    expect(name1 != state.globals.end(), "UNDO should expose first-row NAME");
    expect(name2 != state.globals.end(), "UNDO should expose second-row NAME");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "UNDO APPEND FROM ARRAY should restore original row count");
    }
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "ALPHA",
            "UNDO APPEND FROM ARRAY should preserve first-row NAME");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "BRAVO",
            "UNDO APPEND FROM ARRAY should preserve second-row NAME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_all_reverts_multiple_latest_commands() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_all.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'MUTATED1'\n"
        "GO 2\n"
        "REPLACE NAME WITH 'MUTATED2'\n"
        "UNDO ALL\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "UNDO ALL replace script should complete");
    expect(has_runtime_event(state.events, "runtime.command_undo", "ALL"),
        "UNDO ALL should emit a runtime.command_undo event");

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    expect(name1 != state.globals.end(), "UNDO ALL should capture first-row NAME");
    expect(name2 != state.globals.end(), "UNDO ALL should capture second-row NAME");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "ALPHA",
            "UNDO ALL should restore first row NAME");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "BRAVO",
            "UNDO ALL should restore second row NAME");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "UNDO ALL should leave DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA",
            "UNDO ALL should persist original first-row NAME");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO",
            "UNDO ALL should persist original second-row NAME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_without_history_fails_deterministically() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_command_undo_empty";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "undo_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "UNDO\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "empty UNDO stack should pause with an error");
    expect(state.location.line == 2U, "empty UNDO should report the UNDO line");
    expect(state.message == "No command to UNDO",
        "empty UNDO should route deterministic no-command error through the default locale catalog");

    const auto name = state.globals.find("cname");
    expect(name == state.globals.end(), "script should not execute statements after a failed UNDO");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "empty UNDO script should leave DBF readable");
    if (parse_result.ok && parse_result.table.records.size() >= 1U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "failed UNDO should not mutate row 1");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO", "failed UNDO should not mutate row 2");
    }

    fs::remove_all(temp_root, ignored);
}

} // namespace copperfin::table_mutation_tests
