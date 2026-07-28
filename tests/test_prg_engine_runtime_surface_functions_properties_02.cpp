#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_desktop_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_desktop";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_desktop.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasDesktop = PEMSTATUS(oBaseForm, 'Desktop', 1)\n"
            "lBaseDesktopReadOnly = PEMSTATUS(oBaseForm, 'Desktop', 5)\n"
            "lBaseBefore = oBaseForm.Desktop\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'Desktop')\n"
            "oBaseForm.Desktop = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.Desktop\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'Desktop', .T.)\n"
            "lBaseAfterSetPem = oBaseForm.Desktop\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'Desktop', .T.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'Desktop')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.Desktop\n"
            "lChildBefore = oDerived.cmdSave.ReadDesktop()\n"
            "oDerived.cmdSave.TryUndock()\n"
            "lDerivedAfterChild = oDerived.Desktop\n"
            "lDerivedStillDesktop = oDerived.Desktop\n"
            "xDerivedGetPem = GETPEM(oDerived, 'Desktop')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasDesktop = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'DESKTOP'\n"
            "        lPropHasDesktop = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadDesktop\n"
            "        RETURN THISFORM.Desktop\n"
            "    ENDFUNC\n"
            "    PROCEDURE TryUndock\n"
            "        THISFORM.Desktop = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    Desktop = .T.\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form Desktop property script should complete: ") + state.message +
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

        check("lbasehasdesktop", "true");
        check("lbasedesktopreadonly", "true");
        check("lbasebefore", "false");
        check("xbasegetpembefore", "false");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "false");
        check("lbaseaftersetpem", "false");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("lchildbefore", "true");
        check("lderivedafterchild", "true");
        check("lderivedstilldesktop", "true");
        check("xderivedgetpem", "true");
        check("lprophasdesktop", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_show_in_taskbar_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_show_in_taskbar";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_show_in_taskbar.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasShowInTaskbar = PEMSTATUS(oBaseForm, 'ShowInTaskbar', 1)\n"
            "lBaseShowInTaskbarReadOnly = PEMSTATUS(oBaseForm, 'ShowInTaskbar', 5)\n"
            "lBaseBefore = oBaseForm.ShowInTaskbar\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ShowInTaskbar')\n"
            "oBaseForm.ShowInTaskbar = .F.\n"
            "lBaseAfterDirectAssign = oBaseForm.ShowInTaskbar\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ShowInTaskbar', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.ShowInTaskbar\n"
            "lBasePutPem = PUTPEM(oBaseForm, 'ShowInTaskbar', .F.)\n"
            "lBaseAfterPutPem = oBaseForm.ShowInTaskbar\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ShowInTaskbar', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ShowInTaskbar')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.ShowInTaskbar\n"
            "lChildBefore = oDerived.cmdSave.ReadShowInTaskbar()\n"
            "oDerived.cmdSave.DisableTaskbar()\n"
            "lDerivedAfterChild = oDerived.ShowInTaskbar\n"
            "xDerivedGetPem = GETPEM(oDerived, 'ShowInTaskbar')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasShowInTaskbar = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SHOWINTASKBAR'\n"
            "        lPropHasShowInTaskbar = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadShowInTaskbar\n"
            "        RETURN THISFORM.ShowInTaskbar\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableTaskbar\n"
            "        THISFORM.ShowInTaskbar = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ShowInTaskbar = .F.\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ShowInTaskbar property script should complete: ") + state.message +
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

        check("lbasehasshowintaskbar", "true");
        check("lbaseshowintaskbarreadonly", "true");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "true");
        check("lbasesetpem", "false");
        check("lbaseaftersetpem", "true");
        check("lbaseputpem", "false");
        check("lbaseafterputpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "false");
        check("lchildbefore", "false");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophasshowintaskbar", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_controlbox_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_controlbox";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_controlbox.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasControlBox = PEMSTATUS(oBaseForm, 'ControlBox', 1)\n"
            "lBaseControlBoxReadOnly = PEMSTATUS(oBaseForm, 'ControlBox', 5)\n"
            "lBaseBefore = oBaseForm.ControlBox\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ControlBox')\n"
            "oBaseForm.ControlBox = .F.\n"
            "lBaseAfterDirectAssign = oBaseForm.ControlBox\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ControlBox', .T.)\n"
            "lBaseAfterSetPem = oBaseForm.ControlBox\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ControlBox', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ControlBox')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.ControlBox\n"
            "cChildBefore = oDerived.cmdSave.ReadControlBox()\n"
            "oDerived.cmdSave.DisableControlBox()\n"
            "lDerivedAfterChild = oDerived.ControlBox\n"
            "xDerivedGetPem = GETPEM(oDerived, 'ControlBox')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasControlBox = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CONTROLBOX'\n"
            "        lPropHasControlBox = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadControlBox\n"
            "        RETURN IIF(THISFORM.ControlBox, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableControlBox\n"
            "        THISFORM.ControlBox = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ControlBox property script should complete: ") + state.message +
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

        check("lbasehascontrolbox", "true");
        check("lbasecontrolboxreadonly", "false");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("cchildbefore", "T");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophascontrolbox", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_closable_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_closable";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_closable.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasClosable = PEMSTATUS(oBaseForm, 'Closable', 1)\n"
            "lBaseClosableReadOnly = PEMSTATUS(oBaseForm, 'Closable', 5)\n"
            "lBaseBefore = oBaseForm.Closable\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'Closable')\n"
            "oBaseForm.Closable = .F.\n"
            "lBaseAfterDirectAssign = oBaseForm.Closable\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'Closable', .T.)\n"
            "lBaseAfterSetPem = oBaseForm.Closable\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'Closable', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'Closable')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.Closable\n"
            "cChildBefore = oDerived.cmdSave.ReadClosable()\n"
            "oDerived.cmdSave.DisableClose()\n"
            "lDerivedAfterChild = oDerived.Closable\n"
            "xDerivedGetPem = GETPEM(oDerived, 'Closable')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasClosable = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CLOSABLE'\n"
            "        lPropHasClosable = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadClosable\n"
            "        RETURN IIF(THISFORM.Closable, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableClose\n"
            "        THISFORM.Closable = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form Closable property script should complete: ") + state.message +
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

        check("lbasehasclosable", "true");
        check("lbaseclosablereadonly", "false");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("cchildbefore", "T");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophasclosable", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_minbutton_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_minbutton";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_minbutton.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasMinButton = PEMSTATUS(oBaseForm, 'MinButton', 1)\n"
            "lBaseMinButtonReadOnly = PEMSTATUS(oBaseForm, 'MinButton', 5)\n"
            "lBaseBefore = oBaseForm.MinButton\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'MinButton')\n"
            "oBaseForm.MinButton = .F.\n"
            "lBaseAfterDirectAssign = oBaseForm.MinButton\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'MinButton', .T.)\n"
            "lBaseAfterSetPem = oBaseForm.MinButton\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'MinButton', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'MinButton')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.MinButton\n"
            "cChildBefore = oDerived.cmdSave.ReadMinButton()\n"
            "oDerived.cmdSave.DisableMinimize()\n"
            "lDerivedAfterChild = oDerived.MinButton\n"
            "xDerivedGetPem = GETPEM(oDerived, 'MinButton')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasMinButton = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MINBUTTON'\n"
            "        lPropHasMinButton = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadMinButton\n"
            "        RETURN IIF(THISFORM.MinButton, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableMinimize\n"
            "        THISFORM.MinButton = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form MinButton property script should complete: ") + state.message +
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

        check("lbasehasminbutton", "true");
        check("lbaseminbuttonreadonly", "false");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("cchildbefore", "T");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophasminbutton", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_maxbutton_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_maxbutton";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_maxbutton.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasMaxButton = PEMSTATUS(oBaseForm, 'MaxButton', 1)\n"
            "lBaseMaxButtonReadOnly = PEMSTATUS(oBaseForm, 'MaxButton', 5)\n"
            "lBaseBefore = oBaseForm.MaxButton\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'MaxButton')\n"
            "oBaseForm.MaxButton = .F.\n"
            "lBaseAfterDirectAssign = oBaseForm.MaxButton\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'MaxButton', .T.)\n"
            "lBaseAfterSetPem = oBaseForm.MaxButton\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'MaxButton', .F.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'MaxButton')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.MaxButton\n"
            "cChildBefore = oDerived.cmdSave.ReadMaxButton()\n"
            "oDerived.cmdSave.DisableMaximize()\n"
            "lDerivedAfterChild = oDerived.MaxButton\n"
            "xDerivedGetPem = GETPEM(oDerived, 'MaxButton')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasMaxButton = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'MAXBUTTON'\n"
            "        lPropHasMaxButton = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadMaxButton\n"
            "        RETURN IIF(THISFORM.MaxButton, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE DisableMaximize\n"
            "        THISFORM.MaxButton = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form MaxButton property script should complete: ") + state.message +
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

        check("lbasehasmaxbutton", "true");
        check("lbasemaxbuttonreadonly", "false");
        check("lbasebefore", "true");
        check("xbasegetpembefore", "true");
        check("lbaseafterdirectassign", "false");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "true");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "true");
        check("cchildbefore", "T");
        check("lderivedafterchild", "false");
        check("xderivedgetpem", "false");
        check("lprophasmaxbutton", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_autocenter_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_autocenter";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_autocenter.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasAutoCenter = PEMSTATUS(oBaseForm, 'AutoCenter', 1)\n"
            "lBaseAutoCenterReadOnly = PEMSTATUS(oBaseForm, 'AutoCenter', 5)\n"
            "lBaseBefore = oBaseForm.AutoCenter\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'AutoCenter')\n"
            "oBaseForm.AutoCenter = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.AutoCenter\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'AutoCenter', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.AutoCenter\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'AutoCenter', .T.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'AutoCenter')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.AutoCenter\n"
            "cChildBefore = oDerived.cmdSave.ReadAutoCenter()\n"
            "oDerived.cmdSave.EnableCentering()\n"
            "lDerivedAfterChild = oDerived.AutoCenter\n"
            "xDerivedGetPem = GETPEM(oDerived, 'AutoCenter')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasAutoCenter = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'AUTOCENTER'\n"
            "        lPropHasAutoCenter = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadAutoCenter\n"
            "        RETURN IIF(THISFORM.AutoCenter, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE EnableCentering\n"
            "        THISFORM.AutoCenter = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form AutoCenter property script should complete: ") + state.message +
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

        check("lbasehasautocenter", "true");
        check("lbaseautocenterreadonly", "false");
        check("lbasebefore", "false");
        check("xbasegetpembefore", "false");
        check("lbaseafterdirectassign", "true");
        check("lbasesetpem", "true");
        check("lbaseaftersetpem", "false");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("lderivedbefore", "false");
        check("cchildbefore", "F");
        check("lderivedafterchild", "true");
        check("xderivedgetpem", "true");
        check("lprophasautocenter", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_enabled_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_enabled";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_enabled.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('CommandButton')\n"
            "lPlainHasEnabled = PEMSTATUS(oPlain, 'Enabled', 1)\n"
            "lPlainEnabledReadOnly = PEMSTATUS(oPlain, 'Enabled', 5)\n"
            "lPlainBefore = oPlain.Enabled\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'Enabled')\n"
            "oPlain.Enabled = .F.\n"
            "lPlainAfterDirectAssign = oPlain.Enabled\n"
            "lPlainSetPem = SETPEM(oPlain, 'Enabled', .T.)\n"
            "lPlainAfterSetPem = oPlain.Enabled\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'Enabled', .F.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'Enabled')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lChildBefore = oForm.oToolbar.cmdRed.Enabled\n"
            "oForm.oToolbar.cmdRed.Enabled = .F.\n"
            "lChildAfterDirectAssign = oForm.oToolbar.cmdRed.Enabled\n"
            "lChildSetPem = SETPEM(oForm.oToolbar.cmdRed, 'Enabled', .T.)\n"
            "lChildAfterSetPem = oForm.oToolbar.cmdRed.Enabled\n"
            "xChildGetPem = GETPEM(oForm.oToolbar.cmdRed, 'Enabled')\n"
            "lChildHasEnabled = PEMSTATUS(oForm.oToolbar.cmdRed, 'Enabled', 1)\n"
            "lChildEnabledReadOnly = PEMSTATUS(oForm.oToolbar.cmdRed, 'Enabled', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.oToolbar.cmdRed, 1)\n"
            "lPropHasEnabled = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ENABLED'\n"
            "        lPropHasEnabled = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS MainToolbar AS Toolbar\n"
            "    ADD OBJECT cmdRed AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT oToolbar AS MainToolbar\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual Enabled property script should complete: ") + state.message +
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

        check("lplainhasenabled", "true");
        check("lplainenabledreadonly", "false");
        check("lplainbefore", "true");
        check("xplaingetpembefore", "true");
        check("lplainafterdirectassign", "false");
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lchildbefore", "true");
        check("lchildafterdirectassign", "false");
        check("lchildsetpem", "true");
        check("lchildaftersetpem", "true");
        check("xchildgetpem", "true");
        check("lchildhasenabled", "true");
        check("lchildenabledreadonly", "false");
        check("lprophasenabled", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_visible_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_visible";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_visible.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('CommandButton')\n"
            "lPlainHasVisible = PEMSTATUS(oPlain, 'Visible', 1)\n"
            "lPlainVisibleReadOnly = PEMSTATUS(oPlain, 'Visible', 5)\n"
            "lPlainBefore = oPlain.Visible\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'Visible')\n"
            "oPlain.Visible = .F.\n"
            "lPlainAfterDirectAssign = oPlain.Visible\n"
            "lPlainSetPem = SETPEM(oPlain, 'Visible', .T.)\n"
            "lPlainAfterSetPem = oPlain.Visible\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'Visible', .F.)\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'Visible')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lChildBefore = oForm.oToolbar.cmdRed.Visible\n"
            "oForm.oToolbar.cmdRed.Visible = .F.\n"
            "lChildAfterDirectAssign = oForm.oToolbar.cmdRed.Visible\n"
            "lChildSetPem = SETPEM(oForm.oToolbar.cmdRed, 'Visible', .T.)\n"
            "lChildAfterSetPem = oForm.oToolbar.cmdRed.Visible\n"
            "xChildGetPem = GETPEM(oForm.oToolbar.cmdRed, 'Visible')\n"
            "lChildHasVisible = PEMSTATUS(oForm.oToolbar.cmdRed, 'Visible', 1)\n"
            "lChildVisibleReadOnly = PEMSTATUS(oForm.oToolbar.cmdRed, 'Visible', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.oToolbar.cmdRed, 1)\n"
            "lPropHasVisible = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'VISIBLE'\n"
            "        lPropHasVisible = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS MainToolbar AS Toolbar\n"
            "    ADD OBJECT cmdRed AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT oToolbar AS MainToolbar\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual Visible property script should complete: ") + state.message +
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

        check("lplainhasvisible", "true");
        check("lplainvisiblereadonly", "false");
        check("lplainbefore", "true");
        check("xplaingetpembefore", "true");
        check("lplainafterdirectassign", "false");
        check("lplainsetpem", "true");
        check("lplainaftersetpem", "true");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("lchildbefore", "true");
        check("lchildafterdirectassign", "false");
        check("lchildsetpem", "true");
        check("lchildaftersetpem", "true");
        check("xchildgetpem", "true");
        check("lchildhasvisible", "true");
        check("lchildvisiblereadonly", "false");
        check("lprophasvisible", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_string_control_value_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_string_control_value";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_string_control_value.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('TextBox')\n"
            "lPlainHasValue = PEMSTATUS(oPlain, 'Value', 1)\n"
            "lPlainValueReadOnly = PEMSTATUS(oPlain, 'Value', 5)\n"
            "cPlainBefore = oPlain.Value\n"
            "xPlainGetPemBefore = GETPEM(oPlain, 'Value')\n"
            "oPlain.Value = 'alpha'\n"
            "cPlainAfterDirectAssign = oPlain.Value\n"
            "lPlainSetPem = SETPEM(oPlain, 'Value', 'beta')\n"
            "cPlainAfterSetPem = oPlain.Value\n"
            "lPlainAddProperty = ADDPROPERTY(oPlain, 'Value', 'shadow')\n"
            "lPlainRemoveProperty = REMOVEPROPERTY(oPlain, 'Value')\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "cChildBefore = oForm.cboMonth.Value\n"
            "oForm.cboMonth.Value = '07'\n"
            "cChildAfterDirectAssign = oForm.cboMonth.Value\n"
            "lChildSetPem = SETPEM(oForm.cboMonth, 'Value', '12')\n"
            "cChildAfterSetPem = oForm.cboMonth.Value\n"
            "xChildGetPem = GETPEM(oForm.cboMonth, 'Value')\n"
            "lChildHasValue = PEMSTATUS(oForm.cboMonth, 'Value', 1)\n"
            "lChildValueReadOnly = PEMSTATUS(oForm.cboMonth, 'Value', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm.cboMonth, 1)\n"
            "lPropHasValue = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'VALUE'\n"
            "        lPropHasValue = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cboMonth AS ComboBox\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native string-control Value property script should complete: ") + state.message +
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

        check("lplainhasvalue", "true");
        check("lplainvaluereadonly", "false");
        check("cplainbefore", "");
        check("xplaingetpembefore", "");
        check("cplainafterdirectassign", "alpha");
        check("lplainsetpem", "true");
        check("cplainaftersetpem", "beta");
        check("lplainaddproperty", "false");
        check("lplainremoveproperty", "false");
        check("cchildbefore", "");
        check("cchildafterdirectassign", "07");
        check("lchildsetpem", "true");
        check("cchildaftersetpem", "12");
        check("xchildgetpem", "12");
        check("lchildhasvalue", "true");
        check("lchildvaluereadonly", "false");
        check("lprophasvalue", "true");

        fs::remove_all(temp_root, ignored);
    }

}
