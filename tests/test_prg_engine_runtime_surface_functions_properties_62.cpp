#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_grid_linkmaster_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_grid_linkmaster";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_grid_linkmaster.prg";
        write_text(
            main_path,
            "oGrid = CREATEOBJECT('Grid')\n"
            "lHas = PEMSTATUS(oGrid, 'LinkMaster', 1)\n"
            "lReadOnly = PEMSTATUS(oGrid, 'LinkMaster', 5)\n"
            "cDefault = oGrid.LinkMaster\n"
            "oGrid.LinkMaster = 'customer'\n"
            "cDirect = oGrid.LinkMaster\n"
            "lSetPem = SETPEM(oGrid, 'LinkMaster', 'customer_alias')\n"
            "cAfterSetPem = GETPEM(oGrid, 'LinkMaster')\n"
            "lPutPem = PUTPEM(oGrid, 'LinkMaster', 'customer_parent')\n"
            "cAfterPutPem = GETPEM(oGrid, 'LinkMaster')\n"
            "lAdd = ADDPROPERTY(oGrid, 'LinkMaster', 'shadow')\n"
            "lRemove = REMOVEPROPERTY(oGrid, 'LinkMaster')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oGrid, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'LINKMASTER'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedLinkMasterGrid')\n"
            "cDerived = oDerived.LinkMaster\n"
            "RETURN\n"
            "DEFINE CLASS DerivedLinkMasterGrid AS Grid\n"
            "    LinkMaster = 'customer_parent'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Grid LinkMaster script should complete: ") + state.message +
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
        check("lreadonly", "false");
        check("cdefault", "");
        check("cdirect", "customer");
        check("lsetpem", "true");
        check("caftersetpem", "customer_alias");
        check("lputpem", "true");
        check("cafterputpem", "customer_parent");
        check("ladd", "false");
        check("lremove", "false");
        check("lmember", "true");
        check("cderived", "customer_parent");

        fs::remove_all(temp_root, ignored);
    }
}
