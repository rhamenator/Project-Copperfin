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
    const auto index_predicate = state.globals.find("nindexpredicate");
    const auto no_match = state.globals.find("nnomatch");
    const auto metadata_cleared = state.globals.find("lmetadatacleared");

    expect(greater_than_five != state.globals.end(), "ASCAN block-style predicate result should be captured");
    expect(index_predicate != state.globals.end(), "ASCAN metadata predicate result should be captured");
    expect(no_match != state.globals.end(), "ASCAN no-match predicate result should be captured");
    expect(metadata_cleared != state.globals.end(), "ASCAN predicate metadata cleanup flag should be captured");

    if (greater_than_five != state.globals.end()) {
        expect(copperfin::runtime::format_value(greater_than_five->second) == "3",
            "ASCAN predicate block should return the first element whose value satisfies the expression");
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

}  // namespace

int main() {
    test_ascan_predicate_expression_search();
    test_acopy_two_dimensional_row_and_column_workflows();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
