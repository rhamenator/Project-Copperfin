#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace {

using namespace copperfin::test_support;

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
        "cResolvedArray = 'aGrid'\n"
        "cLinesName = 'aLines'\n"
        "cCopyName = 'aCopy'\n"
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
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "macro-expanded array helper script should complete");

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
        "STORE 5 TO aVals[1]\n"
        "STORE 6 TO &cMacroTarget\n"
        "STORE 7 TO nScalar, aVals[3]\n"
        "nOne = aVals[1]\n"
        "nTwo = aVals[2]\n"
        "nThree = aVals[3]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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
            "STORE should assign macro-expanded array element targets");
    }
    if (three != state.globals.end()) {
        expect(copperfin::runtime::format_value(three->second) == "7",
            "STORE should assign array targets alongside scalar targets");
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

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

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

void test_afont_returns_non_empty_stub_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_afont";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "afont.prg";
    write_text(
        main_path,
        "nAll    = AFONT(aAllFonts)\n"
        "nArial  = AFONT(aArialSizes, 'Arial')\n"
        "nBogus  = AFONT(aBogus, 'ZZZNoSuchFont')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AFONT script should complete");

    const auto all = state.globals.find("nall");
    const auto arial = state.globals.find("narial");
    const auto bogus = state.globals.find("nbogus");
    expect(all   != state.globals.end(), "AFONT all-fonts count should be captured");
    expect(arial != state.globals.end(), "AFONT Arial sizes count should be captured");
    expect(bogus != state.globals.end(), "AFONT unknown-font count should be captured");
    if (all != state.globals.end()) {
        const std::string av = copperfin::runtime::format_value(all->second);
        expect(av != "0" && !av.empty() && av != "false",
            "AFONT with no filter should return at least one font name");
    }
    if (arial != state.globals.end()) {
        const std::string arv = copperfin::runtime::format_value(arial->second);
        expect(arv != "0" && !arv.empty() && arv != "false",
            "AFONT('Arial') should return a non-empty size list");
    }
    if (bogus != state.globals.end()) {
        expect(copperfin::runtime::format_value(bogus->second) == "0",
            "AFONT for an unknown font should return 0");
    }

    fs::remove_all(temp_root, ignored);
}

void test_aprinters_returns_non_empty_array() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aprinters";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "aprinters.prg";
    write_text(
        main_path,
        "nCount = APRINTERS(aPrint)\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "APRINTERS script should complete");

    const auto count = state.globals.find("ncount");
    expect(count != state.globals.end(), "APRINTERS count should be captured");
    if (count != state.globals.end()) {
        const std::string cv = copperfin::runtime::format_value(count->second);
        expect(cv != "0" && !cv.empty() && cv != "false",
            "APRINTERS should return at least one entry");
    }

    fs::remove_all(temp_root, ignored);
}

void test_agetfileversion_existing_and_missing_files() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_agetfileversion";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Create a dummy file for the "existing file" test.
    const fs::path dummy = temp_root / "dummy.exe";
    {
        std::ofstream out(dummy);
        out << "stub";
    }

    const fs::path main_path = temp_root / "agetfileversion.prg";
    write_text(
        main_path,
        "cDummy  = '" + dummy.string() + "'\n"
        "nExist  = AGETFILEVERSION(aVer,  cDummy)\n"
        "nMiss   = AGETFILEVERSION(aVer2, 'C:\\nonexistent\\missing.exe')\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create({
        .startup_path = main_path.string(),
        .working_directory = temp_root.string(),
        .stop_on_entry = false
    });

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AGETFILEVERSION script should complete");

    const auto nexist = state.globals.find("nexist");
    const auto nmiss  = state.globals.find("nmiss");
    expect(nexist != state.globals.end(), "AGETFILEVERSION existing-file count should be captured");
    expect(nmiss  != state.globals.end(), "AGETFILEVERSION missing-file count should be captured");
    if (nexist != state.globals.end()) {
        expect(copperfin::runtime::format_value(nexist->second) == "7",
            "AGETFILEVERSION should return 7 elements for an existing file");
    }
    if (nmiss != state.globals.end()) {
        expect(copperfin::runtime::format_value(nmiss->second) == "0",
            "AGETFILEVERSION should return 0 for a missing file");
    }

    fs::remove_all(temp_root, ignored);
}

int main() {
    test_ascan_predicate_expression_search();
    test_acopy_two_dimensional_row_and_column_workflows();
    test_array_dimension_and_element_assignment();
    test_array_metadata_and_text_functions();
    test_macro_expanded_array_helpers_and_access();
    test_store_uses_assignment_target_semantics();
    test_asessions_returns_at_least_default_session();
    test_afont_returns_non_empty_stub_array();
    test_aprinters_returns_non_empty_array();
    test_agetfileversion_existing_and_missing_files();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
