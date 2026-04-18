#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
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

}  // namespace

int main() {
    test_ascan_predicate_expression_search();
    test_acopy_two_dimensional_row_and_column_workflows();
    test_array_dimension_and_element_assignment();
    test_array_metadata_and_text_functions();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
