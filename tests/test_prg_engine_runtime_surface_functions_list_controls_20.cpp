#include "test_prg_engine_runtime_surface_functions_support.h"
#include "test_prg_engine_runtime_surface_functions_tests.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_list_controls_popup_rowsource_materializes_static_bars()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
            "copperfin_native_list_control_popup_rowsource";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "popup_rowsource.prg";
        write_text(
            main_path,
            "DEFINE POPUP choices\n"
            "DEFINE BAR 20 OF choices PROMPT 'Twenty'\n"
            "DEFINE BAR 2 OF choices PROMPT 'Two'\n"
            "DEFINE BAR 3 OF choices PROMPT '\\<Three'\n"
            "DEFINE BAR 4 OF choices PROMPT '\\Disabled'\n"
            "DEFINE BAR 5 OF choices PROMPT '\\-'\n"
            "DEFINE POPUP alternate\n"
            "DEFINE BAR 1 OF alternate PROMPT 'Alternate'\n"
            "oList = CREATEOBJECT('ListBox')\n"
            "oList.RowSourceType = 9\n"
            "oList.RowSource = 'choices'\n"
            "oList.Requery()\n"
            "nInitialCount = oList.ListCount\n"
            "cFirst = oList.List(1)\n"
            "cSecond = oList.List(2)\n"
            "cThird = oList.List(5)\n"
            "nPopupCount = CNTBAR('choices')\n"
            "nMissingPopupCount = CNTBAR('missing_popup')\n"
            "nFirstBar = GETBAR('choices', 1)\n"
            "nSecondBar = GETBAR('choices', 2)\n"
            "nFifthBar = GETBAR('choices', 5)\n"
            "nMissingBar = GETBAR('choices', 99)\n"
            "cPromptByPosition = PRMBAR('choices', GETBAR('choices', 5))\n"
            "cPromptTwo = PRMBAR('choices', 2)\n"
            "cPromptThree = PRMBAR('choices', 3)\n"
            "cPromptDisabled = PRMBAR('choices', 4)\n"
            "cPromptSeparator = PRMBAR('choices', 5)\n"
            "cPromptTwenty = PRMBAR('choices', 20)\n"
            "cPromptMissing = PRMBAR('choices', 99)\n"
            "SET SKIP OF BAR 4 OF choices .T.\n"
            "lDisabled = SKPBAR('choices', 4)\n"
            "lEnabled = SKPBAR('choices', 2)\n"
            "lMissingSkip = SKPBAR('choices', 99)\n"
            "SET SKIP OF BAR 4 OF choices .F.\n"
            "lEnabledAfterSet = SKPBAR('choices', 4)\n"
            "SET SKIP OF BAR 4 OF choices .T.\n"
            "SET MARK OF BAR 4 OF choices TO .T.\n"
            "lMarked = MRKBAR('choices', 4)\n"
            "lUnmarked = MRKBAR('choices', 2)\n"
            "lMissingMark = MRKBAR('choices', 99)\n"
            "SET MARK OF BAR 4 OF choices TO .F.\n"
            "lUnmarkedAfterSet = MRKBAR('choices', 4)\n"
            "SET MARK OF BAR 4 OF choices TO .T.\n"
            "DEFINE POPUP choices\n"
            "lRedefinedSkip = SKPBAR('choices', 4)\n"
            "lRedefinedMark = MRKBAR('choices', 4)\n"
            "DEFINE BAR 1 OF choices PROMPT 'Replacement'\n"
            "nReplacementPopupCount = CNTBAR('choices')\n"
            "oList.Requery()\n"
            "nReplacementCount = oList.ListCount\n"
            "cReplacement = oList.List(1)\n"
            "lcPopup = 'choices'\n"
            "lcPopupHolder = 'lcPopup'\n"
            "cMacroEval = &lcPopupHolder\n"
            "oMacro = CREATEOBJECT('ListBox')\n"
            "oMacro.RowSourceType = 9\n"
            "oMacro.RowSource = '&lcPopupHolder'\n"
            "nMacroType = oMacro.RowSourceType\n"
            "cMacroRaw = oMacro.RowSource\n"
            "oMacro.Requery()\n"
            "cMacroFirst = oMacro.List(1)\n"
            "lcPopup = 'alternate'\n"
            "oMacro.Requery()\n"
            "cMacroChanged = oMacro.List(1)\n"
            "RELEASE POPUP choices\n"
            "oList.Requery()\n"
            "nReleasedCount = oList.ListCount\n"
            "oMissing = CREATEOBJECT('ComboBox')\n"
            "oMissing.RowSourceType = 9\n"
            "oMissing.RowSource = 'missing_popup'\n"
            "oMissing.Requery()\n"
            "nMissingCount = oMissing.ListCount\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root))
            .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("static popup RowSource script should complete: ") + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto found = state.globals.find(name);
            expect(found != state.globals.end(), name + " should be captured");
            if (found != state.globals.end())
            {
                expect(copperfin::runtime::format_value(found->second) == expected,
                       name + " expected '" + expected + "' got '" +
                           copperfin::runtime::format_value(found->second) + "'");
            }
        };

        check("ninitialcount", "5");
        check("cfirst", "Two");
        check("csecond", "\\<Three");
        check("cthird", "Twenty");
        check("npopupcount", "5");
        check("nmissingpopupcount", "0");
        check("nfirstbar", "2");
        check("nsecondbar", "3");
        check("nfifthbar", "20");
        check("nmissingbar", "0");
        check("cpromptbyposition", "Twenty");
        check("cprompttwo", "Two");
        check("cpromptthree", "Three");
        check("cpromptdisabled", "Disabled");
        check("cpromptseparator", "");
        check("cprompttwenty", "Twenty");
        check("cpromptmissing", "");
        check("ldisabled", "true");
        check("lenabled", "false");
        check("lmissingskip", "false");
        check("lenabledafterset", "false");
        check("lredefinedskip", "false");
        check("lmarked", "true");
        check("lunmarked", "false");
        check("lmissingmark", "false");
        check("lunmarkedafterset", "false");
        check("lredefinedmark", "false");
        check("nreplacementcount", "1");
        check("nreplacementpopupcount", "1");
        check("creplacement", "Replacement");
        check("nreleasedcount", "0");
        check("nmissingcount", "0");
        check("cmacroeval", "choices");
        check("cmacroraw", "&lcPopupHolder");
        check("nmacrotype", "9");
        check("cmacrofirst", "Replacement");
        check("cmacrochanged", "Alternate");
        expect(state.ole_objects.size() == 3U,
               "popup RowSource coverage should register the populated, missing, and macro controls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_popup_bar_selection_dispatches_registered_callback()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() /
            "copperfin_native_popup_bar_selection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "popup_selection.prg";
        write_text(
            main_path,
            "DEFINE POPUP choices\n"
            "DEFINE BAR 1 OF choices PROMPT 'Run'\n"
            "DEFINE BAR 2 OF choices PROMPT '\\-'\n"
            "DEFINE BAR 3 OF choices PROMPT 'Skipped'\n"
            "DEFINE BAR 4 OF choices PROMPT 'Fallback'\n"
            "ON SELECTION BAR 1 OF choices DO FirstHandler\n"
            "ON SELECTION BAR 1 OF choices DO ReplacedHandler\n"
            "ON SELECTION BAR 2 OF choices DO ReplacedHandler\n"
            "ON SELECTION BAR 3 OF choices DO ReplacedHandler\n"
            "ON SELECTION POPUP choices DO PopupHandler\n"
            "PUBLIC nSelected\n"
            "PUBLIC cSelected\n"
            "nSelected = 0\n"
            "cSelected = ''\n"
            "SET SKIP OF BAR 3 OF choices .T.\n"
            "DEACTIVATE POPUP choices\n"
            "DEACTIVATE MENU choices\n"
            "ACTIVATE POPUP choices\n"
            "RETURN\n"
            "PROCEDURE FirstHandler\n"
            "nSelected = -1\n"
            "RETURN\n"
            "PROCEDURE ReplacedHandler\n"
            "nSelected = nSelected + 1\n"
            "cSelected = 'replaced'\n"
            "RETURN\n"
            "PROCEDURE PopupHandler\n"
            "nSelected = nSelected + 100\n"
            "cSelected = 'popup'\n"
            "RETURN\n");

        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
        auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.waiting_for_events,
               "popup selection script should pause in the popup event loop");
        expect(!session.dispatch_popup_bar_selection("missing", 1),
               "missing popup selection should fail without dispatch");
        expect(!session.dispatch_popup_bar_selection("choices", 99),
               "missing popup bar selection should fail without dispatch");
        expect(!session.dispatch_popup_bar_selection("choices", 2),
               "separator popup bar selection should fail without dispatch");
        expect(!session.dispatch_popup_bar_selection("choices", 3),
               "skipped popup bar selection should fail without dispatch");
        expect(session.dispatch_popup_bar_selection("CHOICES", 1),
               "popup bar selection should dispatch a registered callback case-insensitively");

        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.waiting_for_events,
               "popup callback should return to the event loop after its frame completes");
        const auto selected = state.globals.find("nselected");
        expect(selected != state.globals.end() &&
                   copperfin::runtime::format_value(selected->second) == "1",
               "re-registering a popup bar should replace its earlier callback");
        const auto selected_text = state.globals.find("cselected");
        expect(selected_text != state.globals.end() &&
                   copperfin::runtime::format_value(selected_text->second) == "replaced",
               "popup callback should run through the selected routine");
        expect(std::any_of(
                   state.events.begin(),
                   state.events.end(),
                   [](const copperfin::runtime::RuntimeEvent& event)
                   {
                       return event.category == "runtime.popup.selection" &&
                           event.detail == "choices bar=1";
                   }),
               "popup callback should emit stable selection telemetry");
        expect(std::any_of(
                   state.events.begin(),
                   state.events.end(),
                   [](const copperfin::runtime::RuntimeEvent& event)
                   {
                       return event.category == "popup.deactivate" &&
                           event.detail == "choices";
                   }),
               "deactivating a popup should emit stable lifecycle telemetry");
        expect(std::any_of(
                   state.events.begin(),
                   state.events.end(),
                   [](const copperfin::runtime::RuntimeEvent& event)
                   {
                       return event.category == "menu.deactivate" &&
                           event.detail == "choices";
                   }),
               "deactivating a menu should emit stable lifecycle telemetry");

        expect(session.dispatch_popup_bar_selection("choices", 4),
               "popup selection should fall back when the bar has no specific callback");
        state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.waiting_for_events,
               "popup-level callback should return to the event loop after its frame completes");
        const auto popup_selected = state.globals.find("nselected");
        expect(popup_selected != state.globals.end() &&
                   copperfin::runtime::format_value(state.globals.at("nselected")) == "101",
               "popup-level callback should execute after bar-specific callback");
        const auto popup_selected_text = state.globals.find("cselected");
        expect(popup_selected_text != state.globals.end() &&
                   copperfin::runtime::format_value(state.globals.at("cselected")) == "popup",
               "popup-level callback should run through the selected routine");

        const fs::path clear_path = temp_root / "popup_selection_clear.prg";
        write_text(
            clear_path,
            "DEFINE POPUP choices\n"
            "DEFINE BAR 1 OF choices PROMPT 'Run'\n"
            "ON SELECTION POPUP choices DO ReplacedHandler\n"
            "ON SELECTION POPUP choices\n"
            "ACTIVATE POPUP choices\n"
            "RETURN\n"
            "PROCEDURE ReplacedHandler\n"
            "RETURN\n");
        auto cleared_session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(clear_path, temp_root));
        const auto cleared_state = cleared_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(cleared_state.waiting_for_events &&
                   !cleared_session.dispatch_popup_bar_selection("choices", 1),
               "popup-level no-command form should clear the registered callback");

        fs::remove_all(temp_root, ignored);
    }
}
