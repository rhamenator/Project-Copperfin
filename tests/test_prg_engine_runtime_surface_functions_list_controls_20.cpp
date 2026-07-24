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
            "cPromptTwo = PRMBAR('choices', 2)\n"
            "cPromptThree = PRMBAR('choices', 3)\n"
            "cPromptDisabled = PRMBAR('choices', 4)\n"
            "cPromptSeparator = PRMBAR('choices', 5)\n"
            "cPromptTwenty = PRMBAR('choices', 20)\n"
            "cPromptMissing = PRMBAR('choices', 99)\n"
            "DEFINE POPUP choices\n"
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
        check("cprompttwo", "Two");
        check("cpromptthree", "Three");
        check("cpromptdisabled", "Disabled");
        check("cpromptseparator", "");
        check("cprompttwenty", "Twenty");
        check("cpromptmissing", "");
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
}
