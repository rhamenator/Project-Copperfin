#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_createobject_and_newobject_instantiate_same_prg_line_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_line_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_line_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerLine')\n"
            "oLeaf = NEWOBJECT('WorkerLine')\n"
            "cCreateBorderWidth = ALLTRIM(STR(oCreate.BorderWidth))\n"
            "cLeafBorderWidth = ALLTRIM(STR(oLeaf.BorderWidth))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 162)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerLine AS Line\n"
            "    BorderWidth = 3\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.BorderWidth))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native Line-base class script should complete: ") + state.message +
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

        check("ccreateborderwidth", "3");
        check("cleafborderwidth", "3");
        check("ccreatedescribe", "prefix:3");
        check("cleafdescribe", "leaf:3");
        check("ccreatebaseclass", "Line");
        check("cleafbaseclass", "Line");
        check("ccreateclass", "WorkerLine");
        check("cleafclass", "WorkerLine");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "162");

        expect(state.ole_objects.size() == 3U,
               "native Line-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerLine",
                   "native Line-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native Line-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "Line",
                   "native Line-base CREATEOBJECT should preserve the builtin Line base token");
            expect(create_object.class_library.empty(),
                   "native Line-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native Line-base CREATEOBJECT should persist native class hierarchy including Line");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERLINE",
                       "native Line-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "LINE",
                       "native Line-base CREATEOBJECT should store the builtin Line base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native Line-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerLine",
                   "native Line-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native Line-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "Line",
                   "native Line-base NEWOBJECT should preserve the builtin Line base token");
            expect(leaf_object.class_library.empty(),
                   "native Line-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native Line-base NEWOBJECT should persist native class hierarchy including Line");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERLINE",
                       "native Line-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "LINE",
                       "native Line-base NEWOBJECT should store the builtin Line base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native Line-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while Line-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerLine.Describe";
        });
        expect(has_describe_invoke_event,
               "native Line-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_optiongroup_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_optiongroup_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_optiongroup_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerOptionGroup')\n"
            "oLeaf = NEWOBJECT('WorkerOptionGroup')\n"
            "cCreateButtonCount = ALLTRIM(STR(oCreate.ButtonCount))\n"
            "cLeafButtonCount = ALLTRIM(STR(oLeaf.ButtonCount))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 163)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerOptionGroup AS OptionGroup\n"
            "    ButtonCount = 2\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.ButtonCount))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native OptionGroup-base class script should complete: ") + state.message +
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

        check("ccreatebuttoncount", "2");
        check("cleafbuttoncount", "2");
        check("ccreatedescribe", "prefix:2");
        check("cleafdescribe", "leaf:2");
        check("ccreatebaseclass", "OptionGroup");
        check("cleafbaseclass", "OptionGroup");
        check("ccreateclass", "WorkerOptionGroup");
        check("cleafclass", "WorkerOptionGroup");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "163");

        expect(state.ole_objects.size() == 3U,
               "native OptionGroup-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerOptionGroup",
                   "native OptionGroup-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native OptionGroup-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "OptionGroup",
                   "native OptionGroup-base CREATEOBJECT should preserve the builtin OptionGroup base token");
            expect(create_object.class_library.empty(),
                   "native OptionGroup-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native OptionGroup-base CREATEOBJECT should persist native class hierarchy including OptionGroup");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKEROPTIONGROUP",
                       "native OptionGroup-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "OPTIONGROUP",
                       "native OptionGroup-base CREATEOBJECT should store the builtin OptionGroup base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native OptionGroup-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerOptionGroup",
                   "native OptionGroup-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native OptionGroup-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "OptionGroup",
                   "native OptionGroup-base NEWOBJECT should preserve the builtin OptionGroup base token");
            expect(leaf_object.class_library.empty(),
                   "native OptionGroup-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native OptionGroup-base NEWOBJECT should persist native class hierarchy including OptionGroup");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKEROPTIONGROUP",
                       "native OptionGroup-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "OPTIONGROUP",
                       "native OptionGroup-base NEWOBJECT should store the builtin OptionGroup base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native OptionGroup-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while OptionGroup-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerOptionGroup.Describe";
        });
        expect(has_describe_invoke_event,
               "native OptionGroup-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_commandgroup_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_commandgroup_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_commandgroup_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerCommandGroup')\n"
            "oLeaf = NEWOBJECT('WorkerCommandGroup')\n"
            "cCreateButtonCount = ALLTRIM(STR(oCreate.ButtonCount))\n"
            "cLeafButtonCount = ALLTRIM(STR(oLeaf.ButtonCount))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 164)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerCommandGroup AS CommandGroup\n"
            "    ButtonCount = 3\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.ButtonCount))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native CommandGroup-base class script should complete: ") + state.message +
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

        check("ccreatebuttoncount", "3");
        check("cleafbuttoncount", "3");
        check("ccreatedescribe", "prefix:3");
        check("cleafdescribe", "leaf:3");
        check("ccreatebaseclass", "CommandGroup");
        check("cleafbaseclass", "CommandGroup");
        check("ccreateclass", "WorkerCommandGroup");
        check("cleafclass", "WorkerCommandGroup");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "164");

        expect(state.ole_objects.size() == 3U,
               "native CommandGroup-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerCommandGroup",
                   "native CommandGroup-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native CommandGroup-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "CommandGroup",
                   "native CommandGroup-base CREATEOBJECT should preserve the builtin CommandGroup base token");
            expect(create_object.class_library.empty(),
                   "native CommandGroup-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native CommandGroup-base CREATEOBJECT should persist native class hierarchy including CommandGroup");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERCOMMANDGROUP",
                       "native CommandGroup-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "COMMANDGROUP",
                       "native CommandGroup-base CREATEOBJECT should store the builtin CommandGroup base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native CommandGroup-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerCommandGroup",
                   "native CommandGroup-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native CommandGroup-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "CommandGroup",
                   "native CommandGroup-base NEWOBJECT should preserve the builtin CommandGroup base token");
            expect(leaf_object.class_library.empty(),
                   "native CommandGroup-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native CommandGroup-base NEWOBJECT should persist native class hierarchy including CommandGroup");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERCOMMANDGROUP",
                       "native CommandGroup-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "COMMANDGROUP",
                       "native CommandGroup-base NEWOBJECT should store the builtin CommandGroup base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native CommandGroup-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while CommandGroup-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerCommandGroup.Describe";
        });
        expect(has_describe_invoke_event,
               "native CommandGroup-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_olecontrol_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_olecontrol_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_olecontrol_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerOleControl')\n"
            "oLeaf = NEWOBJECT('WorkerOleControl')\n"
            "cCreateControlSource = oCreate.ControlSource\n"
            "cLeafControlSource = oLeaf.ControlSource\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 165)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerOleControl AS OleControl\n"
            "    ControlSource = 'customer.name'\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + THIS.ControlSource\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native OleControl-base class script should complete: ") + state.message +
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

        check("ccreatecontrolsource", "customer.name");
        check("cleafcontrolsource", "customer.name");
        check("ccreatedescribe", "prefix:customer.name");
        check("cleafdescribe", "leaf:customer.name");
        check("ccreatebaseclass", "OleControl");
        check("cleafbaseclass", "OleControl");
        check("ccreateclass", "WorkerOleControl");
        check("cleafclass", "WorkerOleControl");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "165");

        expect(state.ole_objects.size() == 3U,
               "native OleControl-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerOleControl",
                   "native OleControl-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native OleControl-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "OleControl",
                   "native OleControl-base CREATEOBJECT should preserve the builtin OleControl base token");
            expect(create_object.class_library.empty(),
                   "native OleControl-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native OleControl-base CREATEOBJECT should persist native class hierarchy including OleControl");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKEROLECONTROL",
                       "native OleControl-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "OLECONTROL",
                       "native OleControl-base CREATEOBJECT should store the builtin OleControl base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native OleControl-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerOleControl",
                   "native OleControl-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native OleControl-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "OleControl",
                   "native OleControl-base NEWOBJECT should preserve the builtin OleControl base token");
            expect(leaf_object.class_library.empty(),
                   "native OleControl-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native OleControl-base NEWOBJECT should persist native class hierarchy including OleControl");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKEROLECONTROL",
                       "native OleControl-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "OLECONTROL",
                       "native OleControl-base NEWOBJECT should store the builtin OleControl base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native OleControl-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while OleControl-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerOleControl.Describe";
        });
        expect(has_describe_invoke_event,
               "native OleControl-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_pageheader_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_pageheader_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pageheader_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerPageHeader')\n"
            "oLeaf = NEWOBJECT('WorkerPageHeader')\n"
            "cCreateHeight = ALLTRIM(STR(oCreate.Height))\n"
            "cLeafHeight = ALLTRIM(STR(oLeaf.Height))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 166)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerPageHeader AS PageHeader\n"
            "    Height = 24\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.Height))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageHeader-base class script should complete: ") + state.message +
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

        check("ccreateheight", "24");
        check("cleafheight", "24");
        check("ccreatedescribe", "prefix:24");
        check("cleafdescribe", "leaf:24");
        check("ccreatebaseclass", "PageHeader");
        check("cleafbaseclass", "PageHeader");
        check("ccreateclass", "WorkerPageHeader");
        check("cleafclass", "WorkerPageHeader");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "166");

        expect(state.ole_objects.size() == 3U,
               "native PageHeader-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerPageHeader",
                   "native PageHeader-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native PageHeader-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "PageHeader",
                   "native PageHeader-base CREATEOBJECT should preserve the builtin PageHeader base token");
            expect(create_object.class_library.empty(),
                   "native PageHeader-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native PageHeader-base CREATEOBJECT should persist native class hierarchy including PageHeader");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERPAGEHEADER",
                       "native PageHeader-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "PAGEHEADER",
                       "native PageHeader-base CREATEOBJECT should store the builtin PageHeader base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native PageHeader-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerPageHeader",
                   "native PageHeader-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native PageHeader-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "PageHeader",
                   "native PageHeader-base NEWOBJECT should preserve the builtin PageHeader base token");
            expect(leaf_object.class_library.empty(),
                   "native PageHeader-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native PageHeader-base NEWOBJECT should persist native class hierarchy including PageHeader");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERPAGEHEADER",
                       "native PageHeader-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "PAGEHEADER",
                       "native PageHeader-base NEWOBJECT should store the builtin PageHeader base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native PageHeader-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while PageHeader-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerPageHeader.Describe";
        });
        expect(has_describe_invoke_event,
               "native PageHeader-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

    void test_createobject_and_newobject_instantiate_same_prg_pagefooter_native_class()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_native_prg_pagefooter_class";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "native_pagefooter_class.prg";
        write_text(
            main_path,
            "oCreate = CREATEOBJECT('WorkerPageFooter')\n"
            "oLeaf = NEWOBJECT('WorkerPageFooter')\n"
            "cCreateHeight = ALLTRIM(STR(oCreate.Height))\n"
            "cLeafHeight = ALLTRIM(STR(oLeaf.Height))\n"
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
            "lDictSet = SETPEM(oDict, 'comparemode', 167)\n"
            "nDictCompare = GETPEM(oDict, 'comparemode')\n"
            "RETURN\n"
            "DEFINE CLASS WorkerPageFooter AS PageFooter\n"
            "    Height = 18\n"
            "    FUNCTION Describe\n"
            "        LPARAMETERS tcPrefix\n"
            "        RETURN tcPrefix + ':' + ALLTRIM(STR(THIS.Height))\n"
            "    ENDFUNC\n"
            "ENDDEFINE\n");

        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               std::string("native PageFooter-base class script should complete: ") + state.message +
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

        check("ccreateheight", "18");
        check("cleafheight", "18");
        check("ccreatedescribe", "prefix:18");
        check("cleafdescribe", "leaf:18");
        check("ccreatebaseclass", "PageFooter");
        check("cleafbaseclass", "PageFooter");
        check("ccreateclass", "WorkerPageFooter");
        check("cleafclass", "WorkerPageFooter");
        check("lcreatehasdescribe", "true");
        check("lleafhasdescribe", "true");
        check("lcreatehasbaseclass", "true");
        check("lleafhasbaseclass", "true");
        check("ldictset", "true");
        check("ndictcompare", "167");

        expect(state.ole_objects.size() == 3U,
               "native PageFooter-base CREATEOBJECT/NEWOBJECT plus COM NEWOBJECT should register three runtime objects");
        if (state.ole_objects.size() == 3U)
        {
            const auto &create_object = state.ole_objects[0];
            const auto &leaf_object = state.ole_objects[1];
            expect(create_object.prog_id == "WorkerPageFooter",
                   "native PageFooter-base CREATEOBJECT should preserve the PRG class name");
            expect(create_object.source == main_path.string(),
                   "native PageFooter-base CREATEOBJECT should preserve defining PRG provenance");
            expect(create_object.base_class_name == "PageFooter",
                   "native PageFooter-base CREATEOBJECT should preserve the builtin PageFooter base token");
            expect(create_object.class_library.empty(),
                   "native PageFooter-base CREATEOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(create_object.class_hierarchy.size() == 3U,
                   "native PageFooter-base CREATEOBJECT should persist native class hierarchy including PageFooter");
            if (create_object.class_hierarchy.size() == 3U)
            {
                expect(create_object.class_hierarchy[0] == "WORKERPAGEFOOTER",
                       "native PageFooter-base CREATEOBJECT should store the native class first");
                expect(create_object.class_hierarchy[1] == "PAGEFOOTER",
                       "native PageFooter-base CREATEOBJECT should store the builtin PageFooter base second");
                expect(create_object.class_hierarchy[2] == "OBJECT",
                       "native PageFooter-base CREATEOBJECT should store the terminal OBJECT token third");
            }

            expect(leaf_object.prog_id == "WorkerPageFooter",
                   "native PageFooter-base NEWOBJECT should preserve the PRG class name");
            expect(leaf_object.source == main_path.string(),
                   "native PageFooter-base NEWOBJECT should preserve defining PRG provenance");
            expect(leaf_object.base_class_name == "PageFooter",
                   "native PageFooter-base NEWOBJECT should preserve the builtin PageFooter base token");
            expect(leaf_object.class_library.empty(),
                   "native PageFooter-base NEWOBJECT should keep ClassLibrary empty for same-PRG classes");
            expect(leaf_object.class_hierarchy.size() == 3U,
                   "native PageFooter-base NEWOBJECT should persist native class hierarchy including PageFooter");
            if (leaf_object.class_hierarchy.size() == 3U)
            {
                expect(leaf_object.class_hierarchy[0] == "WORKERPAGEFOOTER",
                       "native PageFooter-base NEWOBJECT should store the native class first");
                expect(leaf_object.class_hierarchy[1] == "PAGEFOOTER",
                       "native PageFooter-base NEWOBJECT should store the builtin PageFooter base second");
                expect(leaf_object.class_hierarchy[2] == "OBJECT",
                       "native PageFooter-base NEWOBJECT should store the terminal OBJECT token third");
            }

            expect(state.ole_objects[2].prog_id == "Scripting.Dictionary",
                   "COM NEWOBJECT should remain stable while PageFooter-base native activation lands");
        }

        const bool has_describe_invoke_event = std::any_of(state.events.begin(), state.events.end(), [](const auto &event)
        {
            return event.category == "prg.object.invoke" &&
                   event.detail == "WorkerPageFooter.Describe";
        });
        expect(has_describe_invoke_event,
               "native PageFooter-base activation should dispatch ordinary native method calls");

        fs::remove_all(temp_root, ignored);
    }

}
