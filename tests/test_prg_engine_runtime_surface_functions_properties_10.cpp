#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_textbox_inputmask_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_inputmask";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_inputmask.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "lHas = PEMSTATUS(oText, 'InputMask', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'InputMask', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'InputMask', 1)\n"
            "cDefault = oText.InputMask\n"
            "oText.InputMask = '999-999'\n"
            "cDirect = oText.InputMask\n"
            "lSetPem = SETPEM(oText, 'InputMask', 123)\n"
            "cSetPem = GETPEM(oText, 'InputMask')\n"
            "lPutPem = PUTPEM(oText, 'InputMask', 'AAA/999')\n"
            "cPutPem = GETPEM(oText, 'InputMask')\n"
            "lAddProperty = ADDPROPERTY(oText, 'InputMask', 'X')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'InputMask')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'INPUTMASK'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedInputMaskText')\n"
            "cDerived = oDerived.InputMask\n"
            "RETURN\n"
            "DEFINE CLASS DerivedInputMaskText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.InputMask = 'AA-999'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native InputMask script should complete: ") + state.message +
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
        check("ledithas", "false");
        check("cdefault", "");
        check("cdirect", "999-999");
        check("lsetpem", "true");
        check("csetpem", "123");
        check("lputpem", "true");
        check("cputpem", "AAA/999");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cderived", "AA-999");
        expect(state.ole_objects.size() == 3U,
               "native InputMask coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_textbox_dynamicinputmask_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_textbox_dynamicinputmask";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_dynamicinputmask.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('Column')\n"
            "oEdit = CREATEOBJECT('TextBox')\n"
            "lHas = PEMSTATUS(oText, 'DynamicInputMask', 1)\n"
            "lReadOnly = PEMSTATUS(oText, 'DynamicInputMask', 5)\n"
            "lEditHas = PEMSTATUS(oEdit, 'DynamicInputMask', 1)\n"
            "cDefault = oText.DynamicInputMask\n"
            "oText.DynamicInputMask = 'IIF(EMPTY(Value), 999999, 111111)'\n"
            "cDirect = oText.DynamicInputMask\n"
            "lSetPem = SETPEM(oText, 'DynamicInputMask', 123)\n"
            "cSetPem = GETPEM(oText, 'DynamicInputMask')\n"
            "lPutPem = PUTPEM(oText, 'DynamicInputMask', 'UPPER(Value)')\n"
            "cPutPem = GETPEM(oText, 'DynamicInputMask')\n"
            "lAddProperty = ADDPROPERTY(oText, 'DynamicInputMask', 'X')\n"
            "lRemoveProperty = REMOVEPROPERTY(oText, 'DynamicInputMask')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oText, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'DYNAMICINPUTMASK'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedDynamicInputMaskText')\n"
            "cDerived = oDerived.DynamicInputMask\n"
            "RETURN\n"
            "DEFINE CLASS DerivedDynamicInputMaskText AS Column\n"
            "    PROCEDURE Init\n"
            "        THIS.DynamicInputMask = 'TRANSFORM(Value)'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DynamicInputMask script should complete: ") + state.message +
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
        check("ledithas", "false");
        check("cdefault", "");
        check("cdirect", "IIF(EMPTY(Value), 999999, 111111)");
        check("lsetpem", "true");
        check("csetpem", "123");
        check("lputpem", "true");
        check("cputpem", "UPPER(Value)");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cderived", "TRANSFORM(Value)");
        expect(state.ole_objects.size() == 3U,
               "native DynamicInputMask coverage should register the base, non-owning EditBox, and derived TextBox");

        fs::remove_all(temp_root, ignored);
    }
}
