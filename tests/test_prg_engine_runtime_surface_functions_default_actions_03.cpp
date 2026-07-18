#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_visual_geometry_members_reflect_and_resist_shadowing()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_geometry_members";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_geometry_members.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('GeometryForm')\n"
            "nFormLeftBefore = oForm.Left\n"
            "nFormTopBefore = oForm.Top\n"
            "nFormWidthBefore = oForm.Width\n"
            "nFormHeightBefore = oForm.Height\n"
            "xFormLeftGetPemBefore = GETPEM(oForm, 'Left')\n"
            "xFormWidthGetPemBefore = GETPEM(oForm, 'Width')\n"
            "lFormHasLeft = PEMSTATUS(oForm, 'Left', 1)\n"
            "lFormHasTop = PEMSTATUS(oForm, 'Top', 1)\n"
            "lFormHasWidth = PEMSTATUS(oForm, 'Width', 1)\n"
            "lFormHasHeight = PEMSTATUS(oForm, 'Height', 1)\n"
            "lFormLeftReadOnly = PEMSTATUS(oForm, 'Left', 5)\n"
            "lSetFormLeft = SETPEM(oForm, 'Left', 12)\n"
            "lSetFormHeight = SETPEM(oForm, 'Height', 34)\n"
            "nFormLeftAfter = oForm.Left\n"
            "nFormHeightAfter = oForm.Height\n"
            "lAddFormLeft = ADDPROPERTY(oForm, 'Left', 99)\n"
            "lRemoveFormWidth = REMOVEPROPERTY(oForm, 'Width')\n"
            "nFormWidthAfterRemove = oForm.Width\n"
            "nChildLeftBefore = oForm.cmdSave.Left\n"
            "nChildTopBefore = oForm.cmdSave.Top\n"
            "xChildHeightGetPemBefore = GETPEM(oForm.cmdSave, 'Height')\n"
            "lChildHasWidth = PEMSTATUS(oForm.cmdSave, 'Width', 1)\n"
            "lChildHeightReadOnly = PEMSTATUS(oForm.cmdSave, 'Height', 5)\n"
            "oForm.cmdSave.Width = 45\n"
            "lSetChildTop = SETPEM(oForm.cmdSave, 'Top', 67)\n"
            "nChildWidthAfter = oForm.cmdSave.Width\n"
            "nChildTopAfter = oForm.cmdSave.Top\n"
            "lAddChildHeight = ADDPROPERTY(oForm.cmdSave, 'Height', 88)\n"
            "lRemoveChildLeft = REMOVEPROPERTY(oForm.cmdSave, 'Left')\n"
            "nChildLeftAfterRemove = oForm.cmdSave.Left\n"
            "nPropCount = AMEMBERS(aProps, oForm, 1)\n"
            "lPropHasLeft = .F.\n"
            "lPropHasWidth = .F.\n"
            "FOR i = 1 TO nPropCount\n"
            "    IF UPPER(aProps[i]) == 'LEFT'\n"
            "        lPropHasLeft = .T.\n"
            "    ENDIF\n"
            "    IF UPPER(aProps[i]) == 'WIDTH'\n"
            "        lPropHasWidth = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS GeometryForm AS Form\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native geometry property script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("nformleftbefore", "0");
        check("nformtopbefore", "0");
        check("nformwidthbefore", "0");
        check("nformheightbefore", "0");
        check("xformleftgetpembefore", "0");
        check("xformwidthgetpembefore", "0");
        check("lformhasleft", "true");
        check("lformhastop", "true");
        check("lformhaswidth", "true");
        check("lformhasheight", "true");
        check("lformleftreadonly", "false");
        check("lsetformleft", "true");
        check("lsetformheight", "true");
        check("nformleftafter", "12");
        check("nformheightafter", "34");
        check("laddformleft", "false");
        check("lremoveformwidth", "false");
        check("nformwidthafterremove", "0");
        check("nchildleftbefore", "0");
        check("nchildtopbefore", "0");
        check("xchildheightgetpembefore", "0");
        check("lchildhaswidth", "true");
        check("lchildheightreadonly", "false");
        check("lsetchildtop", "true");
        check("nchildwidthafter", "45");
        check("nchildtopafter", "67");
        check("laddchildheight", "false");
        check("lremovechildleft", "false");
        check("nchildleftafterremove", "0");
        check("lprophasleft", "true");
        check("lprophaswidth", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_move_override_wins_over_builtin_geometry_updates()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_move_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_move_override.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MoveForm')\n"
            "oForm.Move(55, 66, 77, 88)\n"
            "lMoveRan = oForm.lMoveRan\n"
            "nLeft = oForm.Left\n"
            "nTop = oForm.Top\n"
            "nWidth = oForm.Width\n"
            "nHeight = oForm.Height\n"
            "RETURN\n"
            "DEFINE CLASS MoveForm AS Form\n"
            "    Left = 11\n"
            "    Top = 12\n"
            "    Width = 200\n"
            "    Height = 150\n"
            "    lMoveRan = .F.\n"
            "    PROCEDURE Move\n"
            "        LPARAMETERS tnLeft, tnTop, tnWidth, tnHeight\n"
            "        THIS.lMoveRan = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Move override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lmoveran", "true");
        check("nleft", "11");
        check("ntop", "12");
        check("nwidth", "200");
        check("nheight", "150");

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "MoveForm.Move";
        });
        expect(has_invoke_event,
               "native Move override should emit a prg.object.invoke event");

        const bool has_builtin_move_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.move";
        });
        expect(!has_builtin_move_event,
               "native Move override should not emit the builtin prg.object.move event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_show_hide_builtin_fallback_updates_visible_state()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_show_hide_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_show_hide_builtin.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.Hide\n"
            "oForm.cmdSave.Hide()\n"
            "oForm.cmdSave.Show\n"
            "lFormVisible = oForm.Visible\n"
            "lButtonVisible = oForm.cmdSave.Visible\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Show/Hide builtin fallback script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto form_visible = state.globals.find("lformvisible");
        expect(form_visible != state.globals.end(),
               "native Show/Hide builtin fallback script should preserve form visibility state");
        if (form_visible != state.globals.end())
        {
            expect(copperfin::runtime::format_value(form_visible->second) == "false",
                   "native Hide builtin fallback should set form Visible to false");
        }

        const auto button_visible = state.globals.find("lbuttonvisible");
        expect(button_visible != state.globals.end(),
               "native Show/Hide builtin fallback script should preserve child visibility state");
        if (button_visible != state.globals.end())
        {
            expect(copperfin::runtime::format_value(button_visible->second) == "true",
                   "native Show builtin fallback should set child Visible to true");
        }

        const std::size_t hide_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.hide";
            }));
        expect(hide_event_count == 2U,
               "native Show/Hide builtin fallback should emit one hide event per representative hide call");

        const std::size_t show_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.show";
            }));
        expect(show_event_count == 1U,
               "native Show/Hide builtin fallback should emit one show event per representative show call");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_show_builtin_fallback_accepts_modal_argument()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_show_modal_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_show_modal_builtin.prg";
        write_text(
            main_path,
            "#DEFINE SHOW_MODAL 1\n"
            "oForm = CREATEOBJECT('TaskEditForm')\n"
            "oForm.Visible = .F.\n"
            "oForm.Show(SHOW_MODAL)\n"
            "lVisible = oForm.Visible\n"
            "RETURN\n"
            "DEFINE CLASS TaskEditForm AS Form\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Show modal builtin fallback script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto visible = state.globals.find("lvisible");
        expect(visible != state.globals.end(),
               "native Show modal builtin fallback script should preserve visible state");
        if (visible != state.globals.end())
        {
            expect(copperfin::runtime::format_value(visible->second) == "true",
                   "native Show modal builtin fallback should accept SHOW_MODAL-style arguments and set Visible");
        }

        const std::size_t show_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto& event)
            {
                return event.category == "prg.object.show";
            }));
        expect(show_event_count == 1U,
               "native Show modal builtin fallback should emit one show event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_show_override_wins_over_builtin_visible_toggle()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_show_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_show_override.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.Show()\n"
            "lShowRan = oForm.lShowRan\n"
            "lVisible = oForm.Visible\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    Visible = .F.\n"
            "    lShowRan = .F.\n"
            "    PROCEDURE Show\n"
            "        THIS.lShowRan = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Show override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto show_ran = state.globals.find("lshowran");
        expect(show_ran != state.globals.end(),
               "native Show override script should preserve show override flag");
        if (show_ran != state.globals.end())
        {
            expect(copperfin::runtime::format_value(show_ran->second) == "true",
                   "native Show override should still invoke the class-defined Show method");
        }

        const auto visible = state.globals.find("lvisible");
        expect(visible != state.globals.end(),
               "native Show override script should preserve visible state");
        if (visible != state.globals.end())
        {
            expect(copperfin::runtime::format_value(visible->second) == "false",
                   "native Show override should not fall through to the builtin visible toggle");
        }

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "MainForm.Show";
        });
        expect(has_invoke_event,
               "native Show override should emit a prg.object.invoke event");

        const bool has_builtin_show_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.show";
        });
        expect(!has_builtin_show_event,
               "native Show override should not emit the builtin prg.object.show event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_show_override_preserves_modal_argument_without_builtin_fallthrough()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_show_modal_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_show_modal_override.prg";
        write_text(
            main_path,
            "#DEFINE SHOW_MODAL 1\n"
            "oForm = CREATEOBJECT('TaskEditForm')\n"
            "oForm.Show(SHOW_MODAL)\n"
            "lShowRan = oForm.lShowRan\n"
            "nShowArgs = oForm.nShowArgs\n"
            "nShowStyle = oForm.nShowStyle\n"
            "lVisible = oForm.Visible\n"
            "RETURN\n"
            "DEFINE CLASS TaskEditForm AS Form\n"
            "    Visible = .F.\n"
            "    lShowRan = .F.\n"
            "    nShowArgs = 0\n"
            "    nShowStyle = 0\n"
            "    PROCEDURE Show\n"
            "        LPARAMETERS tnStyle\n"
            "        THIS.lShowRan = .T.\n"
            "        THIS.nShowArgs = PCOUNT()\n"
            "        IF PCOUNT() >= 1\n"
            "            THIS.nShowStyle = tnStyle\n"
            "        ENDIF\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Show modal override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto check = [&](const std::string& name, const std::string& expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " variable not found");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lshowran", "true");
        check("nshowargs", "1");
        check("nshowstyle", "1");
        check("lvisible", "false");

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "TaskEditForm.Show";
        });
        expect(has_invoke_event,
               "native Show modal override coverage should emit a prg.object.invoke event");

        const bool has_builtin_show_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "prg.object.show";
        });
        expect(!has_builtin_show_event,
               "native Show modal override coverage should not emit the builtin prg.object.show event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_setfocus_builtin_fallback_updates_owner_activecontrol()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_setfocus_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_setfocus_builtin.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.txtName.SetFocus\n"
            "lHasActiveAfterText = PEMSTATUS(oForm, 'ActiveControl', 1)\n"
            "cActiveAfterText = oForm.ActiveControl.BaseClass\n"
            "oForm.cmdSave.SetFocus()\n"
            "cActiveAfterButton = oForm.ActiveControl.BaseClass\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT txtName AS TextBox\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SetFocus builtin fallback script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto has_active_after_text = state.globals.find("lhasactiveaftertext");
        expect(has_active_after_text != state.globals.end(),
               "native SetFocus builtin fallback script should preserve ActiveControl presence");
        if (has_active_after_text != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_active_after_text->second) == "true",
                   "native SetFocus builtin fallback should materialize ActiveControl on the owner form");
        }

        const auto active_after_text = state.globals.find("cactiveaftertext");
        expect(active_after_text != state.globals.end(),
               "native SetFocus builtin fallback script should preserve first focus target metadata");
        if (active_after_text != state.globals.end())
        {
            expect(copperfin::runtime::format_value(active_after_text->second) == "TextBox",
                   "native SetFocus builtin fallback should set ActiveControl to the focused text box");
        }

        const auto active_after_button = state.globals.find("cactiveafterbutton");
        expect(active_after_button != state.globals.end(),
               "native SetFocus builtin fallback script should preserve second focus target metadata");
        if (active_after_button != state.globals.end())
        {
            expect(copperfin::runtime::format_value(active_after_button->second) == "CommandButton",
                   "native SetFocus builtin fallback should update ActiveControl to the focused button");
        }

        const std::size_t setfocus_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.setfocus";
            }));
        expect(setfocus_event_count == 2U,
               "native SetFocus builtin fallback should emit one setfocus event per representative focus call");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_setfocus_override_wins_over_builtin_activecontrol_toggle()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_setfocus_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_setfocus_override.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.txtName.SetFocus()\n"
            "lSetFocusRan = oForm.txtName.lSetFocusRan\n"
            "lHasActiveControl = PEMSTATUS(oForm, 'ActiveControl', 1)\n"
            "RETURN\n"
            "DEFINE CLASS FocusableBox AS TextBox\n"
            "    lSetFocusRan = .F.\n"
            "    PROCEDURE SetFocus\n"
            "        THIS.lSetFocusRan = .T.\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT txtName AS FocusableBox\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SetFocus override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto setfocus_ran = state.globals.find("lsetfocusran");
        expect(setfocus_ran != state.globals.end(),
               "native SetFocus override script should preserve override flag");
        if (setfocus_ran != state.globals.end())
        {
            expect(copperfin::runtime::format_value(setfocus_ran->second) == "true",
                   "native SetFocus override should still invoke the class-defined SetFocus method");
        }

        const auto has_active_control = state.globals.find("lhasactivecontrol");
        expect(has_active_control != state.globals.end(),
               "native SetFocus override script should preserve ActiveControl presence state");
        if (has_active_control != state.globals.end())
        {
            expect(copperfin::runtime::format_value(has_active_control->second) == "false",
                   "native SetFocus override should not fall through to the builtin ActiveControl update");
        }

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "FocusableBox.SetFocus";
        });
        expect(has_invoke_event,
               "native SetFocus override should emit a prg.object.invoke event");

        const bool has_builtin_setfocus_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.setfocus";
        });
        expect(!has_builtin_setfocus_event,
               "native SetFocus override should not emit the builtin prg.object.setfocus event");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_application_activeform_aliases_track_representative_native_form()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_activeform_aliases";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_activeform_aliases.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.Show()\n"
            "oForm.oToolbar.cmdRed.SetFocus()\n"
            "oVfpActive = _VFP.ActiveForm\n"
            "oScreenActive = _SCREEN.ActiveForm\n"
            "cVfpActiveBaseClass = oVfpActive.BaseClass\n"
            "cScreenActiveCaption = oScreenActive.Caption\n"
            "cDirectActiveCaption = _VFP.ActiveForm.Caption\n"
            "lNestedEnabled = _VFP.ActiveForm.oToolbar.cmdRed.Enabled\n"
            "cActiveControlBaseClass = _SCREEN.ActiveForm.ActiveControl.BaseClass\n"
            "_SCREEN.ActiveForm.Hide()\n"
            "lVisibleAfterHide = oForm.Visible\n"
            "RETURN\n"
            "DEFINE CLASS MainToolbar AS Toolbar\n"
            "    ADD OBJECT cmdRed AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    Caption = 'ColorHost'\n"
            "    ADD OBJECT oToolbar AS MainToolbar\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime ActiveForm alias script should complete: ") + state.message +
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

        check("cvfpactivebaseclass", "Form");
        check("cscreenactivecaption", "ColorHost");
        check("cdirectactivecaption", "ColorHost");
        check("lnestedenabled", "true");
        check("cactivecontrolbaseclass", "CommandButton");
        check("lvisibleafterhide", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_application_forms_aliases_track_representative_window_collection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_forms_aliases";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_forms_aliases.prg";
        write_text(
            main_path,
            "oFirst = CREATEOBJECT('FirstForm')\n"
            "oToolbar = CREATEOBJECT('MainToolbar')\n"
            "oSecond = CREATEOBJECT('SecondForm')\n"
            "oFirst.Show()\n"
            "oToolbar.Show()\n"
            "oSecond.Show()\n"
            "nScreenFormCount = _SCREEN.FormCount\n"
            "nVfpFormCount = _VFP.FormCount\n"
            "nScreenFormsCount = _SCREEN.Forms.Count\n"
            "nVfpFormsCount = _VFP.Forms.Count\n"
            "cScreenFirstCaption = _SCREEN.Forms(1).Caption\n"
            "cScreenSecondBaseClass = _SCREEN.Forms(2).BaseClass\n"
            "cScreenThirdCaption = _SCREEN.Forms[3].Caption\n"
            "cVfpFirstCaption = _VFP.Forms[1].Caption\n"
            "cForEachOrder = ''\n"
            "FOR EACH oWindow IN _SCREEN.Forms FOXOBJECT\n"
            "    cLabel = oWindow.BaseClass + ':' + oWindow.Caption\n"
            "    cForEachOrder = IIF(EMPTY(cForEachOrder), cLabel, cForEachOrder + '|' + cLabel)\n"
            "ENDFOR\n"
            "RETURN\n"
            "DEFINE CLASS FirstForm AS Form\n"
            "    Caption = 'First'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainToolbar AS Toolbar\n"
            "    Caption = 'Tools'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SecondForm AS Form\n"
            "    Caption = 'Second'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime Forms alias script should complete: ") + state.message +
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

        check("nscreenformcount", "3");
        check("nvfpformcount", "3");
        check("nscreenformscount", "3");
        check("nvfpformscount", "3");
        check("cscreenfirstcaption", "Second");
        check("cscreensecondbaseclass", "Toolbar");
        check("cscreenthirdcaption", "First");
        check("cvfpfirstcaption", "Second");
        check("cforeachorder", "Form:Second|Toolbar:Tools|Form:First");

        fs::remove_all(temp_root, ignored);
    }

    void test_runtime_application_forms_aliases_invoke_builtin_and_custom_methods()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_forms_alias_method_invocation";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "runtime_forms_alias_method_invocation.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('AliasForm')\n"
            "oForm.Show()\n"
            "oForm.SetRefTable('direct')\n"
            "cDirectSetId = oForm.cSetId\n"
            "_SCREEN.Forms(1).Hide()\n"
            "_VFP.Forms(1).Show()\n"
            "_SCREEN.Forms(1).SetRefTable('foxref')\n"
            "lVisibleAfterShow = oForm.Visible\n"
            "cSetId = oForm.cSetId\n"
            "RETURN\n"
            "DEFINE CLASS AliasForm AS Form\n"
            "    cSetId = ''\n"
            "    PROCEDURE SetRefTable(tcSetId)\n"
            "        THIS.cSetId = tcSetId\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("runtime Forms alias method invocation script should complete: ") + state.message +
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

        check("cdirectsetid", "direct");
        check("lvisibleaftershow", "true");
        check("csetid", "foxref");

        const auto alias_form_it = std::find_if(state.ole_objects.begin(), state.ole_objects.end(), [](const auto &object)
        {
            return object.prog_id == "AliasForm";
        });
        expect(alias_form_it != state.ole_objects.end(),
               "Forms alias method invocation script should retain the AliasForm runtime object");
        if (alias_form_it != state.ole_objects.end())
        {
            expect(alias_form_it->source == main_path.string(),
                   "Forms alias method invocation script should preserve AliasForm source metadata");
            expect(std::find(alias_form_it->methods.begin(), alias_form_it->methods.end(), "SetRefTable") != alias_form_it->methods.end(),
                   "Forms alias method invocation script should expose SetRefTable in the AliasForm method table");
        }

        const bool has_hide_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.hide" &&
                   event.detail == "AliasForm.Hide";
        });
        expect(has_hide_event,
               "Forms alias hide call should emit the builtin hide event against the selected form");

        const bool has_show_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.show" &&
                   event.detail == "AliasForm.Show";
        });
        expect(has_show_event,
               "Forms alias show call should emit the builtin show event against the selected form");

        const bool has_setref_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "AliasForm.SetRefTable";
        });
        expect(has_setref_invoke_event,
               "Forms alias custom method call should emit a prg.object.invoke event");

        fs::remove_all(temp_root, ignored);
    }

}
