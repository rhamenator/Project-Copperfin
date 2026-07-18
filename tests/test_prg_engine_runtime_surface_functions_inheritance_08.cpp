#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_prg_base_methods_resolve_addobject_children_against_defining_library()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_external_base_inherited_addobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "widgetlib.prg";
        write_text(
            library_path,
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_base_inherited_addobject.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oNew = NEWOBJECT('LeafForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lNewHasChild = PEMSTATUS(oNew, 'cmdSave', 1)\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cNewInitOwnerCaption = oNew.cInitOwnerCaption\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lNewChildHasParent = PEMSTATUS(oNew.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 18)\n"
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
               std::string("external-base inherited ADDOBJECT script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lnewhaschild", "true");
        check("ccreateinitchildcaption", "Save");
        check("cnewinitchildcaption", "Save");
        check("ccreateinitownercaption", "MainForm");
        check("cnewinitownercaption", "MainForm");
        check("ccreatechildcaption", "Save");
        check("cnewchildcaption", "Save");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("lcreatechildhasparent", "true");
        check("lnewchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "18");

        expect(state.ole_objects.size() == 5U,
               "external-base inherited ADDOBJECT script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &new_parent = state.ole_objects[2];
            const auto &new_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "external-base inherited ADDOBJECT should preserve CREATEOBJECT parent class identity");
            expect(create_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT should materialize the inherited child class");
            expect(create_child.source == library_path.string(),
                   "external-base inherited ADDOBJECT should resolve child classes against the inherited method's defining external PRG");
            expect(new_parent.prog_id == "LeafForm",
                   "external-base inherited ADDOBJECT should preserve NEWOBJECT leaf class identity");
            expect(new_child.prog_id == "SaveButton",
                   "external-base inherited ADDOBJECT should materialize inherited children for leaf instances");
            expect(new_child.source == library_path.string(),
                   "external-base inherited ADDOBJECT should preserve external PRG provenance for leaf child instances");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while external-base inherited ADDOBJECT lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "external-base inherited ADDOBJECT should emit child materialization events");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_addobject_materializes_child_declared_only_in_derived_startup_prg()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_addobject_derived_startup_child";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

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

        const fs::path main_path = temp_root / "inherited_external_parent_addobject_derived_startup_child.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "lCreateHasChild = PEMSTATUS(oCreate, 'cmdSave', 1)\n"
            "lLeafHasChild = PEMSTATUS(oLeaf, 'cmdSave', 1)\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cLeafInitChildCaption = oLeaf.cInitChildCaption\n"
            "cCreateInitOwnerCaption = oCreate.cInitOwnerCaption\n"
            "cLeafInitOwnerCaption = oLeaf.cInitOwnerCaption\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cLeafChildCaption = oLeaf.cmdSave.Caption\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeaf.cmdSave.OwnerCaption()\n"
            "cCreateSavedCaption = oCreate.cmdSave.TriggerSave()\n"
            "cLeafSavedCaption = oLeaf.cmdSave.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "cLeafChildClass = oLeaf.cmdSave.Class\n"
            "cLeafChildBaseClass = oLeaf.cmdSave.BaseClass\n"
            "cLeafChildParentClass = oLeaf.cmdSave.ParentClass\n"
            "xLeafChildClassLibrary = oLeaf.cmdSave.ClassLibrary\n"
            "cLeafChildClassReflect = GETPEM(oLeaf.cmdSave, 'Class')\n"
            "cLeafChildBaseClassReflect = GETPEM(oLeaf.cmdSave, 'BaseClass')\n"
            "cLeafChildParentClassReflect = GETPEM(oLeaf.cmdSave, 'ParentClass')\n"
            "xLeafChildClassLibraryReflect = GETPEM(oLeaf.cmdSave, 'ClassLibrary')\n"
            "lLeafChildHasClass = PEMSTATUS(oLeaf.cmdSave, 'Class', 1)\n"
            "lLeafChildHasBaseClass = PEMSTATUS(oLeaf.cmdSave, 'BaseClass', 1)\n"
            "lLeafChildHasParentClass = PEMSTATUS(oLeaf.cmdSave, 'ParentClass', 1)\n"
            "lLeafChildHasClassLibrary = PEMSTATUS(oLeaf.cmdSave, 'ClassLibrary', 1)\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeaf.cmdSave, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeaf.cmdSave, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeaf.cmdSave, 'ParentClass', 5)\n"
            "nLeafMembersProps = AMEMBERS(aLeafMembersProps, oLeaf.cmdSave, 1)\n"
            "nLeafMembersUnion = AMEMBERS(aLeafMembersUnion, oLeaf.cmdSave, 3)\n"
            "cLeafProp1 = aLeafMembersProps[1]\n"
            "cLeafProp2 = aLeafMembersProps[2]\n"
            "cLeafProp3 = aLeafMembersProps[3]\n"
            "cLeafProp4 = aLeafMembersProps[4]\n"
            "cLeafProp5 = aLeafMembersProps[5]\n"
            "nLeafClassCount = ACLASS(aLeafClass, oLeaf.cmdSave)\n"
            "cLeafClass1 = aLeafClass[1]\n"
            "cLeafClass2 = aLeafClass[2]\n"
            "cLeafClass3 = aLeafClass[3]\n"
            "lCreateChildHasParent = PEMSTATUS(oCreate.cmdSave, 'Parent', 1)\n"
            "lLeafChildHasParent = PEMSTATUS(oLeaf.cmdSave, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 126)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent ADDOBJECT derived-startup child script should complete: ") + state.message +
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

        check("lcreatehaschild", "true");
        check("lleafhaschild", "true");
        check("ccreateinitchildcaption", "Save");
        check("cleafinitchildcaption", "Save");
        check("ccreateinitownercaption", "MainForm");
        check("cleafinitownercaption", "MainForm");
        check("ccreatechildcaption", "Save");
        check("cleafchildcaption", "Save");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("cleafchildclass", "SaveButton");
        check("cleafchildbaseclass", "Custom");
        check("cleafchildparentclass", "Custom");
        check("cleafchildclassreflect", "SaveButton");
        check("cleafchildbaseclassreflect", "Custom");
        check("cleafchildparentclassreflect", "Custom");
        check("lleafchildhasclass", "true");
        check("lleafchildhasbaseclass", "true");
        check("lleafchildhasparentclass", "true");
        check("lleafchildhasclasslibrary", "false");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("nleafmembersprops", "5");
        check("cleafprop1", "BASECLASS");
        check("cleafprop2", "CAPTION");
        check("cleafprop3", "CLASS");
        check("cleafprop4", "PARENT");
        check("cleafprop5", "PARENTCLASS");
        check("nleafclasscount", "3");
        check("cleafclass1", "SAVEBUTTON");
        check("cleafclass2", "CUSTOM");
        check("cleafclass3", "OBJECT");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "126");

        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child should leave the leaf child ClassLibrary empty on ordinary reads");

        const auto leaf_child_class_library_reflect = state.globals.find("xleafchildclasslibraryreflect");
        expect(leaf_child_class_library_reflect != state.globals.end() &&
                   leaf_child_class_library_reflect->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child should leave the leaf child ClassLibrary empty through GETPEM()");

        const auto leaf_members_union = state.globals.find("nleafmembersunion");
        expect(leaf_members_union != state.globals.end() &&
                   std::stoi(copperfin::runtime::format_value(leaf_members_union->second)) >= 5,
               "inherited external-parent ADDOBJECT derived-startup child should keep leaf union member enumeration including inherited child methods");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent ADDOBJECT derived-startup child script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent ADDOBJECT derived-startup child should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child should materialize the CREATEOBJECT child");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child should resolve the CREATEOBJECT child against the derived startup PRG when the external parent library lacks the class");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent ADDOBJECT derived-startup child should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child should materialize leaf inherited children");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child should preserve derived-startup provenance for leaf child instances");
            expect(leaf_child.base_class_name == "Custom",
                   "inherited external-parent ADDOBJECT derived-startup child should preserve the leaf child immediate builtin base");
            expect(leaf_child.class_hierarchy.size() == 3U,
                   "inherited external-parent ADDOBJECT derived-startup child should preserve the leaf child class hierarchy");
            if (leaf_child.class_hierarchy.size() == 3U)
            {
                expect(leaf_child.class_hierarchy[0] == "SAVEBUTTON",
                       "inherited external-parent ADDOBJECT derived-startup child should store the leaf child class first");
                expect(leaf_child.class_hierarchy[1] == "CUSTOM",
                       "inherited external-parent ADDOBJECT derived-startup child should store the leaf builtin base second");
                expect(leaf_child.class_hierarchy[2] == "OBJECT",
                       "inherited external-parent ADDOBJECT derived-startup child should store the leaf terminal object token third");
            }
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent ADDOBJECT derived-startup child should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent ADDOBJECT derived-startup child activation lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent ADDOBJECT derived-startup child should emit child materialization events");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent ADDOBJECT derived-startup child should keep materialized children fully usable through THISFORM owner dispatch");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_addobject_derived_startup_child_identity_metadata_resists_mutation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_addobject_derived_startup_child_identity";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_addobject_derived_startup_child_identity.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oLeaf = NEWOBJECT('LeafForm')\n"
            "oCreateChild = oCreate.cmdSave\n"
            "oLeafChild = oLeaf.cmdSave\n"
            "lCreateChildClassReadOnly = PEMSTATUS(oCreateChild, 'Class', 5)\n"
            "lCreateChildBaseClassReadOnly = PEMSTATUS(oCreateChild, 'BaseClass', 5)\n"
            "lCreateChildParentClassReadOnly = PEMSTATUS(oCreateChild, 'ParentClass', 5)\n"
            "lSetCreateChildClass = SETPEM(oCreateChild, 'Class', 'OtherClass')\n"
            "lSetCreateChildBaseClass = SETPEM(oCreateChild, 'BaseClass', 'OtherBase')\n"
            "lSetCreateChildParentClass = SETPEM(oCreateChild, 'ParentClass', 'OtherParent')\n"
            "lSetCreateChildClassLibrary = SETPEM(oCreateChild, 'ClassLibrary', 'other.prg')\n"
            "lAddCreateChildClass = ADDPROPERTY(oCreateChild, 'Class', 'OtherClass')\n"
            "lAddCreateChildBaseClass = ADDPROPERTY(oCreateChild, 'BaseClass', 'OtherBase')\n"
            "lAddCreateChildParentClass = ADDPROPERTY(oCreateChild, 'ParentClass', 'OtherParent')\n"
            "lAddCreateChildClassLibrary = ADDPROPERTY(oCreateChild, 'ClassLibrary', 'other.prg')\n"
            "cCreateChildClassAfter = oCreateChild.Class\n"
            "cCreateChildBaseClassAfter = oCreateChild.BaseClass\n"
            "cCreateChildParentClassAfter = oCreateChild.ParentClass\n"
            "xCreateChildClassLibraryAfter = oCreateChild.ClassLibrary\n"
            "cCreateChildClassReflectAfter = GETPEM(oCreateChild, 'Class')\n"
            "cCreateChildBaseClassReflectAfter = GETPEM(oCreateChild, 'BaseClass')\n"
            "cCreateChildParentClassReflectAfter = GETPEM(oCreateChild, 'ParentClass')\n"
            "xCreateChildClassLibraryReflectAfter = GETPEM(oCreateChild, 'ClassLibrary')\n"
            "lRemoveLeafChildClass = REMOVEPROPERTY(oLeafChild, 'Class')\n"
            "lRemoveLeafChildBaseClass = REMOVEPROPERTY(oLeafChild, 'BaseClass')\n"
            "lRemoveLeafChildParentClass = REMOVEPROPERTY(oLeafChild, 'ParentClass')\n"
            "lRemoveLeafChildClassLibrary = REMOVEPROPERTY(oLeafChild, 'ClassLibrary')\n"
            "oLeafChild.Class = 'OtherClass'\n"
            "oLeafChild.BaseClass = 'OtherBase'\n"
            "oLeafChild.ParentClass = 'OtherParent'\n"
            "oLeafChild.ClassLibrary = 'other.prg'\n"
            "lLeafChildClassReadOnly = PEMSTATUS(oLeafChild, 'Class', 5)\n"
            "lLeafChildBaseClassReadOnly = PEMSTATUS(oLeafChild, 'BaseClass', 5)\n"
            "lLeafChildParentClassReadOnly = PEMSTATUS(oLeafChild, 'ParentClass', 5)\n"
            "cLeafChildClassAfter = oLeafChild.Class\n"
            "cLeafChildBaseClassAfter = oLeafChild.BaseClass\n"
            "cLeafChildParentClassAfter = oLeafChild.ParentClass\n"
            "xLeafChildClassLibraryAfter = oLeafChild.ClassLibrary\n"
            "cLeafChildClassReflectAfter = GETPEM(oLeafChild, 'Class')\n"
            "cLeafChildBaseClassReflectAfter = GETPEM(oLeafChild, 'BaseClass')\n"
            "cLeafChildParentClassReflectAfter = GETPEM(oLeafChild, 'ParentClass')\n"
            "xLeafChildClassLibraryReflectAfter = GETPEM(oLeafChild, 'ClassLibrary')\n"
            "cCreateOwnerCaption = oCreateChild.OwnerCaption()\n"
            "cLeafOwnerCaption = oLeafChild.OwnerCaption()\n"
            "cCreateSavedCaption = oCreateChild.TriggerSave()\n"
            "cLeafSavedCaption = oLeafChild.TriggerSave()\n"
            "cCreateCaptionAfterSave = oCreate.Caption\n"
            "cLeafCaptionAfterSave = oLeaf.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 130)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent ADDOBJECT derived-startup child identity script should complete: ") + state.message +
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

        check("lcreatechildclassreadonly", "true");
        check("lcreatechildbaseclassreadonly", "true");
        check("lcreatechildparentclassreadonly", "true");
        check("lsetcreatechildclass", "false");
        check("lsetcreatechildbaseclass", "false");
        check("lsetcreatechildparentclass", "false");
        check("lsetcreatechildclasslibrary", "false");
        check("laddcreatechildclass", "false");
        check("laddcreatechildbaseclass", "false");
        check("laddcreatechildparentclass", "false");
        check("laddcreatechildclasslibrary", "false");
        check("ccreatechildclassafter", "SaveButton");
        check("ccreatechildbaseclassafter", "Custom");
        check("ccreatechildparentclassafter", "Custom");
        check("ccreatechildclassreflectafter", "SaveButton");
        check("ccreatechildbaseclassreflectafter", "Custom");
        check("ccreatechildparentclassreflectafter", "Custom");
        check("lremoveleafchildclass", "false");
        check("lremoveleafchildbaseclass", "false");
        check("lremoveleafchildparentclass", "false");
        check("lremoveleafchildclasslibrary", "false");
        check("lleafchildclassreadonly", "true");
        check("lleafchildbaseclassreadonly", "true");
        check("lleafchildparentclassreadonly", "true");
        check("cleafchildclassafter", "SaveButton");
        check("cleafchildbaseclassafter", "Custom");
        check("cleafchildparentclassafter", "Custom");
        check("cleafchildclassreflectafter", "SaveButton");
        check("cleafchildbaseclassreflectafter", "Custom");
        check("cleafchildparentclassreflectafter", "Custom");
        check("ccreateownercaption", "MainForm");
        check("cleafownercaption", "MainForm");
        check("ccreatesavedcaption", "MainForm-Saved");
        check("cleafsavedcaption", "MainForm-Saved");
        check("ccreatecaptionaftersave", "MainForm-Saved");
        check("cleafcaptionaftersave", "MainForm-Saved");
        check("ldictset", "true");
        check("ndictcompare", "130");

        const auto create_child_class_library_after = state.globals.find("xcreatechildclasslibraryafter");
        expect(create_child_class_library_after != state.globals.end() &&
                   create_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child identity should leave the CREATEOBJECT child ClassLibrary empty after failed mutation");

        const auto create_child_class_library_reflect_after = state.globals.find("xcreatechildclasslibraryreflectafter");
        expect(create_child_class_library_reflect_after != state.globals.end() &&
                   create_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child identity should leave the CREATEOBJECT child ClassLibrary empty through GETPEM() after failed mutation");

        const auto leaf_child_class_library_after = state.globals.find("xleafchildclasslibraryafter");
        expect(leaf_child_class_library_after != state.globals.end() &&
                   leaf_child_class_library_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child identity should leave the leaf child ClassLibrary empty after rejected mutation");

        const auto leaf_child_class_library_reflect_after = state.globals.find("xleafchildclasslibraryreflectafter");
        expect(leaf_child_class_library_reflect_after != state.globals.end() &&
                   leaf_child_class_library_reflect_after->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child identity should leave the leaf child ClassLibrary empty through GETPEM() after rejected mutation");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent ADDOBJECT derived-startup child identity should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve CREATEOBJECT parent identity");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "Custom",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve the CREATEOBJECT child builtin base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve CREATEOBJECT child source identity");
            expect(create_child.class_library.empty(),
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve empty CREATEOBJECT child ClassLibrary semantics");
            expect(create_child.class_hierarchy.size() == 3U,
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve CREATEOBJECT child class hierarchy");
            expect(!create_child.properties.contains("class"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a CREATEOBJECT child Class shadow");
            expect(!create_child.properties.contains("baseclass"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a CREATEOBJECT child BaseClass shadow");
            expect(!create_child.properties.contains("parentclass"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a CREATEOBJECT child ParentClass shadow");
            expect(!create_child.properties.contains("classlibrary"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a CREATEOBJECT child ClassLibrary shadow");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve NEWOBJECT leaf identity");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve leaf child identity");
            expect(leaf_child.base_class_name == "Custom",
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve the leaf child builtin base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve leaf child source identity");
            expect(leaf_child.class_library.empty(),
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve empty leaf child ClassLibrary semantics");
            expect(leaf_child.class_hierarchy.size() == 3U,
                   "inherited external-parent ADDOBJECT derived-startup child identity should preserve leaf child class hierarchy");
            expect(!leaf_child.properties.contains("class"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a leaf child Class shadow");
            expect(!leaf_child.properties.contains("baseclass"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a leaf child BaseClass shadow");
            expect(!leaf_child.properties.contains("parentclass"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a leaf child ParentClass shadow");
            expect(!leaf_child.properties.contains("classlibrary"),
                   "inherited external-parent ADDOBJECT derived-startup child identity should not materialize a leaf child ClassLibrary shadow");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent ADDOBJECT derived-startup child identity lands");
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent ADDOBJECT derived-startup child identity should keep child owner dispatch usable after rejected mutations");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_addobject_derived_startup_child_dotted_access_resolves_live_child_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_addobject_derived_startup_child_dotted";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path widget_library_path = temp_root / "widgetlib.prg";
        write_text(
            widget_library_path,
            "DEFINE CLASS ParentForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_addobject_derived_startup_child_dotted.prg";
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
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 138)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LeafForm AS ChildForm\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent ADDOBJECT derived-startup child dotted script should complete: ") + state.message +
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
        check("ccreatechildbaseclass", "Custom");
        check("cleafchildbaseclass", "Custom");
        check("lcreatechildhasparent", "true");
        check("lleafchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "138");

        const auto create_child_class_library = state.globals.find("xcreatechildclasslibrary");
        expect(create_child_class_library != state.globals.end() &&
                   create_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child dotted access should leave the CREATEOBJECT child ClassLibrary empty");

        const auto leaf_child_class_library = state.globals.find("xleafchildclasslibrary");
        expect(leaf_child_class_library != state.globals.end() &&
                   leaf_child_class_library->second.kind == copperfin::runtime::PrgValueKind::empty,
               "inherited external-parent ADDOBJECT derived-startup child dotted access should leave the leaf child ClassLibrary empty");

        expect(state.ole_objects.size() == 5U,
               "inherited external-parent ADDOBJECT derived-startup child dotted script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            const auto &create_parent = state.ole_objects[0];
            const auto &create_child = state.ole_objects[1];
            const auto &leaf_parent = state.ole_objects[2];
            const auto &leaf_child = state.ole_objects[3];
            expect(create_parent.prog_id == "ChildForm",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve CREATEOBJECT parent identity");
            expect(create_parent.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve CREATEOBJECT parent source");
            expect(create_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve CREATEOBJECT child identity");
            expect(create_child.base_class_name == "Custom",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the CREATEOBJECT child builtin base");
            expect(create_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve CREATEOBJECT child derived-startup source identity");
            expect(create_child.class_library.empty(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve empty CREATEOBJECT child ClassLibrary semantics");
            expect(leaf_parent.prog_id == "LeafForm",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve NEWOBJECT leaf identity");
            expect(leaf_parent.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve NEWOBJECT leaf source");
            expect(leaf_child.prog_id == "SaveButton",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve leaf child identity");
            expect(leaf_child.base_class_name == "Custom",
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the leaf child builtin base");
            expect(leaf_child.source == main_path.string(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve leaf child derived-startup source identity");
            expect(leaf_child.class_library.empty(),
                   "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve empty leaf child ClassLibrary semantics");

            const auto create_parent_caption = create_parent.properties.find("caption");
            const auto create_child_caption = create_child.properties.find("caption");
            const auto leaf_parent_caption = leaf_parent.properties.find("caption");
            const auto leaf_child_caption = leaf_child.properties.find("caption");
            if (create_parent_caption != create_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(create_parent_caption->second) == "Done-Saved",
                       "inherited external-parent ADDOBJECT derived-startup child dotted access should let dotted child method calls update the CREATEOBJECT owner");
            }
            else
            {
                expect(false, "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the CREATEOBJECT owner caption");
            }
            if (create_child_caption != create_child.properties.end())
            {
                expect(copperfin::runtime::format_value(create_child_caption->second) == "Go",
                       "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the CREATEOBJECT child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the CREATEOBJECT child caption");
            }
            if (leaf_parent_caption != leaf_parent.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_parent_caption->second) == "Ready-Saved",
                       "inherited external-parent ADDOBJECT derived-startup child dotted access should let dotted child method calls update the leaf owner");
            }
            else
            {
                expect(false, "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the leaf owner caption");
            }
            if (leaf_child_caption != leaf_child.properties.end())
            {
                expect(copperfin::runtime::format_value(leaf_child_caption->second) == "Ship",
                       "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the leaf child dotted assignment");
            }
            else
            {
                expect(false, "inherited external-parent ADDOBJECT derived-startup child dotted access should preserve the leaf child caption");
            }
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while inherited external-parent ADDOBJECT derived-startup child dotted access lands");
        }

        const bool has_addobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.addobject" &&
                   (event.detail == "ChildForm.cmdsave:SaveButton" ||
                    event.detail == "LeafForm.cmdsave:SaveButton");
        });
        expect(has_addobject_event,
               "inherited external-parent ADDOBJECT derived-startup child dotted access should emit child materialization events");

        const bool has_owner_caption_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "SaveButton.OwnerCaption";
        });
        expect(has_owner_caption_invoke_event,
               "inherited external-parent ADDOBJECT derived-startup child dotted access should dispatch direct dotted child method calls");

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent ADDOBJECT derived-startup child dotted access should keep child THISFORM owner dispatch stable");

        fs::remove_all(temp_root, ignored);
    }

}
