#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_wvisible_tracks_modeled_vfp_windows()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_wvisible";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "wvisible.prg";
        write_text(
            main_path,
            "lMainVisible = WVISIBLE('')\n"
            "lMainExists = WEXIST('')\n"
            "oForm = CREATEOBJECT('VisibilityForm')\n"
            "lInitialVisible = WVISIBLE('visibilityform')\n"
            "lInitialExists = WEXIST('visibilityform')\n"
            "oForm.Hide()\n"
            "lHidden = WVISIBLE('VisibilityForm')\n"
            "lHiddenExists = WEXIST('VisibilityForm')\n"
            "oForm.Show()\n"
            "lShown = WVISIBLE('VisibilityForm')\n"
            "oForm.Visible = .F.\n"
            "lDirectHidden = WVISIBLE('VisibilityForm')\n"
            "oForm.Visible = .T.\n"
            "lDirectShown = WVISIBLE('VisibilityForm')\n"
            "oPlain = CREATEOBJECT('PlainObject')\n"
            "lNonVisual = WVISIBLE('PlainObject')\n"
            "lNonVisualExists = WEXIST('PlainObject')\n"
            "lUnknown = WVISIBLE('missing-window')\n"
            "lUnknownExists = WEXIST('missing-window')\n"
            "RETURN\n"
            "DEFINE CLASS VisibilityForm AS Form\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PlainObject AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "WVISIBLE modeled-window script should complete: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            if (it == state.globals.end())
            {
                expect(false, name + " variable not found");
                return;
            }
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        };

        check("lmainvisible", "true");
        check("lmainexists", "true");
        check("linitialvisible", "true");
        check("linitialexists", "true");
        check("lhidden", "false");
        check("lhiddenexists", "true");
        check("lshown", "true");
        check("ldirecthidden", "false");
        check("ldirectshown", "true");
        check("lnonvisual", "false");
        check("lnonvisualexists", "false");
        check("lunknown", "false");
        check("lunknownexists", "false");

        fs::remove_all(temp_root, ignored);
    }
}
