#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_commandbutton_default_cancel_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_commandbutton_default_cancel";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandbutton_default_cancel.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lDefaultHas = PEMSTATUS(oButton, 'Default', 1)\n"
            "lCancelHas = PEMSTATUS(oButton, 'Cancel', 1)\n"
            "lDefaultReadOnly = PEMSTATUS(oButton, 'Default', 5)\n"
            "lCancelReadOnly = PEMSTATUS(oButton, 'Cancel', 5)\n"
            "lDefaultMember = .F.\n"
            "lCancelMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DEFAULT'\n"
            "        lDefaultMember = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aMembers[i]) == 'CANCEL'\n"
            "        lCancelMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lDefaultInitial = oButton.Default\n"
            "lCancelInitial = GETPEM(oButton, 'Cancel')\n"
            "oButton.Default = 1.6\n"
            "lDefaultDirect = oButton.Default\n"
            "lCancelSetPem = SETPEM(oButton, 'Cancel', 1)\n"
            "lCancelAfterSetPem = GETPEM(oButton, 'Cancel')\n"
            "lDefaultPutPem = PUTPEM(oButton, 'Default', 0)\n"
            "lDefaultAfterPutPem = GETPEM(oButton, 'Default')\n"
            "lAddDefault = ADDPROPERTY(oButton, 'Default', .T.)\n"
            "lRemoveCancel = REMOVEPROPERTY(oButton, 'Cancel')\n"
            "oDerived = CREATEOBJECT('DerivedCommandButton')\n"
            "lDerivedDefault = oDerived.Default\n"
            "lDerivedCancel = oDerived.Cancel\n"
            "RETURN\n"
            "DEFINE CLASS DerivedCommandButton AS CommandButton\n"
            "    Default = .T.\n"
            "    Cancel = 1.6\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandButton Default/Cancel script should complete: ") + state.message +
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

        check("ldefaulthas", "true");
        check("lcancelhas", "true");
        check("ldefaultreadonly", "false");
        check("lcancelreadonly", "false");
        check("ldefaultmember", "true");
        check("lcancelmember", "true");
        check("ldefaultinitial", "false");
        check("lcancelinitial", "false");
        check("ldefaultdirect", "true");
        check("lcancelsetpem", "true");
        check("lcancelaftersetpem", "true");
        check("ldefaultputpem", "true");
        check("ldefaultafterputpem", "false");
        check("ladddefault", "false");
        check("lremovecancel", "false");
        check("lderiveddefault", "true");
        check("lderivedcancel", "true");
        expect(state.ole_objects.size() == 2U,
               "native CommandButton Default/Cancel coverage should register the base and derived objects");

        fs::remove_all(temp_root, ignored);
    }
}
