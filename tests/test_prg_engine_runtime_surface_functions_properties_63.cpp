#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_childorder_defaults_are_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_childorder";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_childorder.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "lHas = PEMSTATUS(oGrid, 'ChildOrder', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'ChildOrder', 5)\n"
            "cDefault = oGrid.ChildOrder\n"
            "cGetPem = GETPEM(oGrid, 'ChildOrder')\n"
            "oGrid.ChildOrder = 'customer_id'\n"
            "cAfterDirect = oGrid.ChildOrder\n"
            "lSetPem = SETPEM(oGrid, 'ChildOrder', 'shadow_setpem')\n"
            "cAfterSetPem = GETPEM(oGrid, 'ChildOrder')\n"
            "lPutPem = PUTPEM(oGrid, 'ChildOrder', 'shadow_putpem')\n"
            "cAfterPutPem = GETPEM(oGrid, 'ChildOrder')\n"
            "lAdd = ADDPROPERTY(oGrid, 'ChildOrder', 'shadow')\n"
            "lRemove = REMOVEPROPERTY(oGrid, 'ChildOrder')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oGrid, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'CHILDORDER'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedChildOrderGrid')\n"
            "cDerived = oDerived.ChildOrder\n"
            "RETURN\n"
            "DEFINE CLASS DerivedChildOrderGrid AS Grid\n"
            "    ChildOrder = 'customer_id'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid ChildOrder script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "true");
        check("cdefault", "");
        check("cgetpem", "");
        check("cafterdirect", "");
        check("lsetpem", "false");
        check("caftersetpem", "");
        check("lputpem", "false");
        check("cafterputpem", "");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("cderived", "customer_id");

        fs::remove_all(temp_root, ignored);
    }
}
