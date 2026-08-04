// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_macro_expansion_preserves_expression_precedence_and_forms() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_macro_expression_forms";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "macro_expression_forms.prg";
    write_text(
        main_path,
        "cExpr = '1 + 1'\n"
        "nMixedPrecedence = 2 * &cExpr\n"
        "cValue = 'ready'\n"
        "cFieldName = 'cValue'\n"
        "cFieldResult = &cFieldName\n"
        "cFunctionName = 'LEN'\n"
        "nMacroFunctionResult = &cFunctionName('abcd')\n"
        "DIMENSION aValues[2]\n"
        "aValues[1] = 'first'\n"
        "aValues[2] = 'second'\n"
        "cIndex = '2'\n"
        "cArrayResult = aValues[&cIndex]\n"
        "cAngleSource = '\"A<<B\"'\n"
        "cLiteralAngleResult = &cAngleSource\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(
            make_runtime_session_options(main_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed,
           std::string("macro expression forms should complete: ") + state.message);

    const auto check = [&](const std::string &name, const std::string &expected) {
        const auto found = state.globals.find(name);
        expect(found != state.globals.end(), name + " should be captured");
        if (found != state.globals.end()) {
            expect(copperfin::runtime::format_value(found->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(found->second) + "'");
        }
    };
    check("nmixedprecedence", "3");
    check("cfieldresult", "ready");
    check("nmacrofunctionresult", "4");
    check("carrayresult", "second");
    check("cliteralangleresult", "A<<B");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_macro_expansion_preserves_expression_precedence_and_forms();
    if (const int failures = test_failures(); failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }
    return 0;
}
