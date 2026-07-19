#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_native_form_load_precedes_child_and_init_lifecycle_events()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_load_lifecycle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_load_lifecycle.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('LifecycleForm')\n"
            "oLeaf = NEWOBJECT('LifecycleForm')\n"
            "cCreateSequence = oCreate.cSequence\n"
            "cLeafSequence = oLeaf.cSequence\n"
            "lCreateLoadSawChild = oCreate.lLoadSawChild\n"
            "lLeafLoadSawChild = oLeaf.lLoadSawChild\n"
            "lCreateInitSawChild = oCreate.lInitSawChild\n"
            "lLeafInitSawChild = oLeaf.lInitSawChild\n"
            "lCreateLoadSawThisForm = oCreate.lLoadSawThisForm\n"
            "lLeafLoadSawThisForm = oLeaf.lLoadSawThisForm\n"
            "RETURN\n"
            "DEFINE CLASS LifecycleForm AS Form\n"
            "    ADD OBJECT cmdProbe AS LifecycleProbe\n"
            "    cSequence = ''\n"
            "    lLoadSawChild = .F.\n"
            "    lInitSawChild = .F.\n"
            "    lLoadSawThisForm = .F.\n"
            "    PROCEDURE Load\n"
            "        THIS.cSequence = THIS.cSequence + 'L'\n"
            "        THIS.lLoadSawChild = PEMSTATUS(THIS, 'cmdProbe', 1)\n"
            "        THIS.lLoadSawThisForm = THISFORM.Class == THIS.Class\n"
            "    ENDPROC\n"
            "    PROCEDURE Init\n"
            "        THIS.cSequence = THIS.cSequence + 'I'\n"
            "        THIS.lInitSawChild = PEMSTATUS(THIS, 'cmdProbe', 1)\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LifecycleProbe AS CommandButton\n"
            "    PROCEDURE Init\n"
            "        THIS.Parent.cSequence = THIS.Parent.cSequence + 'C'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "native Form Load lifecycle script should complete: " + state.message);

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

        check("ccreatesequence", "LCI");
        check("cleafsequence", "LCI");
        check("lcreateloadsawchild", "false");
        check("lleafloadsawchild", "false");
        check("lcreateinitsawchild", "true");
        check("lleafinitsawchild", "true");
        check("lcreateloadsawthisform", "true");
        check("lleafloadsawthisform", "true");

        const std::size_t load_event_count = static_cast<std::size_t>(std::count_if(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.load" &&
                       event.detail == "LifecycleForm.Load";
            }));
        expect(load_event_count == 2U,
               "CREATEOBJECT and NEWOBJECT should each dispatch one Form Load event");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_formset_load_precedes_contained_form_load()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_formset_load_lifecycle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_formset_load_lifecycle.prg";
        write_text(
            main_path,
            "oSet = CREATEOBJECT('LifecycleFormSet')\n"
            "cSequence = oSet.cSequence\n"
            "lFormSawThisFormSet = oSet.frmProbe.lLoadSawThisFormSet\n"
            "RETURN\n"
            "DEFINE CLASS LifecycleFormSet AS FormSet\n"
            "    ADD OBJECT frmProbe AS LifecycleForm\n"
            "    cSequence = ''\n"
            "    PROCEDURE Load\n"
            "        THIS.cSequence = THIS.cSequence + 'S'\n"
            "    ENDPROC\n"
            "    PROCEDURE Init\n"
            "        THIS.cSequence = THIS.cSequence + 'I'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS LifecycleForm AS Form\n"
            "    lLoadSawThisFormSet = .F.\n"
            "    PROCEDURE Load\n"
            "        THISFORMSET.cSequence = THISFORMSET.cSequence + 'L'\n"
            "        THIS.lLoadSawThisFormSet = THISFORMSET.Class == 'LifecycleFormSet'\n"
            "    ENDPROC\n"
            "    PROCEDURE Init\n"
            "        THISFORMSET.cSequence = THISFORMSET.cSequence + 'F'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "native FormSet Load lifecycle script should complete: " + state.message);

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

        check("csequence", "SLFI");
        check("lformsawthisformset", "true");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_load_returning_false_prevents_object_creation()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_load_rejection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_load_rejection.prg";
        write_text(
            main_path,
            "oRejected = CREATEOBJECT('RejectedForm')\n"
            "lRejectedIsNull = ISNULL(oRejected)\n"
            "RETURN\n"
            "DEFINE CLASS RejectedForm AS Form\n"
            "    PROCEDURE Load\n"
            "        RETURN .F.\n"
            "    ENDPROC\n"
            "    PROCEDURE Init\n"
            "        nInitCount = nInitCount + 1\n"
            "    ENDPROC\n"
            "    PROCEDURE Destroy\n"
            "        nDestroyCount = nDestroyCount + 1\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "native Form Load rejection should continue caller execution: " + state.message);
        expect(state.ole_objects.empty(),
               "Form Load returning .F. should remove the rejected native object without Destroy");

        const auto rejected = state.globals.find("lrejectedisnull");
        expect(rejected != state.globals.end() &&
                   copperfin::runtime::format_value(rejected->second) == "true",
               "Form Load returning .F. should yield .NULL. to CREATEOBJECT");
        expect(state.globals.find("ninitcount") == state.globals.end(),
               "Form Load returning .F. should prevent Init");
        expect(state.globals.find("ndestroycount") == state.globals.end(),
               "Form Load returning .F. should not invoke Destroy");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_commandbutton_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_commandbutton_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandbutton_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerButton')\n"
            "oLeaf = NEWOBJECT('WorkerButton')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
            "cCreateDescribe = oCreate.Describe('prefix')\n"
            "cLeafDescribe = oLeaf.Describe('leaf')\n"
            "cCreateBaseClass = oCreate.BaseClass\n"
            "cLeafBaseClass = oLeaf.BaseClass\n"
            "cCreateClass = oCreate.Class\n"
            "cLeafClass = oLeaf.Class\n"
            "lCreateHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lLeafHasDescribe = GETPEM(oLeaf, 'Describe')\n"
            "lCreateHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lLeafHasBaseClass = PEMSTATUS(oLeaf, 'BaseClass', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 144)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerButton AS CommandButton\n"
            "    Caption = 'WorkerButton'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandButton-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerButton");
        check("cleafcaption", "WorkerButton");
        check("ccreatedescribe", "prefix:WorkerButton");
        check("cleafdescribe", "leaf:WorkerButton");
        check("ccreatebaseclass", "CommandButton");
        check("cleafbaseclass", "CommandButton");
        check("ccreateclass", "WorkerButton");
        check("cleafclass", "WorkerButton");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "144");

        expect(state.ole_objects.size() == 3U,
               "native CommandButton-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerButton",
                   "native CommandButton-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native CommandButton-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "CommandButton",
                   "native CommandButton-base CREATEOBJECT should preserve the builtin CommandButton base token");
            expect(create_object.class_library.empty(),
                   "native CommandButton-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native CommandButton-base CREATEOBJECT should persist native class hierarchy including CommandButton");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERBUTTON",
                       "native CommandButton-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "COMMANDBUTTON",
                       "native CommandButton-base CREATEOBJECT should store the builtin CommandButton base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native CommandButton-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerButton",
                   "native CommandButton-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native CommandButton-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "CommandButton",
                   "native CommandButton-base NEWOBJECT should preserve the builtin CommandButton base token");
            expect(leaf_object.class_library.empty(),
                   "native CommandButton-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native CommandButton-base NEWOBJECT should persist native class hierarchy including CommandButton");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERBUTTON",
                       "native CommandButton-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "COMMANDBUTTON",
                       "native CommandButton-base NEWOBJECT should store the builtin CommandButton base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native CommandButton-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while CommandButton-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerButton.Describe";
        });
        expect(has_describe_invoke_event,
               "native CommandButton-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_textbox_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_textbox_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_textbox_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerTextBox')\n"
            "oLeaf = NEWOBJECT('WorkerTextBox')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
            "cCreateDescribe = oCreate.Describe('prefix')\n"
            "cLeafDescribe = oLeaf.Describe('leaf')\n"
            "cCreateBaseClass = oCreate.BaseClass\n"
            "cLeafBaseClass = oLeaf.BaseClass\n"
            "cCreateClass = oCreate.Class\n"
            "cLeafClass = oLeaf.Class\n"
            "lCreateHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lLeafHasDescribe = GETPEM(oLeaf, 'Describe')\n"
            "lCreateHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lLeafHasBaseClass = PEMSTATUS(oLeaf, 'BaseClass', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 145)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerTextBox AS TextBox\n"
            "    Caption = 'WorkerTextBox'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native TextBox-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerTextBox");
        check("cleafcaption", "WorkerTextBox");
        check("ccreatedescribe", "prefix:WorkerTextBox");
        check("cleafdescribe", "leaf:WorkerTextBox");
        check("ccreatebaseclass", "TextBox");
        check("cleafbaseclass", "TextBox");
        check("ccreateclass", "WorkerTextBox");
        check("cleafclass", "WorkerTextBox");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "145");

        expect(state.ole_objects.size() == 3U,
               "native TextBox-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerTextBox",
                   "native TextBox-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native TextBox-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "TextBox",
                   "native TextBox-base CREATEOBJECT should preserve the builtin TextBox base token");
            expect(create_object.class_library.empty(),
                   "native TextBox-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native TextBox-base CREATEOBJECT should persist native class hierarchy including TextBox");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERTEXTBOX",
                       "native TextBox-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "TEXTBOX",
                       "native TextBox-base CREATEOBJECT should store the builtin TextBox base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native TextBox-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerTextBox",
                   "native TextBox-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native TextBox-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "TextBox",
                   "native TextBox-base NEWOBJECT should preserve the builtin TextBox base token");
            expect(leaf_object.class_library.empty(),
                   "native TextBox-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native TextBox-base NEWOBJECT should persist native class hierarchy including TextBox");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERTEXTBOX",
                       "native TextBox-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "TEXTBOX",
                       "native TextBox-base NEWOBJECT should store the builtin TextBox base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native TextBox-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while TextBox-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerTextBox.Describe";
        });
        expect(has_describe_invoke_event,
               "native TextBox-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_label_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_label_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_label_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerLabel')\n"
            "oLeaf = NEWOBJECT('WorkerLabel')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
            "cCreateDescribe = oCreate.Describe('prefix')\n"
            "cLeafDescribe = oLeaf.Describe('leaf')\n"
            "cCreateBaseClass = oCreate.BaseClass\n"
            "cLeafBaseClass = oLeaf.BaseClass\n"
            "cCreateClass = oCreate.Class\n"
            "cLeafClass = oLeaf.Class\n"
            "lCreateHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lLeafHasDescribe = GETPEM(oLeaf, 'Describe')\n"
            "lCreateHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lLeafHasBaseClass = PEMSTATUS(oLeaf, 'BaseClass', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 146)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerLabel AS Label\n"
            "    Caption = 'WorkerLabel'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Label-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerLabel");
        check("cleafcaption", "WorkerLabel");
        check("ccreatedescribe", "prefix:WorkerLabel");
        check("cleafdescribe", "leaf:WorkerLabel");
        check("ccreatebaseclass", "Label");
        check("cleafbaseclass", "Label");
        check("ccreateclass", "WorkerLabel");
        check("cleafclass", "WorkerLabel");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "146");

        expect(state.ole_objects.size() == 3U,
               "native Label-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerLabel",
                   "native Label-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Label-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Label",
                   "native Label-base CREATEOBJECT should preserve the builtin Label base token");
            expect(create_object.class_library.empty(),
                   "native Label-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Label-base CREATEOBJECT should persist native class hierarchy including Label");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERLABEL",
                       "native Label-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "LABEL",
                       "native Label-base CREATEOBJECT should store the builtin Label base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Label-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerLabel",
                   "native Label-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Label-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Label",
                   "native Label-base NEWOBJECT should preserve the builtin Label base token");
            expect(leaf_object.class_library.empty(),
                   "native Label-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Label-base NEWOBJECT should persist native class hierarchy including Label");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERLABEL",
                       "native Label-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "LABEL",
                       "native Label-base NEWOBJECT should store the builtin Label base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Label-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Label-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerLabel.Describe";
        });
        expect(has_describe_invoke_event,
               "native Label-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_container_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_container_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_container_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerContainer')\n"
            "oLeaf = NEWOBJECT('WorkerContainer')\n"
            "cCreateCaption = oCreate.Caption\n"
            "cLeafCaption = oLeaf.Caption\n"
            "cCreateDescribe = oCreate.Describe('prefix')\n"
            "cLeafDescribe = oLeaf.Describe('leaf')\n"
            "cCreateBaseClass = oCreate.BaseClass\n"
            "cLeafBaseClass = oLeaf.BaseClass\n"
            "cCreateClass = oCreate.Class\n"
            "cLeafClass = oLeaf.Class\n"
            "lCreateHasDescribe = GETPEM(oCreate, 'Describe')\n"
            "lLeafHasDescribe = GETPEM(oLeaf, 'Describe')\n"
            "lCreateHasBaseClass = PEMSTATUS(oCreate, 'BaseClass', 1)\n"
            "lLeafHasBaseClass = PEMSTATUS(oLeaf, 'BaseClass', 1)\n"
            "oDict = NEWOBJECT('Scripting.Dictionary', 'vbscript.dll')\n"
            "lDictSet = SETPEM(oDict, 'comparemode', 147)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerContainer AS Container\n"
            "    Caption = 'WorkerContainer'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Container-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerContainer");
        check("cleafcaption", "WorkerContainer");
        check("ccreatedescribe", "prefix:WorkerContainer");
        check("cleafdescribe", "leaf:WorkerContainer");
        check("ccreatebaseclass", "Container");
        check("cleafbaseclass", "Container");
        check("ccreateclass", "WorkerContainer");
        check("cleafclass", "WorkerContainer");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "147");

        expect(state.ole_objects.size() == 3U,
               "native Container-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerContainer",
                   "native Container-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Container-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Container",
                   "native Container-base CREATEOBJECT should preserve the builtin Container base token");
            expect(create_object.class_library.empty(),
                   "native Container-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Container-base CREATEOBJECT should persist native class hierarchy including Container");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCONTAINER",
                       "native Container-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "CONTAINER",
                       "native Container-base CREATEOBJECT should store the builtin Container base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Container-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerContainer",
                   "native Container-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Container-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Container",
                   "native Container-base NEWOBJECT should preserve the builtin Container base token");
            expect(leaf_object.class_library.empty(),
                   "native Container-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Container-base NEWOBJECT should persist native class hierarchy including Container");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCONTAINER",
                       "native Container-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "CONTAINER",
                       "native Container-base NEWOBJECT should store the builtin Container base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Container-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Container-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerContainer.Describe";
        });
        expect(has_describe_invoke_event,
               "native Container-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_native_form_and_formset_unload_lifecycle_order()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_form_unload_lifecycle";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_form_unload_lifecycle.prg";
        write_text(
            main_path,
            "cSequence = ''\n"
            "oSet = CREATEOBJECT('UnloadFormSet')\n"
            "lReleased = oSet.Release()\n"
            "RETURN\n"
            "DEFINE CLASS UnloadFormSet AS FormSet\n"
            "    ADD OBJECT frmProbe AS UnloadForm\n"
            "    PROCEDURE Destroy\n"
            "        cSequence = cSequence + 'set-destroy>'\n"
            "    ENDPROC\n"
            "    PROCEDURE Unload\n"
            "        cSequence = cSequence + 'set-unload>'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n"
            "DEFINE CLASS UnloadForm AS Form\n"
            "    PROCEDURE Destroy\n"
            "        cSequence = cSequence + 'form-destroy>'\n"
            "    ENDPROC\n"
            "    PROCEDURE Unload\n"
            "        cSequence = cSequence + 'form-unload>'\n"
            "    ENDPROC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "native Form/FormSet Unload lifecycle script should complete: " + state.message);

        const auto check = [&](const std::string &name, const std::string &expected)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be exported by the Unload lifecycle script");
            if (it != state.globals.end())
            {
                expect(copperfin::runtime::format_value(it->second) == expected,
                       name + " expected '" + expected + "' got '" + copperfin::runtime::format_value(it->second) + "'");
            }
        };

        check("lreleased", "true");
        check("csequence", "form-destroy>form-unload>set-destroy>set-unload>");
        const bool has_form_unload_event = std::any_of(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.unload" &&
                       event.detail == "UnloadForm.Unload";
            });
        expect(has_form_unload_event,
               "native Form release should emit the Form Unload lifecycle event");
        const bool has_formset_unload_event = std::any_of(
            state.events.begin(),
            state.events.end(),
            [](const auto &event)
            {
                return event.category == "prg.object.unload" &&
                       event.detail == "UnloadFormSet.Unload";
            });
        expect(has_formset_unload_event,
               "native FormSet release should emit the FormSet Unload lifecycle event");

        fs::remove_all(temp_root, ignored);
    }

}
