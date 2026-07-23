#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_control_columnlines_property_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_list_columnlines";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_list_columnlines.prg";
        write_text(
            main_path,
            "oList = CREATEOBJECT('ListBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "lDefault = oList.ColumnLines\n"
            "lListHas = PEMSTATUS(oList, 'ColumnLines', 1)\n"
            "lComboHas = PEMSTATUS(oCombo, 'ColumnLines', 1)\n"
            "lReadOnly = PEMSTATUS(oList, 'ColumnLines', 5)\n"
            "oList.ColumnLines = .F.\n"
            "lDirect = oList.ColumnLines\n"
            "lSetPem = SETPEM(oList, 'ColumnLines', .T.)\n"
            "lAfterSetPem = GETPEM(oList, 'ColumnLines')\n"
            "lPutPem = PUTPEM(oList, 'ColumnLines', .F.)\n"
            "lAfterPutPem = GETPEM(oList, 'ColumnLines')\n"
            "oList.ColumnLines = 1\n"
            "lNormalized = oList.ColumnLines\n"
            "lAddProperty = ADDPROPERTY(oList, 'ColumnLines', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'ColumnLines')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oList, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'COLUMNLINES'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedColumnList')\n"
            "lDerived = oDerived.ColumnLines\n"
            "RETURN\n"
            "DEFINE CLASS DerivedColumnList AS ListBox\n"
            "    PROCEDURE Init\n"
            "        THIS.ColumnLines = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ColumnLines script should complete: ") + state.message +
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

        check("ldefault", "true");
        check("llisthas", "true");
        check("lcombohas", "true");
        check("lreadonly", "false");
        check("ldirect", "false");
        check("lsetpem", "true");
        check("laftersetpem", "true");
        check("lputpem", "true");
        check("lafterputpem", "false");
        check("lnormalized", "true");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        expect(state.ole_objects.size() == 3U,
               "native ColumnLines coverage should register ListBox, ComboBox, and derived ListBox");

        fs::remove_all(temp_root, ignored);
    }
}
