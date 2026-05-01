#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"
#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#if defined(_WIN32)
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#define _getpid getpid
#endif
#include <sstream>
#include <system_error>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

void test_scatter_memvar_from_current_record() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "people.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "scatter_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "grabbed = m.NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER MEMVAR script should complete");

    const auto grabbed = state.globals.find("grabbed");
    expect(grabbed != state.globals.end(), "grabbed variable should exist after SCATTER MEMVAR");
    if (grabbed != state.globals.end()) {
        const std::string val = copperfin::runtime::format_value(grabbed->second);
        expect(val.find("Alice") != std::string::npos || val == "Alice      ",
            "SCATTER MEMVAR should copy NAME field into m.NAME (got: '" + val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_memvar_fields_blank_and_for_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_memvar_fields";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42", "true"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "scatter/gather typed DBF fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_memvar_fields.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME, AGE MEMVAR\n"
        "cName = m.NAME\n"
        "nAgePlus = m.AGE + 1\n"
        "cActiveType = VARTYPE(m.ACTIVE)\n"
        "SCATTER FIELDS AGE, ACTIVE MEMVAR BLANK\n"
        "nBlankAge = m.AGE\n"
        "lBlankActive = m.ACTIVE\n"
        "m.NAME = 'Skipped'\n"
        "GATHER MEMVAR FIELDS NAME FOR .F.\n"
        "m.NAME = 'Updated'\n"
        "GATHER MEMVAR FIELDS NAME FOR .T.\n"
        "cAfterGather = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER MEMVAR field-filter script should complete");

    const auto name = state.globals.find("cname");
    const auto age_plus = state.globals.find("nageplus");
    const auto active_type = state.globals.find("cactivetype");
    const auto blank_age = state.globals.find("nblankage");
    const auto blank_active = state.globals.find("lblankactive");
    const auto after_gather = state.globals.find("caftergather");
    const bool has_skipped_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.gather" && ev.detail == "memvar skipped";
        });

    expect(name != state.globals.end(), "SCATTER FIELDS should populate selected NAME memvar");
    expect(age_plus != state.globals.end(), "SCATTER should preserve numeric AGE as a numeric memvar");
    expect(active_type != state.globals.end(), "SCATTER FIELDS should leave unselected ACTIVE undefined");
    expect(blank_age != state.globals.end(), "SCATTER BLANK should create a numeric blank AGE value");
    expect(blank_active != state.globals.end(), "SCATTER BLANK should create a logical blank ACTIVE value");
    expect(has_skipped_event, "GATHER FOR .F. should skip field replacement");
    expect(after_gather != state.globals.end(), "GATHER FOR .T. should update the selected field");

    if (name != state.globals.end()) {
        expect(copperfin::runtime::format_value(name->second) == "Alice",
            "SCATTER FIELDS should copy NAME into m.NAME");
    }
    if (age_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(age_plus->second) == "43",
            "SCATTER should expose numeric AGE as arithmetic-capable");
    }
    if (active_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(active_type->second) == "U",
            "SCATTER FIELDS should not populate omitted ACTIVE memvar");
    }
    if (blank_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_age->second) == "0",
            "SCATTER BLANK should use numeric zero for numeric fields");
    }
    if (blank_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_active->second) == "false",
            "SCATTER BLANK should use false for logical fields");
    }
    if (after_gather != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather->second) == "Updated",
            "GATHER FOR .T. should apply field replacement");
    }
    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "GATHER MEMVAR destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty() && !persisted.table.records[0U].values.empty()) {
        expect(persisted.table.records[0U].values[0U].display_value == "Updated",
            "only the FOR .T. GATHER should persist the NAME update");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_memvar_single_name_field_filter_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_memvar_name_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "single NAME-field SCATTER/GATHER MEMVAR fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_memvar_name_field.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "m.AGE = 900\n"
        "SCATTER FIELDS NAME MEMVAR\n"
        "cScatteredName = m.NAME\n"
        "nAgeAfterScatter = m.AGE\n"
        "m.NAME = 'NameOnly'\n"
        "m.AGE = 901\n"
        "GATHER MEMVAR FIELDS NAME\n"
        "cAfterGatherName = NAME\n"
        "SCATTER FIELDS AGE MEMVAR\n"
        "nAfterGatherAge = m.AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "single NAME-field SCATTER/GATHER MEMVAR script should complete: " + state.message);

    const auto scattered_name = state.globals.find("cscatteredname");
    const auto age_after_scatter = state.globals.find("nageafterscatter");
    const auto after_gather_name = state.globals.find("caftergathername");
    const auto after_gather_age = state.globals.find("naftergatherage");

    expect(scattered_name != state.globals.end(), "SCATTER FIELDS NAME MEMVAR should populate m.NAME");
    expect(age_after_scatter != state.globals.end(), "SCATTER FIELDS NAME MEMVAR should preserve an unrelated preseeded m.AGE value");
    expect(after_gather_name != state.globals.end(), "GATHER MEMVAR FIELDS NAME should update NAME");
    expect(after_gather_age != state.globals.end(), "GATHER MEMVAR FIELDS NAME should leave AGE readable");

    if (scattered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(scattered_name->second) == "Alice",
            "SCATTER FIELDS NAME MEMVAR should read the NAME field even when it matches a command keyword");
    }
    if (age_after_scatter != state.globals.end()) {
        expect(copperfin::runtime::format_value(age_after_scatter->second) == "900",
            "SCATTER FIELDS NAME MEMVAR should not overwrite unrelated memvars");
    }
    if (after_gather_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather_name->second) == "NameOnly",
            "GATHER MEMVAR FIELDS NAME should write back only the NAME field");
    }
    if (after_gather_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather_age->second) == "42",
            "GATHER MEMVAR FIELDS NAME should leave AGE unchanged");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_to_array_and_gather_from_array_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "scatter/gather array DBF fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_array.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME, AGE TO aRow\n"
        "nArrayLen = ALEN(aRow)\n"
        "nRows = ALEN(aRow, 1)\n"
        "nCols = ALEN(aRow, 2)\n"
        "cFirst = aRow[1]\n"
        "nSecondPlus = aRow(2) + 1\n"
        "REPLACE NAME WITH 'Changed', AGE WITH 7\n"
        "GATHER FROM aRow FIELDS NAME, AGE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "cMacroRowName = 'aMacroRow'\n"
        "SCATTER FIELDS NAME, AGE TO &cMacroRowName\n"
        "nMacroArrayLen = ALEN(&cMacroRowName)\n"
        "cMacroFirst = &cMacroRowName[1]\n"
        "REPLACE NAME WITH 'MacroChg', AGE WITH 8\n"
        "GATHER FROM &cMacroRowName FIELDS NAME, AGE\n"
        "cMacroAfterName = NAME\n"
        "nMacroAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER TO array / GATHER FROM array script should complete: " + state.message);

    const auto array_len = state.globals.find("narraylen");
    const auto rows = state.globals.find("nrows");
    const auto cols = state.globals.find("ncols");
    const auto first = state.globals.find("cfirst");
    const auto second_plus = state.globals.find("nsecondplus");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");
    const auto macro_array_len = state.globals.find("nmacroarraylen");
    const auto macro_first = state.globals.find("cmacrofirst");
    const auto macro_after_name = state.globals.find("cmacroaftername");
    const auto macro_after_age = state.globals.find("nmacroafterage");

    expect(array_len != state.globals.end(), "ALEN(aRow) should expose array element count");
    expect(rows != state.globals.end(), "ALEN(aRow, 1) should expose first dimension");
    expect(cols != state.globals.end(), "ALEN(aRow, 2) should expose second dimension");
    expect(first != state.globals.end(), "aRow[1] should read the first scattered value");
    expect(second_plus != state.globals.end(), "aRow(2) should read the second scattered value");
    expect(after_name != state.globals.end(), "GATHER FROM array should restore NAME");
    expect(after_age != state.globals.end(), "GATHER FROM array should restore AGE");
    expect(macro_array_len != state.globals.end(), "SCATTER TO macro-expanded array should expose array element count");
    expect(macro_first != state.globals.end(), "SCATTER TO macro-expanded array should be readable through macro access");
    expect(macro_after_name != state.globals.end(), "GATHER FROM macro-expanded array should restore NAME");
    expect(macro_after_age != state.globals.end(), "GATHER FROM macro-expanded array should restore AGE");

    if (array_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_len->second) == "2",
            "SCATTER TO array should create two array elements");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "2",
            "SCATTER TO array should create a one-dimensional row count matching field count");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "1",
            "SCATTER TO array should expose one column for first-pass one-dimensional arrays");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "Alice",
            "array bracket access should read the first scattered value");
    }
    if (second_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(second_plus->second) == "43",
            "array paren access should preserve numeric scattered values");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "GATHER FROM array should write NAME from the array");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "GATHER FROM array should write AGE from the array");
    }
    if (macro_array_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_array_len->second) == "2",
            "SCATTER TO macro-expanded array should create two array elements");
    }
    if (macro_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_first->second) == "Alice",
            "macro-expanded SCATTER array access should read the first scattered value");
    }
    if (macro_after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_after_name->second) == "Alice",
            "GATHER FROM macro-expanded array should write NAME from the array");
    }
    if (macro_after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_after_age->second) == "42",
            "GATHER FROM macro-expanded array should write AGE from the array");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_memvar_preserves_date_and_datetime_like_values() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_memvar_dates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "BIRTHDAY", .type = 'D', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "20260418", "julian:2460447 millis:49556000"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "date/datetime scatter-gather memvar fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_memvar_dates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "cBirth = m.BIRTHDAY\n"
        "cStamp = m.STAMP\n"
        "m.BIRTHDAY = '04/19/2026'\n"
        "m.STAMP = '04/19/2026 01:02:03'\n"
        "GATHER MEMVAR FIELDS BIRTHDAY, STAMP\n"
        "cAfterBirth = DTOC(BIRTHDAY, 1)\n"
        "cAfterStamp = TTOC(STAMP, 1)\n"
        "SCATTER MEMVAR BLANK\n"
        "cBlankBirthType = VARTYPE(m.BIRTHDAY)\n"
        "cBlankStampType = VARTYPE(m.STAMP)\n"
        "m.BIRTHDAY = m.BIRTHDAY\n"
        "m.STAMP = m.STAMP\n"
        "GATHER MEMVAR FIELDS BIRTHDAY, STAMP\n"
        "SCATTER MEMVAR\n"
        "cAfterBlankBirth = m.BIRTHDAY\n"
        "cAfterBlankStamp = m.STAMP\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "date/datetime SCATTER/GATHER MEMVAR script should complete");

    const auto birth = state.globals.find("cbirth");
    const auto stamp = state.globals.find("cstamp");
    const auto after_birth = state.globals.find("cafterbirth");
    const auto after_stamp = state.globals.find("cafterstamp");
    const auto blank_birth_type = state.globals.find("cblankbirthtype");
    const auto blank_stamp_type = state.globals.find("cblankstamptype");
    const auto after_blank_birth = state.globals.find("cafterblankbirth");
    const auto after_blank_stamp = state.globals.find("cafterblankstamp");

    expect(birth != state.globals.end(), "SCATTER MEMVAR should populate date fields");
    expect(stamp != state.globals.end(), "SCATTER MEMVAR should populate datetime fields");
    expect(after_birth != state.globals.end(), "GATHER MEMVAR should restore updated date fields");
    expect(after_stamp != state.globals.end(), "GATHER MEMVAR should restore updated datetime fields");
    expect(blank_birth_type != state.globals.end(), "SCATTER MEMVAR BLANK should still define date memvars");
    expect(blank_stamp_type != state.globals.end(), "SCATTER MEMVAR BLANK should still define datetime memvars");
    expect(after_blank_birth != state.globals.end(), "blank GATHER should leave a readable blank date field");
    expect(after_blank_stamp != state.globals.end(), "blank GATHER should leave a readable blank datetime field");

    if (birth != state.globals.end()) {
        expect(copperfin::runtime::format_value(birth->second) == "04/18/2026",
            "SCATTER MEMVAR should expose dates in runtime date format");
    }
    if (stamp != state.globals.end()) {
        expect(copperfin::runtime::format_value(stamp->second) == "04/18/2026 13:45:56",
            "SCATTER MEMVAR should expose datetimes in runtime datetime format");
    }
    if (after_birth != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_birth->second) == "20260419",
            "GATHER MEMVAR should serialize runtime date strings back into the date field");
    }
    if (after_stamp != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_stamp->second) == "20260419010203",
            "GATHER MEMVAR should serialize runtime datetime strings back into the datetime field");
    }
    if (blank_birth_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_birth_type->second) == "C",
            "SCATTER MEMVAR BLANK should keep date fields as blank string values, not undefined values");
    }
    if (blank_stamp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_stamp_type->second) == "C",
            "SCATTER MEMVAR BLANK should keep datetime fields as blank string values, not undefined values");
    }
    if (after_blank_birth != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_blank_birth->second).empty(),
            "blank GATHER should round-trip to an empty runtime date value");
    }
    if (after_blank_stamp != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_blank_stamp->second).empty(),
            "blank GATHER should round-trip to an empty runtime datetime value");
    }

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "date/datetime GATHER MEMVAR destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty() && persisted.table.records[0U].values.size() >= 3U) {
        expect(persisted.table.records[0U].values[1U].display_value.empty(),
            "blank GATHER should persist a blank D field");
        expect(persisted.table.records[0U].values[2U].display_value == "julian:0 millis:0",
            "blank GATHER should persist zero datetime storage for T fields");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_array_preserves_date_and_datetime_like_values() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_array_dates";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BIRTHDAY", .type = 'D', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U},
    };
    const std::vector<std::vector<std::string>> records{
        {"20260418", "julian:2460447 millis:49556000"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "date/datetime scatter-gather array fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_array_dates.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS BIRTHDAY, STAMP TO aRow\n"
        "cArrayBirth = aRow[1]\n"
        "cArrayStamp = aRow[2]\n"
        "aRow[1] = '04/20/2026'\n"
        "aRow[2] = '04/20/2026 07:08:09'\n"
        "GATHER FROM aRow FIELDS BIRTHDAY, STAMP\n"
        "cAfterArrayBirth = DTOC(BIRTHDAY, 1)\n"
        "cAfterArrayStamp = TTOC(STAMP, 1)\n"
        "SCATTER FIELDS BIRTHDAY, STAMP TO aBlank BLANK\n"
        "cBlankArrayBirthType = VARTYPE(aBlank[1])\n"
        "cBlankArrayStampType = VARTYPE(aBlank[2])\n"
        "GATHER FROM aBlank FIELDS BIRTHDAY, STAMP\n"
        "SCATTER FIELDS BIRTHDAY, STAMP TO aAfterBlank\n"
        "cAfterBlankArrayBirth = aAfterBlank[1]\n"
        "cAfterBlankArrayStamp = aAfterBlank[2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "date/datetime SCATTER/GATHER array script should complete");

    const auto array_birth = state.globals.find("carraybirth");
    const auto array_stamp = state.globals.find("carraystamp");
    const auto after_array_birth = state.globals.find("cafterarraybirth");
    const auto after_array_stamp = state.globals.find("cafterarraystamp");
    const auto blank_array_birth_type = state.globals.find("cblankarraybirthtype");
    const auto blank_array_stamp_type = state.globals.find("cblankarraystamptype");
    const auto after_blank_array_birth = state.globals.find("cafterblankarraybirth");
    const auto after_blank_array_stamp = state.globals.find("cafterblankarraystamp");

    expect(array_birth != state.globals.end(), "SCATTER TO array should expose date fields");
    expect(array_stamp != state.globals.end(), "SCATTER TO array should expose datetime fields");
    expect(after_array_birth != state.globals.end(), "GATHER FROM array should restore updated date fields");
    expect(after_array_stamp != state.globals.end(), "GATHER FROM array should restore updated datetime fields");
    expect(blank_array_birth_type != state.globals.end(), "SCATTER TO array BLANK should still define blank date elements");
    expect(blank_array_stamp_type != state.globals.end(), "SCATTER TO array BLANK should still define blank datetime elements");
    expect(after_blank_array_birth != state.globals.end(), "blank array GATHER should leave a readable blank date field");
    expect(after_blank_array_stamp != state.globals.end(), "blank array GATHER should leave a readable blank datetime field");

    if (array_birth != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_birth->second) == "04/18/2026",
            "SCATTER TO array should expose dates in runtime date format");
    }
    if (array_stamp != state.globals.end()) {
        expect(copperfin::runtime::format_value(array_stamp->second) == "04/18/2026 13:45:56",
            "SCATTER TO array should expose datetimes in runtime datetime format");
    }
    if (after_array_birth != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(after_array_birth->second);
        expect(actual == "20260420",
            "GATHER FROM array should serialize runtime date strings back into the date field (got '" + actual + "')");
    }
    if (after_array_stamp != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(after_array_stamp->second);
        expect(actual == "20260420070809",
            "GATHER FROM array should serialize runtime datetime strings back into the datetime field (got '" + actual + "')");
    }
    if (blank_array_birth_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_array_birth_type->second) == "C",
            "SCATTER TO array BLANK should keep date values as blank strings");
    }
    if (blank_array_stamp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_array_stamp_type->second) == "C",
            "SCATTER TO array BLANK should keep datetime values as blank strings");
    }
    if (after_blank_array_birth != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_blank_array_birth->second).empty(),
            "blank array GATHER should round-trip to an empty runtime date value");
    }
    if (after_blank_array_stamp != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_blank_array_stamp->second).empty(),
            "blank array GATHER should round-trip to an empty runtime datetime value");
    }

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "date/datetime GATHER FROM array destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty() && persisted.table.records[0U].values.size() >= 2U) {
        expect(persisted.table.records[0U].values[0U].display_value.empty(),
            "blank array GATHER should persist a blank D field");
        expect(persisted.table.records[0U].values[1U].display_value == "julian:0 millis:0",
            "blank array GATHER should persist zero datetime storage for T fields");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_object_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_object";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME, AGE NAME oRow\n"
        "cObjName = GETPEM(oRow, 'NAME')\n"
        "nObjAgePlus = GETPEM(oRow, 'AGE') + 1\n"
        "=SETPem(oRow, 'NAME', 'FromObject')\n"
        "=SETPem(oRow, 'AGE', 77)\n"
        "GATHER NAME oRow FIELDS NAME, AGE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER NAME script should complete: " + state.message);

    const auto obj_name = state.globals.find("cobjname");
    const auto obj_age_plus = state.globals.find("nobjageplus");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");

    expect(obj_name != state.globals.end(), "SCATTER NAME should populate NAME as an object property");
    expect(obj_age_plus != state.globals.end(), "SCATTER NAME should preserve numeric properties");
    expect(after_name != state.globals.end(), "GATHER NAME should restore NAME from object properties");
    expect(after_age != state.globals.end(), "GATHER NAME should restore AGE from object properties");

    if (obj_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(obj_name->second) == "Alice",
            "SCATTER NAME should expose NAME via GETPEM");
    }
    if (obj_age_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(obj_age_plus->second) == "43",
            "SCATTER NAME should preserve numeric values for arithmetic");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "FromObject",
            "GATHER NAME should write updated NAME back to the record");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "77",
            "GATHER NAME should write updated AGE back to the record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_name_additive_merges_existing_object_properties() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_name_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER NAME ADDITIVE fixture should be created");

    const fs::path main_path = temp_root / "scatter_name_additive.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "=ADDPROPERTY(oRow, 'NAME', 'OldName')\n"
        "cBeforeExtra = GETPEM(oRow, 'EXTRA')\n"
        "cBeforeName = GETPEM(oRow, 'NAME')\n"
        "SCATTER FIELDS NAME, AGE NAME oRow ADDITIVE\n"
        "cAfterExtra = GETPEM(oRow, 'EXTRA')\n"
        "cAfterName = GETPEM(oRow, 'NAME')\n"
        "nAfterAge = GETPEM(oRow, 'AGE')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER NAME ADDITIVE script should complete: " + state.message);

    const auto before_extra = state.globals.find("cbeforeextra");
    const auto before_name = state.globals.find("cbeforename");
    const auto after_extra = state.globals.find("cafterextra");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");

    expect(before_extra != state.globals.end(), "existing object property should be readable before SCATTER NAME ADDITIVE");
    expect(before_name != state.globals.end(), "existing object field property should be readable before SCATTER NAME ADDITIVE");
    expect(after_extra != state.globals.end(), "SCATTER NAME ADDITIVE should preserve unrelated existing properties");
    expect(after_name != state.globals.end(), "SCATTER NAME ADDITIVE should refresh matching properties from record fields");
    expect(after_age != state.globals.end(), "SCATTER NAME ADDITIVE should add missing field properties");

    if (before_extra != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_extra->second) == "KeepMe",
            "seeded EXTRA property should be present before additive scatter");
    }
    if (before_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(before_name->second) == "OldName",
            "seeded NAME property should be present before additive scatter");
    }
    if (after_extra != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_extra->second) == "KeepMe",
            "SCATTER NAME ADDITIVE should preserve unrelated existing properties");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "SCATTER NAME ADDITIVE should overwrite matching properties with current record values");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "SCATTER NAME ADDITIVE should add missing field properties from the current record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_single_name_field_filter_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "single NAME-field SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_field.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oRow, 'AGE', 900)\n"
        "SCATTER FIELDS NAME NAME oRow ADDITIVE\n"
        "cScatteredName = GETPEM(oRow, 'NAME')\n"
        "nScatteredAge = GETPEM(oRow, 'AGE')\n"
        "=SETPem(oRow, 'NAME', 'NameOnly')\n"
        "=SETPem(oRow, 'AGE', 901)\n"
        "GATHER NAME oRow FIELDS NAME\n"
        "cAfterGatherName = NAME\n"
        "SCATTER FIELDS AGE MEMVAR\n"
        "nAfterGatherAge = m.AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "single NAME-field SCATTER/GATHER NAME script should complete: " + state.message);

    const auto scattered_name = state.globals.find("cscatteredname");
    const auto scattered_age = state.globals.find("nscatteredage");
    const auto after_gather_name = state.globals.find("caftergathername");
    const auto after_gather_age = state.globals.find("naftergatherage");

    expect(scattered_name != state.globals.end(), "SCATTER FIELDS NAME NAME oRow should populate the NAME property");
    expect(scattered_age != state.globals.end(), "SCATTER FIELDS NAME NAME oRow ADDITIVE should preserve preexisting AGE");
    expect(after_gather_name != state.globals.end(), "GATHER NAME oRow FIELDS NAME should update NAME");
    expect(after_gather_age != state.globals.end(), "GATHER NAME oRow FIELDS NAME should leave AGE readable");

    if (scattered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(scattered_name->second) == "Alice",
            "SCATTER FIELDS NAME NAME oRow should treat NAME as a selected field, not as the clause boundary");
    }
    if (scattered_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(scattered_age->second) == "900",
            "SCATTER FIELDS NAME NAME oRow ADDITIVE should preserve non-selected object properties");
    }
    if (after_gather_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather_name->second) == "NameOnly",
            "GATHER NAME oRow FIELDS NAME should write back only the NAME property");
    }
    if (after_gather_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_gather_age->second) == "42",
            "GATHER NAME oRow FIELDS NAME should leave AGE unchanged");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER/GATHER NAME LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oRow, 'AGE', 900)\n"
        "SCATTER FIELDS LIKE N* NAME oRow ADDITIVE\n"
        "cLikeName = GETPEM(oRow, 'NAME')\n"
        "cLikeNote = GETPEM(oRow, 'NOTE')\n"
        "nLikeAge = GETPEM(oRow, 'AGE')\n"
        "=SETPem(oRow, 'NAME', 'LikeName')\n"
        "=SETPem(oRow, 'NOTE', 'LikeNote')\n"
        "=SETPem(oRow, 'AGE', 901)\n"
        "GATHER NAME oRow FIELDS EXCEPT AGE\n"
        "cAfterName = NAME\n"
        "cAfterNote = NOTE\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER NAME LIKE/EXCEPT script should complete: " + state.message);

    const auto like_name = state.globals.find("clikename");
    const auto like_note = state.globals.find("clikenote");
    const auto like_age = state.globals.find("nlikeage");
    const auto after_name = state.globals.find("caftername");
    const auto after_note = state.globals.find("cafternote");
    const auto after_age = state.globals.find("nafterage");

    expect(like_name != state.globals.end(), "SCATTER FIELDS LIKE N* NAME should populate NAME");
    expect(like_note != state.globals.end(), "SCATTER FIELDS LIKE N* NAME should populate NOTE");
    expect(like_age != state.globals.end(), "SCATTER FIELDS LIKE N* NAME ADDITIVE should preserve preexisting AGE");
    expect(after_name != state.globals.end(), "GATHER NAME FIELDS EXCEPT AGE should update NAME");
    expect(after_note != state.globals.end(), "GATHER NAME FIELDS EXCEPT AGE should update NOTE");
    expect(after_age != state.globals.end(), "GATHER NAME FIELDS EXCEPT AGE should leave AGE readable");

    if (like_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(like_name->second) == "Alice",
            "SCATTER FIELDS LIKE N* NAME should include keyword-heavy NAME");
    }
    if (like_note != state.globals.end()) {
        expect(copperfin::runtime::format_value(like_note->second) == "Ready",
            "SCATTER FIELDS LIKE N* NAME should include adjacent NOTE");
    }
    if (like_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(like_age->second) == "900",
            "SCATTER FIELDS LIKE N* NAME ADDITIVE should preserve excluded properties");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "LikeName",
            "GATHER NAME FIELDS EXCEPT AGE should write NAME back");
    }
    if (after_note != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_note->second) == "LikeNote",
            "GATHER NAME FIELDS EXCEPT AGE should write NOTE back");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "GATHER NAME FIELDS EXCEPT AGE should leave AGE unchanged");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_supports_macro_object_variable_names() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_macro";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "macro SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_macro.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "cObjectName = 'oRow'\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "SCATTER FIELDS NAME, AGE NAME &cObjectName ADDITIVE\n"
        "cAfterExtra = GETPEM(oRow, 'EXTRA')\n"
        "cAfterName = GETPEM(oRow, 'NAME')\n"
        "=SETPem(oRow, 'NAME', 'MacroObj')\n"
        "=SETPem(oRow, 'AGE', 55)\n"
        "GATHER NAME &cObjectName FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro SCATTER/GATHER NAME script should complete: " + state.message);

    const auto after_extra = state.globals.find("cafterextra");
    const auto after_name = state.globals.find("caftername");
    const auto gathered_name = state.globals.find("cgatheredname");
    const auto gathered_age = state.globals.find("ngatheredage");

    expect(after_extra != state.globals.end(), "macro SCATTER NAME should preserve existing additive properties");
    expect(after_name != state.globals.end(), "macro SCATTER NAME should populate matching field properties");
    expect(gathered_name != state.globals.end(), "macro GATHER NAME should write NAME back to the record");
    expect(gathered_age != state.globals.end(), "macro GATHER NAME should write AGE back to the record");

    if (after_extra != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_extra->second) == "KeepMe",
            "macro SCATTER NAME ADDITIVE should preserve unrelated existing properties");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "macro SCATTER NAME should resolve the target object variable before populating field properties");
    }
    if (gathered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_name->second) == "MacroObj",
            "macro GATHER NAME should resolve the source object variable before restoring field values");
    }
    if (gathered_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_age->second) == "55",
            "macro GATHER NAME should preserve numeric object properties");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_supports_nested_object_targets() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_nested";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "nested SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_nested.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oHolder = CREATEOBJECT('Empty')\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oHolder, 'Row', oRow)\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "SCATTER FIELDS NAME, AGE NAME oHolder.Row ADDITIVE\n"
        "cAfterExtra = GETPEM(GETPEM(oHolder, 'Row'), 'EXTRA')\n"
        "cAfterName = GETPEM(GETPEM(oHolder, 'Row'), 'NAME')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'NAME', 'NestedObj')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'AGE', 61)\n"
        "GATHER NAME oHolder.Row FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "nested SCATTER/GATHER NAME script should complete: " + state.message);

    const auto after_extra = state.globals.find("cafterextra");
    const auto after_name = state.globals.find("caftername");
    const auto gathered_name = state.globals.find("cgatheredname");
    const auto gathered_age = state.globals.find("ngatheredage");

    expect(after_extra != state.globals.end(), "nested SCATTER NAME should preserve additive child-object properties");
    expect(after_name != state.globals.end(), "nested SCATTER NAME should populate child-object field properties");
    expect(gathered_name != state.globals.end(), "nested GATHER NAME should write NAME back from the child object");
    expect(gathered_age != state.globals.end(), "nested GATHER NAME should write AGE back from the child object");

    if (after_extra != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_extra->second) == "KeepMe",
            "nested SCATTER NAME ADDITIVE should preserve unrelated child-object properties");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "nested SCATTER NAME should resolve object-property targets before populating field properties");
    }
    if (gathered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_name->second) == "NestedObj",
            "nested GATHER NAME should restore field values from the child object target");
    }
    if (gathered_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_age->second) == "61",
            "nested GATHER NAME should preserve numeric child-object properties");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_supports_macro_expanded_nested_property_segments() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_macro_nested_segment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "macro nested-segment SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_macro_nested_segment.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "cHolder = 'oHolder'\n"
        "cChild = 'Row'\n"
        "oHolder = CREATEOBJECT('Empty')\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oHolder, 'Row', oRow)\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "SCATTER FIELDS NAME, AGE NAME &cHolder.&cChild ADDITIVE\n"
        "cAfterExtra = GETPEM(GETPEM(oHolder, 'Row'), 'EXTRA')\n"
        "cAfterName = GETPEM(GETPEM(oHolder, 'Row'), 'NAME')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'NAME', 'MacroNest')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'AGE', 63)\n"
        "GATHER NAME &cHolder.&cChild FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro nested-segment SCATTER/GATHER NAME script should complete: " + state.message);

    const auto after_extra = state.globals.find("cafterextra");
    const auto after_name = state.globals.find("caftername");
    const auto gathered_name = state.globals.find("cgatheredname");
    const auto gathered_age = state.globals.find("ngatheredage");

    expect(after_extra != state.globals.end(), "macro nested-segment SCATTER NAME should preserve additive properties");
    expect(after_name != state.globals.end(), "macro nested-segment SCATTER NAME should populate field properties");
    expect(gathered_name != state.globals.end(), "macro nested-segment GATHER NAME should restore NAME");
    expect(gathered_age != state.globals.end(), "macro nested-segment GATHER NAME should restore AGE");

    if (after_extra != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_extra->second) == "KeepMe",
            "macro nested-segment SCATTER NAME ADDITIVE should preserve unrelated child-object properties");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "macro nested-segment SCATTER NAME should expand object-path segments before populating fields");
    }
    if (gathered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_name->second) == "MacroNest",
            "macro nested-segment GATHER NAME should read back from the expanded nested target");
    }
    if (gathered_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_age->second) == "63",
            "macro nested-segment GATHER NAME should preserve numeric values on the expanded nested target");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_name_creates_missing_nested_object_targets() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_nested_create";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "missing nested SCATTER/GATHER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_nested_create.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oHolder = CREATEOBJECT('Empty')\n"
        "SCATTER FIELDS NAME, AGE NAME oHolder.Row ADDITIVE\n"
        "cAfterName = GETPEM(GETPEM(oHolder, 'Row'), 'NAME')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'NAME', 'BuiltChild')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'AGE', 62)\n"
        "GATHER NAME oHolder.Row FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "missing nested SCATTER/GATHER NAME script should complete: " + state.message);

    const auto after_name = state.globals.find("caftername");
    const auto gathered_name = state.globals.find("cgatheredname");
    const auto gathered_age = state.globals.find("ngatheredage");

    expect(after_name != state.globals.end(), "SCATTER NAME should create and populate a missing nested object target");
    expect(gathered_name != state.globals.end(), "GATHER NAME should write NAME back from a created nested object target");
    expect(gathered_age != state.globals.end(), "GATHER NAME should write AGE back from a created nested object target");

    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "SCATTER NAME should populate the newly created nested object target");
    }
    if (gathered_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_name->second) == "BuiltChild",
            "GATHER NAME should restore field values from the newly created nested object target");
    }
    if (gathered_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(gathered_age->second) == "62",
            "GATHER NAME should preserve numeric values on the newly created nested object target");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_name_without_additive_replaces_existing_nested_target_object() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_name_replace_nested";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "non-additive nested SCATTER NAME fixture should be created");

    const fs::path main_path = temp_root / "scatter_name_replace_nested.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "oHolder = CREATEOBJECT('Empty')\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oHolder, 'Row', oRow)\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "=ADDPROPERTY(oRow, 'NAME', 'OldName')\n"
        "SCATTER FIELDS NAME, AGE NAME oHolder.Row\n"
        "cAfterName = GETPEM(GETPEM(oHolder, 'Row'), 'NAME')\n"
        "nAfterAge = GETPEM(GETPEM(oHolder, 'Row'), 'AGE')\n"
        "lExtraExists = PEMSTATUS(GETPEM(oHolder, 'Row'), 'EXTRA', 1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "non-additive nested SCATTER NAME script should complete: " + state.message);

    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");
    const auto extra_exists = state.globals.find("lextraexists");

    expect(after_name != state.globals.end(), "non-additive nested SCATTER NAME should populate NAME on the replacement object");
    expect(after_age != state.globals.end(), "non-additive nested SCATTER NAME should populate AGE on the replacement object");
    expect(extra_exists != state.globals.end(), "non-additive nested SCATTER NAME should expose whether stale properties survived replacement");

    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "non-additive nested SCATTER NAME should overwrite NAME with current record data");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "non-additive nested SCATTER NAME should populate AGE on the replacement object");
    }
    if (extra_exists != state.globals.end()) {
        expect(copperfin::runtime::format_value(extra_exists->second) == "false",
            "non-additive nested SCATTER NAME should replace the target object instead of preserving unrelated stale properties");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_predeclared_2d_array_row_one_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_2d_row";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42", "true"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "2D SCATTER/GATHER fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_2d_row.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "DIMENSION aRow[1,3]\n"
        "SCATTER FIELDS NAME, AGE, ACTIVE TO aRow\n"
        "cRowName = aRow[1,1]\n"
        "nRowAgePlus = aRow[1,2] + 1\n"
        "lRowActive = aRow[1,3]\n"
        "aRow[1,1] = 'TwoD'\n"
        "aRow[1,2] = 99\n"
        "aRow[1,3] = .F.\n"
        "GATHER FROM aRow FIELDS NAME, AGE, ACTIVE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "lAfterActive = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "2D row-1 SCATTER/GATHER script should complete: " + state.message);

    const auto row_name = state.globals.find("crowname");
    const auto row_age_plus = state.globals.find("nrowageplus");
    const auto row_active = state.globals.find("lrowactive");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");
    const auto after_active = state.globals.find("lafteractive");

    expect(row_name != state.globals.end(), "SCATTER TO predeclared 2D array should populate row 1 col 1");
    expect(row_age_plus != state.globals.end(), "SCATTER TO predeclared 2D array should keep numeric values");
    expect(row_active != state.globals.end(), "SCATTER TO predeclared 2D array should populate logical values");
    expect(after_name != state.globals.end(), "GATHER FROM predeclared 2D array should restore NAME");
    expect(after_age != state.globals.end(), "GATHER FROM predeclared 2D array should restore AGE");
    expect(after_active != state.globals.end(), "GATHER FROM predeclared 2D array should restore ACTIVE");

    if (row_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(row_name->second) == "Alice",
            "2D SCATTER should write field values to row 1 columns");
    }
    if (row_age_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(row_age_plus->second) == "43",
            "2D SCATTER should preserve numeric values");
    }
    if (row_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(row_active->second) == "true",
            "2D SCATTER should preserve logical values");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "TwoD",
            "2D GATHER should write NAME from row 1 col 1");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "99",
            "2D GATHER should write AGE from row 1 col 2");
    }
    if (after_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_active->second) == "false",
            "2D GATHER should write ACTIVE from row 1 col 3");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_two_column_name_value_array_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_name_value_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "42", "true"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "name/value SCATTER/GATHER fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_name_value_array.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "DIMENSION aPair[3,2]\n"
        "SCATTER FIELDS NAME, AGE, ACTIVE TO aPair\n"
        "cPairField1 = aPair[1,1]\n"
        "cPairValue1 = aPair[1,2]\n"
        "cPairField2 = aPair[2,1]\n"
        "nPairValue2 = aPair[2,2]\n"
        "aPair[1,2] = 'PairName'\n"
        "aPair[2,2] = 66\n"
        "aPair[3,2] = .F.\n"
        "GATHER FROM aPair FIELDS NAME, AGE, ACTIVE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "lAfterActive = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "2-column name/value SCATTER/GATHER script should complete: " + state.message);

    const auto pair_field_1 = state.globals.find("cpairfield1");
    const auto pair_value_1 = state.globals.find("cpairvalue1");
    const auto pair_field_2 = state.globals.find("cpairfield2");
    const auto pair_value_2 = state.globals.find("npairvalue2");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");
    const auto after_active = state.globals.find("lafteractive");

    expect(pair_field_1 != state.globals.end(), "SCATTER TO [n,2] array should write field names in column 1");
    expect(pair_value_1 != state.globals.end(), "SCATTER TO [n,2] array should write field values in column 2");
    expect(pair_field_2 != state.globals.end(), "SCATTER TO [n,2] array should keep field order");
    expect(pair_value_2 != state.globals.end(), "SCATTER TO [n,2] array should preserve numeric values");
    expect(after_name != state.globals.end(), "GATHER FROM [n,2] array should restore NAME");
    expect(after_age != state.globals.end(), "GATHER FROM [n,2] array should restore AGE");
    expect(after_active != state.globals.end(), "GATHER FROM [n,2] array should restore ACTIVE");

    if (pair_field_1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(pair_field_1->second) == "NAME",
            "name/value SCATTER should store first field name in column 1");
    }
    if (pair_value_1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(pair_value_1->second) == "Alice",
            "name/value SCATTER should store first field value in column 2");
    }
    if (pair_field_2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(pair_field_2->second) == "AGE",
            "name/value SCATTER should store second field name in column 1");
    }
    if (pair_value_2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(pair_value_2->second) == "42",
            "name/value SCATTER should preserve numeric field values in column 2");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "PairName",
            "name/value GATHER should restore NAME by matching field names");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "66",
            "name/value GATHER should restore AGE by matching field names");
    }
    if (after_active != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_active->second) == "false",
            "name/value GATHER should restore ACTIVE by matching field names");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_memo_clause_controls_memo_field_inclusion() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_memo_clause";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "notes.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTES", .type = 'M', .length = 10U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Memo payload"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER MEMO fixture should be created");

    const fs::path main_path = temp_root / "scatter_memo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "cNoMemoType = VARTYPE(m.NOTES)\n"
        "SCATTER MEMVAR MEMO\n"
        "cWithMemoType = VARTYPE(m.NOTES)\n"
        "cWithMemoValue = m.NOTES\n"
        "SCATTER TO aNoMemo\n"
        "nNoMemoLen = ALEN(aNoMemo)\n"
        "SCATTER TO aWithMemo MEMO\n"
        "nWithMemoLen = ALEN(aWithMemo)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER MEMO script should complete: " + state.message);

    const auto no_memo_type = state.globals.find("cnomemotype");
    const auto with_memo_type = state.globals.find("cwithmemotype");
    const auto with_memo_value = state.globals.find("cwithmemovalue");
    const auto no_memo_len = state.globals.find("nnomemolen");
    const auto with_memo_len = state.globals.find("nwithmemolen");

    expect(no_memo_type != state.globals.end(), "SCATTER MEMVAR without MEMO should leave memo memvar undefined");
    expect(with_memo_type != state.globals.end(), "SCATTER MEMVAR MEMO should include memo memvar");
    expect(with_memo_value != state.globals.end(), "SCATTER MEMVAR MEMO should capture memo field value");
    expect(no_memo_len != state.globals.end(), "SCATTER TO array without MEMO should return array length");
    expect(with_memo_len != state.globals.end(), "SCATTER TO array MEMO should return array length");

    if (no_memo_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(no_memo_type->second) == "U",
            "SCATTER without MEMO should not include memo fields");
    }
    if (with_memo_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(with_memo_type->second) == "C",
            "SCATTER MEMO should expose memo fields as string values");
    }
    if (with_memo_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(with_memo_value->second) == "Memo payload",
            "SCATTER MEMO should preserve memo field text");
    }
    if (no_memo_len != state.globals.end() && with_memo_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(no_memo_len->second) == "1",
            "SCATTER TO array without MEMO should include non-memo fields only");
        expect(copperfin::runtime::format_value(with_memo_len->second) == "2",
            "SCATTER TO array MEMO should include memo fields");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_array_mutator_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_mutators";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "tools.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "FIRST", .type = 'C', .length = 10U},
        {.name = "SECOND", .type = 'C', .length = 10U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Zulu", "Alpha"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "array mutator DBF fixture should be created");

    const fs::path main_path = temp_root / "array_mutators.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "SCATTER FIELDS FIRST, SECOND TO aTools\n"
        "nScanAlpha = ASCAN(aTools, 'Alpha')\n"
        "nScanAlphaInsensitive = ASCAN(aTools, 'alpha', -1, -1, -1, 1)\n"
        "nSortOk = ASORT(aTools)\n"
        "cSortedFirst = aTools[1]\n"
        "cSortedSecond = aTools[2]\n"
        "nDeleteOk = ADEL(aTools, 1)\n"
        "cAfterDeleteFirst = aTools[1]\n"
        "nInsertOk = AINS(aTools, 1)\n"
        "cAfterInsertFirstType = VARTYPE(aTools[1])\n"
        "cAfterInsertSecond = aTools[2]\n"
        "nResize = ASIZE(aTools, 4)\n"
        "nLenAfterResize = ALEN(aTools)\n"
        "cPreservedAfterResize = aTools[2]\n"
        "DIMENSION aSource[2,3], aTarget[2,3], aFlat[1]\n"
        "aSource[1,1] = 'A'\n"
        "aSource[1,2] = 'B'\n"
        "aSource[1,3] = 'C'\n"
        "aSource[2,1] = 'D'\n"
        "aSource[2,2] = 'E'\n"
        "aSource[2,3] = 'F'\n"
        "DIMENSION aScanFlags[1]\n"
        "aScanFlags[1] = 'Product'\n"
        "SET EXACT ON\n"
        "nScanProdExactDefault = ASCAN(aScanFlags, 'Prod')\n"
        "nScanProdExactOff = ASCAN(aScanFlags, 'Prod', -1, -1, -1, 4)\n"
        "nScanProductInsensitiveExact = ASCAN(aScanFlags, 'product', -1, -1, -1, 7)\n"
        "SET EXACT OFF\n"
        "nScanDFromSecond = ASCAN(aSource, 'D', 2)\n"
        "nScanAFromSecond = ASCAN(aSource, 'A', 2)\n"
        "nScanCWindow = ASCAN(aSource, 'C', 2, 2)\n"
        "nScanDWindowMiss = ASCAN(aSource, 'D', 2, 2)\n"
        "nScanEColumnElement = ASCAN(aSource, 'E', -1, -1, 2, 0)\n"
        "nScanEColumnRow = ASCAN(aSource, 'E', -1, -1, 2, 8)\n"
        "nElement = AELEMENT(aSource, 2, 2)\n"
        "nElementRow = ASUBSCRIPT(aSource, nElement, 1)\n"
        "nElementColumn = ASUBSCRIPT(aSource, nElement, 2)\n"
        "nCopyWindow = ACOPY(aSource, aTarget, 2, 3, 3)\n"
        "cTargetOne = aTarget[1,3]\n"
        "cTargetTwo = aTarget[2,1]\n"
        "cTargetThree = aTarget[2,2]\n"
        "nCopyAll = ACOPY(aSource, aFlat)\n"
        "nFlatLen = ALEN(aFlat)\n"
        "cFlatSix = aFlat[6]\n"
        "DIMENSION aNumericSort[3]\n"
        "aNumericSort[1] = 10\n"
        "aNumericSort[2] = 2\n"
        "aNumericSort[3] = 1\n"
        "nSortNumeric = ASORT(aNumericSort)\n"
        "nSortNumericOne = aNumericSort[1]\n"
        "nSortNumericTwo = aNumericSort[2]\n"
        "nSortNumericThree = aNumericSort[3]\n"
        "DIMENSION aSortWindow[5]\n"
        "aSortWindow[1] = 'A'\n"
        "aSortWindow[2] = 'B'\n"
        "aSortWindow[3] = 'D'\n"
        "aSortWindow[4] = 'C'\n"
        "aSortWindow[5] = 'E'\n"
        "nSortWindow = ASORT(aSortWindow, 2, 3, 1)\n"
        "cSortWindowOne = aSortWindow[1]\n"
        "cSortWindowTwo = aSortWindow[2]\n"
        "cSortWindowThree = aSortWindow[3]\n"
        "cSortWindowFour = aSortWindow[4]\n"
        "cSortWindowFive = aSortWindow[5]\n"
        "DIMENSION aRows[3,2]\n"
        "aRows[1,1] = 'G'\n"
        "aRows[1,2] = 'A'\n"
        "aRows[2,1] = 'C'\n"
        "aRows[2,2] = 'Z'\n"
        "aRows[3,1] = 'B'\n"
        "aRows[3,2] = 'N'\n"
        "nSortRowsByFirst = ASORT(aRows, 1)\n"
        "cRowsFirstSortRow1Col1 = aRows[1,1]\n"
        "cRowsFirstSortRow1Col2 = aRows[1,2]\n"
        "nSortRowsBySecondFromRow2 = ASORT(aRows, 4)\n"
        "cRowsSecondSortRow1Col1 = aRows[1,1]\n"
        "cRowsSecondSortRow2Col1 = aRows[2,1]\n"
        "cRowsSecondSortRow3Col1 = aRows[3,1]\n"
        "DIMENSION aDeleteRow[3,2]\n"
        "aDeleteRow[1,1] = 'A'\n"
        "aDeleteRow[1,2] = 'B'\n"
        "aDeleteRow[2,1] = 'C'\n"
        "aDeleteRow[2,2] = 'D'\n"
        "aDeleteRow[3,1] = 'E'\n"
        "aDeleteRow[3,2] = 'F'\n"
        "nDeleteRow = ADEL(aDeleteRow, 2)\n"
        "cDeleteRow21 = aDeleteRow[2,1]\n"
        "cDeleteRow22 = aDeleteRow[2,2]\n"
        "cDeleteRow31Type = VARTYPE(aDeleteRow[3,1])\n"
        "DIMENSION aDeleteColumn[2,3]\n"
        "aDeleteColumn[1,1] = 'A'\n"
        "aDeleteColumn[1,2] = 'B'\n"
        "aDeleteColumn[1,3] = 'C'\n"
        "aDeleteColumn[2,1] = 'D'\n"
        "aDeleteColumn[2,2] = 'E'\n"
        "aDeleteColumn[2,3] = 'F'\n"
        "nDeleteColumn = ADEL(aDeleteColumn, 2, 2)\n"
        "cDeleteColumn12 = aDeleteColumn[1,2]\n"
        "cDeleteColumn22 = aDeleteColumn[2,2]\n"
        "cDeleteColumn13Type = VARTYPE(aDeleteColumn[1,3])\n"
        "DIMENSION aInsertRow[3,2]\n"
        "aInsertRow[1,1] = 'A'\n"
        "aInsertRow[1,2] = 'B'\n"
        "aInsertRow[2,1] = 'C'\n"
        "aInsertRow[2,2] = 'D'\n"
        "aInsertRow[3,1] = 'E'\n"
        "aInsertRow[3,2] = 'F'\n"
        "nInsertRow = AINS(aInsertRow, 2)\n"
        "cInsertRow21Type = VARTYPE(aInsertRow[2,1])\n"
        "cInsertRow31 = aInsertRow[3,1]\n"
        "DIMENSION aInsertColumn[2,3]\n"
        "aInsertColumn[1,1] = 'A'\n"
        "aInsertColumn[1,2] = 'B'\n"
        "aInsertColumn[1,3] = 'C'\n"
        "aInsertColumn[2,1] = 'D'\n"
        "aInsertColumn[2,2] = 'E'\n"
        "aInsertColumn[2,3] = 'F'\n"
        "nInsertColumn = AINS(aInsertColumn, 2, 2)\n"
        "cInsertColumn12Type = VARTYPE(aInsertColumn[1,2])\n"
        "cInsertColumn13 = aInsertColumn[1,3]\n"
        "cInsertColumn23 = aInsertColumn[2,3]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "runtime array mutator script should complete");

    const auto scan_alpha = state.globals.find("nscanalpha");
    const auto scan_alpha_insensitive = state.globals.find("nscanalphainsensitive");
    const auto sort_ok = state.globals.find("nsortok");
    const auto sorted_first = state.globals.find("csortedfirst");
    const auto sorted_second = state.globals.find("csortedsecond");
    const auto delete_ok = state.globals.find("ndeleteok");
    const auto after_delete_first = state.globals.find("cafterdeletefirst");
    const auto insert_ok = state.globals.find("ninsertok");
    const auto after_insert_first_type = state.globals.find("cafterinsertfirsttype");
    const auto after_insert_second = state.globals.find("cafterinsertsecond");
    const auto resize = state.globals.find("nresize");
    const auto len_after_resize = state.globals.find("nlenafterresize");
    const auto preserved_after_resize = state.globals.find("cpreservedafterresize");
    const auto scan_prod_exact_default = state.globals.find("nscanprodexactdefault");
    const auto scan_prod_exact_off = state.globals.find("nscanprodexactoff");
    const auto scan_product_insensitive_exact = state.globals.find("nscanproductinsensitiveexact");
    const auto scan_d_from_second = state.globals.find("nscandfromsecond");
    const auto scan_a_from_second = state.globals.find("nscanafromsecond");
    const auto scan_c_window = state.globals.find("nscancwindow");
    const auto scan_d_window_miss = state.globals.find("nscandwindowmiss");
    const auto scan_e_column_element = state.globals.find("nscanecolumnelement");
    const auto scan_e_column_row = state.globals.find("nscanecolumnrow");
    const auto element = state.globals.find("nelement");
    const auto element_row = state.globals.find("nelementrow");
    const auto element_column = state.globals.find("nelementcolumn");
    const auto copy_window = state.globals.find("ncopywindow");
    const auto target_one = state.globals.find("ctargetone");
    const auto target_two = state.globals.find("ctargettwo");
    const auto target_three = state.globals.find("ctargetthree");
    const auto copy_all = state.globals.find("ncopyall");
    const auto flat_len = state.globals.find("nflatlen");
    const auto flat_six = state.globals.find("cflatsix");
    const auto sort_numeric = state.globals.find("nsortnumeric");
    const auto sort_numeric_one = state.globals.find("nsortnumericone");
    const auto sort_numeric_two = state.globals.find("nsortnumerictwo");
    const auto sort_numeric_three = state.globals.find("nsortnumericthree");
    const auto sort_window = state.globals.find("nsortwindow");
    const auto sort_window_one = state.globals.find("csortwindowone");
    const auto sort_window_two = state.globals.find("csortwindowtwo");
    const auto sort_window_three = state.globals.find("csortwindowthree");
    const auto sort_window_four = state.globals.find("csortwindowfour");
    const auto sort_window_five = state.globals.find("csortwindowfive");
    const auto sort_rows_by_first = state.globals.find("nsortrowsbyfirst");
    const auto rows_first_sort_row1_col1 = state.globals.find("crowsfirstsortrow1col1");
    const auto rows_first_sort_row1_col2 = state.globals.find("crowsfirstsortrow1col2");
    const auto sort_rows_by_second_from_row2 = state.globals.find("nsortrowsbysecondfromrow2");
    const auto rows_second_sort_row1_col1 = state.globals.find("crowssecondsortrow1col1");
    const auto rows_second_sort_row2_col1 = state.globals.find("crowssecondsortrow2col1");
    const auto rows_second_sort_row3_col1 = state.globals.find("crowssecondsortrow3col1");
    const auto delete_row = state.globals.find("ndeleterow");
    const auto delete_row_21 = state.globals.find("cdeleterow21");
    const auto delete_row_22 = state.globals.find("cdeleterow22");
    const auto delete_row_31_type = state.globals.find("cdeleterow31type");
    const auto delete_column = state.globals.find("ndeletecolumn");
    const auto delete_column_12 = state.globals.find("cdeletecolumn12");
    const auto delete_column_22 = state.globals.find("cdeletecolumn22");
    const auto delete_column_13_type = state.globals.find("cdeletecolumn13type");
    const auto insert_row = state.globals.find("ninsertrow");
    const auto insert_row_21_type = state.globals.find("cinsertrow21type");
    const auto insert_row_31 = state.globals.find("cinsertrow31");
    const auto insert_column = state.globals.find("ninsertcolumn");
    const auto insert_column_12_type = state.globals.find("cinsertcolumn12type");
    const auto insert_column_13 = state.globals.find("cinsertcolumn13");
    const auto insert_column_23 = state.globals.find("cinsertcolumn23");

    expect(scan_alpha != state.globals.end(), "ASCAN should return a captured position");
    expect(scan_alpha_insensitive != state.globals.end(), "ASCAN should support case-insensitive flags");
    expect(sort_ok != state.globals.end(), "ASORT should return a status");
    expect(sorted_first != state.globals.end(), "ASORT should leave a readable first element");
    expect(sorted_second != state.globals.end(), "ASORT should leave a readable second element");
    expect(delete_ok != state.globals.end(), "ADEL should return a status");
    expect(after_delete_first != state.globals.end(), "ADEL should shift following elements left");
    expect(insert_ok != state.globals.end(), "AINS should return a status");
    expect(after_insert_first_type != state.globals.end(), "AINS should insert an empty slot");
    expect(after_insert_second != state.globals.end(), "AINS should shift existing elements right");
    expect(resize != state.globals.end(), "ASIZE should return the new element count");
    expect(len_after_resize != state.globals.end(), "ALEN should reflect ASIZE result");
    expect(preserved_after_resize != state.globals.end(), "ASIZE should preserve existing values");
    expect(scan_prod_exact_default != state.globals.end(), "ASCAN should respect SET EXACT by default");
    expect(scan_prod_exact_off != state.globals.end(), "ASCAN flags should allow exact-off matching");
    expect(scan_product_insensitive_exact != state.globals.end(), "ASCAN flags should combine case-insensitive and exact matching");
    expect(scan_d_from_second != state.globals.end(), "ASCAN should support a start element");
    expect(scan_a_from_second != state.globals.end(), "ASCAN should not match entries before the start element");
    expect(scan_c_window != state.globals.end(), "ASCAN should support a bounded search window");
    expect(scan_d_window_miss != state.globals.end(), "ASCAN bounded windows should stop before later matches");
    expect(scan_e_column_element != state.globals.end(), "ASCAN should support column-restricted scans");
    expect(scan_e_column_row != state.globals.end(), "ASCAN should optionally return the matched row");
    expect(element != state.globals.end(), "AELEMENT should return a linear element number");
    expect(element_row != state.globals.end(), "ASUBSCRIPT should resolve the element row");
    expect(element_column != state.globals.end(), "ASUBSCRIPT should resolve the element column");
    expect(copy_window != state.globals.end(), "ACOPY should return a copied window count");
    expect(target_one != state.globals.end(), "ACOPY should copy into the requested target element");
    expect(target_two != state.globals.end(), "ACOPY should continue across target rows");
    expect(target_three != state.globals.end(), "ACOPY should copy the full requested window");
    expect(copy_all != state.globals.end(), "ACOPY should copy all remaining source elements by default");
    expect(flat_len != state.globals.end(), "ACOPY should grow a one-dimensional target when needed");
    expect(flat_six != state.globals.end(), "ACOPY should preserve the final copied source element");
    expect(sort_numeric != state.globals.end(), "ASORT should sort numeric arrays using numeric order");
    expect(sort_window != state.globals.end(), "ASORT should sort bounded one-dimensional windows");
    expect(sort_rows_by_first != state.globals.end(), "ASORT should sort two-dimensional arrays by row");
    expect(sort_rows_by_second_from_row2 != state.globals.end(), "ASORT should sort a two-dimensional row subset by start column");
    expect(delete_row != state.globals.end(), "ADEL should delete rows in two-dimensional arrays");
    expect(delete_column != state.globals.end(), "ADEL should delete columns in two-dimensional arrays");
    expect(insert_row != state.globals.end(), "AINS should insert rows in two-dimensional arrays");
    expect(insert_column != state.globals.end(), "AINS should insert columns in two-dimensional arrays");

    if (scan_alpha != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_alpha->second) == "2", "ASCAN should find Alpha in the original second slot");
    }
    if (scan_alpha_insensitive != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_alpha_insensitive->second) == "2", "ASCAN flag 1 should find case-insensitive matches");
    }
    if (sort_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_ok->second) == "1", "ASORT should report success");
    }
    if (sorted_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(sorted_first->second) == "Alpha", "ASORT should sort Alpha first");
    }
    if (sorted_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(sorted_second->second) == "Zulu", "ASORT should sort Zulu second");
    }
    if (delete_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_ok->second) == "1", "ADEL should report success");
    }
    if (after_delete_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_delete_first->second) == "Zulu", "ADEL should shift Zulu into the first slot");
    }
    if (insert_ok != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_ok->second) == "1", "AINS should report success");
    }
    if (after_insert_first_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_insert_first_type->second) == "L", "AINS should insert a false slot");
    }
    if (after_insert_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_insert_second->second) == "Zulu", "AINS should shift Zulu into the second slot");
    }
    if (resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(resize->second) == "4", "ASIZE should report the new element count");
    }
    if (len_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(len_after_resize->second) == "4", "ALEN should reflect ASIZE growth");
    }
    if (preserved_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(preserved_after_resize->second) == "Zulu", "ASIZE should preserve shifted values");
    }
    if (scan_prod_exact_default != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_prod_exact_default->second) == "0", "ASCAN should respect SET EXACT ON without override flags");
    }
    if (scan_prod_exact_off != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_prod_exact_off->second) == "1", "ASCAN flag 4 should allow prefix matches with exact off");
    }
    if (scan_product_insensitive_exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_product_insensitive_exact->second) == "1", "ASCAN flags 1+2+4 should match exactly without case sensitivity");
    }
    if (scan_d_from_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_d_from_second->second) == "4", "ASCAN start element should scan later row-major values");
    }
    if (scan_a_from_second != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_a_from_second->second) == "0", "ASCAN start element should exclude earlier values");
    }
    if (scan_c_window != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_c_window->second) == "3", "ASCAN count should include values inside the requested window");
    }
    if (scan_d_window_miss != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_d_window_miss->second) == "0", "ASCAN count should exclude values after the requested window");
    }
    if (scan_e_column_element != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_e_column_element->second) == "5", "ASCAN should find E in column 2 as element 5");
    }
    if (scan_e_column_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(scan_e_column_row->second) == "2", "ASCAN flag 8 should return the matched row");
    }
    if (element != state.globals.end()) {
        expect(copperfin::runtime::format_value(element->second) == "5", "AELEMENT(aSource, 2, 2) should return row-major element 5");
    }
    if (element_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(element_row->second) == "2", "ASUBSCRIPT(..., 1) should return row 2");
    }
    if (element_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(element_column->second) == "2", "ASUBSCRIPT(..., 2) should return column 2");
    }
    if (copy_window != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_window->second) == "3", "ACOPY should report three copied elements");
    }
    if (target_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_one->second) == "B", "ACOPY should place source element 2 in target element 3");
    }
    if (target_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_two->second) == "C", "ACOPY should place source element 3 in target element 4");
    }
    if (target_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_three->second) == "D", "ACOPY should place source element 4 in target element 5");
    }
    if (copy_all != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_all->second) == "6", "ACOPY without count should copy all source elements");
    }
    if (flat_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_len->second) == "6", "ACOPY should grow a flat target to six elements");
    }
    if (flat_six != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_six->second) == "F", "ACOPY should copy the final element into the grown flat target");
    }
    if (sort_numeric != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_numeric->second) == "1", "ASORT numeric array should report success");
    }
    if (sort_numeric_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_numeric_one->second) == "1", "ASORT numeric array should sort 1 first");
    }
    if (sort_numeric_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_numeric_two->second) == "2", "ASORT numeric array should sort 2 second");
    }
    if (sort_numeric_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_numeric_three->second) == "10", "ASORT numeric array should sort 10 after 2");
    }
    if (sort_window != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window->second) == "1", "ASORT bounded window should report success");
    }
    if (sort_window_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window_one->second) == "A", "ASORT bounded window should preserve earlier elements");
    }
    if (sort_window_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window_two->second) == "D", "ASORT descending bounded window should move D first");
    }
    if (sort_window_three != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window_three->second) == "C", "ASORT descending bounded window should move C second");
    }
    if (sort_window_four != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window_four->second) == "B", "ASORT descending bounded window should move B third");
    }
    if (sort_window_five != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_window_five->second) == "E", "ASORT bounded window should preserve later elements");
    }
    if (sort_rows_by_first != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_rows_by_first->second) == "1", "ASORT two-dimensional row sort should report success");
    }
    if (rows_first_sort_row1_col1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_first_sort_row1_col1->second) == "B", "ASORT should move the row with the lowest first column first");
    }
    if (rows_first_sort_row1_col2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_first_sort_row1_col2->second) == "N", "ASORT should preserve paired columns while moving rows");
    }
    if (sort_rows_by_second_from_row2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(sort_rows_by_second_from_row2->second) == "1", "ASORT two-dimensional subset sort should report success");
    }
    if (rows_second_sort_row1_col1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_second_sort_row1_col1->second) == "B", "ASORT subset sort should preserve rows before the start row");
    }
    if (rows_second_sort_row2_col1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_second_sort_row2_col1->second) == "G", "ASORT subset sort should use the starting element column as key");
    }
    if (rows_second_sort_row3_col1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows_second_sort_row3_col1->second) == "C", "ASORT subset sort should leave the higher key later");
    }
    if (delete_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_row->second) == "1", "ADEL row should report success");
    }
    if (delete_row_21 != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_row_21->second) == "E", "ADEL row should shift later rows up");
    }
    if (delete_row_22 != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_row_22->second) == "F", "ADEL row should preserve shifted row columns");
    }
    if (delete_row_31_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_row_31_type->second) == "L", "ADEL row should fill the last row with false values");
    }
    if (delete_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_column->second) == "1", "ADEL column should report success");
    }
    if (delete_column_12 != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_column_12->second) == "C", "ADEL column should shift later columns left");
    }
    if (delete_column_22 != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_column_22->second) == "F", "ADEL column should shift every row");
    }
    if (delete_column_13_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(delete_column_13_type->second) == "L", "ADEL column should fill the last column with false values");
    }
    if (insert_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_row->second) == "1", "AINS row should report success");
    }
    if (insert_row_21_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_row_21_type->second) == "L", "AINS row should fill inserted row with false values");
    }
    if (insert_row_31 != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_row_31->second) == "C", "AINS row should shift rows down and drop the former last row");
    }
    if (insert_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_column->second) == "1", "AINS column should report success");
    }
    if (insert_column_12_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_column_12_type->second) == "L", "AINS column should fill inserted column with false values");
    }
    if (insert_column_13 != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_column_13->second) == "B", "AINS column should shift row columns right");
    }
    if (insert_column_23 != state.globals.end()) {
        expect(copperfin::runtime::format_value(insert_column_23->second) == "E", "AINS column should shift every row and drop former last column");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_writes_variables_to_file() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_to";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "session_state.mem";
    const fs::path main_path = temp_root / "save_to_test.prg";
    write_text(
        main_path,
        "x = 'hello'\n"
        "nVal = 42\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO script should complete");
    expect(fs::exists(mem_path), "SAVE TO should create a .mem file");

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("x=C:hello") != std::string::npos,
            "SAVE TO should persist character variable x");
        expect(contents.find("nval=N:42") != std::string::npos,
            "SAVE TO should persist numeric variable nVal");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_loads_variables_from_file() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "roundtrip.mem";

    const fs::path save_path = temp_root / "save_source.prg";
    write_text(
        save_path,
        "x = 'hello'\n"
        "nVal = 42\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession save_session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = save_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });
        const auto save_state = save_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(save_state.completed, "SAVE TO setup script should complete before RESTORE FROM test");
    }

    const fs::path restore_path = temp_root / "restore_target.prg";
    write_text(
        restore_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "restored_x = x\n"
        "restored_n = nVal\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession restore_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = restore_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto restore_state = restore_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(restore_state.completed, "RESTORE FROM script should complete");

    const auto restored_x = restore_state.globals.find("restored_x");
    const auto restored_n = restore_state.globals.find("restored_n");
    expect(restored_x != restore_state.globals.end(), "RESTORE FROM should restore character variable x");
    expect(restored_n != restore_state.globals.end(), "RESTORE FROM should restore numeric variable nVal");
    if (restored_x != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_x->second) == "hello",
            "RESTORE FROM should keep x value");
    }
    if (restored_n != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_n->second) == "42",
            "RESTORE FROM should keep nVal value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_additive_merges_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "additive.mem";
    const fs::path main_path = temp_root / "restore_additive.prg";
    write_text(
        main_path,
        "existing = 'kept'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "another = 'new'\n"
        "RESTORE FROM '" + mem_path.string() + "' ADDITIVE\n"
        "after_existing = existing\n"
        "after_another = another\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM ADDITIVE script should complete");

    const auto after_existing = state.globals.find("after_existing");
    const auto after_another = state.globals.find("after_another");
    expect(after_existing != state.globals.end(), "RESTORE FROM ADDITIVE should retain saved existing variable");
    expect(after_another != state.globals.end(), "RESTORE FROM ADDITIVE should retain non-file variable another");
    if (after_existing != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_existing->second) == "kept",
            "RESTORE FROM ADDITIVE should preserve saved existing value");
    }
    if (after_another != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_another->second) == "new",
            "RESTORE FROM ADDITIVE should preserve additive local global variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_like_pattern_filters_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_like";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "filtered.mem";
    const fs::path main_path = temp_root / "save_like.prg";
    write_text(
        main_path,
        "cName = 'Alice'\n"
        "nAge = 30\n"
        "SAVE TO '" + mem_path.string() + "' ALL LIKE c*\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO ALL LIKE script should complete");
    expect(fs::exists(mem_path), "SAVE TO ALL LIKE should create output file");

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cname=C:Alice") != std::string::npos,
            "SAVE TO ALL LIKE should include matching cName variable");
        expect(contents.find("nage=N:30") == std::string::npos,
            "SAVE TO ALL LIKE should exclude non-matching nAge variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_except_pattern_filters_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "filtered.mem";
    const fs::path main_path = temp_root / "save_except.prg";
    write_text(
        main_path,
        "cName = 'Alice'\n"
        "nAge = 30\n"
        "SAVE TO '" + mem_path.string() + "' ALL EXCEPT c*\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO ALL EXCEPT script should complete");
    expect(fs::exists(mem_path), "SAVE TO ALL EXCEPT should create output file");

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cname=C:Alice") == std::string::npos,
            "SAVE TO ALL EXCEPT should exclude matching cName variable");
        expect(contents.find("nage=N:30") != std::string::npos,
            "SAVE TO ALL EXCEPT should include non-matching nAge variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_auto_mem_extension_without_explicit_extension() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_auto_extension";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path base_path = temp_root / "session_state";
    const fs::path mem_path = temp_root / "session_state.mem";
    const fs::path main_path = temp_root / "save_restore_auto_extension.prg";
    write_text(
        main_path,
        "x = 'auto'\n"
        "SAVE TO '" + base_path.string() + "'\n"
        "x = 'changed'\n"
        "RESTORE FROM '" + base_path.string() + "'\n"
        "restored_x = x\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE without explicit extension should complete");
    expect(fs::exists(mem_path), "SAVE TO without extension should append .mem");

    const auto restored_x = state.globals.find("restored_x");
    expect(restored_x != state.globals.end(), "RESTORE FROM without extension should load saved variable");
    if (restored_x != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_x->second) == "auto",
            "RESTORE FROM without extension should read from the auto-appended .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_escaped_string_and_types() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_escaped";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "escaped.mem";
    const fs::path save_path = temp_root / "save_escaped.prg";
    write_text(
        save_path,
        "cEscaped = 'left=right:slash\\' + CHR(10) + 'line2' + CHR(9) + 'tail'\n"
        "lFlag = .T.\n"
        "nAmount = 12.5\n"
        "dStamp = '01/15/2026'\n"
        "PUBLIC eOnly\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    {
        copperfin::runtime::PrgRuntimeSession save_session = copperfin::runtime::PrgRuntimeSession::create({
            .startup_path = save_path.string(),
            .working_directory = temp_root.string(),
            .stop_on_entry = false
        });
        const auto save_state = save_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(save_state.completed, "SAVE TO escaped setup script should complete");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cescaped=C:") != std::string::npos,
            "SAVE TO should persist escaped string variable");
        expect(contents.find("left\\=right\\:") != std::string::npos,
            "SAVE TO should escape delimiter-sensitive characters");
        expect(contents.find("slash\\\\") != std::string::npos,
            "SAVE TO should escape literal backslashes");
        expect(contents.find("\\nline2\\t") != std::string::npos,
            "SAVE TO should escape newline and tab control characters");
        expect(contents.find("dstamp=D:01/15/2026") != std::string::npos,
            "SAVE TO should persist recognized date values using type code D");
        expect(contents.find("eonly=E,PUBLIC:") != std::string::npos,
            "SAVE TO should persist explicit empty PUBLIC values with type code E");
    }

    const fs::path restore_path = temp_root / "restore_escaped.prg";
    write_text(
        restore_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "restored_s = cEscaped\n"
        "restored_l = lFlag\n"
        "restored_n = nAmount\n"
        "restored_d = dStamp\n"
        "restored_e_type = VARTYPE(eOnly)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession restore_session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = restore_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto restore_state = restore_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(restore_state.completed, "RESTORE FROM escaped script should complete");

    const auto restored_s = restore_state.globals.find("restored_s");
    const auto restored_l = restore_state.globals.find("restored_l");
    const auto restored_n = restore_state.globals.find("restored_n");
    const auto restored_d = restore_state.globals.find("restored_d");
    const auto restored_e_type = restore_state.globals.find("restored_e_type");
    const auto restored_empty = restore_state.globals.find("eonly");

    expect(restored_s != restore_state.globals.end(), "RESTORE FROM should restore escaped string variable");
    expect(restored_l != restore_state.globals.end(), "RESTORE FROM should restore logical variable");
    expect(restored_n != restore_state.globals.end(), "RESTORE FROM should restore numeric variable");
    expect(restored_d != restore_state.globals.end(), "RESTORE FROM should restore date variable");
    expect(restored_e_type != restore_state.globals.end(), "RESTORE FROM should restore explicit empty variable");
    expect(restored_empty != restore_state.globals.end(), "RESTORE FROM should materialize explicit empty variables");

    if (restored_s != restore_state.globals.end()) {
        const std::string expected = std::string("left=right:slash\\") + "\n" + "line2" + "\t" + "tail";
        expect(copperfin::runtime::format_value(restored_s->second) == expected,
            "RESTORE FROM should unescape delimiter-sensitive and control characters");
    }
    if (restored_l != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_l->second) == "true",
            "RESTORE FROM should preserve logical type fidelity");
    }
    if (restored_n != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_n->second) == "12.5",
            "RESTORE FROM should preserve numeric type fidelity");
    }
    if (restored_d != restore_state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_d->second) == "01/15/2026",
            "RESTORE FROM should preserve date values");
    }
    if (restored_e_type != restore_state.globals.end()) {
        const std::string restored_type = copperfin::runtime::format_value(restored_e_type->second);
        expect(restored_type == "X" || restored_type == "U",
            "RESTORE FROM should preserve explicit empty value kind");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_rejects_numeric_trailing_garbage() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_numeric_garbage";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "numeric.mem";
    write_text(mem_path, "ngood=N:12.5\nnbad=N:12.5oops\n");

    const fs::path main_path = temp_root / "restore_numeric_garbage.prg";
    write_text(
        main_path,
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_good = nGood\n"
        "after_bad = nBad\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM numeric garbage script should complete");

    const auto after_good = state.globals.find("after_good");
    const auto after_bad = state.globals.find("after_bad");
    expect(after_good != state.globals.end(), "RESTORE FROM should parse valid numeric values");
    expect(after_bad != state.globals.end(), "RESTORE FROM should still materialize invalid numeric entries");
    if (after_good != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_good->second) == "12.5",
            "RESTORE FROM should keep valid numeric values");
    }
    if (after_bad != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_bad->second) == "0",
            "RESTORE FROM should reject numerics with trailing garbage and fall back to 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_prior_globals() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_non_additive";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "state.mem";
    const fs::path main_path = temp_root / "restore_non_additive.prg";
    write_text(
        main_path,
        "from_file = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "stale_only = 'drop-me'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_from_file = from_file\n"
        "after_stale_type = VARTYPE(stale_only)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM non-additive script should complete");

    const auto after_from_file = state.globals.find("after_from_file");
    const auto after_stale_type = state.globals.find("after_stale_type");
    expect(after_from_file != state.globals.end(), "non-additive RESTORE should restore file-backed variables");
    expect(after_stale_type != state.globals.end(), "non-additive RESTORE should allow stale variable type inspection");

    if (after_from_file != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_from_file->second) == "saved",
            "non-additive RESTORE should keep file-backed values");
    }
    if (after_stale_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_stale_type->second) == "U",
            "non-additive RESTORE should clear globals not present in the .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_honors_current_frame_local_bindings() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_locals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "locals.mem";
    const fs::path main_path = temp_root / "restore_locals.prg";
    write_text(
        main_path,
        "saved_value = 'outer'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DO restore_proc\n"
        "after_proc_type = TYPE('saved_value')\n"
        "RETURN\n"
        "PROCEDURE restore_proc\n"
        "LOCAL saved_value\n"
        "saved_value = 'stale'\n"
        "RESTORE FROM '" + mem_path.string() + "' ADDITIVE\n"
        "restored_local = saved_value\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "RESTORE FROM should complete inside a LOCAL frame");

    const auto restored_local = state.globals.find("restored_local");
    const auto after_proc_type = state.globals.find("after_proc_type");
    expect(restored_local != state.globals.end(), "restored local value should be captured");
    expect(after_proc_type != state.globals.end(), "post-procedure TYPE() should be captured");
    if (restored_local != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_local->second) == "outer",
            "RESTORE FROM ADDITIVE should populate a current-frame LOCAL binding instead of a hidden global");
    }
    if (after_proc_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_proc_type->second) == "C",
            "restoring into a LOCAL should not destroy the visible outer global binding");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_arrays() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_arrays";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "arrays.mem";
    const fs::path main_path = temp_root / "save_restore_arrays.prg";
    write_text(
        main_path,
        "DIMENSION aValues[2,2]\n"
        "aValues[1,1] = 'left'\n"
        "aValues[1,2] = 12.5\n"
        "aValues[2,1] = .T.\n"
        "aValues[2,2] = '01/15/2026'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DIMENSION aValues[1]\n"
        "aValues[1] = 'stale'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "restored_type = TYPE('aValues')\n"
        "restored_rows = ALEN(aValues, 1)\n"
        "restored_cols = ALEN(aValues, 2)\n"
        "restored_11 = aValues[1,1]\n"
        "restored_12 = aValues[1,2]\n"
        "restored_21 = aValues[2,1]\n"
        "restored_22 = aValues[2,2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE array roundtrip should complete");

    const auto restored_type = state.globals.find("restored_type");
    const auto restored_rows = state.globals.find("restored_rows");
    const auto restored_cols = state.globals.find("restored_cols");
    const auto restored_11 = state.globals.find("restored_11");
    const auto restored_12 = state.globals.find("restored_12");
    const auto restored_21 = state.globals.find("restored_21");
    const auto restored_22 = state.globals.find("restored_22");

    expect(restored_type != state.globals.end(), "restored array TYPE() should be captured");
    expect(restored_rows != state.globals.end(), "restored array rows should be captured");
    expect(restored_cols != state.globals.end(), "restored array columns should be captured");
    expect(restored_11 != state.globals.end(), "restored array [1,1] should be captured");
    expect(restored_12 != state.globals.end(), "restored array [1,2] should be captured");
    expect(restored_21 != state.globals.end(), "restored array [2,1] should be captured");
    expect(restored_22 != state.globals.end(), "restored array [2,2] should be captured");

    if (restored_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_type->second) == "A",
            "RESTORE FROM should recreate saved arrays");
    }
    if (restored_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_rows->second) == "2",
            "RESTORE FROM should recreate the saved array row count");
    }
    if (restored_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_cols->second) == "2",
            "RESTORE FROM should recreate the saved array column count");
    }
    if (restored_11 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_11->second) == "left",
            "RESTORE FROM should preserve string array elements");
    }
    if (restored_12 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_12->second) == "12.5",
            "RESTORE FROM should preserve numeric array elements");
    }
    if (restored_21 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_21->second) == "true",
            "RESTORE FROM should preserve logical array elements");
    }
    if (restored_22 != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_22->second) == "01/15/2026",
            "RESTORE FROM should preserve date-like array elements");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("avalues=A:2,2|") != std::string::npos,
            "SAVE TO should serialize arrays with dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_restore_round_trips_public_scope() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_restore_public_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "public.mem";
    const fs::path main_path = temp_root / "save_restore_public_scope.prg";
    write_text(
        main_path,
        "PUBLIC cPub, aPub\n"
        "cPub = 'visible'\n"
        "DIMENSION aPub[1]\n"
        "aPub[1] = 'array-visible'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RELEASE ALL\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "PUBLIC pub_scalar_type, pub_array_type, pub_scalar, pub_array, after_release_scalar, after_release_array\n"
        "pub_scalar_type = TYPE('cPub')\n"
        "pub_array_type = TYPE('aPub')\n"
        "pub_scalar = cPub\n"
        "pub_array = aPub[1]\n"
        "RELEASE ALL\n"
        "after_release_scalar = TYPE('cPub')\n"
        "after_release_array = TYPE('aPub')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE/RESTORE public scope roundtrip should complete");

    const auto pub_scalar_type = state.globals.find("pub_scalar_type");
    const auto pub_array_type = state.globals.find("pub_array_type");
    const auto pub_scalar = state.globals.find("pub_scalar");
    const auto pub_array = state.globals.find("pub_array");
    const auto after_release_scalar = state.globals.find("after_release_scalar");
    const auto after_release_array = state.globals.find("after_release_array");

    expect(pub_scalar_type != state.globals.end(), "restored public scalar TYPE() should be captured");
    expect(pub_array_type != state.globals.end(), "restored public array TYPE() should be captured");
    expect(pub_scalar != state.globals.end(), "restored public scalar should be captured");
    expect(pub_array != state.globals.end(), "restored public array element should be captured");
    expect(after_release_scalar != state.globals.end(), "post-release public scalar TYPE() should be captured");
    expect(after_release_array != state.globals.end(), "post-release public array TYPE() should be captured");

    if (pub_scalar_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_scalar_type->second) == "C",
            "RESTORE FROM should recreate saved PUBLIC scalar bindings");
    }
    if (pub_array_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_array_type->second) == "A",
            "RESTORE FROM should recreate saved PUBLIC array bindings");
    }
    if (pub_scalar != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_scalar->second) == "visible",
            "RESTORE FROM should preserve PUBLIC scalar values");
    }
    if (pub_array != state.globals.end()) {
        expect(copperfin::runtime::format_value(pub_array->second) == "array-visible",
            "RESTORE FROM should preserve PUBLIC array values");
    }
    if (after_release_scalar != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_scalar->second) == "C",
            "restored PUBLIC scalars should remain pinned across RELEASE ALL");
    }
    if (after_release_array != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_array->second) == "A",
            "restored PUBLIC arrays should remain pinned across RELEASE ALL");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cpub=C,PUBLIC:visible") != std::string::npos,
            "SAVE TO should persist PUBLIC scalar scope markers");
        expect(contents.find("apub=A,PUBLIC:1,1|") != std::string::npos,
            "SAVE TO should persist PUBLIC array scope markers");
    }

    fs::remove_all(temp_root, ignored);
}

void test_save_to_shadowed_public_name_does_not_persist_public_scope_marker() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_save_shadowed_public_scope";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "shadowed.mem";
    const fs::path main_path = temp_root / "save_shadowed_public_scope.prg";
    write_text(
        main_path,
        "PUBLIC cScope\n"
        "cScope = 'public'\n"
        "DO saver\n"
        "RELEASE ALL\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "PUBLIC restored_scope, after_release_type\n"
        "restored_scope = cScope\n"
        "RELEASE ALL\n"
        "after_release_type = TYPE('cScope')\n"
        "RETURN\n"
        "PROCEDURE saver\n"
        "LOCAL cScope\n"
        "cScope = 'local'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SAVE TO shadowed PUBLIC scope script should complete");

    const auto restored_scope = state.globals.find("restored_scope");
    const auto after_release_type = state.globals.find("after_release_type");
    expect(restored_scope != state.globals.end(), "restored_scope should be captured");
    expect(after_release_type != state.globals.end(), "after_release_type should be captured");

    if (restored_scope != state.globals.end()) {
        expect(copperfin::runtime::format_value(restored_scope->second) == "local",
            "RESTORE FROM should restore the visible shadowed binding value");
    }
    if (after_release_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_release_type->second) == "U",
            "shadowed local bindings saved through SAVE TO should not come back as PUBLIC after RELEASE ALL");
    }

    if (fs::exists(mem_path)) {
        const std::string contents = read_text(mem_path);
        expect(contents.find("cscope=C,PUBLIC:local") == std::string::npos,
            "SAVE TO should not persist a PUBLIC scope marker when the visible binding is a LOCAL shadow");
        expect(contents.find("cscope=C:local") != std::string::npos,
            "SAVE TO should still persist the visible shadowed binding value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_stale_arrays() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_clears_arrays";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "arrays.mem";
    const fs::path main_path = temp_root / "restore_clears_arrays.prg";
    write_text(
        main_path,
        "from_file = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "DIMENSION stale_arr[1]\n"
        "stale_arr[1] = 'drop-me'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "after_stale_array_type = TYPE('stale_arr')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "non-additive RESTORE should clear stale arrays");

    const auto after_stale_array_type = state.globals.find("after_stale_array_type");
    expect(after_stale_array_type != state.globals.end(), "stale array TYPE() after restore should be captured");
    if (after_stale_array_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_stale_array_type->second) == "U",
            "non-additive RESTORE should clear arrays not present in the .mem file");
    }

    fs::remove_all(temp_root, ignored);
}

void test_restore_from_without_additive_clears_private_shadow_state() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_restore_clears_private";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path mem_path = temp_root / "private.mem";
    const fs::path main_path = temp_root / "restore_clears_private.prg";
    write_text(
        main_path,
        "saved_value = 'saved'\n"
        "SAVE TO '" + mem_path.string() + "'\n"
        "outer = 'outer'\n"
        "DO restore_proc\n"
        "after_type = TYPE('outer')\n"
        "RETURN\n"
        "PROCEDURE restore_proc\n"
        "PRIVATE outer\n"
        "outer = 'shadow'\n"
        "RESTORE FROM '" + mem_path.string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "non-additive RESTORE should complete inside PRIVATE shadowing");

    const auto after_type = state.globals.find("after_type");
    expect(after_type != state.globals.end(), "post-restore TYPE() should be captured");
    if (after_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_type->second) == "U",
            "non-additive RESTORE should clear deferred PRIVATE shadow state instead of restoring stale outer bindings");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_emits_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"row1", "row2"});

    const fs::path main_path = temp_root / "copy_to_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO 'dest'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.copy_to";
        });
    expect(has_event, "COPY TO should emit a runtime.copy_to event");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_creates_destination_dbf() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_full";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob", "Carol"});

    const fs::path main_path = temp_root / "copy_to_full.prg";
    const std::string dest_path = (temp_root / "dest.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO full script should complete");
    expect(fs::exists(dest_path), "COPY TO should create destination DBF file");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY TO destination DBF should be readable");
        expect(result.table.records.size() == 3U,
            "COPY TO should copy all 3 records (got " + std::to_string(result.table.records.size()) + ")");
        if (result.table.records.size() >= 1U) {
            const auto& first = result.table.records[0U].values;
            const bool has_alice = !first.empty() && first[0U].display_value.find("Alice") != std::string::npos;
            expect(has_alice, "COPY TO first record should contain Alice");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_structure_to_creates_empty_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_struct";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_struct.prg";
    const std::string dest_path = (temp_root / "schema.dbf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY STRUCTURE TO '" + dest_path + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY STRUCTURE TO script should complete");
    expect(fs::exists(dest_path), "COPY STRUCTURE TO should create destination DBF");

    if (fs::exists(dest_path)) {
        const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path, 100U);
        expect(result.ok, "COPY STRUCTURE TO destination DBF should be readable");
        expect(result.table.records.empty(), "COPY STRUCTURE TO should produce zero rows");
        expect(!result.table.fields.empty(), "COPY STRUCTURE TO should preserve field descriptors");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_copies_records_into_current_table() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create destination with one row; source with two rows
    write_simple_dbf(temp_root / "dest.dbf", {"Alice"});
    write_simple_dbf(temp_root / "source.dbf", {"Bob", "Carol"});

    const fs::path main_path = temp_root / "append_from.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "source.dbf").string() + "'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM script should complete");

    const bool has_event = std::any_of(state.events.begin(), state.events.end(),
        [](const copperfin::runtime::RuntimeEvent& ev) {
            return ev.category == "runtime.append_from";
        });
    expect(has_event, "APPEND FROM should emit a runtime.append_from event");

    // Verify destination now has 3 records (1 original + 2 from source)
    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM destination DBF should be readable after append");
    expect(result.table.records.size() == 3U,
        "APPEND FROM should produce 3 records total (got " + std::to_string(result.table.records.size()) + ")");
    if (result.table.records.size() >= 3U) {
        const bool has_bob = result.table.records[1U].values[0U].display_value.find("Bob") != std::string::npos;
        expect(has_bob, "APPEND FROM second record should be Bob");
        const bool has_carol = result.table.records[2U].values[0U].display_value.find("Carol") != std::string::npos;
        expect(has_carol, "APPEND FROM third record should be Carol");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_sdf_writes_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "source.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "copy_to_sdf.prg";
    const std::string dest_path = (temp_root / "people.sdf").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "COPY TO '" + dest_path + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE SDF script should complete");
    expect(fs::exists(dest_path), "COPY TO TYPE SDF should create the destination text file");
    if (fs::exists(dest_path)) {
        const std::string contents = read_text(dest_path);
        expect(contents == "Alice     \r\nBob       \r\n",
            "COPY TO TYPE SDF should write fixed-width rows using DBF field lengths");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_sdf_imports_fixed_width_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_sdf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.sdf", "Dora      \r\nEvan      \r\n");

    const fs::path main_path = temp_root / "append_from_sdf.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.sdf").string() + "' TYPE SDF\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE SDF script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE SDF destination DBF should be readable");
    expect(result.table.records.size() == 2U,
        "APPEND FROM TYPE SDF should append two rows");
    if (result.table.records.size() >= 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Dora",
            "first SDF row should import into the first DBF record");
        expect(result.table.records[1U].values[0U].display_value == "Evan",
            "second SDF row should import into the second DBF record");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_csv_and_delimited_text_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Ann,Lee", 7}, {"Bob", 42}});

    const fs::path main_path = temp_root / "copy_to_csv.prg";
    const std::string csv_path = (temp_root / "people.csv").string();
    const std::string pipe_path = (temp_root / "people_pipe.txt").string();
    const std::string custom_path = (temp_root / "people_custom.txt").string();
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO '" + csv_path + "' TYPE CSV FIELDS NAME, AGE\n"
        "COPY TO '" + pipe_path + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE FOR AGE > 10\n"
        "COPY TO '" + custom_path + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE FOR AGE > 10\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO TYPE CSV/DELIMITED script should complete");
    expect(fs::exists(csv_path), "COPY TO TYPE CSV should create destination text file");
    expect(fs::exists(pipe_path), "COPY TO DELIMITED should create destination text file");
    expect(fs::exists(custom_path), "COPY TO DELIMITED custom enclosure should create destination text file");

    if (fs::exists(csv_path)) {
        const std::string contents = read_text(csv_path);
        expect(contents == "NAME,AGE\r\n\"Ann,Lee\",7\r\n\"Bob\",42\r\n",
            "COPY TO TYPE CSV should write a field-name header and quote character fields");
    }
    if (fs::exists(pipe_path)) {
        const std::string contents = read_text(pipe_path);
        expect(contents == "\"Bob\"|42\r\n",
            "COPY TO DELIMITED WITH CHARACTER should honor the delimiter and FOR clause");
    }
    if (fs::exists(custom_path)) {
        const std::string contents = read_text(custom_path);
        expect(contents == "_Bob_;42\r\n",
            "COPY TO DELIMITED WITH enclosure plus WITH CHARACTER should honor both VFP options");
    }

    fs::remove_all(temp_root, ignored);
}

void test_append_from_type_csv_imports_delimited_rows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_csv";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "dest.dbf", {});
    write_text(temp_root / "people.csv", "NAME,AGE\r\n\"Ivy, Jr\",9\r\n\"Max\",44\r\n");
    write_text(temp_root / "people_pipe.txt", "\"Nia\"|12\r\n");
    write_text(temp_root / "people_custom.txt", "_Ora_;15\r\n");

    const fs::path main_path = temp_root / "append_from_csv.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM '" + (temp_root / "people.csv").string() + "' TYPE CSV FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_pipe.txt").string() + "' DELIMITED WITH CHARACTER '|' FIELDS NAME, AGE\n"
        "APPEND FROM '" + (temp_root / "people_custom.txt").string() + "' DELIMITED WITH '_' WITH CHARACTER ';' FIELDS NAME, AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM TYPE CSV/DELIMITED script should complete");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(
        (temp_root / "dest.dbf").string(), 100U);
    expect(result.ok, "APPEND FROM TYPE CSV destination DBF should be readable");
    expect(result.table.records.size() == 4U,
        "APPEND FROM TYPE CSV/DELIMITED should append four rows");
    if (result.table.records.size() >= 4U) {
        expect(result.table.records[0U].values[0U].display_value == "Ivy, Jr",
            "first CSV row should preserve comma inside quoted character field");
        expect(result.table.records[0U].values[1U].display_value == "9",
            "first CSV numeric field should import into AGE");
        expect(result.table.records[1U].values[0U].display_value == "Max",
            "second CSV row should import NAME");
        expect(result.table.records[1U].values[1U].display_value == "44",
            "second CSV row should import AGE");
        expect(result.table.records[2U].values[0U].display_value == "Nia",
            "DELIMITED WITH CHARACTER row should import NAME");
        expect(result.table.records[2U].values[1U].display_value == "12",
            "DELIMITED WITH CHARACTER row should import AGE");
        expect(result.table.records[3U].values[0U].display_value == "Ora",
            "DELIMITED custom enclosure row should import NAME");
        expect(result.table.records[3U].values[1U].display_value == "15",
            "DELIMITED custom enclosure row should import AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_tab_and_append_from_type_tab_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_tab_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7"},
        {"Ben", "42"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE TAB source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE TAB destination fixture should be created");

    const std::string tab_path = (temp_root / "people.txt").string();
    const fs::path main_path = temp_root / "tab_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + tab_path + "' TYPE TAB FIELDS NAME, AGE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + tab_path + "' TYPE TAB FIELDS NAME, AGE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE TAB script should complete: " + state.message);
    expect(fs::exists(tab_path), "COPY TO TYPE TAB should create the text file");

    if (fs::exists(tab_path)) {
        const std::string contents = read_text(tab_path);
        expect(contents == "\"Ava\"\t7\r\n\"Ben\"\t42\r\n",
            "COPY TO TYPE TAB should emit tab-delimited rows");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE TAB round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("cname2", "Ben");
    check("nage2", "42");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE TAB destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE TAB should append both tab-delimited rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE TAB row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE TAB row 1 should preserve AGE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE TAB row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE TAB row 2 should preserve AGE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_xls_and_append_from_type_xls_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_xls_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE XLS source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE XLS destination fixture should be created");

    const std::string xls_path = (temp_root / "people.xls").string();
    const fs::path main_path = temp_root / "xls_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + xls_path + "' TYPE XLS FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + xls_path + "' TYPE XLS FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE XLS script should complete: " + state.message);
    expect(fs::exists(xls_path), "COPY TO TYPE XLS should create the workbook file");

    if (fs::exists(xls_path)) {
        const std::string xml_text = read_text(xls_path);
        expect(xml_text.find("<Workbook") != std::string::npos && xml_text.find("<Worksheet") != std::string::npos,
            "COPY TO TYPE XLS should emit SpreadsheetML workbook content");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE XLS round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE XLS destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE XLS should append both workbook rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE XLS row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE XLS row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE XLS row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE XLS row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE XLS row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE XLS row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_dif_and_append_from_type_dif_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dif_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE DIF source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE DIF destination fixture should be created");

    const std::string dif_path = (temp_root / "people.dif").string();
    const fs::path main_path = temp_root / "dif_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + dif_path + "' TYPE DIF FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + dif_path + "' TYPE DIF FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE DIF script should complete: " + state.message);
    expect(fs::exists(dif_path), "COPY TO TYPE DIF should create the interchange file");

    if (fs::exists(dif_path)) {
        const std::string dif_text = read_text(dif_path);
        expect(dif_text.find("TABLE") != std::string::npos &&
               dif_text.find("DATA") != std::string::npos &&
               dif_text.find("EOD") != std::string::npos,
            "COPY TO TYPE DIF should emit DIF-style table markers");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE DIF round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE DIF destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE DIF should append both interchange rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE DIF row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE DIF row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE DIF row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE DIF row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE DIF row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE DIF row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_sylk_and_append_from_type_sylk_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_sylk_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE SYLK source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE SYLK destination fixture should be created");

    const std::string sylk_path = (temp_root / "people.slk").string();
    const fs::path main_path = temp_root / "sylk_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + sylk_path + "' TYPE SYLK FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + sylk_path + "' TYPE SYLK FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE SYLK script should complete: " + state.message);
    expect(fs::exists(sylk_path), "COPY TO TYPE SYLK should create the interchange file");

    if (fs::exists(sylk_path)) {
        const std::string sylk_text = read_text(sylk_path);
        expect(sylk_text.find("ID;P") != std::string::npos &&
               sylk_text.find("B;Y") != std::string::npos &&
               sylk_text.find("\nE\n") != std::string::npos,
            "COPY TO TYPE SYLK should emit SYLK-style table markers");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE SYLK round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE SYLK destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE SYLK should append both interchange rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE SYLK row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE SYLK row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE SYLK row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE SYLK row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE SYLK row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE SYLK row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_type_json_and_append_from_type_json_round_trip() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_json_round_trip";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Ava", "7", "true"},
        {"Ben", "42", "false"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "TYPE JSON source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "TYPE JSON destination fixture should be created");

    const std::string json_path = (temp_root / "people.json").string();
    const fs::path main_path = temp_root / "json_round_trip.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "' ALIAS Source IN 0\n"
        "SELECT Source\n"
        "COPY TO '" + json_path + "' TYPE JSON FIELDS NAME, AGE, ACTIVE\n"
        "USE '" + dest_path.string() + "' ALIAS Dest IN 0\n"
        "SELECT Dest\n"
        "APPEND FROM '" + json_path + "' TYPE JSON FIELDS NAME, AGE, ACTIVE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "nAge1 = AGE\n"
        "lActive1 = ACTIVE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "nAge2 = AGE\n"
        "lActive2 = ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO/APPEND FROM TYPE JSON script should complete: " + state.message);
    expect(fs::exists(json_path), "COPY TO TYPE JSON should create the interchange file");

    if (fs::exists(json_path)) {
        const std::string contents = read_text(json_path);
        expect(contents.find("[") != std::string::npos &&
               contents.find("\"NAME\": \"Ava\"") != std::string::npos &&
               contents.find("\"ACTIVE\": true") != std::string::npos,
            "COPY TO TYPE JSON should emit object-array JSON with typed logical values");
    }

    const auto check = [&](const std::string &name, const std::string &expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should be captured after TYPE JSON round trip");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected,
                name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cname1", "Ava");
    check("nage1", "7");
    check("lactive1", "true");
    check("cname2", "Ben");
    check("nage2", "42");
    check("lactive2", "false");

    const auto result = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(result.ok, "APPEND FROM TYPE JSON destination DBF should remain readable");
    expect(result.table.records.size() == 2U, "APPEND FROM TYPE JSON should append both JSON rows");
    if (result.ok && result.table.records.size() == 2U) {
        expect(result.table.records[0U].values[0U].display_value == "Ava",
            "TYPE JSON row 1 should preserve NAME");
        expect(result.table.records[0U].values[1U].display_value == "7",
            "TYPE JSON row 1 should preserve AGE");
        expect(result.table.records[0U].values[2U].display_value == "true",
            "TYPE JSON row 1 should preserve ACTIVE");
        expect(result.table.records[1U].values[0U].display_value == "Ben",
            "TYPE JSON row 2 should preserve NAME");
        expect(result.table.records[1U].values[1U].display_value == "42",
            "TYPE JSON row 2 should preserve AGE");
        expect(result.table.records[1U].values[2U].display_value == "false",
            "TYPE JSON row 2 should preserve ACTIVE");
    }

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_array_fills_2d_runtime_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});

    const fs::path main_path = temp_root / "copy_to_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "'\n"
        "COPY TO ARRAY myarr\n"
        "cMacroArray = 'macroarr'\n"
        "COPY TO ARRAY &cMacroArray\n"
        "row1_name = myarr[1, 1]\n"
        "row1_age = myarr[1, 2]\n"
        "row2_name = myarr[2, 1]\n"
        "row2_age = myarr[2, 2]\n"
        "arr_rows = ALEN(myarr, 1)\n"
        "arr_cols = ALEN(myarr, 2)\n"
        "macro_row1_name = &cMacroArray[1, 1]\n"
        "macro_rows = ALEN(&cMacroArray, 1)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY script should complete");

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk("arr_rows",  "2",     "COPY TO ARRAY 2 records should give 2 rows");
    chk("arr_cols",  "2",     "COPY TO ARRAY 2 fields should give 2 columns");
    chk("row1_name", "Alice", "COPY TO ARRAY row 1 col 1 should be NAME");
    chk("row1_age",  "30",    "COPY TO ARRAY row 1 col 2 should be AGE");
    chk("row2_name", "Bob",   "COPY TO ARRAY row 2 col 1 should be NAME");
    chk("row2_age",  "25",    "COPY TO ARRAY row 2 col 2 should be AGE");
    chk("macro_row1_name", "Alice", "COPY TO ARRAY macro-expanded target row 1 col 1 should be NAME");
    chk("macro_rows", "2", "COPY TO ARRAY macro-expanded target should give 2 rows");

    fs::remove_all(temp_root, ignored);
}

void test_copy_to_array_fields_clause_allows_keyword_named_field() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_to_array_keyword_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Primary", "30"},
        {"Backup", "25"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "COPY TO ARRAY keyword-field fixture should be created");

    const fs::path main_path = temp_root / "copy_to_array_keyword_field.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "COPY TO ARRAY aTypeOnly FIELDS TYPE\n"
        "nRows = ALEN(aTypeOnly, 1)\n"
        "nCols = ALEN(aTypeOnly, 2)\n"
        "cRow1Type = aTypeOnly[1, 1]\n"
        "cRow2Type = aTypeOnly[2, 1]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY FIELDS TYPE script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "COPY TO ARRAY FIELDS TYPE should preserve both qualifying records");
    chk("ncols", "1", "COPY TO ARRAY FIELDS TYPE should preserve only the selected keyword-named field");
    chk("crow1type", "Primary", "COPY TO ARRAY FIELDS TYPE should copy row 1 TYPE");
    chk("crow2type", "Backup", "COPY TO ARRAY FIELDS TYPE should copy row 2 TYPE");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_writes_records_from_2d_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Source table has two records; dest table starts empty.
    write_people_dbf(temp_root / "source.dbf", {{"Carol", 55}, {"Dave", 19}});
    write_people_dbf(temp_root / "dest.dbf", {});

    const fs::path main_path = temp_root / "append_from_array.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "source.dbf").string() + "'\n"
        "cTempArray = 'tmparr'\n"
        "COPY TO ARRAY &cTempArray\n"
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM ARRAY &cTempArray\n"
        "GO 1\n"
        "dest_name1 = NAME\n"
        "dest_age1 = AGE\n"
        "GO 2\n"
        "dest_name2 = NAME\n"
        "dest_age2 = AGE\n"
        "dest_rc = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY script should complete");

    const auto chk2 = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };
    chk2("dest_rc",    "2",     "APPEND FROM ARRAY should append 2 records");
    chk2("dest_name1", "Carol", "APPEND FROM ARRAY record 1 NAME should match");
    chk2("dest_age1",  "55",    "APPEND FROM ARRAY record 1 AGE should match");
    chk2("dest_name2", "Dave",  "APPEND FROM ARRAY record 2 NAME should match");
    chk2("dest_age2",  "19",    "APPEND FROM ARRAY record 2 AGE should match");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_fields_clause_allows_keyword_named_field() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_keyword_field";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "TYPE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Primary", "30"},
        {"Backup", "25"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "APPEND FROM ARRAY keyword-field source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "APPEND FROM ARRAY keyword-field destination fixture should be created");

    const fs::path main_path = temp_root / "append_from_array_keyword_field.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO ARRAY aTypeOnly FIELDS TYPE\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY aTypeOnly FIELDS TYPE\n"
        "GO 1\n"
        "cType1 = TYPE\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cType2 = TYPE\n"
        "nAge2 = AGE\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY FIELDS TYPE script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "APPEND FROM ARRAY FIELDS TYPE should append both rows");
    chk("ctype1", "Primary", "APPEND FROM ARRAY FIELDS TYPE should restore row 1 TYPE");
    chk("nage1", "0", "APPEND FROM ARRAY FIELDS TYPE should leave omitted AGE at numeric blank");
    chk("ctype2", "Backup", "APPEND FROM ARRAY FIELDS TYPE should restore row 2 TYPE");
    chk("nage2", "0", "APPEND FROM ARRAY FIELDS TYPE should leave omitted AGE at numeric blank");

    fs::remove_all(temp_root, ignored);
}

void test_copy_append_array_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_copy_append_array_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"Alpha", "One", "30"},
        {"Bravo", "Two", "25"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "COPY/APPEND ARRAY LIKE/EXCEPT source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "COPY/APPEND ARRAY LIKE/EXCEPT destination fixture should be created");

    const fs::path main_path = temp_root / "copy_append_array_like_except.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "COPY TO ARRAY aSelected FIELDS LIKE N*\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY aSelected FIELDS EXCEPT AGE\n"
        "GO 1\n"
        "cName1 = NAME\n"
        "cNote1 = NOTE\n"
        "nAge1 = AGE\n"
        "GO 2\n"
        "cName2 = NAME\n"
        "cNote2 = NOTE\n"
        "nAge2 = AGE\n"
        "nRows = RECCOUNT()\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "COPY TO ARRAY FIELDS LIKE / APPEND FROM ARRAY FIELDS EXCEPT script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("nrows", "2", "APPEND FROM ARRAY FIELDS EXCEPT AGE should append both rows");
    chk("cname1", "Alpha", "COPY TO ARRAY FIELDS LIKE N* should preserve keyword-heavy NAME for row 1");
    chk("cnote1", "One", "COPY TO ARRAY FIELDS LIKE N* should preserve NOTE for row 1");
    chk("nage1", "0", "APPEND FROM ARRAY FIELDS EXCEPT AGE should leave AGE blank for row 1");
    chk("cname2", "Bravo", "COPY TO ARRAY FIELDS LIKE N* should preserve NAME for row 2");
    chk("cnote2", "Two", "COPY TO ARRAY FIELDS LIKE N* should preserve NOTE for row 2");
    chk("nage2", "0", "APPEND FROM ARRAY FIELDS EXCEPT AGE should leave AGE blank for row 2");

    fs::remove_all(temp_root, ignored);
}

void test_append_from_array_macro_source_preserves_date_and_datetime_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_append_from_array_date_time";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "source.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BIRTHDAY", .type = 'D', .length = 8U},
        {.name = "STAMP", .type = 'T', .length = 8U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> source_records{
        {"20240117", "julian:2459625 millis:37230000", "41"},
    };
    const auto source_create = copperfin::vfp::create_dbf_table_file(source_path.string(), fields, source_records);
    expect(source_create.ok, "APPEND FROM ARRAY date/datetime source fixture should be created");

    const fs::path dest_path = temp_root / "dest.dbf";
    const auto dest_create = copperfin::vfp::create_dbf_table_file(dest_path.string(), fields, {});
    expect(dest_create.ok, "APPEND FROM ARRAY date/datetime destination fixture should be created");

    const fs::path main_path = temp_root / "append_from_array_date_time.prg";
    write_text(
        main_path,
        "USE '" + source_path.string() + "'\n"
        "cArrayName = 'aTemporal'\n"
        "COPY TO ARRAY &cArrayName FIELDS BIRTHDAY, STAMP\n"
        "USE '" + dest_path.string() + "'\n"
        "APPEND FROM ARRAY &cArrayName FIELDS BIRTHDAY, STAMP\n"
        "GO 1\n"
        "cBirthday = DTOC(BIRTHDAY, 1)\n"
        "cStamp = TTOC(STAMP, 1)\n"
        "nAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APPEND FROM ARRAY macro date/datetime script should complete: " + state.message);

    const auto birthday = state.globals.find("cbirthday");
    const auto stamp = state.globals.find("cstamp");
    const auto age = state.globals.find("nage");
    expect(birthday != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture BIRTHDAY");
    expect(stamp != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture STAMP");
    expect(age != state.globals.end(), "APPEND FROM ARRAY date/datetime script should capture AGE");
    if (birthday != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(birthday->second);
        expect(actual == "20240117",
            "APPEND FROM ARRAY should serialize runtime date strings back into date fields (got '" + actual + "')");
    }
    if (stamp != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(stamp->second);
        expect(actual == "20240117102030",
            "APPEND FROM ARRAY should serialize runtime datetime strings back into datetime fields (got '" + actual + "')");
    }
    if (age != state.globals.end()) {
        expect(copperfin::runtime::format_value(age->second) == "0",
            "APPEND FROM ARRAY FIELDS BIRTHDAY, STAMP should leave omitted numeric fields blank");
    }

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(dest_path.string(), 10U);
    expect(persisted.ok, "APPEND FROM ARRAY date/datetime destination table should remain readable");
    expect(persisted.table.records.size() == 1U, "APPEND FROM ARRAY date/datetime destination should have one row");
    if (persisted.ok && persisted.table.records.size() == 1U) {
        expect(persisted.table.records[0].values[0].display_value == "2024-01-17",
            "APPEND FROM ARRAY should persist date storage strings through the DBF writer (got '" +
                persisted.table.records[0].values[0].display_value + "')");
        expect(persisted.table.records[0].values[1].display_value == "julian:2459625 millis:37230000",
            "APPEND FROM ARRAY should persist datetime storage strings through the DBF writer");
    }

    fs::remove_all(temp_root, ignored);
}

void test_gather_memvar_round_trips_field_values() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_rt";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "table.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "gather_rt.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "table.dbf").string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "m.NAME = 'Updated'\n"
        "GATHER MEMVAR\n"
        "updated_name = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER MEMVAR round-trip script should complete");

    const auto it = state.globals.find("updated_name");
    expect(it != state.globals.end(), "updated_name variable should exist after GATHER MEMVAR");
    if (it != state.globals.end()) {
        const std::string val = copperfin::runtime::format_value(it->second);
        expect(val.find("Updated") != std::string::npos,
            "GATHER MEMVAR should write m.NAME back to record (got '" + val + "')");
    }

    fs::remove_all(temp_root, ignored);
}

void test_m_dot_namespace_shares_bare_memory_variable_binding() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_m_dot_namespace";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "m_dot.prg";
    write_text(
        main_path,
        "m.customer = 'Alice'\n"
        "from_bare = customer\n"
        "M.customer = 'Bob'\n"
        "from_prefixed = m.customer\n"
        "customer = 'Carol'\n"
        "from_m_after_bare = m.customer\n"
        "PUBLIC m.public_name\n"
        "m.public_name = 'Public'\n"
        "from_public_bare = public_name\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "m. namespace script should complete");

    const auto customer = state.globals.find("customer");
    const auto prefixed_customer = state.globals.find("m.customer");
    const auto from_bare = state.globals.find("from_bare");
    const auto from_prefixed = state.globals.find("from_prefixed");
    const auto from_m_after_bare = state.globals.find("from_m_after_bare");
    const auto from_public_bare = state.globals.find("from_public_bare");

    expect(customer != state.globals.end(), "m.customer should be stored under the bare customer binding");
    expect(prefixed_customer == state.globals.end(), "m.customer should not create a separate prefixed global binding");
    expect(from_bare != state.globals.end(), "bare reads should see m.customer assignments");
    expect(from_prefixed != state.globals.end(), "m. reads should see bare memory-variable storage");
    expect(from_m_after_bare != state.globals.end(), "m. reads should see later bare assignments");
    expect(from_public_bare != state.globals.end(), "PUBLIC m.name should declare the bare memory-variable binding");

    if (customer != state.globals.end()) {
        expect(copperfin::runtime::format_value(customer->second) == "Carol",
            "bare assignment should update the shared m.customer binding");
    }
    if (from_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_bare->second) == "Alice",
            "bare reads should resolve the initial m.customer value");
    }
    if (from_prefixed != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_prefixed->second) == "Bob",
            "m.customer reads should resolve the updated shared value");
    }
    if (from_m_after_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_m_after_bare->second) == "Carol",
            "m.customer should resolve a value assigned through the bare name");
    }
    if (from_public_bare != state.globals.end()) {
        expect(copperfin::runtime::format_value(from_public_bare->second) == "Public",
            "PUBLIC m.public_name should be readable through public_name");
    }

    fs::remove_all(temp_root, ignored);
}

void test_browse_emits_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_browse";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}, {"Bob", 25}});
    write_people_dbf(temp_root / "other.dbf", {{"Carol", 55}});

    const fs::path main_path = temp_root / "browse.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "USE '" + (temp_root / "other.dbf").string() + "' ALIAS other AGAIN IN 0\n"
        "SELECT people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "BROWSE\n"
        "BROWSE IN other FIELDS AGE FOR AGE > 40\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "BROWSE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> browse_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.browse") {
            browse_events.push_back(event);
        }
    }

    expect(browse_events.size() == 2U, "BROWSE commands should emit two runtime.browse events");
    if (browse_events.size() >= 2U) {
        expect(browse_events[0].detail.find("people@") != std::string::npos,
            "default BROWSE should target the selected cursor");
        expect(browse_events[0].detail.find("fields=NAME") != std::string::npos,
            "default BROWSE should honor SET FIELDS state");
        expect(browse_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "default BROWSE should surface the current cursor filter");

        expect(browse_events[1].detail.find("other@") != std::string::npos,
            "targeted BROWSE IN should surface the requested cursor");
        expect(browse_events[1].detail.find("fields=AGE") != std::string::npos,
            "BROWSE FIELDS should override SET FIELDS for the event payload");
        expect(browse_events[1].detail.find("for=AGE > 40") != std::string::npos,
            "BROWSE FOR should surface the inline filter expression");
    }

    fs::remove_all(temp_root, ignored);
}

void test_browse_like_and_except_field_filters_surface_event_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_browse_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "BROWSE LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "browse_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "SET FIELDS TO LIKE N*\n"
        "BROWSE\n"
        "BROWSE FIELDS EXCEPT NOTE FOR AGE >= 25\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "BROWSE LIKE/EXCEPT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> browse_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.browse") {
            browse_events.push_back(event);
        }
    }

    expect(browse_events.size() == 2U, "BROWSE LIKE/EXCEPT commands should emit two runtime.browse events");
    if (browse_events.size() >= 2U) {
        expect(browse_events[0].detail.find("fields=NAME,NOTE") != std::string::npos,
            "SET FIELDS TO LIKE N* should surface NAME and NOTE in browse metadata");
        expect(browse_events[1].detail.find("fields=NAME,AGE") != std::string::npos,
            "BROWSE FIELDS EXCEPT NOTE should exclude NOTE in browse metadata");
        expect(browse_events[1].detail.find("for=AGE >= 25") != std::string::npos,
            "BROWSE FIELDS EXCEPT NOTE should preserve the FOR clause in metadata");
    }

    fs::remove_all(temp_root, ignored);
}

void test_edit_command_emits_runtime_edit_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_edit_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "edit_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO LIKE N*\n"
        "EDIT\n"
        "EDIT MEMO notes\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "EDIT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> edit_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.edit") {
            edit_events.push_back(event);
        }
    }

    expect(edit_events.size() == 2U, "EDIT commands should emit two runtime.edit events");
    if (edit_events.size() >= 2U) {
        expect(edit_events[0].detail.find("people@") != std::string::npos,
            "EDIT should surface the selected cursor");
        expect(edit_events[0].detail.find("fields=NAME") != std::string::npos,
            "EDIT should honor SET FIELDS metadata");
        expect(edit_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "EDIT should surface the current filter");
        expect(edit_events[1].detail.find("memo=notes") != std::string::npos,
            "EDIT MEMO should record the memo field name");
    }

    fs::remove_all(temp_root, ignored);
}

void test_change_command_emits_runtime_change_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_change_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "change_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "CHANGE\n"
        "CHANGE FIELD NAME,AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "CHANGE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> change_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.change") {
            change_events.push_back(event);
        }
    }

    expect(change_events.size() == 2U, "CHANGE commands should emit two runtime.change events");
    if (change_events.size() >= 2U) {
        expect(change_events[0].detail.find("people@") != std::string::npos,
            "CHANGE should surface the selected cursor");
        expect(change_events[0].detail.find("fields=NAME,AGE") != std::string::npos,
            "CHANGE without an explicit field list should surface the effective visible fields");
        expect(change_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "CHANGE should surface the current filter");
        expect(change_events[1].detail.find("fields=NAME,AGE") != std::string::npos,
            "CHANGE FIELD should record the field list");
    }

    fs::remove_all(temp_root, ignored);
}

void test_input_command_emits_runtime_input_event_with_prompt() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_input_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "input_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "INPUT \"Enter a value: \" TO myvar\n"
        "cAfterInput = myvar\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INPUT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> input_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.input") {
            input_events.push_back(event);
        }
    }

    expect(input_events.size() == 1U, "INPUT command should emit one runtime.input event");
    const auto after_input = state.globals.find("cafterinput");
    if (input_events.size() >= 1U) {
        expect(input_events[0].detail.find("people@") != std::string::npos,
            "INPUT should surface the selected cursor");
        expect(input_events[0].detail.find("fields=NAME") != std::string::npos,
            "INPUT should honor SET FIELDS metadata");
        expect(input_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "INPUT should surface the current filter");
        expect(input_events[0].detail.find("prompt=") != std::string::npos,
            "INPUT event should include the prompt field");
        expect(input_events[0].detail.find("target=myvar") != std::string::npos,
            "INPUT event should include the target variable name");
        expect(input_events[0].detail.find("result=''") != std::string::npos,
            "INPUT event should surface the deterministic headless result");
    }
    expect(after_input != state.globals.end(), "INPUT should assign a deterministic headless result to the target");
    if (after_input != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_input->second).empty(),
            "INPUT should assign an empty-string headless result to the target variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_accept_command_emits_runtime_accept_event_with_prompt() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_accept_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "accept_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "ACCEPT \"Enter your name: \" TO username\n"
        "cAfterAccept = username\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ACCEPT script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> accept_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.accept") {
            accept_events.push_back(event);
        }
    }

    expect(accept_events.size() == 1U, "ACCEPT command should emit one runtime.accept event");
    const auto after_accept = state.globals.find("cafteraccept");
    if (accept_events.size() >= 1U) {
        expect(accept_events[0].detail.find("people@") != std::string::npos,
            "ACCEPT should surface the selected cursor");
        expect(accept_events[0].detail.find("fields=NAME") != std::string::npos,
            "ACCEPT should honor SET FIELDS metadata");
        expect(accept_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "ACCEPT should surface the current filter");
        expect(accept_events[0].detail.find("prompt=") != std::string::npos,
            "ACCEPT event should include the prompt field");
        expect(accept_events[0].detail.find("target=username") != std::string::npos,
            "ACCEPT event should include the target variable name");
        expect(accept_events[0].detail.find("result=''") != std::string::npos,
            "ACCEPT event should surface the deterministic headless result");
    }
    expect(after_accept != state.globals.end(), "ACCEPT should assign a deterministic headless result to the target");
    if (after_accept != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_accept->second).empty(),
            "ACCEPT should assign an empty-string headless result to the target variable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_getfile_command_emits_runtime_getfile_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_getfile_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "getfile_test.prg";
    write_text(
        main_path,
        "GETFILE PROMPT \"Pick a file\" TITLE \"Open\" DEFAULT \"./data\" FILTER \"*.dbf\" TO lcPath\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GETFILE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.getfile") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "GETFILE command should emit one runtime.getfile event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "GETFILE event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "GETFILE event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "GETFILE event should include default detail");
        expect(dialog_events[0].detail.find("filter=") != std::string::npos,
            "GETFILE event should include filter detail");
        expect(dialog_events[0].detail.find("target=lcPath") != std::string::npos,
            "GETFILE event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "GETFILE event should include deterministic empty-string result detail");
    }

    const auto path_value = state.globals.find("lcpath");
    expect(path_value != state.globals.end(), "GETFILE TO target should be assigned");
    if (path_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(path_value->second).empty(),
            "GETFILE TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_putfile_command_emits_runtime_putfile_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_putfile_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "putfile_test.prg";
    write_text(
        main_path,
        "PUTFILE PROMPT \"Save as\" TITLE \"Save\" DEFAULT \"./output\" FILTER \"*.txt\" TO lcSavePath\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "PUTFILE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.putfile") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "PUTFILE command should emit one runtime.putfile event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "PUTFILE event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "PUTFILE event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "PUTFILE event should include default detail");
        expect(dialog_events[0].detail.find("filter=") != std::string::npos,
            "PUTFILE event should include filter detail");
        expect(dialog_events[0].detail.find("target=lcSavePath") != std::string::npos,
            "PUTFILE event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "PUTFILE event should include deterministic empty-string result detail");
    }

    const auto save_path_value = state.globals.find("lcsavepath");
    expect(save_path_value != state.globals.end(), "PUTFILE TO target should be assigned");
    if (save_path_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(save_path_value->second).empty(),
            "PUTFILE TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_getdir_command_emits_runtime_getdir_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_getdir_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "getdir_test.prg";
    write_text(
        main_path,
        "GETDIR PROMPT \"Choose folder\" TITLE \"Browse\" DEFAULT \"./workspace\" TO lcDir\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GETDIR script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.getdir") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "GETDIR command should emit one runtime.getdir event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "GETDIR event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "GETDIR event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "GETDIR event should include default detail");
        expect(dialog_events[0].detail.find("target=lcDir") != std::string::npos,
            "GETDIR event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "GETDIR event should include deterministic empty-string result detail");
    }

    const auto dir_value = state.globals.find("lcdir");
    expect(dir_value != state.globals.end(), "GETDIR TO target should be assigned");
    if (dir_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(dir_value->second).empty(),
            "GETDIR TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_inputbox_command_emits_runtime_inputbox_event_with_clause_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_inputbox_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "inputbox_test.prg";
    write_text(
        main_path,
        "INPUTBOX PROMPT \"Enter value\" TITLE \"Question\" DEFAULT \"42\" TO lcAnswer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "INPUTBOX script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> dialog_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.inputbox") {
            dialog_events.push_back(event);
        }
    }

    expect(dialog_events.size() == 1U, "INPUTBOX command should emit one runtime.inputbox event");
    if (dialog_events.size() >= 1U) {
        expect(dialog_events[0].detail.find("prompt=") != std::string::npos,
            "INPUTBOX event should include prompt detail");
        expect(dialog_events[0].detail.find("title=") != std::string::npos,
            "INPUTBOX event should include title detail");
        expect(dialog_events[0].detail.find("default=") != std::string::npos,
            "INPUTBOX event should include default detail");
        expect(dialog_events[0].detail.find("target=lcAnswer") != std::string::npos,
            "INPUTBOX event should include target detail");
        expect(dialog_events[0].detail.find("result=''") != std::string::npos,
            "INPUTBOX event should include deterministic empty-string result detail");
    }

    const auto answer_value = state.globals.find("lcanswer");
    expect(answer_value != state.globals.end(), "INPUTBOX TO target should be assigned");
    if (answer_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(answer_value->second).empty(),
            "INPUTBOX TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_dialog_commands_parenthesized_forms_assign_targets_and_extract_positional_details() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_dialog_parenthesized";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "dialog_parenthesized_test.prg";
    write_text(
        main_path,
        "LOCAL lcLocalFile\n"
        "GETFILE(\"*.dbf\", \"Pick a file\", \"Open\", \"./data\") TO lcLocalFile\n"
        "cLocalFileResult = lcLocalFile\n"
        "PUTFILE(\"*.txt\", \"Save as\", \"Save\", \"./out\") TO cPut\n"
        "GETDIR(\"./workspace\", \"Choose folder\", \"Browse\") TO m.cMemDir\n"
        "INPUTBOX(\"Enter value\", \"Question\", \"42\") TO cAnswer\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "parenthesized dialog script should complete");

    const auto local_file_result = state.globals.find("clocalfileresult");
    const auto put_result = state.globals.find("cput");
    const auto mem_dir_result = state.globals.find("cmemdir");
    const auto answer_result = state.globals.find("canswer");
    expect(local_file_result != state.globals.end(), "GETFILE TO LOCAL target should be assignable and readable in-scope");
    expect(put_result != state.globals.end(), "PUTFILE TO target should be assigned");
    expect(mem_dir_result != state.globals.end(), "GETDIR TO m.<var> target should be assigned through memory-variable path");
    expect(answer_result != state.globals.end(), "INPUTBOX TO target should be assigned");

    if (local_file_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(local_file_result->second).empty(),
            "GETFILE parenthesized TO LOCAL target should default to empty string");
    }
    if (put_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(put_result->second).empty(),
            "PUTFILE parenthesized TO target should default to empty string");
    }
    if (mem_dir_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(mem_dir_result->second).empty(),
            "GETDIR parenthesized TO m.<var> target should default to empty string");
    }
    if (answer_result != state.globals.end()) {
        expect(copperfin::runtime::format_value(answer_result->second).empty(),
            "INPUTBOX parenthesized TO target should default to empty string");
    }

    const auto find_event = [&](const std::string& category) -> const copperfin::runtime::RuntimeEvent* {
        for (const auto& event : state.events) {
            if (event.category == category) {
                return &event;
            }
        }
        return nullptr;
    };

    const auto* getfile_event = find_event("runtime.getfile");
    const auto* putfile_event = find_event("runtime.putfile");
    const auto* getdir_event = find_event("runtime.getdir");
    const auto* inputbox_event = find_event("runtime.inputbox");
    expect(getfile_event != nullptr, "GETFILE parenthesized call should emit runtime.getfile event");
    expect(putfile_event != nullptr, "PUTFILE parenthesized call should emit runtime.putfile event");
    expect(getdir_event != nullptr, "GETDIR parenthesized call should emit runtime.getdir event");
    expect(inputbox_event != nullptr, "INPUTBOX parenthesized call should emit runtime.inputbox event");

    if (getfile_event != nullptr) {
        expect(getfile_event->detail.find("filter=\"*.dbf\"") != std::string::npos,
            "GETFILE positional extraction should map first argument to filter");
        expect(getfile_event->detail.find("prompt=\"Pick a file\"") != std::string::npos,
            "GETFILE positional extraction should map second argument to prompt");
        expect(getfile_event->detail.find("title=\"Open\"") != std::string::npos,
            "GETFILE positional extraction should map third argument to title");
        expect(getfile_event->detail.find("default=\"./data\"") != std::string::npos,
            "GETFILE positional extraction should map fourth argument to default");
        expect(getfile_event->detail.find("target=lcLocalFile") != std::string::npos,
            "GETFILE parenthesized event should retain TO target detail");
    }
    if (putfile_event != nullptr) {
        expect(putfile_event->detail.find("filter=\"*.txt\"") != std::string::npos,
            "PUTFILE positional extraction should map first argument to filter");
        expect(putfile_event->detail.find("prompt=\"Save as\"") != std::string::npos,
            "PUTFILE positional extraction should map second argument to prompt");
        expect(putfile_event->detail.find("title=\"Save\"") != std::string::npos,
            "PUTFILE positional extraction should map third argument to title");
        expect(putfile_event->detail.find("default=\"./out\"") != std::string::npos,
            "PUTFILE positional extraction should map fourth argument to default");
        expect(putfile_event->detail.find("target=cPut") != std::string::npos,
            "PUTFILE parenthesized event should retain TO target detail");
    }
    if (getdir_event != nullptr) {
        expect(getdir_event->detail.find("default=\"./workspace\"") != std::string::npos,
            "GETDIR positional extraction should map first argument to default");
        expect(getdir_event->detail.find("prompt=\"Choose folder\"") != std::string::npos,
            "GETDIR positional extraction should map second argument to prompt");
        expect(getdir_event->detail.find("title=\"Browse\"") != std::string::npos,
            "GETDIR positional extraction should map third argument to title");
        expect(getdir_event->detail.find("target=m.cMemDir") != std::string::npos,
            "GETDIR parenthesized event should retain TO target detail");
    }
    if (inputbox_event != nullptr) {
        expect(inputbox_event->detail.find("prompt=\"Enter value\"") != std::string::npos,
            "INPUTBOX positional extraction should map first argument to prompt");
        expect(inputbox_event->detail.find("title=\"Question\"") != std::string::npos,
            "INPUTBOX positional extraction should map second argument to title");
        expect(inputbox_event->detail.find("default=\"42\"") != std::string::npos,
            "INPUTBOX positional extraction should map third argument to default");
        expect(inputbox_event->detail.find("target=cAnswer") != std::string::npos,
            "INPUTBOX parenthesized event should retain TO target detail");
    }

    fs::remove_all(temp_root, ignored);
}

void test_wait_window_command_emits_runtime_wait_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_wait_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "wait_test.prg";
    write_text(
        main_path,
        "cPrompt = \"Please wait...\"\n"
        "nDelay = 5\n"
        "cWaitTarget = \"m.wResult\"\n"
        "WAIT WINDOW cPrompt TIMEOUT nDelay TO &cWaitTarget NOWAIT NOCLEAR\n"
        "cAfterWait = m.wResult\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WAIT WINDOW script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> wait_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.wait") {
            wait_events.push_back(event);
        }
    }

    expect(wait_events.size() == 1U, "WAIT WINDOW command should emit one runtime.wait event");
    const auto after_wait = state.globals.find("cafterwait");
    if (wait_events.size() >= 1U) {
        expect(wait_events[0].detail.find("mode=WINDOW") != std::string::npos,
            "WAIT WINDOW event should report mode=WINDOW");
        expect(wait_events[0].detail.find("prompt=Please wait...") != std::string::npos,
            "WAIT WINDOW event should surface the resolved prompt text");
        expect(wait_events[0].detail.find("prompt_expr=cPrompt") != std::string::npos,
            "WAIT WINDOW event should preserve the source prompt expression");
        expect(wait_events[0].detail.find("timeout=5") != std::string::npos,
            "WAIT WINDOW event should surface the resolved timeout expression");
        expect(wait_events[0].detail.find("timeout_expr=nDelay") != std::string::npos,
            "WAIT WINDOW event should preserve the source timeout expression");
        expect(wait_events[0].detail.find("flag=NOWAIT") != std::string::npos,
            "WAIT WINDOW event should surface the NOWAIT flag");
        expect(wait_events[0].detail.find("flag=NOCLEAR") != std::string::npos,
            "WAIT WINDOW event should surface the NOCLEAR flag");
        expect(wait_events[0].detail.find("target=&cWaitTarget") != std::string::npos,
            "WAIT WINDOW event should include the raw TO target expression");
        expect(wait_events[0].detail.find("target_resolved=m.wResult") != std::string::npos,
            "WAIT WINDOW event should include the resolved TO target");
        expect(wait_events[0].detail.find("result=''") != std::string::npos,
            "WAIT WINDOW event should surface the deterministic headless result");
    }
    expect(after_wait != state.globals.end(), "WAIT WINDOW TO target should be assigned");
    if (after_wait != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_wait->second).empty(),
            "WAIT WINDOW TO target should default to an empty string when host response is unavailable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_wait_clear_command_emits_runtime_wait_clear_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_wait_clear_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "wait_clear_test.prg";
    write_text(
        main_path,
        "WAIT CLEAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "WAIT CLEAR script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> wait_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.wait") {
            wait_events.push_back(event);
        }
    }

    expect(wait_events.size() == 1U, "WAIT CLEAR command should emit one runtime.wait event");
    if (wait_events.size() >= 1U) {
        expect(wait_events[0].detail.find("mode=CLEAR") != std::string::npos,
            "WAIT CLEAR event should report mode=CLEAR");
    }

    fs::remove_all(temp_root, ignored);
}

void test_keyboard_command_emits_runtime_keyboard_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_keyboard_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "keyboard_test.prg";
    write_text(
        main_path,
        "cKeys = \"ABC\"\n"
        "KEYBOARD cKeys PLAIN CLEAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "KEYBOARD script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> kb_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.keyboard") {
            kb_events.push_back(event);
        }
    }

    expect(kb_events.size() == 1U, "KEYBOARD command should emit one runtime.keyboard event");
    if (kb_events.size() >= 1U) {
        expect(kb_events[0].detail.find("keys=ABC") != std::string::npos,
            "KEYBOARD event should include the resolved keys payload");
        expect(kb_events[0].detail.find("keys_expr=cKeys") != std::string::npos,
            "KEYBOARD event should preserve the source key expression");
        expect(kb_events[0].detail.find("flag=PLAIN") != std::string::npos,
            "KEYBOARD event should include the PLAIN flag");
        expect(kb_events[0].detail.find("flag=CLEAR") != std::string::npos,
            "KEYBOARD event should include the CLEAR flag");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_structure_emits_runtime_display_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "display_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "DISPLAY STRUCTURE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY STRUCTURE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY STRUCTURE command should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=STRUCTURE") != std::string::npos,
            "DISPLAY STRUCTURE event should report mode=STRUCTURE");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY STRUCTURE should surface the selected cursor");
        expect(display_events[0].detail.find("field_count=2") != std::string::npos,
            "DISPLAY STRUCTURE should surface the schema field count");
        expect(display_events[0].detail.find("schema_fields=NAME,AGE") != std::string::npos,
            "DISPLAY STRUCTURE should surface schema field names");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_status_surfaces_session_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_status";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "display_status_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "DISPLAY STATUS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY STATUS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY STATUS should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=STATUS") != std::string::npos,
            "DISPLAY STATUS event should report mode=STATUS");
        expect(display_events[0].detail.find("datasession=1") != std::string::npos,
            "DISPLAY STATUS should report the current data session");
        expect(display_events[0].detail.find("open_cursors=1") != std::string::npos,
            "DISPLAY STATUS should report open cursor count");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY STATUS should surface the selected cursor");
        expect(display_events[0].detail.find("fields=NAME") != std::string::npos,
            "DISPLAY STATUS should honor current visible-field metadata");
        expect(display_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "DISPLAY STATUS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_memory_surfaces_visible_variable_and_array_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_memory";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "display_memory_test.prg";
    write_text(
        main_path,
        "PUBLIC cPublic\n"
        "cPublic = \"pub\"\n"
        "cGlobal = \"glob\"\n"
        "PRIVATE cPrivate\n"
        "cPrivate = \"priv\"\n"
        "LOCAL nLocal\n"
        "nLocal = 42\n"
        "LOCAL cPublic\n"
        "cPublic = \"localpub\"\n"
        "DIMENSION aMemory(2,2)\n"
        "aMemory[1,1] = \"x\"\n"
        "oThing = CREATEOBJECT('Empty')\n"
        "DISPLAY MEMORY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY MEMORY script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY MEMORY should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=MEMORY") != std::string::npos,
            "DISPLAY MEMORY event should report mode=MEMORY");
        expect(display_events[0].detail.find("memvar_count=5") != std::string::npos,
            "DISPLAY MEMORY should report visible memory-variable count");
        expect(display_events[0].detail.find("public_count=0") != std::string::npos,
            "DISPLAY MEMORY should report public variable count");
        expect(display_events[0].detail.find("private_count=1") != std::string::npos,
            "DISPLAY MEMORY should report private variable count");
        expect(display_events[0].detail.find("local_count=2") != std::string::npos,
            "DISPLAY MEMORY should report local variable count");
        expect(display_events[0].detail.find("global_count=2") != std::string::npos,
            "DISPLAY MEMORY should report ordinary global variable count");
        expect(display_events[0].detail.find("array_count=1") != std::string::npos,
            "DISPLAY MEMORY should report visible runtime array count");
        expect(display_events[0].detail.find("cpublic{local:C=localpub}") != std::string::npos,
            "DISPLAY MEMORY should include the visible shadowing local value");
        expect(display_events[0].detail.find("cprivate{private:C=priv}") != std::string::npos,
            "DISPLAY MEMORY should include private variable scope/type/value detail");
        expect(display_events[0].detail.find("nlocal{local:N=42}") != std::string::npos,
            "DISPLAY MEMORY should include local variable scope/type/value detail");
        expect(display_events[0].detail.find("cglobal{global:C=glob}") != std::string::npos,
            "DISPLAY MEMORY should include global variable scope/type/value detail");
        expect(display_events[0].detail.find("othing{global:O=<object:Empty props=0>}") != std::string::npos,
            "DISPLAY MEMORY should include object scope/type/detail");
        expect(display_events[0].detail.find("shadowed=cpublic{public:C=pub}") != std::string::npos,
            "DISPLAY MEMORY should include shadowed public bindings");
        expect(display_events[0].detail.find("amemory{global:A=2x2}") != std::string::npos,
            "DISPLAY MEMORY should include runtime array scope and dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_display_records_surfaces_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_display_records";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "DISPLAY RECORDS fixture should be created");

    const fs::path main_path = temp_root / "display_records.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO LIKE N*\n"
        "DISPLAY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "DISPLAY RECORDS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> display_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.display") {
            display_events.push_back(event);
        }
    }

    expect(display_events.size() == 1U, "DISPLAY RECORDS should emit one runtime.display event");
    if (display_events.size() >= 1U) {
        expect(display_events[0].detail.find("mode=RECORDS") != std::string::npos,
            "DISPLAY RECORDS event should report mode=RECORDS");
        expect(display_events[0].detail.find("people@") != std::string::npos,
            "DISPLAY RECORDS event should surface the selected cursor");
        expect(display_events[0].detail.find("fields=NAME,NOTE") != std::string::npos,
            "DISPLAY RECORDS should honor SET FIELDS LIKE metadata");
        expect(display_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "DISPLAY RECORDS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_status_emits_runtime_list_event() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_cmd";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "list_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "SET FILTER TO AGE >= 25\n"
        "SET FIELDS TO NAME\n"
        "LIST STATUS\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST STATUS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST STATUS command should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=STATUS") != std::string::npos,
            "LIST STATUS event should report mode=STATUS");
        expect(list_events[0].detail.find("datasession=1") != std::string::npos,
            "LIST STATUS should report the current data session");
        expect(list_events[0].detail.find("open_cursors=1") != std::string::npos,
            "LIST STATUS should report open cursor count");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST STATUS should surface the selected cursor");
        expect(list_events[0].detail.find("fields=NAME") != std::string::npos,
            "LIST STATUS should honor current visible-field metadata");
        expect(list_events[0].detail.find("filter=AGE >= 25") != std::string::npos,
            "LIST STATUS should surface the current filter");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_memory_surfaces_visible_variable_and_array_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_memory";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "list_memory_test.prg";
    write_text(
        main_path,
        "PUBLIC cPublic\n"
        "cPublic = \"pub\"\n"
        "cGlobal = \"glob\"\n"
        "PRIVATE cPrivate\n"
        "cPrivate = \"priv\"\n"
        "LOCAL nLocal\n"
        "nLocal = 42\n"
        "LOCAL cPublic\n"
        "cPublic = \"localpub\"\n"
        "DIMENSION aMemory(2,2)\n"
        "aMemory[1,1] = \"x\"\n"
        "oThing = CREATEOBJECT('Empty')\n"
        "LIST MEMORY\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST MEMORY script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST MEMORY should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=MEMORY") != std::string::npos,
            "LIST MEMORY event should report mode=MEMORY");
        expect(list_events[0].detail.find("memvar_count=5") != std::string::npos,
            "LIST MEMORY should report visible memory-variable count");
        expect(list_events[0].detail.find("public_count=0") != std::string::npos,
            "LIST MEMORY should report public variable count");
        expect(list_events[0].detail.find("private_count=1") != std::string::npos,
            "LIST MEMORY should report private variable count");
        expect(list_events[0].detail.find("local_count=2") != std::string::npos,
            "LIST MEMORY should report local variable count");
        expect(list_events[0].detail.find("global_count=2") != std::string::npos,
            "LIST MEMORY should report ordinary global variable count");
        expect(list_events[0].detail.find("array_count=1") != std::string::npos,
            "LIST MEMORY should report visible runtime array count");
        expect(list_events[0].detail.find("cpublic{local:C=localpub}") != std::string::npos,
            "LIST MEMORY should include the visible shadowing local value");
        expect(list_events[0].detail.find("cprivate{private:C=priv}") != std::string::npos,
            "LIST MEMORY should include private variable scope/type/value detail");
        expect(list_events[0].detail.find("nlocal{local:N=42}") != std::string::npos,
            "LIST MEMORY should include local variable scope/type/value detail");
        expect(list_events[0].detail.find("cglobal{global:C=glob}") != std::string::npos,
            "LIST MEMORY should include global variable scope/type/value detail");
        expect(list_events[0].detail.find("othing{global:O=<object:Empty props=0>}") != std::string::npos,
            "LIST MEMORY should include object scope/type/detail");
        expect(list_events[0].detail.find("shadowed=cpublic{public:C=pub}") != std::string::npos,
            "LIST MEMORY should include shadowed public bindings");
        expect(list_events[0].detail.find("amemory{global:A=2x2}") != std::string::npos,
            "LIST MEMORY should include runtime array scope and dimensions");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_structure_surfaces_selected_cursor_schema() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_structure";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 30}});

    const fs::path main_path = temp_root / "list_structure_test.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS people\n"
        "LIST STRUCTURE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST STRUCTURE script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST STRUCTURE should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=STRUCTURE") != std::string::npos,
            "LIST STRUCTURE event should report mode=STRUCTURE");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST STRUCTURE should surface the selected cursor");
        expect(list_events[0].detail.find("field_count=2") != std::string::npos,
            "LIST STRUCTURE should surface the schema field count");
        expect(list_events[0].detail.find("schema_fields=NAME,AGE") != std::string::npos,
            "LIST STRUCTURE should surface schema field names");
    }

    fs::remove_all(temp_root, ignored);
}

void test_list_records_surfaces_effective_cursor_view_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_list_records";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Ready", "30"},
        {"Bob", "Later", "22"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "LIST RECORDS fixture should be created");

    const fs::path main_path = temp_root / "list_records.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS people\n"
        "LIST FIELDS EXCEPT NOTE FOR AGE >= 25\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "LIST RECORDS script should complete");

    std::vector<copperfin::runtime::RuntimeEvent> list_events;
    for (const auto &event : state.events) {
        if (event.category == "runtime.list") {
            list_events.push_back(event);
        }
    }

    expect(list_events.size() == 1U, "LIST RECORDS should emit one runtime.list event");
    if (list_events.size() >= 1U) {
        expect(list_events[0].detail.find("mode=RECORDS") != std::string::npos,
            "LIST RECORDS event should report mode=RECORDS");
        expect(list_events[0].detail.find("people@") != std::string::npos,
            "LIST RECORDS event should surface the selected cursor");
        expect(list_events[0].detail.find("fields=NAME,AGE") != std::string::npos,
            "LIST RECORDS should honor inline FIELDS EXCEPT metadata");
        expect(list_events[0].detail.find("for=AGE >= 25") != std::string::npos,
            "LIST RECORDS should surface the FOR clause");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_scatter_memvar_from_current_record();
    test_scatter_gather_memvar_fields_blank_and_for_semantics();
    test_scatter_gather_memvar_single_name_field_filter_semantics();
    test_scatter_to_array_and_gather_from_array_round_trip();
    test_scatter_gather_memvar_preserves_date_and_datetime_like_values();
    test_scatter_gather_array_preserves_date_and_datetime_like_values();
    test_scatter_gather_name_object_round_trip();
    test_scatter_name_additive_merges_existing_object_properties();
    test_scatter_gather_name_single_name_field_filter_semantics();
    test_scatter_gather_name_like_and_except_field_filters();
    test_scatter_gather_name_supports_macro_object_variable_names();
    test_scatter_gather_name_supports_nested_object_targets();
    test_scatter_gather_name_supports_macro_expanded_nested_property_segments();
    test_scatter_gather_name_creates_missing_nested_object_targets();
    test_scatter_name_without_additive_replaces_existing_nested_target_object();
    test_scatter_gather_predeclared_2d_array_row_one_semantics();
    test_scatter_gather_two_column_name_value_array_semantics();
    test_scatter_memo_clause_controls_memo_field_inclusion();
    test_runtime_array_mutator_functions();
    test_save_to_writes_variables_to_file();
    test_restore_from_loads_variables_from_file();
    test_restore_from_additive_merges_variables();
    test_save_to_like_pattern_filters_variables();
    test_save_to_except_pattern_filters_variables();
    test_save_restore_auto_mem_extension_without_explicit_extension();
    test_save_restore_round_trips_escaped_string_and_types();
    test_restore_from_honors_current_frame_local_bindings();
    test_save_restore_round_trips_arrays();
    test_save_restore_round_trips_public_scope();
    test_save_to_shadowed_public_name_does_not_persist_public_scope_marker();
    test_restore_from_without_additive_clears_prior_globals();
    test_restore_from_without_additive_clears_stale_arrays();
    test_restore_from_without_additive_clears_private_shadow_state();
    test_restore_from_rejects_numeric_trailing_garbage();
    test_copy_to_emits_event();
    test_copy_to_creates_destination_dbf();
    test_copy_structure_to_creates_empty_schema();
    test_append_from_copies_records_into_current_table();
    test_copy_to_type_sdf_writes_fixed_width_text_rows();
    test_append_from_type_sdf_imports_fixed_width_text_rows();
    test_copy_to_type_csv_and_delimited_text_rows();
    test_append_from_type_csv_imports_delimited_rows();
    test_copy_to_type_tab_and_append_from_type_tab_round_trip();
    test_copy_to_type_xls_and_append_from_type_xls_round_trip();
    test_copy_to_type_dif_and_append_from_type_dif_round_trip();
    test_copy_to_type_sylk_and_append_from_type_sylk_round_trip();
    test_copy_to_type_json_and_append_from_type_json_round_trip();
    test_copy_to_array_fills_2d_runtime_array();
    test_copy_to_array_fields_clause_allows_keyword_named_field();
    test_append_from_array_writes_records_from_2d_array();
    test_append_from_array_fields_clause_allows_keyword_named_field();
    test_copy_append_array_like_and_except_field_filters();
    test_append_from_array_macro_source_preserves_date_and_datetime_fields();
    test_gather_memvar_round_trips_field_values();
    test_m_dot_namespace_shares_bare_memory_variable_binding();
    test_browse_emits_effective_cursor_view_metadata();
    test_browse_like_and_except_field_filters_surface_event_metadata();
    test_edit_command_emits_runtime_edit_event();
    test_change_command_emits_runtime_change_event();
    test_input_command_emits_runtime_input_event_with_prompt();
    test_accept_command_emits_runtime_accept_event_with_prompt();
    test_getfile_command_emits_runtime_getfile_event_with_clause_details();
    test_putfile_command_emits_runtime_putfile_event_with_clause_details();
    test_getdir_command_emits_runtime_getdir_event_with_clause_details();
    test_inputbox_command_emits_runtime_inputbox_event_with_clause_details();
    test_dialog_commands_parenthesized_forms_assign_targets_and_extract_positional_details();
    test_wait_window_command_emits_runtime_wait_event();
    test_wait_clear_command_emits_runtime_wait_clear_event();
    test_keyboard_command_emits_runtime_keyboard_event();
    test_display_structure_emits_runtime_display_event();
    test_display_status_surfaces_session_metadata();
    test_display_memory_surfaces_visible_variable_and_array_metadata();
    test_display_records_surfaces_effective_cursor_view_metadata();
    test_list_status_emits_runtime_list_event();
    test_list_memory_surfaces_visible_variable_and_array_metadata();
    test_list_structure_surfaces_selected_cursor_schema();
    test_list_records_surfaces_effective_cursor_view_metadata();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
