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

// vfp::replace_record_field_value()/append_blank_record_to_file() used to
// read the ENTIRE table file into memory, mutate it, and atomically
// rewrite the entire file on every single call, making an N-record bulk
// REPLACE or APPEND O(n^2) in total I/O instead of O(n): before the fix, a
// 2000-record SCAN...REPLACE...ENDSCAN loop measured ~27.5s in isolation,
// and doubling the record count roughly quadrupled the time. This proves
// both correctness (every record ends up with the right value, none are
// skipped or duplicated) and, implicitly, performance -- this test would
// have been prohibitively slow under the old quadratic behavior, so its
// own reasonable completion time is part of what it's proving.
void test_scan_replace_loop_updates_every_record_at_scale() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scan_replace_scale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr int record_count = 2000;
    std::vector<std::pair<std::string, int>> seed_records;
    seed_records.reserve(static_cast<std::size_t>(record_count));
    for (int i = 0; i < record_count; ++i) {
        seed_records.push_back({"SEED", 0});
    }
    const fs::path table_path = temp_root / "bulk.dbf";
    write_people_dbf(table_path, seed_records);

    const fs::path main_path = temp_root / "scan_replace_scale.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Bulk IN 0\n"
        "GO TOP\n"
        "SCAN\n"
        // NAME (C(10)) rather than AGE (N(3)): a per-record marker derived
        // from RECNO() needs to fit at 2000 records without hitting the
        // separate, deliberately-unrelated numeric-overflow behavior this
        // file also tests elsewhere.
        "    REPLACE NAME WITH 'R' + LTRIM(STR(RECNO()))\n"
        "ENDSCAN\n"
        "nRecordCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "scaled SCAN/REPLACE loop should complete: " + state.message);

    const auto record_count_global = state.globals.find("nrecordcount");
    expect(record_count_global != state.globals.end(), "script should expose RECCOUNT()");
    if (record_count_global != state.globals.end()) {
        expect(copperfin::runtime::format_value(record_count_global->second) == std::to_string(record_count),
               "SCAN/REPLACE must not add, drop, or duplicate records");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), static_cast<std::size_t>(record_count));
    expect(parse_result.ok && parse_result.table.records.size() == static_cast<std::size_t>(record_count),
           "table should remain readable with exactly the seeded record count");
    if (parse_result.ok && parse_result.table.records.size() == static_cast<std::size_t>(record_count)) {
        for (std::size_t index = 0U; index < parse_result.table.records.size(); ++index) {
            expect(parse_result.table.records[index].values[0U].display_value == "R" + std::to_string(index + 1U),
                   "record " + std::to_string(index + 1U) + " should hold its own RECNO() marker, not another record's value");
        }
    }

    fs::remove_all(temp_root, ignored);
}

// Same "would have been prohibitively slow before the fix" proof for the
// append side: appending N records one at a time used to read and
// atomically rewrite the whole (ever-growing) file on every single append.
void test_append_blank_loop_creates_every_record_at_scale() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_blank_scale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    constexpr int record_count = 2000;
    const fs::path table_path = temp_root / "bulk.dbf";
    write_people_dbf(table_path, {});

    const fs::path main_path = temp_root / "append_blank_scale.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Bulk IN 0\n"
        "FOR i = 1 TO " + std::to_string(record_count) + "\n"
        "    APPEND BLANK\n"
        // NAME (C(10)) rather than AGE (N(3)): see the SCAN/REPLACE test
        // above for why a RECNO()-derived marker at this record count
        // needs a wide-enough field.
        "    REPLACE NAME WITH 'R' + LTRIM(STR(i))\n"
        "ENDFOR\n"
        "nRecordCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "scaled APPEND BLANK loop should complete: " + state.message);

    const auto record_count_global = state.globals.find("nrecordcount");
    expect(record_count_global != state.globals.end(), "script should expose RECCOUNT()");
    if (record_count_global != state.globals.end()) {
        expect(copperfin::runtime::format_value(record_count_global->second) == std::to_string(record_count),
               "appending N records one at a time should end with exactly N records");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), static_cast<std::size_t>(record_count));
    expect(parse_result.ok && parse_result.table.records.size() == static_cast<std::size_t>(record_count),
           "table should remain readable with exactly the appended record count");
    if (parse_result.ok && parse_result.table.records.size() == static_cast<std::size_t>(record_count)) {
        for (std::size_t index = 0U; index < parse_result.table.records.size(); ++index) {
            expect(!parse_result.table.records[index].deleted,
                   "appended record " + std::to_string(index + 1U) + " should not be marked deleted");
            expect(parse_result.table.records[index].values[0U].display_value == "R" + std::to_string(index + 1U),
                   "appended record " + std::to_string(index + 1U) + " should hold its own value, not another record's");
        }
    }

    fs::remove_all(temp_root, ignored);
}

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
        "SET DELETED ON\n"
        "RECALL FOR AGE = 40\n"
        "SET DELETED OFF\n"
        "LOCATE FOR NAME = 'DELTA'\n"
        "lRecalledDeletedFor = DELETED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "mutation/scan script should complete");

    const auto found = state.globals.find("cfound");
    const auto found_age = state.globals.find("nfoundage");
    const auto total = state.globals.find("ntotal");
    const auto names = state.globals.find("cnames");
    const auto deleted = state.globals.find("ldeleted");
    const auto recalled = state.globals.find("lrecalled");
    const auto deleted_name = state.globals.find("cdeletedname");
    const auto recalled_deleted_for = state.globals.find("lrecalleddeletedfor");

    expect(found != state.globals.end(), "LOCATE should expose the found NAME field");
    expect(found_age != state.globals.end(), "LOCATE should expose the found AGE field");
    expect(total != state.globals.end(), "SCAN aggregate should be captured");
    expect(names != state.globals.end(), "SCAN field concatenation should be captured");
    expect(deleted != state.globals.end(), "DELETE state should be captured");
    expect(recalled != state.globals.end(), "RECALL state should be captured");
    expect(deleted_name != state.globals.end(), "LOCATE FOR DELETED() should identify the tombstoned record");
    expect(recalled_deleted_for != state.globals.end(), "RECALL FOR with SET DELETED ON should expose the recalled record state");

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
    if (recalled_deleted_for != state.globals.end()) {
        expect(copperfin::runtime::format_value(recalled_deleted_for->second) == "false", "RECALL FOR should clear a deleted row even when SET DELETED is ON");
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

void test_delete_all_and_recall_all_affect_whole_local_table() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_delete_recall_all_local";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "delete_recall_all_local.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
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
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3683: DELETE ALL / RECALL ALL local-table script should complete");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

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

// #5508: parse_aggregate_scope_clause() assumed ALL/REST are always bare,
// trailing tokens (true for SCAN/DELETE/RECALL), but REPLACE's grammar puts
// the scope keyword *before* its mandatory field-assignment list in the
// same segment ("REPLACE ALL AMOUNT WITH 999"). That made the early-return
// guard for a non-empty tail fire, leaving "ALL AMOUNT WITH 999" completely
// unparsed and producing "target field not found" for the literal field
// name "ALL AMOUNT". Proves REPLACE ALL/REST/NEXT/RECORD now actually work.
void test_replace_all_updates_every_record() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_all_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_all_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE ALL NAME WITH 'SAME'\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE ALL should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE ALL script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE ALL script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE ALL script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "SAME", "REPLACE ALL should update record 1");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "SAME", "REPLACE ALL should update record 2");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "SAME", "REPLACE ALL should update record 3");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_rest_updates_only_remaining_records() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_rest_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_rest_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "REPLACE REST NAME WITH 'TAIL'\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE REST should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE REST script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE REST script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE REST script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "ALPHA", "REPLACE REST should not update record 1 (before the starting position)");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "TAIL", "REPLACE REST should update record 2 (the starting position)");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "TAIL", "REPLACE REST should update record 3 (after the starting position)");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_next_updates_exact_record_count() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_next_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_next_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE NEXT 2 NAME WITH 'HEAD'\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE NEXT should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE NEXT script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE NEXT script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE NEXT script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "HEAD", "REPLACE NEXT 2 should update record 1");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "HEAD", "REPLACE NEXT 2 should update record 2");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "CHARLIE", "REPLACE NEXT 2 should not update the third record (past the count)");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_record_updates_exactly_one_record() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_record_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_record_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE RECORD 2 NAME WITH 'TARGETED'\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE RECORD should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE RECORD script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE RECORD script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE RECORD script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "ALPHA", "REPLACE RECORD 2 should not update record 1");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "TARGETED", "REPLACE RECORD 2 should update record 2");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "CHARLIE", "REPLACE RECORD 2 should not update record 3");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_all_composes_with_for_clause() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_all_for_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_all_for_scope.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "REPLACE ALL NAME WITH 'OLD' FOR AGE < 25\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE ALL ... FOR should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE ALL ... FOR script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE ALL ... FOR script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE ALL ... FOR script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "OLD", "REPLACE ALL ... FOR should update matching record 1");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "OLD", "REPLACE ALL ... FOR should update matching record 2");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "CHARLIE", "REPLACE ALL ... FOR should not update non-matching record 3");
    }

    fs::remove_all(temp_root, ignored);
}

// A table can legally have a field literally named ALL/REST/NEXT/RECORD.
// parse_leading_aggregate_scope_clause()'s first version always consumed a
// leading ALL/REST/NEXT/RECORD as a scope keyword regardless of what
// followed, so "REPLACE ALL WITH 1" (ALL itself being the field name)
// would leave "WITH 1" as the assignment text and produce an empty field
// name -- breaking a program that used to work (by relying on the
// trailing-form parser's own, different, but for this shape correct,
// rejection). Proves a field named ALL still works as an ordinary
// current-record REPLACE.
void test_replace_field_named_all_is_not_treated_as_scope() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_field_named_all";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "flags.dbf";
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {
            {.name = "NAME", .type = 'C', .length = 10U},
            {.name = "ALL", .type = 'N', .length = 3U}
        },
        {{"ALPHA", "1"}});
    expect(create_result.ok, "field-named-ALL fixture creation should succeed");
    if (!create_result.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const fs::path main_path = temp_root / "replace_field_named_all.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Flags IN 0\n"
        "REPLACE ALL WITH 42\n"
        "nAll = ALL\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE ALL WITH 42 (ALL as a field name) should complete: " + state.message);

    const auto all_value = state.globals.find("nall");
    expect(all_value != state.globals.end(), "script should capture the ALL field's value");
    if (all_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(all_value->second) == "42",
               "REPLACE ALL WITH 42 should assign 42 to the field literally named ALL, not treat ALL as a scope keyword");
    }

    fs::remove_all(temp_root, ignored);
}

// The count/record-number expression for a leading NEXT/RECORD scope can
// itself contain whitespace (a parenthesized expression). Grabbing just
// the first whitespace-delimited token would truncate it and misroute the
// rest into the field-assignment list.
void test_replace_next_accepts_parenthesized_count_expression() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_next_paren_count";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_next_paren_count.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nNext = 1\n"
        "REPLACE NEXT (nNext + 1) NAME WITH 'HEAD'\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "GO 3\n"
        "cName3 = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPLACE NEXT (expr) should complete: " + state.message);

    const auto name1 = state.globals.find("cname1");
    const auto name2 = state.globals.find("cname2");
    const auto name3 = state.globals.find("cname3");
    expect(name1 != state.globals.end(), "REPLACE NEXT (expr) script should capture cName1");
    expect(name2 != state.globals.end(), "REPLACE NEXT (expr) script should capture cName2");
    expect(name3 != state.globals.end(), "REPLACE NEXT (expr) script should capture cName3");
    if (name1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name1->second) == "HEAD",
               "REPLACE NEXT (nNext + 1) should update record 1, not stop early on the parenthesized expression");
    }
    if (name2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name2->second) == "HEAD",
               "REPLACE NEXT (nNext + 1) should update record 2 (2 records total)");
    }
    if (name3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(name3->second) == "CHARLIE",
               "REPLACE NEXT (nNext + 1) should not update the third record (past the evaluated count)");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_scope_clauses_bound_physical_record_ranges() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_scopes";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(
        table_path,
        {{"ALPHA", 1}, {"BRAVO", 2}, {"CHARLIE", 3}, {"DELTA", 4}, {"ECHO", 5}});

    const fs::path main_path = temp_root / "replace_scopes.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        // This test's own subject is scope-clause record targeting, not
        // overflow handling; it uses overlong NAME literals purely as
        // convenient, visually distinct markers and has always relied on
        // them being cut to fit rather than erroring.
        "SET TRUNCATEONOVERFLOW ON\n"
        "GO 1\n"
        "REPLACE NAME WITH 'CURRENT'\n"
        "nNext = 2\n"
        "GO 2\n"
        "REPLACE AGE WITH AGE + 10 NEXT nNext IN People NOOPTIMIZE\n"
        "GO 1 IN People\n"
        "nNext1 = AGE\n"
        "GO 2 IN People\n"
        "nNext2 = AGE\n"
        "GO 3 IN People\n"
        "nNext3 = AGE\n"
        "GO 4 IN People\n"
        "nNext4 = AGE\n"
        "GO 5 IN People\n"
        "nNext5 = AGE\n"
        "REPLACE AGE WITH RECNO() ALL IN People\n"
        "GO 1 IN People\n"
        "nAll1 = AGE\n"
        "GO 2 IN People\n"
        "nAll2 = AGE\n"
        "GO 3 IN People\n"
        "nAll3 = AGE\n"
        "GO 4 IN People\n"
        "nAll4 = AGE\n"
        "GO 5 IN People\n"
        "nAll5 = AGE\n"
        "GO 3 IN People\n"
        "REPLACE AGE WITH AGE + 20 REST FOR RECNO() <> 4 WHILE RECNO() <= 5 IN People\n"
        "nTarget = 2\n"
        "REPLACE AGE WITH 99 RECORD nTarget IN People\n"
        "GO 4 IN People\n"
        "DELETE IN People\n"
        "SET DELETED ON\n"
        "SET FILTER TO AGE >= 2 IN People\n"
        "REPLACE NAME WITH 'VISIBLE' ALL IN People\n"
        "REPLACE NAME WITH 'DELETED_RECORD' RECORD 4 IN People\n"
        "REPLACE NAME WITH 'FILTERED_RECORD' RECORD 1 IN People\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3927: REPLACE scope-clause script should complete");

    const auto expect_global = [&](const std::string& name, const std::string& expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), "#3927: scope script should capture " + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   "#3927: scope result mismatch for " + name);
        }
    };
    const std::vector<std::string> expected_next{"1", "12", "13", "4", "5"};
    const std::vector<std::string> expected_all{"1", "2", "3", "4", "5"};
    for (std::size_t index = 0U; index < 5U; ++index) {
        expect_global("nnext" + std::to_string(index + 1U), expected_next[index]);
        expect_global("nall" + std::to_string(index + 1U), expected_all[index]);
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok && parse_result.table.records.size() == 5U,
           "#3927: REPLACE scope-clause table should remain readable in record order");
    if (parse_result.ok && parse_result.table.records.size() == 5U) {
        const std::vector<std::string> expected_names{
            "CURRENT", "VISIBLE", "VISIBLE", "DELETED_RE", "VISIBLE"
        };
        const std::vector<std::string> expected_ages{"1", "99", "23", "4", "25"};
        for (std::size_t index = 0U; index < parse_result.table.records.size(); ++index) {
            expect(parse_result.table.records[index].values[0U].display_value == expected_names[index],
                   "#3927: REPLACE default/ALL/filter/deleted scope mismatch at record " +
                       std::to_string(index + 1U) + ": expected '" + expected_names[index] +
                       "' got '" + parse_result.table.records[index].values[0U].display_value + "'");
            expect(parse_result.table.records[index].values[1U].display_value == expected_ages[index],
                   "#3927: REPLACE NEXT/RECORD/REST range mismatch at record " +
                       std::to_string(index + 1U));
        }
        expect(parse_result.table.records[3U].deleted,
               "#3927: scoped REPLACE should preserve the deleted marker it filtered out");
    }

    const bool has_next_scope_event = std::any_of(
        state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.replace" &&
                   event.detail.find("NEXT nNext") != std::string::npos;
        });
    const bool has_rest_scope_event = std::any_of(
        state.events.begin(), state.events.end(), [](const auto& event) {
            return event.category == "runtime.replace" &&
                   event.detail.find("REST FOR RECNO() <> 4 WHILE RECNO() <= 5") != std::string::npos;
        });
    expect(has_next_scope_event && has_rest_scope_event,
           "#3927: runtime.replace events should retain invariant scope and predicate context");

    fs::remove_all(temp_root, ignored);
}

void test_delete_and_recall_scope_clauses_bound_physical_record_ranges() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_delete_recall_scopes";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(
        table_path,
        {{"ALPHA", 1}, {"BRAVO", 2}, {"CHARLIE", 3}, {"DELTA", 4}, {"ECHO", 5}});

    const fs::path main_path = temp_root / "delete_recall_scopes.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 2\n"
        "nNext = 2\n"
        "DELETE NEXT nNext IN People\n"
        "GO 1 IN People\n"
        "lDeleteNext1 = DELETED()\n"
        "GO 2 IN People\n"
        "lDeleteNext2 = DELETED()\n"
        "GO 3 IN People\n"
        "lDeleteNext3 = DELETED()\n"
        "GO 4 IN People\n"
        "lDeleteNext4 = DELETED()\n"
        "GO 2 IN People\n"
        "RECALL NEXT nNext IN People\n"
        "GO 2 IN People\n"
        "lRecallNext2 = DELETED()\n"
        "GO 3 IN People\n"
        "lRecallNext3 = DELETED()\n"
        "DELETE RECORD 4 IN People\n"
        "GO 4 IN People\n"
        "lDeleteRecord4 = DELETED()\n"
        "RECALL RECORD 4 IN People\n"
        "GO 4 IN People\n"
        "lRecallRecord4 = DELETED()\n"
        "GO 3 IN People\n"
        "DELETE REST IN People\n"
        "GO 2 IN People\n"
        "lDeleteRest2 = DELETED()\n"
        "GO 3 IN People\n"
        "lDeleteRest3 = DELETED()\n"
        "GO 5 IN People\n"
        "lDeleteRest5 = DELETED()\n"
        "GO 3 IN People\n"
        "RECALL REST IN People\n"
        "GO 3 IN People\n"
        "lRecallRest3 = DELETED()\n"
        "GO 5 IN People\n"
        "lRecallRest5 = DELETED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#4030: DELETE/RECALL scope-clause script should complete");

    const auto expect_global = [&](const std::string& name, const std::string& expected) {
        const auto value = state.globals.find(name);
        expect(value != state.globals.end(), "#4030: scope script should capture " + name);
        if (value != state.globals.end()) {
            expect(copperfin::runtime::format_value(value->second) == expected,
                   "#4030: scope result mismatch for " + name);
        }
    };
    expect_global("ldeletenext1", "false");
    expect_global("ldeletenext2", "true");
    expect_global("ldeletenext3", "true");
    expect_global("ldeletenext4", "false");
    expect_global("lrecallnext2", "false");
    expect_global("lrecallnext3", "false");
    expect_global("ldeleterecord4", "true");
    expect_global("lrecallrecord4", "false");
    expect_global("ldeleterest2", "false");
    expect_global("ldeleterest3", "true");
    expect_global("ldeleterest5", "true");
    expect_global("lrecallrest3", "false");
    expect_global("lrecallrest5", "false");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok && parse_result.table.records.size() == 5U,
           "#4030: DELETE/RECALL scope table should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 5U) {
        expect(std::none_of(parse_result.table.records.begin(), parse_result.table.records.end(), [](const auto& record) {
            return record.deleted;
        }), "#4030: matching RECALL scopes should restore every scoped record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_additive_appends_only_memo_assignments() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "notes.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTE", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"FIRST", "A"}, {"SECOND", "B"}});
    expect(create_result.ok, "#3927: REPLACE ADDITIVE memo fixture should be created");

    const fs::path main_path = temp_root / "replace_additive.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Notes IN 0\n"
        "GO 1\n"
        "REPLACE NOTE WITH '-one' ADDITIVE\n"
        "GO 2\n"
        "REPLACE NOTE WITH '-two' ADDITIVE, NAME WITH 'UPDATED'\n"
        "GO TOP\n"
        "REPLACE NOTE WITH '+' ADDITIVE ALL\n"
        "REPLACE NOTE WITH '-undo' ADDITIVE ALL\n"
        "UNDO\n"
        "GO 1\n"
        "REPLACE NAME WITH 'PLAIN' ADDITIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3927: REPLACE ADDITIVE script should complete");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok && parse_result.table.records.size() == 2U,
           "#3927: REPLACE ADDITIVE table should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0U].values[0U].display_value == "PLAIN" &&
                   parse_result.table.records[1U].values[0U].display_value == "UPDATED",
               "#3927: ADDITIVE should be ignored for non-memo assignments");
        expect(parse_result.table.records[0U].values[1U].display_value == "A-one+" &&
                   parse_result.table.records[1U].values[1U].display_value == "B-two+",
               "#3927: per-assignment ADDITIVE should append memo values across scoped records");
    }

    fs::remove_all(temp_root, ignored);
}

void test_replace_matches_local_field_names_case_insensitively() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_engine_replace_field_case";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "mixed_case.dbf";
    const std::string narrow_high_byte_field =
        std::string(1U, static_cast<char>(0xC4U)) + "CODE";
    const std::string wide_high_byte_field =
        std::string(1U, static_cast<char>(0xD6U)) + "CODE";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "CuStOmEr", .type = 'C', .length = 10U},
        {.name = "NoTeS", .type = 'M', .length = 4U},
        {.name = narrow_high_byte_field, .type = 'C', .length = 3U},
        {.name = wide_high_byte_field, .type = 'C', .length = 10U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"ALPHA", "First", "ONE", "TWO"}});
    expect(create_result.ok, "#3984: runtime mixed-case descriptor fixture should be created");

    const fs::path main_path = temp_root / "replace_field_case.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS MixedCase IN 0\n"
        "REPLACE customer WITH 'BRAVO'\n"
        "REPLACE CUSTOMER WITH 'CHARLIE'\n"
        "REPLACE cUsToMeR WITH 'DELTA'\n"
        "REPLACE nOtEs WITH '-second' ADDITIVE\n"
        "UPDATE MixedCase SET CUstOMer = 'ECHO'\n"
        "REPLACE " + wide_high_byte_field + " WITH 'LONGVALUE'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3984: local REPLACE should ignore descriptor-name case");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.fields.size() == 4U &&
               parse_result.table.records.size() == 1U,
           "#3984: runtime-updated mixed-case table should remain readable");
    if (parse_result.ok && parse_result.table.fields.size() == 4U &&
        parse_result.table.records.size() == 1U) {
        expect(parse_result.table.fields[0U].name == "CuStOmEr" &&
                   parse_result.table.fields[1U].name == "NoTeS",
               "#3984: runtime REPLACE should preserve descriptor spelling");
        expect(parse_result.table.records[0U].values[0U].display_value == "ECHO" &&
                   parse_result.table.records[0U].values[1U].display_value == "First-second",
               "#3984: runtime REPLACE and UPDATE should persist mixed-case field updates");
        expect(parse_result.table.records[0U].values[2U].display_value == "ONE" &&
                   parse_result.table.records[0U].values[3U].display_value == "LONGVALUE",
               "#3984: runtime field lookup should preserve distinct high-byte identifiers");
    }

    fs::remove_all(temp_root, ignored);
}

void test_undo_restores_scoped_additive_replace_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_additive_undo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "notes.dbf";
    const fs::path memo_path = temp_root / "notes.fpt";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTE", .type = 'M', .length = 4U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"FIRST", "A"}, {"SECOND", "B"}});
    expect(create_result.ok, "#3927: scoped additive undo fixture should be created");

    const auto read_bytes = [](const fs::path& path) {
        std::ifstream input(path, std::ios::binary);
        return std::vector<std::uint8_t>{
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>()
        };
    };
    const std::vector<std::uint8_t> original_table_bytes = read_bytes(table_path);
    const std::vector<std::uint8_t> original_memo_bytes = read_bytes(memo_path);

    const fs::path main_path = temp_root / "replace_additive_undo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS Notes IN 0\n"
        "REPLACE NOTE WITH '-pending' ADDITIVE ALL\n"
        "UNDO\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3927: scoped additive REPLACE undo script should complete");
    expect(read_bytes(table_path) == original_table_bytes && read_bytes(memo_path) == original_memo_bytes,
           "#3927: UNDO should restore exact DBF/FPT bytes after scoped additive REPLACE");

    fs::remove_all(temp_root, ignored);
}

void test_multi_field_replace_uses_original_values_for_later_expressions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_original_values";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}, {"CHARLIE", 30}});

    const fs::path main_path = temp_root / "replace_original_values.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "LOCATE FOR NAME = 'BRAVO'\n"
        "REPLACE NAME WITH 'X', AGE WITH LEN(NAME)\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "multi-field REPLACE original-value script should complete");

    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");
    expect(after_name != state.globals.end(), "multi-field REPLACE should expose the updated NAME");
    expect(after_age != state.globals.end(), "multi-field REPLACE should expose the later AGE expression result");

    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "X", "multi-field REPLACE should still update the first assignment");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "5", "later REPLACE expressions should read the original NAME value before any assignments are applied");
    }

    fs::remove_all(temp_root, ignored);
}

} // namespace copperfin::table_mutation_tests
