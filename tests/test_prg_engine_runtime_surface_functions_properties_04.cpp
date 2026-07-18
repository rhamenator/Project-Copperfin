#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_splitbar_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_splitbar";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_splitbar.prg";
        write_text(
            main_path,
            "oBaseGrid = CREATEOBJECT('Grid')\n"
            "lBaseHasSplitBar = PEMSTATUS(oBaseGrid, 'SplitBar', 1)\n"
            "lBaseSplitBarReadOnly = PEMSTATUS(oBaseGrid, 'SplitBar', 5)\n"
            "lBaseBefore = oBaseGrid.SplitBar\n"
            "xBaseGetPemBefore = GETPEM(oBaseGrid, 'SplitBar')\n"
            "oBaseGrid.SplitBar = .F.\n"
            "lBaseAfterDirectAssign = oBaseGrid.SplitBar\n"
            "lBaseSetPem = SETPEM(oBaseGrid, 'SplitBar', .F.)\n"
            "lBaseAfterSetPem = oBaseGrid.SplitBar\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseGrid, 'SplitBar', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseGrid, 'SplitBar')\n"
            "oDerived = CREATEOBJECT('DemoGrid')\n"
            "lDerivedBefore = oDerived.SplitBar\n"
            "lChildBefore = oDerived.cmdProbe.ReadSplitBar()\n"
            "oDerived.cmdProbe.TrySplit()\n"
            "lDerivedAfterChild = oDerived.SplitBar\n"
            "xDerivedGetPem = GETPEM(oDerived, 'SplitBar')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasSplitBar = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SPLITBAR'\n"
            "        lPropHasSplitBar = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadSplitBar\n"
            "        RETURN THIS.Parent.SplitBar\n"
            "    ENDFUNC\n"
            "    PROCEDURE TrySplit\n"
            "        THIS.Parent.SplitBar = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoGrid AS Grid\n"
            "    SplitBar = .F.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SplitBar property script should complete: ") + state.message +
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

        check("lbasehassplitbar", "true");
        check("lbasesplitbarreadonly", "true");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "true");
        check("lbasesetpem", "false");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "false");
        check("lchildbefore", "false");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophassplitbar", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_allowaddnew_default_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_allowaddnew";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_allowaddnew.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasAllowAddNew = PEMSTATUS(oPlain, 'AllowAddNew', 1)\n"
            "lPlainAllowAddNewReadOnly = PEMSTATUS(oPlain, 'AllowAddNew', 5)\n"
            "lPlainBefore = oPlain.AllowAddNew\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'AllowAddNew')\n"
            "oPlain.AllowAddNew = .T.\n"
            "lPlainAfterDirectAssign = oPlain.AllowAddNew\n"
            "lPlainSetPem = SETPEM(oPlain, 'AllowAddNew', .F.)\n"
            "lPlainAfterSetPem = oPlain.AllowAddNew\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'AllowAddNew', .T.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'AllowAddNew')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lGridBefore = oForm.grdCust.AllowAddNew\n"
            "lGridRead = oForm.cmdProbe.ReadAllowAddNew()\n"
            "oForm.cmdProbe.RestyleGrid()\n"
            "lGridAfterChild = oForm.grdCust.AllowAddNew\n"
            "lGridSetPem = SETPEM(oForm.grdCust, 'AllowAddNew', .T.)\n"
            "lGridAfterSetPem = oForm.grdCust.AllowAddNew\n"
            "xGridGetPem = GETPEM(oForm.grdCust, 'AllowAddNew')\n"
            "lGridHasAllowAddNew = PEMSTATUS(oForm.grdCust, 'AllowAddNew', 1)\n"
            "lGridAllowAddNewReadOnly = PEMSTATUS(oForm.grdCust, 'AllowAddNew', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdCust, 1)\n"
            "lPropHasAllowAddNew = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ALLOWADDNEW'\n"
            "        lPropHasAllowAddNew = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('StyledGrid')\n"
            "lDerivedBefore = oDerived.AllowAddNew\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadAllowAddNew\n"
            "        RETURN THISFORM.grdCust.AllowAddNew\n"
            "    ENDFUNC\n"
            "    PROCEDURE RestyleGrid\n"
            "        THISFORM.grdCust.AllowAddNew = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdCust AS Grid WITH AllowAddNew = .T.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS StyledGrid AS Grid\n"
            "    AllowAddNew = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native AllowAddNew property script should complete: ") + state.message +
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

        check("lplainhasallowaddnew", "true");
        check("lplainallowaddnewreadonly", "false");
        check("lplainbefore", "false");
        check("xplaingetpembefore", "false");
        check("lplainafterdirectassign", "true");
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "false");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lgridbefore", "true");
        check("lgridread", "true");
        check("lgridafterchild", "false");
        check("lgridsetpem", "true");
        check("lgridaftersetpem", "true");
        check("xgridgetpem", "true");
        check("lgridhasallowaddnew", "true");
        check("lgridallowaddnewreadonly", "false");
        check("lprophasallowaddnew", "true");
        check("lderivedbefore", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_columnorder_default_reorders_siblings_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_columnorder";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_columnorder.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Column')\n"
            "lPlainHasColumnOrder = PEMSTATUS(oPlain, 'ColumnOrder', 1)\n"
            "lPlainColumnOrderReadOnly = PEMSTATUS(oPlain, 'ColumnOrder', 5)\n"
            "nPlainBefore = oPlain.ColumnOrder\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ColumnOrder')\n"
            "oPlain.ColumnOrder = 4\n"
            "nPlainAfterDirectAssign = oPlain.ColumnOrder\n"
            "lPlainSetPem = SETPEM(oPlain, 'ColumnOrder', 2)\n"
            "nPlainAfterSetPem = oPlain.ColumnOrder\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ColumnOrder', 9)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ColumnOrder')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nFirstBefore = oForm.grd.colFirst.ColumnOrder\n"
            "nSecondBefore = oForm.grd.colSecond.ColumnOrder\n"
            "nThirdBefore = oForm.grd.colThird.ColumnOrder\n"
            "nProbeRead = oForm.cmdProbe.ReadFirstOrder()\n"
            "oForm.cmdProbe.MoveFirstToThird()\n"
            "nFirstAfterChild = oForm.grd.colFirst.ColumnOrder\n"
            "nSecondAfterChild = oForm.grd.colSecond.ColumnOrder\n"
            "nThirdAfterChild = oForm.grd.colThird.ColumnOrder\n"
            "lThirdSetPem = SETPEM(oForm.grd.colThird, 'ColumnOrder', 1)\n"
            "nFirstAfterSetPem = oForm.grd.colFirst.ColumnOrder\n"
            "nSecondAfterSetPem = oForm.grd.colSecond.ColumnOrder\n"
            "nThirdAfterSetPem = oForm.grd.colThird.ColumnOrder\n"
            "xFirstGetPem = GETPEM(oForm.grd.colFirst, 'ColumnOrder')\n"
            "lGridHasColumnOrder = PEMSTATUS(oForm.grd.colFirst, 'ColumnOrder', 1)\n"
            "lGridColumnOrderReadOnly = PEMSTATUS(oForm.grd.colFirst, 'ColumnOrder', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grd.colFirst, 1)\n"
            "lPropHasColumnOrder = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'COLUMNORDER'\n"
            "        lPropHasColumnOrder = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('OrderedColumn')\n"
            "nDerivedBefore = oDerived.ColumnOrder\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadFirstOrder\n"
            "        RETURN THISFORM.grd.colFirst.ColumnOrder\n"
            "    ENDFUNC\n"
            "    PROCEDURE MoveFirstToThird\n"
            "        THISFORM.grd.colFirst.ColumnOrder = 3\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS OrderedGrid AS Grid\n"
            "    ADD OBJECT colFirst AS Column WITH ColumnOrder = 1\n"
            "    ADD OBJECT colSecond AS Column WITH ColumnOrder = 2\n"
            "    ADD OBJECT colThird AS Column WITH ColumnOrder = 3\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grd AS OrderedGrid\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS OrderedColumn AS Column\n"
            "    ColumnOrder = 7\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ColumnOrder property script should complete: ") + state.message +
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

        check("lplainhascolumnorder", "true");
        check("lplaincolumnorderreadonly", "false");
        check("nplainbefore", "1");
        check("xplaingetpembefore", "1");
        check("nplainafterdirectassign", "4");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "2");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("nfirstbefore", "1");
        check("nsecondbefore", "2");
        check("nthirdbefore", "3");
        check("nproberead", "1");
        check("nfirstafterchild", "3");
        check("nsecondafterchild", "1");
        check("nthirdafterchild", "2");
        check("lthirdsetpem", "true");
        check("nfirstaftersetpem", "3");
        check("nsecondaftersetpem", "2");
        check("nthirdaftersetpem", "1");
        check("xfirstgetpem", "3");
        check("lgridhascolumnorder", "true");
        check("lgridcolumnorderreadonly", "false");
        check("lprophascolumnorder", "true");
        check("nderivedbefore", "7");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_recordsourcetype_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_recordsourcetype";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_recordsourcetype.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasRecordSourceType = PEMSTATUS(oPlain, 'RecordSourceType', 1)\n"
            "lPlainRecordSourceTypeReadOnly = PEMSTATUS(oPlain, 'RecordSourceType', 5)\n"
            "nPlainBefore = oPlain.RecordSourceType\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'RecordSourceType')\n"
            "oPlain.RecordSourceType = 4\n"
            "nPlainAfterDirectAssign = oPlain.RecordSourceType\n"
            "lPlainSetPem = SETPEM(oPlain, 'RecordSourceType', 0)\n"
            "nPlainAfterSetPem = oPlain.RecordSourceType\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'RecordSourceType', 2)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'RecordSourceType')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nGridBefore = oForm.grdCust.RecordSourceType\n"
            "nGridRead = oForm.cmdProbe.ReadRecordSourceType()\n"
            "oForm.cmdProbe.RebindGrid()\n"
            "nGridAfterChild = oForm.grdCust.RecordSourceType\n"
            "lGridSetPem = SETPEM(oForm.grdCust, 'RecordSourceType', 1)\n"
            "nGridAfterSetPem = oForm.grdCust.RecordSourceType\n"
            "xGridGetPem = GETPEM(oForm.grdCust, 'RecordSourceType')\n"
            "lGridHasRecordSourceType = PEMSTATUS(oForm.grdCust, 'RecordSourceType', 1)\n"
            "lGridRecordSourceTypeReadOnly = PEMSTATUS(oForm.grdCust, 'RecordSourceType', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdCust, 1)\n"
            "lPropHasRecordSourceType = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'RECORDSOURCETYPE'\n"
            "        lPropHasRecordSourceType = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundGrid')\n"
            "nDerivedBefore = oDerived.RecordSourceType\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadRecordSourceType\n"
            "        RETURN THISFORM.grdCust.RecordSourceType\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindGrid\n"
            "        THISFORM.grdCust.RecordSourceType = 3\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdCust AS Grid WITH RecordSourceType = 4\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundGrid AS Grid\n"
            "    RecordSourceType = 2\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RecordSourceType property script should complete: ") + state.message +
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

        check("lplainhasrecordsourcetype", "true");
        check("lplainrecordsourcetypereadonly", "false");
        check("nplainbefore", "1");
        check("xplaingetpembefore", "1");
        check("nplainafterdirectassign", "4");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "0");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ngridbefore", "4");
        check("ngridread", "4");
        check("ngridafterchild", "3");
        check("lgridsetpem", "true");
        check("ngridaftersetpem", "1");
        check("xgridgetpem", "1");
        check("lgridhasrecordsourcetype", "true");
        check("lgridrecordsourcetypereadonly", "false");
        check("lprophasrecordsourcetype", "true");
        check("nderivedbefore", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_rowsource_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_rowsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_rowsource.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasRowSource = PEMSTATUS(oPlain, 'RowSource', 1)\n"
            "lPlainRowSourceReadOnly = PEMSTATUS(oPlain, 'RowSource', 5)\n"
            "cPlainBefore = oPlain.RowSource\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'RowSource')\n"
            "oPlain.RowSource = 'months_alias'\n"
            "cPlainAfterDirectAssign = oPlain.RowSource\n"
            "lPlainSetPem = SETPEM(oPlain, 'RowSource', 'calendar_alias')\n"
            "cPlainAfterSetPem = oPlain.RowSource\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'RowSource', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'RowSource')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cComboBefore = oForm.cboMonth.RowSource\n"
            "cListBefore = oForm.lstYear.RowSource\n"
            "cComboRead = oForm.cmdProbe.ReadComboRowSource()\n"
            "cListRead = oForm.cmdProbe.ReadListRowSource()\n"
            "oForm.cmdProbe.RebindLists()\n"
            "cComboAfterChild = oForm.cboMonth.RowSource\n"
            "cListAfterChild = oForm.lstYear.RowSource\n"
            "lComboSetPem = SETPEM(oForm.cboMonth, 'RowSource', 'fiscal_months')\n"
            "cComboAfterSetPem = oForm.cboMonth.RowSource\n"
            "xComboGetPem = GETPEM(oForm.cboMonth, 'RowSource')\n"
            "lComboHasRowSource = PEMSTATUS(oForm.cboMonth, 'RowSource', 1)\n"
            "lComboRowSourceReadOnly = PEMSTATUS(oForm.cboMonth, 'RowSource', 5)\n"
            "lListSetPem = SETPEM(oForm.lstYear, 'RowSource', 'fiscal_years')\n"
            "cListAfterSetPem = oForm.lstYear.RowSource\n"
            "xListGetPem = GETPEM(oForm.lstYear, 'RowSource')\n"
            "lListHasRowSource = PEMSTATUS(oForm.lstYear, 'RowSource', 1)\n"
            "lListRowSourceReadOnly = PEMSTATUS(oForm.lstYear, 'RowSource', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.lstYear, 1)\n"
            "lPropHasRowSource = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ROWSOURCE'\n"
            "        lPropHasRowSource = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundList')\n"
            "cDerivedBefore = oDerived.RowSource\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadComboRowSource\n"
            "        RETURN THISFORM.cboMonth.RowSource\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadListRowSource\n"
            "        RETURN THISFORM.lstYear.RowSource\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindLists\n"
            "        THISFORM.cboMonth.RowSource = 'quarter_alias'\n"
            "        THISFORM.lstYear.RowSource = 'year_alias'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH RowSource = 'report_months'\n"
            "    ADD OBJECT lstYear AS ListBox WITH RowSource = 'report_years'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundList AS ListBox\n"
            "    RowSource = 'customer_regions'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RowSource property script should complete: ") + state.message +
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

        check("lplainhasrowsource", "true");
        check("lplainrowsourcereadonly", "false");
        check("cplainbefore", "");
        check("xplaingetpembefore", "");
        check("cplainafterdirectassign", "months_alias");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "calendar_alias");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ccombobefore", "report_months");
        check("clistbefore", "report_years");
        check("ccomboread", "report_months");
        check("clistread", "report_years");
        check("ccomboafterchild", "quarter_alias");
        check("clistafterchild", "year_alias");
        check("lcombosetpem", "true");
        check("ccomboaftersetpem", "fiscal_months");
        check("xcombogetpem", "fiscal_months");
        check("lcombohasrowsource", "true");
        check("lcomborowsourcereadonly", "false");
        check("llistsetpem", "true");
        check("clistaftersetpem", "fiscal_years");
        check("xlistgetpem", "fiscal_years");
        check("llisthasrowsource", "true");
        check("llistrowsourcereadonly", "false");
        check("lprophasrowsource", "true");
        check("cderivedbefore", "customer_regions");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_rowsourcetype_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_rowsourcetype";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_rowsourcetype.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasRowSourceType = PEMSTATUS(oPlain, 'RowSourceType', 1)\n"
            "lPlainRowSourceTypeReadOnly = PEMSTATUS(oPlain, 'RowSourceType', 5)\n"
            "nPlainBefore = oPlain.RowSourceType\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'RowSourceType')\n"
            "oPlain.RowSourceType = 1\n"
            "nPlainAfterDirectAssign = oPlain.RowSourceType\n"
            "lPlainSetPem = SETPEM(oPlain, 'RowSourceType', 6)\n"
            "nPlainAfterSetPem = oPlain.RowSourceType\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'RowSourceType', 2)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'RowSourceType')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nComboBefore = oForm.cboMonth.RowSourceType\n"
            "nListBefore = oForm.lstYear.RowSourceType\n"
            "nComboRead = oForm.cmdProbe.ReadComboRowSourceType()\n"
            "nListRead = oForm.cmdProbe.ReadListRowSourceType()\n"
            "oForm.cmdProbe.RebindLists()\n"
            "nComboAfterChild = oForm.cboMonth.RowSourceType\n"
            "nListAfterChild = oForm.lstYear.RowSourceType\n"
            "lComboSetPem = SETPEM(oForm.cboMonth, 'RowSourceType', 2)\n"
            "nComboAfterSetPem = oForm.cboMonth.RowSourceType\n"
            "xComboGetPem = GETPEM(oForm.cboMonth, 'RowSourceType')\n"
            "lComboHasRowSourceType = PEMSTATUS(oForm.cboMonth, 'RowSourceType', 1)\n"
            "lComboRowSourceTypeReadOnly = PEMSTATUS(oForm.cboMonth, 'RowSourceType', 5)\n"
            "lListSetPem = SETPEM(oForm.lstYear, 'RowSourceType', 7)\n"
            "nListAfterSetPem = oForm.lstYear.RowSourceType\n"
            "xListGetPem = GETPEM(oForm.lstYear, 'RowSourceType')\n"
            "lListHasRowSourceType = PEMSTATUS(oForm.lstYear, 'RowSourceType', 1)\n"
            "lListRowSourceTypeReadOnly = PEMSTATUS(oForm.lstYear, 'RowSourceType', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.lstYear, 1)\n"
            "lPropHasRowSourceType = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ROWSOURCETYPE'\n"
            "        lPropHasRowSourceType = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundList')\n"
            "nDerivedBefore = oDerived.RowSourceType\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadComboRowSourceType\n"
            "        RETURN THISFORM.cboMonth.RowSourceType\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadListRowSourceType\n"
            "        RETURN THISFORM.lstYear.RowSourceType\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindLists\n"
            "        THISFORM.cboMonth.RowSourceType = 5\n"
            "        THISFORM.lstYear.RowSourceType = 3\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH RowSourceType = 6\n"
            "    ADD OBJECT lstYear AS ListBox WITH RowSourceType = 2\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundList AS ListBox\n"
            "    RowSourceType = 4\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RowSourceType property script should complete: ") + state.message +
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

        check("lplainhasrowsourcetype", "true");
        check("lplainrowsourcetypereadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
        check("nplainafterdirectassign", "1");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "6");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ncombobefore", "6");
        check("nlistbefore", "2");
        check("ncomboread", "6");
        check("nlistread", "2");
        check("ncomboafterchild", "5");
        check("nlistafterchild", "3");
        check("lcombosetpem", "true");
        check("ncomboaftersetpem", "2");
        check("xcombogetpem", "2");
        check("lcombohasrowsourcetype", "true");
        check("lcomborowsourcetypereadonly", "false");
        check("llistsetpem", "true");
        check("nlistaftersetpem", "7");
        check("xlistgetpem", "7");
        check("llisthasrowsourcetype", "true");
        check("llistrowsourcetypereadonly", "false");
        check("lprophasrowsourcetype", "true");
        check("nderivedbefore", "4");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_combobox_style_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_combobox_style";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_combobox_style.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('ComboBox')\n"
            "lPlainHasStyle = PEMSTATUS(oPlain, 'Style', 1)\n"
            "lPlainStyleReadOnly = PEMSTATUS(oPlain, 'Style', 5)\n"
            "nPlainBefore = oPlain.Style\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'Style')\n"
            "oPlain.Style = 2\n"
            "nPlainAfterDirectAssign = oPlain.Style\n"
            "lPlainSetPem = SETPEM(oPlain, 'Style', 0)\n"
            "nPlainAfterSetPem = oPlain.Style\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'Style', 1)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'Style')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "nChildBefore = oForm.cboMonth.Style\n"
            "nChildRead = oForm.cmdProbe.ReadStyle()\n"
            "oForm.cmdProbe.MakeDropDownList()\n"
            "nChildAfterChild = oForm.cboMonth.Style\n"
            "lChildSetPem = SETPEM(oForm.cboMonth, 'Style', 0)\n"
            "nChildAfterSetPem = oForm.cboMonth.Style\n"
            "xChildGetPem = GETPEM(oForm.cboMonth, 'Style')\n"
            "lChildHasStyle = PEMSTATUS(oForm.cboMonth, 'Style', 1)\n"
            "lChildStyleReadOnly = PEMSTATUS(oForm.cboMonth, 'Style', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cboMonth, 1)\n"
            "lPropHasStyle = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'STYLE'\n"
            "        lPropHasStyle = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('PickerCombo')\n"
            "nDerivedBefore = oDerived.Style\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadStyle\n"
            "        RETURN THISFORM.cboMonth.Style\n"
            "    ENDFUNC\n"
            "    PROCEDURE MakeDropDownList\n"
            "        THISFORM.cboMonth.Style = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH Style = 2\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PickerCombo AS ComboBox\n"
            "    Style = 2\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ComboBox Style property script should complete: ") + state.message +
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

        check("lplainhasstyle", "true");
        check("lplainstylereadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
        check("nplainafterdirectassign", "2");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "0");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("nchildbefore", "2");
        check("nchildread", "2");
        check("nchildafterchild", "2");
        check("lchildsetpem", "true");
        check("nchildaftersetpem", "0");
        check("xchildgetpem", "0");
        check("lchildhasstyle", "true");
        check("lchildstylereadonly", "false");
        check("lprophasstyle", "true");
        check("nderivedbefore", "2");

        fs::remove_all(temp_root, ignored);
    }

}
