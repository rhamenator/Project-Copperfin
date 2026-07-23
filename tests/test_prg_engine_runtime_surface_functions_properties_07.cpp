#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_tag_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_visual_tag";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_visual_tag.prg";
        write_text(
            main_path,
            "oPlain = CREATEOBJECT('TextBox')\n"
            "lHas = PEMSTATUS(oPlain, 'Tag', 1)\n"
            "lReadOnly = PEMSTATUS(oPlain, 'Tag', 5)\n"
            "cBefore = oPlain.Tag\n"
            "xBefore = GETPEM(oPlain, 'Tag')\n"
            "oPlain.Tag = 'plain tag'\n"
            "cAfterDirect = oPlain.Tag\n"
            "lSetPem = SETPEM(oPlain, 'Tag', 'setpem tag')\n"
            "cAfterSetPem = GETPEM(oPlain, 'Tag')\n"
            "lAddProperty = ADDPROPERTY(oPlain, 'Tag', 'shadow')\n"
            "lRemoveProperty = REMOVEPROPERTY(oPlain, 'Tag')\n"
            "nMembers = AMEMBERS(aMembers, oPlain, 1)\n"
            "lMember = .F.\n"
            "FOR i = 1 TO nMembers\n"
            "    IF UPPER(aMembers[i]) == 'TAG'\n"
            "        lMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oForm = CREATEOBJECT('TagForm')\n"
            "cChildBefore = oForm.txtValue.Tag\n"
            "cChildRead = oForm.cmdProbe.ReadTag()\n"
            "oForm.cmdProbe.SetTag()\n"
            "cChildAfter = oForm.txtValue.Tag\n"
            "oDerived = CREATEOBJECT('DerivedTagText')\n"
            "cDerived = oDerived.Tag\n"
            "RETURN\n"
            "DEFINE CLASS TagProbe AS CommandButton\n"
            "    FUNCTION ReadTag\n"
            "        RETURN THISFORM.txtValue.Tag\n"
            "    ENDFUNC\n"
            "    PROCEDURE SetTag\n"
            "        THISFORM.txtValue.Tag = 'child tag'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS TagForm AS Form\n"
            "    ADD OBJECT txtValue AS TextBox WITH Tag = 'initial tag'\n"
            "    ADD OBJECT cmdProbe AS TagProbe\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DerivedTagText AS TextBox\n"
            "    Tag = 'derived tag'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Tag script should complete: ") + state.message +
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

        check("lhas", "true");
        check("lreadonly", "false");
        check("cbefore", "");
        check("xbefore", "");
        check("cafterdirect", "plain tag");
        check("lsetpem", "true");
        check("caftersetpem", "setpem tag");
        check("laddproperty", "false");
        check("lremoveproperty", "false");
        check("lmember", "true");
        check("cchildbefore", "initial tag");
        check("cchildread", "initial tag");
        check("cchildafter", "child tag");
        check("cderived", "derived tag");

        fs::remove_all(temp_root, ignored);
    }
}
