#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_inherited_external_parent_child_methods_resolve_thisformset_to_live_derived_owner()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_thisformset";
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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORMSET.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION OwnerRef\n"
            "        RETURN THISFORMSET\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_thisformset.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cFormCaptionAfterSave = oCreate.Caption\n"
            "oOwnerRef = oChild.OwnerRef()\n"
            "cOwnerRefCaption = oOwnerRef.Caption\n"
            "cOwnerRefClass = oOwnerRef.Class\n"
            "oOwnerRef.Caption = oOwnerRef.Caption + '-Ref'\n"
            "cFormCaptionAfterRef = oCreate.Caption\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent THISFORMSET script should complete: ") + state.message +
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

        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("cownerrefcaption", "MainForm-Saved");
        check("cownerrefclass", "ChildForm");
        check("cformcaptionafterref", "MainForm-Saved-Ref");

        expect(state.ole_objects.size() == 2U,
               "inherited external-parent THISFORMSET script should register derived owner and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited external-parent THISFORMSET should preserve the live derived owner identity");
            expect(owner_object.source == main_path.string(),
                   "inherited external-parent THISFORMSET should preserve the derived owner source path");
            expect(child_object.prog_id == "SaveButton",
                   "inherited external-parent THISFORMSET should preserve the child object identity");
            expect(child_object.source == widget_library_path.string(),
                   "inherited external-parent THISFORMSET should preserve the external parent-library child source path");
            const auto caption = owner_object.properties.find("caption");
            if (caption != owner_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved-Ref",
                       "inherited external-parent THISFORMSET should let child methods and returned owner refs mutate the live derived owner");
            }
            else
            {
                expect(false, "inherited external-parent THISFORMSET should preserve the derived owner caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent THISFORMSET should route child method calls into the inherited owner Save method");

        fs::remove_all(temp_root, ignored);
    }

    void test_external_prg_base_child_methods_resolve_parent_thisform_and_thisformset()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_external_prg_child_method_ownership";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "    FUNCTION ParentCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormCaption\n"
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormsetCaption\n"
            "        RETURN THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormsetSave\n"
            "        RETURN THISFORMSET.Save()\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "external_child_method_ownership.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cParentCaption = oChild.ParentCaption()\n"
            "cFormCaption = oChild.FormCaption()\n"
            "cFormsetCaption = oChild.FormsetCaption()\n"
            "cSavedCaption = oChild.TriggerFormSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "cSavedCaption2 = oChild.TriggerFormsetSave()\n"
            "cFormCaptionAfterSave2 = oForm.Caption\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("external-base child ownership script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("cformcaption", "MainForm");
        check("cformsetcaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("csavedcaption2", "MainForm-Saved-Saved");
        check("cformcaptionaftersave2", "MainForm-Saved-Saved");

        expect(state.ole_objects.size() == 2U,
               "external-base child ownership script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(parent_object.prog_id == "DemoForm",
                   "external-base child ownership should preserve parent class identity");
            expect(child_object.prog_id == "SaveButton",
                   "external-base child ownership should preserve child class identity");
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved-Saved",
                       "external-base child ownership should let inherited child methods update the owning form");
            }
            else
            {
                expect(false, "external-base child ownership should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "external-base child ownership should route inherited child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_external_base_child_methods_resolve_thisform_and_thisformset_to_live_derived_owner()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_external_base_child_ownership";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path button_library_path = temp_root / "buttons.prg";
        write_text(
            button_library_path,
            "DEFINE CLASS ParentButton AS Custom\n"
            "    FUNCTION ParentCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormCaption\n"
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION FormsetCaption\n"
            "        RETURN THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerFormsetSave\n"
            "        RETURN THISFORMSET.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION FormOwnerRef\n"
            "        RETURN THISFORM\n"
            "    ENDFUNC\n"
            "    FUNCTION FormsetOwnerRef\n"
            "        RETURN THISFORMSET\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

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
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS ParentButton OF buttons.prg\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_external_base_child_ownership.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('ChildForm')\n"
            "oChild = oCreate.cmdSave\n"
            "cParentCaption = oChild.ParentCaption()\n"
            "cFormCaption = oChild.FormCaption()\n"
            "cFormsetCaption = oChild.FormsetCaption()\n"
            "cSavedCaption = oChild.TriggerFormSave()\n"
            "cFormCaptionAfterSave = oCreate.Caption\n"
            "cSavedCaption2 = oChild.TriggerFormsetSave()\n"
            "cFormCaptionAfterSave2 = oCreate.Caption\n"
            "oFormRef = oChild.FormOwnerRef()\n"
            "oFormsetRef = oChild.FormsetOwnerRef()\n"
            "cFormRefCaption = oFormRef.Caption\n"
            "cFormsetRefCaption = oFormsetRef.Caption\n"
            "cFormRefClass = oFormRef.Class\n"
            "cFormsetRefClass = oFormsetRef.Class\n"
            "oFormRef.Caption = oFormRef.Caption + '-Ref'\n"
            "cFormCaptionAfterRef = oCreate.Caption\n"
            "RETURN\n"
            "DEFINE CLASS ChildForm AS ParentForm OF widgetlib.prg\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("inherited external-parent external-base child ownership script should complete: ") + state.message +
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

        check("cparentcaption", "MainForm");
        check("cformcaption", "MainForm");
        check("cformsetcaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("csavedcaption2", "MainForm-Saved-Saved");
        check("cformcaptionaftersave2", "MainForm-Saved-Saved");
        check("cformrefcaption", "MainForm-Saved-Saved");
        check("cformsetrefcaption", "MainForm-Saved-Saved");
        check("cformrefclass", "ChildForm");
        check("cformsetrefclass", "ChildForm");
        check("cformcaptionafterref", "MainForm-Saved-Saved-Ref");

        expect(state.ole_objects.size() == 2U,
               "inherited external-parent external-base child ownership should register derived owner and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited external-parent external-base child ownership should preserve derived owner identity");
            expect(owner_object.source == main_path.string(),
                   "inherited external-parent external-base child ownership should preserve the derived owner source path");
            expect(child_object.prog_id == "SaveButton",
                   "inherited external-parent external-base child ownership should preserve child class identity");
            expect(child_object.source == widget_library_path.string(),
                   "inherited external-parent external-base child ownership should preserve the external parent-library child source path");
            expect(child_object.base_class_name == "ParentButton",
                   "inherited external-parent external-base child ownership should preserve the child immediate external base class identity");
            expect(child_object.class_library == button_library_path.string(),
                   "inherited external-parent external-base child ownership should preserve the child external base class library path");
            const auto caption = owner_object.properties.find("caption");
            if (caption != owner_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved-Saved-Ref",
                       "inherited external-parent external-base child ownership should let inherited external-base child methods mutate the live derived owner");
            }
            else
            {
                expect(false, "inherited external-parent external-base child ownership should preserve the derived owner caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent external-base child ownership should route inherited external-base child methods into the inherited owner Save method");

        fs::remove_all(temp_root, ignored);
    }

    void test_dotted_native_child_chains_traverse_contained_objects()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_chain";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_chain.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "cChildCaption = oForm.cmdSave.Caption\n"
            "oParent = oForm.cmdSave.Parent\n"
            "cParentCaption = oForm.cmdSave.Parent.Caption\n"
            "cOwnerCaption = oForm.cmdSave.OwnerCaption()\n"
            "cSavedCaption = oForm.cmdSave.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 14)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "    FUNCTION Save\n"
            "        THIS.Caption = THIS.Caption + '-Saved'\n"
            "        RETURN THIS.Caption\n"
            "    ENDFUNC\n"
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
               std::string("native child-chain script should complete: ") + state.message +
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

        check("cchildcaption", "Save");
        check("cparentcaption", "MainForm");
        check("cownercaption", "MainForm");
        check("csavedcaption", "MainForm-Saved");
        check("cformcaptionaftersave", "MainForm-Saved");
        check("ldictset", "true");
        check("ndictcompare", "14");

        expect(state.ole_objects.size() == 3U,
               "native child-chain script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto parent_caption = state.ole_objects[0].properties.find("caption");
            if (parent_caption != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(parent_caption->second) == "MainForm-Saved",
                       "native child-chain traversal should let dotted child method calls update the owning form");
            }
            else
            {
                expect(false, "native child-chain traversal should preserve the owning form caption");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native child-chain traversal lands");
        }

        const bool has_child_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "SaveButton.OwnerCaption";
        });
        expect(has_child_invoke_event,
               "native child-chain traversal should dispatch direct dotted child method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_dotted_native_child_assignments_traverse_contained_objects()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_child_chain_assignment";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_child_chain_assignment.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oForm.cmdSave.Caption = 'Go'\n"
            "oForm.cmdSave.Parent.Caption = 'Done'\n"
            "cChildCaption = oForm.cmdSave.Caption\n"
            "cParentCaption = oForm.Caption\n"
            "cOwnerCaption = oForm.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 15)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native child-chain assignment script should complete: ") + state.message +
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

        check("cchildcaption", "Go");
        check("cparentcaption", "Done");
        check("cownercaption", "Done");
        check("ldictset", "true");
        check("ndictcompare", "15");

        expect(state.ole_objects.size() == 3U,
               "native child-chain assignment script should register parent, child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto parent_caption = state.ole_objects[0].properties.find("caption");
            const auto child_caption = state.ole_objects[1].properties.find("caption");
            if (parent_caption != state.ole_objects[0].properties.end())
            {
                expect(copperfin::runtime::format_value(parent_caption->second) == "Done",
                       "native child-chain assignment should update the parent through the dotted chain");
            }
            else
            {
                expect(false, "native child-chain assignment should preserve the parent caption property");
            }
            if (child_caption != state.ole_objects[1].properties.end())
            {
                expect(copperfin::runtime::format_value(child_caption->second) == "Go",
                       "native child-chain assignment should update the contained child through the dotted chain");
            }
            else
            {
                expect(false, "native child-chain assignment should preserve the child caption property");
            }
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native child-chain assignment lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_removeobject_detaches_child_and_clears_parent_reference()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_removeobject";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_removeobject.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cChildCaptionBeforeRemove = oChild.Caption\n"
            "lRemoved = oForm.RemoveObject('cmdSave')\n"
            "lRemovedMissing = oForm.RemoveObject('cmdSave')\n"
            "lHasChildAfterRemove = PEMSTATUS(oForm, 'cmdSave', 1)\n"
            "xRemovedChild = GETPEM(oForm, 'cmdSave')\n"
            "lChildHasParentAfterRemove = PEMSTATUS(oChild, 'Parent', 1)\n"
            "cChildCaptionAfterRemove = oChild.Caption\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 16)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    PROCEDURE Init\n"
            "        THIS.AddObject('cmdSave', 'SaveButton')\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native REMOVEOBJECT script should complete: ") + state.message +
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

        check("cchildcaptionbeforeremove", "Save");
        check("lremoved", "true");
        check("lremovedmissing", "false");
        check("lhaschildafterremove", "false");
        check("lchildhasparentafterremove", "false");
        check("cchildcaptionafterremove", "Save");
        check("ldictset", "true");
        check("ndictcompare", "16");

        const auto removed_child = state.globals.find("xremovedchild");
        expect(removed_child != state.globals.end() &&
                   removed_child->second.kind == copperfin::runtime::PrgValueKind::empty,
               "native REMOVEOBJECT should make GETPEM() return empty for the removed child");

        expect(state.ole_objects.size() == 3U,
               "native REMOVEOBJECT script should register parent, detached child, and COM objects");
        if (state.ole_objects.size() == 3U)
        {
            expect(!state.ole_objects[0].properties.contains("cmdsave"),
                   "native REMOVEOBJECT should remove the child reference from the parent");
            expect(!state.ole_objects[1].properties.contains("parent"),
                   "native REMOVEOBJECT should clear the detached child's parent reference");
            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while native REMOVEOBJECT lands");
        }

        const bool has_removeobject_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.removeobject" &&
                   event.detail == "DemoForm.cmdsave";
        });
        expect(has_removeobject_event,
               "native REMOVEOBJECT should emit detachment events");

        fs::remove_all(temp_root, ignored);
    }

}
