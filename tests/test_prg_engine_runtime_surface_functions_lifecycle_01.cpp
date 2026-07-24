#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_combobox_boundcolumn_columncount_and_columnwidths_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_combobox_list_shape";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_combobox_list_shape.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasBoundColumn = PEMSTATUS(oPlain, 'BoundColumn', 1)\n"
            "lPlainBoundColumnReadOnly = PEMSTATUS(oPlain, 'BoundColumn', 5)\n"
            "nPlainBoundColumnBefore = oPlain.BoundColumn\n"
            "xPlainBoundColumnGetPemBefore = GETPEM(oPlain, 'BoundColumn')\n"
            "oPlain.BoundColumn = 2\n"
            "nPlainBoundColumnAfterDirectAssign = oPlain.BoundColumn\n"
            "lPlainBoundColumnSetPem = SETPEM(oPlain, 'BoundColumn', 3)\n"
            "nPlainBoundColumnAfterSetPem = oPlain.BoundColumn\n"
            "lPlainBoundColumnAddProperty = ADDPROPERTY(oPlain, 'BoundColumn', 4)\n"
            "lPlainBoundColumnRemoveProperty = REMOVEPROPERTY(oPlain, 'BoundColumn')\n"
            "lPlainHasColumnCount = PEMSTATUS(oPlain, 'ColumnCount', 1)\n"
            "lPlainColumnCountReadOnly = PEMSTATUS(oPlain, 'ColumnCount', 5)\n"
            "nPlainColumnCountBefore = oPlain.ColumnCount\n"
            "xPlainColumnCountGetPemBefore = GETPEM(oPlain, 'ColumnCount')\n"
            "oPlain.ColumnCount = 2\n"
            "nPlainColumnCountAfterDirectAssign = oPlain.ColumnCount\n"
            "lPlainColumnCountSetPem = SETPEM(oPlain, 'ColumnCount', 4)\n"
            "nPlainColumnCountAfterSetPem = oPlain.ColumnCount\n"
            "lPlainColumnCountAddProperty = ADDPROPERTY(oPlain, 'ColumnCount', 5)\n"
            "lPlainColumnCountRemoveProperty = REMOVEPROPERTY(oPlain, 'ColumnCount')\n"
            "lPlainHasColumnWidths = PEMSTATUS(oPlain, 'ColumnWidths', 1)\n"
            "lPlainColumnWidthsReadOnly = PEMSTATUS(oPlain, 'ColumnWidths', 5)\n"
            "cPlainColumnWidthsBefore = oPlain.ColumnWidths\n"
            "xPlainColumnWidthsGetPemBefore = GETPEM(oPlain, 'ColumnWidths')\n"
            "oPlain.ColumnWidths = '72,0'\n"
            "cPlainColumnWidthsAfterDirectAssign = oPlain.ColumnWidths\n"
            "oPlain.ColumnWidths = 48\n"
            "cPlainColumnWidthsAfterNumericAssign = oPlain.ColumnWidths\n"
            "lPlainColumnWidthsNumericType = VARTYPE(oPlain.ColumnWidths) == 'C'\n"
            "lPlainColumnWidthsSetPem = SETPEM(oPlain, 'ColumnWidths', '60,12')\n"
            "cPlainColumnWidthsAfterSetPem = oPlain.ColumnWidths\n"
            "lPlainColumnWidthsPutPem = PUTPEM(oPlain, 'ColumnWidths', 36)\n"
            "cPlainColumnWidthsAfterPutPem = oPlain.ColumnWidths\n"
            "lPlainColumnWidthsPutPemType = VARTYPE(oPlain.ColumnWidths) == 'C'\n"
            "lPlainColumnWidthsAddProperty = ADDPROPERTY(oPlain, 'ColumnWidths', 'shadow')\n"
            "lPlainColumnWidthsRemoveProperty = REMOVEPROPERTY(oPlain, 'ColumnWidths')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nChildBoundColumnBefore = oForm.cboMonth.BoundColumn\n"
            "nChildColumnCountBefore = oForm.cboMonth.ColumnCount\n"
            "cChildColumnWidthsBefore = oForm.cboMonth.ColumnWidths\n"
            "nChildBoundColumnRead = oForm.cmdProbe.ReadBoundColumn()\n"
            "nChildColumnCountRead = oForm.cmdProbe.ReadColumnCount()\n"
            "cChildColumnWidthsRead = oForm.cmdProbe.ReadColumnWidths()\n"
            "oForm.cmdProbe.ReconfigureCombo()\n"
            "nChildBoundColumnAfterChild = oForm.cboMonth.BoundColumn\n"
            "nChildColumnCountAfterChild = oForm.cboMonth.ColumnCount\n"
            "cChildColumnWidthsAfterChild = oForm.cboMonth.ColumnWidths\n"
            "lChildBoundColumnSetPem = SETPEM(oForm.cboMonth, 'BoundColumn', 2)\n"
            "nChildBoundColumnAfterSetPem = oForm.cboMonth.BoundColumn\n"
            "xChildBoundColumnGetPem = GETPEM(oForm.cboMonth, 'BoundColumn')\n"
            "lChildBoundColumnReadOnly = PEMSTATUS(oForm.cboMonth, 'BoundColumn', 5)\n"
            "lChildColumnCountSetPem = SETPEM(oForm.cboMonth, 'ColumnCount', 5)\n"
            "nChildColumnCountAfterSetPem = oForm.cboMonth.ColumnCount\n"
            "xChildColumnCountGetPem = GETPEM(oForm.cboMonth, 'ColumnCount')\n"
            "lChildColumnCountReadOnly = PEMSTATUS(oForm.cboMonth, 'ColumnCount', 5)\n"
            "lChildColumnWidthsSetPem = SETPEM(oForm.cboMonth, 'ColumnWidths', '80,0,24,0,12')\n"
            "cChildColumnWidthsAfterSetPem = oForm.cboMonth.ColumnWidths\n"
            "xChildColumnWidthsGetPem = GETPEM(oForm.cboMonth, 'ColumnWidths')\n"
            "lChildColumnWidthsReadOnly = PEMSTATUS(oForm.cboMonth, 'ColumnWidths', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cboMonth, 1)\n"
            "lPropHasBoundColumn = .F.\n"
            "lPropHasColumnCount = .F.\n"
            "lPropHasColumnWidths = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    DO CASE\n"
            "    CASE UPPER(aPropMembers[i]) == 'BOUNDCOLUMN'\n"
            "        lPropHasBoundColumn = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNCOUNT'\n"
            "        lPropHasColumnCount = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNWIDTHS'\n"
            "        lPropHasColumnWidths = .T.\n"
            "    ENDCASE\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DateCombo')\n"
            "nDerivedBoundColumnBefore = oDerived.BoundColumn\n"
            "nDerivedColumnCountBefore = oDerived.ColumnCount\n"
            "cDerivedColumnWidthsBefore = oDerived.ColumnWidths\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadBoundColumn\n"
            "        RETURN THISFORM.cboMonth.BoundColumn\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadColumnCount\n"
            "        RETURN THISFORM.cboMonth.ColumnCount\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadColumnWidths\n"
            "        RETURN THISFORM.cboMonth.ColumnWidths\n"
            "    ENDFUNC\n"
            "    PROCEDURE ReconfigureCombo\n"
            "        THISFORM.cboMonth.BoundColumn = 3\n"
            "        THISFORM.cboMonth.ColumnCount = 3\n"
            "        THISFORM.cboMonth.ColumnWidths = '72,0,18'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH BoundColumn = 2, ColumnCount = 2, ColumnWidths = '72,0'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DateCombo AS ComboBox\n"
            "    BoundColumn = 2\n"
            "    ColumnCount = 2\n"
            "    ColumnWidths = 90\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ComboBox BoundColumn/ColumnCount/ColumnWidths script should complete: ") + state.message +
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

        check("lplainhasboundcolumn", "true");
        check("lplainboundcolumnreadonly", "false");
        check("nplainboundcolumnbefore", "1");
        check("xplainboundcolumngetpembefore", "1");
        check("nplainboundcolumnafterdirectassign", "2");
        check("lplainboundcolumnsetpem", "true");
        check("nplainboundcolumnaftersetpem", "3");
        check("lplainboundcolumnaddproperty", "false");
        check("lplainboundcolumnremoveproperty", "false");
        check("lplainhascolumncount", "true");
        check("lplaincolumncountreadonly", "false");
        check("nplaincolumncountbefore", "0");
        check("xplaincolumncountgetpembefore", "0");
        check("nplaincolumncountafterdirectassign", "2");
        check("lplaincolumncountsetpem", "true");
        check("nplaincolumncountaftersetpem", "4");
        check("lplaincolumncountaddproperty", "false");
        check("lplaincolumncountremoveproperty", "false");
        check("lplainhascolumnwidths", "true");
        check("lplaincolumnwidthsreadonly", "false");
        check("cplaincolumnwidthsbefore", "");
        check("xplaincolumnwidthsgetpembefore", "");
        check("cplaincolumnwidthsafterdirectassign", "72,0");
        check("cplaincolumnwidthsafternumericassign", "48");
        check("lplaincolumnwidthsnumerictype", "true");
        check("lplaincolumnwidthssetpem", "true");
        check("cplaincolumnwidthsaftersetpem", "60,12");
        check("lplaincolumnwidthsputpem", "true");
        check("cplaincolumnwidthsafterputpem", "36");
        check("lplaincolumnwidthsputpemtype", "true");
        check("lplaincolumnwidthsaddproperty", "false");
        check("lplaincolumnwidthsremoveproperty", "false");
        check("nchildboundcolumnbefore", "2");
        check("nchildcolumncountbefore", "2");
        check("cchildcolumnwidthsbefore", "72,0");
        check("nchildboundcolumnread", "2");
        check("nchildcolumncountread", "2");
        check("cchildcolumnwidthsread", "72,0");
        check("nchildboundcolumnafterchild", "3");
        check("nchildcolumncountafterchild", "3");
        check("cchildcolumnwidthsafterchild", "72,0,18");
        check("lchildboundcolumnsetpem", "true");
        check("nchildboundcolumnaftersetpem", "2");
        check("xchildboundcolumngetpem", "2");
        check("lchildboundcolumnreadonly", "false");
        check("lchildcolumncountsetpem", "true");
        check("nchildcolumncountaftersetpem", "5");
        check("xchildcolumncountgetpem", "5");
        check("lchildcolumncountreadonly", "false");
        check("lchildcolumnwidthssetpem", "true");
        check("cchildcolumnwidthsaftersetpem", "80,0,24,0,12");
        check("xchildcolumnwidthsgetpem", "80,0,24,0,12");
        check("lchildcolumnwidthsreadonly", "false");
        check("lprophasboundcolumn", "true");
        check("lprophascolumncount", "true");
        check("lprophascolumnwidths", "true");
        check("nderivedboundcolumnbefore", "2");
        check("nderivedcolumncountbefore", "2");
        check("cderivedcolumnwidthsbefore", "90");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_list_control_boundto_and_listbox_list_shape_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_control_boundto_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_control_boundto_builtin.prg";
        write_text(
            main_path,
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lComboHasBoundTo = PEMSTATUS(oCombo, 'BoundTo', 1)\n"
            "lComboBoundToReadOnly = PEMSTATUS(oCombo, 'BoundTo', 5)\n"
            "lComboBoundToBefore = oCombo.BoundTo\n"
            "xComboBoundToGetPemBefore = GETPEM(oCombo, 'BoundTo')\n"
            "oCombo.BoundTo = .T.\n"
            "lComboBoundToAfterDirectAssign = oCombo.BoundTo\n"
            "lComboBoundToSetPem = SETPEM(oCombo, 'BoundTo', .F.)\n"
            "lComboBoundToAfterSetPem = oCombo.BoundTo\n"
            "lComboBoundToAddProperty = ADDPROPERTY(oCombo, 'BoundTo', .T.)\n"
            "lComboBoundToRemoveProperty = REMOVEPROPERTY(oCombo, 'BoundTo')\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "lListHasBoundTo = PEMSTATUS(oList, 'BoundTo', 1)\n"
            "lListBoundToReadOnly = PEMSTATUS(oList, 'BoundTo', 5)\n"
            "lListBoundToBefore = oList.BoundTo\n"
            "xListBoundToGetPemBefore = GETPEM(oList, 'BoundTo')\n"
            "oList.BoundTo = .T.\n"
            "lListBoundToAfterDirectAssign = oList.BoundTo\n"
            "lListBoundToSetPem = SETPEM(oList, 'BoundTo', .F.)\n"
            "lListBoundToAfterSetPem = oList.BoundTo\n"
            "lListBoundToAddProperty = ADDPROPERTY(oList, 'BoundTo', .T.)\n"
            "lListBoundToRemoveProperty = REMOVEPROPERTY(oList, 'BoundTo')\n"
            "lListHasBoundColumn = PEMSTATUS(oList, 'BoundColumn', 1)\n"
            "lListBoundColumnReadOnly = PEMSTATUS(oList, 'BoundColumn', 5)\n"
            "nListBoundColumnBefore = oList.BoundColumn\n"
            "xListBoundColumnGetPemBefore = GETPEM(oList, 'BoundColumn')\n"
            "oList.BoundColumn = 2\n"
            "nListBoundColumnAfterDirectAssign = oList.BoundColumn\n"
            "lListBoundColumnSetPem = SETPEM(oList, 'BoundColumn', 3)\n"
            "nListBoundColumnAfterSetPem = oList.BoundColumn\n"
            "lListBoundColumnAddProperty = ADDPROPERTY(oList, 'BoundColumn', 4)\n"
            "lListBoundColumnRemoveProperty = REMOVEPROPERTY(oList, 'BoundColumn')\n"
            "lListHasColumnCount = PEMSTATUS(oList, 'ColumnCount', 1)\n"
            "lListColumnCountReadOnly = PEMSTATUS(oList, 'ColumnCount', 5)\n"
            "nListColumnCountBefore = oList.ColumnCount\n"
            "xListColumnCountGetPemBefore = GETPEM(oList, 'ColumnCount')\n"
            "oList.ColumnCount = 2\n"
            "nListColumnCountAfterDirectAssign = oList.ColumnCount\n"
            "lListColumnCountSetPem = SETPEM(oList, 'ColumnCount', 4)\n"
            "nListColumnCountAfterSetPem = oList.ColumnCount\n"
            "lListColumnCountAddProperty = ADDPROPERTY(oList, 'ColumnCount', 5)\n"
            "lListColumnCountRemoveProperty = REMOVEPROPERTY(oList, 'ColumnCount')\n"
            "lListHasColumnWidths = PEMSTATUS(oList, 'ColumnWidths', 1)\n"
            "lListColumnWidthsReadOnly = PEMSTATUS(oList, 'ColumnWidths', 5)\n"
            "cListColumnWidthsBefore = oList.ColumnWidths\n"
            "xListColumnWidthsGetPemBefore = GETPEM(oList, 'ColumnWidths')\n"
            "oList.ColumnWidths = '72,0'\n"
            "cListColumnWidthsAfterDirectAssign = oList.ColumnWidths\n"
            "lListColumnWidthsSetPem = SETPEM(oList, 'ColumnWidths', '60,12')\n"
            "cListColumnWidthsAfterSetPem = oList.ColumnWidths\n"
            "lListColumnWidthsAddProperty = ADDPROPERTY(oList, 'ColumnWidths', 'shadow')\n"
            "lListColumnWidthsRemoveProperty = REMOVEPROPERTY(oList, 'ColumnWidths')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lChildBoundToBefore = oForm.lstMonth.BoundTo\n"
            "nChildBoundColumnBefore = oForm.lstMonth.BoundColumn\n"
            "nChildColumnCountBefore = oForm.lstMonth.ColumnCount\n"
            "cChildColumnWidthsBefore = oForm.lstMonth.ColumnWidths\n"
            "lChildBoundToRead = oForm.cmdProbe.ReadBoundTo()\n"
            "nChildBoundColumnRead = oForm.cmdProbe.ReadBoundColumn()\n"
            "nChildColumnCountRead = oForm.cmdProbe.ReadColumnCount()\n"
            "cChildColumnWidthsRead = oForm.cmdProbe.ReadColumnWidths()\n"
            "oForm.cmdProbe.ConfigureList()\n"
            "lChildBoundToAfterChild = oForm.lstMonth.BoundTo\n"
            "nChildBoundColumnAfterChild = oForm.lstMonth.BoundColumn\n"
            "nChildColumnCountAfterChild = oForm.lstMonth.ColumnCount\n"
            "cChildColumnWidthsAfterChild = oForm.lstMonth.ColumnWidths\n"
            "lChildBoundToSetPem = SETPEM(oForm.lstMonth, 'BoundTo', .F.)\n"
            "lChildBoundToAfterSetPem = oForm.lstMonth.BoundTo\n"
            "xChildBoundToGetPem = GETPEM(oForm.lstMonth, 'BoundTo')\n"
            "lChildBoundToReadOnly = PEMSTATUS(oForm.lstMonth, 'BoundTo', 5)\n"
            "lChildBoundColumnSetPem = SETPEM(oForm.lstMonth, 'BoundColumn', 2)\n"
            "nChildBoundColumnAfterSetPem = oForm.lstMonth.BoundColumn\n"
            "xChildBoundColumnGetPem = GETPEM(oForm.lstMonth, 'BoundColumn')\n"
            "lChildBoundColumnReadOnly = PEMSTATUS(oForm.lstMonth, 'BoundColumn', 5)\n"
            "lChildColumnCountSetPem = SETPEM(oForm.lstMonth, 'ColumnCount', 5)\n"
            "nChildColumnCountAfterSetPem = oForm.lstMonth.ColumnCount\n"
            "xChildColumnCountGetPem = GETPEM(oForm.lstMonth, 'ColumnCount')\n"
            "lChildColumnCountReadOnly = PEMSTATUS(oForm.lstMonth, 'ColumnCount', 5)\n"
            "lChildColumnWidthsSetPem = SETPEM(oForm.lstMonth, 'ColumnWidths', '80,0,24,0,12')\n"
            "cChildColumnWidthsAfterSetPem = oForm.lstMonth.ColumnWidths\n"
            "xChildColumnWidthsGetPem = GETPEM(oForm.lstMonth, 'ColumnWidths')\n"
            "lChildColumnWidthsReadOnly = PEMSTATUS(oForm.lstMonth, 'ColumnWidths', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.lstMonth, 1)\n"
            "lPropHasBoundTo = .F.\n"
            "lPropHasBoundColumn = .F.\n"
            "lPropHasColumnCount = .F.\n"
            "lPropHasColumnWidths = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    DO CASE\n"
            "    CASE UPPER(aPropMembers[i]) == 'BOUNDTO'\n"
            "        lPropHasBoundTo = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'BOUNDCOLUMN'\n"
            "        lPropHasBoundColumn = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNCOUNT'\n"
            "        lPropHasColumnCount = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNWIDTHS'\n"
            "        lPropHasColumnWidths = .T.\n"
            "    ENDCASE\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('StatusList')\n"
            "lDerivedBoundToBefore = oDerived.BoundTo\n"
            "nDerivedBoundColumnBefore = oDerived.BoundColumn\n"
            "nDerivedColumnCountBefore = oDerived.ColumnCount\n"
            "cDerivedColumnWidthsBefore = oDerived.ColumnWidths\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadBoundTo\n"
            "        RETURN THISFORM.lstMonth.BoundTo\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadBoundColumn\n"
            "        RETURN THISFORM.lstMonth.BoundColumn\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadColumnCount\n"
            "        RETURN THISFORM.lstMonth.ColumnCount\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadColumnWidths\n"
            "        RETURN THISFORM.lstMonth.ColumnWidths\n"
            "    ENDFUNC\n"
            "    PROCEDURE ConfigureList\n"
            "        THISFORM.lstMonth.BoundTo = .T.\n"
            "        THISFORM.lstMonth.BoundColumn = 3\n"
            "        THISFORM.lstMonth.ColumnCount = 3\n"
            "        THISFORM.lstMonth.ColumnWidths = '72,0,18'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT lstMonth AS ListBox WITH BoundTo = .T., BoundColumn = 2, ColumnCount = 2, ColumnWidths = '72,0'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS StatusList AS ListBox\n"
            "    BoundTo = .T.\n"
            "    BoundColumn = 2\n"
            "    ColumnCount = 2\n"
            "    ColumnWidths = '90,0'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native list-control BoundTo/BoundColumn/ColumnCount/ColumnWidths built-in script should complete: ") + state.message +
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

        check("lcombohasboundto", "true");
        check("lcomboboundtoreadonly", "false");
        check("lcomboboundtobefore", "false");
        check("xcomboboundtogetpembefore", "false");
        check("lcomboboundtoafterdirectassign", "true");
        check("lcomboboundtosetpem", "true");
        check("lcomboboundtoaftersetpem", "false");
        check("lcomboboundtoaddproperty", "false");
        check("lcomboboundtoremoveproperty", "false");
        check("llisthasboundto", "true");
        check("llistboundtoreadonly", "false");
        check("llistboundtobefore", "false");
        check("xlistboundtogetpembefore", "false");
        check("llistboundtoafterdirectassign", "true");
        check("llistboundtosetpem", "true");
        check("llistboundtoaftersetpem", "false");
        check("llistboundtoaddproperty", "false");
        check("llistboundtoremoveproperty", "false");
        check("llisthasboundcolumn", "true");
        check("llistboundcolumnreadonly", "false");
        check("nlistboundcolumnbefore", "1");
        check("xlistboundcolumngetpembefore", "1");
        check("nlistboundcolumnafterdirectassign", "2");
        check("llistboundcolumnsetpem", "true");
        check("nlistboundcolumnaftersetpem", "3");
        check("llistboundcolumnaddproperty", "false");
        check("llistboundcolumnremoveproperty", "false");
        check("llisthascolumncount", "true");
        check("llistcolumncountreadonly", "false");
        check("nlistcolumncountbefore", "0");
        check("xlistcolumncountgetpembefore", "0");
        check("nlistcolumncountafterdirectassign", "2");
        check("llistcolumncountsetpem", "true");
        check("nlistcolumncountaftersetpem", "4");
        check("llistcolumncountaddproperty", "false");
        check("llistcolumncountremoveproperty", "false");
        check("llisthascolumnwidths", "true");
        check("llistcolumnwidthsreadonly", "false");
        check("clistcolumnwidthsbefore", "");
        check("xlistcolumnwidthsgetpembefore", "");
        check("clistcolumnwidthsafterdirectassign", "72,0");
        check("llistcolumnwidthssetpem", "true");
        check("clistcolumnwidthsaftersetpem", "60,12");
        check("llistcolumnwidthsaddproperty", "false");
        check("llistcolumnwidthsremoveproperty", "false");
        check("lchildboundtobefore", "true");
        check("nchildboundcolumnbefore", "2");
        check("nchildcolumncountbefore", "2");
        check("cchildcolumnwidthsbefore", "72,0");
        check("lchildboundtoread", "true");
        check("nchildboundcolumnread", "2");
        check("nchildcolumncountread", "2");
        check("cchildcolumnwidthsread", "72,0");
        check("lchildboundtoafterchild", "true");
        check("nchildboundcolumnafterchild", "3");
        check("nchildcolumncountafterchild", "3");
        check("cchildcolumnwidthsafterchild", "72,0,18");
        check("lchildboundtosetpem", "true");
        check("lchildboundtoaftersetpem", "false");
        check("xchildboundtogetpem", "false");
        check("lchildboundtoreadonly", "false");
        check("lchildboundcolumnsetpem", "true");
        check("nchildboundcolumnaftersetpem", "2");
        check("xchildboundcolumngetpem", "2");
        check("lchildboundcolumnreadonly", "false");
        check("lchildcolumncountsetpem", "true");
        check("nchildcolumncountaftersetpem", "5");
        check("xchildcolumncountgetpem", "5");
        check("lchildcolumncountreadonly", "false");
        check("lchildcolumnwidthssetpem", "true");
        check("cchildcolumnwidthsaftersetpem", "80,0,24,0,12");
        check("xchildcolumnwidthsgetpem", "80,0,24,0,12");
        check("lchildcolumnwidthsreadonly", "false");
        check("lprophasboundto", "true");
        check("lprophasboundcolumn", "true");
        check("lprophascolumncount", "true");
        check("lprophascolumnwidths", "true");
        check("lderivedboundtobefore", "true");
        check("nderivedboundcolumnbefore", "2");
        check("nderivedcolumncountbefore", "2");
        check("cderivedcolumnwidthsbefore", "90,0");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_grid_columncount_defaults_materialize_columns_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_columncount";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_columncount.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasColumnCount = PEMSTATUS(oPlain, 'ColumnCount', 1)\n"
            "lPlainColumnCountReadOnly = PEMSTATUS(oPlain, 'ColumnCount', 5)\n"
            "nPlainBefore = oPlain.ColumnCount\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ColumnCount')\n"
            "lPlainHasColumns = PEMSTATUS(oPlain, 'Columns', 1)\n"
            "lPlainColumnsReadOnly = PEMSTATUS(oPlain, 'Columns', 5)\n"
            "nPlainColumnsCountBefore = oPlain.Columns.Count\n"
            "oPlain.ColumnCount = 0\n"
            "nPlainAfterDirectAssign = oPlain.ColumnCount\n"
            "nPlainColumnsCountAfterDirectAssign = oPlain.Columns.Count\n"
            "lPlainHasColumn1AfterDirectAssign = PEMSTATUS(oPlain, 'Column1', 1)\n"
            "lPlainSetPem = SETPEM(oPlain, 'ColumnCount', 2)\n"
            "nPlainAfterSetPem = oPlain.ColumnCount\n"
            "nPlainColumnsCountAfterSetPem = oPlain.Columns.Count\n"
            "cPlainColumn2Name = EVAL('oPlain.Column2.Name')\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ColumnCount', 3)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ColumnCount')\n"
            "lPlainColumnsAddProperty = ADDPROPERTY(oPlain, 'Columns', .NULL.)\n"
            "lPlainColumnsRemoveProperty = REMOVEPROPERTY(oPlain, 'Columns')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nGridBefore = oForm.grdLedger.ColumnCount\n"
            "nGridColumnsBefore = oForm.grdLedger.Columns.Count\n"
            "cGridColumn1Before = EVAL('oForm.grdLedger.Column1.Name')\n"
            "oForm.cmdProbe.GrowGrid()\n"
            "nGridAfterChild = oForm.grdLedger.ColumnCount\n"
            "nGridColumnsAfterChild = oForm.grdLedger.Columns.Count\n"
            "cGridColumn3AfterChild = EVAL('oForm.grdLedger.Column3.Name')\n"
            "lGridSetPem = SETPEM(oForm.grdLedger, 'ColumnCount', 1)\n"
            "nGridAfterSetPem = oForm.grdLedger.ColumnCount\n"
            "nGridColumnsAfterSetPem = oForm.grdLedger.Columns.Count\n"
            "lGridHasColumn2AfterSetPem = PEMSTATUS(oForm.grdLedger, 'Column2', 1)\n"
            "xGridGetPem = GETPEM(oForm.grdLedger, 'ColumnCount')\n"
            "lGridHasColumns = PEMSTATUS(oForm.grdLedger, 'Columns', 1)\n"
            "lGridColumnsReadOnly = PEMSTATUS(oForm.grdLedger, 'Columns', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdLedger, 1)\n"
            "lPropHasColumnCount = .F.\n"
            "lPropHasColumns = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    DO CASE\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNCOUNT'\n"
            "        lPropHasColumnCount = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'COLUMNS'\n"
            "        lPropHasColumns = .T.\n"
            "    ENDCASE\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('LedgerGrid')\n"
            "nDerivedBefore = oDerived.ColumnCount\n"
            "nDerivedColumnsBefore = oDerived.Columns.Count\n"
            "cDerivedColumn1Name = EVAL('oDerived.Column1.Name')\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    PROCEDURE GrowGrid\n"
            "        THISFORM.grdLedger.ColumnCount = 3\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LedgerGrid AS Grid\n"
            "    ColumnCount = 1\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdLedger AS LedgerGrid\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid ColumnCount property script should complete: ") + state.message +
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

        check("lplainhascolumncount", "true");
        check("lplaincolumncountreadonly", "false");
        check("nplainbefore", "-1");
        check("xplaingetpembefore", "-1");
        check("lplainhascolumns", "true");
        check("lplaincolumnsreadonly", "true");
        check("nplaincolumnscountbefore", "0");
        check("nplainafterdirectassign", "0");
        check("nplaincolumnscountafterdirectassign", "0");
        check("lplainhascolumn1afterdirectassign", "false");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "2");
        check("nplaincolumnscountaftersetpem", "2");
        check("cplaincolumn2name", "Column2");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lplaincolumnsaddproperty", "false");
        check("lplaincolumnsremoveproperty", "false");
        check("ngridbefore", "1");
        check("ngridcolumnsbefore", "1");
        check("cgridcolumn1before", "Column1");
        check("ngridafterchild", "3");
        check("ngridcolumnsafterchild", "3");
        check("cgridcolumn3afterchild", "Column3");
        check("lgridsetpem", "true");
        check("ngridaftersetpem", "1");
        check("ngridcolumnsaftersetpem", "1");
        check("lgridhascolumn2aftersetpem", "false");
        check("xgridgetpem", "1");
        check("lgridhascolumns", "true");
        check("lgridcolumnsreadonly", "true");
        check("lprophascolumncount", "true");
        check("lprophascolumns", "true");
        check("nderivedbefore", "1");
        check("nderivedcolumnsbefore", "1");
        check("cderivedcolumn1name", "Column1");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_column_bound_defaults_coordinate_controlsource_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_bound";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_bound.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Column')\n"
            "lPlainHasBound = PEMSTATUS(oPlain, 'Bound', 1)\n"
            "lPlainBoundReadOnly = PEMSTATUS(oPlain, 'Bound', 5)\n"
            "lPlainHasControlSource = PEMSTATUS(oPlain, 'ControlSource', 1)\n"
            "lPlainControlSourceReadOnly = PEMSTATUS(oPlain, 'ControlSource', 5)\n"
            "lPlainBoundBefore = oPlain.Bound\n"
            "cPlainControlSourceBefore = oPlain.ControlSource\n"
            "xPlainBoundGetPemBefore = GETPEM(oPlain, 'Bound')\n"
            "xPlainControlSourceGetPemBefore = GETPEM(oPlain, 'ControlSource')\n"
            "oPlain.Bound = .F.\n"
            "lPlainBoundAfterDirectAssign = oPlain.Bound\n"
            "lPlainBoundSetPem = SETPEM(oPlain, 'Bound', .T.)\n"
            "lPlainBoundAfterSetPem = oPlain.Bound\n"
            "oPlain.ControlSource = 'customer.company'\n"
            "cPlainControlSourceAfterDirectAssign = oPlain.ControlSource\n"
            "lPlainControlSourceSetPem = SETPEM(oPlain, 'ControlSource', 'customer.contact')\n"
            "cPlainControlSourceAfterSetPem = oPlain.ControlSource\n"
            "lPlainBoundAddProperty = ADDPROPERTY(oPlain, 'Bound', .F.)\n"
            "lPlainBoundRemoveProperty = REMOVEPROPERTY(oPlain, 'Bound')\n"
            "oBound = CREATEOBJECT('InvoiceColumn')\n"
            "lBoundBefore = oBound.Bound\n"
            "cBoundColumnControlSourceBefore = oBound.ControlSource\n"
            "cBoundChildControlSourceBefore = oBound.txtCell.ControlSource\n"
            "oBound.txtCell.ControlSource = 'orders.note'\n"
            "cBoundChildControlSourceAfterDirectBlocked = oBound.txtCell.ControlSource\n"
            "lBoundChildSetPemBlocked = SETPEM(oBound.txtCell, 'ControlSource', 'orders.memo')\n"
            "cBoundChildControlSourceAfterSetPemBlocked = oBound.txtCell.ControlSource\n"
            "oBound.Bound = .F.\n"
            "lBoundAfterUnbind = oBound.Bound\n"
            "oBound.txtCell.ControlSource = 'orders.note'\n"
            "cBoundChildControlSourceAfterDirectUnbound = oBound.txtCell.ControlSource\n"
            "lBoundChildSetPemUnbound = SETPEM(oBound.txtCell, 'ControlSource', 'orders.memo')\n"
            "cBoundChildControlSourceAfterSetPemUnbound = oBound.txtCell.ControlSource\n"
            "oBound.ControlSource = 'orders.total'\n"
            "cBoundColumnControlSourceAfterOverride = oBound.ControlSource\n"
            "cBoundChildControlSourceAfterOverride = oBound.txtCell.ControlSource\n"
            "nPropMembers = AMEMBERS(aPropMembers, oBound, 1)\n"
            "lPropHasBound = .F.\n"
            "lPropHasControlSource = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    DO CASE\n"
            "    CASE UPPER(aPropMembers[i]) == 'BOUND'\n"
            "        lPropHasBound = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'CONTROLSOURCE'\n"
            "        lPropHasControlSource = .T.\n"
            "    ENDCASE\n"
            "ENDFOR\n"
            "oFree = CREATEOBJECT('FreeColumn')\n"
            "lFreeBefore = oFree.Bound\n"
            "cFreeColumnControlSourceBefore = oFree.ControlSource\n"
            "cFreeChildControlSourceBefore = oFree.txtCell.ControlSource\n"
            "RETURN\n"
            "DEFINE CLASS InvoiceColumn AS Column\n"
            "    ControlSource = 'orders.status'\n"
            "    ADD OBJECT txtCell AS TextBox\n"
            "ENDDEFINE\n"
            "DEFINE CLASS FreeColumn AS Column\n"
            "    Bound = .F.\n"
            "    ControlSource = 'orders.header'\n"
            "    ADD OBJECT txtCell AS TextBox WITH ControlSource = 'orders.note'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column Bound property script should complete: ") + state.message +
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

        check("lplainhasbound", "true");
        check("lplainboundreadonly", "false");
        check("lplainhascontrolsource", "true");
        check("lplaincontrolsourcereadonly", "false");
        check("lplainboundbefore", "true");
        check("cplaincontrolsourcebefore", "");
        check("xplainboundgetpembefore", "true");
        check("xplaincontrolsourcegetpembefore", "");
        check("lplainboundafterdirectassign", "false");
        check("lplainboundsetpem", "true");
        check("lplainboundaftersetpem", "true");
        check("cplaincontrolsourceafterdirectassign", "customer.company");
        check("lplaincontrolsourcesetpem", "true");
        check("cplaincontrolsourceaftersetpem", "customer.contact");
        check("lplainboundaddproperty", "false");
        check("lplainboundremoveproperty", "false");
        check("lboundbefore", "true");
        check("cboundcolumncontrolsourcebefore", "orders.status");
        check("cboundchildcontrolsourcebefore", "orders.status");
        check("cboundchildcontrolsourceafterdirectblocked", "orders.status");
        check("lboundchildsetpemblocked", "false");
        check("cboundchildcontrolsourceaftersetpemblocked", "orders.status");
        check("lboundafterunbind", "false");
        check("cboundchildcontrolsourceafterdirectunbound", "orders.note");
        check("lboundchildsetpemunbound", "true");
        check("cboundchildcontrolsourceaftersetpemunbound", "orders.memo");
        check("cboundcolumncontrolsourceafteroverride", "orders.total");
        check("cboundchildcontrolsourceafteroverride", "orders.total");
        check("lprophasbound", "true");
        check("lprophascontrolsource", "true");
        check("lfreebefore", "false");
        check("cfreecolumncontrolsourcebefore", "orders.header");
        check("cfreechildcontrolsourcebefore", "orders.note");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_backcolor_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_backcolor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_backcolor.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Form')\n"
            "lPlainHasBackColor = PEMSTATUS(oPlain, 'BackColor', 1)\n"
            "lPlainBackColorReadOnly = PEMSTATUS(oPlain, 'BackColor', 5)\n"
            "nPlainBefore = oPlain.BackColor\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'BackColor')\n"
            "oPlain.BackColor = 255\n"
            "nPlainAfterDirectAssign = oPlain.BackColor\n"
            "lPlainSetPem = SETPEM(oPlain, 'BackColor', 65280)\n"
            "nPlainAfterSetPem = oPlain.BackColor\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'BackColor', 1)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'BackColor')\n"
            "oForm = CREATEOBJECT('PaintForm')\n"
            "nFormBefore = oForm.BackColor\n"
            "nChildBefore = oForm.cmdSave.BackColor\n"
            "nChildRead = oForm.cmdProbe.ReadBackColor()\n"
            "oForm.cmdProbe.PaintChild()\n"
            "nChildAfterChild = oForm.cmdSave.BackColor\n"
            "nFormAfterChild = oForm.BackColor\n"
            "lChildSetPem = SETPEM(oForm.cmdSave, 'BackColor', 16776960)\n"
            "nChildAfterSetPem = oForm.cmdSave.BackColor\n"
            "xChildGetPem = GETPEM(oForm.cmdSave, 'BackColor')\n"
            "lChildHasBackColor = PEMSTATUS(oForm.cmdSave, 'BackColor', 1)\n"
            "lChildBackColorReadOnly = PEMSTATUS(oForm.cmdSave, 'BackColor', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cmdSave, 1)\n"
            "lPropHasBackColor = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'BACKCOLOR'\n"
            "        lPropHasBackColor = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('PaintButton')\n"
            "nDerivedBefore = oDerived.BackColor\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadBackColor\n"
            "        RETURN THISFORM.cmdSave.BackColor\n"
            "    ENDFUNC\n"
            "    PROCEDURE PaintChild\n"
            "        THISFORM.cmdSave.BackColor = 65535\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PaintForm AS Form\n"
            "    BackColor = 16711935\n"
            "    ADD OBJECT cmdSave AS CommandButton WITH BackColor = 16711680\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PaintButton AS CommandButton\n"
            "    BackColor = 255\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual BackColor property script should complete: ") + state.message +
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

        check("lplainhasbackcolor", "true");
        check("lplainbackcolorreadonly", "false");
        check("nplainbefore", "16777215");
        check("xplaingetpembefore", "16777215");
        check("nplainafterdirectassign", "255");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "65280");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("nformbefore", "16711935");
        check("nchildbefore", "16711680");
        check("nchildread", "16711680");
        check("nchildafterchild", "65535");
        check("nformafterchild", "16711935");
        check("lchildsetpem", "true");
        check("nchildaftersetpem", "16776960");
        check("xchildgetpem", "16776960");
        check("lchildhasbackcolor", "true");
        check("lchildbackcolorreadonly", "false");
        check("lprophasbackcolor", "true");
        check("nderivedbefore", "255");

        fs::remove_all(temp_root, ignored);
    }

}
