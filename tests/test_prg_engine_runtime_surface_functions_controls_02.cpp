#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_optionbutton_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_optionbutton_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_optionbutton_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerOptionButton')\n"
            "oLeaf = NEWOBJECT('WorkerOptionButton')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 156)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerOptionButton AS OptionButton\n"
            "    Caption = 'WorkerOptionButton'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native OptionButton-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerOptionButton");
        check("cleafcaption", "WorkerOptionButton");
        check("ccreatedescribe", "prefix:WorkerOptionButton");
        check("cleafdescribe", "leaf:WorkerOptionButton");
        check("ccreatebaseclass", "OptionButton");
        check("cleafbaseclass", "OptionButton");
        check("ccreateclass", "WorkerOptionButton");
        check("cleafclass", "WorkerOptionButton");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "156");

        expect(state.ole_objects.size() == 3U,
               "native OptionButton-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerOptionButton",
                   "native OptionButton-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native OptionButton-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "OptionButton",
                   "native OptionButton-base CREATEOBJECT should preserve the builtin OptionButton base token");
            expect(create_object.class_library.empty(),
                   "native OptionButton-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native OptionButton-base CREATEOBJECT should persist native class hierarchy including OptionButton");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKEROPTIONBUTTON",
                       "native OptionButton-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "OPTIONBUTTON",
                       "native OptionButton-base CREATEOBJECT should store the builtin OptionButton base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native OptionButton-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerOptionButton",
                   "native OptionButton-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native OptionButton-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "OptionButton",
                   "native OptionButton-base NEWOBJECT should preserve the builtin OptionButton base token");
            expect(leaf_object.class_library.empty(),
                   "native OptionButton-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native OptionButton-base NEWOBJECT should persist native class hierarchy including OptionButton");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKEROPTIONBUTTON",
                       "native OptionButton-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "OPTIONBUTTON",
                       "native OptionButton-base NEWOBJECT should store the builtin OptionButton base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native OptionButton-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while OptionButton-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerOptionButton.Describe";
        });
        expect(has_describe_invoke_event,
               "native OptionButton-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_spinner_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_spinner_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_spinner_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerSpinner')\n"
            "oLeaf = NEWOBJECT('WorkerSpinner')\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 157)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerSpinner AS Spinner\n"
            "    Caption = 'WorkerSpinner'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Caption\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Spinner-base class script should complete: ") + state.message +
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

        check("ccreatecaption", "WorkerSpinner");
        check("cleafcaption", "WorkerSpinner");
        check("ccreatedescribe", "prefix:WorkerSpinner");
        check("cleafdescribe", "leaf:WorkerSpinner");
        check("ccreatebaseclass", "Spinner");
        check("cleafbaseclass", "Spinner");
        check("ccreateclass", "WorkerSpinner");
        check("cleafclass", "WorkerSpinner");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "157");

        expect(state.ole_objects.size() == 3U,
               "native Spinner-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerSpinner",
                   "native Spinner-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Spinner-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Spinner",
                   "native Spinner-base CREATEOBJECT should preserve the builtin Spinner base token");
            expect(create_object.class_library.empty(),
                   "native Spinner-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Spinner-base CREATEOBJECT should persist native class hierarchy including Spinner");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSPINNER",
                       "native Spinner-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "SPINNER",
                       "native Spinner-base CREATEOBJECT should store the builtin Spinner base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Spinner-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerSpinner",
                   "native Spinner-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Spinner-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Spinner",
                   "native Spinner-base NEWOBJECT should preserve the builtin Spinner base token");
            expect(leaf_object.class_library.empty(),
                   "native Spinner-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Spinner-base NEWOBJECT should persist native class hierarchy including Spinner");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSPINNER",
                       "native Spinner-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "SPINNER",
                       "native Spinner-base NEWOBJECT should store the builtin Spinner base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Spinner-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Spinner-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerSpinner.Describe";
        });
        expect(has_describe_invoke_event,
               "native Spinner-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_editbox_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_editbox_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_editbox_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerEditBox')\n"
            "oLeaf = NEWOBJECT('WorkerEditBox')\n"
            "cCreateValue = oCreate.Value\n"
            "cLeafValue = oLeaf.Value\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 158)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerEditBox AS EditBox\n"
            "    Value = 'WorkerEditBox'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Value\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native EditBox-base class script should complete: ") + state.message +
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

        check("ccreatevalue", "WorkerEditBox");
        check("cleafvalue", "WorkerEditBox");
        check("ccreatedescribe", "prefix:WorkerEditBox");
        check("cleafdescribe", "leaf:WorkerEditBox");
        check("ccreatebaseclass", "EditBox");
        check("cleafbaseclass", "EditBox");
        check("ccreateclass", "WorkerEditBox");
        check("cleafclass", "WorkerEditBox");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "158");

        expect(state.ole_objects.size() == 3U,
               "native EditBox-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerEditBox",
                   "native EditBox-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native EditBox-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "EditBox",
                   "native EditBox-base CREATEOBJECT should preserve the builtin EditBox base token");
            expect(create_object.class_library.empty(),
                   "native EditBox-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native EditBox-base CREATEOBJECT should persist native class hierarchy including EditBox");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKEREDITBOX",
                       "native EditBox-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "EDITBOX",
                       "native EditBox-base CREATEOBJECT should store the builtin EditBox base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native EditBox-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerEditBox",
                   "native EditBox-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native EditBox-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "EditBox",
                   "native EditBox-base NEWOBJECT should preserve the builtin EditBox base token");
            expect(leaf_object.class_library.empty(),
                   "native EditBox-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native EditBox-base NEWOBJECT should persist native class hierarchy including EditBox");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKEREDITBOX",
                       "native EditBox-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "EDITBOX",
                       "native EditBox-base NEWOBJECT should store the builtin EditBox base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native EditBox-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while EditBox-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerEditBox.Describe";
        });
        expect(has_describe_invoke_event,
               "native EditBox-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_image_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_image_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_image_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerImage')\n"
            "oLeaf = NEWOBJECT('WorkerImage')\n"
            "cCreatePicture = oCreate.Picture\n"
            "cLeafPicture = oLeaf.Picture\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 159)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerImage AS Image\n"
            "    Picture = 'WorkerImage.bmp'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.Picture\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Image-base class script should complete: ") + state.message +
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

        check("ccreatepicture", "WorkerImage.bmp");
        check("cleafpicture", "WorkerImage.bmp");
        check("ccreatedescribe", "prefix:WorkerImage.bmp");
        check("cleafdescribe", "leaf:WorkerImage.bmp");
        check("ccreatebaseclass", "Image");
        check("cleafbaseclass", "Image");
        check("ccreateclass", "WorkerImage");
        check("cleafclass", "WorkerImage");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "159");

        expect(state.ole_objects.size() == 3U,
               "native Image-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerImage",
                   "native Image-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Image-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Image",
                   "native Image-base CREATEOBJECT should preserve the builtin Image base token");
            expect(create_object.class_library.empty(),
                   "native Image-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Image-base CREATEOBJECT should persist native class hierarchy including Image");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERIMAGE",
                       "native Image-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "IMAGE",
                       "native Image-base CREATEOBJECT should store the builtin Image base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Image-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerImage",
                   "native Image-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Image-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Image",
                   "native Image-base NEWOBJECT should preserve the builtin Image base token");
            expect(leaf_object.class_library.empty(),
                   "native Image-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Image-base NEWOBJECT should persist native class hierarchy including Image");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERIMAGE",
                       "native Image-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "IMAGE",
                       "native Image-base NEWOBJECT should store the builtin Image base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Image-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Image-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerImage.Describe";
        });
        expect(has_describe_invoke_event,
               "native Image-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_timer_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_timer_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_timer_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerTimer')\n"
            "oLeaf = NEWOBJECT('WorkerTimer')\n"
            "cCreateInterval = ALLTRIM(STR(oCreate.Interval))\n"
            "cLeafInterval = ALLTRIM(STR(oLeaf.Interval))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 160)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerTimer AS Timer\n"
            "    Interval = 250\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.Interval))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Timer-base class script should complete: ") + state.message +
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

        check("ccreateinterval", "250");
        check("cleafinterval", "250");
        check("ccreatedescribe", "prefix:250");
        check("cleafdescribe", "leaf:250");
        check("ccreatebaseclass", "Timer");
        check("cleafbaseclass", "Timer");
        check("ccreateclass", "WorkerTimer");
        check("cleafclass", "WorkerTimer");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "160");

        expect(state.ole_objects.size() == 3U,
               "native Timer-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerTimer",
                   "native Timer-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Timer-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Timer",
                   "native Timer-base CREATEOBJECT should preserve the builtin Timer base token");
            expect(create_object.class_library.empty(),
                   "native Timer-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Timer-base CREATEOBJECT should persist native class hierarchy including Timer");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERTIMER",
                       "native Timer-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "TIMER",
                       "native Timer-base CREATEOBJECT should store the builtin Timer base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Timer-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerTimer",
                   "native Timer-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Timer-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Timer",
                   "native Timer-base NEWOBJECT should preserve the builtin Timer base token");
            expect(leaf_object.class_library.empty(),
                   "native Timer-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Timer-base NEWOBJECT should persist native class hierarchy including Timer");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERTIMER",
                       "native Timer-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "TIMER",
                       "native Timer-base NEWOBJECT should store the builtin Timer base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Timer-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Timer-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerTimer.Describe";
        });
        expect(has_describe_invoke_event,
               "native Timer-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_shape_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_shape_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_shape_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerShape')\n"
            "oLeaf = NEWOBJECT('WorkerShape')\n"
            "cCreateBackStyle = ALLTRIM(STR(oCreate.BackStyle))\n"
            "cLeafBackStyle = ALLTRIM(STR(oLeaf.BackStyle))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 161)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerShape AS Shape\n"
            "    BackStyle = 1\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.BackStyle))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Shape-base class script should complete: ") + state.message +
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

        check("ccreatebackstyle", "1");
        check("cleafbackstyle", "1");
        check("ccreatedescribe", "prefix:1");
        check("cleafdescribe", "leaf:1");
        check("ccreatebaseclass", "Shape");
        check("cleafbaseclass", "Shape");
        check("ccreateclass", "WorkerShape");
        check("cleafclass", "WorkerShape");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "161");

        expect(state.ole_objects.size() == 3U,
               "native Shape-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerShape",
                   "native Shape-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Shape-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Shape",
                   "native Shape-base CREATEOBJECT should preserve the builtin Shape base token");
            expect(create_object.class_library.empty(),
                   "native Shape-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Shape-base CREATEOBJECT should persist native class hierarchy including Shape");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERSHAPE",
                       "native Shape-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "SHAPE",
                       "native Shape-base CREATEOBJECT should store the builtin Shape base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Shape-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerShape",
                   "native Shape-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Shape-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Shape",
                   "native Shape-base NEWOBJECT should preserve the builtin Shape base token");
            expect(leaf_object.class_library.empty(),
                   "native Shape-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Shape-base NEWOBJECT should persist native class hierarchy including Shape");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERSHAPE",
                       "native Shape-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "SHAPE",
                       "native Shape-base NEWOBJECT should store the builtin Shape base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Shape-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Shape-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerShape.Describe";
        });
        expect(has_describe_invoke_event,
               "native Shape-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
