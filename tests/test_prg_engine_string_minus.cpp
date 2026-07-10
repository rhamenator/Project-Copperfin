// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

using namespace copperfin::test_support;

void test_string_minus_uses_vfp_trailing_space_concatenation()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_string_minus_target";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "string_minus.prg";
    write_text(
        main_path,
        "cBasic = 'ab' - 'cd'\n"
        "cTrailing = 'ab  ' - 'cd '\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3844: string minus script should complete instead of throwing");

    const auto basic = state.globals.find("cbasic");
    const auto trailing = state.globals.find("ctrailing");
    expect(basic != state.globals.end(), "#3844: basic string-minus result should be captured");
    expect(trailing != state.globals.end(), "#3844: trailing-space string-minus result should be captured");
    if (basic != state.globals.end())
    {
        expect(copperfin::runtime::format_value(basic->second) == "abcd",
               "#3844: 'ab' - 'cd' should concatenate strings without crashing");
    }
    if (trailing != state.globals.end())
    {
        expect(copperfin::runtime::format_value(trailing->second) == "abcd   ",
               "#3844: string minus should move left trailing spaces to the end of the concatenated result");
    }

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_string_minus_uses_vfp_trailing_space_concatenation();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
