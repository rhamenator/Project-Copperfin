#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_pageframe_activepage_reflects_runtime_selection_and_bounded_reflection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_activepage_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_activepage_property.prg";
        write_text(
            main_path,
            "oFrame = CREATEOBJECT('DemoPageFrame')\n"
            "nActivePageDefault = oFrame.ActivePage\n"
            "xActivePageGetPem = GETPEM(oFrame, 'ActivePage')\n"
            "lHasActivePage = PEMSTATUS(oFrame, 'ActivePage', 1)\n"
            "lActivePageReadOnly = PEMSTATUS(oFrame, 'ActivePage', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oFrame, 1)\n"
            "nUnionMembers = AMEMBERS(aUnionMembers, oFrame, 3)\n"
            "lPropHasActivePage = .F.\n"
            "lUnionHasActivePage = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'ACTIVEPAGE'\n"
            "        lPropHasActivePage = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "FOR i = 1 TO nUnionMembers\n"
            "    IF UPPER(aUnionMembers[i]) == 'ACTIVEPAGE'\n"
            "        lUnionHasActivePage = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "oFrame.ActivePage = 2\n"
            "nActivePageAfterDirectSet = oFrame.ActivePage\n"
            "lSetActivePage = SETPEM(oFrame, 'ActivePage', 3)\n"
            "nActivePageAfterSetPem = oFrame.ActivePage\n"
            "oFrame.ActivePage = 99\n"
            "nActivePageAfterHighClamp = oFrame.ActivePage\n"
            "lAddActivePage = ADDPROPERTY(oFrame, 'ActivePage', 7)\n"
            "lRemoveActivePage = REMOVEPROPERTY(oFrame, 'ActivePage')\n"
            "lRemoved = oFrame.RemoveObject('pgGamma')\n"
            "nActivePageAfterRemove = oFrame.ActivePage\n"
            "cCurrentTagAfterRemove = oFrame.Pages(oFrame.ActivePage).cTag\n"
            "oFrame.ActivePage = 0\n"
            "nActivePageAfterLowClamp = oFrame.ActivePage\n"
            "oEmpty = CREATEOBJECT('EmptyPageFrame')\n"
            "nEmptyActivePageDefault = oEmpty.ActivePage\n"
            "oEmpty.ActivePage = 7\n"
            "nEmptyActivePageAfterSet = oEmpty.ActivePage\n"
            "RETURN\n"
            "DEFINE CLASS AlphaPage AS Page\n"
            "    cTag = 'alpha'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BetaPage AS Page\n"
            "    cTag = 'beta'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS GammaPage AS Page\n"
            "    cTag = 'gamma'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoPageFrame AS PageFrame\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('pgAlpha', 'AlphaPage')\n"
            "        THIS.AddObject('pgBeta', 'BetaPage')\n"
            "        THIS.AddObject('pgGamma', 'GammaPage')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS EmptyPageFrame AS PageFrame\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ActivePage property script should complete: ") + state.message +
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

        check("nactivepagedefault", "1");
        check("xactivepagegetpem", "1");
        check("lhasactivepage", "true");
        check("lactivepagereadonly", "false");
        check("lprophasactivepage", "true");
        check("lunionhasactivepage", "true");
        check("nactivepageafterdirectset", "2");
        check("lsetactivepage", "true");
        check("nactivepageaftersetpem", "3");
        check("nactivepageafterhighclamp", "3");
        check("nactivepageafterlowclamp", "1");
        check("laddactivepage", "false");
        check("lremoveactivepage", "false");
        check("lremoved", "true");
        check("nactivepageafterremove", "2");
        check("ccurrenttagafterremove", "beta");
        check("nemptyactivepagedefault", "0");
        check("nemptyactivepageafterset", "0");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_pageframe_pages_follow_live_addobject_order_instead_of_member_names()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_pageframe_live_page_order";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pageframe_live_page_order.prg";
        write_text(
            main_path,
            "oFrame = CREATEOBJECT('DemoPageFrame')\n"
            "cFirstTag = oFrame.Pages(1).cTag\n"
            "cSecondTag = oFrame.Pages(2).cTag\n"
            "cThirdTag = oFrame.Pages(3).cTag\n"
            "cLoop = ''\n"
            "FOR EACH oPage IN oFrame.Pages FOXOBJECT\n"
            "    cLoop = cLoop + IIF(EMPTY(cLoop), '', ',') + oPage.cTag\n"
            "ENDFOR\n"
            "oFrame.ActivePage = 3\n"
            "cActiveTag = oFrame.Pages(oFrame.ActivePage).cTag\n"
            "oFrame.PageCount = 2\n"
            "cRemainingSecondTag = oFrame.Pages(2).cTag\n"
            "lSecondNamedPageStillExists = PEMSTATUS(oFrame, 'aaSecond', 1)\n"
            "lThirdNamedPageStillExists = PEMSTATUS(oFrame, 'mmThird', 1)\n"
            "RETURN\n"
            "DEFINE CLASS FirstPage AS Page\n"
            "    cTag = 'first'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SecondPage AS Page\n"
            "    cTag = 'second'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ThirdPage AS Page\n"
            "    cTag = 'third'\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoPageFrame AS PageFrame\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('zzFirst', 'FirstPage')\n"
            "        THIS.AddObject('aaSecond', 'SecondPage')\n"
            "        THIS.AddObject('mmThird', 'ThirdPage')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageFrame live page order script should complete: ") + state.message +
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

        check("cfirsttag", "first");
        check("csecondtag", "second");
        check("cthirdtag", "third");
        check("cloop", "first,second,third");
        check("cactivetag", "third");
        check("cremainingsecondtag", "second");
        check("lsecondnamedpagestillexists", "true");
        check("lthirdnamedpagestillexists", "false");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_controlcount_reflects_controls_count_and_stays_read_only()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_controlcount_property";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_controlcount_property.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "nControlCountDirect = oForm.ControlCount\n"
            "nControlCountCollection = oForm.Controls.Count\n"
            "xControlCountGetPem = GETPEM(oForm, 'ControlCount')\n"
            "lHasControlCount = PEMSTATUS(oForm, 'ControlCount', 1)\n"
            "lControlCountReadOnly = PEMSTATUS(oForm, 'ControlCount', 5)\n"
            "nPropMembers = AMEMBERS(aPropMembers, oForm, 1)\n"
            "nUnionMembers = AMEMBERS(aUnionMembers, oForm, 3)\n"
            "lPropHasControlCount = .F.\n"
            "lUnionHasControlCount = .F.\n"
            "FOR i = 1 TO nPropMembers\n"
            "    IF UPPER(aPropMembers[i]) == 'CONTROLCOUNT'\n"
            "        lPropHasControlCount = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "FOR i = 1 TO nUnionMembers\n"
            "    IF UPPER(aUnionMembers[i]) == 'CONTROLCOUNT'\n"
            "        lUnionHasControlCount = .T.\n"
            "    ENDIF\n"
            "ENDFOR\n"
            "lSetControlCount = SETPEM(oForm, 'ControlCount', 99)\n"
            "lAddControlCount = ADDPROPERTY(oForm, 'ControlCount', 99)\n"
            "lRemoveControlCount = REMOVEPROPERTY(oForm, 'ControlCount')\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "nControlCountAfterRemove = oForm.ControlCount\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS BadgeLabel AS Label\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoForm AS Form\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.AddObject('lblBadge', 'BadgeLabel')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ControlCount property script should complete: ") + state.message +
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

        check("ncontrolcountdirect", "2");
        check("ncontrolcountcollection", "2");
        check("xcontrolcountgetpem", "2");
        check("lhascontrolcount", "true");
        check("lcontrolcountreadonly", "true");
        check("lprophascontrolcount", "true");
        check("lunionhascontrolcount", "true");
        check("lsetcontrolcount", "false");
        check("laddcontrolcount", "false");
        check("lremovecontrolcount", "false");
        check("lremoved", "true");
        check("ncontrolcountafterremove", "1");

        fs::remove_all(temp_root, ignored);
    }

}
