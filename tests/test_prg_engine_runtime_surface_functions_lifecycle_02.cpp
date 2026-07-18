#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_forecolor_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_forecolor";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_forecolor.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Form')\n"
            "lPlainHasForeColor = PEMSTATUS(oPlain, 'ForeColor', 1)\n"
            "lPlainForeColorReadOnly = PEMSTATUS(oPlain, 'ForeColor', 5)\n"
            "nPlainBefore = oPlain.ForeColor\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ForeColor')\n"
            "oPlain.ForeColor = 255\n"
            "nPlainAfterDirectAssign = oPlain.ForeColor\n"
            "lPlainSetPem = SETPEM(oPlain, 'ForeColor', 65280)\n"
            "nPlainAfterSetPem = oPlain.ForeColor\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ForeColor', 1)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ForeColor')\n"
            "oForm = CREATEOBJECT('PaintForm')\n"
            "nFormBefore = oForm.ForeColor\n"
            "nChildBefore = oForm.cmdSave.ForeColor\n"
            "nChildRead = oForm.cmdProbe.ReadForeColor()\n"
            "oForm.cmdProbe.PaintChild()\n"
            "nChildAfterChild = oForm.cmdSave.ForeColor\n"
            "nFormAfterChild = oForm.ForeColor\n"
            "lChildSetPem = SETPEM(oForm.cmdSave, 'ForeColor', 16776960)\n"
            "nChildAfterSetPem = oForm.cmdSave.ForeColor\n"
            "xChildGetPem = GETPEM(oForm.cmdSave, 'ForeColor')\n"
            "lChildHasForeColor = PEMSTATUS(oForm.cmdSave, 'ForeColor', 1)\n"
            "lChildForeColorReadOnly = PEMSTATUS(oForm.cmdSave, 'ForeColor', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cmdSave, 1)\n"
            "lPropHasForeColor = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'FORECOLOR'\n"
            "        lPropHasForeColor = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('PaintButton')\n"
            "nDerivedBefore = oDerived.ForeColor\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadForeColor\n"
            "        RETURN THISFORM.cmdSave.ForeColor\n"
            "    ENDFUNC\n"
            "    PROCEDURE PaintChild\n"
            "        THISFORM.cmdSave.ForeColor = 65535\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PaintForm AS Form\n"
            "    ForeColor = 16711935\n"
            "    ADD OBJECT cmdSave AS CommandButton WITH ForeColor = 16711680\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PaintButton AS CommandButton\n"
            "    ForeColor = 255\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual ForeColor property script should complete: ") + state.message +
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

        check("lplainhasforecolor", "true");
        check("lplainforecolorreadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
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
        check("lchildhasforecolor", "true");
        check("lchildforecolorreadonly", "false");
        check("lprophasforecolor", "true");
        check("nderivedbefore", "255");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_tabindex_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_tabindex";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_tabindex.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('CommandButton')\n"
            "lPlainHasTabIndex = PEMSTATUS(oPlain, 'TabIndex', 1)\n"
            "lPlainTabIndexReadOnly = PEMSTATUS(oPlain, 'TabIndex', 5)\n"
            "nPlainBefore = oPlain.TabIndex\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'TabIndex')\n"
            "oPlain.TabIndex = 4\n"
            "nPlainAfterDirectAssign = oPlain.TabIndex\n"
            "lPlainSetPem = SETPEM(oPlain, 'TabIndex', 7)\n"
            "nPlainAfterSetPem = oPlain.TabIndex\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'TabIndex', 1)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'TabIndex')\n"
            "oForm = CREATEOBJECT('TabForm')\n"
            "nCmdBefore = oForm.cmdOpen.TabIndex\n"
            "nTextBefore = oForm.txtName.TabIndex\n"
            "nContainerBefore = oForm.cntHost.TabIndex\n"
            "nNestedBefore = oForm.cntHost.cmdNested.TabIndex\n"
            "nNestedRead = oForm.cmdProbe.ReadNestedTabIndex()\n"
            "oForm.cmdProbe.MoveNestedTabIndex()\n"
            "nNestedAfterChild = oForm.cntHost.cmdNested.TabIndex\n"
            "lChildSetPem = SETPEM(oForm.txtName, 'TabIndex', 9)\n"
            "nTextAfterSetPem = oForm.txtName.TabIndex\n"
            "xChildGetPem = GETPEM(oForm.txtName, 'TabIndex')\n"
            "lChildHasTabIndex = PEMSTATUS(oForm.txtName, 'TabIndex', 1)\n"
            "lChildTabIndexReadOnly = PEMSTATUS(oForm.txtName, 'TabIndex', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.txtName, 1)\n"
            "lPropHasTabIndex = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'TABINDEX'\n"
            "        lPropHasTabIndex = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedButton')\n"
            "nDerivedBefore = oDerived.TabIndex\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadNestedTabIndex\n"
            "        RETURN THISFORM.cntHost.cmdNested.TabIndex\n"
            "    ENDFUNC\n"
            "    PROCEDURE MoveNestedTabIndex\n"
            "        THISFORM.cntHost.cmdNested.TabIndex = 5\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabForm AS Form\n"
            "    ADD OBJECT cmdOpen AS CommandButton\n"
            "    ADD OBJECT txtName AS TextBox\n"
            "    ADD OBJECT cntHost AS HostContainer\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HostContainer AS Container\n"
            "    ADD OBJECT cmdNested AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedButton AS CommandButton\n"
            "    TabIndex = 11\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TabIndex property script should complete: ") + state.message +
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

        check("lplainhastabindex", "true");
        check("lplaintabindexreadonly", "false");
        check("nplainbefore", "0");
        check("xplaingetpembefore", "0");
        check("nplainafterdirectassign", "4");
        check("lplainsetpem", "true");
        check("nplainaftersetpem", "7");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ncmdbefore", "0");
        check("ntextbefore", "1");
        check("ncontainerbefore", "2");
        check("nnestedbefore", "0");
        check("nnestedread", "0");
        check("nnestedafterchild", "5");
        check("lchildsetpem", "true");
        check("ntextaftersetpem", "9");
        check("xchildgetpem", "9");
        check("lchildhastabindex", "true");
        check("lchildtabindexreadonly", "false");
        check("lprophastabindex", "true");
        check("nderivedbefore", "11");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_tabstop_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_tabstop";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_tabstop.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('CommandButton')\n"
            "lPlainHasTabStop = PEMSTATUS(oPlain, 'TabStop', 1)\n"
            "lPlainTabStopReadOnly = PEMSTATUS(oPlain, 'TabStop', 5)\n"
            "lPlainBefore = oPlain.TabStop\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'TabStop')\n"
            "oPlain.TabStop = .F.\n"
            "lPlainAfterDirectAssign = oPlain.TabStop\n"
            "lPlainSetPem = SETPEM(oPlain, 'TabStop', .T.)\n"
            "lPlainAfterSetPem = oPlain.TabStop\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'TabStop', .F.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'TabStop')\n"
            "oForm = CREATEOBJECT('TabStopForm')\n"
            "lTextBefore = oForm.txtName.TabStop\n"
            "lContainerBefore = oForm.cntHost.TabStop\n"
            "lNestedBefore = oForm.cntHost.cmdNested.TabStop\n"
            "lNestedRead = oForm.cmdProbe.ReadNestedTabStop()\n"
            "oForm.cmdProbe.DisableNestedTabStop()\n"
            "lNestedAfterChild = oForm.cntHost.cmdNested.TabStop\n"
            "lChildSetPem = SETPEM(oForm.txtName, 'TabStop', .F.)\n"
            "lTextAfterSetPem = oForm.txtName.TabStop\n"
            "xChildGetPem = GETPEM(oForm.txtName, 'TabStop')\n"
            "lChildHasTabStop = PEMSTATUS(oForm.txtName, 'TabStop', 1)\n"
            "lChildTabStopReadOnly = PEMSTATUS(oForm.txtName, 'TabStop', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.txtName, 1)\n"
            "lPropHasTabStop = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'TABSTOP'\n"
            "        lPropHasTabStop = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedButton')\n"
            "lDerivedBefore = oDerived.TabStop\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadNestedTabStop\n"
            "        RETURN THISFORM.cntHost.cmdNested.TabStop\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableNestedTabStop\n"
            "        THISFORM.cntHost.cmdNested.TabStop = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TabStopForm AS Form\n"
            "    ADD OBJECT txtName AS TextBox\n"
            "    ADD OBJECT cntHost AS HostContainer\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HostContainer AS Container\n"
            "    ADD OBJECT cmdNested AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedButton AS CommandButton\n"
            "    TabStop = .F.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TabStop property script should complete: ") + state.message +
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

        check("lplainhastabstop", "true");
        check("lplaintabstopreadonly", "false");
        check("lplainbefore", "true");
        check("xplaingetpembefore", "true");
        check("lplainafterdirectassign", "false");
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ltextbefore", "true");
        check("lcontainerbefore", "true");
        check("lnestedbefore", "true");
        check("lnestedread", "true");
        check("lnestedafterchild", "false");
        check("lchildsetpem", "true");
        check("ltextaftersetpem", "false");
        check("xchildgetpem", "false");
        check("lchildhastabstop", "true");
        check("lchildtabstopreadonly", "false");
        check("lprophastabstop", "true");
        check("lderivedbefore", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_text_entry_readonly_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_text_entry_readonly";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_text_entry_readonly.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('TextBox')\n"
            "lPlainHasReadOnly = PEMSTATUS(oPlain, 'ReadOnly', 1)\n"
            "lPlainReadOnlyReadOnly = PEMSTATUS(oPlain, 'ReadOnly', 5)\n"
            "lPlainBefore = oPlain.ReadOnly\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'ReadOnly')\n"
            "oPlain.ReadOnly = .T.\n"
            "lPlainAfterDirectAssign = oPlain.ReadOnly\n"
            "lPlainSetPem = SETPEM(oPlain, 'ReadOnly', .F.)\n"
            "lPlainAfterSetPem = oPlain.ReadOnly\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'ReadOnly', .T.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'ReadOnly')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lTextBefore = oForm.txtName.ReadOnly\n"
            "lEditBefore = oForm.edtNotes.ReadOnly\n"
            "lChildRead = oForm.cmdProbe.ReadTextReadOnly()\n"
            "oForm.cmdProbe.MakeEditable()\n"
            "lTextAfterChild = oForm.txtName.ReadOnly\n"
            "lEditAfterChild = oForm.edtNotes.ReadOnly\n"
            "lChildSetPem = SETPEM(oForm.edtNotes, 'ReadOnly', .T.)\n"
            "lEditAfterSetPem = oForm.edtNotes.ReadOnly\n"
            "xChildGetPem = GETPEM(oForm.txtName, 'ReadOnly')\n"
            "lChildHasReadOnly = PEMSTATUS(oForm.edtNotes, 'ReadOnly', 1)\n"
            "lChildReadOnlyReadOnly = PEMSTATUS(oForm.edtNotes, 'ReadOnly', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.edtNotes, 1)\n"
            "lPropHasReadOnly = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'READONLY'\n"
            "        lPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS ProbeButton AS CommandButton\n"
            "    FUNCTION ReadTextReadOnly\n"
            "        RETURN THISFORM.txtName.ReadOnly\n"
            "    ENDFUNC\n"
            "    PROCEDURE MakeEditable\n"
            "        THISFORM.txtName.ReadOnly = .F.\n"
            "        THISFORM.edtNotes.ReadOnly = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT txtName AS TextBox WITH ReadOnly = .T.\n"
            "    ADD OBJECT edtNotes AS EditBox WITH ReadOnly = .T.\n"
            "    ADD OBJECT cmdProbe AS ProbeButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native text-entry ReadOnly property script should complete: ") + state.message +
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
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "false");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("ltextbefore", "true");
        check("leditbefore", "true");
        check("lchildread", "true");
        check("ltextafterchild", "false");
        check("leditafterchild", "false");
        check("lchildsetpem", "true");
        check("leditaftersetpem", "true");
        check("xchildgetpem", "false");
        check("lchildhasreadonly", "true");
        check("lchildreadonlyreadonly", "false");
        check("lprophasreadonly", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_grid_and_column_readonly_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_column_readonly";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_column_readonly.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "lGridHasReadOnly = PEMSTATUS(oGrid, 'ReadOnly', 1)\n"
            "lGridReadOnlyReadOnly = PEMSTATUS(oGrid, 'ReadOnly', 5)\n"
            "lGridBefore = oGrid.ReadOnly\n"
            "xGridGetPemBefore = GETPEM(oGrid, 'ReadOnly')\n"
            "oGrid.ReadOnly = .T.\n"
            "lGridAfterDirectAssign = oGrid.ReadOnly\n"
            "lGridSetPem = SETPEM(oGrid, 'ReadOnly', .F.)\n"
            "lGridAfterSetPem = oGrid.ReadOnly\n"
            "lGridAddProperty = ADDPROPERTY(oGrid, 'ReadOnly', .T.)\n"
            "lGridRemoveProperty = REMOVEPROPERTY(oGrid, 'ReadOnly')\n"
            "nGridPropMembers = AMEMBERS(aGridPropMembers, oGrid, 1)\n"
            "lGridPropHasReadOnly = .F.\n"
            "FOR i = 1 TO nGridPropMembers\n"
            "    IF UPPER(aGridPropMembers[i]) == 'READONLY'\n"
            "        lGridPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerivedGrid = CREATEOBJECT('WorkerGrid')\n"
            "lDerivedGridBefore = oDerivedGrid.ReadOnly\n"
            "oDerivedGrid.ReadOnly = .F.\n"
            "lDerivedGridAfterDirectAssign = oDerivedGrid.ReadOnly\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lColumnHasReadOnly = PEMSTATUS(oColumn, 'ReadOnly', 1)\n"
            "lColumnReadOnlyReadOnly = PEMSTATUS(oColumn, 'ReadOnly', 5)\n"
            "lColumnBefore = oColumn.ReadOnly\n"
            "xColumnGetPemBefore = GETPEM(oColumn, 'ReadOnly')\n"
            "oColumn.ReadOnly = .T.\n"
            "lColumnAfterDirectAssign = oColumn.ReadOnly\n"
            "lColumnSetPem = SETPEM(oColumn, 'ReadOnly', .F.)\n"
            "lColumnAfterSetPem = oColumn.ReadOnly\n"
            "lColumnAddProperty = ADDPROPERTY(oColumn, 'ReadOnly', .T.)\n"
            "lColumnRemoveProperty = REMOVEPROPERTY(oColumn, 'ReadOnly')\n"
            "nColumnPropMembers = AMEMBERS(aColumnPropMembers, oColumn, 1)\n"
            "lColumnPropHasReadOnly = .F.\n"
            "FOR j = 1 TO nColumnPropMembers\n"
            "    IF UPPER(aColumnPropMembers[j]) == 'READONLY'\n"
            "        lColumnPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerivedColumn = CREATEOBJECT('WorkerColumn')\n"
            "lDerivedColumnBefore = oDerivedColumn.ReadOnly\n"
            "lDerivedColumnSetPem = SETPEM(oDerivedColumn, 'ReadOnly', .F.)\n"
            "lDerivedColumnAfterSetPem = oDerivedColumn.ReadOnly\n"
            "RETURN\n"
            "DEFINE CLASS WorkerGrid AS Grid\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS WorkerColumn AS Column\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid/Column ReadOnly property script should complete: ") + state.message +
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

        check("lgridhasreadonly", "true");
        check("lgridreadonlyreadonly", "false");
        check("lgridbefore", "false");
        check("xgridgetpembefore", "false");
        check("lgridafterdirectassign", "true");
        check("lgridsetpem", "true");
        check("lgridaftersetpem", "false");
        check("lgridaddproperty", "false");
        check("lgridremoveproperty", "false");
        check("lgridprophasreadonly", "true");
        check("lderivedgridbefore", "true");
        check("lderivedgridafterdirectassign", "false");
        check("lcolumnhasreadonly", "true");
        check("lcolumnreadonlyreadonly", "false");
        check("lcolumnbefore", "false");
        check("xcolumngetpembefore", "false");
        check("lcolumnafterdirectassign", "true");
        check("lcolumnsetpem", "true");
        check("lcolumnaftersetpem", "false");
        check("lcolumnaddproperty", "false");
        check("lcolumnremoveproperty", "false");
        check("lcolumnprophasreadonly", "true");
        check("lderivedcolumnbefore", "true");
        check("lderivedcolumnsetpem", "true");
        check("lderivedcolumnaftersetpem", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_checkbox_and_spinner_readonly_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_checkbox_spinner_readonly";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_checkbox_spinner_readonly.prg";
        write_text(
            main_path,
            "oCheck = CREATEOBJECT('CheckBox')\n"
            "lCheckHasReadOnly = PEMSTATUS(oCheck, 'ReadOnly', 1)\n"
            "lCheckReadOnlyReadOnly = PEMSTATUS(oCheck, 'ReadOnly', 5)\n"
            "lCheckBefore = oCheck.ReadOnly\n"
            "xCheckGetPemBefore = GETPEM(oCheck, 'ReadOnly')\n"
            "oCheck.ReadOnly = .T.\n"
            "lCheckAfterDirectAssign = oCheck.ReadOnly\n"
            "lCheckSetPem = SETPEM(oCheck, 'ReadOnly', .F.)\n"
            "lCheckAfterSetPem = oCheck.ReadOnly\n"
            "lCheckAddProperty = ADDPROPERTY(oCheck, 'ReadOnly', .T.)\n"
            "lCheckRemoveProperty = REMOVEPROPERTY(oCheck, 'ReadOnly')\n"
            "nCheckPropMembers = AMEMBERS(aCheckPropMembers, oCheck, 1)\n"
            "lCheckPropHasReadOnly = .F.\n"
            "FOR i = 1 TO nCheckPropMembers\n"
            "    IF UPPER(aCheckPropMembers[i]) == 'READONLY'\n"
            "        lCheckPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerivedCheck = CREATEOBJECT('WorkerCheck')\n"
            "lDerivedCheckBefore = oDerivedCheck.ReadOnly\n"
            "oDerivedCheck.ReadOnly = .F.\n"
            "lDerivedCheckAfterDirectAssign = oDerivedCheck.ReadOnly\n"
            "oSpinner = CREATEOBJECT('Spinner')\n"
            "lSpinnerHasReadOnly = PEMSTATUS(oSpinner, 'ReadOnly', 1)\n"
            "lSpinnerReadOnlyReadOnly = PEMSTATUS(oSpinner, 'ReadOnly', 5)\n"
            "lSpinnerBefore = oSpinner.ReadOnly\n"
            "xSpinnerGetPemBefore = GETPEM(oSpinner, 'ReadOnly')\n"
            "oSpinner.ReadOnly = .T.\n"
            "lSpinnerAfterDirectAssign = oSpinner.ReadOnly\n"
            "lSpinnerSetPem = SETPEM(oSpinner, 'ReadOnly', .F.)\n"
            "lSpinnerAfterSetPem = oSpinner.ReadOnly\n"
            "lSpinnerAddProperty = ADDPROPERTY(oSpinner, 'ReadOnly', .T.)\n"
            "lSpinnerRemoveProperty = REMOVEPROPERTY(oSpinner, 'ReadOnly')\n"
            "nSpinnerPropMembers = AMEMBERS(aSpinnerPropMembers, oSpinner, 1)\n"
            "lSpinnerPropHasReadOnly = .F.\n"
            "FOR j = 1 TO nSpinnerPropMembers\n"
            "    IF UPPER(aSpinnerPropMembers[j]) == 'READONLY'\n"
            "        lSpinnerPropHasReadOnly = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerivedSpinner = CREATEOBJECT('WorkerSpinner')\n"
            "lDerivedSpinnerBefore = oDerivedSpinner.ReadOnly\n"
            "lDerivedSpinnerSetPem = SETPEM(oDerivedSpinner, 'ReadOnly', .F.)\n"
            "lDerivedSpinnerAfterSetPem = oDerivedSpinner.ReadOnly\n"
            "RETURN\n"
            "DEFINE CLASS WorkerCheck AS CheckBox\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS WorkerSpinner AS Spinner\n"
            "    ReadOnly = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CheckBox/Spinner ReadOnly property script should complete: ") + state.message +
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

        check("lcheckhasreadonly", "true");
        check("lcheckreadonlyreadonly", "false");
        check("lcheckbefore", "false");
        check("xcheckgetpembefore", "false");
        check("lcheckafterdirectassign", "true");
        check("lchecksetpem", "true");
        check("lcheckaftersetpem", "false");
        check("lcheckaddproperty", "false");
        check("lcheckremoveproperty", "false");
        check("lcheckprophasreadonly", "true");
        check("lderivedcheckbefore", "true");
        check("lderivedcheckafterdirectassign", "false");
        check("lspinnerhasreadonly", "true");
        check("lspinnerreadonlyreadonly", "false");
        check("lspinnerbefore", "false");
        check("xspinnergetpembefore", "false");
        check("lspinnerafterdirectassign", "true");
        check("lspinnersetpem", "true");
        check("lspinneraftersetpem", "false");
        check("lspinneraddproperty", "false");
        check("lspinnerremoveproperty", "false");
        check("lspinnerprophasreadonly", "true");
        check("lderivedspinnerbefore", "true");
        check("lderivedspinnersetpem", "true");
        check("lderivedspinneraftersetpem", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_name_reflects_parent_chain_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_name_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_name_property.prg";
        write_text(
            main_path,
            "oSet = CREATEOBJECT('MainFormSet')\n"
            "cSetName = oSet.Name\n"
            "cFormName = oSet.frmWork.Name\n"
            "cFormParentName = oSet.frmWork.Parent.Name\n"
            "cButtonName = oSet.frmWork.cmdSave.Name\n"
            "cButtonParentName = oSet.frmWork.cmdSave.Parent.Name\n"
            "lSetHasName = PEMSTATUS(oSet, 'Name', 1)\n"
            "lSetNameReadOnly = PEMSTATUS(oSet, 'Name', 5)\n"
            "xSetGetPem = GETPEM(oSet, 'Name')\n"
            "lSetPemName = SETPEM(oSet, 'Name', 'RenamedSet')\n"
            "lAddSetName = ADDPROPERTY(oSet, 'Name', 'ShadowSet')\n"
            "lRemoveSetName = REMOVEPROPERTY(oSet, 'Name')\n"
            "oHost = CREATEOBJECT('HostForm')\n"
            "lAddOk = oHost.AddObject('txtAdded', 'TextBox')\n"
            "cHostName = oHost.Name\n"
            "cAddedName = oHost.txtAdded.Name\n"
            "cAddedParentName = oHost.txtAdded.Parent.Name\n"
            "xAddedNameGetPem = GETPEM(oHost.txtAdded, 'Name')\n"
            "lAddedNameReadOnly = PEMSTATUS(oHost.txtAdded, 'Name', 5)\n"
            "RETURN\n"
            "DEFINE CLASS WorkerForm AS Form\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainFormSet AS FormSet\n"
            "    ADD OBJECT frmWork AS WorkerForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS HostForm AS Form\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Name property script should complete: ") + state.message +
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

        check("csetname", "MainFormSet");
        check("cformname", "frmWork");
        check("cformparentname", "MainFormSet");
        check("cbuttonname", "cmdSave");
        check("cbuttonparentname", "frmWork");
        check("lsethasname", "true");
        check("lsetnamereadonly", "true");
        check("xsetgetpem", "MainFormSet");
        check("lsetpemname", "false");
        check("laddsetname", "false");
        check("lremovesetname", "false");
        check("laddok", "true");
        check("chostname", "HostForm");
        check("caddedname", "txtAdded");
        check("caddedparentname", "HostForm");
        check("xaddednamegetpem", "txtAdded");
        check("laddednamereadonly", "true");

        fs::remove_all(temp_root, ignored);
    }

}
