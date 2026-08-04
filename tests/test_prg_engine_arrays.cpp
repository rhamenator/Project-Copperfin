// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

void test_ascan_matches_object_references_and_nulls_exactly() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_ascan_identity";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ascan_identity.prg";
    write_text(
        main_path,
        "SET EXACT OFF\n"
        "oTarget = CREATEOBJECT('Empty')\n"
        "oFill2 = CREATEOBJECT('Empty')\n"
        "oFill3 = CREATEOBJECT('Empty')\n"
        "oFill4 = CREATEOBJECT('Empty')\n"
        "oFill5 = CREATEOBJECT('Empty')\n"
        "oFill6 = CREATEOBJECT('Empty')\n"
        "oFill7 = CREATEOBJECT('Empty')\n"
        "oFill8 = CREATEOBJECT('Empty')\n"
        "oFill9 = CREATEOBJECT('Empty')\n"
        "oFill10 = CREATEOBJECT('Empty')\n"
        "oFill11 = CREATEOBJECT('Empty')\n"
        "oFill12 = CREATEOBJECT('Empty')\n"
        "oFill13 = CREATEOBJECT('Empty')\n"
        "oFill14 = CREATEOBJECT('Empty')\n"
        "oFill15 = CREATEOBJECT('Empty')\n"
        "oFill16 = CREATEOBJECT('Empty')\n"
        "oFill17 = CREATEOBJECT('Empty')\n"
        "oFill18 = CREATEOBJECT('Empty')\n"
        "DIMENSION aObjects[2], aValues[2]\n"
        "aObjects[1] = CREATEOBJECT('Empty')\n"
        "aObjects[2] = oTarget\n"
        "aValues[1] = .NULL.\n"
        "aValues[2] = 0\n"
        "cFirstObject = aObjects[1]\n"
        "nObjectMatch = ASCAN(aObjects, oTarget)\n"
        "nZeroMatch = ASCAN(aValues, 0)\n"
        "nNullMatch = ASCAN(aValues, .NULL.)\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASCAN identity script should complete");

    const auto target = state.globals.find("otarget");
    const auto first_object = state.globals.find("cfirstobject");
    const auto object_match = state.globals.find("nobjectmatch");
    const auto zero_match = state.globals.find("nzeromatch");
    const auto null_match = state.globals.find("nnullmatch");
    expect(target != state.globals.end(), "ASCAN object reference target should be captured");
    expect(first_object != state.globals.end(), "ASCAN object reference candidate should be captured");
    expect(object_match != state.globals.end(), "ASCAN object-reference result should be captured");
    expect(zero_match != state.globals.end(), "ASCAN zero result should be captured");
    expect(null_match != state.globals.end(), "ASCAN null result should be captured");
    if (target != state.globals.end() && first_object != state.globals.end()) {
        const std::string target_value = copperfin::runtime::format_value(target->second);
        const std::string first_object_value = copperfin::runtime::format_value(first_object->second);
        expect(first_object_value != target_value,
               "ASCAN identity regression must create a distinct object reference");
    }
    if (object_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(object_match->second) == "2",
               "ASCAN must compare live object references by exact identity when SET EXACT is OFF");
    }
    if (zero_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(zero_match->second) == "2",
               "ASCAN must not match .NULL. while searching for numeric zero");
    }
    if (null_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(null_match->second) == "1",
               "ASCAN must match .NULL. only against another null value");
    }

    fs::remove_all(temp_root, ignored);
}

void test_aelement_single_subscript_uses_linear_index() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_aelement_linear";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aelement_linear.prg";
    write_text(
        main_path,
        "DIMENSION aValues[3,2]\n"
        "nSecond = AELEMENT(aValues, 2)\n"
        "nFourth = AELEMENT(aValues, 4)\n"
        "nOutOfRange = AELEMENT(aValues, 7)\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AELEMENT linear-index script should complete");

    const auto second = state.globals.find("nsecond");
    const auto fourth = state.globals.find("nfourth");
    const auto out_of_range = state.globals.find("noutofrange");
    expect(second != state.globals.end(), "AELEMENT linear index 2 should be captured");
    expect(fourth != state.globals.end(), "AELEMENT linear index 4 should be captured");
    expect(out_of_range != state.globals.end(), "AELEMENT out-of-range result should be captured");
    if (second != state.globals.end()) {
        expect(copperfin::runtime::format_value(second->second) == "2",
               "AELEMENT single-subscript form should preserve a valid linear index");
    }
    if (fourth != state.globals.end()) {
        expect(copperfin::runtime::format_value(fourth->second) == "4",
               "AELEMENT single-subscript form should not interpret a linear index as a row number");
    }
    if (out_of_range != state.globals.end()) {
        expect(copperfin::runtime::format_value(out_of_range->second) == "0",
               "AELEMENT single-subscript form should reject indexes beyond the array size");
    }

    fs::remove_all(temp_root, ignored);
}

void test_asort_order_values_follow_vfp_contract() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_asort_order";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "asort_order.prg";
    write_text(
        main_path,
        "DIMENSION aAscending[3], aInsensitive[3], aDescending[3]\n"
        "aAscending[1] = 'B'\n"
        "aAscending[2] = 'A'\n"
        "aAscending[3] = 'C'\n"
        "nAscending = ASORT(aAscending, 1, -1, 1)\n"
        "cAscending = aAscending[1] + aAscending[2] + aAscending[3]\n"
        "aInsensitive[1] = 'b'\n"
        "aInsensitive[2] = 'A'\n"
        "aInsensitive[3] = 'c'\n"
        "nInsensitive = ASORT(aInsensitive, 1, -1, 3)\n"
        "cInsensitive = aInsensitive[1] + aInsensitive[2] + aInsensitive[3]\n"
        "aDescending[1] = 'a'\n"
        "aDescending[2] = 'B'\n"
        "aDescending[3] = 'c'\n"
        "nDescending = ASORT(aDescending, 1, -1, 4)\n"
        "cDescending = aDescending[1] + aDescending[2] + aDescending[3]\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASORT order-value script should complete");

    const auto ascending = state.globals.find("cascending");
    const auto insensitive = state.globals.find("cinsensitive");
    const auto descending = state.globals.find("cdescending");
    expect(ascending != state.globals.end(), "ASORT order-1 result should be captured");
    expect(insensitive != state.globals.end(), "ASORT order-3 result should be captured");
    expect(descending != state.globals.end(), "ASORT order-4 result should be captured");
    if (ascending != state.globals.end()) {
        expect(copperfin::runtime::format_value(ascending->second) == "ABC",
               "ASORT order 1 should sort ascending case-sensitively");
    }
    if (insensitive != state.globals.end()) {
        expect(copperfin::runtime::format_value(insensitive->second) == "Abc",
               "ASORT order 3 should sort ascending case-insensitively");
    }
    if (descending != state.globals.end()) {
        expect(copperfin::runtime::format_value(descending->second) == "cBa",
               "ASORT order 4 should sort descending case-insensitively");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ascan_column_start_uses_column_relative_row() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_ascan_column_start";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ascan_column_start.prg";
    write_text(
        main_path,
        "DIMENSION aValues[4,2]\n"
        "aValues[1,2] = 'X'\n"
        "aValues[2,2] = 'M'\n"
        "aValues[3,2] = 'Z'\n"
        "aValues[4,2] = 'Q'\n"
        "nExcludedMatch = ASCAN(aValues, 'M', 3, 2, 2)\n"
        "nWindowMatch = ASCAN(aValues, 'Z', 3, 2, 2)\n"
        "nWindowRow = ASCAN(aValues, 'Z', 3, 2, 2, 8)\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASCAN column-relative start script should complete");

    const auto excluded_match = state.globals.find("nexcludedmatch");
    const auto window_match = state.globals.find("nwindowmatch");
    const auto window_row = state.globals.find("nwindowrow");
    expect(excluded_match != state.globals.end(), "ASCAN column-relative excluded-match result should be captured");
    expect(window_match != state.globals.end(), "ASCAN column-relative window-match result should be captured");
    expect(window_row != state.globals.end(), "ASCAN column-relative row result should be captured");
    if (excluded_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(excluded_match->second) == "0",
               "ASCAN column-relative start should exclude matches before the requested row");
    }
    if (window_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(window_match->second) == "6",
               "ASCAN column-relative start should return the matched linear element index");
    }
    if (window_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(window_row->second) == "3",
               "ASCAN column-relative start should retain the matched row return mode");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ascan_predicate_expression_search() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_ascan_predicate";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ascan_predicate.prg";
    write_text(
        main_path,
        "DIMENSION aValues[4]\n"
        "aValues[1] = 1\n"
        "aValues[2] = 4\n"
        "aValues[3] = 7\n"
        "aValues[4] = 10\n"
        "nGreaterThanFive = ASCAN(aValues, '{|x| x > 5}', -1, -1, -1, 16)\n"
        "nUnquotedBlock = ASCAN(aValues, {|x| x > 8}, -1, -1, -1, 16)\n"
        "nIndexPredicate = ASCAN(aValues, '_ASCANINDEX = 2', -1, -1, -1, 16)\n"
        "nNoMatch = ASCAN(aValues, '{|x| x > 100}', -1, -1, -1, 16)\n"
        "lMetadataCleared = TYPE('_ASCANVALUE') = 'U'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASCAN predicate-expression script should complete");

    const auto greater_than_five = state.globals.find("ngreaterthanfive");
    const auto unquoted_block = state.globals.find("nunquotedblock");
    const auto index_predicate = state.globals.find("nindexpredicate");
    const auto no_match = state.globals.find("nnomatch");
    const auto metadata_cleared = state.globals.find("lmetadatacleared");

    expect(greater_than_five != state.globals.end(), "ASCAN block-style predicate result should be captured");
    expect(unquoted_block != state.globals.end(), "ASCAN unquoted block-style predicate result should be captured");
    expect(index_predicate != state.globals.end(), "ASCAN metadata predicate result should be captured");
    expect(no_match != state.globals.end(), "ASCAN no-match predicate result should be captured");
    expect(metadata_cleared != state.globals.end(), "ASCAN predicate metadata cleanup flag should be captured");

    if (greater_than_five != state.globals.end()) {
        expect(copperfin::runtime::format_value(greater_than_five->second) == "3",
            "ASCAN predicate block should return the first element whose value satisfies the expression");
    }
    if (unquoted_block != state.globals.end()) {
        expect(copperfin::runtime::format_value(unquoted_block->second) == "4",
            "ASCAN should accept an unquoted block-style predicate argument");
    }
    if (index_predicate != state.globals.end()) {
        expect(copperfin::runtime::format_value(index_predicate->second) == "2",
            "ASCAN predicate metadata should expose the one-based scan index");
    }
    if (no_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(no_match->second) == "0",
            "ASCAN predicate search should return 0 when no value matches");
    }
    if (metadata_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(metadata_cleared->second) == "true",
            "ASCAN predicate metadata variables should be restored after scanning");
    }

    fs::remove_all(temp_root, ignored);
}

void test_acopy_two_dimensional_row_and_column_workflows() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_acopy_2d";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "acopy_2d.prg";
    write_text(
        main_path,
        "DIMENSION aSource[3,3]\n"
        "DIMENSION aRows[1,3]\n"
        "DIMENSION aColumns[3]\n"
        "aSource[1,1] = 'A'\n"
        "aSource[1,2] = 'B'\n"
        "aSource[1,3] = 'C'\n"
        "aSource[2,1] = 'D'\n"
        "aSource[2,2] = 'E'\n"
        "aSource[2,3] = 'F'\n"
        "aSource[3,1] = 'G'\n"
        "aSource[3,2] = 'H'\n"
        "aSource[3,3] = 'I'\n"
        "nRowCopy = ACOPY(aSource, aRows, AELEMENT(aSource, 2, 1), ALEN(aSource, 2), 1)\n"
        "nColumnCopy1 = ACOPY(aSource, aColumns, AELEMENT(aSource, 1, 2), 1, 1)\n"
        "nColumnCopy2 = ACOPY(aSource, aColumns, AELEMENT(aSource, 2, 2), 1, 2)\n"
        "nColumnCopy3 = ACOPY(aSource, aColumns, AELEMENT(aSource, 3, 2), 1, 3)\n"
        "cCopiedRow = aRows[1,1] + aRows[1,2] + aRows[1,3]\n"
        "cCopiedColumn = aColumns[1] + aColumns[2] + aColumns[3]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ACOPY two-dimensional workflow script should complete");

    const auto row_copy = state.globals.find("nrowcopy");
    const auto column_copy1 = state.globals.find("ncolumncopy1");
    const auto column_copy2 = state.globals.find("ncolumncopy2");
    const auto column_copy3 = state.globals.find("ncolumncopy3");
    const auto copied_row = state.globals.find("ccopiedrow");
    const auto copied_column = state.globals.find("ccopiedcolumn");

    expect(row_copy != state.globals.end(), "ACOPY row-copy count should be captured");
    expect(column_copy1 != state.globals.end(), "ACOPY first column-copy count should be captured");
    expect(column_copy2 != state.globals.end(), "ACOPY second column-copy count should be captured");
    expect(column_copy3 != state.globals.end(), "ACOPY third column-copy count should be captured");
    expect(copied_row != state.globals.end(), "ACOPY copied-row value should be captured");
    expect(copied_column != state.globals.end(), "ACOPY copied-column value should be captured");

    if (row_copy != state.globals.end()) {
        expect(copperfin::runtime::format_value(row_copy->second) == "3",
            "ACOPY should copy a whole row when AELEMENT() supplies the row start and ALEN(..., 2) supplies the width");
    }
    if (column_copy1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(column_copy1->second) == "1",
            "ACOPY should copy one source-column element into a one-dimensional column helper target");
    }
    if (column_copy2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(column_copy2->second) == "1",
            "ACOPY should copy the second source-column element into a one-dimensional column helper target");
    }
    if (column_copy3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(column_copy3->second) == "1",
            "ACOPY should copy the third source-column element into a one-dimensional column helper target");
    }
    if (copied_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_row->second) == "DEF",
            "ACOPY row workflow should preserve source row order");
    }
    if (copied_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_column->second) == "BEH",
            "ACOPY column workflow should preserve source column order");
    }

    fs::remove_all(temp_root, ignored);
}

void test_acopy_clamps_to_existing_target_capacity() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_acopy_clamp";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "acopy_clamp.prg";
    write_text(
        main_path,
        "DIMENSION aSource[5]\n"
        "DIMENSION aTarget[2]\n"
        "aSource[1] = 'A'\n"
        "aSource[2] = 'B'\n"
        "aSource[3] = 'C'\n"
        "aSource[4] = 'D'\n"
        "aSource[5] = 'E'\n"
        "nCopied = ACOPY(aSource, aTarget)\n"
        "nTargetSize = ALEN(aTarget)\n"
        "cTargetOne = aTarget[1]\n"
        "cTargetTwo = aTarget[2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ACOPY clamp script should complete");

    const auto copied = state.globals.find("ncopied");
    const auto target_size = state.globals.find("ntargetsize");
    const auto target_one = state.globals.find("ctargetone");
    const auto target_two = state.globals.find("ctargettwo");

    expect(copied != state.globals.end(), "ACOPY clamp copy count should be captured");
    expect(target_size != state.globals.end(), "ACOPY clamp target size should be captured");
    expect(target_one != state.globals.end(), "ACOPY clamp first copied value should be captured");
    expect(target_two != state.globals.end(), "ACOPY clamp second copied value should be captured");

    if (copied != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied->second) == "2",
            "ACOPY should report only the number of elements that fit in an existing target array");
    }
    if (target_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_size->second) == "2",
            "ACOPY should not resize an existing target array as a side effect");
    }
    if (target_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_one->second) == "A",
            "ACOPY clamp behavior should preserve the first copied element");
    }
    if (target_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(target_two->second) == "B",
            "ACOPY clamp behavior should preserve the second copied element");
    }

    fs::remove_all(temp_root, ignored);
}

void test_array_dimension_and_element_assignment() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_assignment";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "array_assignment.prg";
    write_text(
        main_path,
        "DIMENSION aGrid[2,3], aList(2)\n"
        "DECLARE aDeclared[1,2]\n"
        "aGrid[1,1] = 'A'\n"
        "aGrid[1,2] = 'B'\n"
        "aGrid(2,3) = 'F'\n"
        "aDeclared[1,2] = 'D'\n"
        "aList[1] = 10\n"
        "aList(2) = 15\n"
        "nGridRows = ALEN(aGrid, 1)\n"
        "nGridCols = ALEN(aGrid, 2)\n"
        "nDeclaredCols = ALEN(aDeclared, 2)\n"
        "cGridA = aGrid[1,1]\n"
        "cGridB = aGrid(1,2)\n"
        "cGridF = aGrid[2,3]\n"
        "cDeclared = aDeclared[1,2]\n"
        "nListSum = aList[1] + aList(2)\n"
        "nResize = ASIZE(aGrid, 3, 4)\n"
        "cGridFAfterResize = aGrid[2,3]\n"
        "aGrid[3,4] = 'L'\n"
        "cGridL = aGrid(3,4)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array declaration and element assignment script should complete");

    const auto rows = state.globals.find("ngridrows");
    const auto cols = state.globals.find("ngridcols");
    const auto declared_cols = state.globals.find("ndeclaredcols");
    const auto grid_a = state.globals.find("cgrida");
    const auto grid_b = state.globals.find("cgridb");
    const auto grid_f = state.globals.find("cgridf");
    const auto declared = state.globals.find("cdeclared");
    const auto list_sum = state.globals.find("nlistsum");
    const auto resize = state.globals.find("nresize");
    const auto grid_f_after_resize = state.globals.find("cgridfafterresize");
    const auto grid_l = state.globals.find("cgridl");

    expect(rows != state.globals.end(), "DIMENSION should expose row count through ALEN");
    expect(cols != state.globals.end(), "DIMENSION should expose column count through ALEN");
    expect(declared_cols != state.globals.end(), "DECLARE should expose column count through ALEN");
    expect(grid_a != state.globals.end(), "bracket array assignment should be readable");
    expect(grid_b != state.globals.end(), "paren array read should be readable");
    expect(grid_f != state.globals.end(), "paren array assignment should be readable through bracket syntax");
    expect(declared != state.globals.end(), "DECLARE array assignment should be readable");
    expect(list_sum != state.globals.end(), "one-dimensional array assignment should support arithmetic reads");
    expect(resize != state.globals.end(), "ASIZE should resize declared 2D arrays");
    expect(grid_f_after_resize != state.globals.end(), "ASIZE should preserve existing 2D values");
    expect(grid_l != state.globals.end(), "array assignment should write newly grown 2D cells");

    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "2", "DIMENSION aGrid[2,3] should create two rows");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "3", "DIMENSION aGrid[2,3] should create three columns");
    }
    if (declared_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(declared_cols->second) == "2", "DECLARE aDeclared[1,2] should create two columns");
    }
    if (grid_a != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_a->second) == "A", "aGrid[1,1] should contain A");
    }
    if (grid_b != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_b->second) == "B", "aGrid(1,2) should contain B");
    }
    if (grid_f != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_f->second) == "F", "aGrid[2,3] should contain F");
    }
    if (declared != state.globals.end()) {
        expect(copperfin::runtime::format_value(declared->second) == "D", "aDeclared[1,2] should contain D");
    }
    if (list_sum != state.globals.end()) {
        expect(copperfin::runtime::format_value(list_sum->second) == "25", "array element reads should participate in arithmetic");
    }
    if (resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(resize->second) == "12", "ASIZE(aGrid, 3, 4) should report twelve elements");
    }
    if (grid_f_after_resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_f_after_resize->second) == "F", "ASIZE should preserve existing 2D cells");
    }
    if (grid_l != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_l->second) == "L", "aGrid[3,4] should contain L after growth");
    }

    fs::remove_all(temp_root, ignored);
}

void test_preprocessor_constants_expand_in_array_subscripts_but_not_bracket_literals() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_preprocessor_dimensions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_text(
        temp_root / "array_constants.h",
        "#DEFINE MAX_ROWS 2\n"
        "#DEFINE MAX_COLUMNS 3\n");
    const fs::path main_path = temp_root / "array_constants.prg";
    write_text(
        main_path,
        "#INCLUDE \"array_constants.h\"\n"
        "DIMENSION aValues[MAX_ROWS,MAX_COLUMNS]\n"
        "aValues[2,3] = 'ok'\n"
        "nRows = ALEN(aValues, 1)\n"
        "nColumns = ALEN(aValues, 2)\n"
        "cBracketLiteral = [MAX_ROWS]\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array dimensions should expand included preprocessor constants");

    const auto rows = state.globals.find("nrows");
    const auto columns = state.globals.find("ncolumns");
    const auto literal = state.globals.find("cbracketliteral");
    expect(rows != state.globals.end(), "preprocessor array-dimension rows should be observable");
    expect(columns != state.globals.end(), "preprocessor array-dimension columns should be observable");
    expect(literal != state.globals.end(), "bracket literals should remain observable");
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "2",
               "included MAX_ROWS should expand inside an array subscript");
    }
    if (columns != state.globals.end()) {
        expect(copperfin::runtime::format_value(columns->second) == "3",
               "included MAX_COLUMNS should expand inside an array subscript");
    }
    if (literal != state.globals.end()) {
        expect(copperfin::runtime::format_value(literal->second) == "MAX_ROWS",
               "bracket string literals must not expand preprocessor constants");
    }

    fs::remove_all(temp_root, ignored);
}

void test_asize_two_argument_form_preserves_existing_column_count() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_asize_preserves_columns";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "asize_preserves_columns.prg";
    write_text(
        main_path,
        "DIMENSION aGrid[2,2]\n"
        "aGrid[1,1] = 'A'\n"
        "aGrid[1,2] = 'B'\n"
        "aGrid[2,1] = 'C'\n"
        "aGrid[2,2] = 'D'\n"
        "nResize = ASIZE(aGrid, 3)\n"
        "nRows = ALEN(aGrid, 1)\n"
        "nCols = ALEN(aGrid, 2)\n"
        "cPreservedOne = aGrid[1,2]\n"
        "cPreservedTwo = aGrid[2,2]\n"
        "aGrid[3,2] = 'E'\n"
        "cNewValue = aGrid[3,2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASIZE two-argument preservation script should complete");

    const auto resize = state.globals.find("nresize");
    const auto rows = state.globals.find("nrows");
    const auto cols = state.globals.find("ncols");
    const auto preserved_one = state.globals.find("cpreservedone");
    const auto preserved_two = state.globals.find("cpreservedtwo");
    const auto new_value = state.globals.find("cnewvalue");

    expect(resize != state.globals.end(), "ASIZE two-argument form should report the new element count");
    expect(rows != state.globals.end(), "ASIZE two-argument form should preserve ALEN row metadata");
    expect(cols != state.globals.end(), "ASIZE two-argument form should preserve ALEN column metadata");
    expect(preserved_one != state.globals.end(), "ASIZE two-argument form should preserve existing second-column values");
    expect(preserved_two != state.globals.end(), "ASIZE two-argument form should preserve lower-row second-column values");
    expect(new_value != state.globals.end(), "ASIZE two-argument form should keep the second column writable after growth");

    if (resize != state.globals.end()) {
        expect(copperfin::runtime::format_value(resize->second) == "6",
               "ASIZE(aGrid, 3) should preserve a two-column array shape and report six elements");
    }
    if (rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(rows->second) == "3",
               "ASIZE(aGrid, 3) should grow the array to three rows");
    }
    if (cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(cols->second) == "2",
               "ASIZE(aGrid, 3) should preserve the existing two-column shape");
    }
    if (preserved_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(preserved_one->second) == "B",
               "ASIZE(aGrid, 3) should preserve row-one second-column values");
    }
    if (preserved_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(preserved_two->second) == "D",
               "ASIZE(aGrid, 3) should preserve row-two second-column values");
    }
    if (new_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(new_value->second) == "E",
               "ASIZE(aGrid, 3) should keep the preserved second column writable on new rows");
    }

    fs::remove_all(temp_root, ignored);
}

void test_array_metadata_and_text_functions() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_metadata";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_people_dbf(temp_root / "people.dbf", {{"Alice", 41}});
    write_text(temp_root / "alpha.txt", "abc");
    write_text(temp_root / "beta.bin", "not matched");

    const fs::path main_path = temp_root / "array_metadata.prg";
    write_text(
        main_path,
        "USE '" + (temp_root / "people.dbf").string() + "' ALIAS People\n"
        "nUsedCount = AUSED(aUsed)\n"
        "nUsedCols = ALEN(aUsed, 2)\n"
        "cUsedAlias = aUsed[1,1]\n"
        "nUsedArea = aUsed[1,2]\n"
        "nLineCount = ALINES(aLines, ' red ' + CHR(13) + CHR(10) + 'blue', 1)\n"
        "cLineOne = aLines[1]\n"
        "cLineTwo = aLines[2]\n"
        "nFileCount = ADIR(aFiles, '" + (temp_root / "*.txt").string() + "')\n"
        "cFileName = aFiles[1,1]\n"
        "nFileSize = aFiles[1,2]\n"
        "cFileAttr = aFiles[1,5]\n"
        "nFieldCount = AFIELDS(aFields)\n"
        "nFieldCols = ALEN(aFields, 2)\n"
        "cFieldOneName = aFields[1,1]\n"
        "cFieldOneType = aFields[1,2]\n"
        "nFieldOneWidth = aFields[1,3]\n"
        "cFieldTwoName = aFields[2,1]\n"
        "cFieldTwoType = aFields[2,2]\n"
        "nFieldTwoWidth = aFields[2,3]\n"
        "nFieldTwoDecimals = aFields[2,4]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "array metadata and text function script should complete");

    const auto line_count = state.globals.find("nlinecount");
    const auto used_count = state.globals.find("nusedcount");
    const auto used_cols = state.globals.find("nusedcols");
    const auto used_alias = state.globals.find("cusedalias");
    const auto used_area = state.globals.find("nusedarea");
    const auto line_one = state.globals.find("clineone");
    const auto line_two = state.globals.find("clinetwo");
    const auto file_count = state.globals.find("nfilecount");
    const auto file_name = state.globals.find("cfilename");
    const auto file_size = state.globals.find("nfilesize");
    const auto file_attr = state.globals.find("cfileattr");
    const auto field_count = state.globals.find("nfieldcount");
    const auto field_cols = state.globals.find("nfieldcols");
    const auto field_one_name = state.globals.find("cfieldonename");
    const auto field_one_type = state.globals.find("cfieldonetype");
    const auto field_one_width = state.globals.find("nfieldonewidth");
    const auto field_two_name = state.globals.find("cfieldtwoname");
    const auto field_two_type = state.globals.find("cfieldtwotype");
    const auto field_two_width = state.globals.find("nfieldtwowidth");
    const auto field_two_decimals = state.globals.find("nfieldtwodecimals");

    expect(line_count != state.globals.end(), "ALINES should return a count");
    expect(used_count != state.globals.end(), "AUSED should return an open alias count");
    expect(used_cols != state.globals.end(), "AUSED should expose its metadata column count");
    expect(used_alias != state.globals.end(), "AUSED should populate the open alias");
    expect(used_area != state.globals.end(), "AUSED should populate the open work area");
    expect(line_one != state.globals.end(), "ALINES should populate first line");
    expect(line_two != state.globals.end(), "ALINES should populate second line");
    expect(file_count != state.globals.end(), "ADIR should return a count");
    expect(file_name != state.globals.end(), "ADIR should populate file name");
    expect(file_size != state.globals.end(), "ADIR should populate file size");
    expect(file_attr != state.globals.end(), "ADIR should populate attribute column");
    expect(field_count != state.globals.end(), "AFIELDS should return a field count");
    expect(field_cols != state.globals.end(), "AFIELDS should expose its metadata column count");
    expect(field_one_name != state.globals.end(), "AFIELDS should populate first field name");
    expect(field_one_type != state.globals.end(), "AFIELDS should populate first field type");
    expect(field_one_width != state.globals.end(), "AFIELDS should populate first field width");
    expect(field_two_name != state.globals.end(), "AFIELDS should populate second field name");
    expect(field_two_type != state.globals.end(), "AFIELDS should populate second field type");
    expect(field_two_width != state.globals.end(), "AFIELDS should populate second field width");
    expect(field_two_decimals != state.globals.end(), "AFIELDS should populate second field decimals");

    if (line_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_count->second) == "2", "ALINES should split two lines");
    }
    if (used_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_count->second) == "1", "AUSED should report one open cursor in the current data session");
    }
    if (used_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_cols->second) == "2", "AUSED should expose alias and work-area columns");
    }
    if (used_alias != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_alias->second) == "People", "AUSED should expose the open alias");
    }
    if (used_area != state.globals.end()) {
        expect(copperfin::runtime::format_value(used_area->second) == "1", "AUSED should expose the open work area number");
    }
    if (line_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_one->second) == "red", "ALINES flag 1 should trim the first line");
    }
    if (line_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_two->second) == "blue", "ALINES should preserve second line text");
    }
    if (file_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_count->second) == "1", "ADIR should match only the txt file");
    }
    if (file_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_name->second) == "alpha.txt", "ADIR should return the matched file name");
    }
    if (file_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_size->second) == "3", "ADIR should return the file size");
    }
    if (file_attr != state.globals.end()) {
        expect(copperfin::runtime::format_value(file_attr->second).find("D") == std::string::npos,
            "ADIR should not mark normal files as directories");
    }
    if (field_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_count->second) == "2", "AFIELDS should report two fields");
    }
    if (field_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_cols->second) == "16", "AFIELDS should expose sixteen metadata columns");
    }
    if (field_one_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_name->second) == "NAME", "AFIELDS first field name should be NAME");
    }
    if (field_one_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_type->second) == "C", "AFIELDS first field type should be C");
    }
    if (field_one_width != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_one_width->second) == "10", "AFIELDS first field width should be 10");
    }
    if (field_two_name != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_name->second) == "AGE", "AFIELDS second field name should be AGE");
    }
    if (field_two_type != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_type->second) == "N", "AFIELDS second field type should be N");
    }
    if (field_two_width != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_width->second) == "3", "AFIELDS second field width should be 3");
    }
    if (field_two_decimals != state.globals.end()) {
        expect(copperfin::runtime::format_value(field_two_decimals->second) == "0", "AFIELDS second field decimals should be 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_macro_expanded_array_helpers_and_access() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_array_macro_helpers";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "array_macro_helpers.prg";
    write_text(
        main_path,
        "cArrayName = 'cResolvedArray'\n"
        "cArrayNameHolder = 'cArrayName'\n"
        "cArrayNameDeepHolder = 'cArrayNameHolder'\n"
        "cResolvedArray = 'aGrid'\n"
        "cLinesName = 'aLines'\n"
        "cLinesNameHolder = 'cLinesName'\n"
        "cLinesNameDeepHolder = 'cLinesNameHolder'\n"
        "cCopyName = 'aCopy'\n"
        "cCopyNameHolder = 'cCopyName'\n"
        "cCopyNameDeepHolder = 'cCopyNameHolder'\n"
        "DIMENSION aGrid[2,2]\n"
        "aGrid[1,1] = 'A'\n"
        "aGrid[1,2] = 'B'\n"
        "aGrid[2,1] = 'C'\n"
        "aGrid[2,2] = 42\n"
        "nLineCount = ALINES(&cLinesName, 'north' + CHR(13) + CHR(10) + 'south')\n"
        "nGridSize = ALEN(&cArrayName)\n"
        "nGridRows = ALEN(&cArrayName, 1)\n"
        "nGridCols = ALEN(&cArrayName, 2)\n"
        "cMacroBracket = &cArrayName[2,1]\n"
        "cMacroParen = &cArrayName(1,2)\n"
        "nMacroElement = AELEMENT(&cArrayName, 2, 2)\n"
        "nCopied = ACOPY(&cArrayName, &cCopyName, 2, 2, 1)\n"
        "nCopySize = ALEN(&cCopyName)\n"
        "cCopiedOne = &cCopyName[1]\n"
        "cCopiedTwo = &cCopyName[2]\n"
        "cLineOne = &cLinesName[1]\n"
        "cLineTwo = &cLinesName[2]\n"
        "nLineCountSecondHop = ALINES(&cLinesNameDeepHolder, 'east' + CHR(13) + CHR(10) + 'west')\n"
        "nGridSizeSecondHop = ALEN(&cArrayNameDeepHolder)\n"
        "nGridRowsSecondHop = ALEN(&cArrayNameDeepHolder, 1)\n"
        "nGridColsSecondHop = ALEN(&cArrayNameDeepHolder, 2)\n"
        "cMacroBracketSecondHop = &cArrayNameDeepHolder[2,1]\n"
        "cMacroParenSecondHop = &cArrayNameDeepHolder(1,2)\n"
        "nMacroElementSecondHop = AELEMENT(&cArrayNameDeepHolder, 2, 2)\n"
        "nCopiedSecondHop = ACOPY(&cArrayNameDeepHolder, &cCopyNameDeepHolder, 2, 2, 1)\n"
        "nCopySizeSecondHop = ALEN(&cCopyNameDeepHolder)\n"
        "cCopiedOneSecondHop = &cCopyNameDeepHolder[1]\n"
        "cCopiedTwoSecondHop = &cCopyNameDeepHolder[2]\n"
        "cLineOneSecondHop = &cLinesNameDeepHolder[1]\n"
        "cLineTwoSecondHop = &cLinesNameDeepHolder[2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded array helper script should complete: " + state.message);

    const auto line_count = state.globals.find("nlinecount");
    const auto grid_size = state.globals.find("ngridsize");
    const auto grid_rows = state.globals.find("ngridrows");
    const auto grid_cols = state.globals.find("ngridcols");
    const auto macro_bracket = state.globals.find("cmacrobracket");
    const auto macro_paren = state.globals.find("cmacroparen");
    const auto macro_element = state.globals.find("nmacroelement");
    const auto copied = state.globals.find("ncopied");
    const auto copy_size = state.globals.find("ncopysize");
    const auto copied_one = state.globals.find("ccopiedone");
    const auto copied_two = state.globals.find("ccopiedtwo");
    const auto line_one = state.globals.find("clineone");
    const auto line_two = state.globals.find("clinetwo");
    const auto line_count_second_hop = state.globals.find("nlinecountsecondhop");
    const auto grid_size_second_hop = state.globals.find("ngridsizesecondhop");
    const auto grid_rows_second_hop = state.globals.find("ngridrowssecondhop");
    const auto grid_cols_second_hop = state.globals.find("ngridcolssecondhop");
    const auto macro_bracket_second_hop = state.globals.find("cmacrobracketsecondhop");
    const auto macro_paren_second_hop = state.globals.find("cmacroparensecondhop");
    const auto macro_element_second_hop = state.globals.find("nmacroelementsecondhop");
    const auto copied_second_hop = state.globals.find("ncopiedsecondhop");
    const auto copy_size_second_hop = state.globals.find("ncopysizesecondhop");
    const auto copied_one_second_hop = state.globals.find("ccopiedonesecondhop");
    const auto copied_two_second_hop = state.globals.find("ccopiedtwosecondhop");
    const auto line_one_second_hop = state.globals.find("clineonesecondhop");
    const auto line_two_second_hop = state.globals.find("clinetwosecondhop");

    expect(line_count != state.globals.end(), "ALINES should accept a macro-expanded target array name");
    expect(grid_size != state.globals.end(), "ALEN should accept a macro-expanded array identifier");
    expect(grid_rows != state.globals.end(), "ALEN(...,1) should accept a macro-expanded array identifier");
    expect(grid_cols != state.globals.end(), "ALEN(...,2) should accept a macro-expanded array identifier");
    expect(macro_bracket != state.globals.end(), "macro-expanded bracket array access should resolve");
    expect(macro_paren != state.globals.end(), "macro-expanded paren array access should resolve");
    expect(macro_element != state.globals.end(), "AELEMENT should accept a macro-expanded array identifier");
    expect(copied != state.globals.end(), "ACOPY should accept macro-expanded source and target array names");
    expect(copy_size != state.globals.end(), "macro-expanded ACOPY target should be readable through ALEN");
    expect(copied_one != state.globals.end(), "macro-expanded ACOPY target first element should be readable");
    expect(copied_two != state.globals.end(), "macro-expanded ACOPY target second element should be readable");
    expect(line_one != state.globals.end(), "macro-expanded ALINES target first line should be readable");
    expect(line_two != state.globals.end(), "macro-expanded ALINES target second line should be readable");
    expect(line_count_second_hop != state.globals.end(), "ALINES should accept a second-hop macro-expanded target array name");
    expect(grid_size_second_hop != state.globals.end(), "ALEN should accept a second-hop macro-expanded array identifier");
    expect(grid_rows_second_hop != state.globals.end(), "ALEN(...,1) should accept a second-hop macro-expanded array identifier");
    expect(grid_cols_second_hop != state.globals.end(), "ALEN(...,2) should accept a second-hop macro-expanded array identifier");
    expect(macro_bracket_second_hop != state.globals.end(), "second-hop macro-expanded bracket array access should resolve");
    expect(macro_paren_second_hop != state.globals.end(), "second-hop macro-expanded paren array access should resolve");
    expect(macro_element_second_hop != state.globals.end(), "AELEMENT should accept a second-hop macro-expanded array identifier");
    expect(copied_second_hop != state.globals.end(), "ACOPY should accept second-hop macro-expanded source and target array names");
    expect(copy_size_second_hop != state.globals.end(), "second-hop macro-expanded ACOPY target should be readable through ALEN");
    expect(copied_one_second_hop != state.globals.end(), "second-hop macro-expanded ACOPY target first element should be readable");
    expect(copied_two_second_hop != state.globals.end(), "second-hop macro-expanded ACOPY target second element should be readable");
    expect(line_one_second_hop != state.globals.end(), "second-hop macro-expanded ALINES target first line should be readable");
    expect(line_two_second_hop != state.globals.end(), "second-hop macro-expanded ALINES target second line should be readable");

    if (line_count != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_count->second) == "2",
            "ALINES should populate a macro-expanded target array name");
    }
    if (grid_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_size->second) == "4",
            "ALEN should report total elements for a macro-expanded array identifier");
    }
    if (grid_rows != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_rows->second) == "2",
            "ALEN(..., 1) should report rows for a macro-expanded array identifier");
    }
    if (grid_cols != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_cols->second) == "2",
            "ALEN(..., 2) should report columns for a macro-expanded array identifier");
    }
    if (macro_bracket != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_bracket->second) == "C",
            "&macro[ row, col ] should resolve through the expanded array identifier");
    }
    if (macro_paren != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_paren->second) == "B",
            "&macro(row, col) should resolve through the expanded array identifier");
    }
    if (macro_element != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_element->second) == "4",
            "AELEMENT should use the expanded array identifier and preserve mixed-type cells");
    }
    if (copied != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied->second) == "2",
            "ACOPY should copy from a macro-expanded source into a macro-expanded target array");
    }
    if (copy_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_size->second) == "2",
            "macro-expanded ACOPY target should have two copied elements");
    }
    if (copied_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_one->second) == "B",
            "ACOPY should preserve the first copied value through a macro-expanded target name");
    }
    if (copied_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_two->second) == "C",
            "ACOPY should preserve adjacent mixed-type-compatible copied values through a macro-expanded target name");
    }
    if (line_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_one->second) == "north",
            "ALINES should preserve the first line in a macro-expanded target array");
    }
    if (line_two != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_two->second) == "south",
            "ALINES should preserve the second line in a macro-expanded target array");
    }
    if (line_count_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_count_second_hop->second) == "2",
            "ALINES should populate a second-hop macro-expanded target array name");
    }
    if (grid_size_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_size_second_hop->second) == "4",
            "ALEN should report total elements for a second-hop macro-expanded array identifier");
    }
    if (grid_rows_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_rows_second_hop->second) == "2",
            "ALEN(..., 1) should report rows for a second-hop macro-expanded array identifier");
    }
    if (grid_cols_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(grid_cols_second_hop->second) == "2",
            "ALEN(..., 2) should report columns for a second-hop macro-expanded array identifier");
    }
    if (macro_bracket_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_bracket_second_hop->second) == "C",
            "second-hop &macro[ row, col ] should resolve through the expanded array identifier");
    }
    if (macro_paren_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_paren_second_hop->second) == "B",
            "second-hop &macro(row, col) should resolve through the expanded array identifier");
    }
    if (macro_element_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(macro_element_second_hop->second) == "4",
            "AELEMENT should use the second-hop expanded array identifier and preserve mixed-type cells");
    }
    if (copied_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_second_hop->second) == "2",
            "ACOPY should copy from a second-hop macro-expanded source into a second-hop macro-expanded target array");
    }
    if (copy_size_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(copy_size_second_hop->second) == "2",
            "second-hop macro-expanded ACOPY target should have two copied elements");
    }
    if (copied_one_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_one_second_hop->second) == "B",
            "ACOPY should preserve the first copied value through a second-hop macro-expanded target name");
    }
    if (copied_two_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(copied_two_second_hop->second) == "C",
            "ACOPY should preserve adjacent mixed-type-compatible copied values through a second-hop macro-expanded target name");
    }
    if (line_one_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_one_second_hop->second) == "east",
            "ALINES should preserve the first line in a second-hop macro-expanded target array");
    }
    if (line_two_second_hop != state.globals.end()) {
        expect(copperfin::runtime::format_value(line_two_second_hop->second) == "west",
            "ALINES should preserve the second line in a second-hop macro-expanded target array");
    }

    fs::remove_all(temp_root, ignored);
}

void test_store_uses_assignment_target_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_store_targets";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "store_targets.prg";
    write_text(
        main_path,
        "DIMENSION aVals[3]\n"
        "cMacroTarget = 'aVals[2]'\n"
        "cMacroTargetHolder = 'cMacroTarget'\n"
        "cMacroTargetDeepHolder = 'cMacroTargetHolder'\n"
        "STORE 5 TO aVals[1]\n"
        "STORE 6 TO &cMacroTargetDeepHolder\n"
        "STORE 7 TO nScalar, aVals[3]\n"
        "nOne = aVals[1]\n"
        "nTwo = aVals[2]\n"
        "nThree = aVals[3]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "STORE target semantics script should complete");

    const auto scalar = state.globals.find("nscalar");
    const auto one = state.globals.find("none");
    const auto two = state.globals.find("ntwo");
    const auto three = state.globals.find("nthree");
    expect(scalar != state.globals.end(), "STORE should still assign scalar targets");
    expect(one != state.globals.end(), "STORE should assign direct array element targets");
    expect(two != state.globals.end(), "STORE should assign macro-expanded array element targets");
    expect(three != state.globals.end(), "STORE should assign mixed scalar/array targets");
    if (scalar != state.globals.end()) {
        expect(copperfin::runtime::format_value(scalar->second) == "7",
            "STORE should assign scalar targets through the shared assignment path");
    }
    if (one != state.globals.end()) {
        expect(copperfin::runtime::format_value(one->second) == "5",
            "STORE should assign direct array element targets");
    }
    if (two != state.globals.end()) {
        expect(copperfin::runtime::format_value(two->second) == "6",
            "STORE should assign second-hop macro-expanded array element targets");
    }
    if (three != state.globals.end()) {
        expect(copperfin::runtime::format_value(three->second) == "7",
            "STORE should assign array targets alongside scalar targets");
    }

    fs::remove_all(temp_root, ignored);
}

void test_ascan_macro_expanded_predicate_and_metadata_cleanup() {
    // #391 (#97 slice): ASCAN() must accept a predicate string that arrives via a
    // variable reference or a second-hop &macro holder chain, and must restore all
    // four scan-metadata variables after the scan completes (match and no-match paths).
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_arrays_ascan_macro_pred";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "ascan_macro_pred.prg";
    write_text(
        main_path,
        "DIMENSION aValues[5]\n"
        "aValues[1] = 2\n"
        "aValues[2] = 4\n"
        "aValues[3] = 7\n"
        "aValues[4] = 9\n"
        "aValues[5] = 1\n"
        "* scan 1: variable-reference predicate - predicate string lives in a variable\n"
        "cVarPred = '{|x| x > 5}'\n"
        "nVarRef = ASCAN(aValues, cVarPred, -1, -1, -1, 16)\n"
        "lStep1Value = TYPE('_ASCANVALUE') = 'U'\n"
        "lStep1Row = TYPE('_ASCANROW') = 'U'\n"
        "lStep1Column = TYPE('_ASCANCOLUMN') = 'U'\n"
        "* scan 2: first-hop macro predicate - &cMacroPred expands to cVarPred identifier\n"
        "* which resolves to the predicate string '{|x| x > 5}'\n"
        "cMacroPred = 'cVarPred'\n"
        "nMacroRef = ASCAN(aValues, &cMacroPred, -1, -1, -1, 16)\n"
        "lStep2Value = TYPE('_ASCANVALUE') = 'U'\n"
        "lStep2Row = TYPE('_ASCANROW') = 'U'\n"
        "* scan 3: second-hop holder predicate - &cPredDeepHolder -> cPredExpr -> string\n"
        "cPredExpr = '{|x| x > 8}'\n"
        "cPredDeepHolder = 'cPredExpr'\n"
        "nDeepHolder = ASCAN(aValues, &cPredDeepHolder, -1, -1, -1, 16)\n"
        "lValueCleared = TYPE('_ASCANVALUE') = 'U'\n"
        "lIndexCleared = TYPE('_ASCANINDEX') = 'U'\n"
        "lRowCleared = TYPE('_ASCANROW') = 'U'\n"
        "lColumnCleared = TYPE('_ASCANCOLUMN') = 'U'\n"
        "* no-match path must also clean up metadata\n"
        "nNoMatch = ASCAN(aValues, '{|x| x > 100}', -1, -1, -1, 16)\n"
        "lNoMatchValueCleared = TYPE('_ASCANVALUE') = 'U'\n"
        "lNoMatchRowCleared = TYPE('_ASCANROW') = 'U'\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#391: macro-predicate ASCAN script should complete");

    const auto n_var_ref = state.globals.find("nvarref");
    const auto n_macro_ref = state.globals.find("nmacroref");
    const auto n_deep_holder = state.globals.find("ndeepholder");
    const auto n_no_match = state.globals.find("nnomatch");
    const auto l_step1_value = state.globals.find("lstep1value");
    const auto l_step1_row = state.globals.find("lstep1row");
    const auto l_step1_column = state.globals.find("lstep1column");
    const auto l_step2_value = state.globals.find("lstep2value");
    const auto l_step2_row = state.globals.find("lstep2row");
    const auto l_value_cleared = state.globals.find("lvaluecleared");
    const auto l_index_cleared = state.globals.find("lindexcleared");
    const auto l_row_cleared = state.globals.find("lrowcleared");
    const auto l_column_cleared = state.globals.find("lcolumncleared");
    const auto l_no_match_value_cleared = state.globals.find("lnomatchvaluecleared");
    const auto l_no_match_row_cleared = state.globals.find("lnomatchrowcleared");

    expect(n_var_ref != state.globals.end(), "#391: variable-reference predicate result should be captured");
    expect(n_macro_ref != state.globals.end(), "#391: first-hop macro predicate result should be captured");
    expect(n_deep_holder != state.globals.end(), "#391: second-hop holder predicate result should be captured");
    expect(n_no_match != state.globals.end(), "#391: no-match predicate result should be captured");
    expect(l_step1_value != state.globals.end(), "#391: post-scan-1 _ASCANVALUE cleanup flag should be captured");
    expect(l_step2_value != state.globals.end(), "#391: post-scan-2 _ASCANVALUE cleanup flag should be captured");
    expect(l_value_cleared != state.globals.end(), "#391: post-scan-3 _ASCANVALUE cleanup flag should be captured");
    expect(l_index_cleared != state.globals.end(), "#391: _ASCANINDEX cleanup flag should be captured");
    expect(l_row_cleared != state.globals.end(), "#391: _ASCANROW cleanup flag should be captured");
    expect(l_column_cleared != state.globals.end(), "#391: _ASCANCOLUMN cleanup flag should be captured");
    expect(l_no_match_value_cleared != state.globals.end(), "#391: no-match _ASCANVALUE cleanup flag should be captured");
    expect(l_no_match_row_cleared != state.globals.end(), "#391: no-match _ASCANROW cleanup flag should be captured");

    if (n_var_ref != state.globals.end()) {
        expect(copperfin::runtime::format_value(n_var_ref->second) == "3",
               "#391: variable-reference predicate should find first element > 5 at index 3");
    }
    if (n_macro_ref != state.globals.end()) {
        expect(copperfin::runtime::format_value(n_macro_ref->second) == "3",
               "#391: first-hop macro predicate should find the same element at index 3");
    }
    if (n_deep_holder != state.globals.end()) {
        expect(copperfin::runtime::format_value(n_deep_holder->second) == "4",
               "#391: second-hop holder predicate should find first element > 8 at index 4");
    }
    if (n_no_match != state.globals.end()) {
        expect(copperfin::runtime::format_value(n_no_match->second) == "0",
               "#391: no-match predicate should return 0");
    }
    if (l_step1_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_step1_value->second) == "true",
               "#391: _ASCANVALUE should be restored after scan 1 (variable-reference predicate)");
    }
    if (l_step1_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_step1_row->second) == "true",
               "#391: _ASCANROW should be restored after scan 1");
    }
    if (l_step1_column != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_step1_column->second) == "true",
               "#391: _ASCANCOLUMN should be restored after scan 1");
    }
    if (l_step2_value != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_step2_value->second) == "true",
               "#391: _ASCANVALUE should be restored after scan 2 (first-hop macro predicate)");
    }
    if (l_step2_row != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_step2_row->second) == "true",
               "#391: _ASCANROW should be restored after scan 2");
    }
    if (l_value_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_value_cleared->second) == "true",
               "#391: _ASCANVALUE should be restored after scan 3 (second-hop holder)");
    }
    if (l_index_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_index_cleared->second) == "true",
               "#391: _ASCANINDEX should be restored after scan 3");
    }
    if (l_row_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_row_cleared->second) == "true",
               "#391: _ASCANROW should be restored after scan 3");
    }
    if (l_column_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_column_cleared->second) == "true",
               "#391: _ASCANCOLUMN should be restored after scan 3");
    }
    if (l_no_match_value_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_no_match_value_cleared->second) == "true",
               "#391: _ASCANVALUE should be restored after no-match scan");
    }
    if (l_no_match_row_cleared != state.globals.end()) {
        expect(copperfin::runtime::format_value(l_no_match_row_cleared->second) == "true",
               "#391: _ASCANROW should be restored after no-match scan");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

void test_asessions_returns_at_least_default_session() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_asessions";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "asessions.prg";
    write_text(
        main_path,
        "nCount = ASESSIONS(aSess)\n"
        "nFirst = aSess[1]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "ASESSIONS script should complete");

    const auto count = state.globals.find("ncount");
    const auto first = state.globals.find("nfirst");
    expect(count != state.globals.end(), "ASESSIONS count variable should be captured");
    expect(first != state.globals.end(), "ASESSIONS first element should be captured");
    if (count != state.globals.end()) {
        const std::string cv = copperfin::runtime::format_value(count->second);
        expect(cv != "0" && !cv.empty() && cv != "false",
            "ASESSIONS should return at least 1 (the default session)");
    }
    if (first != state.globals.end()) {
        expect(copperfin::runtime::format_value(first->second) == "1",
            "ASESSIONS first element should be session ID 1");
    }

    fs::remove_all(temp_root, ignored);
}

void test_afont_returns_host_aware_font_array_with_deterministic_size_contract() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_afont";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "afont.prg";
    write_text(
        main_path,
        "nAll    = AFONT(aAllFonts)\n"
        "cFirst  = aAllFonts(1)\n"
        "nKnown  = AFONT(aKnownSizes, cFirst)\n"
        "nExact  = AFONT(aKnownExact, cFirst, 12)\n"
        "nFirstSize = aKnownSizes(1)\n"
        "nBogus  = AFONT(aBogus, 'ZZZNoSuchFont')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AFONT script should complete");

    const auto all = state.globals.find("nall");
    const auto known = state.globals.find("nknown");
    const auto exact = state.globals.find("nexact");
    const auto first_size = state.globals.find("nfirstsize");
    const auto bogus = state.globals.find("nbogus");
    expect(all        != state.globals.end(), "AFONT all-fonts count should be captured");
    expect(known      != state.globals.end(), "AFONT known-font size-list count should be captured");
    expect(exact      != state.globals.end(), "AFONT size-filter result should be captured");
    expect(first_size != state.globals.end(), "AFONT first known size should be captured");
    expect(bogus      != state.globals.end(), "AFONT unknown-font count should be captured");
    if (all != state.globals.end()) {
        const std::string av = copperfin::runtime::format_value(all->second);
        expect(av != "0" && !av.empty() && av != "false",
            "AFONT with no filter should return at least one font name");
    }
    if (known != state.globals.end()) {
        const std::string kv = copperfin::runtime::format_value(known->second);
        expect(kv != "0" && !kv.empty() && kv != "false",
            "AFONT for a reported host font should return a non-empty size list");
    }
    if (exact != state.globals.end()) {
        expect(copperfin::runtime::format_value(exact->second) == "1",
            "AFONT size-filter queries should accept a representative positive size for a reported host font");
    }
    if (first_size != state.globals.end()) {
        expect(copperfin::runtime::format_value(first_size->second) == "8",
            "AFONT deterministic size enumeration should begin with the shared MVP size list");
    }
    if (bogus != state.globals.end()) {
        expect(copperfin::runtime::format_value(bogus->second) == "0",
            "AFONT for an unknown font should return 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_aprinters_supports_injected_enumeration_and_deterministic_no_printer_fallback() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aprinters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path fake_path = temp_root / "fake-bin";
    const fs::path empty_path = temp_root / "empty-bin";
    fs::create_directories(fake_path);
    fs::create_directories(empty_path);

#if defined(_WIN32)
    const fs::path fake_lpstat = fake_path / "lpstat.cmd";
    write_text(
        fake_lpstat,
        "@echo off\r\n"
        "if \"%1\"==\"-a\" (\r\n"
        "  echo Office_Printer accepting requests since Sat Jul 4 01:00:11 2026\r\n"
        "  echo Label_Printer accepting requests since Sat Jul 4 01:00:18 2026\r\n"
        "  exit /b 0\r\n"
        ")\r\n"
        "if \"%1\"==\"-p\" (\r\n"
        "  echo printer Office_Printer is idle. enabled since Sat Jul 4 01:00:11 2026\r\n"
        "  echo printer Label_Printer is idle. enabled since Sat Jul 4 01:00:18 2026\r\n"
        "  exit /b 0\r\n"
        ")\r\n"
        "exit /b 0\r\n");
#else
    const fs::path fake_lpstat = fake_path / "lpstat";
    write_text(
        fake_lpstat,
        "#!/bin/sh\n"
        "if [ \"$1\" = \"-a\" ]; then\n"
        "  echo \"Office_Printer accepting requests since Sat Jul 4 01:00:11 2026\"\n"
        "  echo \"Label_Printer accepting requests since Sat Jul 4 01:00:18 2026\"\n"
        "  exit 0\n"
        "fi\n"
        "if [ \"$1\" = \"-p\" ]; then\n"
        "  echo \"printer Office_Printer is idle. enabled since Sat Jul 4 01:00:11 2026\"\n"
        "  echo \"printer Label_Printer is idle. enabled since Sat Jul 4 01:00:18 2026\"\n"
        "  exit 0\n"
        "fi\n"
        "exit 0\n");
    fs::permissions(
        fake_lpstat,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const fs::path main_path = temp_root / "aprinters_enumerated.prg";
    write_text(
        main_path,
        "nCount = APRINTERS(aPrint)\n"
        "cFirst = aPrint(1)\n"
        "cSecond = aPrint(2)\n"
        "RETURN\n");

    {
        copperfin::test_support::ScopedEnvironmentValue scoped_path("PATH");
        scoped_path.set(fake_path.string());

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "APRINTERS injected-enumeration script should complete");

        const auto count = state.globals.find("ncount");
        const auto first = state.globals.find("cfirst");
        const auto second = state.globals.find("csecond");
        expect(count != state.globals.end(), "APRINTERS injected count should be captured");
        expect(first != state.globals.end(), "APRINTERS injected first printer should be captured");
        expect(second != state.globals.end(), "APRINTERS injected second printer should be captured");
        if (count != state.globals.end()) {
            expect(copperfin::runtime::format_value(count->second) == "2",
                "APRINTERS injected enumeration should return the scripted printer count");
        }
        if (first != state.globals.end()) {
            expect(copperfin::runtime::format_value(first->second) == "Office_Printer",
                "APRINTERS injected enumeration should preserve the first scripted printer name");
        }
        if (second != state.globals.end()) {
            expect(copperfin::runtime::format_value(second->second) == "Label_Printer",
                "APRINTERS injected enumeration should preserve the second scripted printer name");
        }
    }

    const fs::path fallback_path = temp_root / "aprinters_fallback.prg";
    write_text(
        fallback_path,
        "nCount = APRINTERS(aPrint)\n"
        "cFirst = aPrint(1)\n"
        "RETURN\n");

    {
        copperfin::test_support::ScopedEnvironmentValue scoped_path("PATH");
        scoped_path.set(empty_path.string());

        copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(fallback_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed, "APRINTERS fallback script should complete");

        const auto count = state.globals.find("ncount");
        const auto first = state.globals.find("cfirst");
        expect(count != state.globals.end(), "APRINTERS fallback count should be captured");
        expect(first != state.globals.end(), "APRINTERS fallback first entry should be captured");
        if (count != state.globals.end()) {
            expect(copperfin::runtime::format_value(count->second) == "1",
                "APRINTERS fallback should return a single deterministic placeholder entry");
        }
        if (first != state.globals.end()) {
            expect(copperfin::runtime::format_value(first->second) == "(none)",
                "APRINTERS fallback should return the deterministic no-printer placeholder");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_agetfileversion_existing_and_missing_files() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_agetfileversion";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto quote_prg_string = [](const fs::path& path) {
        return std::string{"'"} + path.generic_string() + "'";
    };

    const auto write_utf16le_string = [](std::vector<std::uint8_t>& bytes, const std::string& value) {
        for (const unsigned char ch : value) {
            bytes.push_back(ch);
            bytes.push_back(0);
        }
        bytes.push_back(0);
        bytes.push_back(0);
    };

    // Create a dummy file for the "existing file" test.
    const fs::path dummy = temp_root / "dummy.exe";
    {
        std::ofstream out(dummy);
        out << "stub";
    }

    const fs::path synthetic_versioned = temp_root / "versioned-resource.bin";
    {
        std::vector<std::uint8_t> bytes;
        const std::vector<std::string> strings = {
            "VS_VERSION_INFO",
            "StringFileInfo",
            "040904B0",
            "CompanyName",
            "Copperfin Fixtures",
            "FileDescription",
            "Synthetic Version Fixture",
            "FileVersion",
            "9.8.7.6",
            "LegalCopyright",
            "Copyright",
            "Copperfin Test Fixture Copyright",
            "ProductName",
            "Copperfin Test Product",
            "ProductVersion",
            "9.8.7.6",
            "VarFileInfo",
            "Translation",
        };
        for (const std::string& value : strings) {
            write_utf16le_string(bytes, value);
        }
        std::ofstream out(synthetic_versioned, std::ios::binary);
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    fs::path versioned_fixture = synthetic_versioned;
    std::string expected_version = "9.8.7.6";
    std::string expected_description = "Synthetic Version Fixture";
    std::string expected_company = "Copperfin Fixtures";
    std::string expected_product = "Copperfin Test Product";
    std::string expected_copyright = "Copperfin Test Fixture Copyright";
    bool expect_nonempty_version_fields = false;

#if defined(_WIN32)
    wchar_t system_directory[MAX_PATH]{};
    const UINT system_directory_length = GetSystemDirectoryW(system_directory, MAX_PATH);
    if (system_directory_length > 0 && system_directory_length < MAX_PATH) {
        const fs::path kernel32 = fs::path(system_directory) / "kernel32.dll";
        if (fs::exists(kernel32, ignored)) {
            versioned_fixture = kernel32;
            expect_nonempty_version_fields = true;
        }
    }
#else
    const fs::path mounted_vfp9 =
        "/run/media/rich/VFPPROD1/program files/microsoft visual foxpro 9/vfp9.exe";
    if (fs::exists(mounted_vfp9, ignored)) {
        versioned_fixture = mounted_vfp9;
        expected_version = "9.0.00.2412";
        expected_description = "Microsoft Visual FoxPro 9.0";
        expected_company = "Microsoft Corporation";
        expected_product = "Microsoft Visual FoxPro";
        expected_copyright = "Microsoft Corporation 1992-2004. All rights reserved.";
    }
#endif

    const fs::path missing = temp_root / "missing.exe";
    const fs::path main_path = temp_root / "agetfileversion.prg";
    write_text(
        main_path,
        "cDummy      = " + quote_prg_string(dummy) + "\n"
        "cVersioned  = " + quote_prg_string(versioned_fixture) + "\n"
        "cMissing    = " + quote_prg_string(missing) + "\n"
        "nExist      = AGETFILEVERSION(aVer, cDummy)\n"
        "cExist1     = aVer(1)\n"
        "cExist2     = aVer(2)\n"
        "cExist3     = aVer(3)\n"
        "cExist4     = aVer(4)\n"
        "cExist5     = aVer(5)\n"
        "cExist6     = aVer(6)\n"
        "cExist7     = aVer(7)\n"
        "nVersioned  = AGETFILEVERSION(aVer3, cVersioned)\n"
        "cVersioned1 = aVer3(1)\n"
        "cVersioned2 = aVer3(2)\n"
        "cVersioned3 = aVer3(3)\n"
        "cVersioned4 = aVer3(4)\n"
        "cVersioned5 = aVer3(5)\n"
        "cVersioned6 = aVer3(6)\n"
        "cVersioned7 = aVer3(7)\n"
        "nMiss       = AGETFILEVERSION(aVer2, cMissing)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AGETFILEVERSION script should complete");

    const auto nexist = state.globals.find("nexist");
    const auto nversioned = state.globals.find("nversioned");
    const auto nmiss  = state.globals.find("nmiss");
    const auto cexist1 = state.globals.find("cexist1");
    const auto cexist2 = state.globals.find("cexist2");
    const auto cexist3 = state.globals.find("cexist3");
    const auto cexist4 = state.globals.find("cexist4");
    const auto cexist5 = state.globals.find("cexist5");
    const auto cexist6 = state.globals.find("cexist6");
    const auto cexist7 = state.globals.find("cexist7");
    const auto cversioned1 = state.globals.find("cversioned1");
    const auto cversioned2 = state.globals.find("cversioned2");
    const auto cversioned3 = state.globals.find("cversioned3");
    const auto cversioned4 = state.globals.find("cversioned4");
    const auto cversioned5 = state.globals.find("cversioned5");
    const auto cversioned6 = state.globals.find("cversioned6");
    const auto cversioned7 = state.globals.find("cversioned7");
    expect(nexist != state.globals.end(), "AGETFILEVERSION existing-file count should be captured");
    expect(nversioned != state.globals.end(), "AGETFILEVERSION versioned-fixture count should be captured");
    expect(nmiss  != state.globals.end(), "AGETFILEVERSION missing-file count should be captured");
    expect(cexist1 != state.globals.end() && cexist2 != state.globals.end() &&
           cexist3 != state.globals.end() && cexist4 != state.globals.end() &&
           cexist5 != state.globals.end() && cexist6 != state.globals.end() &&
           cexist7 != state.globals.end(),
           "AGETFILEVERSION fallback rows should be captured");
    expect(cversioned1 != state.globals.end() && cversioned2 != state.globals.end() &&
           cversioned3 != state.globals.end() && cversioned4 != state.globals.end() &&
           cversioned5 != state.globals.end() && cversioned6 != state.globals.end() &&
           cversioned7 != state.globals.end(),
           "AGETFILEVERSION versioned rows should be captured");
    if (nexist != state.globals.end()) {
        expect(copperfin::runtime::format_value(nexist->second) == "7",
            "AGETFILEVERSION should return 7 elements for an existing file");
    }
    if (nversioned != state.globals.end()) {
        expect(copperfin::runtime::format_value(nversioned->second) == "7",
            "AGETFILEVERSION should return 7 elements for a versioned fixture");
    }
    if (nmiss != state.globals.end()) {
        expect(copperfin::runtime::format_value(nmiss->second) == "0",
            "AGETFILEVERSION should return 0 for a missing file");
    }
    if (cexist1 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist1->second) == "0.0.0.0",
            "AGETFILEVERSION should fall back to the default full version for plain files");
    }
    if (cexist2 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist2->second) == "dummy.exe",
            "AGETFILEVERSION should fall back to the filename for plain-file descriptions");
    }
    if (cexist3 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist3->second).empty(),
            "AGETFILEVERSION should leave company name empty when no version metadata exists");
    }
    if (cexist4 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist4->second) == "0.0.0.0",
            "AGETFILEVERSION should fall back to the default file version for plain files");
    }
    if (cexist5 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist5->second).empty(),
            "AGETFILEVERSION should leave product name empty when no version metadata exists");
    }
    if (cexist6 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist6->second) == "0.0.0.0",
            "AGETFILEVERSION should fall back to the default product version for plain files");
    }
    if (cexist7 != state.globals.end()) {
        expect(copperfin::runtime::format_value(cexist7->second).empty(),
            "AGETFILEVERSION should leave trademark/copyright empty when no version metadata exists");
    }
    if (expect_nonempty_version_fields) {
        if (cversioned1 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned1->second).empty() &&
                       copperfin::runtime::format_value(cversioned1->second) != "0.0.0.0",
                   "AGETFILEVERSION should read a non-empty full version from real Windows versioned files");
        }
        if (cversioned2 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned2->second).empty(),
                   "AGETFILEVERSION should read a non-empty file description from real Windows versioned files");
        }
        if (cversioned3 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned3->second).empty(),
                   "AGETFILEVERSION should read a non-empty company name from real Windows versioned files");
        }
        if (cversioned4 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned4->second).empty() &&
                       copperfin::runtime::format_value(cversioned4->second) != "0.0.0.0",
                   "AGETFILEVERSION should read a non-empty file version from real Windows versioned files");
        }
        if (cversioned5 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned5->second).empty(),
                   "AGETFILEVERSION should read a non-empty product name from real Windows versioned files");
        }
        if (cversioned6 != state.globals.end()) {
            expect(!copperfin::runtime::format_value(cversioned6->second).empty() &&
                       copperfin::runtime::format_value(cversioned6->second) != "0.0.0.0",
                   "AGETFILEVERSION should read a non-empty product version from real Windows versioned files");
        }
    } else {
        if (cversioned1 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned1->second) == expected_version,
                "AGETFILEVERSION should extract the expected full version from the versioned fallback fixture");
        }
        if (cversioned2 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned2->second) == expected_description,
                "AGETFILEVERSION should extract the expected description from the versioned fallback fixture");
        }
        if (cversioned3 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned3->second) == expected_company,
                "AGETFILEVERSION should extract the expected company from the versioned fallback fixture");
        }
        if (cversioned4 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned4->second) == expected_version,
                "AGETFILEVERSION should extract the expected file version from the versioned fallback fixture");
        }
        if (cversioned5 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned5->second) == expected_product,
                "AGETFILEVERSION should extract the expected product name from the versioned fallback fixture");
        }
        if (cversioned6 != state.globals.end()) {
            expect(copperfin::runtime::format_value(cversioned6->second) == expected_version,
                "AGETFILEVERSION should extract the expected product version from the versioned fallback fixture");
        }
        if (cversioned7 != state.globals.end()) {
            const std::string legal_text = copperfin::runtime::format_value(cversioned7->second);
            expect(!legal_text.empty() &&
                       legal_text.find(expected_copyright) != std::string::npos,
                   "AGETFILEVERSION should extract the expected legal text from the versioned fallback fixture");
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_agetfileversion_strict_verified_bytes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_agetfileversion_verified_bytes";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto quote_prg_string = [](const fs::path& path) {
        return std::string{"'"} + path.generic_string() + "'";
    };
    const fs::path admitted_file = temp_root / "admitted-versioned-file.bin";
    std::string expected_version = "9.8.7.6";
    std::string admitted_bytes;

#if defined(_WIN32)
    wchar_t system_directory[MAX_PATH]{};
    const UINT system_directory_length = GetSystemDirectoryW(system_directory, MAX_PATH);
    const fs::path kernel32 = fs::path(system_directory) / "kernel32.dll";
    expect(system_directory_length > 0 && system_directory_length < MAX_PATH &&
               fs::exists(kernel32, ignored),
           "strict AGETFILEVERSION should find the Windows versioned fixture");
    if (system_directory_length > 0 && system_directory_length < MAX_PATH &&
        fs::exists(kernel32, ignored))
    {
        admitted_bytes = read_text(kernel32);
        std::ofstream output(admitted_file, std::ios::binary);
        output.write(admitted_bytes.data(), static_cast<std::streamsize>(admitted_bytes.size()));
    }
#else
    const auto write_utf16le_string = [](std::string& bytes, const std::string& value) {
        for (const unsigned char ch : value) {
            bytes.push_back(static_cast<char>(ch));
            bytes.push_back('\0');
        }
        bytes.push_back('\0');
        bytes.push_back('\0');
    };
    const std::vector<std::string> strings = {
        "VS_VERSION_INFO", "StringFileInfo", "040904B0", "FileVersion", "9.8.7.6",
        "FileDescription", "Verified Version Fixture", "CompanyName", "Copperfin Fixtures",
        "ProductName", "Copperfin Test Product", "ProductVersion", "9.8.7.6",
        "LegalCopyright", "Copperfin Test Fixture Copyright", "VarFileInfo", "Translation"};
    for (const std::string& value : strings) {
        write_utf16le_string(admitted_bytes, value);
    }
    write_text(admitted_file, admitted_bytes);
#endif

    if (admitted_bytes.empty()) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    write_text(admitted_file, "tampered bytes must not be read in strict mode");
    const fs::path missing_file = temp_root / "not-admitted.bin";
    const fs::path main_path = temp_root / "agetfileversion_verified.prg";
    write_text(
        main_path,
        "cAdmitted = " + quote_prg_string(admitted_file) + "\n"
        "nVerified = AGETFILEVERSION(aVerified, cAdmitted)\n"
        "cVersion = aVerified(1)\n"
        "nMissing = AGETFILEVERSION(aMissing, " + quote_prg_string(missing_file) + ")\n"
        "RETURN\n");

    auto options = make_runtime_session_options(main_path.string(), temp_root.string());
    options.verified_file_byte_overrides.emplace(admitted_file.string(), admitted_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = copperfin::runtime::PrgRuntimeSession::create(options)
                           .run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "strict AGETFILEVERSION script should complete: " + state.message);

    const auto verified = state.globals.find("nverified");
    const auto version = state.globals.find("cversion");
    const auto missing = state.globals.find("nmissing");
    expect(verified != state.globals.end(), "strict AGETFILEVERSION count should be captured");
    expect(version != state.globals.end(), "strict AGETFILEVERSION version should be captured");
    expect(missing != state.globals.end(), "strict AGETFILEVERSION missing count should be captured");
    if (verified != state.globals.end()) {
        expect(copperfin::runtime::format_value(verified->second) == "7",
               "strict AGETFILEVERSION should preserve the seven-row contract");
    }
    if (version != state.globals.end()) {
#if defined(_WIN32)
        expect(!copperfin::runtime::format_value(version->second).empty() &&
                   copperfin::runtime::format_value(version->second) != "0.0.0.0",
               "strict AGETFILEVERSION should read the admitted Windows version resource");
#else
        expect(copperfin::runtime::format_value(version->second) == expected_version,
               "strict AGETFILEVERSION should read the admitted POSIX fixture bytes");
#endif
    }
    if (missing != state.globals.end()) {
        expect(copperfin::runtime::format_value(missing->second) == "0",
               "strict AGETFILEVERSION should reject an unadmitted file");
    }

    fs::remove_all(temp_root, ignored);
}

int main() {
    test_ascan_matches_object_references_and_nulls_exactly();
    test_aelement_single_subscript_uses_linear_index();
    test_asort_order_values_follow_vfp_contract();
    test_ascan_column_start_uses_column_relative_row();
    test_ascan_predicate_expression_search();
    test_acopy_two_dimensional_row_and_column_workflows();
    test_acopy_clamps_to_existing_target_capacity();
    test_array_dimension_and_element_assignment();
    test_preprocessor_constants_expand_in_array_subscripts_but_not_bracket_literals();
    test_asize_two_argument_form_preserves_existing_column_count();
    test_array_metadata_and_text_functions();
    test_macro_expanded_array_helpers_and_access();
    test_store_uses_assignment_target_semantics();
    test_ascan_macro_expanded_predicate_and_metadata_cleanup();
    test_asessions_returns_at_least_default_session();
    test_afont_returns_host_aware_font_array_with_deterministic_size_contract();
    test_aprinters_supports_injected_enumeration_and_deterministic_no_printer_fallback();
    test_agetfileversion_existing_and_missing_files();
    test_agetfileversion_strict_verified_bytes();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
