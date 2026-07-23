#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_column_resizable_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_column_resizable";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_column_resizable.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('Column')\n"
            "oText = CREATEOBJECT('TextBox')\n"
            "lTextHas = PEMSTATUS(oText, 'Resizable', 1)\n"
            "lHas = PEMSTATUS(oPlain, 'Resizable', 1)\n"
            "lReadOnly = PEMSTATUS(oPlain, 'Resizable', 5)\n"
            "lDefault = oPlain.Resizable\n"
            "xGetPem = GETPEM(oPlain, 'Resizable')\n"
            "oPlain.Resizable = 1\n"
            "lDirect = oPlain.Resizable\n"
            "lSetPem = SETPEM(oPlain, 'Resizable', 0)\n"
            "lAfterSetPem = GETPEM(oPlain, 'Resizable')\n"
            "lPutPem = PUTPEM(oPlain, 'Resizable', 1)\n"
            "lAfterPutPem = GETPEM(oPlain, 'Resizable')\n"
            "lAddProperty = ADDPROPERTY(oPlain, 'Resizable', .F.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oPlain, 'Resizable')\n"
            "nMembers = AMEMBERS(aMembers, oPlain, 1)\n"
            "lMember = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'RESIZABLE'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedResizableColumn')\n"
            "lDerived = oDerived.Resizable\n"
            "oDeclarative = CREATEOBJECT('DeclarativeResizableColumn')\n"
            "lDeclarative = oDeclarative.Resizable\n"
            "RETURN\n"
            "DEFINE CLASS DerivedResizableColumn AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.Resizable = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DeclarativeResizableColumn AS Column\n"
            "    Resizable = .T.\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Column Resizable script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ltexthas", "false");
        check("lhas", "true");
        check("lreadonly", "false");
        check("ldefault", "false");
        check("xgetpem", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("laftersetpem", "false");
        check("lputpem", "true");
        check("lafterputpem", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("lderived", "true");
        check("ldeclarative", "true");

        fs::remove_all(temp_root, ignored);
    }
}
