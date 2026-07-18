#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_controlsource_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_controlsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_controlsource.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('TextBox')\n"
            "lPlainHasControlSource = PEMSTATUS(oPlain, 'ControlSource', 1)\n"
            "lPlainControlSourceReadOnly = PEMSTATUS(oPlain, 'ControlSource', 5)\n"
            "cPlainBefore = oPlain.ControlSource\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ControlSource')\n"
            "oPlain.ControlSource = 'customer.first_name'\n"
            "cPlainAfterDirectAssign = oPlain.ControlSource\n"
            "lPlainSetPem = SETPEM(oPlain, 'ControlSource', 'customer.last_name')\n"
            "cPlainAfterSetPem = oPlain.ControlSource\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ControlSource', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ControlSource')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cChildBefore = oForm.cboMonth.ControlSource\n"
            "cChildRead = oForm.cmdProbe.ReadControlSource()\n"
            "oForm.cmdProbe.BindMonth()\n"
            "cChildAfterChild = oForm.cboMonth.ControlSource\n"
            "lChildSetPem = SETPEM(oForm.cboMonth, 'ControlSource', 'invoice.fiscal_month')\n"
            "cChildAfterSetPem = oForm.cboMonth.ControlSource\n"
            "xChildGetPem = GETPEM(oForm.cboMonth, 'ControlSource')\n"
            "lChildHasControlSource = PEMSTATUS(oForm.cboMonth, 'ControlSource', 1)\n"
            "lChildControlSourceReadOnly = PEMSTATUS(oForm.cboMonth, 'ControlSource', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cboMonth, 1)\n"
            "lPropHasControlSource = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CONTROLSOURCE'\n"
            "        lPropHasControlSource = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundEdit')\n"
            "cDerivedBefore = oDerived.ControlSource\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadControlSource\n"
            "        RETURN THISFORM.cboMonth.ControlSource\n"
            "    ENDFUNC\n"
            "    PROCEDURE BindMonth\n"
            "        THISFORM.cboMonth.ControlSource = 'invoice.calendar_month'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox WITH ControlSource = 'invoice.report_month'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundEdit AS EditBox\n"
            "    ControlSource = 'customer.notes'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ControlSource property script should complete: ") + state.message +
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

        check("lplainhascontrolsource", "true");
        check("lplaincontrolsourcereadonly", "false");
        check("cplainbefore", "");
        check("xplaingetpembefore", "");
        check("cplainafterdirectassign", "customer.first_name");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "customer.last_name");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("cchildbefore", "invoice.report_month");
        check("cchildread", "invoice.report_month");
        check("cchildafterchild", "invoice.calendar_month");
        check("lchildsetpem", "true");
        check("cchildaftersetpem", "invoice.fiscal_month");
        check("xchildgetpem", "invoice.fiscal_month");
        check("lchildhascontrolsource", "true");
        check("lchildcontrolsourcereadonly", "false");
        check("lprophascontrolsource", "true");
        check("cderivedbefore", "customer.notes");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_currentcontrol_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_currentcontrol";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_currentcontrol.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Column')\n"
            "lPlainHasCurrentControl = PEMSTATUS(oPlain, 'CurrentControl', 1)\n"
            "lPlainCurrentControlReadOnly = PEMSTATUS(oPlain, 'CurrentControl', 5)\n"
            "cPlainBefore = oPlain.CurrentControl\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'CurrentControl')\n"
            "oPlain.CurrentControl = 'chkReady'\n"
            "cPlainAfterDirectAssign = oPlain.CurrentControl\n"
            "lPlainSetPem = SETPEM(oPlain, 'CurrentControl', 'cmbReady')\n"
            "cPlainAfterSetPem = oPlain.CurrentControl\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'CurrentControl', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'CurrentControl')\n"
            "oGrid = CREATEOBJECT('MainGrid')\n"
            "cChildBefore = oGrid.colStatus.CurrentControl\n"
            "cChildRead = oGrid.ReadCurrentControl()\n"
            "oGrid.RebindControl()\n"
            "cChildAfterChild = oGrid.colStatus.CurrentControl\n"
            "lChildSetPem = SETPEM(oGrid.colStatus, 'CurrentControl', 'spnStatus')\n"
            "cChildAfterSetPem = oGrid.colStatus.CurrentControl\n"
            "xChildGetPem = GETPEM(oGrid.colStatus, 'CurrentControl')\n"
            "lChildHasCurrentControl = PEMSTATUS(oGrid.colStatus, 'CurrentControl', 1)\n"
            "lChildCurrentControlReadOnly = PEMSTATUS(oGrid.colStatus, 'CurrentControl', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid.colStatus, 1)\n"
            "lPropHasCurrentControl = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CURRENTCONTROL'\n"
            "        lPropHasCurrentControl = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundColumn')\n"
            "cDerivedBefore = oDerived.CurrentControl\n"
            "RETURN\n"
            "DEFINE CLASS MainGrid AS Grid\n"
            "    ADD OBJECT colStatus AS Column WITH CurrentControl = 'chkStatus'\n"
            "    FUNCTION ReadCurrentControl\n"
            "        RETURN THIS.colStatus.CurrentControl\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindControl\n"
            "        THIS.colStatus.CurrentControl = 'cmbStatus'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundColumn AS Column\n"
            "    CurrentControl = 'spnHours'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CurrentControl property script should complete: ") + state.message +
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

        check("lplainhascurrentcontrol", "true");
        check("lplaincurrentcontrolreadonly", "false");
        check("cplainbefore", "Text1");
        check("xplaingetpembefore", "Text1");
        check("cplainafterdirectassign", "chkReady");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "cmbReady");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("cchildbefore", "chkStatus");
        check("cchildread", "chkStatus");
        check("cchildafterchild", "cmbStatus");
        check("lchildsetpem", "true");
        check("cchildaftersetpem", "spnStatus");
        check("xchildgetpem", "spnStatus");
        check("lchildhascurrentcontrol", "true");
        check("lchildcurrentcontrolreadonly", "false");
        check("lprophascurrentcontrol", "true");
        check("cderivedbefore", "spnHours");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_dynamiccurrentcontrol_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_dynamiccurrentcontrol";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_dynamiccurrentcontrol.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Column')\n"
            "lPlainHasDynamicCurrentControl = PEMSTATUS(oPlain, 'DynamicCurrentControl', 1)\n"
            "lPlainDynamicCurrentControlReadOnly = PEMSTATUS(oPlain, 'DynamicCurrentControl', 5)\n"
            "cPlainBefore = oPlain.DynamicCurrentControl\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'DynamicCurrentControl')\n"
            "oPlain.DynamicCurrentControl = 'chkReady'\n"
            "cPlainAfterDirectAssign = oPlain.DynamicCurrentControl\n"
            "lPlainSetPem = SETPEM(oPlain, 'DynamicCurrentControl', 'cmbReady')\n"
            "cPlainAfterSetPem = oPlain.DynamicCurrentControl\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'DynamicCurrentControl', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'DynamicCurrentControl')\n"
            "oGrid = CREATEOBJECT('MainGrid')\n"
            "cChildBefore = oGrid.colStatus.DynamicCurrentControl\n"
            "cChildRead = oGrid.ReadDynamicCurrentControl()\n"
            "oGrid.RebindControl()\n"
            "cChildAfterChild = oGrid.colStatus.DynamicCurrentControl\n"
            "lChildSetPem = SETPEM(oGrid.colStatus, 'DynamicCurrentControl', 'spnStatus')\n"
            "cChildAfterSetPem = oGrid.colStatus.DynamicCurrentControl\n"
            "xChildGetPem = GETPEM(oGrid.colStatus, 'DynamicCurrentControl')\n"
            "lChildHasDynamicCurrentControl = PEMSTATUS(oGrid.colStatus, 'DynamicCurrentControl', 1)\n"
            "lChildDynamicCurrentControlReadOnly = PEMSTATUS(oGrid.colStatus, 'DynamicCurrentControl', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oGrid.colStatus, 1)\n"
            "lPropHasDynamicCurrentControl = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DYNAMICCURRENTCONTROL'\n"
            "        lPropHasDynamicCurrentControl = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundColumn')\n"
            "cDerivedBefore = oDerived.DynamicCurrentControl\n"
            "RETURN\n"
            "DEFINE CLASS MainGrid AS Grid\n"
            "    ADD OBJECT colStatus AS Column WITH DynamicCurrentControl = 'chkStatus'\n"
            "    FUNCTION ReadDynamicCurrentControl\n"
            "        RETURN THIS.colStatus.DynamicCurrentControl\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindControl\n"
            "        THIS.colStatus.DynamicCurrentControl = 'cmbStatus'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundColumn AS Column\n"
            "    DynamicCurrentControl = 'spnHours'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicCurrentControl property script should complete: ") + state.message +
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

        check("lplainhasdynamiccurrentcontrol", "true");
        check("lplaindynamiccurrentcontrolreadonly", "false");
        check("cplainbefore", "Text1");
        check("xplaingetpembefore", "Text1");
        check("cplainafterdirectassign", "chkReady");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "cmbReady");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("cchildbefore", "chkStatus");
        check("cchildread", "chkStatus");
        check("cchildafterchild", "cmbStatus");
        check("lchildsetpem", "true");
        check("cchildaftersetpem", "spnStatus");
        check("xchildgetpem", "spnStatus");
        check("lchildhasdynamiccurrentcontrol", "true");
        check("lchilddynamiccurrentcontrolreadonly", "false");
        check("lprophasdynamiccurrentcontrol", "true");
        check("cderivedbefore", "spnHours");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_recordsource_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_recordsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_recordsource.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasRecordSource = PEMSTATUS(oPlain, 'RecordSource', 1)\n"
            "lPlainRecordSourceReadOnly = PEMSTATUS(oPlain, 'RecordSource', 5)\n"
            "cPlainBefore = oPlain.RecordSource\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'RecordSource')\n"
            "oPlain.RecordSource = 'customer_alias'\n"
            "cPlainAfterDirectAssign = oPlain.RecordSource\n"
            "lPlainSetPem = SETPEM(oPlain, 'RecordSource', 'invoice_alias')\n"
            "cPlainAfterSetPem = oPlain.RecordSource\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'RecordSource', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'RecordSource')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cGridBefore = oForm.grdCust.RecordSource\n"
            "cGridRead = oForm.cmdProbe.ReadRecordSource()\n"
            "oForm.cmdProbe.RebindGrid()\n"
            "cGridAfterChild = oForm.grdCust.RecordSource\n"
            "lGridSetPem = SETPEM(oForm.grdCust, 'RecordSource', 'history_alias')\n"
            "cGridAfterSetPem = oForm.grdCust.RecordSource\n"
            "xGridGetPem = GETPEM(oForm.grdCust, 'RecordSource')\n"
            "lGridHasRecordSource = PEMSTATUS(oForm.grdCust, 'RecordSource', 1)\n"
            "lGridRecordSourceReadOnly = PEMSTATUS(oForm.grdCust, 'RecordSource', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdCust, 1)\n"
            "lPropHasRecordSource = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'RECORDSOURCE'\n"
            "        lPropHasRecordSource = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('BoundGrid')\n"
            "cDerivedBefore = oDerived.RecordSource\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadRecordSource\n"
            "        RETURN THISFORM.grdCust.RecordSource\n"
            "    ENDFUNC\n"
            "    PROCEDURE RebindGrid\n"
            "        THISFORM.grdCust.RecordSource = 'report_alias'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdCust AS Grid WITH RecordSource = 'customer_view'\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BoundGrid AS Grid\n"
            "    RecordSource = 'orders_cursor'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RecordSource property script should complete: ") + state.message +
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

        check("lplainhasrecordsource", "true");
        check("lplainrecordsourcereadonly", "false");
        check("cplainbefore", "");
        check("xplaingetpembefore", "");
        check("cplainafterdirectassign", "customer_alias");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "invoice_alias");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("cgridbefore", "customer_view");
        check("cgridread", "customer_view");
        check("cgridafterchild", "report_alias");
        check("lgridsetpem", "true");
        check("cgridaftersetpem", "history_alias");
        check("xgridgetpem", "history_alias");
        check("lgridhasrecordsource", "true");
        check("lgridrecordsourcereadonly", "false");
        check("lprophasrecordsource", "true");
        check("cderivedbefore", "orders_cursor");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_leftcolumn_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_leftcolumn";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_leftcolumn.prg";
        write_text(
            main_path,
            "oBaseGrid = CREATEOBJECT('Grid')\n"
            "lBaseHasLeftColumn = PEMSTATUS(oBaseGrid, 'LeftColumn', 1)\n"
            "lBaseLeftColumnReadOnly = PEMSTATUS(oBaseGrid, 'LeftColumn', 5)\n"
            "nBaseBefore = oBaseGrid.LeftColumn\n"
            "xBaseGetPemBefore = GETPEM(oBaseGrid, 'LeftColumn')\n"
            "oBaseGrid.LeftColumn = 3\n"
            "nBaseAfterDirectAssign = oBaseGrid.LeftColumn\n"
            "lBaseSetPem = SETPEM(oBaseGrid, 'LeftColumn', 2)\n"
            "nBaseAfterSetPem = oBaseGrid.LeftColumn\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseGrid, 'LeftColumn', 4)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseGrid, 'LeftColumn')\n"
            "oDerived = CREATEOBJECT('DemoGrid')\n"
            "nDerivedBefore = oDerived.LeftColumn\n"
            "nChildBefore = oDerived.cmdProbe.ReadLeftColumn()\n"
            "oDerived.cmdProbe.TryScroll()\n"
            "nDerivedAfterChild = oDerived.LeftColumn\n"
            "xDerivedGetPem = GETPEM(oDerived, 'LeftColumn')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasLeftColumn = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'LEFTCOLUMN'\n"
            "        lPropHasLeftColumn = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadLeftColumn\n"
            "        RETURN THIS.Parent.LeftColumn\n"
            "    ENDFUNC\n"
            "    PROCEDURE TryScroll\n"
            "        THIS.Parent.LeftColumn = 5\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoGrid AS Grid\n"
            "    LeftColumn = 3\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native LeftColumn property script should complete: ") + state.message +
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

        check("lbasehasleftcolumn", "true");
        check("lbaseleftcolumnreadonly", "true");
        check("nbasebefore", "1");
        check("xbasegetpembefore", "1");
        check("nbaseafterdirectassign", "1");
        check("lbasesetpem", "false");
        check("nbaseaftersetpem", "1");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "3");
        check("nchildbefore", "3");
        check("nderivedafterchild", "3");
        check("xderivedgetpem", "3");
        check("lprophasleftcolumn", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_grid_display_properties_default_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_display_properties";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_display_properties.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasAllowCellSelection = PEMSTATUS(oPlain, 'AllowCellSelection', 1)\n"
            "lPlainAllowCellSelectionReadOnly = PEMSTATUS(oPlain, 'AllowCellSelection', 5)\n"
            "lPlainHasGridLines = PEMSTATUS(oPlain, 'GridLines', 1)\n"
            "lPlainGridLinesReadOnly = PEMSTATUS(oPlain, 'GridLines', 5)\n"
            "lPlainHasHighlight = PEMSTATUS(oPlain, 'Highlight', 1)\n"
            "lPlainHighlightReadOnly = PEMSTATUS(oPlain, 'Highlight', 5)\n"
            "lPlainHasHighlightRow = PEMSTATUS(oPlain, 'HighlightRow', 1)\n"
            "lPlainHighlightRowReadOnly = PEMSTATUS(oPlain, 'HighlightRow', 5)\n"
            "lPlainHasRecordMark = PEMSTATUS(oPlain, 'RecordMark', 1)\n"
            "lPlainRecordMarkReadOnly = PEMSTATUS(oPlain, 'RecordMark', 5)\n"
            "lPlainAllowBefore = oPlain.AllowCellSelection\n"
            "nPlainGridLinesBefore = oPlain.GridLines\n"
            "lPlainHighlightBefore = oPlain.Highlight\n"
            "lPlainHighlightRowBefore = oPlain.HighlightRow\n"
            "lPlainRecordMarkBefore = oPlain.RecordMark\n"
            "xPlainAllowGetPemBefore = GETPEM(oPlain, 'AllowCellSelection')\n"
            "xPlainGridLinesGetPemBefore = GETPEM(oPlain, 'GridLines')\n"
            "oPlain.AllowCellSelection = .F.\n"
            "oPlain.GridLines = 2\n"
            "oPlain.Highlight = .F.\n"
            "oPlain.HighlightRow = .F.\n"
            "oPlain.RecordMark = .F.\n"
            "lPlainAllowAfterDirectAssign = oPlain.AllowCellSelection\n"
            "nPlainGridLinesAfterDirectAssign = oPlain.GridLines\n"
            "lPlainHighlightAfterDirectAssign = oPlain.Highlight\n"
            "lPlainHighlightRowAfterDirectAssign = oPlain.HighlightRow\n"
            "lPlainRecordMarkAfterDirectAssign = oPlain.RecordMark\n"
            "lPlainAllowSetPem = SETPEM(oPlain, 'AllowCellSelection', .T.)\n"
            "lPlainGridLinesSetPem = SETPEM(oPlain, 'GridLines', 1)\n"
            "lPlainHighlightSetPem = SETPEM(oPlain, 'Highlight', .T.)\n"
            "lPlainHighlightRowSetPem = SETPEM(oPlain, 'HighlightRow', .T.)\n"
            "lPlainRecordMarkSetPem = SETPEM(oPlain, 'RecordMark', .T.)\n"
            "lPlainAllowAfterSetPem = oPlain.AllowCellSelection\n"
            "nPlainGridLinesAfterSetPem = oPlain.GridLines\n"
            "lPlainHighlightAfterSetPem = oPlain.Highlight\n"
            "lPlainHighlightRowAfterSetPem = oPlain.HighlightRow\n"
            "lPlainRecordMarkAfterSetPem = oPlain.RecordMark\n"
            "lPlainAllowAddProperty = ADDPROPERTY(oPlain, 'AllowCellSelection', .F.)\n"
            "lPlainAllowRemoveProperty = REMOVEPROPERTY(oPlain, 'AllowCellSelection')\n"
            "lPlainGridLinesAddProperty = ADDPROPERTY(oPlain, 'GridLines', 0)\n"
            "lPlainGridLinesRemoveProperty = REMOVEPROPERTY(oPlain, 'GridLines')\n"
            "lPlainHighlightAddProperty = ADDPROPERTY(oPlain, 'Highlight', .F.)\n"
            "lPlainHighlightRemoveProperty = REMOVEPROPERTY(oPlain, 'Highlight')\n"
            "lPlainHighlightRowAddProperty = ADDPROPERTY(oPlain, 'HighlightRow', .F.)\n"
            "lPlainHighlightRowRemoveProperty = REMOVEPROPERTY(oPlain, 'HighlightRow')\n"
            "lPlainRecordMarkAddProperty = ADDPROPERTY(oPlain, 'RecordMark', .F.)\n"
            "lPlainRecordMarkRemoveProperty = REMOVEPROPERTY(oPlain, 'RecordMark')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lGridAllowBefore = oForm.grdCust.AllowCellSelection\n"
            "nGridGridLinesBefore = oForm.grdCust.GridLines\n"
            "lGridHighlightBefore = oForm.grdCust.Highlight\n"
            "lGridHighlightRowBefore = oForm.grdCust.HighlightRow\n"
            "lGridRecordMarkBefore = oForm.grdCust.RecordMark\n"
            "lGridAllowRead = oForm.cmdProbe.ReadAllowCellSelection()\n"
            "nGridGridLinesRead = oForm.cmdProbe.ReadGridLines()\n"
            "lGridHighlightRead = oForm.cmdProbe.ReadHighlight()\n"
            "lGridHighlightRowRead = oForm.cmdProbe.ReadHighlightRow()\n"
            "lGridRecordMarkRead = oForm.cmdProbe.ReadRecordMark()\n"
            "oForm.cmdProbe.RestyleGrid()\n"
            "lGridAllowAfterChild = oForm.grdCust.AllowCellSelection\n"
            "nGridGridLinesAfterChild = oForm.grdCust.GridLines\n"
            "lGridHighlightAfterChild = oForm.grdCust.Highlight\n"
            "lGridHighlightRowAfterChild = oForm.grdCust.HighlightRow\n"
            "lGridRecordMarkAfterChild = oForm.grdCust.RecordMark\n"
            "xGridAllowGetPem = GETPEM(oForm.grdCust, 'AllowCellSelection')\n"
            "xGridGridLinesGetPem = GETPEM(oForm.grdCust, 'GridLines')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdCust, 1)\n"
            "lPropHasAllowCellSelection = .F.\n"
            "lPropHasGridLines = .F.\n"
            "lPropHasHighlight = .F.\n"
            "lPropHasHighlightRow = .F.\n"
            "lPropHasRecordMark = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    DO CASE\n"
            "    CASE UPPER(aPropMembers[i]) == 'ALLOWCELLSELECTION'\n"
            "        lPropHasAllowCellSelection = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'GRIDLINES'\n"
            "        lPropHasGridLines = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'HIGHLIGHTROW'\n"
            "        lPropHasHighlightRow = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'HIGHLIGHT'\n"
            "        lPropHasHighlight = .T.\n"
            "    CASE UPPER(aPropMembers[i]) == 'RECORDMARK'\n"
            "        lPropHasRecordMark = .T.\n"
            "    ENDCASE\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('StyledGrid')\n"
            "lDerivedAllowBefore = oDerived.AllowCellSelection\n"
            "nDerivedGridLinesBefore = oDerived.GridLines\n"
            "lDerivedHighlightBefore = oDerived.Highlight\n"
            "lDerivedHighlightRowBefore = oDerived.HighlightRow\n"
            "lDerivedRecordMarkBefore = oDerived.RecordMark\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadAllowCellSelection\n"
            "        RETURN THISFORM.grdCust.AllowCellSelection\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadGridLines\n"
            "        RETURN THISFORM.grdCust.GridLines\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadHighlight\n"
            "        RETURN THISFORM.grdCust.Highlight\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadHighlightRow\n"
            "        RETURN THISFORM.grdCust.HighlightRow\n"
            "    ENDFUNC\n"
            "    FUNCTION ReadRecordMark\n"
            "        RETURN THISFORM.grdCust.RecordMark\n"
            "    ENDFUNC\n"
            "    PROCEDURE RestyleGrid\n"
            "        THISFORM.grdCust.AllowCellSelection = .T.\n"
            "        THISFORM.grdCust.GridLines = 0\n"
            "        THISFORM.grdCust.Highlight = .T.\n"
            "        THISFORM.grdCust.HighlightRow = .T.\n"
            "        THISFORM.grdCust.RecordMark = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdCust AS Grid WITH AllowCellSelection = .F., GridLines = 2, Highlight = .F., HighlightRow = .F., RecordMark = .F.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS StyledGrid AS Grid\n"
            "    AllowCellSelection = .F.\n"
            "    GridLines = 2\n"
            "    Highlight = .F.\n"
            "    HighlightRow = .F.\n"
            "    RecordMark = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid display property script should complete: ") + state.message +
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

        check("lplainhasallowcellselection", "true");
        check("lplainallowcellselectionreadonly", "false");
        check("lplainhasgridlines", "true");
        check("lplaingridlinesreadonly", "false");
        check("lplainhashighlight", "true");
        check("lplainhighlightreadonly", "false");
        check("lplainhashighlightrow", "true");
        check("lplainhighlightrowreadonly", "false");
        check("lplainhasrecordmark", "true");
        check("lplainrecordmarkreadonly", "false");
        check("lplainallowbefore", "true");
        check("nplaingridlinesbefore", "3");
        check("lplainhighlightbefore", "true");
        check("lplainhighlightrowbefore", "true");
        check("lplainrecordmarkbefore", "true");
        check("xplainallowgetpembefore", "true");
        check("xplaingridlinesgetpembefore", "3");
        check("lplainallowafterdirectassign", "false");
        check("nplaingridlinesafterdirectassign", "2");
        check("lplainhighlightafterdirectassign", "false");
        check("lplainhighlightrowafterdirectassign", "false");
        check("lplainrecordmarkafterdirectassign", "false");
        check("lplainallowsetpem", "true");
        check("lplaingridlinessetpem", "true");
        check("lplainhighlightsetpem", "true");
        check("lplainhighlightrowsetpem", "true");
        check("lplainrecordmarksetpem", "true");
        check("lplainallowaftersetpem", "true");
        check("nplaingridlinesaftersetpem", "1");
        check("lplainhighlightaftersetpem", "true");
        check("lplainhighlightrowaftersetpem", "true");
        check("lplainrecordmarkaftersetpem", "true");
        check("lplainallowaddproperty", "false");
        check("lplainallowremoveproperty", "false");
        check("lplaingridlinesaddproperty", "false");
        check("lplaingridlinesremoveproperty", "false");
        check("lplainhighlightaddproperty", "false");
        check("lplainhighlightremoveproperty", "false");
        check("lplainhighlightrowaddproperty", "false");
        check("lplainhighlightrowremoveproperty", "false");
        check("lplainrecordmarkaddproperty", "false");
        check("lplainrecordmarkremoveproperty", "false");
        check("lgridallowbefore", "false");
        check("ngridgridlinesbefore", "2");
        check("lgridhighlightbefore", "false");
        check("lgridhighlightrowbefore", "false");
        check("lgridrecordmarkbefore", "false");
        check("lgridallowread", "false");
        check("ngridgridlinesread", "2");
        check("lgridhighlightread", "false");
        check("lgridhighlightrowread", "false");
        check("lgridrecordmarkread", "false");
        check("lgridallowafterchild", "true");
        check("ngridgridlinesafterchild", "0");
        check("lgridhighlightafterchild", "true");
        check("lgridhighlightrowafterchild", "true");
        check("lgridrecordmarkafterchild", "true");
        check("xgridallowgetpem", "true");
        check("xgridgridlinesgetpem", "0");
        check("lprophasallowcellselection", "true");
        check("lprophasgridlines", "true");
        check("lprophashighlight", "true");
        check("lprophashighlightrow", "true");
        check("lprophasrecordmark", "true");
        check("lderivedallowbefore", "false");
        check("nderivedgridlinesbefore", "2");
        check("lderivedhighlightbefore", "false");
        check("lderivedhighlightrowbefore", "false");
        check("lderivedrecordmarkbefore", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_deletemark_default_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_deletemark";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_deletemark.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Grid')\n"
            "lPlainHasDeleteMark = PEMSTATUS(oPlain, 'DeleteMark', 1)\n"
            "lPlainDeleteMarkReadOnly = PEMSTATUS(oPlain, 'DeleteMark', 5)\n"
            "lPlainBefore = oPlain.DeleteMark\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'DeleteMark')\n"
            "oPlain.DeleteMark = .F.\n"
            "lPlainAfterDirectAssign = oPlain.DeleteMark\n"
            "lPlainSetPem = SETPEM(oPlain, 'DeleteMark', .T.)\n"
            "lPlainAfterSetPem = oPlain.DeleteMark\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'DeleteMark', .F.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'DeleteMark')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lGridBefore = oForm.grdCust.DeleteMark\n"
            "lGridRead = oForm.cmdProbe.ReadDeleteMark()\n"
            "oForm.cmdProbe.RestyleGrid()\n"
            "lGridAfterChild = oForm.grdCust.DeleteMark\n"
            "lGridSetPem = SETPEM(oForm.grdCust, 'DeleteMark', .F.)\n"
            "lGridAfterSetPem = oForm.grdCust.DeleteMark\n"
            "xGridGetPem = GETPEM(oForm.grdCust, 'DeleteMark')\n"
            "lGridHasDeleteMark = PEMSTATUS(oForm.grdCust, 'DeleteMark', 1)\n"
            "lGridDeleteMarkReadOnly = PEMSTATUS(oForm.grdCust, 'DeleteMark', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.grdCust, 1)\n"
            "lPropHasDeleteMark = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DELETEMARK'\n"
            "        lPropHasDeleteMark = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('StyledGrid')\n"
            "lDerivedBefore = oDerived.DeleteMark\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadDeleteMark\n"
            "        RETURN THISFORM.grdCust.DeleteMark\n"
            "    ENDFUNC\n"
            "    PROCEDURE RestyleGrid\n"
            "        THISFORM.grdCust.DeleteMark = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT grdCust AS Grid WITH DeleteMark = .F.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS StyledGrid AS Grid\n"
            "    DeleteMark = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DeleteMark property script should complete: ") + state.message +
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

        check("lplainhasdeletemark", "true");
        check("lplaindeletemarkreadonly", "false");
        check("lplainbefore", "true");
        check("xplaingetpembefore", "true");
        check("lplainafterdirectassign", "false");
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lgridbefore", "false");
        check("lgridread", "false");
        check("lgridafterchild", "true");
        check("lgridsetpem", "true");
        check("lgridaftersetpem", "false");
        check("xgridgetpem", "false");
        check("lgridhasdeletemark", "true");
        check("lgriddeletemarkreadonly", "false");
        check("lprophasdeletemark", "true");
        check("lderivedbefore", "false");

        fs::remove_all(temp_root, ignored);
    }

}
