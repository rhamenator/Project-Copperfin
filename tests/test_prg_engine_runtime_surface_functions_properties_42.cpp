#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_runtime_application_righttoleft_aliases_track_representative_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_righttoleft_aliases";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_righttoleft_aliases.prg";
        write_text(
            main_path,
            "lScreenDefault = _SCREEN.RightToLeft\n"
            "lVfpDefault = _VFP.RightToLeft\n"
            "_SCREEN.RightToLeft = .F.\n"
            "lScreenAfterScreenSet = _SCREEN.RightToLeft\n"
            "lVfpAfterScreenSet = _VFP.RightToLeft\n"
            "_VFP.RightToLeft = .T.\n"
            "lScreenAfterVfpSet = _SCREEN.RightToLeft\n"
            "lVfpAfterVfpSet = _VFP.RightToLeft\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(
                make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime RightToLeft alias script should complete: ") + state.message +
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
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        };

        check("lscreendefault", "true");
        check("lvfpdefault", "true");
        check("lscreenafterscreenset", "false");
        check("lvfpafterscreenset", "false");
        check("lscreenaftervfpset", "true");
        check("lvfpaftervfpset", "true");

        fs::remove_all(temp_root, ignored);
    }
}
