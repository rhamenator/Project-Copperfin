// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
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

void test_pack_compacts_deleted_local_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_pack";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});
    const auto original_size = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "pack.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "PACK\n"
        "nAfterPack = RECCOUNT()\n"
        "GO 1\n"
        "cFirst = NAME\n"
        "GO 2\n"
        "cSecond = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PACK script should complete");

    const auto after_pack = state.globals.find("nafterpack");
    const auto first = state.globals.find("cfirst");
    const auto second = state.globals.find("csecond");
    expect(after_pack != state.globals.end(), "PACK should expose updated RECCOUNT()");
    expect(first != state.globals.end(), "PACK should preserve the first undeleted row");
    expect(second != state.globals.end(), "PACK should compact later undeleted rows");
    if (after_pack != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_pack->second) == "2", "PACK should reduce RECCOUNT() after deleted records are removed");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "ALPHA", "PACK should preserve row order before the deleted record");
    }
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "CHARLIE", "PACK should move later rows into the compacted slot");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.pack";
    }), "PACK should emit a runtime.pack event");
    expect(std::filesystem::file_size(table_path) < original_size, "PACK should physically shrink the DBF file");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "PACK should leave the DBF readable");
    expect(parse_result.table.records.size() == 2U, "PACK should persist only undeleted records");
    if (parse_result.table.records.size() == 2U) {
        expect(!parse_result.table.records[0].deleted, "PACK should write active row markers for kept row 1");
        expect(!parse_result.table.records[1].deleted, "PACK should write active row markers for kept row 2");
        expect(parse_result.table.records[1].values[0].display_value == "CHARLIE", "PACK should persist compacted row values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_zap_truncates_local_table_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_zap";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    const auto original_size = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "zap.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "ZAP\n"
        "nAfterZap = RECCOUNT()\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'DELTA', AGE WITH 40\n"
        "nAfterAppend = RECCOUNT()\n"
        "GO 1\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ZAP script should complete");

    const auto after_zap = state.globals.find("nafterzap");
    const auto after_append = state.globals.find("nafterappend");
    const auto name = state.globals.find("cname");
    expect(after_zap != state.globals.end(), "ZAP should expose zero RECCOUNT()");
    expect(after_append != state.globals.end(), "APPEND BLANK should work after ZAP");
    expect(name != state.globals.end(), "field lookup should work after ZAP and append");
    if (after_zap != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_zap->second) == "0", "ZAP should clear all records");
    }
    if (after_append != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_append->second) == "1", "APPEND BLANK after ZAP should create the first record");
    }
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "DELTA", "REPLACE after ZAP should persist new values");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.zap";
    }), "ZAP should emit a runtime.zap event");
    expect(std::filesystem::file_size(table_path) < original_size, "ZAP followed by one append should keep the DBF smaller than the original two-row table");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "ZAP should leave the DBF readable");
    expect(parse_result.table.records.size() == 1U, "ZAP plus one append should persist one record");
    if (parse_result.table.records.size() == 1U) {
        expect(parse_result.table.records[0].values[0].display_value == "DELTA", "ZAP should preserve schema for later appended row values");
        expect(parse_result.table.records[0].values[1].display_value == "40", "ZAP should preserve numeric schema for later appended row values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_pack_is_reverted_by_undo() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_pack_undo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "pack_undo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "PACK\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "GO 2\n"
        "cSecond = NAME\n"
        "lDeleted = DELETED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PACK + UNDO script should complete: " + state.message);
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO after PACK should emit a runtime.command_undo event");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "nCount variable should exist after UNDO");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "3",
            "UNDO should restore PACK's physically-removed record (got '" + copperfin::runtime::format_value(count->second) + "')");
    }

    const auto second = state.globals.find("csecond");
    const auto deleted = state.globals.find("ldeleted");
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "BRAVO",
            "UNDO should restore the record PACK removed, in its original position");
    }
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true",
            "UNDO should restore the deleted-record tombstone that PACK physically removed");
    }

    fs::remove_all(temp_root, ignored);
}

void test_zap_is_reverted_by_undo() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_zap_undo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "zap_undo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "ZAP\n"
        "UNDO\n"
        "nCount = RECCOUNT()\n"
        "GO 1\n"
        "cFirst = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ZAP + UNDO script should complete: " + state.message);
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO after ZAP should emit a runtime.command_undo event");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "nCount variable should exist after UNDO");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "UNDO should restore ZAP's truncated records (got '" + copperfin::runtime::format_value(count->second) + "')");
    }

    const auto first = state.globals.find("cfirst");
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "ALPHA",
            "UNDO should restore the original first record ZAP removed");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_character_field_truncates_to_field_width() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_truncate_char";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "truncate_char.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'ABCDEFGHIJKL'\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "character truncation script should complete");

    const auto name = state.globals.find("cname");
    expect(name != state.globals.end(), "truncation script should expose the updated NAME value");
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "ABCDEFGHIJ",
               "REPLACE should truncate character values to field width");
    }

    fs::remove_all(temp_root, ignored);
}

void test_character_field_at_maximum_width_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_exact_char_width";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "exact_width_char.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'MAXWIDTH10'\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "exact-width character round-trip script should complete");

    const auto name = state.globals.find("cname");
    expect(name != state.globals.end(), "exact-width round-trip script should expose the updated NAME value");
    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "MAXWIDTH10",
               "REPLACE should preserve values that exactly match field width");
    }

    fs::remove_all(temp_root, ignored);
}

void test_memo_field_replace_with_empty_string() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_empty_memo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "replace_empty_memo.prg";
    write_text(
        main_path,
        "CREATE TABLE '" + (temp_root / "memo_items.dbf").string() + "' (NOTE M)\n"
        "INSERT INTO memo_items (NOTE) VALUES ('HELLO')\n"
        "REPLACE NOTE WITH ''\n"
        "cNote = NOTE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "memo empty-string replace script should complete");

    const auto note = state.globals.find("cnote");
    expect(note != state.globals.end(), "memo empty-string replace script should expose NOTE");
    if (note != state.globals.end()) {
        expect(copperfin::runtime::format_value(note->second).empty(),
               "REPLACE memo field with an empty string should round-trip as empty");
    }

    fs::remove_all(temp_root, ignored);
}

void test_set_exclusive_controls_table_maintenance_guards() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_exclusive_maintenance";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path shared_path = temp_root / "shared_people.dbf";
    write_people_dbf(shared_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path shared_prg = temp_root / "shared_pack.prg";
    write_text(
        shared_prg,
        "cDefaultExclusive = SET('EXCLUSIVE')\n"
        "SET EXCLUSIVE OFF\n"
        "cOffExclusive = SET('EXCLUSIVE')\n"
        "USE '" + shared_path.string() + "' ALIAS People SHARED IN 0\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "PACK\n"
        "xAfterPack = 1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession shared_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(shared_prg, temp_root));

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "en-US", true);
    const auto shared_state = shared_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(shared_state.reason == copperfin::runtime::DebugPauseReason::error, "PACK on a shared local cursor should pause with an error");
    expect(shared_state.message == "PACK requires exclusive use of the target cursor",
           "en-US PACK shared-cursor failure should use the catalog message");
    expect(shared_state.globals.find("xafterpack") == shared_state.globals.end(), "PACK shared-cursor failure should stop before later statements");

    const auto default_exclusive = shared_state.globals.find("cdefaultexclusive");
    const auto off_exclusive = shared_state.globals.find("coffexclusive");
    expect(default_exclusive != shared_state.globals.end(), "SET('EXCLUSIVE') default should be captured");
    expect(off_exclusive != shared_state.globals.end(), "SET('EXCLUSIVE') after OFF should be captured");
    if (default_exclusive != shared_state.globals.end()) {
        expect(copperfin::runtime::format_value(default_exclusive->second) == "ON", "SET('EXCLUSIVE') should default to ON");
    }
    if (off_exclusive != shared_state.globals.end()) {
        expect(copperfin::runtime::format_value(off_exclusive->second) == "OFF", "SET EXCLUSIVE OFF should update SET('EXCLUSIVE')");
    }

    const auto shared_parse = copperfin::vfp::parse_dbf_table_from_file(shared_path.string(), 5U);
    expect(shared_parse.ok, "shared PACK failure should leave the DBF readable");
    expect(shared_parse.table.records.size() == 3U, "shared PACK failure should not compact the DBF");

    for (const auto& [locale, expected_text] : std::vector<std::pair<std::string, std::string>>{
             {"es-419", "PACK requiere el uso exclusivo del cursor de destino"},
             {"pt-BR", "PACK exige o uso exclusivo do cursor de destino"}}) {
        set_env_value("COPPERFIN_LOCALE", locale, true);
        auto localized_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(shared_prg, temp_root));
        const auto localized_state = localized_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(localized_state.reason == copperfin::runtime::DebugPauseReason::error,
               locale + " PACK shared-cursor failure should pause with an error");
        expect(localized_state.message == expected_text,
               locale + " PACK shared-cursor failure should use the localized catalog message");
    }

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    auto pseudo_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(shared_prg, temp_root));
    const auto pseudo_state = pseudo_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(pseudo_state.reason == copperfin::runtime::DebugPauseReason::error,
           "qps-ploc PACK shared-cursor failure should pause with an error");
    expect(pseudo_state.message.starts_with("[!! ") && pseudo_state.message.find("PACK") != std::string::npos &&
               pseudo_state.message != "PACK requires exclusive use of the target cursor",
           "qps-ploc PACK shared-cursor failure should pseudo-localize prose while preserving the command token");

    const fs::path exclusive_path = temp_root / "exclusive_people.dbf";
    write_people_dbf(exclusive_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path exclusive_prg = temp_root / "exclusive_pack.prg";
    write_text(
        exclusive_prg,
        "cDefaultExclusive = SET('EXCLUSIVE')\n"
        "SET EXCLUSIVE OFF\n"
        "cOffExclusive = SET('EXCLUSIVE')\n"
        "SET DATASESSION TO 2\n"
        "cSession2Exclusive = SET('EXCLUSIVE')\n"
        "SET DATASESSION TO 1\n"
        "cRestoredExclusive = SET('EXCLUSIVE')\n"
        "USE '" + exclusive_path.string() + "' ALIAS People EXCLUSIVE IN 0\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "PACK\n"
        "SET EXCLUSIVE ON\n"
        "cOnExclusive = SET('EXCLUSIVE')\n"
        "nAfterPack = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession exclusive_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(exclusive_prg, temp_root));

    const auto exclusive_state = exclusive_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exclusive_state.completed, "USE ... EXCLUSIVE should allow local PACK even when SET EXCLUSIVE is OFF");

    const auto default_exclusive_2 = exclusive_state.globals.find("cdefaultexclusive");
    const auto off_exclusive_2 = exclusive_state.globals.find("coffexclusive");
    const auto session2_exclusive = exclusive_state.globals.find("csession2exclusive");
    const auto restored_exclusive = exclusive_state.globals.find("crestoredexclusive");
    const auto on_exclusive = exclusive_state.globals.find("conexclusive");
    const auto after_pack = exclusive_state.globals.find("nafterpack");
    expect(default_exclusive_2 != exclusive_state.globals.end(), "SET('EXCLUSIVE') default should be captured in the exclusive-open script");
    expect(off_exclusive_2 != exclusive_state.globals.end(), "SET('EXCLUSIVE') after OFF should be captured in the exclusive-open script");
    expect(session2_exclusive != exclusive_state.globals.end(), "SET('EXCLUSIVE') in session 2 should be captured");
    expect(restored_exclusive != exclusive_state.globals.end(), "SET('EXCLUSIVE') after restoring session 1 should be captured");
    expect(on_exclusive != exclusive_state.globals.end(), "SET('EXCLUSIVE') after ON should be captured");
    expect(after_pack != exclusive_state.globals.end(), "exclusive PACK should expose updated RECCOUNT()");
    if (default_exclusive_2 != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(default_exclusive_2->second) == "ON", "SET('EXCLUSIVE') should default to ON in a fresh session");
    }
    if (off_exclusive_2 != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(off_exclusive_2->second) == "OFF", "SET EXCLUSIVE OFF should update SET('EXCLUSIVE') before switching sessions");
    }
    if (session2_exclusive != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(session2_exclusive->second) == "ON", "a fresh second session should keep the default SET('EXCLUSIVE') state");
    }
    if (restored_exclusive != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_exclusive->second) == "OFF", "restoring session 1 should restore its prior SET('EXCLUSIVE') value");
    }
    if (on_exclusive != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(on_exclusive->second) == "ON", "SET EXCLUSIVE ON should update SET('EXCLUSIVE')");
    }
    if (after_pack != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(after_pack->second) == "2", "exclusive PACK should compact deleted local rows");
    }

    fs::remove_all(temp_root, ignored);
}

} // namespace copperfin::table_mutation_tests
