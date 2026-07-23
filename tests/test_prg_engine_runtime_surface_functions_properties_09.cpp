#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_text_alignment_defaults_mutate_and_stay_builtin()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_text_alignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_text_alignment.prg";
        write_text(
            main_path,
            "oText = CREATEOBJECT('TextBox')\n"
            "oEdit = CREATEOBJECT('EditBox')\n"
            "oLabel = CREATEOBJECT('Label')\n"
            "lTextHas = PEMSTATUS(oText, 'Alignment', 1)\n"
            "lTextReadOnly = PEMSTATUS(oText, 'Alignment', 5)\n"
            "nTextDefault = oText.Alignment\n"
            "oText.Alignment = 4\n"
            "nTextDirect = oText.Alignment\n"
            "lTextSetPem = SETPEM(oText, 'Alignment', -1)\n"
            "nTextSetPem = GETPEM(oText, 'Alignment')\n"
            "lTextPutPem = PUTPEM(oText, 'Alignment', '1.8')\n"
            "nTextPutPem = GETPEM(oText, 'Alignment')\n"
            "lTextAddProperty = ADDPROPERTY(oText, 'Alignment', 0)\n"
            "lTextRemoveProperty = REMOVEPROPERTY(oText, 'Alignment')\n"
            "lTextMember = .F.\n"
            "nTextMembers = AMEMBERS(aTextMembers, oText, 1)\n"
            "FOR i = 1 TO nTextMembers\n"
            "    IF UPPER(aTextMembers[i]) == 'ALIGNMENT'\n"
            "        lTextMember = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lEditHas = PEMSTATUS(oEdit, 'Alignment', 1)\n"
            "lLabelHas = PEMSTATUS(oLabel, 'Alignment', 1)\n"
            "oEdit.Alignment = 1\n"
            "oLabel.Alignment = 2\n"
            "nEditAlignment = oEdit.Alignment\n"
            "nLabelAlignment = oLabel.Alignment\n"
            "oDerived = CREATEOBJECT('DerivedAlignmentText')\n"
            "nDerivedAlignment = oDerived.Alignment\n"
            "RETURN\n"
            "DEFINE CLASS DerivedAlignmentText AS TextBox\n"
            "    PROCEDURE Init\n"
            "        THIS.Alignment = 2\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Alignment script should complete: ") + state.message +
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

        check("ltexthas", "true");
        check("ltextreadonly", "false");
        check("ntextdefault", "0");
        check("ntextdirect", "2");
        check("ltextsetpem", "true");
        check("ntextsetpem", "0");
        check("ltextputpem", "true");
        check("ntextputpem", "2");
        check("ltextaddproperty", "false");
        check("ltextremoveproperty", "false");
        check("ltextmember", "true");
        check("ledithas", "true");
        check("llabelhas", "true");
        check("neditalignment", "1");
        check("nlabelalignment", "2");
        check("nderivedalignment", "2");
        expect(state.ole_objects.size() == 4U,
               "native Alignment coverage should register all three controls and the derived control");

        fs::remove_all(temp_root, ignored);
    }
}
