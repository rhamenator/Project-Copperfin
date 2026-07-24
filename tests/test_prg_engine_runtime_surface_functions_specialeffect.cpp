#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_specialeffect_defaults_mutates_and_stays_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_specialeffect";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_specialeffect.prg";
        write_text(
            main_path,
            "oCheck = CREATEOBJECT('CheckBox')\n"
            "oCombo = CREATEOBJECT('ComboBox')\n"
            "oButton = CREATEOBJECT('CommandButton')\n"
            "oGroup = CREATEOBJECT('CommandGroup')\n"
            "oContainer = CREATEOBJECT('Container')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oOption = CREATEOBJECT('OptionButton')\n"
            "oOptions = CREATEOBJECT('OptionGroup')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oShape = CREATEOBJECT('Shape')\n"
            "oSpinner = CREATEOBJECT('Spinner')\n"
            "lAll = PEMSTATUS(oCheck, 'SpecialEffect', 1) AND PEMSTATUS(oCombo, 'SpecialEffect', 1) AND PEMSTATUS(oButton, 'SpecialEffect', 1) AND PEMSTATUS(oGroup, 'SpecialEffect', 1) AND PEMSTATUS(oContainer, 'SpecialEffect', 1) AND PEMSTATUS(oEdit, 'SpecialEffect', 1) AND PEMSTATUS(oList, 'SpecialEffect', 1) AND PEMSTATUS(oOption, 'SpecialEffect', 1) AND PEMSTATUS(oOptions, 'SpecialEffect', 1) AND PEMSTATUS(oPageFrame, 'SpecialEffect', 1) AND PEMSTATUS(oShape, 'SpecialEffect', 1) AND PEMSTATUS(oSpinner, 'SpecialEffect', 1)\n"
            "nCheckDefault = oCheck.SpecialEffect\n"
            "nContainerDefault = oContainer.SpecialEffect\n"
            "oCheck.SpecialEffect = 1.6\n"
            "nDirect = oCheck.SpecialEffect\n"
            "lSetPem = SETPEM(oCombo, 'SpecialEffect', 2)\n"
            "nSetPem = GETPEM(oCombo, 'SpecialEffect')\n"
            "lPutPem = PUTPEM(oShape, 'SpecialEffect', 9)\n"
            "nPutPem = GETPEM(oShape, 'SpecialEffect')\n"
            "oContainer.SpecialEffect = -1\n"
            "nNegative = oContainer.SpecialEffect\n"
            "lAddProperty = ADDPROPERTY(oList, 'SpecialEffect', 1)\n"
            "lRemoveProperty = REMOVEPROPERTY(oList, 'SpecialEffect')\n"
            "lMember = .F.\n"
            "nMembers = AMEMBERS(aMembers, oShape, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'SPECIALEFFECT'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedSpecialEffectShape')\n"
            "nDerived = oDerived.SpecialEffect\n"
            "RETURN\n"
            "DEFINE CLASS DerivedSpecialEffectShape AS Shape\n"
            "    SpecialEffect = 1\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native visual SpecialEffect script should complete: ") + state.message +
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

        check("lall", "true");
        check("ncheckdefault", "0");
        check("ncontainerdefault", "1");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "2");
        check("lputpem", "true");
        check("nputpem", "2");
        check("nnegative", "0");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("nderived", "1");

        fs::remove_all(temp_root, ignored);
    }
}
