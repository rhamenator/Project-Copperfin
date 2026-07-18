#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_deeper_external_child_external_base_classlibrary_cannot_be_shadowed_through_direct_assignment()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_external_base_direct_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS RootButton AS Custom\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS RootButton OF rootbuttons.prg\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_external_base_direct_assignment.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "oChild.Class = 'OtherClass'\n"
            "oChild.BaseClass = 'OtherBase'\n"
            "oChild.ParentClass = 'OtherParent'\n"
            "oChild.ClassLibrary = 'other.prg'\n"
            "oLeafChild.Class = 'OtherClass'\n"
            "oLeafChild.BaseClass = 'OtherBase'\n"
            "oLeafChild.ParentClass = 'OtherParent'\n"
            "oLeafChild.ClassLibrary = 'other.prg'\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "cChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "cChildClassLibraryPropAfter = oChild.ClassLibrary\n"
            "cLeafChildClassAfter = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassAfter = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassAfter = GETPEM(oLeafChild, 'ParentClass')\n"
            "cLeafChildClassLibraryAfter = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "cLeafChildClassLibraryPropAfter = oLeafChild.ClassLibrary\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "lLeafChildClassLibraryReadOnly = PEMSTATUS(oLeafChild, 'ClassLibrary', 5)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 89)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child external-base direct-assignment script should complete: ") + state.message +
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

        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("cchildclasslibraryafter", button_library_path.string());
        check("cchildclasslibrarypropafter", button_library_path.string());
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "ParentButton");
        check("cleafchildparentclassafter", "ParentButton");
        check("cleafchildclasslibraryafter", button_library_path.string());
        check("cleafchildclasslibrarypropafter", button_library_path.string());
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lchildclasslibraryreadonly", "true");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("lleafchildclasslibraryreadonly", "true");
        check("ldictset", "true");
        check("ndictcompare", "89");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT deeper external child external-base direct assignment should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve CREATEOBJECT immediate child base-class identity");
            expect(create_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the CREATEOBJECT child source path");
            expect(create_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the CREATEOBJECT immediate external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve NEWOBJECT leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve NEWOBJECT leaf child immediate base-class identity");
            expect(leaf_child.source == widget_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the NEWOBJECT leaf child source path");
            expect(leaf_child.class_library == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the NEWOBJECT leaf child immediate external ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should preserve the NEWOBJECT leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a NEWOBJECT leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a NEWOBJECT leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a NEWOBJECT leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "external-base inherited ADDOBJECT deeper external child external-base direct assignment should not materialize a NEWOBJECT leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child external-base direct assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_same_prg_native_dodefault_dispatches_base_methods_and_preserves_byref_init_flow()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_dodefault";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_dodefault.prg";
        write_text(
            main_path,
            "nSeed = 5\n"
            "oCreate = CREATEOBJECT('ChildWidget', @nSeed)\n"
            "oPlain = CREATEOBJECT('Empty')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "oPlain.Extra = 'plain'\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 7)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cDescribe = oCreate.Describe('prefix')\n"
            "cWho = oCreate.Who()\n"
            "nSeedAfter = nSeed\n"
            "cCaption = oCreate.Caption\n"
            "nStored = oCreate.nValue\n"
            "lParentInit = oCreate.lParentInit\n"
            "lChildInit = oCreate.lChildInit\n"
            "cPlain = oPlain.Extra\n"
            "RETURN\n"
            "DEFINE CLASS ParentWidget AS Custom\n"
            "    Caption = 'Parent'\n"
            "    nValue = 0\n"
            "    lParentInit = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        tnSeed = tnSeed + 2\n"
            "        THIS.nValue = tnSeed\n"
            "        THIS.Caption = THIS.Caption + '-P'\n"
            "        THIS.lParentInit = .T.\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN 'Parent'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ChildWidget AS ParentWidget\n"
            "    Caption = 'Child'\n"
            "    lChildInit = .F.\n"
            "    PROCEDURE Init\n"
            "        LPARAMETERS tnSeed\n"
            "        LOCAL lcBaseCaption\n"
            "        lcBaseCaption = DODEFAULT(@tnSeed)\n"
            "        THIS.Caption = lcBaseCaption + '-C'\n"
            "        THIS.lChildInit = .T.\n"
            "        RETURN THIS.Caption\n"
            "    ENDPROC\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN DODEFAULT(tcPrefix) + ':Child'\n"
            "    ENDFUNC\n"
            "    FUNCTION Who\n"
            "        RETURN DODEFAULT() + '+Child'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native DODEFAULT script should complete: ") + state.message +
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

        check("cdescribe", "prefix:Child-P-C:Child");
        check("cwho", "Parent+Child");
        check("nseedafter", "7");
        check("ccaption", "Child-P-C");
        check("nstored", "7");
        check("lparentinit", "true");
        check("lchildinit", "true");
        check("cplain", "plain");
        check("ldictset", "true");
        check("ndictcompare", "7");

        expect(state.ole_objects.size() == 3U,
               "native DODEFAULT script should register native, plain, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &native_object = state.ole_objects[0];
            expect(native_object.prog_id == "ChildWidget",
                   "native DODEFAULT should preserve child class identity");
            const auto caption = native_object.properties.find("caption");
            const auto value = native_object.properties.find("nvalue");
            const auto parent_init = native_object.properties.find("lparentinit");
            const auto child_init = native_object.properties.find("lchildinit");
            expect(caption != native_object.properties.end(),
                   "native DODEFAULT should preserve child/base Init-updated caption state");
            expect(value != native_object.properties.end(),
                   "native DODEFAULT should preserve by-reference Init-updated numeric state");
            expect(parent_init != native_object.properties.end(),
                   "native DODEFAULT should preserve parent Init state");
            expect(child_init != native_object.properties.end(),
                   "native DODEFAULT should preserve child Init state");
            if (caption != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "Child-P-C",
                       "native DODEFAULT should compose child Init logic after base Init");
            }
            if (value != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(value->second) == "7",
                       "native DODEFAULT should preserve base Init by-reference write-back results");
            }
            if (parent_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(parent_init->second) == "true",
                       "native DODEFAULT should run parent Init through the base-call path");
            }
            if (child_init != native_object.properties.end())
            {
                expect(copperfin::runtime::format_value(child_init->second) == "true",
                       "native DODEFAULT should continue child Init logic after the base-call path");
            }

            expect(state.ole_objects[1].prog_id == "Empty",
                   "plain CREATEOBJECT should remain stable while native DODEFAULT lands");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native DODEFAULT lands");
        }

        const bool has_base_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.baseinvoke" &&
                   (event.detail == "ParentWidget.Init" ||
                    event.detail == "ParentWidget.Describe" ||
                    event.detail == "ParentWidget.Who");
        });
        expect(has_base_invoke_event,
               "native DODEFAULT should emit a base-invoke runtime event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_setall_recurses_over_descendants_and_honors_class_filters()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_setall";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_setall.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "nFlagUpdated = oForm.SetAll('lFlag', .F.)\n"
            "nTagUpdated = oForm.SetAll('cTagValue', 'Queued', 'SaveButton')\n"
            "lHostFlag = oForm.cmdHost.lFlag\n"
            "lContainerFlag = oForm.cntMain.lFlag\n"
            "lNestedFlag = oForm.cntMain.cmdNested.lFlag\n"
            "lPlainFlag = oForm.cntMain.cmdPlain.lFlag\n"
            "lTextFlag = oForm.cntMain.txtName.lFlag\n"
            "cHostTag = oForm.cmdHost.cTagValue\n"
            "cNestedTag = oForm.cntMain.cmdNested.cTagValue\n"
            "cPlainTag = oForm.cntMain.cmdPlain.cTagValue\n"
            "lContainerHasTag = PEMSTATUS(oForm.cntMain, 'cTagValue', 1)\n"
            "lTextHasTag = PEMSTATUS(oForm.cntMain.txtName, 'cTagValue', 1)\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    cTagValue = 'Save'\n"
            "    lFlag = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS PlainButton AS CommandButton\n"
            "    cTagValue = 'Plain'\n"
            "    lFlag = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS NameBox AS TextBox\n"
            "    lFlag = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainContainer AS Container\n"
            "    lFlag = .T.\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    lFlag = .T.\n"
            "    ADD OBJECT cmdHost AS SaveButton\n"
            "    ADD OBJECT cntMain AS MainContainer\n"
            "    PROCEDURE Init\n"
            "        THIS.cntMain.AddObject('cmdNested', 'SaveButton')\n"
            "        THIS.cntMain.AddObject('cmdPlain', 'PlainButton')\n"
            "        THIS.cntMain.AddObject('txtName', 'NameBox')\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native SetAll script should complete: ") + state.message +
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

        check("nflagupdated", "5");
        check("ntagupdated", "2");
        check("lhostflag", "false");
        check("lcontainerflag", "false");
        check("lnestedflag", "false");
        check("lplainflag", "false");
        check("ltextflag", "false");
        check("chosttag", "Queued");
        check("cnestedtag", "Queued");
        check("cplaintag", "Plain");
        check("lcontainerhastag", "false");
        check("ltexthastag", "false");

        const std::size_t setall_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.setall";
            }));
        expect(setall_event_count == 2U,
               "native SetAll should emit one runtime event per SetAll invocation");

        fs::remove_all(temp_root, ignored);
    }

    void test_bare_dotted_native_refresh_statement_invokes_same_prg_override()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_bare_refresh_override";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_bare_refresh_override.prg";
        write_text(
            main_path,
            "nRefreshProbeCalls = 0\n"
            "oForm = CREATEOBJECT('MainForm')\n"
            "lRefreshRan = oForm.lRefreshRan\n"
            "RETURN\n"
            "FUNCTION RefreshProbe\n"
            "    nRefreshProbeCalls = nRefreshProbeCalls + 1\n"
            "    RETURN 42\n"
            "ENDFUNC\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    lRefreshRan = .F.\n"
            "    PROCEDURE Init\n"
            "        THIS.Refresh\n"
            "    ENDPROC\n"
            "    PROCEDURE Refresh\n"
            "        THIS.lRefreshRan = RefreshProbe() = 42\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("bare dotted native Refresh override script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto refreshed = state.globals.find("lrefreshran");
        expect(refreshed != state.globals.end(),
               "bare dotted native Refresh override script should preserve refresh flag");
        if (refreshed != state.globals.end())
        {
            expect(copperfin::runtime::format_value(refreshed->second) == "true",
                   "bare dotted native Refresh override should invoke the class-defined Refresh method");
        }

        const auto refresh_probe_calls = state.globals.find("nrefreshprobecalls");
        expect(refresh_probe_calls != state.globals.end(),
               "bare dotted native Refresh override should preserve the nested UDF result state");
        if (refresh_probe_calls != state.globals.end())
        {
            expect(copperfin::runtime::format_value(refresh_probe_calls->second) == "1",
                   "bare dotted native Refresh override should resume the nested UDF exactly once");
        }

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "MainForm.Refresh";
        });
        expect(has_invoke_event,
               "bare dotted native Refresh override should emit a prg.object.invoke event");

        fs::remove_all(temp_root, ignored);
    }

    void test_bare_dotted_native_release_statement_uses_builtin_release_path()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_bare_release_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_bare_release_builtin.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oChild = oForm.cmdSave\n"
            "oChild.Release\n"
            "lOwnerHasChildAfter = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "lHeldChildHasParentAfter = PEMSTATUS(oChild, 'Parent', 1)\n"
            "cOwnerCaptionAfter = oForm.Caption\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    Caption = 'Main'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("bare dotted native Release builtin script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto owner_has_child_after = state.globals.find("lownerhaschildafter");
        expect(owner_has_child_after != state.globals.end(),
               "bare dotted native Release builtin script should preserve owner child-slot state");
        if (owner_has_child_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(owner_has_child_after->second) == "false",
                   "bare dotted native Release builtin should detach the released child from its owner");
        }

        const auto held_child_has_parent_after = state.globals.find("lheldchildhasparentafter");
        expect(held_child_has_parent_after != state.globals.end(),
               "bare dotted native Release builtin script should preserve held child parent state");
        if (held_child_has_parent_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(held_child_has_parent_after->second) == "false",
                   "bare dotted native Release builtin should invalidate Parent on the released child");
        }

        const auto owner_caption_after = state.globals.find("cownercaptionafter");
        expect(owner_caption_after != state.globals.end(),
               "bare dotted native Release builtin script should preserve owner state after child release");
        if (owner_caption_after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(owner_caption_after->second) == "Main",
                   "bare dotted native Release builtin should keep the owner alive after child release");
        }

        const std::size_t release_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.release";
            }));
        expect(release_event_count == 1U,
               "bare dotted native Release builtin should emit one release event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_refresh_builtin_fallback_succeeds_for_form_and_children()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_refresh_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_refresh_builtin.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.Refresh()\n"
            "oForm.cmdSave.Refresh\n"
            "oForm.cntHost.cmdInner.Refresh\n"
            "lAfter = .T.\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "    ADD OBJECT cntHost AS Container\n"
            "    PROCEDURE Init\n"
            "        THIS.cntHost.AddObject('cmdInner', 'CommandButton')\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Refresh builtin fallback script should complete: ") + state.message +
                   " @line=" + std::to_string(state.location.line));

        const auto after = state.globals.find("lafter");
        expect(after != state.globals.end(),
               "native Refresh builtin fallback script should preserve post-refresh execution");
        if (after != state.globals.end())
        {
            expect(copperfin::runtime::format_value(after->second) == "true",
                   "native Refresh builtin fallback should keep execution moving after form/child refresh calls");
        }

        const std::size_t refresh_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.refresh";
            }));
        expect(refresh_event_count == 3U,
               "native Refresh builtin fallback should emit one refresh event for each representative refresh call");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_refresh_override_preserves_arguments_for_common_force_pattern()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_refresh_override_args";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_refresh_override_args.prg";
        write_text(
            main_path,
            "oNode = CREATEOBJECT('BrowserNode')\n"
            "oNode.Refresh(.T.)\n"
            "lRefreshRan = oNode.lRefreshRan\n"
            "nRefreshArgs = oNode.nRefreshArgs\n"
            "lRefreshForce = oNode.lRefreshForce\n"
            "RETURN\n"
            "DEFINE CLASS BrowserNode AS Custom\n"
            "    lRefreshRan = .F.\n"
            "    nRefreshArgs = 0\n"
            "    lRefreshForce = .F.\n"
            "    PROCEDURE Refresh\n"
            "        LPARAMETERS tlForce\n"
            "        THIS.lRefreshRan = .T.\n"
            "        THIS.nRefreshArgs = PCOUNT()\n"
            "        IF PCOUNT() >= 1\n"
            "            THIS.lRefreshForce = tlForce\n"
            "        ENDIF\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Refresh override argument script should complete: ") + state.message +
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

        check("lrefreshran", "true");
        check("nrefreshargs", "1");
        check("lrefreshforce", "true");

        const bool has_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "BrowserNode.Refresh";
        });
        expect(has_invoke_event,
               "native Refresh override argument coverage should emit a prg.object.invoke event");

        const bool has_builtin_refresh_event = std::any_of(state.events.begin(), state.events.end(), [](const auto& event)
        {
            return event.category == "prg.object.refresh";
        });
        expect(!has_builtin_refresh_event,
               "native Refresh override argument coverage should not emit the builtin prg.object.refresh event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_move_builtin_fallback_updates_visual_geometry()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_move_builtin";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_move_builtin.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('MainForm')\n"
            "oForm.Move(-5000)\n"
            "oForm.cmdSave.Move(30, 40, 120, 22)\n"
            "nFormLeft = oForm.Left\n"
            "nFormTop = oForm.Top\n"
            "nFormWidth = oForm.Width\n"
            "nFormHeight = oForm.Height\n"
            "nButtonLeft = oForm.cmdSave.Left\n"
            "nButtonTop = oForm.cmdSave.Top\n"
            "nButtonWidth = oForm.cmdSave.Width\n"
            "nButtonHeight = oForm.cmdSave.Height\n"
            "RETURN\n"
            "DEFINE CLASS MainForm AS Form\n"
            "    Left = 11\n"
            "    Top = 12\n"
            "    Width = 200\n"
            "    Height = 150\n"
            "    ADD OBJECT cmdSave AS CommandButton\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Move builtin fallback script should complete: ") + state.message +
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

        check("nformleft", "-5000");
        check("nformtop", "12");
        check("nformwidth", "200");
        check("nformheight", "150");
        check("nbuttonleft", "30");
        check("nbuttontop", "40");
        check("nbuttonwidth", "120");
        check("nbuttonheight", "22");

        const std::size_t move_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.move";
            }));
        expect(move_event_count == 2U,
               "native Move builtin fallback should emit one move event per representative move call");

        fs::remove_all(temp_root, ignored);
    }

}
