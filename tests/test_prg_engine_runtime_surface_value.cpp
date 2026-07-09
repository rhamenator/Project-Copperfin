// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_native_list_control_value_tracks_selection_and_boundcolumn() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_value";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_value.prg";
    write_text(
        main_path,
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.BoundColumn = 2\n"
        "oCombo.AddListItem('Alpha')\n"
        "oCombo.AddListItem('A', oCombo.NewItemID, 2)\n"
        "oCombo.AddListItem('Beta')\n"
        "oCombo.AddListItem('B', oCombo.NewItemID, 2)\n"
        "oCombo.ListIndex = 2\n"
        "cComboValueAfterSelect = oCombo.Value\n"
        "xComboGetPemAfterSelect = GETPEM(oCombo, 'Value')\n"
        "oCombo.BoundColumn = 1\n"
        "cComboValueAfterBoundColumnShift = oCombo.Value\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "lListHasValue = PEMSTATUS(oList, 'Value', 1)\n"
        "lListValueReadOnly = PEMSTATUS(oList, 'Value', 5)\n"
        "xListValueBefore = GETPEM(oList, 'Value')\n"
        "lListSetPemBeforeRows = SETPEM(oList, 'Value', 'draft')\n"
        "xListValueAfterSetBeforeRows = GETPEM(oList, 'Value')\n"
        "oList.ColumnCount = 2\n"
        "oList.BoundColumn = 2\n"
        "oList.AddListItem('North')\n"
        "oList.AddListItem('N', oList.NewItemID, 2)\n"
        "oList.AddListItem('South')\n"
        "oList.AddListItem('S', oList.NewItemID, 2)\n"
        "oList.ListIndex = 2\n"
        "cListValueAfterSelect = oList.Value\n"
        "xListGetPemAfterSelect = GETPEM(oList, 'Value')\n"
        "oList.BoundColumn = 1\n"
        "cListValueAfterBoundColumnShift = oList.Value\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control Value script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("ccombovalueafterselect", "B");
    check("xcombogetpemafterselect", "B");
    check("ccombovalueafterboundcolumnshift", "Beta");
    check("llisthasvalue", "true");
    check("llistvaluereadonly", "false");
    check("xlistvaluebefore", "");
    check("llistsetpembeforerows", "true");
    check("xlistvalueaftersetbeforerows", "draft");
    check("clistvalueafterselect", "S");
    check("xlistgetpemafterselect", "S");
    check("clistvalueafterboundcolumnshift", "South");

    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_value_requery_tracks_bound_column_selection() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_value_requery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_value_requery.prg";
    write_text(
        main_path,
        "DIMENSION gaMonths[2,2]\n"
        "gaMonths[1,1] = 'Jan'\n"
        "gaMonths[1,2] = '01'\n"
        "gaMonths[2,1] = 'Feb'\n"
        "gaMonths[2,2] = '02'\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.BoundColumn = 2\n"
        "oCombo.RowSourceType = 5\n"
        "oCombo.RowSource = 'gaMonths'\n"
        "oCombo.Requery()\n"
        "oCombo.ListIndex = 2\n"
        "cValueBeforeSecond = oCombo.Value\n"
        "cDisplayBeforeSecond = oCombo.DisplayValue\n"
        "DIMENSION gaMonths[3,2]\n"
        "gaMonths[1,1] = 'Mar'\n"
        "gaMonths[1,2] = '03'\n"
        "gaMonths[2,1] = 'Apr'\n"
        "gaMonths[2,2] = '04'\n"
        "gaMonths[3,1] = 'May'\n"
        "gaMonths[3,2] = '05'\n"
        "oCombo.Requery()\n"
        "nListIndexAfterSecond = oCombo.ListIndex\n"
        "cValueAfterSecond = oCombo.Value\n"
        "cDisplayAfterSecond = oCombo.DisplayValue\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control Value Requery script should complete: ") + state.message +
               " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("cvaluebeforesecond", "02");
    check("cdisplaybeforesecond", "Feb");
    check("nlistindexaftersecond", "2");
    check("cvalueaftersecond", "04");
    check("cdisplayaftersecond", "Apr");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_native_list_control_value_tracks_selection_and_boundcolumn();
    test_native_list_control_value_requery_tracks_bound_column_selection();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
