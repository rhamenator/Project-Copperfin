#include "test_prg_engine_runtime_surface_functions_support.h"

namespace copperfin::runtime_surface_tests
{
    void test_indexed_program_and_sys16_stack_introspection()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_indexed_program_stack_introspection";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "indexed_program_stack.prg";
        write_text(
            main_path,
            "cMainName0 = PROGRAM(0)\n"
            "cMainName1 = PROGRAM(1)\n"
            "cMainName2 = PROGRAM(2)\n"
            "cMainPath0 = SYS(16, 0)\n"
            "cMainPath1 = SYS(16, 1)\n"
            "cMainPath2 = SYS(16, 2)\n"
            "PUBLIC cNestedName0, cNestedName1, cNestedName2, cNestedName3, "
            "cNestedPath0, cNestedPath1, cNestedPath2, cNestedPath3\n"
            "DO CaptureIndexedProgramStack\n"
            "RETURN\n"
            "PROCEDURE CaptureIndexedProgramStack\n"
            "cNestedName0 = PROGRAM(0)\n"
            "cNestedName1 = PROGRAM(1)\n"
            "cNestedName2 = PROGRAM(2)\n"
            "cNestedName3 = PROGRAM(3)\n"
            "cNestedPath0 = SYS(16, 0)\n"
            "cNestedPath1 = SYS(16, 1)\n"
            "cNestedPath2 = SYS(16, 2)\n"
            "cNestedPath3 = SYS(16, 3)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "indexed PROGRAM/SYS(16) stack-introspection script should complete: " + state.message);

        const auto value = [&](const std::string &name)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be captured");
            return it == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(it->second);
        };
        expect(value("cmainname0") == "main", "PROGRAM(0) should report the master routine");
        expect(value("cmainname1") == "main", "PROGRAM(1) should report the master routine");
        expect(value("cmainname2").empty(), "out-of-range PROGRAM(2) should return an empty string");
        expect(value("cnestedname0") == "main", "nested PROGRAM(0) should report the master routine");
        expect(value("cnestedname1") == "main", "nested PROGRAM(1) should report the master routine");
        expect(value("cnestedname2") == "CaptureIndexedProgramStack",
               "nested PROGRAM(2) should report the active routine at level two");
        expect(value("cnestedname3").empty(), "out-of-range nested PROGRAM(3) should be empty");

        const auto expect_same_source_file = [&](const std::string &name)
        {
            expect(fs::path(value(name)).filename() == main_path.filename(),
                   name + " should resolve to the executing PRG file");
        };
        expect_same_source_file("cmainpath0");
        expect_same_source_file("cmainpath1");
        expect(value("cmainpath2").empty(), "out-of-range SYS(16,2) should return an empty string");
        expect_same_source_file("cnestedpath0");
        expect_same_source_file("cnestedpath1");
        expect_same_source_file("cnestedpath2");
        expect(value("cnestedpath3").empty(), "out-of-range nested SYS(16,3) should be empty");

        fs::remove_all(temp_root, ignored);
    }

    void test_sys16_preserves_procedure_context()
    {
        namespace fs = std::filesystem;
        const fs::path temp_root = fs::temp_directory_path() / "copperfin_sys16_procedure_context";
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);

        const fs::path main_path = temp_root / "sys16_procedure_context.prg";
        write_text(
            main_path,
            "cEntryPath = SYS(16)\n"
            "cEntryIndexedPath = SYS(16, 1)\n"
            "PUBLIC cCurrentContext, cIndexedContext, cIndexedEntryPath\n"
            "DO CaptureSys16Context\n"
            "RETURN\n"
            "PROCEDURE CaptureSys16Context\n"
            "cCurrentContext = SYS(16)\n"
            "cIndexedContext = SYS(16, 2)\n"
            "cIndexedEntryPath = SYS(16, 1)\n"
            "RETURN\n");

        const auto state = copperfin::runtime::PrgRuntimeSession::create(
                               make_runtime_session_options(main_path.string(), temp_root.string()))
                               .run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.completed,
               "SYS(16) procedure-context script should complete: " + state.message);

        const auto value = [&](const std::string& name)
        {
            const auto it = state.globals.find(name);
            expect(it != state.globals.end(), name + " should be captured");
            return it == state.globals.end()
                       ? std::string{}
                       : copperfin::runtime::format_value(it->second);
        };
        const std::string expected_context =
            "PROCEDURE CaptureSys16Context " + main_path.string();
        expect(value("centrypath") == main_path.string(),
               "entry SYS(16) should remain a plain file path");
        expect(value("centryindexedpath") == main_path.string(),
               "entry indexed SYS(16) should remain a plain file path");
        expect(value("ccurrentcontext") == expected_context,
               "current SYS(16) should preserve procedure context");
        expect(value("cindexedcontext") == expected_context,
               "indexed SYS(16) should preserve procedure context");
        expect(value("cindexedentrypath") == main_path.string(),
               "indexed SYS(16) should preserve entry-frame path behavior");

        fs::remove_all(temp_root, ignored);
    }
}
