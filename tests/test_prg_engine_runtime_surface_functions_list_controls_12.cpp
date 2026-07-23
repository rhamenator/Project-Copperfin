#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_listbox_integralheight_property_stays_read_only()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_integralheight";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_integralheight.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lDefault = oList.IntegralHeight\n"
            "lHas = PEMSTATUS(oList, 'IntegralHeight', 1)\n"
            "lReadOnly = PEMSTATUS(oList, 'IntegralHeight', 5)\n"
            "lComboHas = PEMSTATUS(oCombo, 'IntegralHeight', 1)\n"
            "lBeforeDirect = oList.IntegralHeight\n"
            "oList.IntegralHeight = .T.\n"
            "lAfterDirect = oList.IntegralHeight\n"
            "lSetPem = SETPEM(oList, 'IntegralHeight', .T.)\n"
            "lAfterSetPem = GETPEM(oList, 'IntegralHeight')\n"
            "lPutPem = PUTPEM(oList, 'IntegralHeight', .T.)\n"
            "lAfterPutPem = GETPEM(oList, 'IntegralHeight')\n"
            "lAddProperty = ADDPROPERTY(oList, 'IntegralHeight', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'IntegralHeight')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'INTEGRALHEIGHT'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedIntegralHeightList')\n"
            "lDerived = oDerived.IntegralHeight\n"
            "lDerivedReadOnly = PEMSTATUS(oDerived, 'IntegralHeight', 5)\n"
            "RETURN\n"
            "DEFINE CLASS DerivedIntegralHeightList AS ListBox\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native IntegralHeight script should complete: ") + state.message +
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

        check("ldefault", "false");
        check("lhas", "true");
        check("lreadonly", "true");
        check("lcombohas", "false");
        check("lbeforedirect", "false");
        check("lafterdirect", "false");
        check("lsetpem", "false");
        check("laftersetpem", "false");
        check("lputpem", "false");
        check("lafterputpem", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        check("lderivedreadonly", "true");
        expect(state.ole_objects.size() == 3U,
               "native IntegralHeight coverage should register ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
