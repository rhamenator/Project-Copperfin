#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_class_body_add_object_with_property_clauses_materialize_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_addobject_with";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_addobject_with.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "nCreateInitChildPriority = oCreate.nInitChildPriority\n"
            "nNewInitChildPriority = oNew.nInitChildPriority\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "nCreateChildPriority = oCreate.cmdSave.nPriority\n"
            "nNewChildPriority = oNew.cmdSave.nPriority\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    nInitChildPriority = 0\n"
            "    ADD OBJECT cmdSave AS SaveButton WITH Caption = 'Commit', nPriority = 7\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.nInitChildPriority = THIS.cmdSave.nPriority\n"
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
               std::string("native class-body ADD OBJECT WITH script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ncreateinitchildpriority", "7");
        check("nnewinitchildpriority", "7");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ncreatechildpriority", "7");
        check("nnewchildpriority", "7");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body ADD OBJECT WITH script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body ADD OBJECT WITH should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body ADD OBJECT WITH should materialize the NEWOBJECT child class");
            const auto create_child_caption = state.ole_objects[1].properties.find("caption");
            const auto create_child_priority = state.ole_objects[1].properties.find("npriority");
            const auto new_child_caption = state.ole_objects[3].properties.find("caption");
            const auto new_child_priority = state.ole_objects[3].properties.find("npriority");
            expect(create_child_caption != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                   "class-body ADD OBJECT WITH should apply caption override to the CREATEOBJECT child");
            expect(create_child_priority != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_priority->second) == "7",
                   "class-body ADD OBJECT WITH should apply numeric override to the CREATEOBJECT child");
            expect(new_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                   "class-body ADD OBJECT WITH should apply caption override to the NEWOBJECT child");
            expect(new_child_priority != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_priority->second) == "7",
                   "class-body ADD OBJECT WITH should apply numeric override to the NEWOBJECT child");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body ADD OBJECT WITH lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_class_body_object_blocks_materialize_children_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_class_body_object_block";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_class_body_object_block.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('DemoForm')\n"
            "oNew = NEWOBJECT('DemoForm')\n"
            "cCreateInitChildCaption = oCreate.cInitChildCaption\n"
            "cNewInitChildCaption = oNew.cInitChildCaption\n"
            "nCreateInitChildPriority = oCreate.nInitChildPriority\n"
            "nNewInitChildPriority = oNew.nInitChildPriority\n"
            "cCreateChildCaption = oCreate.cmdSave.Caption\n"
            "cNewChildCaption = oNew.cmdSave.Caption\n"
            "nCreateChildPriority = oCreate.cmdSave.nPriority\n"
            "nNewChildPriority = oNew.cmdSave.nPriority\n"
            "cCreateOwnerCaption = oCreate.cmdSave.OwnerCaption()\n"
            "cNewOwnerCaption = oNew.cmdSave.OwnerCaption()\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoForm AS Custom\n"
            "    Caption = 'MainForm'\n"
            "    cInitChildCaption = ''\n"
            "    nInitChildPriority = 0\n"
            "    OBJECT cmdSave AS SaveButton\n"
            "        Caption = 'Commit'\n"
            "        nPriority = 7\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        THIS.nInitChildPriority = THIS.cmdSave.nPriority\n"
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
               std::string("native class-body OBJECT block script should complete: ") + state.message +
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

        check("ccreateinitchildcaption", "Commit");
        check("cnewinitchildcaption", "Commit");
        check("ncreateinitchildpriority", "7");
        check("nnewinitchildpriority", "7");
        check("ccreatechildcaption", "Commit");
        check("cnewchildcaption", "Commit");
        check("ncreatechildpriority", "7");
        check("nnewchildpriority", "7");
        check("ccreateownercaption", "MainForm");
        check("cnewownercaption", "MainForm");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "native class-body OBJECT block script should register CREATEOBJECT, NEWOBJECT, child, child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "class-body OBJECT block should materialize the CREATEOBJECT child class");
            expect(state.ole_objects[3].prog_id == "SaveButton",
                   "class-body OBJECT block should materialize the NEWOBJECT child class");
            const auto create_child_caption = state.ole_objects[1].properties.find("caption");
            const auto create_child_priority = state.ole_objects[1].properties.find("npriority");
            const auto new_child_caption = state.ole_objects[3].properties.find("caption");
            const auto new_child_priority = state.ole_objects[3].properties.find("npriority");
            expect(create_child_caption != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_caption->second) == "Commit",
                   "class-body OBJECT block should apply caption override to the CREATEOBJECT child");
            expect(create_child_priority != state.ole_objects[1].properties.end() &&
                       copperfin::runtime::format_value(create_child_priority->second) == "7",
                   "class-body OBJECT block should apply numeric override to the CREATEOBJECT child");
            expect(new_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_caption->second) == "Commit",
                   "class-body OBJECT block should apply caption override to the NEWOBJECT child");
            expect(new_child_priority != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(new_child_priority->second) == "7",
                   "class-body OBJECT block should apply numeric override to the NEWOBJECT child");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while class-body OBJECT block lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_declarative_children_materialize_from_external_prg_sources_before_parent_init()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_declarative_external_children";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path library_path = temp_root / "buttons.prg";
        write_text(
            library_path,
            "DEFINE CLASS SaveButton AS Custom\n"
            "    Caption = 'Save'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS ArchiveButton AS Custom\n"
            "    Caption = 'Archive'\n"
            "    FUNCTION OwnerCaption\n"
            "        RETURN PARENT.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "native_declarative_external_children.prg";
        write_text(
            main_path,
            "oAdd = CREATEOBJECT('DemoFormAdd')\n"
            "oBlock = NEWOBJECT('DemoFormBlock')\n"
            "cAddInitChildCaption = oAdd.cInitChildCaption\n"
            "cBlockInitChildCaption = oBlock.cInitChildCaption\n"
            "cAddChildCaption = oAdd.cmdSave.Caption\n"
            "cBlockChildCaption = oBlock.cmdArchive.Caption\n"
            "cAddOwnerCaption = oAdd.cmdSave.OwnerCaption()\n"
            "cBlockOwnerCaption = oBlock.cmdArchive.OwnerCaption()\n"
            "lAddChildHasParent = PEMSTATUS(oAdd.cmdSave, 'Parent', 1)\n"
            "lBlockChildHasParent = PEMSTATUS(oBlock.cmdArchive, 'Parent', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 13)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS DemoFormAdd AS Custom\n"
            "    Caption = 'MainFormAdd'\n"
            "    cInitChildCaption = ''\n"
            "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdSave.Caption\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS DemoFormBlock AS Custom\n"
            "    Caption = 'MainFormBlock'\n"
            "    cInitChildCaption = ''\n"
            "    OBJECT cmdArchive AS ArchiveButton OF buttons.prg\n"
            "        Caption = 'ArchiveNow'\n"
            "    ENDOBJECT\n"
            "    PROCEDURE Init\n"
            "        THIS.cInitChildCaption = THIS.cmdArchive.Caption\n"
            "        RETURN\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("declarative external child script should complete: ") + state.message +
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

        check("caddinitchildcaption", "Save");
        check("cblockinitchildcaption", "ArchiveNow");
        check("caddchildcaption", "Save");
        check("cblockchildcaption", "ArchiveNow");
        check("caddownercaption", "MainFormAdd");
        check("cblockownercaption", "MainFormBlock");
        check("laddchildhasparent", "true");
        check("lblockchildhasparent", "true");
        check("ldictset", "true");
        check("ndictcompare", "13");

        expect(state.ole_objects.size() == 5U,
               "declarative external child script should register CREATEOBJECT parent/child, NEWOBJECT parent/child, and COM objects");
        if (state.ole_objects.size() == 5U)
        {
            expect(state.ole_objects[0].prog_id == "DemoFormAdd",
                   "declarative external child script should preserve the ADD parent class identity");
            expect(state.ole_objects[1].prog_id == "SaveButton",
                   "declarative external child script should materialize the one-line external child class");
            expect(state.ole_objects[1].source == library_path.string(),
                   "declarative external child script should preserve the resolved one-line external child source path");
            expect(state.ole_objects[2].prog_id == "DemoFormBlock",
                   "declarative external child script should preserve the block parent class identity");
            expect(state.ole_objects[3].prog_id == "ArchiveButton",
                   "declarative external child script should materialize the block external child class");
            expect(state.ole_objects[3].source == library_path.string(),
                   "declarative external child script should preserve the resolved block external child source path");
            const auto block_child_caption = state.ole_objects[3].properties.find("caption");
            expect(block_child_caption != state.ole_objects[3].properties.end() &&
                       copperfin::runtime::format_value(block_child_caption->second) == "ArchiveNow",
                   "declarative external child script should still apply block child property overrides from the parent PRG");
            expect(state.ole_objects[4].prog_id == "Scripting.Dictionary",
                   "COM behavior should remain stable while declarative external child activation lands");
        }

        fs::remove_all(temp_root, ignored);
    }

    void test_native_child_methods_resolve_thisform_through_parent_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_thisform";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_thisform.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oFormRef = oChild.OwnerRef()\n"
            "cOwnerRefCaption = oFormRef.Caption\n"
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
            "    FUNCTION OwnerCaption\n"
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION OwnerRef\n"
            "        RETURN THISFORM\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native THISFORM script should complete: ") + state.message +
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

        expect(state.ole_objects.size() == 2U,
               "native THISFORM script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved",
                       "native THISFORM should let child methods update the owning form");
            }
            else
            {
                expect(false, "native THISFORM should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native THISFORM should route child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_child_methods_resolve_thisformset_through_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_thisformset";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_thisformset.prg";
        write_text(
            main_path,
            "oForm = CREATEOBJECT('DemoForm')\n"
            "oChild = oForm.cmdSave\n"
            "cOwnerCaption = oChild.OwnerCaption()\n"
            "cSavedCaption = oChild.TriggerSave()\n"
            "cFormCaptionAfterSave = oForm.Caption\n"
            "oFormRef = oChild.OwnerRef()\n"
            "cOwnerRefCaption = oFormRef.Caption\n"
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

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native THISFORMSET script should complete: ") + state.message +
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

        expect(state.ole_objects.size() == 2U,
               "native THISFORMSET script should register parent and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &parent_object = state.ole_objects[0];
            const auto caption = parent_object.properties.find("caption");
            if (caption != parent_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved",
                       "native THISFORMSET should let child methods update the owning form");
            }
            else
            {
                expect(false, "native THISFORMSET should preserve the owning form caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "DemoForm.Save";
        });
        expect(has_save_invoke_event,
               "native THISFORMSET should route child method calls into parent methods");

        fs::remove_all(temp_root, ignored);
    }

    void test_formset_child_methods_distinguish_thisform_from_thisformset()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_formset_owner_chain";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_formset_owner_chain.prg";
        write_text(
            main_path,
            "oSet = CREATEOBJECT('MainFormSet')\n"
            "cOwnerSummary = oSet.frmWork.cmdSave.DescribeOwners()\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION DescribeOwners\n"
            "        cThisFormCaption = THISFORM.Caption\n"
            "        cThisFormBaseClass = THISFORM.BaseClass\n"
            "        cThisFormSetCaption = THISFORMSET.Caption\n"
            "        cThisFormSetBaseClass = THISFORMSET.BaseClass\n"
            "        cFormParentCaption = THISFORM.Parent.Caption\n"
            "        cFormParentBaseClass = THISFORM.Parent.BaseClass\n"
            "        cFormFromSetCaption = THISFORMSET.frmWork.Caption\n"
            "        RETURN THISFORM.Caption + '|' + THISFORMSET.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS WorkerForm AS Form\n"
            "    Caption = 'WorkerForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainFormSet AS FormSet\n"
            "    Caption = 'OwnerSet'\n"
            "    ADD OBJECT frmWork AS WorkerForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FormSet owner-chain script should complete: ") + state.message +
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

        check("cownersummary", "WorkerForm|OwnerSet");
        check("cthisformcaption", "WorkerForm");
        check("cthisformbaseclass", "Form");
        check("cthisformsetcaption", "OwnerSet");
        check("cthisformsetbaseclass", "FormSet");
        check("cformparentcaption", "OwnerSet");
        check("cformparentbaseclass", "FormSet");
        check("cformfromsetcaption", "WorkerForm");

        expect(state.ole_objects.size() == 3U,
               "native FormSet owner-chain script should register formset, child form, and child control");

        fs::remove_all(temp_root, ignored);
    }

    void test_formset_activeform_parent_and_release_thisformset_follow_owner_chain()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_formset_activeform";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_formset_activeform.prg";
        write_text(
            main_path,
            "oSet = CREATEOBJECT('MainFormSet')\n"
            "oSet.frmWork.cmdSave.SetFocus()\n"
            "cActiveBaseClass = _SCREEN.ActiveForm.BaseClass\n"
            "cActiveCaption = _SCREEN.ActiveForm.Caption\n"
            "cActiveParentBaseClass = _SCREEN.ActiveForm.Parent.BaseClass\n"
            "cActiveParentCaption = _SCREEN.ActiveForm.Parent.Caption\n"
            "lReleased = oSet.frmWork.cmdSave.CloseOwnerSet()\n"
            "lSetHasCaptionAfterRelease = PEMSTATUS(oSet, 'Caption', 1)\n"
            "RETURN\n"
            "DEFINE CLASS SaveButton AS CommandButton\n"
            "    FUNCTION CloseOwnerSet\n"
            "        RELEASE THISFORMSET\n"
            "        lMethodContinued = .T.\n"
            "        RETURN .T.\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS WorkerForm AS Form\n"
            "    Caption = 'WorkerForm'\n"
            "    ADD OBJECT cmdSave AS SaveButton\n"
            "ENDDEFINE\n"
            "DEFINE CLASS MainFormSet AS FormSet\n"
            "    Caption = 'OwnerSet'\n"
            "    ADD OBJECT frmWork AS WorkerForm\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native FormSet ActiveForm script should complete: ") + state.message +
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

        check("cactivebaseclass", "Form");
        check("cactivecaption", "WorkerForm");
        check("cactiveparentbaseclass", "FormSet");
        check("cactiveparentcaption", "OwnerSet");
        check("lreleased", "true");
        check("lmethodcontinued", "true");
        check("lsethascaptionafterrelease", "false");

        expect(state.ole_objects.empty(),
               "native FormSet ActiveForm script should release the whole formset hierarchy");

        fs::remove_all(temp_root, ignored);
    }

    void test_inherited_external_parent_child_methods_resolve_thisform_to_live_derived_owner()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_inherited_external_parent_thisform";
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
            "        RETURN THISFORM.Caption\n"
            "    ENDFUNC\n"
            "    FUNCTION TriggerSave\n"
            "        RETURN THISFORM.Save()\n"
            "    ENDFUNC\n"
            "    FUNCTION OwnerRef\n"
            "        RETURN THISFORM\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        const fs::path main_path = temp_root / "inherited_external_parent_thisform.prg";
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
               std::string("inherited external-parent THISFORM script should complete: ") + state.message +
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
               "inherited external-parent THISFORM script should register derived owner and child objects");
        if (state.ole_objects.size() == 2U)
        {
            const auto &owner_object = state.ole_objects[0];
            const auto &child_object = state.ole_objects[1];
            expect(owner_object.prog_id == "ChildForm",
                   "inherited external-parent THISFORM should preserve the live derived owner identity");
            expect(owner_object.source == main_path.string(),
                   "inherited external-parent THISFORM should preserve the derived owner source path");
            expect(child_object.prog_id == "SaveButton",
                   "inherited external-parent THISFORM should preserve the child object identity");
            expect(child_object.source == widget_library_path.string(),
                   "inherited external-parent THISFORM should preserve the external parent-library child source path");
            const auto caption = owner_object.properties.find("caption");
            if (caption != owner_object.properties.end())
            {
                expect(copperfin::runtime::format_value(caption->second) == "MainForm-Saved-Ref",
                       "inherited external-parent THISFORM should let child methods and returned owner refs mutate the live derived owner");
            }
            else
            {
                expect(false, "inherited external-parent THISFORM should preserve the derived owner caption property");
            }
        }

        const bool has_save_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "ParentForm.Save";
        });
        expect(has_save_invoke_event,
               "inherited external-parent THISFORM should route child method calls into the inherited owner Save method");

        fs::remove_all(temp_root, ignored);
    }

}
