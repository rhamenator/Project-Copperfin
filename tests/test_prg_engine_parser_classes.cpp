// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "../src/runtime/prg_engine_internal.h"
#include "../src/runtime/prg_engine_helpers.h"
#include "prg_engine_test_support.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <locale>
#include <system_error>

namespace {

namespace fs = std::filesystem;

class comma_decimal_numpunct final : public std::numpunct<char> {
protected:
    char do_decimal_point() const override { return ','; }
    char do_thousands_sep() const override { return '.'; }
    std::string do_grouping() const override { return "\3"; }
};

class scoped_global_locale {
public:
    explicit scoped_global_locale(const std::locale& replacement)
        : previous_(std::locale::global(replacement)) {}

    ~scoped_global_locale() { std::locale::global(previous_); }

    scoped_global_locale(const scoped_global_locale&) = delete;
    scoped_global_locale& operator=(const scoped_global_locale&) = delete;

private:
    std::locale previous_;
};

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

void test_parse_define_class_captures_protected_and_hidden_member_visibility() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_member_visibility";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "visibility.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS VisibilityDemo AS Custom\n"
        "    PROTECTED ProtectedValue, ProtectedAlias\n"
        "    HIDDEN HiddenValue\n"
        "    ProtectedValue = 1\n"
        "    ProtectedAlias = 2\n"
        "    HiddenValue = 3\n"
        "    PROTECTED PROCEDURE ProtectedMethod\n"
        "        RETURN THIS.ProtectedValue\n"
        "    ENDPROC\n"
        "    HIDDEN FUNCTION HiddenMethod\n"
        "        RETURN THIS.HiddenValue\n"
        "    ENDFUNC\n"
        "    PROCEDURE PublicMethod\n"
        "        RETURN THIS.ProtectedAlias\n"
        "    ENDPROC\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("visibilitydemo");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "visibility declarations should preserve the owning class");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        const auto protected_property = class_definition.member_visibility.find("protectedvalue");
        const auto protected_alias = class_definition.member_visibility.find("protectedalias");
        const auto hidden_property = class_definition.member_visibility.find("hiddenvalue");
        copperfin::test_support::expect(
            protected_property != class_definition.member_visibility.end() &&
                protected_property->second == copperfin::runtime::NativeMemberVisibility::protected_member,
            "PROTECTED property lists should record protected visibility");
        copperfin::test_support::expect(
            protected_alias != class_definition.member_visibility.end() &&
                protected_alias->second == copperfin::runtime::NativeMemberVisibility::protected_member,
            "comma-separated PROTECTED property lists should record every member");
        copperfin::test_support::expect(
            hidden_property != class_definition.member_visibility.end() &&
                hidden_property->second == copperfin::runtime::NativeMemberVisibility::hidden_member,
            "HIDDEN property declarations should record hidden visibility");

        const auto protected_method = class_definition.methods.find("protectedmethod");
        const auto hidden_method = class_definition.methods.find("hiddenmethod");
        const auto public_method = class_definition.methods.find("publicmethod");
        copperfin::test_support::expect(
            protected_method != class_definition.methods.end() &&
                protected_method->second.visibility == copperfin::runtime::NativeMemberVisibility::protected_member,
            "PROTECTED method modifiers should preserve method visibility");
        copperfin::test_support::expect(
            hidden_method != class_definition.methods.end() &&
                hidden_method->second.visibility == copperfin::runtime::NativeMemberVisibility::hidden_member,
            "HIDDEN method modifiers should preserve method visibility");
        copperfin::test_support::expect(
            public_method != class_definition.methods.end() &&
                public_method->second.visibility == copperfin::runtime::NativeMemberVisibility::public_member,
            "unmodified methods should remain public by default");
    }

    copperfin::test_support::expect(program.main.statements.empty(),
                                    "visibility declarations should not leak into top-level executable statements");
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

void test_parse_define_class_external_prg_base_sources() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_define_class_external_base";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "external_base.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS ChildWidget AS ParentWidget OF widgets.prg\n"
        "    Caption = 'Child'\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("childwidget");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "external base class PRG should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.base_class_name == "ParentWidget",
                                        "external base class PRG should preserve the base class name separately from the source path");
        copperfin::test_support::expect(class_definition.base_class_source_path == "widgets.prg",
                                        "external base class PRG should preserve the external base source path");
        copperfin::test_support::expect(class_definition.property_statements.size() == 1U,
                                        "external base class PRG should keep ordinary class property assignments stable");
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_class_body_add_object_declarations_distinct_from_property_assignments() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_class_body_add_object";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "class_body_add_object.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS DemoForm AS Custom\n"
        "    Caption = 'Demo'\n"
        "    ADD OBJECT cmdSave AS SaveButton\n"
        "    PROCEDURE Init\n"
        "        RETURN THIS.Caption\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "DEFINE CLASS SaveButton AS Custom\n"
        "    Caption = 'Save'\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "class-body ADD OBJECT test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.property_statements.size() == 1U,
                                        "class-body ADD OBJECT should stay out of ordinary class property assignments");
        if (class_definition.property_statements.size() == 1U) {
            copperfin::test_support::expect(class_definition.property_statements[0].identifier == "Caption",
                                            "class-body ADD OBJECT should preserve surrounding property assignments");
        }
        copperfin::test_support::expect(class_definition.child_object_declarations.size() == 1U,
                                        "class-body ADD OBJECT should be captured as a dedicated child-object declaration");
        if (class_definition.child_object_declarations.size() == 1U) {
            copperfin::test_support::expect(class_definition.child_object_declarations[0].name == "cmdSave",
                                            "class-body ADD OBJECT should preserve the declared child name text");
            copperfin::test_support::expect(class_definition.child_object_declarations[0].class_name == "SaveButton",
                                            "class-body ADD OBJECT should preserve the declared child class text");
        }
    }

    copperfin::test_support::expect(program.main.statements.empty(),
                                    "class-body ADD OBJECT declarations should not leak into top-level main statements");
    fs::remove_all(temp_root, ignored);
}

void test_parse_class_body_add_object_with_property_clauses() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_class_body_add_object_with";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "class_body_add_object_with.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS DemoForm AS Custom\n"
        "    Caption = 'Demo'\n"
        "    ADD OBJECT cmdSave AS SaveButton WITH Caption = 'Commit', nPriority = 7\n"
        "ENDDEFINE\n"
        "DEFINE CLASS SaveButton AS Custom\n"
        "    Caption = 'Save'\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "class-body ADD OBJECT WITH test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.property_statements.size() == 1U,
                                        "class-body ADD OBJECT WITH should stay out of ordinary class property assignments");
        copperfin::test_support::expect(class_definition.child_object_declarations.size() == 1U,
                                        "class-body ADD OBJECT WITH should be captured as a child-object declaration");
        if (class_definition.child_object_declarations.size() == 1U) {
            const auto& declaration = class_definition.child_object_declarations[0];
            copperfin::test_support::expect(declaration.name == "cmdSave",
                                            "class-body ADD OBJECT WITH should preserve the declared child name text");
            copperfin::test_support::expect(declaration.class_name == "SaveButton",
                                            "class-body ADD OBJECT WITH should preserve the declared child class text");
            copperfin::test_support::expect(declaration.property_statements.size() == 2U,
                                            "class-body ADD OBJECT WITH should preserve each property clause separately");
            if (declaration.property_statements.size() == 2U) {
                copperfin::test_support::expect(declaration.property_statements[0].identifier == "Caption",
                                                "class-body ADD OBJECT WITH should preserve the first child property name");
                copperfin::test_support::expect(declaration.property_statements[0].expression == "'Commit'",
                                                "class-body ADD OBJECT WITH should preserve the first child property expression");
                copperfin::test_support::expect(declaration.property_statements[1].identifier == "nPriority",
                                                "class-body ADD OBJECT WITH should preserve the second child property name");
                copperfin::test_support::expect(declaration.property_statements[1].expression == "7",
                                                "class-body ADD OBJECT WITH should preserve the second child property expression");
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_class_body_object_blocks_capture_child_properties() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_class_body_object_block";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "class_body_object_block.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS DemoForm AS Custom\n"
        "    Caption = 'Demo'\n"
        "    OBJECT cmdSave AS SaveButton\n"
        "        Caption = 'Commit'\n"
        "        nPriority = 7\n"
        "    ENDOBJECT\n"
        "ENDDEFINE\n"
        "DEFINE CLASS SaveButton AS Custom\n"
        "    Caption = 'Save'\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "class-body OBJECT block test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.property_statements.size() == 1U,
                                        "class-body OBJECT block should stay out of ordinary class property assignments");
        copperfin::test_support::expect(class_definition.child_object_declarations.size() == 1U,
                                        "class-body OBJECT block should be captured as a child-object declaration");
        if (class_definition.child_object_declarations.size() == 1U) {
            const auto& declaration = class_definition.child_object_declarations[0];
            copperfin::test_support::expect(declaration.name == "cmdSave",
                                            "class-body OBJECT block should preserve the declared child name text");
            copperfin::test_support::expect(declaration.class_name == "SaveButton",
                                            "class-body OBJECT block should preserve the declared child class text");
            copperfin::test_support::expect(declaration.property_statements.size() == 2U,
                                            "class-body OBJECT block should preserve each contained child property assignment");
            if (declaration.property_statements.size() == 2U) {
                copperfin::test_support::expect(declaration.property_statements[0].identifier == "Caption",
                                                "class-body OBJECT block should preserve the first child property name");
                copperfin::test_support::expect(declaration.property_statements[0].expression == "'Commit'",
                                                "class-body OBJECT block should preserve the first child property expression");
                copperfin::test_support::expect(declaration.property_statements[1].identifier == "nPriority",
                                                "class-body OBJECT block should preserve the second child property name");
                copperfin::test_support::expect(declaration.property_statements[1].expression == "7",
                                                "class-body OBJECT block should preserve the second child property expression");
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_declarative_child_external_prg_sources() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_child_external_prg";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "class_body_child_external_prg.prg";
    copperfin::test_support::write_text(
        program_path,
        "DEFINE CLASS DemoForm AS Custom\n"
        "    ADD OBJECT cmdSave AS SaveButton OF buttons.prg\n"
        "    OBJECT cmdArchive AS ArchiveButton OF buttons.prg\n"
        "        Caption = 'ArchiveNow'\n"
        "    ENDOBJECT\n"
        "ENDDEFINE\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "declarative child external PRG test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.child_object_declarations.size() == 2U,
                                        "declarative child external PRG test should capture both child declarations");
        if (class_definition.child_object_declarations.size() == 2U) {
            const auto& add_declaration = class_definition.child_object_declarations[0];
            const auto& block_declaration = class_definition.child_object_declarations[1];
            copperfin::test_support::expect(add_declaration.name == "cmdSave",
                                            "declarative child external PRG test should preserve the one-line child name");
            copperfin::test_support::expect(add_declaration.class_name == "SaveButton",
                                            "declarative child external PRG test should preserve the one-line child class name");
            copperfin::test_support::expect(add_declaration.source_path == "buttons.prg",
                                            "declarative child external PRG test should preserve the one-line child source path");
            copperfin::test_support::expect(add_declaration.property_statements.empty(),
                                            "declarative child external PRG test should keep the plain one-line child free of property clauses");

            copperfin::test_support::expect(block_declaration.name == "cmdArchive",
                                            "declarative child external PRG test should preserve the block child name");
            copperfin::test_support::expect(block_declaration.class_name == "ArchiveButton",
                                            "declarative child external PRG test should preserve the block child class name");
            copperfin::test_support::expect(block_declaration.source_path == "buttons.prg",
                                            "declarative child external PRG test should preserve the block child source path");
            copperfin::test_support::expect(block_declaration.property_statements.size() == 1U,
                                            "declarative child external PRG test should preserve block child property assignments");
            if (block_declaration.property_statements.size() == 1U) {
                copperfin::test_support::expect(block_declaration.property_statements[0].identifier == "Caption",
                                                "declarative child external PRG test should preserve block child property name");
                copperfin::test_support::expect(block_declaration.property_statements[0].expression == "'ArchiveNow'",
                                                "declarative child external PRG test should preserve block child property expression");
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_include_and_define_constants_expand_before_class_body_parsing() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_include_define_constants";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path header_path = temp_root / "ui.h";
    copperfin::test_support::write_text(
        header_path,
        "#define WINDOWTYPE_MODELESS 0\n"
        "#define WINDOWTYPE_MODAL 1\n"
        "#define FORM_CAPTION 'IncludedDemo'\n"
        "#define LINE_BREAK\tchr(13)\n");

    const fs::path program_path = temp_root / "include_define_demo.prg";
    copperfin::test_support::write_text(
        program_path,
        "#INCLUDE \"ui.h\"\n"
        "DEFINE CLASS DemoForm AS Form\n"
        "    Caption = FORM_CAPTION\n"
        "    WindowType = WINDOWTYPE_MODAL\n"
        "    PROCEDURE DisplayCaption\n"
        "        result = FORM_CAPTION + LINE_BREAK\n"
        "    ENDPROC\n"
        "ENDDEFINE\n"
        "nWindowType = WINDOWTYPE_MODELESS\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const copperfin::runtime::Program source_program = copperfin::runtime::parse_program_source(
        program_path.string(),
        copperfin::test_support::read_text(program_path));
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "include/define constant parser test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.property_statements.size() == 2U,
                                        "include/define constant parser test should preserve both class property assignments");
        if (class_definition.property_statements.size() == 2U) {
            copperfin::test_support::expect(class_definition.property_statements[0].identifier == "Caption",
                                            "include/define constant parser test should preserve the class caption property name");
            copperfin::test_support::expect(class_definition.property_statements[0].expression == "'IncludedDemo'",
                                            "include/define constant parser test should expand included string constants in class assignments");
            copperfin::test_support::expect(class_definition.property_statements[1].identifier == "WindowType",
                                            "include/define constant parser test should preserve the class windowtype property name");
            copperfin::test_support::expect(class_definition.property_statements[1].expression == "1",
                                            "include/define constant parser test should expand included numeric constants in class assignments");
        }
        const auto method_found = class_definition.methods.find("displaycaption");
        copperfin::test_support::expect(method_found != class_definition.methods.end(),
                                        "include/define constant parser test should preserve class methods");
        if (method_found != class_definition.methods.end()) {
            copperfin::test_support::expect(method_found->second.statements.size() == 1U,
                                            "include/define constant parser test should preserve the method assignment");
            if (method_found->second.statements.size() == 1U) {
                copperfin::test_support::expect(
                    method_found->second.statements[0].expression == "'IncludedDemo' + chr(13)",
                    "include/define constant parser test should expand included constants inside class methods");
            }
        }
    }

    const auto source_class_found = source_program.classes.find("demoform");
    copperfin::test_support::expect(source_class_found != source_program.classes.end(),
                                    "source-override include/define parser test should parse the owning class definition");
    if (source_class_found != source_program.classes.end()) {
        const auto method_found = source_class_found->second.methods.find("displaycaption");
        copperfin::test_support::expect(method_found != source_class_found->second.methods.end(),
                                        "source-override include/define parser test should preserve class methods");
        if (method_found != source_class_found->second.methods.end() && method_found->second.statements.size() == 1U) {
            copperfin::test_support::expect(
                method_found->second.statements[0].expression == "'IncludedDemo' + chr(13)",
                "source-override include/define parser test should expand included constants inside class methods");
        }
    }

    copperfin::test_support::expect(program.main.statements.size() == 1U,
                                    "include/define constant parser test should preserve top-level statements after the class");
    if (program.main.statements.size() == 1U) {
        copperfin::test_support::expect(program.main.statements[0].kind == copperfin::runtime::StatementKind::assignment,
                                        "include/define constant parser test should keep expanded top-level constants as assignments");
        copperfin::test_support::expect(program.main.statements[0].expression == "0",
                                        "include/define constant parser test should expand included top-level constants before normal parsing");
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_conditional_preprocessor_branches_and_header_guards() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_preprocessor_conditionals";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "include");

    const fs::path header_path = temp_root / "include" / "ui.h";
    copperfin::test_support::write_text(
        header_path,
        "#DEFINE WINDOWTYPE_MODELESS 0\n"
        "#IF 1\n"
        "#DEFINE WINDOWTYPE_MODAL 1\n"
        "#ELSE\n"
        "#DEFINE WINDOWTYPE_MODAL 99\n"
        "#ENDIF\n"
        "#IFNDEF FORM_CAPTION\n"
        "#DEFINE FORM_CAPTION 'GuardedCaption'\n"
        "#ENDIF\n");

    const fs::path program_path = temp_root / "conditional_demo.prg";
    copperfin::test_support::write_text(
        program_path,
        "#DEFINE FORM_CAPTION 'CallerCaption'\n"
        "#INCLUDE include\\\\UI.H\n"
        "DEFINE CLASS DemoForm AS Form\n"
        "    Caption = FORM_CAPTION\n"
        "    WindowType = WINDOWTYPE_MODAL\n"
        "ENDDEFINE\n"
        "#IF 0\n"
        "nBranch = 99\n"
        "#ELSE\n"
        "nBranch = WINDOWTYPE_MODELESS\n"
        "#ENDIF\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    const auto class_found = program.classes.find("demoform");
    copperfin::test_support::expect(class_found != program.classes.end(),
                                    "conditional preprocessor parser test should parse the owning class definition");
    if (class_found != program.classes.end()) {
        const auto& class_definition = class_found->second;
        copperfin::test_support::expect(class_definition.property_statements.size() == 2U,
                                        "conditional preprocessor parser test should preserve both class property assignments");
        if (class_definition.property_statements.size() == 2U) {
            copperfin::test_support::expect(class_definition.property_statements[0].expression == "'CallerCaption'",
                                            "conditional preprocessor parser test should respect #IFNDEF guard suppression when the caller already defined a value");
            copperfin::test_support::expect(class_definition.property_statements[1].expression == "1",
                                            "conditional preprocessor parser test should keep the active #IF branch constant instead of the #ELSE fallback");
        }
    }

    copperfin::test_support::expect(program.main.statements.size() == 1U,
                                    "conditional preprocessor parser test should skip inactive top-level branches");
    if (program.main.statements.size() == 1U) {
        copperfin::test_support::expect(program.main.statements[0].kind == copperfin::runtime::StatementKind::assignment,
                                        "conditional preprocessor parser test should keep the active branch assignment");
        copperfin::test_support::expect(program.main.statements[0].expression == "0",
                                        "conditional preprocessor parser test should expand constants from the active branch through a Windows-style include path");
    }

    fs::remove_all(temp_root, ignored);
}

void test_preprocessor_numeric_text_comparison_ignores_global_locale() {
    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_prg_parser_numeric_text_locale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "numeric_text_locale.prg";
    copperfin::test_support::write_text(
        program_path,
        "#IF 1.25 $ \"1.25\"\n"
        "nMatched = 1\n"
        "#ELSE\n"
        "nMatched = 0\n"
        "#ENDIF\n");

    const std::locale comma_locale(std::locale::classic(), new comma_decimal_numpunct());
    scoped_global_locale locale_guard(comma_locale);
    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    copperfin::test_support::expect(
        program.main.statements.size() == 1U && program.main.statements[0].expression == "1",
        "preprocessor numeric text comparisons should remain period-decimal under a comma-decimal global locale");

    fs::remove_all(temp_root, ignored);
}

void test_invariant_numeric_parser_preserves_vfp_decimal_contract() {
    using copperfin::runtime::try_parse_invariant_double;

    const auto decimal = try_parse_invariant_double("1.25");
    copperfin::test_support::expect(
        decimal.has_value() && *decimal == 1.25,
        "VFP numeric parsing should use the invariant period decimal separator");

    const auto exponent = try_parse_invariant_double("+1.25e2");
    copperfin::test_support::expect(
        exponent.has_value() && *exponent == 125.0,
        "VFP numeric parsing should preserve exponent and leading-plus forms");

    const auto signed_exponent = try_parse_invariant_double("-1.25e-2");
    copperfin::test_support::expect(
        signed_exponent.has_value() && *signed_exponent == -0.0125,
        "VFP numeric parsing should preserve independent mantissa and exponent signs");

    copperfin::test_support::expect(
        !try_parse_invariant_double("1,25").has_value() &&
            !try_parse_invariant_double("1.25 trailing").has_value() &&
            !try_parse_invariant_double(" 1.25").has_value() &&
            !try_parse_invariant_double("1e9999").has_value() &&
            !try_parse_invariant_double("+-1").has_value() &&
            !try_parse_invariant_double("-+1").has_value() &&
            !try_parse_invariant_double("++1").has_value() &&
            !try_parse_invariant_double("--1").has_value() &&
            !try_parse_invariant_double("+").has_value() &&
            !try_parse_invariant_double("-").has_value(),
        "comma-decimal, whitespace, trailing-input, range, and malformed-sign errors must be rejected");

    const auto nan = try_parse_invariant_double("NaN", true);
    const auto lowercase_nan = try_parse_invariant_double("nan", true);
    const auto positive_nan = try_parse_invariant_double("+NaN", true);
    const auto signed_mixed_case_nan = try_parse_invariant_double("-nAn", true);
    const auto short_infinity = try_parse_invariant_double("INF", true);
    const auto positive_infinity = try_parse_invariant_double("+INF", true);
    const auto lowercase_infinity = try_parse_invariant_double("infinity", true);
    const auto uppercase_infinity = try_parse_invariant_double("INFINITY", true);
    const auto negative_infinity = try_parse_invariant_double("-INF", true);
    const auto signed_mixed_case_infinity =
        try_parse_invariant_double("-InFiNiTy", true);
    copperfin::test_support::expect(
        nan.has_value() && std::isnan(*nan) &&
            lowercase_nan.has_value() && std::isnan(*lowercase_nan) &&
            positive_nan.has_value() && std::isnan(*positive_nan) &&
            signed_mixed_case_nan.has_value() && std::isnan(*signed_mixed_case_nan) &&
            short_infinity.has_value() && std::isinf(*short_infinity) && *short_infinity > 0.0 &&
            positive_infinity.has_value() && std::isinf(*positive_infinity) && *positive_infinity > 0.0 &&
            lowercase_infinity.has_value() && std::isinf(*lowercase_infinity) && *lowercase_infinity > 0.0 &&
            uppercase_infinity.has_value() && std::isinf(*uppercase_infinity) && *uppercase_infinity > 0.0 &&
            negative_infinity.has_value() && std::isinf(*negative_infinity) && *negative_infinity < 0.0 &&
            signed_mixed_case_infinity.has_value() &&
            std::isinf(*signed_mixed_case_infinity) && *signed_mixed_case_infinity < 0.0 &&
            !try_parse_invariant_double("NaN").has_value() &&
            !try_parse_invariant_double("-nAn").has_value() &&
            !try_parse_invariant_double("+INF").has_value() &&
            !try_parse_invariant_double("-INF").has_value() &&
            !try_parse_invariant_double("infinity").has_value(),
        "signed and case-insensitive nonfinite parsing should remain available only to explicit binary-field consumers");
}

void test_expression_numeric_literals_preserve_exponent_grammar() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_expression_numeric_exponents";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "numeric_exponents.prg";
    copperfin::test_support::write_text(
        program_path,
        "nPositive = 1.25e2\n"
        "nNegative = 1.25e-2\n"
        "nSigned = 1e+5 + 2\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        copperfin::test_support::make_runtime_session_options(program_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    copperfin::test_support::expect(state.completed,
                                    "PRG expression exponent fixture should complete: " + state.message);
    const auto positive = state.globals.find("npositive");
    const auto negative = state.globals.find("nnegative");
    const auto signed_exponent = state.globals.find("nsigned");
    copperfin::test_support::expect(
        positive != state.globals.end() && copperfin::runtime::format_value(positive->second) == "125",
        "PRG expression parser should preserve positive exponent literals");
    copperfin::test_support::expect(
        negative != state.globals.end() && copperfin::runtime::format_value(negative->second) == "0.0125",
        "PRG expression parser should preserve negative exponent literals");
    copperfin::test_support::expect(
        signed_exponent != state.globals.end() && copperfin::runtime::format_value(signed_exponent->second) == "100002",
        "PRG expression parser should preserve explicitly signed exponent literals");

    const char* invalid_literals[] = {"1e", "1e+", "1e-"};
    for (const char* invalid_literal : invalid_literals)
    {
        const fs::path invalid_program_path = temp_root / (std::string("invalid_") + invalid_literal + ".prg");
        copperfin::test_support::write_text(
            invalid_program_path,
            std::string("nInvalid = ") + invalid_literal + "\n"
            "RETURN\n");
        auto invalid_session = copperfin::runtime::PrgRuntimeSession::create(
            copperfin::test_support::make_runtime_session_options(
                invalid_program_path.string(), temp_root.string(), false));
        const auto invalid_state = invalid_session.run(copperfin::runtime::DebugResumeAction::continue_run);
        copperfin::test_support::expect(
            !invalid_state.completed &&
                invalid_state.message.find("invalid invariant numeric literal") != std::string::npos,
            std::string("PRG expression parser should reject malformed exponent literal ") + invalid_literal);
    }

    fs::remove_all(temp_root, ignored);
}

void test_parse_declare_dll_preserves_vfp_parameter_contract() {
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_parser_declare_dll";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path program_path = temp_root / "declare_dll.prg";
    copperfin::test_support::write_text(
        program_path,
        "DECLARE DOUBLE Pow IN 'native math.dll' AS\tPowAlias DOUBLE base, DOUBLE exponent\n"
        "DECLARE INTEGER Legacy(STRING @, STRING) IN 'kernel32.dll' AS CopyText\n"
        "DECLARE LONG SomeCall IN WIN32API LONG @ value\n"
        "RETURN\n");

    const copperfin::runtime::Program program = copperfin::runtime::parse_program(program_path.string());
    copperfin::test_support::expect(program.main.statements.size() == 4U,
                                    "#3895: DECLARE parser fixture should preserve all statements");
    if (program.main.statements.size() >= 3U) {
        const auto& documented = program.main.statements[0];
        copperfin::test_support::expect(documented.kind == copperfin::runtime::StatementKind::declare_dll,
                                        "#3895: documented DECLARE syntax should retain statement kind");
        copperfin::test_support::expect(documented.identifier == "Pow",
                                        "#3895: documented DECLARE syntax should retain function name");
        copperfin::test_support::expect(documented.expression == "'native math.dll'",
                                        "#3895: quoted DECLARE library paths should stop before AS and parameter clauses");
        copperfin::test_support::expect(documented.secondary_expression == "DOUBLE",
                                        "#3895: documented DECLARE syntax should retain return type");
        copperfin::test_support::expect(documented.tertiary_expression == "DOUBLE base, DOUBLE exponent",
                                        "#3895: documented post-IN parameter declarations should retain names and order");
        copperfin::test_support::expect(documented.quaternary_expression == "PowAlias",
                                        "#3895: documented DECLARE AS aliases should stop before parameter declarations");

        const auto& legacy = program.main.statements[1];
        copperfin::test_support::expect(legacy.expression == "'kernel32.dll'" &&
                                            legacy.tertiary_expression == "STRING @, STRING" &&
                                            legacy.quaternary_expression == "CopyText",
                                        "#3895: existing parenthesized DECLARE syntax should remain stable");

        const auto& bare_library = program.main.statements[2];
        copperfin::test_support::expect(bare_library.expression == "WIN32API" &&
                                            bare_library.tertiary_expression == "LONG @ value",
                                        "#3895: bare DECLARE libraries should retain named by-reference parameter clauses");
    }

    copperfin::test_support::expect(
        !copperfin::runtime::declared_dll_type_uses_64_bit_integer("LONG") &&
            !copperfin::runtime::declared_dll_type_uses_64_bit_integer("INTEGER"),
        "#3932: documented VFP integer declaration types should retain signed 32-bit width");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_type_uses_64_bit_integer("LONGLONG") &&
            copperfin::runtime::declared_dll_type_uses_64_bit_integer("Integer64") &&
            copperfin::runtime::declared_dll_type_uses_64_bit_integer("i64"),
        "#3932: explicitly named Copperfin 64-bit declaration aliases should remain distinct from VFP LONG");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_type_is_single("Single") &&
            !copperfin::runtime::declared_dll_type_is_single("DOUBLE") &&
            !copperfin::runtime::declared_dll_type_is_single("F"),
        "#3933: documented SINGLE should remain distinct from DOUBLE and existing aliases");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_type_is_short("SHORT") &&
            !copperfin::runtime::declared_dll_type_is_short("INTEGER") &&
            !copperfin::runtime::declared_dll_type_is_short("LONG"),
        "#3938: documented SHORT returns should remain distinct from 32-bit integer types");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_type_is_numeric_parameter("INTEGER") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("I") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("LONG") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("L") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("SINGLE") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("DOUBLE") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("D") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("F") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("INTEGER64") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("I64") &&
            copperfin::runtime::declared_dll_type_is_numeric_parameter("LONGLONG") &&
            !copperfin::runtime::declared_dll_type_is_numeric_parameter("STRING") &&
            !copperfin::runtime::declared_dll_type_is_numeric_parameter("OBJECT") &&
            !copperfin::runtime::declared_dll_type_is_numeric_parameter("SHORT"),
        "#3944: numeric by-reference rejection should cover canonical and legacy declaration aliases");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_parameter_list_contains_type(
            "INTEGER first, SHORT @ invalid, DOUBLE third", "SHORT") &&
            !copperfin::runtime::declared_dll_parameter_list_contains_type(
                "INTEGER first, SINGLE second, DOUBLE third, LONG fourth, STRING @ fifth", "SHORT"),
        "#3938: help-grounded SHORT parameter rejection should not affect permitted VFP9 parameter types");
    copperfin::test_support::expect(
        copperfin::runtime::declared_dll_x86_stdcall_stack_bytes("") == 0U &&
            copperfin::runtime::declared_dll_x86_stdcall_stack_bytes("LONG value") == 4U &&
            copperfin::runtime::declared_dll_x86_stdcall_stack_bytes(
                "LONG first, DOUBLE second, INTEGER64 third, STRING @ output") == 24U &&
            copperfin::runtime::declared_dll_x86_stdcall_stack_bytes(
                "DOUBLE @ first, INTEGER64 @ second") == 8U &&
            copperfin::runtime::declared_dll_x86_stdcall_stack_bytes(
                "SINGLE first, SHORT @ second, F third") == 16U,
        "#3941: Win32 stdcall decoration should use declared widths and pointer-sized by-reference slots");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_parse_define_class_captures_metadata_and_methods();
    test_parse_define_class_captures_protected_and_hidden_member_visibility();
    test_parse_mixed_class_blocks_and_top_level_routines_stays_stable();
    test_parse_define_class_external_prg_base_sources();
    test_parse_class_body_add_object_declarations_distinct_from_property_assignments();
    test_parse_class_body_add_object_with_property_clauses();
    test_parse_class_body_object_blocks_capture_child_properties();
    test_parse_declarative_child_external_prg_sources();
    test_parse_include_and_define_constants_expand_before_class_body_parsing();
    test_parse_conditional_preprocessor_branches_and_header_guards();
    test_preprocessor_numeric_text_comparison_ignores_global_locale();
    test_invariant_numeric_parser_preserves_vfp_decimal_contract();
    test_expression_numeric_literals_preserve_exponent_grammar();
    test_parse_declare_dll_preserves_vfp_parameter_contract();

    if (copperfin::test_support::test_failures() != 0) {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
