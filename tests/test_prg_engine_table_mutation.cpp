#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_local_table_mutation_and_scan_flow() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_table_mutation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "mutation.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "cFound = NAME\n"
        "nFoundAge = AGE\n"
        "REPLACE AGE WITH 21, NAME WITH 'BRAVOX'\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'DELTA', AGE WITH 40\n"
        "GO TOP\n"
        "nTotal = 0\n"
        "cNames = ''\n"
        "nScanCount = 0\n"
        "SCAN FOR AGE >= 21\n"
        "    nTotal = nTotal + AGE\n"
        "    nScanCount = nScanCount + 1\n"
        "    IF nScanCount = 1\n"
        "        cNames = NAME\n"
        "    ELSE\n"
        "        cNames = cNames + ',' + NAME\n"
        "    ENDIF\n"
        "ENDSCAN\n"
        "GO 2\n"
        "DELETE\n"
        "lDeleted = DELETED()\n"
        "RECALL\n"
        "lRecalled = DELETED()\n"
        "DELETE FOR AGE = 40\n"
        "LOCATE FOR DELETED()\n"
        "cDeletedName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "mutation/scan script should complete");

    const auto found = state.globals.find("cfound");
    const auto found_age = state.globals.find("nfoundage");
    const auto total = state.globals.find("ntotal");
    const auto names = state.globals.find("cnames");
    const auto deleted = state.globals.find("ldeleted");
    const auto recalled = state.globals.find("lrecalled");
    const auto deleted_name = state.globals.find("cdeletedname");

    expect(found != state.globals.end(), "LOCATE should expose the found NAME field");
    expect(found_age != state.globals.end(), "LOCATE should expose the found AGE field");
    expect(total != state.globals.end(), "SCAN aggregate should be captured");
    expect(names != state.globals.end(), "SCAN field concatenation should be captured");
    expect(deleted != state.globals.end(), "DELETE state should be captured");
    expect(recalled != state.globals.end(), "RECALL state should be captured");
    expect(deleted_name != state.globals.end(), "LOCATE FOR DELETED() should identify the tombstoned record");

    if (found != state.globals.end()) {
        expect(copperfin::runtime::format_value(found->second) == "BRAVO", "LOCATE should position the matching record before REPLACE");
    }
    if (found_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(found_age->second) == "20", "field resolution should expose numeric record values before mutation");
    }
    if (total != state.globals.end()) {
        expect(copperfin::runtime::format_value(total->second) == "91", "SCAN should iterate the mutated matching records and sum AGE");
    }
    if (names != state.globals.end()) {
        expect(copperfin::runtime::format_value(names->second) == "BRAVOX,CHARLIE,DELTA", "SCAN FOR should iterate the matching records in table order");
    }
    if (deleted != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted->second) == "true", "DELETE should tombstone the current record");
    }
    if (recalled != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled->second) == "false", "RECALL should clear the tombstone flag");
    }
    if (deleted_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(deleted_name->second) == "DELTA", "DELETE FOR should tombstone the matching appended record");
    }

    expect(
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.locate"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.scan"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.replace"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.append_blank"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.delete"; }) &&
        std::any_of(state.events.begin(), state.events.end(), [](const auto& event) { return event.category == "runtime.recall"; }),
        "mutation/query commands should emit runtime events");

    fs::remove_all(temp_root, ignored);
}

void test_replace_for_updates_all_matching_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_for";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_for.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NAME WITH 'JUNIOR' FOR AGE < 25 IN People\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE FOR script should complete");

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE FOR should allow reading updated first record value");
    expect(name2 != state.globals.end(), "REPLACE FOR should allow reading updated second record value");
    expect(name3 != state.globals.end(), "REPLACE FOR should preserve non-matching third record value");

    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "JUNIOR", "REPLACE FOR should update matching record 1");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "JUNIOR", "REPLACE FOR should update matching record 2");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "CHARLIE", "REPLACE FOR should not update non-matching records");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.replace" && event.detail.find("FOR AGE < 25") != std::string::npos;
    }), "REPLACE FOR should emit runtime.replace with the FOR filter context");

    fs::remove_all(temp_root, ignored);
}

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession shared_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = shared_prg.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto shared_state = shared_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(shared_state.reason == copperfin::runtime::DebugPauseReason::error, "PACK on a shared local cursor should pause with an error");
    expect(shared_state.message.find("exclusive use") != std::string::npos, "PACK shared-cursor failure should mention exclusive use");
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

    const fs::path exclusive_path = temp_root / "exclusive_people.dbf";
    write_people_dbf(exclusive_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path exclusive_prg = temp_root / "exclusive_pack.prg";
    write_text(
        exclusive_prg,
        "SET EXCLUSIVE OFF\n"
        "USE '" + exclusive_path.string() + "' ALIAS People EXCLUSIVE IN 0\n"
        "DELETE FOR NAME = 'BRAVO'\n"
        "PACK\n"
        "SET EXCLUSIVE ON\n"
        "cOnExclusive = SET('EXCLUSIVE')\n"
        "nAfterPack = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession exclusive_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = exclusive_prg.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto exclusive_state = exclusive_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(exclusive_state.completed, "USE ... EXCLUSIVE should allow local PACK even when SET EXCLUSIVE is OFF");

    const auto on_exclusive = exclusive_state.globals.find("conexclusive");
    const auto after_pack = exclusive_state.globals.find("nafterpack");
    expect(on_exclusive != exclusive_state.globals.end(), "SET('EXCLUSIVE') after ON should be captured");
    expect(after_pack != exclusive_state.globals.end(), "exclusive PACK should expose updated RECCOUNT()");
    if (on_exclusive != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(on_exclusive->second) == "ON", "SET EXCLUSIVE ON should update SET('EXCLUSIVE')");
    }
    if (after_pack != exclusive_state.globals.end()) {
        expect(copperfin::runtime::format_value(after_pack->second) == "2", "exclusive PACK should compact deleted local rows");
    }

    fs::remove_all(temp_root, ignored);
}

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

void test_append_blank_for_unsupported_field_layout_surfaces_runtime_error() {
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

    const auto original_size = std::filesystem::file_size(table_path);

    const fs::path main_path = temp_root / "append_blank_unsupported.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Weird IN 0\n"
        "APPEND BLANK\n"
        "xAfterError = 17\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error, "unsupported-layout APPEND BLANK should pause with an error");
    expect(state.location.line == 2U, "unsupported-layout APPEND BLANK should highlight the mutation statement");
    expect(
        state.message.find("APPEND BLANK is not yet supported") != std::string::npos,
        "unsupported-layout APPEND BLANK should explain the unsupported field layout");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "continuing after unsupported-layout APPEND BLANK failure should keep the session alive");
    const auto after_error = state.globals.find("xaftererror");
    expect(after_error != state.globals.end(), "post-error statements should still execute after unsupported-layout APPEND BLANK failure");
    if (after_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_error->second) == "17", "post-error execution should preserve later statements after unsupported-layout APPEND BLANK failure");
    }

    expect(std::filesystem::file_size(table_path) == original_size, "unsupported-layout APPEND BLANK should not change the DBF size");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

}  // namespace

int main() {
    test_local_table_mutation_and_scan_flow();
    test_replace_for_updates_all_matching_records();
    test_pack_compacts_deleted_local_records();
    test_zap_truncates_local_table_records();
    test_set_exclusive_controls_table_maintenance_guards();
    test_insert_into_and_delete_from_local_table();
    test_insert_into_rolls_back_failed_local_append();
    test_indexed_table_mutation_succeeds_for_structural_indexes();
    test_append_blank_for_unsupported_field_layout_surfaces_runtime_error();
    test_update_command_sets_scoped_records();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
