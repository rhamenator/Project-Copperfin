// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_data_io_support.h"

namespace cf_test_prg_engine_data_io {
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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
        "FUNCTION ShouldGather\n"
        "RETURN .T.\n"
        "ENDFUNC\n"
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
        "GATHER MEMVAR FIELDS NAME FOR ShouldGather()\n"
        "cAfterGather = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

void test_gather_memvar_preserves_fields_without_matching_variables() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_missing_memvars";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "NOTE", .type = 'C', .length = 10U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"Alice", "42", "Keep", "true"}});
    expect(create_result.ok, "GATHER MEMVAR missing-variable fixture should be created");

    const fs::path main_path = temp_root / "gather_missing_memvars.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "m.NAME = 'Updated'\n"
        "GATHER MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER MEMVAR missing-variable script should complete: " + state.message);

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "GATHER MEMVAR missing-variable destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty() && persisted.table.records[0U].values.size() == 4U) {
        const auto& values = persisted.table.records[0U].values;
        expect(values[0U].display_value == "Updated",
            "GATHER MEMVAR should update a field with a matching memvar");
        expect(values[1U].display_value == "42",
            "GATHER MEMVAR should preserve a numeric field with no matching memvar");
        expect(values[2U].display_value == "Keep",
            "GATHER MEMVAR should preserve a character field with no matching memvar");
        expect(values[3U].display_value == "true",
            "GATHER MEMVAR should preserve a logical field with no matching memvar");
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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
        "GATHER FROM resolve_array_name() FIELDS NAME, AGE\n"
        "cAfterName = NAME\n"
        "nAfterAge = AGE\n"
        "cMacroRowName = 'aMacroRow'\n"
        "cMacroRowNameHolder = 'cMacroRowName'\n"
        "cMacroRowNameDeepHolder = 'cMacroRowNameHolder'\n"
        "SCATTER FIELDS NAME, AGE TO &cMacroRowNameDeepHolder\n"
        "nMacroArrayLen = ALEN(&cMacroRowNameDeepHolder)\n"
        "cMacroFirst = &cMacroRowNameDeepHolder[1]\n"
        "REPLACE NAME WITH 'MacroChg', AGE WITH 8\n"
        "GATHER FROM &cMacroRowNameDeepHolder FIELDS NAME, AGE\n"
        "cMacroAfterName = NAME\n"
        "nMacroAfterAge = AGE\n"
        "cMacroRowNameSecondHop = 'aMacroRow2'\n"
        "cMacroRowNameSecondHopHolder = 'cMacroRowNameSecondHop'\n"
        "cMacroRowNameSecondDeepHolder = 'cMacroRowNameSecondHopHolder'\n"
        "SCATTER FIELDS NAME, AGE TO &cMacroRowNameSecondDeepHolder\n"
        "nMacroArrayLenSecondHop = ALEN(&cMacroRowNameSecondHop)\n"
        "cMacroFirstSecondHop = &cMacroRowNameSecondHop[1]\n"
        "REPLACE NAME WITH 'DeepMacroChg', AGE WITH 9\n"
        "GATHER FROM &cMacroRowNameSecondDeepHolder FIELDS NAME, AGE\n"
        "cMacroAfterNameSecondHop = NAME\n"
        "nMacroAfterAgeSecondHop = AGE\n"
        "RETURN\n"
        "FUNCTION resolve_array_name\n"
        "RETURN 'aRow'\n"
        "ENDFUNC\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
    const auto macro_array_len_second_hop = state.globals.find("nmacroarraylensecondhop");
    const auto macro_first_second_hop = state.globals.find("cmacrofirstsecondhop");
    const auto macro_after_name_second_hop = state.globals.find("cmacroafternamesecondhop");
    const auto macro_after_age_second_hop = state.globals.find("nmacroafteragesecondhop");

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
    expect(macro_array_len_second_hop != state.globals.end(), "SCATTER TO second-hop macro-expanded array should expose array element count");
    expect(macro_first_second_hop != state.globals.end(), "SCATTER TO second-hop macro-expanded array should be readable through macro access");
    expect(macro_after_name_second_hop != state.globals.end(), "GATHER FROM second-hop macro-expanded array should restore NAME");
    expect(macro_after_age_second_hop != state.globals.end(), "GATHER FROM second-hop macro-expanded array should restore AGE");

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
    if (macro_array_len_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_array_len_second_hop->second) == "2",
            "SCATTER TO second-hop macro-expanded array should create two array elements");
    }
    if (macro_first_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_first_second_hop->second) == "Alice",
            "second-hop macro-expanded SCATTER array access should read the first scattered value");
    }
    if (macro_after_name_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_after_name_second_hop->second) == "Alice",
            "GATHER FROM second-hop macro-expanded array should write NAME from the array");
    }
    if (macro_after_age_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_after_age_second_hop->second) == "42",
            "GATHER FROM second-hop macro-expanded array should write AGE from the array");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_and_gather_array_preserve_explicit_fields_order() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_array_field_order";
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
    expect(create_result.ok, "scatter/gather array field-order DBF fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_array_field_order.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS AGE, NAME TO aRow\n"
        "nFirstPlus = aRow(1) + 1\n"
        "cSecond = aRow[2]\n"
        "REPLACE NAME WITH 'Changed', AGE WITH 7\n"
        "GATHER FROM aRow FIELDS AGE, NAME\n"
        "nAfterAge = AGE\n"
        "cAfterName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER array explicit field-order script should complete: " + state.message);

    const auto first_plus = state.globals.find("nfirstplus");
    const auto second = state.globals.find("csecond");
    const auto after_age = state.globals.find("nafterage");
    const auto after_name = state.globals.find("caftername");

    expect(first_plus != state.globals.end(), "reordered SCATTER should expose AGE in the first array slot");
    expect(second != state.globals.end(), "reordered SCATTER should expose NAME in the second array slot");
    expect(after_age != state.globals.end(), "reordered GATHER should restore AGE from the first array slot");
    expect(after_name != state.globals.end(), "reordered GATHER should restore NAME from the second array slot");

    if (first_plus != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_plus->second) == "43",
            "SCATTER FIELDS AGE, NAME should preserve the explicit first-slot numeric AGE field");
    }
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "Alice",
            "SCATTER FIELDS AGE, NAME should preserve the explicit second-slot NAME field");
    }
    if (after_age != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_age->second) == "42",
            "GATHER FROM array FIELDS AGE, NAME should restore AGE from the first array element");
    }
    if (after_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(after_name->second) == "Alice",
            "GATHER FROM array FIELDS AGE, NAME should restore NAME from the second array element");
    }

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_array_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_array_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{{"Alice", "Ready", "42"}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER/GATHER array LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_array_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "GO 1\n"
        "SCATTER FIELDS LIKE N* TO aRow\n"
        "cLikeName = aRow[1]\n"
        "cLikeNote = aRow[2]\n"
        "nLikeLen = ALEN(aRow)\n"
        "aRow[1] = 'ArrayName'\n"
        "aRow[2] = 'ArrayNote'\n"
        "GATHER FROM aRow FIELDS EXCEPT AGE\n"
        "cAfterName = People.NAME\n"
        "cAfterNote = People.NOTE\n"
        "nAfterAge = People.AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER FROM array LIKE/EXCEPT script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("clikename", "Alice", "SCATTER FIELDS LIKE N* TO array should include NAME");
    chk("clikenote", "Ready", "SCATTER FIELDS LIKE N* TO array should include NOTE");
    chk("nlikelen", "2", "SCATTER FIELDS LIKE N* TO array should include exactly two fields");
    chk("caftername", "ArrayName", "GATHER FROM array FIELDS EXCEPT AGE should write NAME");
    chk("cafternote", "ArrayNote", "GATHER FROM array FIELDS EXCEPT AGE should write NOTE");
    chk("nafterage", "42", "GATHER FROM array FIELDS EXCEPT AGE should leave AGE unchanged");

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
        expect(copperfin::runtime::format_value(blank_birth_type->second) == "D",
            "SCATTER MEMVAR BLANK should preserve date type metadata while leaving the value blank");
    }
    if (blank_stamp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_stamp_type->second) == "T",
            "SCATTER MEMVAR BLANK should preserve datetime type metadata while leaving the value blank");
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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
        expect(copperfin::runtime::format_value(blank_array_birth_type->second) == "D",
            "SCATTER TO array BLANK should preserve date type metadata on blank elements");
    }
    if (blank_array_stamp_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(blank_array_stamp_type->second) == "T",
            "SCATTER TO array BLANK should preserve datetime type metadata on blank elements");
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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

void test_scatter_gather_memvar_like_and_except_field_filters() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_memvar_like_except";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "people.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 12U},
        {.name = "NOTE", .type = 'C', .length = 12U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{{"Alice", "Ready", "42"}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "SCATTER/GATHER MEMVAR LIKE/EXCEPT fixture should be created");

    const fs::path main_path = temp_root / "scatter_gather_memvar_like_except.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "' ALIAS People\n"
        "GO 1\n"
        "m.AGE = 900\n"
        "SCATTER FIELDS LIKE N* MEMVAR\n"
        "cLikeName = m.NAME\n"
        "cLikeNote = m.NOTE\n"
        "nLikeAge = m.AGE\n"
        "m.NAME = 'LikeName'\n"
        "m.NOTE = 'LikeNote'\n"
        "m.AGE = 901\n"
        "GATHER MEMVAR FIELDS EXCEPT AGE\n"
        "cAfterName = People.NAME\n"
        "cAfterNote = People.NOTE\n"
        "nAfterAge = People.AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER/GATHER MEMVAR LIKE/EXCEPT script should complete: " + state.message);

    const auto chk = [&](const std::string& var, const std::string& expected, const std::string& msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string val = copperfin::runtime::format_value(it->second);
            expect(val == expected, msg + " (got '" + val + "')");
        }
    };

    chk("clikename", "Alice", "SCATTER FIELDS LIKE N* MEMVAR should include keyword-heavy NAME");
    chk("clikenote", "Ready", "SCATTER FIELDS LIKE N* MEMVAR should include NOTE");
    chk("nlikeage", "900", "SCATTER FIELDS LIKE N* MEMVAR should preserve excluded preseeded AGE");
    chk("caftername", "LikeName", "GATHER MEMVAR FIELDS EXCEPT AGE should write NAME back");
    chk("cafternote", "LikeNote", "GATHER MEMVAR FIELDS EXCEPT AGE should write NOTE back");
    chk("nafterage", "42", "GATHER MEMVAR FIELDS EXCEPT AGE should leave AGE unchanged");

    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok, "GATHER MEMVAR LIKE/EXCEPT destination table should remain readable");
    if (persisted.ok && !persisted.table.records.empty()) {
        expect(persisted.table.records[0U].values[0U].display_value == "LikeName",
            "GATHER MEMVAR FIELDS EXCEPT AGE should persist NAME updates");
        expect(persisted.table.records[0U].values[1U].display_value == "LikeNote",
            "GATHER MEMVAR FIELDS EXCEPT AGE should persist NOTE updates");
        expect(persisted.table.records[0U].values[2U].display_value == "42",
            "GATHER MEMVAR FIELDS EXCEPT AGE should leave AGE unchanged on disk");
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
        "cObjectNameHolder = 'cObjectName'\n"
        "cObjectNameDeepHolder = 'cObjectNameHolder'\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "SCATTER FIELDS NAME, AGE NAME &cObjectNameDeepHolder ADDITIVE\n"
        "cAfterExtra = GETPEM(oRow, 'EXTRA')\n"
        "cAfterName = GETPEM(oRow, 'NAME')\n"
        "=SETPem(oRow, 'NAME', 'MacroObj')\n"
        "=SETPem(oRow, 'AGE', 55)\n"
        "GATHER NAME &cObjectNameDeepHolder FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
        "cHolderHolder = 'cHolder'\n"
        "cHolderDeepHolder = 'cHolderHolder'\n"
        "cChild = 'Row'\n"
        "cChildHolder = 'cChild'\n"
        "cChildDeepHolder = 'cChildHolder'\n"
        "oHolder = CREATEOBJECT('Empty')\n"
        "oRow = CREATEOBJECT('Empty')\n"
        "=ADDPROPERTY(oHolder, 'Row', oRow)\n"
        "=ADDPROPERTY(oRow, 'EXTRA', 'KeepMe')\n"
        "SCATTER FIELDS NAME, AGE NAME &cHolder.&cChild ADDITIVE\n"
        "SCATTER FIELDS NAME, AGE NAME &cHolderDeepHolder.&cChildDeepHolder ADDITIVE\n"
        "cAfterExtra = GETPEM(GETPEM(oHolder, 'Row'), 'EXTRA')\n"
        "cAfterName = GETPEM(GETPEM(oHolder, 'Row'), 'NAME')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'NAME', 'MacroNest')\n"
        "=SETPem(GETPEM(oHolder, 'Row'), 'AGE', 63)\n"
        "GATHER NAME &cHolderDeepHolder.&cChildDeepHolder FIELDS NAME, AGE\n"
        "cGatheredName = NAME\n"
        "nGatheredAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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

void test_gather_from_array_skips_memo_fields_by_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_array_skip_memo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "notes.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTES", .type = 'M', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Memo payload", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "GATHER array memo-skip fixture should be created");

    const fs::path main_path = temp_root / "gather_array_skip_memo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER TO aRow\n"
        "nRowLen = ALEN(aRow)\n"
        "cFirst = aRow[1]\n"
        "nSecond = aRow[2]\n"
        "aRow[1] = 'Updated'\n"
        "aRow[2] = 99\n"
        "GATHER FROM aRow\n"
        "cAfterName = NAME\n"
        "cAfterNotes = NOTES\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER FROM array with memo fields should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected, const std::string& message) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, message + " (got '" + actual + "')");
        }
    };

    check("nrowlen", "2", "SCATTER TO array should skip memo fields by default");
    check("cfirst", "Alice", "SCATTER TO array should keep NAME in the first slot");
    check("nsecond", "42", "SCATTER TO array should keep AGE in the second slot after skipping memo fields");
    check("caftername", "Updated", "GATHER FROM array should write NAME from the first array slot");
    check("cafternotes", "Memo payload", "GATHER FROM array should leave memo fields unchanged when no MEMO array data exists");
    check("nafterage", "99", "GATHER FROM array should write AGE from the second array slot after skipping memo fields");

    fs::remove_all(temp_root, ignored);
}

void test_gather_memvar_skips_memo_fields_by_default() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_memvar_skip_memo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "notes.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "NOTES", .type = 'M', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
    };
    const std::vector<std::vector<std::string>> records{
        {"Alice", "Memo payload", "42"},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "GATHER MEMVAR memo-skip fixture should be created");

    const fs::path main_path = temp_root / "gather_memvar_skip_memo.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "GO 1\n"
        "SCATTER MEMVAR\n"
        "cMemoType = VARTYPE(m.NOTES)\n"
        "m.NAME = 'Updated'\n"
        "m.AGE = 99\n"
        "GATHER MEMVAR\n"
        "cAfterName = NAME\n"
        "cAfterNotes = NOTES\n"
        "nAfterAge = AGE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER MEMVAR with memo fields should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected, const std::string& message) {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " should exist in globals");
        if (it != state.globals.end()) {
            const std::string actual = copperfin::runtime::format_value(it->second);
            expect(actual == expected, message + " (got '" + actual + "')");
        }
    };

    check("cmemotype", "U", "SCATTER MEMVAR without MEMO should leave memo memvars undefined");
    check("caftername", "Updated", "GATHER MEMVAR should write NAME from m.NAME");
    check("cafternotes", "Memo payload", "GATHER MEMVAR should leave memo fields unchanged when no memo memvar exists");
    check("nafterage", "99", "GATHER MEMVAR should write AGE from m.AGE");

    fs::remove_all(temp_root, ignored);
}

void test_scatter_memvar_blank_on_empty_table_succeeds() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_blank_empty";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path table_path = temp_root / "empty.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 10U},
        {.name = "AGE", .type = 'N', .length = 3U},
        {.name = "ACTIVE", .type = 'L', .length = 1U},
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {});
    expect(create_result.ok, "SCATTER BLANK empty-table fixture should be created with no records");

    const fs::path main_path = temp_root / "scatter_blank_empty.prg";
    write_text(
        main_path,
        "USE '" + table_path.string() + "'\n"
        "SCATTER MEMVAR BLANK\n"
        "cNameType = VARTYPE(m.NAME)\n"
        "cAgeType = VARTYPE(m.AGE)\n"
        "cActiveType = VARTYPE(m.ACTIVE)\n"
        "nBlankAge = m.AGE\n"
        "lBlankActive = m.ACTIVE\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER MEMVAR BLANK on empty table should complete: " + state.message);

    const auto chk = [&](const std::string &var, const std::string &expected, const std::string &msg) {
        const auto it = state.globals.find(var);
        expect(it != state.globals.end(), var + " should exist in globals");
        if (it != state.globals.end()) {
            expect(copperfin::runtime::format_value(it->second) == expected, msg + " (got '" + copperfin::runtime::format_value(it->second) + "')");
        }
    };

    chk("cnametype", "C", "SCATTER BLANK on empty table should produce character memvar for NAME");
    chk("cagetype", "N", "SCATTER BLANK on empty table should produce numeric memvar for AGE");
    chk("cactivetype", "L", "SCATTER BLANK on empty table should produce logical memvar for ACTIVE");
    chk("nblankage", "0", "SCATTER BLANK numeric memvar should be zero");
    chk("lblankactive", "false", "SCATTER BLANK logical memvar should be false");

    fs::remove_all(temp_root, ignored);
}

void test_scatter_gather_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scatter_gather_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path scatter_no_area_path = temp_root / "scatter_no_area.prg";
    write_text(
        scatter_no_area_path,
        "SCATTER MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession scatter_no_area_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(scatter_no_area_path.string(), temp_root.string(), false));
    const auto scatter_no_area_state = scatter_no_area_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!scatter_no_area_state.completed, "#2712: qps-ploc SCATTER without a work area should fail");
    expect(
        scatter_no_area_state.message == copperfin::localization::pseudo_localize("SCATTER: no current work area"),
        "#2712: qps-ploc SCATTER no-work-area error should route through the pseudo-localization transform");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .length = 8U},
    };
    const auto empty_create =
        copperfin::vfp::create_dbf_table_file((temp_root / "empty.dbf").string(), fields, {});
    expect(empty_create.ok, "#2712: empty SCATTER/GATHER fixture should be created");
    const auto one_row_create =
        copperfin::vfp::create_dbf_table_file((temp_root / "one_row.dbf").string(), fields, {{"Alice"}});
    expect(one_row_create.ok, "#2712: one-row SCATTER/GATHER fixture should be created");

    const fs::path scatter_no_record_path = temp_root / "scatter_no_record.prg";
    write_text(
        scatter_no_record_path,
        "USE '" + (temp_root / "empty.dbf").string() + "'\n"
        "SCATTER MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession scatter_no_record_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(scatter_no_record_path.string(), temp_root.string(), false));
    const auto scatter_no_record_state = scatter_no_record_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!scatter_no_record_state.completed, "#2712: qps-ploc SCATTER without a current record should fail");
    expect(
        scatter_no_record_state.message == copperfin::localization::pseudo_localize("SCATTER: no current record"),
        "#2712: qps-ploc SCATTER no-current-record error should route through the pseudo-localization transform");

    const fs::path scatter_fields_path = temp_root / "scatter_fields_fail.prg";
    write_text(
        scatter_fields_path,
        "USE '" + (temp_root / "one_row.dbf").string() + "'\n"
        "SCATTER FIELDS MissingField MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession scatter_fields_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(scatter_fields_path.string(), temp_root.string(), false));
    const auto scatter_fields_state = scatter_fields_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!scatter_fields_state.completed, "#2712: qps-ploc SCATTER with no matching fields should fail");
    expect(
        scatter_fields_state.message ==
            copperfin::localization::pseudo_localize("SCATTER: no fields match the FIELDS clause"),
        "#2712: qps-ploc SCATTER empty-fields error should route through the pseudo-localization transform");

    const fs::path gather_no_area_path = temp_root / "gather_no_area.prg";
    write_text(
        gather_no_area_path,
        "GATHER MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession gather_no_area_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(gather_no_area_path.string(), temp_root.string(), false));
    const auto gather_no_area_state = gather_no_area_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!gather_no_area_state.completed, "#2712: qps-ploc GATHER without a work area should fail");
    expect(
        gather_no_area_state.message == copperfin::localization::pseudo_localize("GATHER: no current work area"),
        "#2712: qps-ploc GATHER no-work-area error should route through the pseudo-localization transform");

    const fs::path gather_no_record_path = temp_root / "gather_no_record.prg";
    write_text(
        gather_no_record_path,
        "USE '" + (temp_root / "empty.dbf").string() + "'\n"
        "GATHER MEMVAR\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession gather_no_record_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(gather_no_record_path.string(), temp_root.string(), false));
    const auto gather_no_record_state = gather_no_record_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!gather_no_record_state.completed, "#2712: qps-ploc GATHER without a current record should fail");
    expect(
        gather_no_record_state.message == copperfin::localization::pseudo_localize("GATHER: no current record"),
        "#2712: qps-ploc GATHER no-current-record error should route through the pseudo-localization transform");

    const fs::path gather_name_path = temp_root / "gather_name_missing_object.prg";
    write_text(
        gather_name_path,
        "USE '" + (temp_root / "one_row.dbf").string() + "'\n"
        "GATHER NAME oMissing FIELDS NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession gather_name_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(gather_name_path.string(), temp_root.string(), false));
    const auto gather_name_state = gather_name_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!gather_name_state.completed, "#2712: qps-ploc GATHER NAME with a missing object variable should fail");
    expect(
        gather_name_state.message ==
            copperfin::localization::pseudo_localize("GATHER NAME: object variable not found"),
        "#2712: qps-ploc GATHER NAME missing-object error should route through the pseudo-localization transform");

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

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

void test_gather_from_array_is_reverted_by_undo() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_gather_undo";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_simple_dbf(temp_root / "table.dbf", {"Alice", "Bob"});

    const fs::path main_path = temp_root / "gather_undo.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "table.dbf").string() + "'\n"
        "GO 1\n"
        "SCATTER FIELDS NAME TO aRow\n"
        "aRow[1] = 'Changed'\n"
        "GATHER FROM aRow FIELDS NAME\n"
        "UNDO\n"
        "GO 1\n"
        "cName = NAME\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "GATHER FROM array + UNDO script should complete: " + state.message);
    expect(has_runtime_event(state.events, "runtime.command_undo", "LATEST"),
        "UNDO after GATHER should emit a runtime.command_undo event");

    const auto it = state.globals.find("cname");
    expect(it != state.globals.end(), "cName variable should exist after UNDO");
    if (it != state.globals.end()) {
        const std::string actual = copperfin::runtime::format_value(it->second);
        expect(actual == "Alice",
            "UNDO should revert a GATHER FROM array write back to the pre-command NAME value (got '" + actual + "')");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io
