#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_panellink_default_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_panellink";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_panellink.prg";
        write_text(
            main_path,
            "oBaseGrid = CREATEOBJECT('Grid')\n"
            "lBaseHasPanelLink = PEMSTATUS(oBaseGrid, 'PanelLink', 1)\n"
            "lBasePanelLinkReadOnly = PEMSTATUS(oBaseGrid, 'PanelLink', 5)\n"
            "lBaseBefore = oBaseGrid.PanelLink\n"
            "xBaseGetPemBefore = GETPEM(oBaseGrid, 'PanelLink')\n"
            "oBaseGrid.PanelLink = .F.\n"
            "lBaseAfterDirectAssign = oBaseGrid.PanelLink\n"
            "lBaseSetPem = SETPEM(oBaseGrid, 'PanelLink', .T.)\n"
            "lBaseAfterSetPem = oBaseGrid.PanelLink\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseGrid, 'PanelLink', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseGrid, 'PanelLink')\n"
            "oDerived = CREATEOBJECT('DemoGrid')\n"
            "lDerivedBefore = oDerived.PanelLink\n"
            "lChildBefore = oDerived.cmdProbe.ReadPanelLink()\n"
            "oDerived.cmdProbe.DisablePanelLink()\n"
            "lDerivedAfterChild = oDerived.PanelLink\n"
            "xDerivedGetPem = GETPEM(oDerived, 'PanelLink')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasPanelLink = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'PANELLINK'\n"
            "        lPropHasPanelLink = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadPanelLink\n"
            "        RETURN THIS.Parent.PanelLink\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisablePanelLink\n"
            "        THIS.Parent.PanelLink = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoGrid AS Grid\n"
            "    PanelLink = .T.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PanelLink property script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lbasehaspanellink", "true");
        check("lbasepanellinkreadonly", "false");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("lchildbefore", "true");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophaspanellink", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_combobox_readonly_defaults_mutate_and_honor_style_guard()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_combobox_readonly";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_combobox_readonly.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasReadOnly = PEMSTATUS(oPlain, 'ReadOnly', 1)\n"
            "lPlainReadOnlyReadOnly = PEMSTATUS(oPlain, 'ReadOnly', 5)\n"
            "lPlainBefore = oPlain.ReadOnly\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ReadOnly')\n"
            "oPlain.ReadOnly = .T.\n"
            "lPlainAfterDirectAssign = oPlain.ReadOnly\n"
            "oPlain.Style = 2\n"
            "nPlainStyleAfterList = oPlain.Style\n"
            "lPlainAfterStyleList = oPlain.ReadOnly\n"
            "oPlain.ReadOnly = .T.\n"
            "lPlainAfterBlockedDirectAssign = oPlain.ReadOnly\n"
            "lPlainSetPemBlocked = SETPEM(oPlain, 'ReadOnly', .T.)\n"
            "lPlainAfterBlockedSetPem = oPlain.ReadOnly\n"
            "oPlain.Style = 0\n"
            "lPlainSetPemAllowed = SETPEM(oPlain, 'ReadOnly', .T.)\n"
            "lPlainAfterAllowedSetPem = oPlain.ReadOnly\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ReadOnly', .T.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ReadOnly')\n"
            "oForm = CREATEOBJECT('ListForm')\n"
            "nChildStyle = oForm.cboMonth.Style\n"
            "lChildBefore = oForm.cboMonth.ReadOnly\n"
            "lChildRead = oForm.cmdProbe.ReadListReadOnly()\n"
            "oForm.cmdProbe.TryMakeReadOnly()\n"
            "lChildAfterChild = oForm.cboMonth.ReadOnly\n"
            "lChildSetPemBlocked = SETPEM(oForm.cboMonth, 'ReadOnly', .T.)\n"
            "lChildAfterBlockedSetPem = oForm.cboMonth.ReadOnly\n"
            "xChildGetPem = GETPEM(oForm.cboMonth, 'ReadOnly')\n"
            "lChildHasReadOnly = PEMSTATUS(oForm.cboMonth, 'ReadOnly', 1)\n"
            "lChildReadOnlyReadOnly = PEMSTATUS(oForm.cboMonth, 'ReadOnly', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cboMonth, 1)\n"
            "lPropHasReadOnly = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'READONLY'\n"
            "        lPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oEditable = CREATEOBJECT('EditableCombo')\n"
            "lEditableBefore = oEditable.ReadOnly\n"
            "oListDerived = CREATEOBJECT('ListCombo')\n"
            "nListDerivedStyle = oListDerived.Style\n"
            "lListDerivedBefore = oListDerived.ReadOnly\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadListReadOnly\n"
            "        RETURN THISFORM.cboMonth.ReadOnly\n"
            "    ENDFUNC\n"
            "    PROCEDURE TryMakeReadOnly\n"
            "        THISFORM.cboMonth.ReadOnly = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ListForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH Style = 2\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS EditableCombo AS ComboBox\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ListCombo AS ComboBox\n"
            "    Style = 2\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ComboBox ReadOnly property script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lplainhasreadonly", "true");
        check("lplainreadonlyreadonly", "false");
        check("lplainbefore", "false");
        check("xplaingetpembefore", "false");
        check("lplainafterdirectassign", "true");
        check("nplainstyleafterlist", "2");
        check("lplainafterstylelist", "false");
        check("lplainafterblockeddirectassign", "false");
        check("lplainsetpemblocked", "false");
        check("lplainafterblockedsetpem", "false");
        check("lplainsetpemallowed", "true");
        check("lplainafterallowedsetpem", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("nchildstyle", "2");
        check("lchildbefore", "false");
        check("lchildread", "false");
        check("lchildafterchild", "false");
        check("lchildsetpemblocked", "false");
        check("lchildafterblockedsetpem", "false");
        check("xchildgetpem", "false");
        check("lchildhasreadonly", "true");
        check("lchildreadonlyreadonly", "false");
        check("lprophasreadonly", "true");
        check("leditablebefore", "true");
        check("nlistderivedstyle", "2");
        check("llistderivedbefore", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_listindex_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_listindex";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_listindex.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasListIndex = PEMSTATUS(oPlain, 'ListIndex', 1)\n"
            "lPlainListIndexReadOnly = PEMSTATUS(oPlain, 'ListIndex', 5)\n"
            "nPlainBefore = oPlain.ListIndex\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ListIndex')\n"
            "oPlain.ListIndex = 1\n"
            "nPlainAfterDirectAssign = oPlain.ListIndex\n"
            "lPlainSetPem = SETPEM(oPlain, 'ListIndex', 2)\n"
            "nPlainAfterSetPem = oPlain.ListIndex\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ListIndex', 3)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ListIndex')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nComboBefore = oForm.cboMonth.ListIndex\n"
            "nListBefore = oForm.lstYear.ListIndex\n"
            "nComboRead = oForm.cmdProbe.ReadComboListIndex()\n"
            "nListRead = oForm.cmdProbe.ReadListListIndex()\n"
            "oForm.cmdProbe.ReindexLists()\n"
            "nComboAfterChild = oForm.cboMonth.ListIndex\n"
            "nListAfterChild = oForm.lstYear.ListIndex\n"
            "lComboSetPem = SETPEM(oForm.cboMonth, 'ListIndex', 1)\n"
            "nComboAfterSetPem = oForm.cboMonth.ListIndex\n"
            "xComboGetPem = GETPEM(oForm.cboMonth, 'ListIndex')\n"
            "lComboHasListIndex = PEMSTATUS(oForm.cboMonth, 'ListIndex', 1)\n"
            "lComboListIndexReadOnly = PEMSTATUS(oForm.cboMonth, 'ListIndex', 5)\n"
            "lListSetPem = SETPEM(oForm.lstYear, 'ListIndex', 3)\n"
            "nListAfterSetPem = oForm.lstYear.ListIndex\n"
            "xListGetPem = GETPEM(oForm.lstYear, 'ListIndex')\n"
            "lListHasListIndex = PEMSTATUS(oForm.lstYear, 'ListIndex', 1)\n"
            "lListListIndexReadOnly = PEMSTATUS(oForm.lstYear, 'ListIndex', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.lstYear, 1)\n"
            "lPropHasListIndex = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'LISTINDEX'\n"
            "        lPropHasListIndex = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundList')\n"
            "nDerivedBefore = oDerived.ListIndex\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadComboListIndex\n"
            "        RETURN THISFORM.cboMonth.ListIndex\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadListListIndex\n"
            "        RETURN THISFORM.lstYear.ListIndex\n"
            "    ENDFUNC\n"
            "    PROCEDURE ReindexLists\n"
            "        THISFORM.cboMonth.ListIndex = 2\n"
            "        THISFORM.lstYear.ListIndex = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH ListIndex = 3\n"
            "    ADD OBJECT lstYear AS ListBox WITH ListIndex = 2\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundList AS ListBox\n"
            "    ListIndex = 4\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ListIndex property script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lplainhaslistindex", "true");
        check("lplainlistindexreadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
        check("nplainafterdirectassign", "1");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "2");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ncombobefore", "3");
        check("nlistbefore", "2");
        check("ncomboread", "3");
        check("nlistread", "2");
        check("ncomboafterchild", "2");
        check("nlistafterchild", "1");
        check("lcombosetpem", "true");
        check("ncomboaftersetpem", "1");
        check("xcombogetpem", "1");
        check("lcombohaslistindex", "true");
        check("lcombolistindexreadonly", "false");
        check("llistsetpem", "true");
        check("nlistaftersetpem", "3");
        check("xlistgetpem", "3");
        check("llisthaslistindex", "true");
        check("llistlistindexreadonly", "false");
        check("lprophaslistindex", "true");
        check("nderivedbefore", "4");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_displayvalue_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_displayvalue";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_displayvalue.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasDisplayValue = PEMSTATUS(oPlain, 'DisplayValue', 1)\n"
            "lPlainDisplayValueReadOnly = PEMSTATUS(oPlain, 'DisplayValue', 5)\n"
            "cPlainBefore = oPlain.DisplayValue\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'DisplayValue')\n"
            "oPlain.DisplayValue = 'January'\n"
            "cPlainAfterDirectAssign = oPlain.DisplayValue\n"
            "lPlainSetPem = SETPEM(oPlain, 'DisplayValue', 2)\n"
            "xPlainAfterSetPem = oPlain.DisplayValue\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'DisplayValue', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'DisplayValue')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "xComboBefore = oForm.cboMonth.DisplayValue\n"
            "xListBefore = oForm.lstYear.DisplayValue\n"
            "xComboRead = oForm.cmdProbe.ReadComboDisplayValue()\n"
            "xListRead = oForm.cmdProbe.ReadListDisplayValue()\n"
            "oForm.cmdProbe.RepaintLists()\n"
            "xComboAfterChild = oForm.cboMonth.DisplayValue\n"
            "xListAfterChild = oForm.lstYear.DisplayValue\n"
            "lComboSetPem = SETPEM(oForm.cboMonth, 'DisplayValue', 5)\n"
            "xComboAfterSetPem = oForm.cboMonth.DisplayValue\n"
            "xComboGetPem = GETPEM(oForm.cboMonth, 'DisplayValue')\n"
            "lComboHasDisplayValue = PEMSTATUS(oForm.cboMonth, 'DisplayValue', 1)\n"
            "lComboDisplayValueReadOnly = PEMSTATUS(oForm.cboMonth, 'DisplayValue', 5)\n"
            "lListSetPem = SETPEM(oForm.lstYear, 'DisplayValue', 'Fiscal')\n"
            "xListAfterSetPem = oForm.lstYear.DisplayValue\n"
            "xListGetPem = GETPEM(oForm.lstYear, 'DisplayValue')\n"
            "lListHasDisplayValue = PEMSTATUS(oForm.lstYear, 'DisplayValue', 1)\n"
            "lListDisplayValueReadOnly = PEMSTATUS(oForm.lstYear, 'DisplayValue', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.lstYear, 1)\n"
            "lPropHasDisplayValue = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DISPLAYVALUE'\n"
            "        lPropHasDisplayValue = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundList')\n"
            "xDerivedBefore = oDerived.DisplayValue\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadComboDisplayValue\n"
            "        RETURN THISFORM.cboMonth.DisplayValue\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadListDisplayValue\n"
            "        RETURN THISFORM.lstYear.DisplayValue\n"
            "    ENDFUNC\n"
            "    PROCEDURE RepaintLists\n"
            "        THISFORM.cboMonth.DisplayValue = 'April'\n"
            "        THISFORM.lstYear.DisplayValue = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH DisplayValue = 'March'\n"
            "    ADD OBJECT lstYear AS ListBox WITH DisplayValue = '2024'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundList AS ListBox\n"
            "    DisplayValue = 'DerivedText'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DisplayValue property script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lplainhasdisplayvalue", "true");
        check("lplaindisplayvaluereadonly", "false");
        check("cplainbefore", "");
        check("xplaingetpembefore", "");
        check("cplainafterdirectassign", "January");
        check("lplainsetpem", "true");
        check("xplainaftersetpem", "2");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("xcombobefore", "March");
        check("xlistbefore", "2024");
        check("xcomboread", "March");
        check("xlistread", "2024");
        check("xcomboafterchild", "April");
        check("xlistafterchild", "1");
        check("lcombosetpem", "true");
        check("xcomboaftersetpem", "5");
        check("xcombogetpem", "5");
        check("lcombohasdisplayvalue", "true");
        check("lcombodisplayvaluereadonly", "false");
        check("llistsetpem", "true");
        check("xlistaftersetpem", "Fiscal");
        check("xlistgetpem", "Fiscal");
        check("llisthasdisplayvalue", "true");
        check("llistdisplayvaluereadonly", "false");
        check("lprophasdisplayvalue", "true");
        check("xderivedbefore", "DerivedText");

        fs::remove_all(temp_root, ignored);
    }

}
