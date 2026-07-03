// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "../src/runtime/prg_engine_internal.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

namespace fs = std::filesystem;

void test_parse_define_class_captures_metadata_and_methods() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_define_class";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "classdemo.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS MyWidget AS Custom\n"
        "    Caption = 'Demo'\n"
        "    nCount = 3\n"
        "    PROCEDURE Init\n"
        "        RETURN This.Caption\n"
        "    ENDPROC\n"
        "    FUNCTION CanSave\n"
        "        RETURN .T.\n"
        "    ENDFUNC\n"
        "ENDDEFINE\n"
        "RETURN\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    copperfin::test_support::expect(program.routines.empty(),
                                    "class-only PRG should not leak methods into top-level routines");
    copperfin::test_support::expect(program.classes.size() == 1U,
                                    "DEFINE CLASS should populate one parsed class definition");

    const auto class_found = program.classes.find("mywidget");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "parsed class should be indexed by normalized class name");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.name == "MyWidget",
                                        "parsed class should preserve declared class name text");
        copperfin::test_support::expect(class_definition.base_class_name == "Custom",
                                        "parsed class should preserve declared base class text");
        copperfin::test_support::expect(class_definition.property_statements.size() == 2U,
                                        "parsed class should preserve class-level property assignments");
        if (class_definition.property_statements.size() == 2U) {
            copperfin::test_support::expect(
                class_definition.property_statements[0].kind == copperfin::runtime::StatementKind::assignment,
                "class-level default property lines should parse as assignment statements");
            copperfin::test_support::expect(class_definition.property_statements[0].identifier == "Caption",
                                            "class-level property assignment should preserve target text");
            copperfin::test_support::expect(class_definition.property_statements[0].expression == "'Demo'",
                                            "class-level property assignment should preserve expression text");
            copperfin::test_support::expect(class_definition.property_statements[1].identifier == "nCount",
                                            "second class property assignment should preserve target text");
        }

        copperfin::test_support::expect(class_definition.methods.size() == 2U,
                                        "parsed class should preserve contained method declarations");
        const auto init_method = class_definition.methods.find("init");
        copperfin::test_support::expect(init_method != class_definition.methods.end(),
                                        "parsed class should expose PROCEDURE methods by normalized name");
        if (init_method != class_definition.methods.end()) {
            copperfin::test_support::expect(init_method->second.kind == copperfin::runtime::RoutineKind::procedure,
                                            "PROCEDURE members should preserve procedure kind");
            copperfin::test_support::expect(!init_method->second.statements.empty(),
                                            "class method body should preserve parsed statements");
        }

        const auto save_method = class_definition.methods.find("cansave");
        copperfin::test_support::expect(save_method != class_definition.methods.end(),
                                        "parsed class should expose FUNCTION methods by normalized name");
        if (save_method != class_definition.methods.end()) {
            copperfin::test_support::expect(save_method->second.kind == copperfin::runtime::RoutineKind::function,
                                            "FUNCTION members should preserve function kind");
            copperfin::test_support::expect(!save_method->second.statements.empty(),
                                            "class function body should preserve parsed statements");
        }
    }

    copperfin::test_support::expect(program.main.statements.size() == 1U,
                                    "statements after ENDDEFINE should return to the top-level main routine");
    fs::remove_all(temp_root, ignored);
}

void test_parse_mixed_class_blocks_and_top_level_routines_stays_stable() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_mixed_class_and_routines";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "mixed.prg";
    copperfin::test_support::write_text(
        program_path,
        "FUNCTION Helper\n"
        "    RETURN 41\n"
        "ENDFUNC\n"
        "DEFINE CLASS Worker AS Session OLEPUBLIC\n"
        "    Name = 'W'\n"
        "    PROCEDURE Run\n"
        "        RETURN 1\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "PROCEDURE Finish\n"
        "    RETURN\n"
        "ENDPROC\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    copperfin::test_support::expect(program.routines.size() == 2U,
                                    "mixed PRG should keep ordinary top-level routines in the routine table");
    copperfin::test_support::expect(program.routines.find("helper") != program.routines.end(),
                                    "mixed PRG should preserve top-level FUNCTION declarations");
    copperfin::test_support::expect(program.routines.find("finish") != program.routines.end(),
                                    "mixed PRG should preserve top-level PROCEDURE declarations");
    copperfin::test_support::expect(program.classes.size() == 1U,
                                    "mixed PRG should also preserve parsed class definitions");

    const auto class_found = program.classes.find("worker");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "mixed PRG should index parsed class definitions separately");
    if (class_found != program.classes.end()) {
        copperfin::test_support::expect(class_found->second.base_class_name == "Session OLEPUBLIC",
                                        "parsed class should preserve full trailing base-class text");
        copperfin::test_support::expect(class_found->second.methods.size() == 1U,
                                        "mixed PRG should keep class methods inside the class-owned method table");
        copperfin::test_support::expect(class_found->second.methods.find("run") != class_found->second.methods.end(),
                                        "parsed class should keep member procedures separate from top-level routines");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_parse_define_class_captures_metadata_and_methods();
    test_parse_mixed_class_blocks_and_top_level_routines_stays_stable();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
