#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_mousepointer_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_mousepointer";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_mousepointer.prg";
        write_text(
            main_path,
            "oButton = CREATEOBJECT('CommandButton')\n"
            "lHas = PEMSTATUS(oButton, 'MousePointer', 1)\n"
            "lReadOnly = PEMSTATUS(oButton, 'MousePointer', 5)\n"
            "nDefault = oButton.MousePointer\n"
            "oButton.MousePointer = 2\n"
            "nDirect = GETPEM(oButton, 'MousePointer')\n"
            "lSetPem = SETPEM(oButton, 'MousePointer', 4)\n"
            "nSetPem = GETPEM(oButton, 'MousePointer')\n"
            "lPutPem = PUTPEM(oButton, 'MousePointer', 6)\n"
            "nPutPem = GETPEM(oButton, 'MousePointer')\n"
            "oButton.MousePointer = -1\n"
            "nNormalized = oButton.MousePointer\n"
            "lPropHas = .F.\n"
            "nMembers = AMEMBERS(aMembers, oButton, 1)\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'MOUSEPOINTER'\n"
            "        lPropHas = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oDerived = CREATEOBJECT('DerivedMousePointer')\n"
            "nDerived = oDerived.MousePointer\n"
            "RETURN\n"
            "DEFINE CLASS DerivedMousePointer AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.MousePointer = 7\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native MousePointer script should complete: ") + state.message +
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
        check("ndefault", "0");
        check("ndirect", "2");
        check("lsetpem", "true");
        check("nsetpem", "4");
        check("lputpem", "true");
        check("nputpem", "6");
        check("nnormalized", "0");
        check("lprophas", "true");
        check("nderived", "7");
        expect(state.ole_objects.size() == 2U,
               "native MousePointer coverage should register the base and derived controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_visual_caption_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_caption";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_caption.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('CaptionForm')\n"
            "oPage = CREATEOBJECT('Page')\n"
            "oPageFrame = CREATEOBJECT('PageFrame')\n"
            "oGroup = CREATEOBJECT('CommandGroup')\n"
            "oOptionGroup = CREATEOBJECT('OptionGroup')\n"
            "lFormHas = PEMSTATUS(oForm, 'Caption', 1)\n"
            "lTextHas = PEMSTATUS(oForm.txtValue, 'Caption', 1)\n"
            "lPageHas = PEMSTATUS(oPage, 'Caption', 1)\n"
            "lPageFrameHas = PEMSTATUS(oPageFrame, 'Caption', 1)\n"
            "lGroupHas = PEMSTATUS(oGroup, 'Caption', 1)\n"
            "lOptionGroupHas = PEMSTATUS(oOptionGroup, 'Caption', 1)\n"
            "cBefore = oForm.Caption\n"
            "xBefore = GETPEM(oForm, 'Caption')\n"
            "oForm.Caption = 'Main form'\n"
            "cAfterDirect = oForm.Caption\n"
            "lSetPem = SETPEM(oForm, 'Caption', 'Updated form')\n"
            "cAfterSetPem = GETPEM(oForm, 'Caption')\n"
            "lAddProperty = ADDPROPERTY(oForm, 'Caption', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oForm, 'Caption')\n"
            "nMembers = AMEMBERS(aMembers, oForm, 1)\n"
            "lMember = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'CAPTION'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "cChildBefore = oForm.cmdSave.Caption\n"
            "cChildRead = oForm.cmdSave.ReadCaption()\n"
            "oForm.cmdSave.SetCaption()\n"
            "cChildAfter = oForm.cmdSave.Caption\n"
            "cDerived = CREATEOBJECT('DerivedCaptionButton').Caption\n"
            "RETURN\n"
            "DEFINE CLASS CaptionProbe AS CommandButton\n"
            "    FUNCTION ReadCaption\n"
            "        RETURN THISFORM.cmdSave.Caption\n"
            "    ENDFUNC\n"
            "    PROCEDURE SetCaption\n"
            "        THISFORM.cmdSave.Caption = 'child updated'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS CaptionForm AS Form\n"
            "    Caption = 'Initial form'\n"
            "    ADD OBJECT txtValue AS TextBox\n"
            "    ADD OBJECT cmdSave AS CaptionProbe WITH Caption = 'Save'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedCaptionButton AS CommandButton\n"
            "    Caption = 'Derived'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Caption script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
        };

        check("lformhas", "true");
        check("ltexthas", "false");
        check("lpagehas", "true");
        check("lpageframehas", "true");
        check("lgrouphas", "true");
        check("loptiongrouphas", "true");
        check("cbefore", "Initial form");
        check("xbefore", "Initial form");
        check("cafterdirect", "Main form");
        check("lsetpem", "true");
        check("caftersetpem", "Updated form");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cchildbefore", "Save");
        check("cchildread", "Save");
        check("cchildafter", "child updated");
        check("cderived", "Derived");

        fs::remove_all(temp_root, ignored);
    }
}
