// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace
{

using namespace copperfin::test_support;

void test_digit_only_numeric_pictures()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_transform_numeric_pictures";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "transform_numeric_pictures.prg";
    write_text(
        main_path,
        "cNine = TRANSFORM(3, '999')\n"
        "cZero = TRANSFORM(3, '000')\n"
        "cZeroValue = TRANSFORM(0, '000')\n"
        "cRounded = TRANSFORM(9.6, '99')\n"
        "cNegative = TRANSFORM(-5, '999')\n"
        "cOverflow = TRANSFORM(1234, '999')\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "digit-only TRANSFORM script should complete: " + state.message);

    const auto check = [&](const std::string& name, const std::string& expected)
    {
        const auto it = state.globals.find(name);
        expect(it != state.globals.end(), name + " variable should be present");
        if (it != state.globals.end())
        {
            expect(copperfin::runtime::format_value(it->second) == expected,
                   name + " expected '" + expected + "' got '" +
                       copperfin::runtime::format_value(it->second) + "'");
        }
    };

    check("cnine", "  3");
    check("czero", "003");
    check("czerovalue", "000");
    check("crounded", "10");
    check("cnegative", " -5");
    check("coverflow", "***");

    fs::remove_all(temp_root, ignored);
}

} // namespace

int main()
{
    test_digit_only_numeric_pictures();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
