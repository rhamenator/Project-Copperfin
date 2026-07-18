// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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

void test_lock_functions_and_unlock_command_track_session_locks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_lock_functions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    const fs::path other_path = temp_root / "other.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_people_dbf(other_path, {{"OTHER", 1}});

    const fs::path main_path = temp_root / "locks.prg";
    write_text(
        main_path,
        "cDefaultReprocess = SET('REPROCESS')\n"
        "cDefaultMultilocks = SET('MULTILOCKS')\n"
        "SET REPROCESS TO 3\n"
        "SET MULTILOCKS ON\n"
        "cReprocess = SET('REPROCESS')\n"
        "cMultilocks = SET('MULTILOCKS')\n"
        "SET DATASESSION TO 2\n"
        "cReprocessSession2 = SET('REPROCESS')\n"
        "cMultilocksSession2 = SET('MULTILOCKS')\n"
        "SET DATASESSION TO 1\n"
        "cReprocessRestored = SET('REPROCESS')\n"
        "cMultilocksRestored = SET('MULTILOCKS')\n"
        "USE '" + people_path.string() + "' ALIAS People IN 0\n"
        "lRecordLock = RLOCK()\n"
        "lRecordLocked = ISRLOCKED()\n"
        "lFileLock = FLOCK()\n"
        "lFileLocked = ISFLOCKED()\n"
        "UNLOCK\n"
        "lRecordAfterUnlock = ISRLOCKED()\n"
        "lFileAfterUnlock = ISFLOCKED()\n"
        "lNamedRecordLock = RLOCK('People')\n"
        "lFileRelock = FLOCK()\n"
        "GO 2\n"
        "lSecondRecordLock = RLOCK()\n"
        "UNLOCK RECORD 1 IN People\n"
        "GO 1\n"
        "lRecordOneAfterSpecificUnlock = ISRLOCKED()\n"
        "lFileAfterSpecificUnlock = ISFLOCKED()\n"
        "GO 2\n"
        "lRecordTwoAfterSpecificUnlock = ISRLOCKED()\n"
        "USE '" + other_path.string() + "' ALIAS Other IN 0\n"
        "lOtherFileLock = FLOCK()\n"
        "SELECT People\n"
        "lPeopleRecordLocked = ISRLOCKED()\n"
        "UNLOCK ALL\n"
        "lPeopleAfterUnlockAll = ISRLOCKED()\n"
        "SELECT Other\n"
        "lOtherAfterUnlockAll = ISFLOCKED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "lock function script should complete");

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
            name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cdefaultreprocess", "AUTOMATIC");
    check("cdefaultmultilocks", "OFF");
    check("creprocess", "3");
    check("cmultilocks", "ON");
    check("creprocesssession2", "AUTOMATIC");
    check("cmultilockssession2", "OFF");
    check("creprocessrestored", "3");
    check("cmultilocksrestored", "ON");
    check("lrecordlock", "true");
    check("lrecordlocked", "true");
    check("lfilelock", "true");
    check("lfilelocked", "true");
    check("lrecordafterunlock", "false");
    check("lfileafterunlock", "false");
    check("lnamedrecordlock", "true");
    check("lfilerelock", "true");
    check("lsecondrecordlock", "true");
    check("lrecordoneafterspecificunlock", "false");
    check("lfileafterspecificunlock", "true");
    check("lrecordtwoafterspecificunlock", "true");
    check("lotherfilelock", "true");
    check("lpeoplerecordlocked", "true");
    check("lpeopleafterunlockall", "false");
    check("lotherafterunlockall", "false");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock";
    }), "lock functions should emit runtime.lock events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.unlock" && event.detail == "ALL";
    }), "UNLOCK ALL should emit a runtime.unlock event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.unlock" && event.detail == "People RECORD 1";
    }), "UNLOCK RECORD should emit a record-specific runtime.unlock event");

    fs::remove_all(temp_root, ignored);
}

void test_replacing_a_used_work_area_releases_prior_table_locks() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_replace_use_unlock";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path held_path = temp_root / "held.dbf";
    const fs::path replacement_path = temp_root / "replacement.dbf";
    write_people_dbf(held_path, {{"ALPHA", 10}, {"BRAVO", 20}});
    write_people_dbf(replacement_path, {{"OTHER", 1}});

    const fs::path writer_path = temp_root / "writer.prg";
    write_text(
        writer_path,
        "SET MULTILOCKS ON\n"
        "USE '" + held_path.string() + "' ALIAS Held IN 0\n"
        "lHeldLock = FLOCK()\n"
        "USE '" + replacement_path.string() + "' ALIAS Replacement IN Held\n"
        "UNLOCK\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession writer =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(writer_path.string(), temp_root.string()));
        const auto writer_state = writer.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(writer_state.completed, "#3673: writer script should complete after replacing a locked work area");

        const auto held_lock = writer_state.globals.find("lheldlock");
        expect(held_lock != writer_state.globals.end(), "#3673: writer script should capture the initial table lock result");
        if (held_lock != writer_state.globals.end()) {
            expect(copperfin::runtime::format_value(held_lock->second) == "true",
                   "#3673: writer should successfully hold the initial table lock before replacement");
        }
    }

    const fs::path reader_path = temp_root / "reader.prg";
    write_text(
        reader_path,
        "SET MULTILOCKS ON\n"
        "USE '" + held_path.string() + "' ALIAS HeldAgain SHARED IN 0\n"
        "lRelock = FLOCK()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession reader =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(reader_path.string(), temp_root.string()));
    const auto reader_state = reader.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(reader_state.completed, "#3673: reader script should complete after the writer session exits");

    const auto relock = reader_state.globals.find("lrelock");
    expect(relock != reader_state.globals.end(), "#3673: reader script should capture the relock attempt");
    if (relock != reader_state.globals.end()) {
        expect(copperfin::runtime::format_value(relock->second) == "true",
               "#3673: replacing a used work area should release the old table lock for a later runtime session");
    }

    fs::remove_all(temp_root, ignored);
}

void test_reprocess_contention_retries_and_mutation_lock_timeouts() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_reprocess_contention";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "reprocess_contention.prg";
    write_text(
        main_path,
        "cDefaultReprocess = SET('REPROCESS')\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "cDefaultReprocessSession2 = SET('REPROCESS')\n"
        "lDefaultConflict = RLOCK()\n"
        "SET REPROCESS TO 2\n"
        "lRetryConflict = RLOCK()\n"
        "TRY\n"
        "    REPLACE NAME WITH 'BLOCKED'\n"
        "    lReplaceBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lReplaceBlocked = .T.\n"
        "    cReplaceError = err_text.Message\n"
        "ENDTRY\n"
        "SET REPROCESS TO 0\n"
        "lZeroConflict = RLOCK()\n"
        "SET DATASESSION TO 1\n"
        "UNLOCK ALL\n"
        "SET DATASESSION TO 2\n"
        "SET REPROCESS TO 2\n"
        "lAfterRelease = RLOCK()\n"
        "lAfterReleaseState = ISRLOCKED()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "REPROCESS contention script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto it = state.globals.find(name);
        if (it == state.globals.end()) {
            expect(false, name + " should be captured");
            return;
        }
        expect(copperfin::runtime::format_value(it->second) == expected,
               name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
    };

    check("cdefaultreprocess", "AUTOMATIC");
    check("cdefaultreprocesssession2", "AUTOMATIC");
    check("lheldlock", "true");
    check("ldefaultconflict", "false");
    check("lretryconflict", "false");
    check("lreplaceblocked", "true");
    check("lzeroconflict", "false");
    check("lafterrelease", "true");
    check("lafterreleasestate", "true");

    const auto replace_error = state.globals.find("creplaceerror");
    expect(replace_error != state.globals.end(), "REPLACE contention script should capture the caught error text");
    if (replace_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(replace_error->second).find("timed out waiting for record lock") != std::string::npos,
               "REPLACE contention error should report the record-lock timeout");
    }

    const auto count_retry_events = [&](const std::string& detail_fragment) {
        return static_cast<int>(std::count_if(state.events.begin(), state.events.end(), [&](const auto& event) {
            return event.category == "runtime.lock_retry" &&
                   event.detail.find(detail_fragment) != std::string::npos;
        }));
    };

    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=AUTOMATIC") == 8,
           "default REPROCESS should perform eight retry/yield attempts before RLOCK() fails");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=2") == 2,
           "SET REPROCESS TO 2 should perform two retry attempts before RLOCK() fails");
    expect(count_retry_events("REPLACE recno=1 reprocess=2") == 2,
           "REPLACE under lock contention should honor the per-session REPROCESS retry budget");
    expect(count_retry_events("PeopleTwo RLOCK recno=1 reprocess=0") == 0,
           "SET REPROCESS TO 0 should not busy-spin before RLOCK() fails");

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("PeopleTwo RLOCK timeout recno=1 reprocess=AUTOMATIC") != std::string::npos;
    }), "default RLOCK contention should emit a deterministic runtime.lock_timeout event");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_timeout" &&
               event.detail.find("REPLACE timeout recno=1 reprocess=2") != std::string::npos;
    }), "REPLACE contention should emit a deterministic runtime.lock_timeout event");

    fs::remove_all(temp_root, ignored);
}

void test_lock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_lock_retry_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "lock_retry_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "TRY\n"
        "    ENTER CRITICAL shared\n"
        "    REPLACE NAME WITH 'BLOCKED'\n"
        "    lPolicyBlocked = .F.\n"
        "CATCH TO err_text\n"
        "    lPolicyBlocked = .T.\n"
        "    cPolicyError = err_text.Message\n"
        "ENDTRY\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section lock-retry policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto policy_blocked = state.globals.find("lpolicyblocked");
    const auto policy_error = state.globals.find("cpolicyerror");
    expect(held_lock != state.globals.end(), "lock-retry policy script should capture the first-session held lock");
    expect(policy_blocked != state.globals.end(), "lock-retry policy script should capture the policy-block result");
    expect(policy_error != state.globals.end(), "lock-retry policy script should capture the policy-block message");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "lock-retry policy script should hold the first-session record lock");
    }
    if (policy_blocked != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_blocked->second) == "true",
               "REPLACE under contention inside a critical section should be rejected");
    }
    if (policy_error != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_error->second) ==
                   "Blocking operation LOCK RETRY is not allowed while holding CRITICAL section shared",
               "lock-retry policy error should route through the default locale catalog");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos;
    }), "lock-retry contention inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_rlock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_rlock_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "rlock_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "GO 1\n"
        "lHeldLock = RLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "GO 1\n"
        "SET REPROCESS TO 2\n"
        "ENTER CRITICAL shared\n"
        "lSecondLock = RLOCK()\n"
        "IF lSecondLock\n"
        "    lPolicyBlocked = .F.\n"
        "ELSE\n"
        "    lPolicyBlocked = .T.\n"
        "ENDIF\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section RLOCK policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto second_lock = state.globals.find("lsecondlock");
    const auto policy_blocked = state.globals.find("lpolicyblocked");
    expect(held_lock != state.globals.end(), "RLOCK policy script should capture the first-session held lock");
    expect(second_lock != state.globals.end(), "RLOCK policy script should capture the contested second lock result");
    expect(policy_blocked != state.globals.end(), "RLOCK policy script should capture the policy-block result");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "RLOCK policy script should hold the first-session record lock");
    }
    if (second_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_lock->second) == "false",
               "RLOCK under contention inside a critical section should return false");
    }
    if (policy_blocked != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_blocked->second) == "true",
               "RLOCK under contention inside a critical section should be rejected");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_retry" &&
               event.detail.find("PeopleTwo RLOCK") != std::string::npos;
    }), "RLOCK inside a critical section should fail before emitting retry backoff events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos &&
               event.detail.find("PeopleTwo RLOCK") != std::string::npos;
    }), "RLOCK contention inside a critical section should emit a runtime.critical.blocking_violation event");

    fs::remove_all(temp_root, ignored);
}

void test_flock_retry_blocking_is_rejected_inside_critical_section() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_flock_critical_policy";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path people_path = temp_root / "people.dbf";
    write_people_dbf(people_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "flock_critical_policy.prg";
    write_text(
        main_path,
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleOne SHARED IN 0\n"
        "lHeldLock = FLOCK()\n"
        "SET DATASESSION TO 2\n"
        "SET MULTILOCKS ON\n"
        "USE '" + people_path.string() + "' ALIAS PeopleTwo SHARED IN 0\n"
        "ENTER CRITICAL shared\n"
        "lSecondLock = FLOCK()\n"
        "lPolicyRejected = (lSecondLock = .F.)\n"
        "EXIT CRITICAL shared\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "critical-section FLOCK policy script should complete: " + state.message);

    const auto held_lock = state.globals.find("lheldlock");
    const auto second_lock = state.globals.find("lsecondlock");
    const auto policy_rejected = state.globals.find("lpolicyrejected");
    expect(held_lock != state.globals.end(), "FLOCK policy script should capture the first-session file lock");
    expect(second_lock != state.globals.end(), "FLOCK policy script should capture the second-session contested lock attempt");
    expect(policy_rejected != state.globals.end(), "FLOCK policy script should capture rejection state");
    if (held_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(held_lock->second) == "true",
               "first-session FLOCK should acquire and hold the file lock");
    }
    if (second_lock != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_lock->second) == "false",
               "FLOCK under contention inside a critical section should return false");
    }
    if (policy_rejected != state.globals.end()) {
        expect(copperfin::runtime::format_value(policy_rejected->second) == "true",
               "FLOCK contention inside a critical section should be rejected");
    }

    expect(std::none_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.lock_retry" &&
               event.detail.find("PeopleTwo FLOCK") != std::string::npos;
    }), "FLOCK inside a critical section should fail before emitting retry backoff events");
    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.critical.blocking_violation" &&
               event.detail.find("operation=LOCK RETRY") != std::string::npos &&
               event.detail.find("PeopleTwo FLOCK") != std::string::npos;
    }), "FLOCK contention inside a critical section should emit a runtime.critical.blocking_violation event");

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

void test_rollback_transaction_replays_local_dbf_changes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "rollback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "BEGIN TRANSACTION\n"
        "GO 1\n"
        "REPLACE NAME WITH 'MUTATED', AGE WITH 999\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'TEMP', AGE WITH 100\n"
        "ROLLBACK\n"
        "GO 1\n"
        "cFirst = NAME\n"
        "nFirstAge = AGE\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ROLLBACK script should complete");

    const auto first = state.globals.find("cfirst");
    const auto first_age = state.globals.find("nfirstage");
    const auto count = state.globals.find("ncount");
    expect(first != state.globals.end(), "ROLLBACK script should capture first record name");
    expect(first_age != state.globals.end(), "ROLLBACK script should capture first record age");
    expect(count != state.globals.end(), "ROLLBACK script should capture record count");
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "ALPHA", "ROLLBACK should restore original NAME field values");
    }
    if (first_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_age->second) == "10", "ROLLBACK should restore original numeric field values");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2", "ROLLBACK should remove rows appended within the transaction");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.rollback";
    }), "ROLLBACK should emit runtime.transaction.rollback");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "ROLLBACK should keep DBF readable");
    expect(parse_result.table.records.size() == 2U, "ROLLBACK should restore the original DBF row count");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "ROLLBACK should restore row 1 text values");
        expect(parse_result.table.records[0].values[1].display_value == "10", "ROLLBACK should restore row 1 numeric values");
    }

    fs::remove_all(temp_root, ignored);
}

void test_cancel_rolls_back_active_transaction() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_cancel_rollback";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "cancel_rollback.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nInitial = RECCOUNT()\n"
        "BEGIN TRANSACTION\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'TEMP'\n"
        "REPLACE AGE WITH 99\n"
        "CANCEL\n"
        "cShouldNotRun = 'yes'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CANCEL should complete program flow after rolling back active transaction");
    const auto initial_count = state.globals.find("ninitial");
    const auto canceled_marker = state.globals.find("cshouldnotrun");
    if (initial_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(initial_count->second) == "2", "initial RECCOUNT should be captured before transaction");
    }
    expect(canceled_marker == state.globals.end(),
           "CANCEL should prevent statements after it from executing");

    const auto has_cancel = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &event) { return event.category == "runtime.cancel"; });
    const auto has_txn_rollback = std::any_of(state.events.begin(), state.events.end(),
        [](const auto &event) {
            return event.category == "runtime.transaction.rollback" && event.detail == "0";
        });
    expect(has_cancel, "CANCEL should emit runtime.cancel event");
    expect(has_txn_rollback, "CANCEL inside transaction should emit runtime.transaction.rollback event");

    const fs::path transaction_root = temp_root / "runtime-temp" / "transactions";
    if (std::filesystem::exists(transaction_root, ignored))
    {
        bool found_pending_journal = false;
        for (const auto &entry : std::filesystem::directory_iterator(transaction_root, ignored))
        {
            if (!entry.is_directory(ignored))
            {
                continue;
            }
            const std::filesystem::path journal_path = entry.path() / "journal.log";
            if (std::filesystem::exists(journal_path, ignored))
            {
                found_pending_journal = true;
                break;
            }
        }
        expect(!found_pending_journal, "CANCEL should leave no active transaction journal artifacts");
    }

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
        expect(parse_result.ok, "CANCEL rollback should leave DBF readable");
        expect(parse_result.table.records.size() == 2U, "CANCEL rollback should restore original row count");
    if (parse_result.table.records.size() == 2U) {
        expect(parse_result.table.records[0].values[0].display_value == "ALPHA", "CANCEL rollback should preserve first row NAME");
        expect(parse_result.table.records[1].values[0].display_value == "BRAVO", "CANCEL rollback should preserve second row NAME");
    }

    fs::remove_all(temp_root, ignored);
}

void test_rollback_transaction_replays_append_from_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_append_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "rollback_append_array.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "DIMENSION aRows[1,2]\n"
        "aRows[1,1] = 'TEMP'\n"
        "aRows[1,2] = 99\n"
        "BEGIN TRANSACTION\n"
        "APPEND FROM ARRAY aRows\n"
        "ROLLBACK\n"
        "nCount = RECCOUNT()\n"
        "GO 1\n"
        "cFirst = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY rollback script should complete");

    const auto count = state.globals.find("ncount");
    const auto first = state.globals.find("cfirst");
    expect(count != state.globals.end(), "APPEND FROM ARRAY rollback should capture record count");
    expect(first != state.globals.end(), "APPEND FROM ARRAY rollback should capture first record");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "1", "ROLLBACK should remove APPEND FROM ARRAY rows");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "ALPHA", "ROLLBACK should preserve original row after APPEND FROM ARRAY");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "APPEND FROM ARRAY rollback should leave DBF readable");
    expect(parse_result.table.records.size() == 1U, "APPEND FROM ARRAY rollback should restore original row count on disk");

    fs::remove_all(temp_root, ignored);
}

void test_rollback_transaction_prunes_stale_alter_table_field_rules() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_alter_rollback_rules";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}});

    const fs::path main_path = temp_root / "rollback_alter_rules.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "BEGIN TRANSACTION\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8) DEFAULT 'NEW'\n"
        "ROLLBACK\n"
        "INSERT INTO People (NAME, AGE) VALUES ('BRAVO', 20)\n"
        "nCount = RECCOUNT()\n"
        "nFields = FCOUNT('People')\n"
        "GO 2\n"
        "cSecond = NAME\n"
        "nSecondAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ALTER TABLE rollback field-rule pruning script should complete: " + state.message);

    const auto count = state.globals.find("ncount");
    const auto fields = state.globals.find("nfields");
    const auto second = state.globals.find("csecond");
    const auto second_age = state.globals.find("nsecondage");
    expect(count != state.globals.end(), "ALTER TABLE rollback script should capture record count");
    expect(fields != state.globals.end(), "ALTER TABLE rollback script should capture field count");
    expect(second != state.globals.end(), "ALTER TABLE rollback script should capture second row NAME");
    expect(second_age != state.globals.end(), "ALTER TABLE rollback script should capture second row AGE");
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2",
            "ROLLBACK should still allow later inserts after pruning stale ALTER TABLE field rules");
    }
    if (fields != state.globals.end()) {
        expect(copperfin::runtime::format_value(fields->second) == "2",
            "ROLLBACK should restore the original field count after ALTER TABLE replay");
    }
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "BRAVO",
            "ROLLBACK should allow inserts to use the restored pre-ALTER schema");
    }
    if (second_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_age->second) == "20",
            "ROLLBACK should preserve inserted numeric values after schema replay");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(parse_result.ok, "ALTER TABLE rollback should leave DBF readable");
    expect(parse_result.table.fields.size() == 2U, "ALTER TABLE rollback should restore the original schema on disk");
    expect(parse_result.table.records.size() == 2U, "ALTER TABLE rollback should still allow post-rollback inserts");

    fs::remove_all(temp_root, ignored);
}

void test_rollback_transaction_removes_created_table_cursor() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_create_table";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "created_txn.dbf";
    const fs::path main_path = temp_root / "rollback_create_table.prg";
    write_text(
        main_path,
        "BEGIN TRANSACTION\n"
        "CREATE TABLE '" + table_path.string() + "' (NAME C(10), AGE N(3))\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'TEMP', AGE WITH 42\n"
        "ROLLBACK\n"
        "lUsedAfter = USED('created_txn')\n"
        "lFileAfter = FILE('" + table_path.string() + "')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CREATE TABLE rollback script should complete");

    const auto used_after = state.globals.find("lusedafter");
    const auto file_after = state.globals.find("lfileafter");
    expect(used_after != state.globals.end(), "CREATE TABLE rollback should capture USED()");
    expect(file_after != state.globals.end(), "CREATE TABLE rollback should capture FILE()");
    if (used_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_after->second) == "false", "ROLLBACK should close a cursor whose created table was removed");
    }
    if (file_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_after->second) == "false", "ROLLBACK should remove a table created inside the transaction");
    }
    expect(!fs::exists(table_path), "CREATE TABLE rollback should remove the DBF from disk");

    fs::remove_all(temp_root, ignored);
}

// #251 [gap-08b]
void test_transaction_rollback_leaves_table_unchanged() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_rollback_insert";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path main_path = temp_root / "rollback_insert.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "nCountBefore = RECCOUNT()\n"
        "BEGIN TRANSACTION\n"
        "INSERT INTO People (NAME, AGE) VALUES ('GAMMA', 30)\n"
        "nCountDuring = RECCOUNT()\n"
        "ROLLBACK\n"
        "nCountAfter = RECCOUNT()\n"
        "GO 1\n"
        "cFirstAfter = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INSERT INTO rollback script should complete");

    const auto count_before = state.globals.find("ncountbefore");
    const auto count_during = state.globals.find("ncountduring");
    const auto count_after = state.globals.find("ncountafter");
    const auto first_after = state.globals.find("cfirstafter");

    expect(count_before != state.globals.end(), "RECCOUNT() before transaction should be captured");
    expect(count_during != state.globals.end(), "RECCOUNT() during transaction should be captured");
    expect(count_after != state.globals.end(), "RECCOUNT() after ROLLBACK should be captured");
    expect(first_after != state.globals.end(), "first record after ROLLBACK should be captured");

    if (count_before != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_before->second) == "2", "table should start with two records");
    }
    if (count_during != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_during->second) == "3", "INSERT INTO inside transaction should be visible before ROLLBACK");
    }
    if (count_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(count_after->second) == "2", "ROLLBACK should remove the INSERT INTO row and restore original count");
    }
    if (first_after != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_after->second) == "ALPHA", "ROLLBACK should leave original first record intact");
    }

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 5U);
    expect(parse_result.ok, "ROLLBACK should keep DBF readable");
    expect(parse_result.table.records.size() == 2U, "ROLLBACK should remove INSERT INTO row from disk");

    fs::remove_all(temp_root, ignored);
}

void test_startup_replays_pending_transaction_journal() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transaction_replay";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});

    const fs::path writer_path = temp_root / "writer.prg";
    write_text(
        writer_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "BEGIN TRANSACTION\n"
        "GO 1\n"
        "REPLACE NAME WITH 'BROKEN', AGE WITH 777\n"
        "APPEND BLANK\n"
        "REPLACE NAME WITH 'PENDING', AGE WITH 1\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession writer = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(writer_path.string(), temp_root.string()));
        const auto writer_state = writer.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(writer_state.completed, "transaction writer script should complete");
    }

    const fs::path reader_path = temp_root / "reader.prg";
    write_text(
        reader_path,
        "USE '" + table_path.string() + "' ALIAS People IN 0\n"
        "GO 1\n"
        "cFirst = NAME\n"
        "nFirstAge = AGE\n"
        "nCount = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession reader = copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(reader_path.string(), temp_root.string()));
    const auto state = reader.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "replay reader script should complete");

    const auto first = state.globals.find("cfirst");
    const auto first_age = state.globals.find("nfirstage");
    const auto count = state.globals.find("ncount");
    expect(first != state.globals.end(), "startup replay script should capture first record name");
    expect(first_age != state.globals.end(), "startup replay script should capture first record age");
    expect(count != state.globals.end(), "startup replay script should capture record count");
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "ALPHA", "startup replay should restore NAME after interrupted transaction");
    }
    if (first_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_age->second) == "10", "startup replay should restore AGE after interrupted transaction");
    }
    if (count != state.globals.end()) {
        expect(copperfin::runtime::format_value(count->second) == "2", "startup replay should remove rows created by interrupted transaction");
    }

    expect(std::any_of(state.events.begin(), state.events.end(), [](const auto& event) {
        return event.category == "runtime.transaction.replay";
    }), "startup session should emit runtime.transaction.replay when crash recovery runs");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_local_table_mutation_and_scan_flow();
    test_delete_all_and_recall_all_affect_whole_local_table();
    test_replace_for_updates_all_matching_records();
    test_replace_scope_clauses_bound_physical_record_ranges();
    test_delete_and_recall_scope_clauses_bound_physical_record_ranges();
    test_replace_additive_appends_only_memo_assignments();
    test_replace_matches_local_field_names_case_insensitively();
    test_undo_restores_scoped_additive_replace_bytes();
    test_multi_field_replace_uses_original_values_for_later_expressions();
    test_pack_compacts_deleted_local_records();
    test_pack_is_reverted_by_undo();
    test_zap_truncates_local_table_records();
    test_zap_is_reverted_by_undo();
    test_replace_character_field_truncates_to_field_width();
    test_character_field_at_maximum_width_round_trips();
    test_memo_field_replace_with_empty_string();
    test_set_exclusive_controls_table_maintenance_guards();
    test_lock_functions_and_unlock_command_track_session_locks();
    test_replacing_a_used_work_area_releases_prior_table_locks();
    test_reprocess_contention_retries_and_mutation_lock_timeouts();
    test_lock_retry_blocking_is_rejected_inside_critical_section();
    test_rlock_retry_blocking_is_rejected_inside_critical_section();
    test_flock_retry_blocking_is_rejected_inside_critical_section();
    test_insert_into_and_delete_from_local_table();
    test_insert_into_select_materializes_filtered_ordered_rows();
    test_insert_into_select_rolls_back_the_whole_failed_batch();
    test_insert_into_rolls_back_failed_local_append();
    test_indexed_table_mutation_succeeds_for_structural_indexes();
    test_append_blank_supports_opaque_field_layouts_at_runtime();
    test_cancel_rolls_back_active_transaction();
    test_update_command_sets_scoped_records();
    test_update_and_delete_accept_in_subquery_predicates();
    test_sql_style_for_clauses_accept_macro_expressions();
    test_undo_reverts_latest_append_blank();
    test_undo_reverts_latest_delete_command();
    test_undo_reverts_latest_update_command();
    test_undo_reverts_latest_insert_into_command();
    test_undo_reverts_latest_create_table_command();
    test_undo_reverts_latest_alter_table_command();
    test_append_from_array_rolls_back_failed_multi_row_write();
    test_undo_reverts_latest_append_from_array();
    test_undo_reverts_latest_replacement_command();
    test_command_undo_query_reports_available_label_after_bulk_operation();
    test_undo_all_reverts_multiple_latest_commands();
    test_undo_without_history_fails_deterministically();
    test_rollback_transaction_replays_local_dbf_changes();
    test_rollback_transaction_replays_append_from_array();
    test_rollback_transaction_prunes_stale_alter_table_field_rules();
    test_rollback_transaction_removes_created_table_cursor();
    test_transaction_rollback_leaves_table_unchanged();
    test_startup_replays_pending_transaction_journal();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
