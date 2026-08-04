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
#include <locale>
#include <system_error>
#include <vector>

namespace copperfin::table_mutation_tests
{

using namespace copperfin::test_support;

namespace {

class grouped_transaction_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class transaction_global_locale_guard final {
public:
    explicit transaction_global_locale_guard(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~transaction_global_locale_guard() { std::locale::global(previous_); }

private:
    std::locale previous_;
};

std::filesystem::path find_generated_transaction_journal(
    const std::filesystem::path& temp_root,
    std::error_code& ignored) {
    const std::filesystem::path transaction_root = temp_root / "runtime-temp" / "transactions";
    for (const auto& entry : std::filesystem::directory_iterator(transaction_root, ignored)) {
        if (ignored) {
            break;
        }
        const std::filesystem::path candidate = entry.path() / "journal.log";
        if (std::filesystem::is_regular_file(candidate, ignored)) {
            return candidate;
        }
    }
    return {};
}

}  // namespace

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

void test_transaction_journal_serializes_grouped_levels_invariantly() {
    namespace fs = std::filesystem;
    constexpr int transaction_level = 1234;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_prg_engine_transaction_grouped_level";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    std::string source;
    source.reserve(static_cast<std::size_t>(transaction_level) * 18U + 8U);
    for (int level = 0; level < transaction_level; ++level) {
        source += "BEGIN TRANSACTION\n";
    }
    source += "RETURN\n";
    const fs::path writer_path = temp_root / "writer.prg";
    write_text(writer_path, source);

    const std::locale grouping_locale(std::locale::classic(), new grouped_transaction_numpunct());
    {
        transaction_global_locale_guard locale_guard(grouping_locale);
        copperfin::runtime::PrgRuntimeSession writer = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(writer_path.string(), temp_root.string()));
        const auto writer_state = writer.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(writer_state.completed, "grouped-locale transaction writer should complete");
    }

    const fs::path journal_path = find_generated_transaction_journal(temp_root, ignored);
    expect(!journal_path.empty(), "grouped-locale transaction journal should exist");
    if (!journal_path.empty()) {
        const std::string journal_text = read_text(journal_path);
        expect(journal_text.find("LEVEL\t1234\n") != std::string::npos,
               "transaction journal level should use invariant ungrouped digits");
        expect(journal_text.find("LEVEL\t1.234\n") == std::string::npos,
               "transaction journal level must reject host digit grouping at serialization");
    }

    fs::remove_all(temp_root, ignored);
}

void test_startup_rejects_malformed_transaction_journal_scalars() {
    namespace fs = std::filesystem;

    struct MalformedJournalCase {
        std::string name;
        std::string valid_token;
        std::string malformed_token;
    };

    const std::vector<MalformedJournalCase> cases{
        {"unsupported_version", "VERSION\t1\n", "VERSION\t2\n"},
        {"partial_level", "LEVEL\t1\n", "LEVEL\t1garbage\n"},
        {"overflowing_level", "LEVEL\t1\n", "LEVEL\t999999999999999999999999\n"},
        {"malformed_exists", "", ""}};

    for (const auto& test_case : cases) {
        const fs::path temp_root =
            fs::temp_directory_path() / ("copperfin_prg_engine_transaction_replay_" + test_case.name);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path table_path = temp_root / "people.dbf";
        write_people_dbf(table_path, {{"ALPHA", 10}, {"BRAVO", 20}});
        const std::string original_bytes = read_text(table_path);

        const fs::path writer_path = temp_root / "writer.prg";
        write_text(
            writer_path,
            "USE '" + table_path.string() + "' ALIAS People IN 0\n"
            "BEGIN TRANSACTION\n"
            "GO 1\n"
            "REPLACE NAME WITH 'BROKEN', AGE WITH 777\n"
            "RETURN\n");
        {
            copperfin::runtime::PrgRuntimeSession writer = copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(writer_path.string(), temp_root.string()));
            const auto writer_state = writer.run(copperfin::runtime::DebugResumeAction::continue_run);
            expect(writer_state.completed, test_case.name + ": transaction writer should complete");
        }

        const std::string modified_bytes = read_text(table_path);
        expect(modified_bytes != original_bytes,
               test_case.name + ": transaction writer should modify the live DBF before recovery");

        const fs::path journal_path = find_generated_transaction_journal(temp_root, ignored);
        expect(!journal_path.empty(), test_case.name + ": generated pending journal should exist");
        if (journal_path.empty()) {
            fs::remove_all(temp_root, ignored);
            continue;
        }

        std::string journal_text = read_text(journal_path);
        const std::string valid_token = test_case.name == "malformed_exists"
            ? "FILE\t" + table_path.string() + "\t1\t"
            : test_case.valid_token;
        const std::string malformed_token = test_case.name == "malformed_exists"
            ? "FILE\t" + table_path.string() + "\t1garbage\t"
            : test_case.malformed_token;
        const std::size_t token_position = journal_text.find(valid_token);
        expect(token_position != std::string::npos,
               test_case.name + ": generated journal should contain the scalar token under test");
        if (token_position == std::string::npos) {
            fs::remove_all(temp_root, ignored);
            continue;
        }
        journal_text.replace(token_position, valid_token.size(), malformed_token);
        write_text(journal_path, journal_text);

        const fs::path reader_path = temp_root / "reader.prg";
        write_text(reader_path, "RETURN\n");
        copperfin::runtime::PrgRuntimeSession reader = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(reader_path.string(), temp_root.string()));
        const auto reader_state = reader.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(reader_state.completed, test_case.name + ": startup should fail closed without a runtime fault");
        expect(fs::is_regular_file(table_path),
               test_case.name + ": malformed journal must not delete the live DBF");
        expect(read_text(table_path) == modified_bytes,
               test_case.name + ": malformed journal must not overwrite or partially replay the live DBF");
        expect(std::none_of(reader_state.events.begin(), reader_state.events.end(), [](const auto& event) {
            return event.category == "runtime.transaction.replay";
        }), test_case.name + ": malformed journal must not emit a successful replay event");

        fs::remove_all(temp_root, ignored);
    }
}

} // namespace copperfin::table_mutation_tests
