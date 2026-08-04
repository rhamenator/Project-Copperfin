// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_prg_engine_data_io_support.h"

namespace cf_test_prg_engine_data_io {
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
        "cFlatOne = aFlat[1]\n"
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
        "nSortWindow = ASORT(aSortWindow, 2, 3, 2)\n"
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

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string(), false));

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
    const auto flat_one = state.globals.find("cflatone");
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
    expect(flat_len != state.globals.end(), "ACOPY should preserve the existing target size by default");
    expect(flat_one != state.globals.end(), "ACOPY should preserve the copied element that fits in the existing target");
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
        expect(copperfin::runtime::format_value(copy_all->second) == "1", "ACOPY without count should clamp to the existing target capacity");
    }
    if (flat_len != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_len->second) == "1", "ACOPY should not resize an existing flat target");
    }
    if (flat_one != state.globals.end()) {
        expect(copperfin::runtime::format_value(flat_one->second) == "A", "ACOPY should preserve the first copied source element when clamping");
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

void test_set_filter_dimension_sleep_runtime_errors_localize() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_set_filter_dimension_sleep_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

    const fs::path set_filter_path = temp_root / "set_filter_no_area.prg";
    write_text(
        set_filter_path,
        "SET FILTER TO AGE >= 25\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession set_filter_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(set_filter_path.string(), temp_root.string(), false));
    const auto set_filter_state = set_filter_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!set_filter_state.completed, "#2714: qps-ploc SET FILTER without a selected work area should fail");
    expect(
        set_filter_state.message ==
            copperfin::localization::pseudo_localize("SET FILTER requires a selected work area"),
        "#2714: qps-ploc SET FILTER selected-work-area error should route through the pseudo-localization transform");

    const fs::path dimension_path = temp_root / "dimension_no_dims.prg";
    write_text(
        dimension_path,
        "DIMENSION aRows\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession dimension_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(dimension_path.string(), temp_root.string(), false));
    const auto dimension_state = dimension_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!dimension_state.completed, "#2714: qps-ploc DIMENSION without dimensions should fail");
    expect(
        dimension_state.message ==
            copperfin::localization::pseudo_localize("DIMENSION/DECLARE requires array dimensions"),
        "#2714: qps-ploc DIMENSION array-dimensions error should route through the pseudo-localization transform");

    const fs::path sleep_path = temp_root / "sleep_invalid_duration.prg";
    write_text(
        sleep_path,
        "SLEEP -1\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession sleep_session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(sleep_path.string(), temp_root.string(), false));
    const auto sleep_state = sleep_session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!sleep_state.completed, "#2714: qps-ploc SLEEP with a negative duration should fail");
    expect(
        sleep_state.message ==
            copperfin::localization::pseudo_localize("SLEEP: invalid duration"),
        "#2714: qps-ploc SLEEP invalid-duration error should route through the pseudo-localization transform");

    fs::remove_all(temp_root, ignored);
}

void test_aerror_content_for_sql_passthrough_fault() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_aerror_sql_passthrough";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    // Trigger a SQL pass-through fault by calling SQLEXEC() without a valid
    // statement (empty command string), then capture AERROR() columns.
    const fs::path main_path = temp_root / "aerror_sql.prg";
    write_text(
        main_path,
        "nConn = SQLCONNECT('Driver=ODBC Driver 18 for SQL Server;Server=BadHost_DoesNotExist_Copperfin')\n"
        "nSqlResult = SQLEXEC(nConn, '')\n"
        "nRows = AERROR(aErr)\n"
        "nErrCode = aErr[1,1]\n"
        "cErrMsg  = aErr[1,2]\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "AERROR SQL passthrough fault script should complete without crashing");

    const auto rows     = state.globals.find("nrows");
    const auto err_code = state.globals.find("nerrcode");
    const auto err_msg  = state.globals.find("cerrmsg");

    expect(rows     != state.globals.end(), "AERROR after SQL fault should return a row count");
    expect(err_code != state.globals.end(), "AERROR after SQL fault should populate error code");
    expect(err_msg  != state.globals.end(), "AERROR after SQL fault should populate error message");

    if (rows != state.globals.end()) {
        const std::string rows_val = copperfin::runtime::format_value(rows->second);
        expect(rows_val != "0", "AERROR after SQL fault should return at least one row (got 0)");
    }
    if (err_code != state.globals.end()) {
        const std::string code_val = copperfin::runtime::format_value(err_code->second);
        expect(code_val != "0", "AERROR error code after SQL fault must be non-zero");
    }
    if (err_msg != state.globals.end()) {
        const std::string msg_val = copperfin::runtime::format_value(err_msg->second);
        expect(!msg_val.empty(), "AERROR error message after SQL fault must not be empty");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace cf_test_prg_engine_data_io
