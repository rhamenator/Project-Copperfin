#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_runtime_application_caption_aliases_track_representative_caption()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_caption_aliases";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_caption_aliases.prg";
        write_text(
            main_path,
            "cScreenDefault = _SCREEN.Caption\n"
            "cVfpDefault = _VFP.Caption\n"
            "_SCREEN.Caption = 'Copperfin'\n"
            "cScreenAfterScreenSet = _SCREEN.Caption\n"
            "cVfpAfterScreenSet = _VFP.Caption\n"
            "_VFP.Caption = 'Copperfin Runtime'\n"
            "cScreenAfterVfpSet = _SCREEN.Caption\n"
            "cVfpAfterVfpSet = _VFP.Caption\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime caption alias script should complete: ") + state.message +
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

        check("cscreendefault", "Microsoft Visual FoxPro");
        check("cvfpdefault", "Microsoft Visual FoxPro");
        check("cscreenafterscreenset", "Copperfin");
        check("cvfpafterscreenset", "Copperfin");
        check("cscreenaftervfpset", "Copperfin Runtime");
        check("cvfpaftervfpset", "Copperfin Runtime");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_application_windowstate_aliases_track_representative_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_windowstate_aliases";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_windowstate_aliases.prg";
        write_text(
            main_path,
            "nScreenDefault = _SCREEN.WindowState\n"
            "nVfpDefault = _VFP.WindowState\n"
            "_SCREEN.WindowState = 1\n"
            "nScreenAfterScreenSet = _SCREEN.WindowState\n"
            "nVfpAfterScreenSet = _VFP.WindowState\n"
            "_VFP.WindowState = 2\n"
            "nScreenAfterVfpSet = _SCREEN.WindowState\n"
            "nVfpAfterVfpSet = _VFP.WindowState\n"
            "RETURN\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime windowstate alias script should complete: ") + state.message +
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

        check("nscreendefault", "0");
        check("nvfpdefault", "0");
        check("nscreenafterscreenset", "1");
        check("nvfpafterscreenset", "1");
        check("nscreenaftervfpset", "2");
        check("nvfpaftervfpset", "2");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_thisform_command_releases_owner_form()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_release_thisform";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_thisform.prg";
        write_text(
            main_path,
            "PUBLIC lMethodContinued\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lReleaseReturned = oForm.cmdClose.CloseOwner()\n"
            "RETURN\n"
            "DEFINE CLASS CloseButton AS CommandButton\n"
            "    FUNCTION CloseOwner\n"
            "        RELEASE THISFORM\n"
            "        lMethodContinued = .T.\n"
            "        RETURN .T.\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cmdClose AS CloseButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RELEASE THISFORM script should complete: ") + state.message +
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

        check("lreleasereturned", "true");
        check("lmethodcontinued", "true");

        expect(state.ole_objects.empty(),
               "native RELEASE THISFORM should tear down the owner form and its child objects");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_thisformset_command_releases_owner_alias()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_release_thisformset";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_thisformset.prg";
        write_text(
            main_path,
            "PUBLIC lMethodContinued\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lReleaseReturned = oForm.cmdClose.CloseOwner()\n"
            "RETURN\n"
            "DEFINE CLASS CloseButton AS CommandButton\n"
            "    FUNCTION CloseOwner\n"
            "        RELEASE THISFORMSET\n"
            "        lMethodContinued = .T.\n"
            "        RETURN .T.\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cmdClose AS CloseButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native RELEASE THISFORMSET script should complete: ") + state.message +
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

        check("lreleasereturned", "true");
        check("lmethodcontinued", "true");

        expect(state.ole_objects.empty(),
               "native RELEASE THISFORMSET should tear down the representative owner alias and its child objects");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_override_runs_before_builtin_release_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_release_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_override.prg";
        write_text(
            main_path,
            "nReleaseProcCount = 0\n"
            "nDestroyCount = 0\n"
            "oWidget = CREATEOBJECT('Widget')\n"
            "lReleased = oWidget.Release()\n"
            "nReleaseProcCountAfter = nReleaseProcCount\n"
            "nDestroyCountAfter = nDestroyCount\n"
            "lHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "xCaptionAfterRelease = GETPEM(oWidget, 'Caption')\n"
            "RETURN\n"
            "DEFINE CLASS Widget AS Custom\n"
            "    Caption = 'Widget'\n"
            "    PROCEDURE Release\n"
            "        nReleaseProcCount = nReleaseProcCount + 1\n"
            "        RETURN 'override'\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        nDestroyCount = nDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release override script should complete: ") + state.message +
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

        check("lreleased", "true");
        check("nreleaseproccountafter", "1");
        check("ndestroycountafter", "1");
        check("lhascaptionafterrelease", "false");

        const auto caption_after_release = state.globals.find("xcaptionafterrelease");
        expect(caption_after_release != state.globals.end() &&
                   caption_after_release->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native Release override should still fall through to the builtin release path");

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "Widget.Release";
        });
        expect(has_invoke_event,
               "native Release override should invoke the class-defined Release method before builtin release");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "Widget";
        });
        expect(has_release_event,
               "native Release override should still emit the builtin release event when NODEFAULT is absent");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_release_override_nodefault_suppresses_builtin_release_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_release_override_nodefault";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_release_override_nodefault.prg";
        write_text(
            main_path,
            "nReleaseProcCount = 0\n"
            "nDestroyCount = 0\n"
            "oWidget = CREATEOBJECT('Widget')\n"
            "lReleased = oWidget.Release()\n"
            "nReleaseProcCountAfter = nReleaseProcCount\n"
            "nDestroyCountAfter = nDestroyCount\n"
            "lHasCaptionAfterRelease = PEMSTATUS(oWidget, 'Caption', 1)\n"
            "cCaptionAfterRelease = oWidget.Caption\n"
            "RETURN\n"
            "DEFINE CLASS Widget AS Custom\n"
            "    Caption = 'Widget'\n"
            "    PROCEDURE Release\n"
            "        nReleaseProcCount = nReleaseProcCount + 1\n"
            "        NODEFAULT\n"
            "        RETURN .F.\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        nDestroyCount = nDestroyCount + 1\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Release override NODEFAULT script should complete: ") + state.message +
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

        check("lreleased", "false");
        check("nreleaseproccountafter", "1");
        check("ndestroycountafter", "0");
        check("lhascaptionafterrelease", "true");
        check("ccaptionafterrelease", "Widget");

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "Widget.Release";
        });
        expect(has_invoke_event,
               "native Release override NODEFAULT should still invoke the class-defined Release method");

        const bool has_release_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.release" &&
                   event.detail == "Widget";
        });
        expect(!has_release_event,
               "native Release override NODEFAULT should suppress the builtin release event");

        const bool has_destroy_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.destroy" &&
                   event.detail == "Widget.Destroy";
        });
        expect(!has_destroy_event,
               "native Release override NODEFAULT should keep the object alive");

        expect(state.ole_objects.size() == 1U,
               "native Release override NODEFAULT should preserve the live native object");
        if (state.ole_objects.size() == 1U)
        {
            expect(state.ole_objects[0].prog_id == "Widget",
                   "native Release override NODEFAULT should keep the original widget registered");
        }

        fs::remove_all(temp_root, ignored);
    }

}
