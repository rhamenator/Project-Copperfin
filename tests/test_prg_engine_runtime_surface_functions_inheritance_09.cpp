#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_parent_addobject_derived_startup_child_preserves_external_base_provenance()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_addobject_derived_startup_external_base";
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
            "ENDDEFINE\n");

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    cInitOwnerCaption = ''\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.cInitOwnerCaption = THIS.cmdSave.OwnerCaption()\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_addobject_derived_startup_external_base.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cLeafInitChildCaption = oLeaf.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cLeafInitOwnerCaption = oLeaf.cInitOwnerCaption\n"
            "cCreateChildClass = oChild.Class\n"
            "cCreateChildBaseClass = oChild.BaseClass\n"
            "cCreateChildParentClass = oChild.ParentClass\n"
            "cCreateChildClassLibrary = oChild.ClassLibrary\n"
            "cCreateChildClassReflect = GETPEM(oChild, 'Class')\n"
            "cCreateChildBaseClassReflect = GETPEM(oChild, 'BaseClass')\n"
            "cCreateChildParentClassReflect = GETPEM(oChild, 'ParentClass')\n"
            "cCreateChildClassLibraryReflect = GETPEM(oChild, 'ClassLibrary')\n"
            "lCreateChildHasClass = PEMSTATUS(oChild, 'Class', 1)\n"
            "lCreateChildHasBaseClass = PEMSTATUS(oChild, 'BaseClass', 1)\n"
            "lCreateChildHasParentClass = PEMSTATUS(oChild, 'ParentClass', 1)\n"
            "lCreateChildHasClassLibrary = PEMSTATUS(oChild, 'ClassLibrary', 1)\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oChild, 'ParentClass', 5)\n"
            "lCreateChildClassLibraryReadOnly = PEMSTATUS(oChild, 'ClassLibrary', 5)\n"
            "nMembersProps = AMEMBERS(aMembersProps, oChild, 1)\n"
            "nMembersUnion = AMEMBERS(aMembersUnion, oChild, 3)\n"
            "cProp1 = aMembersProps[1]\n"
            "cProp2 = aMembersProps[2]\n"
            "cProp3 = aMembersProps[3]\n"
            "cProp4 = aMembersProps[4]\n"
            "cProp5 = aMembersProps[5]\n"
            "cProp6 = aMembersProps[6]\n"
            "cUnion4 = aMembersUnion[4]\n"
            "nClassCount = ACLASS(aClass, oChild)\n"
            "cClass1 = aClass[1]\n"
            "cClass2 = aClass[2]\n"
            "cClass3 = aClass[3]\n"
            "cClass4 = aClass[4]\n"
            "cClass5 = aClass[5]\n"
            "cCreateOwnerCaption = oChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cLeafChildClass = oLeafChild.Class\n"
            "cLeafChildClassLibrary = oLeafChild.ClassLibrary\n"
            "cLeafChildBaseClass = oLeafChild.BaseClass\n"
            "cLeafChildParentClass = oLeafChild.ParentClass\n"
            "cLeafChildClassReflect = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassReflect = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassReflect = GETPEM(oLeafChild, 'ParentClass')\n"
            "cLeafChildClassLibraryReflect = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "lLeafChildHasClass = PEMSTATUS(oLeafChild, 'Class', 1)\n"
            "lLeafChildHasBaseClass = PEMSTATUS(oLeafChild, 'BaseClass', 1)\n"
            "lLeafChildHasParentClass = PEMSTATUS(oLeafChild, 'ParentClass', 1)\n"
            "lLeafChildHasClassLibrary = PEMSTATUS(oLeafChild, 'ClassLibrary', 1)\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "lLeafChildClassLibraryReadOnly = PEMSTATUS(oLeafChild, 'ClassLibrary', 5)\n"
            "nLeafMembersProps = AMEMBERS(aLeafMembersProps, oLeafChild, 1)\n"
            "nLeafMembersUnion = AMEMBERS(aLeafMembersUnion, oLeafChild, 3)\n"
            "cLeafProp1 = aLeafMembersProps[1]\n"
            "cLeafProp2 = aLeafMembersProps[2]\n"
            "cLeafProp3 = aLeafMembersProps[3]\n"
            "cLeafProp4 = aLeafMembersProps[4]\n"
            "cLeafProp5 = aLeafMembersProps[5]\n"
            "cLeafProp6 = aLeafMembersProps[6]\n"
            "nLeafClassCount = ACLASS(aLeafClass, oLeafChild)\n"
            "cLeafClass1 = aLeafClass[1]\n"
            "cLeafClass2 = aLeafClass[2]\n"
            "cLeafClass3 = aLeafClass[3]\n"
            "cLeafClass4 = aLeafClass[4]\n"
            "cLeafClass5 = aLeafClass[5]\n"
            "lCreateChildHasParent = PEMSTATUS(oChild, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeafChild, 'Parent', 1)\n"
            "cRootToken = oChild.RootToken()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 133)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent ADDOBJECT derived-startup external-base script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Save");
        check("cleafinitchildcaption", "Save");
        check("ccreateinitownercaption", "MainForm");
        check("cleafinitownercaption", "MainForm");
        check("ccreatechildclass", "SaveButton");
        check("ccreatechildbaseclass", "ParentButton");
        check("ccreatechildparentclass", "ParentButton");
        check("ccreatechildclasslibrary", button_library_path.string());
        check("ccreatechildclassreflect", "SaveButton");
        check("ccreatechildbaseclassreflect", "ParentButton");
        check("ccreatechildparentclassreflect", "ParentButton");
        check("ccreatechildclasslibraryreflect", button_library_path.string());
        check("lcreatechildhasclass", "true");
        check("lcreatechildhasbaseclass", "true");
        check("lcreatechildhasparentclass", "true");
        check("lcreatechildhasclasslibrary", "true");
        check("lcreatechildclassreadonly", "true");
        check("lcreatechildbaseclassreadonly", "true");
        check("lcreatechildparentclassreadonly", "true");
        check("lcreatechildclasslibraryreadonly", "true");
        check("nmembersprops", "6");
        check("cprop1", "BASECLASS");
        check("cprop2", "CAPTION");
        check("cprop3", "CLASS");
        check("cprop4", "CLASSLIBRARY");
        check("cprop5", "PARENT");
        check("cprop6", "PARENTCLASS");
        check("nclasscount", "5");
        check("cclass1", "SAVEBUTTON");
        check("cclass2", "PARENTBUTTON");
        check("cclass3", "ROOTBUTTON");
        check("cclass4", "CUSTOM");
        check("cclass5", "OBJECT");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("cleafchildclass", "SaveButton");
        check("cleafchildclasslibrary", button_library_path.string());
        check("cleafchildbaseclass", "ParentButton");
        check("cleafchildparentclass", "ParentButton");
        check("cleafchildclassreflect", "SaveButton");
        check("cleafchildbaseclassreflect", "ParentButton");
        check("cleafchildparentclassreflect", "ParentButton");
        check("cleafchildclasslibraryreflect", button_library_path.string());
        check("lleafchildhasclass", "true");
        check("lleafchildhasbaseclass", "true");
        check("lleafchildhasparentclass", "true");
        check("lleafchildhasclasslibrary", "true");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("lleafchildclasslibraryreadonly", "true");
        check("nleafmembersprops", "6");
        check("cleafprop1", "BASECLASS");
        check("cleafprop2", "CAPTION");
        check("cleafprop3", "CLASS");
        check("cleafprop4", "CLASSLIBRARY");
        check("cleafprop5", "PARENT");
        check("cleafprop6", "PARENTCLASS");
        check("nleafclasscount", "5");
        check("cleafclass1", "SAVEBUTTON");
        check("cleafclass2", "PARENTBUTTON");
        check("cleafclass3", "ROOTBUTTON");
        check("cleafclass4", "CUSTOM");
        check("cleafclass5", "OBJECT");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("croottoken", "RootToken");
        check("ldictset", "true");
        check("ndictcompare", "133");

        const auto leaf_members_union = state.globals.find("nleafmembersunion");
        expect(leaf_members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(leaf_members_union->second)) >= 9,
               "inherited external-parent ADDOBJECT derived-startup external-base should keep leaf union member enumeration including inherited child methods");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent ADDOBJECT derived-startup external-base script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup external-base should materialize the CREATEOBJECT child");
            expect(create_child.base_class_name == "ParentButton",
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the CREATEOBJECT child immediate external base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve derived-startup child source identity");
            expect(create_child.class_library == button_library_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the CREATEOBJECT child external ClassLibrary path");
            expect(create_child.class_hierarchy.size() == 5U,
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the CREATEOBJECT child class hierarchy");
            if (create_child.class_hierarchy.size() == 5U)
            {
                expect(create_child.class_hierarchy[0] == "SAVEBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the CREATEOBJECT child class first");
                expect(create_child.class_hierarchy[1] == "PARENTBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the immediate external base second");
                expect(create_child.class_hierarchy[2] == "ROOTBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the deeper external base third");
                expect(create_child.class_hierarchy[3] == "CUSTOM",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the builtin base token fourth");
                expect(create_child.class_hierarchy[4] == "OBJECT",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the terminal object token fifth");
            }
            expect(!create_child.properties.contains("classlibrary"),
                   "inherited external-parent ADDOBJECT derived-startup external-base should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup external-base should materialize the leaf child");
            expect(leaf_child.base_class_name == "ParentButton",
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the leaf child immediate external base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve leaf child source identity");
            expect(leaf_child.class_library == button_library_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the leaf child external ClassLibrary path");
            expect(leaf_child.class_hierarchy.size() == 5U,
                   "inherited external-parent ADDOBJECT derived-startup external-base should preserve the leaf child class hierarchy");
            if (leaf_child.class_hierarchy.size() == 5U)
            {
                expect(leaf_child.class_hierarchy[0] == "SAVEBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the leaf child class first");
                expect(leaf_child.class_hierarchy[1] == "PARENTBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the leaf immediate external base second");
                expect(leaf_child.class_hierarchy[2] == "ROOTBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the leaf deeper external base third");
                expect(leaf_child.class_hierarchy[3] == "CUSTOM",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the leaf builtin base token fourth");
                expect(leaf_child.class_hierarchy[4] == "OBJECT",
                       "inherited external-parent ADDOBJECT derived-startup external-base should store the leaf terminal object token fifth");
            }
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent ADDOBJECT derived-startup external-base should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent ADDOBJECT derived-startup external-base lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent ADDOBJECT derived-startup external-base should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent ADDOBJECT derived-startup external-base should keep materialized children usable through THISFORM owner dispatch");

        fs::remove_all(temp_root, ignored);
    }

}
