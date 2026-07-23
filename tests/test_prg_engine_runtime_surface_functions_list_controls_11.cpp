#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_control_itemtips_property_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_itemtips";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_itemtips.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lListDefault = oList.ItemTips\n"
            "lComboDefault = oCombo.ItemTips\n"
            "lListHas = PEMSTATUS(oList, 'ItemTips', 1)\n"
            "lComboHas = PEMSTATUS(oCombo, 'ItemTips', 1)\n"
            "lReadOnly = PEMSTATUS(oList, 'ItemTips', 5)\n"
            "oList.ItemTips = .T.\n"
            "lDirect = oList.ItemTips\n"
            "lSetPem = SETPEM(oList, 'ItemTips', .F.)\n"
            "lAfterSetPem = GETPEM(oList, 'ItemTips')\n"
            "lPutPem = PUTPEM(oList, 'ItemTips', 1)\n"
            "lAfterPutPem = GETPEM(oList, 'ItemTips')\n"
            "oList.ItemTips = 0\n"
            "lNormalized = oList.ItemTips\n"
            "lAddProperty = ADDPROPERTY(oList, 'ItemTips', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'ItemTips')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ITEMTIPS'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedItemTipsList')\n"
            "lDerived = oDerived.ItemTips\n"
            "RETURN\n"
            "DEFINE CLASS DerivedItemTipsList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.ItemTips = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ItemTips script should complete: ") + state.message +
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

        check("llistdefault", "false");
        check("lcombodefault", "false");
        check("llisthas", "true");
        check("lcombohas", "true");
        check("lreadonly", "false");
        check("ldirect", "true");
        check("lsetpem", "true");
        check("laftersetpem", "false");
        check("lputpem", "true");
        check("lafterputpem", "true");
        check("lnormalized", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "true");
        expect(state.ole_objects.size() == 3U,
               "native ItemTips coverage should register ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
