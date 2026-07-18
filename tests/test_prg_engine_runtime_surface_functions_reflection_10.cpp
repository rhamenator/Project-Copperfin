#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_base_addobject_deeper_external_child_base_aclass_reflects_inheritance_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_base_aclass";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_base_aclass.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "xChildClassLibrary = GETPEM(oChild, 'ClassLibrary')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child-base ACLASS script should complete: ") + state.message +
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

        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");

        expect(state.ole_objects.size() == 2U,
               "external-base inherited ADDOBJECT deeper external child-base ACLASS should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve child identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child-base ACLASS should preserve the deeper runtime child class hierarchy");
            if (child_object.class_hierarchy.size() == 5U)
            {
                expect(child_object.class_hierarchy[0] == "SAVEBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the derived child class first");
                expect(child_object.class_hierarchy[1] == "PARENTBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the intermediate external parent second");
                expect(child_object.class_hierarchy[2] == "ROOTBUTTON",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the deeper external ancestor third");
                expect(child_object.class_hierarchy[3] == "CUSTOM",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the builtin base token");
                expect(child_object.class_hierarchy[4] == "OBJECT",
                       "external-base inherited ADDOBJECT deeper external child-base ACLASS should store the terminal object token");
            }
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_surfaces_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 70)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "xChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("nmembersprops", "5");
        check("nmembersunion", "12");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "70");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through ordinary reads");
        const auto child_class_library_reflect = state.globals.find("xchildclasslibraryreflect");
        expect(child_class_library_reflect != state.globals.end() &&
                   child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through GETPEM");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity should preserve the deeper runtime child class hierarchy");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_identity_surfaces_stay_coherent()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_identity";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_identity.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 71)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "cChildClass = oChild.Class\n"
            "cChildBaseClass = oChild.BaseClass\n"
            "cChildParentClass = oChild.ParentClass\n"
            "xChildClassLibrary = oChild.ClassLibrary\n"
            "cChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cUnion1 = aMembersUnion[1]\n"
            "cUnion5 = aMembersUnion[5]\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base inherited ADDOBJECT deeper external child identity script should complete: ") + state.message +
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

        check("cchildclass", "SaveButton");
        check("cchildbaseclass", "ParentButton");
        check("cchildparentclass", "ParentButton");
        check("cchildclassreflect", "SaveButton");
        check("cchildbaseclassreflect", "ParentButton");
        check("cchildparentclassreflect", "ParentButton");
        check("lchildhasclass", "true");
        check("lchildhasbaseclass", "true");
        check("lchildhasparentclass", "true");
        check("lchildhasclasslibrary", "false");
        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("nmembersprops", "5");
        check("nmembersunion", "12");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "PARENT");
        check("cprop5", "PARENTCLASS");
        check("cunion1", "BASECLASS");
        check("cunion5", "PARENTCLASS");
        check("ldictset", "true");
        check("ndictcompare", "71");

        const auto child_class_library = state.globals.find("xchildclasslibrary");
        expect(child_class_library != state.globals.end() &&
                   child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through ordinary reads");
        const auto child_class_library_reflect = state.globals.find("xchildclasslibraryreflect");
        expect(child_class_library_reflect != state.globals.end() &&
                   child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty through GETPEM");

        expect(state.ole_objects.size() == 3U,
               "external-base inherited ADDOBJECT deeper external child identity should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child identity should leave the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "external-base inherited ADDOBJECT deeper external child identity should preserve the deeper runtime child class hierarchy");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child identity lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_base_addobject_deeper_external_child_base_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_base_inherited_addobject_deeper_external_child_base_dotted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path root_library_path = temp_root / "rootbuttons.prg";
        write_text(
            root_library_path,
            "DEFINE CLASS RootButton AS Custom\n"
            "    FUNCTION RootToken\n"
            "        RETURN 'RootToken'\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS RootButton OF rootbuttons.prg\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject_deeper_external_child_base_dotted.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "cCreateChildCaptionBefore = oCreate.cmdSave.Caption\n"
            "cLeafChildCaptionBefore = oLeaf.cmdSave.Caption\n"
            "cCreateParentCaptionBefore = oCreate.cmdSave.Parent.Caption\n"
            "cLeafParentCaptionBefore = oLeaf.cmdSave.Parent.Caption\n"
            "oCreate.cmdSave.Caption = 'Go'\n"
            "oLeaf.cmdSave.Caption = 'Ship'\n"
            "oCreate.cmdSave.Parent.Caption = 'Done'\n"
            "oLeaf.cmdSave.Parent.Caption = 'Ready'\n"
            "cCreateChildCaptionAfter = oCreate.cmdSave.Caption\n"
            "cLeafChildCaptionAfter = oLeaf.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeaf.cmdSave.OwnerCaption()\n"
            "cCreateSavedCaption = oCreate.cmdSave.TriggerSave()\n"
            "cLeafSavedCaption = oLeaf.cmdSave.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cCreateChildBaseClass = oCreate.cmdSave.BaseClass\n"
            "cLeafChildBaseClass = oLeaf.cmdSave.BaseClass\n"
            "xCreateChildClassLibrary = oCreate.cmdSave.ClassLibrary\n"
            "xLeafChildClassLibrary = oLeaf.cmdSave.ClassLibrary\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeaf.cmdSave, 'Parent', 1)\n"
            "cCreateRootToken = oCreate.cmdSave.RootToken()\n"
            "cLeafRootToken = oLeaf.cmdSave.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 139)\n"
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
               std::string("external-base inherited ADDOBJECT deeper external child-base dotted script should complete: ") + state.message +
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

        check("ccreatechildcaptionbefore", "Save");
        check("cleafchildcaptionbefore", "Save");
        check("ccreateparentcaptionbefore", "MainForm");
        check("cleafparentcaptionbefore", "MainForm");
        check("ccreatechildcaptionafter", "Go");
        check("cleafchildcaptionafter", "Ship");
        check("ccreateownercaption", "Done");
        check("cleafownercaption", "Ready");
        check("ccreatesavedcaption", "Done-Saved");
        check("cleafsavedcaption", "Ready-Saved");
        check("ccreatecaptionaftersave", "Done-Saved");
        check("cleafcaptionaftersave", "Ready-Saved");
        check("ccreatechildbaseclass", "ParentButton");
        check("cleafchildbaseclass", "ParentButton");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ccreateroottoken", "RootToken");
        check("cleafroottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "139");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty through ordinary reads");
        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "external-base inherited ADDOBJECT deeper external child-base dotted should keep the leaf child ClassLibrary empty through ordinary reads");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT deeper external child-base dotted should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve CREATEOBJECT child definition source");
            expect(create_child.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should keep the CREATEOBJECT child ClassLibrary empty");
            expect(leaf_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve leaf child identity");
            expect(leaf_child.base_class_name == "ParentButton",
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the leaf child immediate external base");
            expect(leaf_child.source == button_library_path.string(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should preserve leaf child definition source");
            expect(leaf_child.class_library.empty(),
                   "external-base inherited ADDOBJECT deeper external child-base dotted should keep the leaf child ClassLibrary empty");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "external-base inherited ADDOBJECT deeper external child-base dotted should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "external-base inherited ADDOBJECT deeper external child-base dotted should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "external-base inherited ADDOBJECT deeper external child-base dotted should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while external-base inherited ADDOBJECT deeper external child-base dotted lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited ADDOBJECT deeper external child-base dotted should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   (event.detail == "SaveButton.OwnerCaption" ||
                    event.detail == "ParentButton.OwnerCaption");
        });
        expect(has_owner_caption_invoke_event,
               "external-base inherited ADDOBJECT deeper external child-base dotted should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base inherited ADDOBJECT deeper external child-base dotted should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_addobject_deeper_external_child_identity_metadata_stays_read_only_to_setpem()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_addobject_deeper_external_child_identity_setpem";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_addobject_deeper_external_child_identity_setpem.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "lChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lSetChildClass = SETPEM(oChild, 'Class', 'OtherClass')\n"
            "lSetChildBaseClass = SETPEM(oChild, 'BaseClass', 'OtherBase')\n"
            "lSetChildParentClass = SETPEM(oChild, 'ParentClass', 'OtherParent')\n"
            "lSetChildClassLibrary = SETPEM(oChild, 'ClassLibrary', 'other.prg')\n"
            "cChildClassAfter = GETPEM(oChild, 'Class')\n"
            "cChildBaseClassAfter = GETPEM(oChild, 'BaseClass')\n"
            "cChildParentClassAfter = GETPEM(oChild, 'ParentClass')\n"
            "xChildClassLibraryAfter = GETPEM(oChild, 'ClassLibrary')\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 72)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton', 'buttons.prg')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native ADDOBJECT deeper external child identity SETPEM script should complete: ") + state.message +
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

        check("lchildclassreadonly", "true");
        check("lchildbaseclassreadonly", "true");
        check("lchildparentclassreadonly", "true");
        check("lsetchildclass", "false");
        check("lsetchildbaseclass", "false");
        check("lsetchildparentclass", "false");
        check("lsetchildclasslibrary", "false");
        check("cchildclassafter", "SaveButton");
        check("cchildbaseclassafter", "ParentButton");
        check("cchildparentclassafter", "ParentButton");
        check("ldictset", "true");
        check("ndictcompare", "72");

        const auto child_class_library_after = state.globals.find("xchildclasslibraryafter");
        expect(child_class_library_after != state.globals.end() &&
                   child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty after failed mutation");

        expect(state.ole_objects.size() == 3U,
               "native ADDOBJECT deeper external child identity SETPEM should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve parent identity");
            expect(child_object.prog_id == "SaveButton",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve child identity");
            expect(child_object.base_class_name == "ParentButton",
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the immediate child base-class identity");
            expect(child_object.source == button_library_path.string(),
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the child definition source path");
            expect(child_object.class_library.empty(),
                   "native ADDOBJECT deeper external child identity SETPEM should keep the derived child ClassLibrary empty");
            expect(child_object.class_hierarchy.size() == 5U,
                   "native ADDOBJECT deeper external child identity SETPEM should preserve the deeper runtime child class hierarchy");
            expect(!child_object.properties.contains("class"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child Class shadow");
            expect(!child_object.properties.contains("baseclass"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child BaseClass shadow");
            expect(!child_object.properties.contains("parentclass"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ParentClass shadow");
            expect(!child_object.properties.contains("classlibrary"),
                   "native ADDOBJECT deeper external child identity SETPEM should not materialize a writable child ClassLibrary shadow");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while native ADDOBJECT deeper external child identity SETPEM lands");
        }

        fs::remove_all(temp_root, ignored);
    }

}
