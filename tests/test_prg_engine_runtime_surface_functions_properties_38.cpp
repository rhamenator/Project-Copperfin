#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_movable_defaults_are_runtime_readonly_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_movable";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_movable.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('Form')\n"
            "lFormHas = PEMSTATUS(oForm, 'Movable', 1)\n"
            "lFormReadOnly = PEMSTATUS(oForm, 'Movable', 5)\n"
            "lFormBefore = oForm.Movable\n"
            "xFormGetPem = GETPEM(oForm, 'Movable')\n"
            "oForm.Movable = .F.\n"
            "lFormAfterDirect = oForm.Movable\n"
            "lFormSetPem = SETPEM(oForm, 'Movable', .F.)\n"
            "lFormAfterSetPem = oForm.Movable\n"
            "lFormPutPem = PUTPEM(oForm, 'Movable', .F.)\n"
            "lFormAddProperty = ADDPROPERTY(oForm, 'Movable', .F.)\n"
            "lFormRemoveProperty = REMOVEPROPERTY(oForm, 'Movable')\n"
            "oColumn = CREATEOBJECT('Column')\n"
            "lColumnHas = PEMSTATUS(oColumn, 'Movable', 1)\n"
            "lColumnReadOnly = PEMSTATUS(oColumn, 'Movable', 5)\n"
            "lColumnBefore = oColumn.Movable\n"
            "oColumn.Movable = .F.\n"
            "lColumnAfterDirect = oColumn.Movable\n"
            "lColumnSetPem = SETPEM(oColumn, 'Movable', .F.)\n"
            "lColumnAddProperty = ADDPROPERTY(oColumn, 'Movable', .F.)\n"
            "lColumnRemoveProperty = REMOVEPROPERTY(oColumn, 'Movable')\n"
            "oDerived = CREATEOBJECT('DerivedMovableForm')\n"
            "lDerived = oDerived.Movable\n"
            "nMembers = AMEMBERS(aMembers, oDerived, 1)\n"
            "lPropHas = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'MOVABLE'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMovableForm AS Form\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Movable property script should complete: ") + state.message +
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

        check("lformhas", "true");
        check("lformreadonly", "true");
        check("lformbefore", "true");
        check("xformgetpem", "true");
        check("lformafterdirect", "true");
        check("lformsetpem", "false");
        check("lformaftersetpem", "true");
        check("lformputpem", "false");
        check("lformaddproperty", "false");
        check("lformremoveproperty", "false");
        check("lcolumnhas", "true");
        check("lcolumnreadonly", "true");
        check("lcolumnbefore", "true");
        check("lcolumnafterdirect", "true");
        check("lcolumnsetpem", "false");
        check("lcolumnaddproperty", "false");
        check("lcolumnremoveproperty", "false");
        check("lderived", "true");
        check("lprophas", "true");
        expect(state.ole_objects.size() == 3U,
               "native Movable coverage should register Form, Column, and derived Form");

        fs::remove_all(temp_root, ignored);
    }
}
