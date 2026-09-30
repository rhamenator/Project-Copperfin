// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace {

using namespace copperfin::test_support;

// Governing requirement: RQ-CF-PRG-LITERAL-TERMINATION-001 (#6509).
//
// Installed VFP9 (COM, 2026-09-23) rejects a PRG whose expression contains an
// unclosed single-quote, double-quote, bracket or brace literal (error 1099).
// Copperfin used to return the accumulated text at end of input and execute the
// statement as if the literal were complete.

std::string global_text(const copperfin::runtime::RuntimePauseState &state, const std::string &name) {
    const auto found = state.globals.find(name);
    return found == state.globals.end() ? std::string("<missing>") : copperfin::runtime::format_value(found->second);
}

void test_unterminated_literals_are_rejected_before_the_statement_executes() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_unterminated_literals_6509";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        copperfin::localization::select_locale());
    const auto message_for = [&](const std::string &key) {
        return catalog.translate("Runtime.Prg.Expression.Error." + key);
    };

    struct Case {
        std::string name;
        std::string statement;   // the malformed statement
        std::string key;         // expected localization key suffix
    };
    const std::vector<Case> cases = {
        {"single_quote", "x = 'unterminated", "UnterminatedStringLiteral"},
        {"double_quote", "x = \"unterminated", "UnterminatedStringLiteral"},
        {"bracket", "x = [unterminated", "UnterminatedBracketLiteral"},
        {"brace", "x = {unterminated", "UnterminatedBraceLiteral"},
        {"nested_brace", "x = {outer{inner}", "UnterminatedBraceLiteral"},
        {"quoted_brace_inside_brace", "x = {'}'", "UnterminatedBraceLiteral"},
        {"return_expression", "RETURN 'unterminated", "UnterminatedStringLiteral"},
        {"function_argument", "x = LEN('unterminated", "UnterminatedStringLiteral"},
    };

    const auto run_script = [&](const std::string &name, const std::string &body) {
        const fs::path script_path = temp_root / (name + ".prg");
        write_text(script_path, body);
        auto session = copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(script_path.string(), temp_root.string(), false));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    for (const Case &c : cases) {
        // Uncaught: execution pauses at the malformed statement with the localized
        // error, and a following statement never runs.
        {
            const auto state = run_script(c.name + "_uncaught",
                "nBefore = 1\n" + c.statement + "\nnAfter = 1\nRETURN\n");
            expect(!state.completed, "#6509 " + c.name + ": a malformed literal must not report successful completion");
            expect(state.message.find(message_for(c.key)) != std::string::npos,
                "#6509 " + c.name + ": expected the localized " + c.key + " diagnostic, got: " + state.message);
            expect(global_text(state, "nbefore") == "1", "#6509 " + c.name + ": statements before the error still run");
            expect(global_text(state, "nafter") == "<missing>",
                "#6509 " + c.name + ": the statement after the parse error must not execute");
        }
        // Caught: the parse failure is a catchable error, and execution resumes afterwards.
        {
            const auto state = run_script(c.name + "_caught",
                "lErr = .F.\n"
                "cMsg = ''\n"
                "TRY\n" + c.statement + "\nCATCH TO oErr\n"
                "lErr = .T.\n"
                "cMsg = oErr.Message\n"
                "ENDTRY\n"
                "lAfter = .T.\n"
                "RETURN\n");
            expect(state.completed, "#6509 " + c.name + ": a caught malformed literal should let the script finish: " + state.message);
            expect(global_text(state, "lerr") == "true", "#6509 " + c.name + ": the malformed literal should be catchable");
            expect(global_text(state, "cmsg").find(message_for(c.key)) != std::string::npos,
                "#6509 " + c.name + ": caught message should be the localized " + c.key + ", got: " + global_text(state, "cmsg"));
            expect(global_text(state, "lafter") == "true", "#6509 " + c.name + ": execution continues after CATCH");
        }
    }

    fs::remove_all(temp_root, ignored);
}

// Valid literals, including doubled delimiters, must keep working exactly as before.
void test_valid_literal_forms_still_evaluate() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_valid_literals_6509";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path script_path = temp_root / "valid_literals.prg";
    write_text(
        script_path,
        "c1 = 'closed'\n"
        "c2 = \"closed\"\n"
        "c3 = [closed]\n"
        "c4 = 'it''s'\n"
        "c5 = \"say \"\"hi\"\"\"\n"
        "c6 = [a]]b]\n"
        "c7 = ''\n"
        "c8 = ''''\n"
        "n9 = LEN('a' + \"b\" + [c])\n"
        "RETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(script_path.string(), temp_root.string(), false));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#6509 valid literals: script should complete: " + state.message);
    const auto check = [&](const std::string &name, const std::string &expected) {
        expect(global_text(state, name) == expected,
            "#6509 valid literals: " + name + " expected '" + expected + "' got '" + global_text(state, name) + "'");
    };
    check("c1", "closed");
    check("c2", "closed");
    check("c3", "closed");
    check("c4", "it's");
    check("c5", "say \"hi\"");
    check("c6", "a]b");
    check("c7", "");
    check("c8", "'");
    check("n9", "3");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_valid_literal_forms_still_evaluate();
    test_unterminated_literals_are_rejected_before_the_statement_executes();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
