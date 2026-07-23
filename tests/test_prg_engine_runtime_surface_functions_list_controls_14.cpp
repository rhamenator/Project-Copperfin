#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_selectonentry_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_selectonentry";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_selectonentry.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lTextDefault = oText.SelectOnEntry\n"
            "lEditDefault = oEdit.SelectOnEntry\n"
            "lColumnDefault = oColumn.SelectOnEntry\n"
            "lTextHas = PEMSTATUS(oText, 'SelectOnEntry', 1)\n"
            "lEditHas = PEMSTATUS(oEdit, 'SelectOnEntry', 1)\n"
            "lColumnHas = PEMSTATUS(oColumn, 'SelectOnEntry', 1)\n"
            "lReadOnly = PEMSTATUS(oColumn, 'SelectOnEntry', 5)\n"
            "oText.SelectOnEntry = 1\n"
            "lTextDirect = oText.SelectOnEntry\n"
            "lEditSetPem = SETPEM(oEdit, 'SelectOnEntry', 1)\n"
            "lEditAfterSetPem = GETPEM(oEdit, 'SelectOnEntry')\n"
            "lColumnPutPem = PUTPEM(oColumn, 'SelectOnEntry', 0)\n"
            "lColumnAfterPutPem = GETPEM(oColumn, 'SelectOnEntry')\n"
            "oText.SelectOnEntry = 0\n"
            "lTextNormalized = oText.SelectOnEntry\n"
            "lAddProperty = ADDPROPERTY(oColumn, 'SelectOnEntry', .T.)\n"
            "lRemoveProperty = REMOVEPROPERTY(oColumn, 'SelectOnEntry')\n"
            "nPropMembers = AMEMBERS(aPropMembers, oColumn, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'SELECTONENTRY'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSelectOnEntryColumn')\n"
            "lDerived = oDerived.SelectOnEntry\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSelectOnEntryColumn AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.SelectOnEntry = .F.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SelectOnEntry script should complete: ") + state.message +
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

        check("ltextdefault", "false");
        check("leditdefault", "false");
        check("lcolumndefault", "true");
        check("ltexthas", "true");
        check("ledithas", "true");
        check("lcolumnhas", "true");
        check("lreadonly", "false");
        check("ltextdirect", "true");
        check("leditsetpem", "true");
        check("leditaftersetpem", "true");
        check("lcolumnputpem", "true");
        check("lcolumnafterputpem", "false");
        check("ltextnormalized", "false");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lprophas", "true");
        check("lderived", "false");
        expect(state.ole_objects.size() == 4U,
               "native SelectOnEntry coverage should register TextBox, EditBox, Column, and derived Column");

        fs::remove_all(temp_root, ignored);
    }
}
