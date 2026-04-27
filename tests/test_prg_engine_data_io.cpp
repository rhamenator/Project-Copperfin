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
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "SCATTER TO array / GATHER FROM array script should complete");

    const auto array_len = state.globals.find("narraylen");
    const auto rows = state.globals.find("nrows");
    const auto cols = state.globals.find("ncols");
    const auto first = state.globals.find("cfirst");
    const auto second_plus = state.globals.find("nsecondplus");
    const auto after_name = state.globals.find("caftername");
    const auto after_age = state.globals.find("nafterage");

    expect(array_len != state.globals.end(), "ALEN(aRow) should expose array element count");
    expect(rows != state.globals.end(), "ALEN(aRow, 1) should expose first dimension");
    expect(cols != state.globals.end(), "ALEN(aRow, 2) should expose second dimension");
    expect(first != state.globals.end(), "aRow[1] should read the first scattered value");
    expect(second_plus != state.globals.end(), "aRow(2) should read the second scattered value");
    expect(after_name != state.globals.end(), "GATHER FROM array should restore NAME");
    expect(after_age != state.globals.end(), "GATHER FROM array should restore AGE");

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
        "row1_name = myarr[1, 1]\n"
        "row1_age = myarr[1, 2]\n"
        "row2_name = myarr[2, 1]\n"
        "row2_age = myarr[2, 2]\n"
        "arr_rows = ALEN(myarr, 1)\n"
        "arr_cols = ALEN(myarr, 2)\n"
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
        "COPY TO ARRAY tmparr\n"
        "USE '" + (temp_root / "dest.dbf").string() + "'\n"
        "APPEND FROM ARRAY tmparr\n"
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

}  // namespace

int main() {
    test_scatter_memvar_from_current_record();
    test_scatter_gather_memvar_fields_blank_and_for_semantics();
    test_scatter_to_array_and_gather_from_array_round_trip();
    test_runtime_array_mutator_functions();
    test_copy_to_emits_event();
    test_copy_to_creates_destination_dbf();
    test_copy_structure_to_creates_empty_schema();
    test_append_from_copies_records_into_current_table();
    test_copy_to_type_sdf_writes_fixed_width_text_rows();
    test_append_from_type_sdf_imports_fixed_width_text_rows();
    test_copy_to_type_csv_and_delimited_text_rows();
    test_append_from_type_csv_imports_delimited_rows();
    test_copy_to_array_fills_2d_runtime_array();
    test_append_from_array_writes_records_from_2d_array();
    test_gather_memvar_round_trips_field_values();
    test_m_dot_namespace_shares_bare_memory_variable_binding();
    test_browse_emits_effective_cursor_view_metadata();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
