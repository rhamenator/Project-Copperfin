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

void test_native_list_control_boundto_switches_selected_value_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_boundto";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_boundto.prg";
    write_text(
        main_path,
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.Value = 0\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.BoundColumn = 2\n"
        "oCombo.AddListItem('North')\n"
        "oCombo.AddListItem('N', oCombo.NewItemID, 2)\n"
        "oCombo.AddListItem('South')\n"
        "oCombo.AddListItem('S', oCombo.NewItemID, 2)\n"
        "oCombo.ListIndex = 2\n"
        "nComboValueDefault = oCombo.Value\n"
        "oCombo.BoundTo = .T.\n"
        "cComboValueBoundToTrue = oCombo.Value\n"
        "oCombo.Value = 0\n"
        "oCombo.BoundTo = .F.\n"
        "nComboValueBoundToFalse = oCombo.Value\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.Value = 0\n"
        "oList.ColumnCount = 2\n"
        "oList.BoundColumn = 2\n"
        "oList.AddListItem('East')\n"
        "oList.AddListItem('E', oList.NewItemID, 2)\n"
        "oList.AddListItem('West')\n"
        "oList.AddListItem('W', oList.NewItemID, 2)\n"
        "oList.ListIndex = 2\n"
        "nListValueDefault = oList.Value\n"
        "oList.BoundTo = .T.\n"
        "cListValueBoundToTrue = oList.Value\n"
        "oList.Value = 0\n"
        "lListSetPemBoundToTrue = SETPEM(oList, 'BoundTo', .T.)\n"
        "cListValueAfterSetPemBoundToTrue = oList.Value\n"
        "lListSetPemBoundToFalse = SETPEM(oList, 'BoundTo', .F.)\n"
        "nListValueBoundToFalse = oList.Value\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control BoundTo script should complete: ") + state.message +
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

    check("ncombovaluedefault", "2");
    check("ccombovalueboundtotrue", "S");
    check("ncombovalueboundtofalse", "2");
    check("nlistvaluedefault", "2");
    check("clistvalueboundtotrue", "W");
    check("llistsetpemboundtotrue", "true");
    check("clistvalueaftersetpemboundtotrue", "W");
    check("llistsetpemboundtofalse", "true");
    check("nlistvalueboundtofalse", "2");

    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_boundto_requery_keeps_numeric_value_coherent() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_boundto_requery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_boundto_requery.prg";
    write_text(
        main_path,
        "DIMENSION gaMonths[2,2]\n"
        "gaMonths[1,1] = 'Jan'\n"
        "gaMonths[1,2] = '01'\n"
        "gaMonths[2,1] = 'Feb'\n"
        "gaMonths[2,2] = '02'\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.Value = 0\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.BoundColumn = 2\n"
        "oCombo.RowSourceType = 5\n"
        "oCombo.RowSource = 'gaMonths'\n"
        "oCombo.Requery()\n"
        "oCombo.ListIndex = 2\n"
        "nValueBeforeSecond = oCombo.Value\n"
        "DIMENSION gaMonths[3,2]\n"
        "gaMonths[1,1] = 'Mar'\n"
        "gaMonths[1,2] = '03'\n"
        "gaMonths[2,1] = 'Apr'\n"
        "gaMonths[2,2] = '04'\n"
        "gaMonths[3,1] = 'May'\n"
        "gaMonths[3,2] = '05'\n"
        "oCombo.Requery()\n"
        "nListIndexAfterSecond = oCombo.ListIndex\n"
        "nValueAfterSecond = oCombo.Value\n"
        "oCombo.BoundTo = .T.\n"
        "cValueAfterBoundToTrue = oCombo.Value\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control BoundTo Requery script should complete: ") + state.message +
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

    check("nvaluebeforesecond", "2");
    check("nlistindexaftersecond", "2");
    check("nvalueaftersecond", "2");
    check("cvalueafterboundtotrue", "04");

    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_controlsource_drives_boundto_value_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_native_list_control_controlsource";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_controlsource.prg";
    write_text(
        main_path,
        "CREATE CURSOR lookupstate (nchoice N(2,0), nchoice2 N(2,0), cchoice C(8))\n"
        "APPEND BLANK\n"
        "REPLACE nchoice WITH 0, nchoice2 WITH 0, cchoice WITH ''\n"
        "oComboNum = CREATEOBJECT('ComboBox')\n"
        "oComboNum.ControlSource = 'lookupstate.nchoice'\n"
        "oComboNum.ColumnCount = 2\n"
        "oComboNum.BoundColumn = 2\n"
        "oComboNum.AddListItem('Alpha')\n"
        "oComboNum.AddListItem('A', oComboNum.NewItemID, 2)\n"
        "oComboNum.AddListItem('Beta')\n"
        "oComboNum.AddListItem('B', oComboNum.NewItemID, 2)\n"
        "oComboNum.ListIndex = 2\n"
        "nComboNumericValue = oComboNum.Value\n"
        "nNumericFieldAfterSelect = lookupstate.nchoice\n"
        "oComboNum.BoundTo = .T.\n"
        "cComboNumericBoundToTrue = oComboNum.Value\n"
        "oListNum = CREATEOBJECT('ListBox')\n"
        "lListHasControlSource = PEMSTATUS(oListNum, 'ControlSource', 1)\n"
        "lListControlSourceReadOnly = PEMSTATUS(oListNum, 'ControlSource', 5)\n"
        "cListControlSourceBefore = GETPEM(oListNum, 'ControlSource')\n"
        "oListNum.ControlSource = 'lookupstate.nchoice2'\n"
        "cListControlSourceAfter = GETPEM(oListNum, 'ControlSource')\n"
        "oListNum.ColumnCount = 2\n"
        "oListNum.BoundColumn = 2\n"
        "oListNum.AddListItem('North')\n"
        "oListNum.AddListItem('N', oListNum.NewItemID, 2)\n"
        "oListNum.AddListItem('South')\n"
        "oListNum.AddListItem('S', oListNum.NewItemID, 2)\n"
        "oListNum.ListIndex = 2\n"
        "nListNumericValue = oListNum.Value\n"
        "nListNumericFieldAfterSelect = lookupstate.nchoice2\n"
        "oListNum.BoundTo = .T.\n"
        "cListNumericBoundToTrue = oListNum.Value\n"
        "cBoundMem = ''\n"
        "oComboChar = CREATEOBJECT('ComboBox')\n"
        "oComboChar.ControlSource = 'cBoundMem'\n"
        "oComboChar.ColumnCount = 2\n"
        "oComboChar.BoundColumn = 2\n"
        "oComboChar.AddListItem('East')\n"
        "oComboChar.AddListItem('E', oComboChar.NewItemID, 2)\n"
        "oComboChar.AddListItem('West')\n"
        "oComboChar.AddListItem('W', oComboChar.NewItemID, 2)\n"
        "oComboChar.ListIndex = 2\n"
        "cComboCharacterValue = oComboChar.Value\n"
        "cCharVariableAfterSelect = cBoundMem\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control ControlSource script should complete: ") + state.message +
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

    check("ncombonumericvalue", "2");
    check("nnumericfieldafterselect", "2");
    check("ccombonumericboundtotrue", "B");
    check("llisthascontrolsource", "true");
    check("llistcontrolsourcereadonly", "false");
    check("clistcontrolsourcebefore", "");
    check("clistcontrolsourceafter", "lookupstate.nchoice2");
    check("nlistnumericvalue", "2");
    check("nlistnumericfieldafterselect", "2");
    check("clistnumericboundtotrue", "S");
    check("ccombocharactervalue", "W");
    check("ccharvariableafterselect", "W");

    fs::remove_all(temp_root, ignored);
}

void test_native_prg_member_visibility_flows_to_pemstatus_attribute_three() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_member_visibility";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_prg_member_visibility.prg";
    write_text(
        main_path,
        "oDemo = CREATEOBJECT('VisibilityDemo')\n"
        "lProtectedProperty = PEMSTATUS(oDemo, 'ProtectedValue', 3)\n"
        "lHiddenProperty = PEMSTATUS(oDemo, 'HiddenValue', 3)\n"
        "lPublicProperty = PEMSTATUS(oDemo, 'PublicValue', 3)\n"
        "lProtectedMethod = PEMSTATUS(oDemo, 'ProtectedMethod', 3)\n"
        "lHiddenMethod = PEMSTATUS(oDemo, 'HiddenMethod', 3)\n"
        "lPublicMethod = PEMSTATUS(oDemo, 'PublicMethod', 3)\n"
        "RETURN\n"
        "DEFINE CLASS VisibilityDemo AS Custom\n"
        "    PROTECTED ProtectedValue\n"
        "    HIDDEN HiddenValue\n"
        "    ProtectedValue = 1\n"
        "    HiddenValue = 2\n"
        "    PublicValue = 3\n"
        "    PROTECTED PROCEDURE ProtectedMethod\n"
        "        RETURN THIS.ProtectedValue\n"
        "    ENDPROC\n"
        "    HIDDEN FUNCTION HiddenMethod\n"
        "        RETURN THIS.HiddenValue\n"
        "    ENDFUNC\n"
        "    PROCEDURE PublicMethod\n"
        "        RETURN THIS.PublicValue\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native member visibility script should complete: ") + state.message);

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " variable should be present");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };
    check("lprotectedproperty", "true");
    check("lhiddenproperty", "true");
    check("lpublicproperty", "false");
    check("lprotectedmethod", "true");
    check("lhiddenmethod", "true");
    check("lpublicmethod", "false");

    fs::remove_all(temp_root, ignored);
}

void test_native_prg_member_visibility_enforces_access_context() {
    namespace fs = std::filesystem;
    const std::string class_source =
        "DEFINE CLASS BaseVisibilityDemo AS Custom\n"
        "    PROTECTED ProtectedValue\n"
        "    HIDDEN HiddenValue\n"
        "    ProtectedValue = 10\n"
        "    HiddenValue = 20\n"
        "    PROTECTED PROCEDURE ProtectedMethod\n"
        "        RETURN THIS.ProtectedValue\n"
        "    ENDPROC\n"
        "    HIDDEN PROCEDURE HiddenMethod\n"
        "        RETURN THIS.HiddenValue\n"
        "    ENDPROC\n"
        "    FUNCTION ReadProtected\n"
        "        RETURN THIS.ProtectedValue\n"
        "    ENDFUNC\n"
        "    FUNCTION ReadHidden\n"
        "        RETURN THIS.HiddenValue\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS ChildVisibilityDemo AS BaseVisibilityDemo\n"
        "    FUNCTION ReadProtectedFromChild\n"
        "        RETURN THIS.ProtectedValue\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n";

    const auto expect_denied = [&](const std::string &case_name, const std::string &statement) {
        const fs::path temp_root = fs::temp_directory_path() / ("copperfin_native_prg_member_access_" + case_name);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const fs::path main_path = temp_root / "native_prg_member_access.prg";
        write_text(
            main_path,
            "oDemo = CREATEOBJECT('ChildVisibilityDemo')\n" + statement + "\nRETURN\n" + class_source);

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path, temp_root));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(!state.completed, case_name + " should be rejected");
        expect(state.message.find("Access denied") != std::string::npos,
               case_name + " should report the localized access-denied diagnostic: " + state.message);
        fs::remove_all(temp_root, ignored);
    };

    expect_denied("external_protected_read", "xValue = oDemo.ProtectedValue");
    expect_denied("external_hidden_write", "oDemo.HiddenValue = 99");
    expect_denied("external_protected_method", "xValue = oDemo.ProtectedMethod()");
    expect_denied("external_hidden_method", "xValue = oDemo.HiddenMethod()");

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_member_access_allowed";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path main_path = temp_root / "native_prg_member_access_allowed.prg";
    write_text(
        main_path,
        "oBase = CREATEOBJECT('BaseVisibilityDemo')\n"
        "nBaseProtected = oBase.ReadProtected()\n"
        "nBaseHidden = oBase.ReadHidden()\n"
        "oChild = CREATEOBJECT('ChildVisibilityDemo')\n"
        "nChildProtected = oChild.ReadProtectedFromChild()\n"
        "RETURN\n" + class_source);

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("same-class and protected-subclass access should complete: ") + state.message);
    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };
    check("nbaseprotected", "10");
    check("nbasehidden", "20");
    check("nchildprotected", "10");
    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_controlsource_requery_keeps_numeric_value_coherent() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_native_list_control_controlsource_requery";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_controlsource_requery.prg";
    write_text(
        main_path,
        "CREATE CURSOR lookupstate (cchoice C(8))\n"
        "APPEND BLANK\n"
        "REPLACE cchoice WITH ''\n"
        "DIMENSION gaMonths[2,2]\n"
        "gaMonths[1,1] = 'Jan'\n"
        "gaMonths[1,2] = '01'\n"
        "gaMonths[2,1] = 'Feb'\n"
        "gaMonths[2,2] = '02'\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ControlSource = 'lookupstate.cchoice'\n"
        "oCombo.ColumnCount = 2\n"
        "oCombo.BoundColumn = 2\n"
        "oCombo.RowSourceType = 5\n"
        "oCombo.RowSource = 'gaMonths'\n"
        "oCombo.Requery()\n"
        "oCombo.ListIndex = 2\n"
        "cValueBeforeSecond = oCombo.Value\n"
        "cFieldBeforeSecond = lookupstate.cchoice\n"
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
        "cFieldAfterSecond = lookupstate.cchoice\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control ControlSource requery script should complete: ") +
               state.message + " @line=" + std::to_string(state.location.line));

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
    check("cfieldbeforesecond", "02");
    check("nlistindexaftersecond", "2");
    check("cvalueaftersecond", "04");
    check("cfieldaftersecond", "04");

    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_controlsource_stays_synchronized_after_row_mutation_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_native_list_control_controlsource_row_mutation";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_controlsource_row_mutation.prg";
    write_text(
        main_path,
        "CREATE CURSOR lookupstate (nchoice N(2,0))\n"
        "APPEND BLANK\n"
        "REPLACE nchoice WITH 0\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ControlSource = 'lookupstate.nchoice'\n"
        "oCombo.AddItem('Alpha')\n"
        "oCombo.AddItem('Beta')\n"
        "oCombo.ListIndex = 2\n"
        "nFieldBeforeRemoveItem = lookupstate.nchoice\n"
        "oCombo.RemoveItem(1)\n"
        "nComboIndexAfterRemoveItem = oCombo.ListIndex\n"
        "nFieldAfterRemoveItem = lookupstate.nchoice\n"
        "cBoundMem = ''\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ControlSource = 'cBoundMem'\n"
        "oList.ColumnCount = 2\n"
        "oList.BoundColumn = 2\n"
        "oList.BoundTo = .T.\n"
        "oList.AddListItem('North', 10)\n"
        "oList.AddListItem('N', 10, 2)\n"
        "oList.AddListItem('South', 20)\n"
        "oList.AddListItem('S', 20, 2)\n"
        "oList.ListItemID = 20\n"
        "cMemBeforeRemoveListItem = cBoundMem\n"
        "oList.RemoveListItem(20)\n"
        "nListIndexAfterRemoveListItem = oList.ListIndex\n"
        "cMemAfterRemoveListItem = cBoundMem\n"
        "oList.Clear()\n"
        "nListIndexAfterClear = oList.ListIndex\n"
        "cMemAfterClear = cBoundMem\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control ControlSource row-mutation script should complete: ") +
               state.message + " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nfieldbeforeremoveitem", "2");
    check("ncomboindexafterremoveitem", "1");
    check("nfieldafterremoveitem", "1");
    check("cmembeforeremovelistitem", "S");
    check("nlistindexafterremovelistitem", "1");
    check("cmemafterremovelistitem", "N");
    check("nlistindexafterclear", "0");
    check("cmemafterclear", "");

    fs::remove_all(temp_root, ignored);
}

void test_native_list_control_controlsource_stays_synchronized_after_sorted_reordering() {
    namespace fs = std::filesystem;
    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_native_list_control_controlsource_sorted";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "native_list_control_controlsource_sorted.prg";
    write_text(
        main_path,
        "CREATE CURSOR lookupstate (nchoice N(2,0))\n"
        "APPEND BLANK\n"
        "REPLACE nchoice WITH 0\n"
        "oCombo = CREATEOBJECT('ComboBox')\n"
        "oCombo.ControlSource = 'lookupstate.nchoice'\n"
        "oCombo.AddItem('Zulu')\n"
        "oCombo.AddItem('alpha')\n"
        "oCombo.AddItem('Echo')\n"
        "oCombo.ListIndex = 2\n"
        "nFieldBeforeSort = lookupstate.nchoice\n"
        "oCombo.Sorted = .T.\n"
        "nComboIndexAfterSort = oCombo.ListIndex\n"
        "nFieldAfterSort = lookupstate.nchoice\n"
        "oCombo.AddItem('Bravo')\n"
        "nComboIndexAfterSortedAdd = oCombo.ListIndex\n"
        "nFieldAfterSortedAdd = lookupstate.nchoice\n"
        "nBoundMem = 0\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.ControlSource = 'nBoundMem'\n"
        "oList.AddItem('Zulu')\n"
        "oList.AddItem('alpha')\n"
        "oList.AddItem('Echo')\n"
        "oList.ListIndex = 2\n"
        "nMemBeforeSort = nBoundMem\n"
        "oList.Sorted = .T.\n"
        "nListIndexAfterSort = oList.ListIndex\n"
        "nMemAfterSort = nBoundMem\n"
        "oList.AddItem('Bravo')\n"
        "nListIndexAfterSortedAdd = oList.ListIndex\n"
        "nMemAfterSortedAdd = nBoundMem\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("native list-control ControlSource sorted-reordering script should complete: ") +
               state.message + " @line=" + std::to_string(state.location.line));

    const auto check = [&](const std::string& name, const std::string& expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };

    check("nfieldbeforesort", "2");
    check("ncomboindexaftersort", "3");
    check("nfieldaftersort", "3");
    check("ncomboindexaftersortedadd", "4");
    check("nfieldaftersortedadd", "4");
    check("nmembeforesort", "2");
    check("nlistindexaftersort", "3");
    check("nmemaftersort", "3");
    check("nlistindexaftersortedadd", "4");
    check("nmemaftersortedadd", "4");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_native_list_control_value_tracks_selection_and_boundcolumn();
    test_native_list_control_value_requery_tracks_bound_column_selection();
    test_native_list_control_boundto_switches_selected_value_semantics();
    test_native_list_control_boundto_requery_keeps_numeric_value_coherent();
    test_native_list_control_controlsource_drives_boundto_value_semantics();
    test_native_list_control_controlsource_requery_keeps_numeric_value_coherent();
    test_native_list_control_controlsource_stays_synchronized_after_row_mutation_methods();
    test_native_list_control_controlsource_stays_synchronized_after_sorted_reordering();
    test_native_prg_member_visibility_flows_to_pemstatus_attribute_three();
    test_native_prg_member_visibility_enforces_access_context();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
