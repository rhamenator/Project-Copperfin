#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_lockscreen_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_lockscreen";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_lockscreen.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasLockScreen = PEMSTATUS(oBaseForm, 'LockScreen', 1)\n"
            "lBaseLockScreenReadOnly = PEMSTATUS(oBaseForm, 'LockScreen', 5)\n"
            "lBaseBefore = oBaseForm.LockScreen\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'LockScreen')\n"
            "oBaseForm.LockScreen = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.LockScreen\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'LockScreen', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.LockScreen\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'LockScreen', .T.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'LockScreen')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.LockScreen\n"
            "cChildBefore = oDerived.cmdSave.ReadLock()\n"
            "oDerived.cmdSave.LockForm()\n"
            "lDerivedAfterChild = oDerived.LockScreen\n"
            "xDerivedGetPem = GETPEM(oDerived, 'LockScreen')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasLockScreen = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'LOCKSCREEN'\n"
            "        lPropHasLockScreen = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadLock\n"
            "        RETURN IIF(THISFORM.LockScreen, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE LockForm\n"
            "        THISFORM.LockScreen = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form LockScreen property script should complete: ") + state.message +
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

        check("lbasehaslockscreen", "true");
        check("lbaselockscreenreadonly", "false");
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
        check("lprophaslockscreen", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_alwaysontop_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_alwaysontop";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_alwaysontop.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasAlwaysOnTop = PEMSTATUS(oBaseForm, 'AlwaysOnTop', 1)\n"
            "lBaseAlwaysOnTopReadOnly = PEMSTATUS(oBaseForm, 'AlwaysOnTop', 5)\n"
            "lBaseBefore = oBaseForm.AlwaysOnTop\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'AlwaysOnTop')\n"
            "oBaseForm.AlwaysOnTop = .T.\n"
            "lBaseAfterDirectAssign = oBaseForm.AlwaysOnTop\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'AlwaysOnTop', .F.)\n"
            "lBaseAfterSetPem = oBaseForm.AlwaysOnTop\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'AlwaysOnTop', .T.)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'AlwaysOnTop')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lDerivedBefore = oDerived.AlwaysOnTop\n"
            "cChildBefore = oDerived.cmdSave.ReadAlwaysOnTop()\n"
            "oDerived.cmdSave.PinForm()\n"
            "lDerivedAfterChild = oDerived.AlwaysOnTop\n"
            "xDerivedGetPem = GETPEM(oDerived, 'AlwaysOnTop')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasAlwaysOnTop = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ALWAYSONTOP'\n"
            "        lPropHasAlwaysOnTop = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadAlwaysOnTop\n"
            "        RETURN IIF(THISFORM.AlwaysOnTop, 'T', 'F')\n"
            "    ENDFUNC\n"
            "    PROCEDURE PinForm\n"
            "        THISFORM.AlwaysOnTop = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form AlwaysOnTop property script should complete: ") + state.message +
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

        check("lbasehasalwaysontop", "true");
        check("lbasealwaysontopreadonly", "false");
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
        check("lprophasalwaysontop", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_showwindow_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_showwindow";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_showwindow.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasShowWindow = PEMSTATUS(oBaseForm, 'ShowWindow', 1)\n"
            "lBaseShowWindowReadOnly = PEMSTATUS(oBaseForm, 'ShowWindow', 5)\n"
            "nBaseBefore = oBaseForm.ShowWindow\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ShowWindow')\n"
            "oBaseForm.ShowWindow = 2\n"
            "nBaseAfterDirectAssign = oBaseForm.ShowWindow\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ShowWindow', 0)\n"
            "nBaseAfterSetPem = oBaseForm.ShowWindow\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ShowWindow', 2)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ShowWindow')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.ShowWindow\n"
            "nChildBefore = oDerived.cmdSave.ReadShowWindow()\n"
            "oDerived.cmdSave.MakeTopLevel()\n"
            "nDerivedAfterChild = oDerived.ShowWindow\n"
            "lDerivedIsTopLevel = (oDerived.ShowWindow = 2)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'ShowWindow')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasShowWindow = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SHOWWINDOW'\n"
            "        lPropHasShowWindow = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadShowWindow\n"
            "        RETURN THISFORM.ShowWindow\n"
            "    ENDFUNC\n"
            "    PROCEDURE MakeTopLevel\n"
            "        THISFORM.ShowWindow = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ShowWindow property script should complete: ") + state.message +
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

        check("lbasehasshowwindow", "true");
        check("lbaseshowwindowreadonly", "false");
        check("nbasebefore", "0");
        check("xbasegetpembefore", "0");
        check("nbaseafterdirectassign", "2");
        check("lbasesetpem", "true");
        check("nbaseaftersetpem", "0");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "0");
        check("nchildbefore", "0");
        check("nderivedafterchild", "2");
        check("lderivedistoplevel", "true");
        check("xderivedgetpem", "2");
        check("lprophasshowwindow", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_windowtype_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_windowtype";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_windowtype.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasWindowType = PEMSTATUS(oBaseForm, 'WindowType', 1)\n"
            "lBaseWindowTypeReadOnly = PEMSTATUS(oBaseForm, 'WindowType', 5)\n"
            "nBaseBefore = oBaseForm.WindowType\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'WindowType')\n"
            "oBaseForm.WindowType = 1\n"
            "nBaseAfterDirectAssign = oBaseForm.WindowType\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'WindowType', 0)\n"
            "nBaseAfterSetPem = oBaseForm.WindowType\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'WindowType', 1)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'WindowType')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.WindowType\n"
            "nChildBefore = oDerived.cmdSave.ReadWindowType()\n"
            "oDerived.cmdSave.MakeModal()\n"
            "nDerivedAfterChild = oDerived.WindowType\n"
            "lDerivedIsModal = (oDerived.WindowType = 1)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'WindowType')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasWindowType = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'WINDOWTYPE'\n"
            "        lPropHasWindowType = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadWindowType\n"
            "        RETURN THISFORM.WindowType\n"
            "    ENDFUNC\n"
            "    PROCEDURE MakeModal\n"
            "        THISFORM.WindowType = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form WindowType property script should complete: ") + state.message +
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

        check("lbasehaswindowtype", "true");
        check("lbasewindowtypereadonly", "false");
        check("nbasebefore", "0");
        check("xbasegetpembefore", "0");
        check("nbaseafterdirectassign", "1");
        check("lbasesetpem", "true");
        check("nbaseaftersetpem", "0");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "0");
        check("nchildbefore", "0");
        check("nderivedafterchild", "1");
        check("lderivedismodal", "true");
        check("xderivedgetpem", "1");
        check("lprophaswindowtype", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_windowtype_include_define_constants_drive_modal_checks()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_windowtype_include_define";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path header_path = temp_root / "ui.h";
        write_text(
            header_path,
            "#DEFINE WINDOWTYPE_MODELESS 0\n"
            "#DEFINE WINDOWTYPE_MODAL 1\n");

        const fs::path main_path = temp_root / "native_form_windowtype_include_define.prg";
        write_text(
            main_path,
            "#INCLUDE \"ui.h\"\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "lBeforeIsModeless = (oDerived.WindowType = WINDOWTYPE_MODELESS)\n"
            "nChildBefore = oDerived.cmdSave.ReadWindowType()\n"
            "oDerived.cmdSave.MakeModal()\n"
            "lAfterIsModal = (oDerived.WindowType = WINDOWTYPE_MODAL)\n"
            "lChildReadsModal = (oDerived.cmdSave.ReadWindowType() = WINDOWTYPE_MODAL)\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadWindowType\n"
            "        RETURN THISFORM.WindowType\n"
            "    ENDFUNC\n"
            "    PROCEDURE MakeModal\n"
            "        THISFORM.WindowType = WINDOWTYPE_MODAL\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    WindowType = WINDOWTYPE_MODELESS\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form WindowType include/define script should complete: ") + state.message +
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

        check("lbeforeismodeless", "true");
        check("nchildbefore", "0");
        check("lafterismodal", "true");
        check("lchildreadsmodal", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_windowtype_conditional_preprocessor_branches_drive_runtime_behavior()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_windowtype_preprocessor_conditionals";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root / "include");

        const fs::path header_path = temp_root / "include" / "ui.h";
        write_text(
            header_path,
            "#DEFINE WINDOWTYPE_MODELESS 0\n"
            "#DEFINE WINDOWTYPE_MODAL 1\n"
            "#DEFINE FEATURE_COMPILEBEFORE .T.\n"
            "#IF 1\n"
            "#DEFINE ACTIVE_MODAL WINDOWTYPE_MODAL\n"
            "#ELSE\n"
            "#DEFINE ACTIVE_MODAL 77\n"
            "#ENDIF\n");

        const fs::path main_path = temp_root / "native_form_windowtype_preprocessor_conditionals.prg";
        write_text(
            main_path,
            "#INCLUDE \"include\\\\ui.h\"\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "#IFDEF FEATURE_COMPILEBEFORE\n"
            "lFeatureEnabled = .T.\n"
            "#ELSE\n"
            "lFeatureEnabled = .F.\n"
            "#ENDIF\n"
            "lBeforeIsModeless = (oDerived.WindowType = WINDOWTYPE_MODELESS)\n"
            "oDerived.cmdSave.MakeConditionalModal()\n"
            "lAfterIsModal = (oDerived.WindowType = ACTIVE_MODAL)\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    PROCEDURE MakeConditionalModal\n"
            "#IF 0\n"
            "        THISFORM.WindowType = 99\n"
            "#ELSE\n"
            "        THISFORM.WindowType = ACTIVE_MODAL\n"
            "#ENDIF\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    WindowType = WINDOWTYPE_MODELESS\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form WindowType conditional-preprocessor script should complete: ") + state.message +
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

        check("lfeatureenabled", "true");
        check("lbeforeismodeless", "true");
        check("lafterismodal", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_windowstate_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_windowstate";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_windowstate.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasWindowState = PEMSTATUS(oBaseForm, 'WindowState', 1)\n"
            "lBaseWindowStateReadOnly = PEMSTATUS(oBaseForm, 'WindowState', 5)\n"
            "nBaseBefore = oBaseForm.WindowState\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'WindowState')\n"
            "oBaseForm.WindowState = 2\n"
            "nBaseAfterDirectAssign = oBaseForm.WindowState\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'WindowState', 1)\n"
            "nBaseAfterSetPem = oBaseForm.WindowState\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'WindowState', 2)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'WindowState')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.WindowState\n"
            "nChildBefore = oDerived.cmdSave.ReadWindowState()\n"
            "oDerived.cmdSave.NormalizeWindow()\n"
            "nDerivedAfterChild = oDerived.WindowState\n"
            "lDerivedIsNormal = (oDerived.WindowState = 0)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'WindowState')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasWindowState = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'WINDOWSTATE'\n"
            "        lPropHasWindowState = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadWindowState\n"
            "        RETURN THISFORM.WindowState\n"
            "    ENDFUNC\n"
            "    PROCEDURE NormalizeWindow\n"
            "        THISFORM.WindowState = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    WindowState = 2\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form WindowState property script should complete: ") + state.message +
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

        check("lbasehaswindowstate", "true");
        check("lbasewindowstatereadonly", "false");
        check("nbasebefore", "0");
        check("xbasegetpembefore", "0");
        check("nbaseafterdirectassign", "2");
        check("lbasesetpem", "true");
        check("nbaseaftersetpem", "1");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "2");
        check("nchildbefore", "2");
        check("nderivedafterchild", "0");
        check("lderivedisnormal", "true");
        check("xderivedgetpem", "0");
        check("lprophaswindowstate", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_borderstyle_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_borderstyle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_borderstyle.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasBorderStyle = PEMSTATUS(oBaseForm, 'BorderStyle', 1)\n"
            "lBaseBorderStyleReadOnly = PEMSTATUS(oBaseForm, 'BorderStyle', 5)\n"
            "nBaseBefore = oBaseForm.BorderStyle\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'BorderStyle')\n"
            "oBaseForm.BorderStyle = 2\n"
            "nBaseAfterDirectAssign = oBaseForm.BorderStyle\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'BorderStyle', 0)\n"
            "nBaseAfterSetPem = oBaseForm.BorderStyle\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'BorderStyle', 1)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'BorderStyle')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.BorderStyle\n"
            "nChildBefore = oDerived.cmdSave.ReadBorderStyle()\n"
            "oDerived.cmdSave.RemoveBorder()\n"
            "nDerivedAfterChild = oDerived.BorderStyle\n"
            "lDerivedHasNoBorder = (oDerived.BorderStyle = 0)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'BorderStyle')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasBorderStyle = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'BORDERSTYLE'\n"
            "        lPropHasBorderStyle = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadBorderStyle\n"
            "        RETURN THISFORM.BorderStyle\n"
            "    ENDFUNC\n"
            "    PROCEDURE RemoveBorder\n"
            "        THISFORM.BorderStyle = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form BorderStyle property script should complete: ") + state.message +
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

        check("lbasehasborderstyle", "true");
        check("lbaseborderstylereadonly", "false");
        check("nbasebefore", "3");
        check("xbasegetpembefore", "3");
        check("nbaseafterdirectassign", "2");
        check("lbasesetpem", "true");
        check("nbaseaftersetpem", "0");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "3");
        check("nchildbefore", "3");
        check("nderivedafterchild", "0");
        check("lderivedhasnoborder", "true");
        check("xderivedgetpem", "0");
        check("lprophasborderstyle", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_titlebar_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_titlebar";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_titlebar.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasTitleBar = PEMSTATUS(oBaseForm, 'TitleBar', 1)\n"
            "lBaseTitleBarReadOnly = PEMSTATUS(oBaseForm, 'TitleBar', 5)\n"
            "nBaseBefore = oBaseForm.TitleBar\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'TitleBar')\n"
            "oBaseForm.TitleBar = 0\n"
            "nBaseAfterDirectAssign = oBaseForm.TitleBar\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'TitleBar', 1)\n"
            "nBaseAfterSetPem = oBaseForm.TitleBar\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'TitleBar', 0)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'TitleBar')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.TitleBar\n"
            "nChildBefore = oDerived.cmdSave.ReadTitleBar()\n"
            "oDerived.cmdSave.HideTitleBar()\n"
            "nDerivedAfterChild = oDerived.TitleBar\n"
            "lDerivedTitleBarHidden = (oDerived.TitleBar = 0)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'TitleBar')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasTitleBar = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'TITLEBAR'\n"
            "        lPropHasTitleBar = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadTitleBar\n"
            "        RETURN THISFORM.TitleBar\n"
            "    ENDFUNC\n"
            "    PROCEDURE HideTitleBar\n"
            "        THISFORM.TitleBar = 0\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form TitleBar property script should complete: ") + state.message +
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

        check("lbasehastitlebar", "true");
        check("lbasetitlebarreadonly", "false");
        check("nbasebefore", "1");
        check("xbasegetpembefore", "1");
        check("nbaseafterdirectassign", "0");
        check("lbasesetpem", "true");
        check("nbaseaftersetpem", "1");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "1");
        check("nchildbefore", "1");
        check("nderivedafterchild", "0");
        check("lderivedtitlebarhidden", "true");
        check("xderivedgetpem", "0");
        check("lprophastitlebar", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_scrollbars_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_scrollbars";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_scrollbars.prg";
        write_text(
            main_path,
            "oBaseForm = CREATEOBJECT('Form')\n"
            "lBaseHasScrollBars = PEMSTATUS(oBaseForm, 'ScrollBars', 1)\n"
            "lBaseScrollBarsReadOnly = PEMSTATUS(oBaseForm, 'ScrollBars', 5)\n"
            "nBaseBefore = oBaseForm.ScrollBars\n"
            "xBaseGetPemBefore = GETPEM(oBaseForm, 'ScrollBars')\n"
            "oBaseForm.ScrollBars = 3\n"
            "nBaseAfterDirectAssign = oBaseForm.ScrollBars\n"
            "lBaseSetPem = SETPEM(oBaseForm, 'ScrollBars', 2)\n"
            "nBaseAfterSetPem = oBaseForm.ScrollBars\n"
            "lBaseAddProperty = ADDPROPERTY(oBaseForm, 'ScrollBars', 1)\n"
            "lBaseRemoveProperty = REMOVEPROPERTY(oBaseForm, 'ScrollBars')\n"
            "oDerived = CREATEOBJECT('DemoForm')\n"
            "nDerivedBefore = oDerived.ScrollBars\n"
            "nChildBefore = oDerived.cmdSave.ReadScrollBars()\n"
            "oDerived.cmdSave.TryHorizontal()\n"
            "nDerivedAfterChild = oDerived.ScrollBars\n"
            "lDerivedStillBoth = (oDerived.ScrollBars = 3)\n"
            "xDerivedGetPem = GETPEM(oDerived, 'ScrollBars')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oDerived, 1)\n"
            "lPropHasScrollBars = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SCROLLBARS'\n"
            "        lPropHasScrollBars = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION ReadScrollBars\n"
            "        RETURN THISFORM.ScrollBars\n"
            "    ENDFUNC\n"
            "    PROCEDURE TryHorizontal\n"
            "        THISFORM.ScrollBars = 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    ScrollBars = 3\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Form ScrollBars property script should complete: ") + state.message +
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

        check("lbasehasscrollbars", "true");
        check("lbasescrollbarsreadonly", "true");
        check("nbasebefore", "0");
        check("xbasegetpembefore", "0");
        check("nbaseafterdirectassign", "0");
        check("lbasesetpem", "false");
        check("nbaseaftersetpem", "0");
        check("lbaseaddproperty", "false");
        check("lbaseremoveproperty", "false");
        check("nderivedbefore", "3");
        check("nchildbefore", "3");
        check("nderivedafterchild", "3");
        check("lderivedstillboth", "true");
        check("xderivedgetpem", "3");
        check("lprophasscrollbars", "true");

        fs::remove_all(temp_root, ignored);
    }

}
