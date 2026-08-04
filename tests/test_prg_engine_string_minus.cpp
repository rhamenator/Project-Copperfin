// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"

#include <filesystem>
#include <iostream>
#include <system_error>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

namespace
{

using namespace copperfin::test_support;

void test_string_plus_preserves_typed_operands_and_rejects_mixed_types()
{
    namespace fs = std::filesystem;
    const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_string_plus_types";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "string_plus.prg";
    write_text(
        main_path,
        "cBasic = 'ab' + 'cd'\n"
        "cNumericText = '1' + '2'\n"
        "nNumeric = 1 + 2\n"
        "cLeft = 'abc' + 1\n"
        "nAfterLeft = 1\n"
        "cRight = 1 + 'abc'\n"
        "nAfterRight = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#3887: character-plus-numeric should pause with a runtime error");
    expect(state.location.line == 4U, "#3887: character-plus-numeric should identify the faulting line");
    expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
           "#3887: mixed string-plus operands should report VFP Error 107 prose");
    expect(has_runtime_event(state.events, "runtime.error", state.message),
           "#3887: mixed string-plus should preserve the invariant runtime.error category");

    const auto basic = state.globals.find("cbasic");
    const auto numeric_text = state.globals.find("cnumerictext");
    const auto numeric = state.globals.find("nnumeric");
    expect(basic != state.globals.end() && copperfin::runtime::format_value(basic->second) == "abcd",
           "#3887: character pairs should retain ordinary plus concatenation");
    expect(numeric_text != state.globals.end() && copperfin::runtime::format_value(numeric_text->second) == "12",
           "#3887: numeric-looking character values should remain character concatenation");
    expect(numeric != state.globals.end() && copperfin::runtime::format_value(numeric->second) == "3",
           "#3887: numeric pairs should retain ordinary addition");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#3887: numeric-plus-character should also pause with a runtime error");
    expect(state.location.line == 6U, "#3887: the second mixed-type fault should identify its source line");
    expect(has_runtime_event(state.events, "runtime.error", state.message),
           "#3887: the second mixed string-plus fault should keep the machine event category invariant");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3887: continuing after mixed string-plus faults should keep the session alive");
    expect(state.globals.contains("nafterleft") && state.globals.contains("nafterright"),
           "#3887: statements after mixed string-plus faults should execute");

    fs::remove_all(temp_root, ignored);
}

void test_string_plus_type_mismatch_uses_the_selected_locale()
{
    namespace fs = std::filesystem;
    const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_string_plus_locale";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "localized_string_plus.prg";
    write_text(main_path, "cValue = 'abc' + 1\nRETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#3887: qps-ploc mixed string-plus should pause with a runtime error");
    expect(state.location.line == 1U,
           "#3887: qps-ploc mixed string-plus should preserve the locale-invariant fault line");
    expect(state.message.find("[!!") != std::string::npos,
           "#3887: mixed string-plus should route Error 107 prose through qps-ploc");
    expect(state.message.find("Operator/operand type mismatch.") == std::string::npos,
           "#3887: qps-ploc mixed string-plus should not leak raw English prose");
    expect(has_runtime_event(state.events, "runtime.error", state.message),
           "#3887: localization should not change the runtime.error category");

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed,
           "#3887: qps-ploc mixed string-plus should remain continuable after the localized fault");

    fs::remove_all(temp_root, ignored);
}

void test_string_plus_rejects_character_date_time_mixes()
{
    namespace fs = std::filesystem;
    const ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_string_plus_date_time";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "string_plus_date_time.prg";
    write_text(
        main_path,
        "cDateRight = 'abc' + DATE(2026, 7, 12)\n"
        "nAfterDateRight = 1\n"
        "cDateLeft = DATE(2026, 7, 12) + 'abc'\n"
        "nAfterDateLeft = 2\n"
        "cDateTimeRight = 'abc' + DATETIME(2026, 7, 12, 3, 4, 5)\n"
        "nAfterDateTimeRight = 3\n"
        "cDateTimeLeft = DATETIME(2026, 7, 12, 3, 4, 5) + 'abc'\n"
        "nAfterDateTimeLeft = 4\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    const std::size_t fault_lines[] = {1U, 3U, 5U, 7U};
    for (const std::size_t line : fault_lines)
    {
        const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#3887: character/Date/DateTime plus should pause with a runtime error");
        expect(state.location.line == line,
               "#3887: character/Date/DateTime plus should identify each faulting source line");
        expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
               "#3887: character/Date/DateTime plus should retain Error 107 prose");
    }

    const auto completed = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(completed.completed,
           "#3887: continuing after character/Date/DateTime plus faults should keep the session alive");
    expect(completed.globals.contains("nafterdateright") && completed.globals.contains("nafterdateleft") &&
               completed.globals.contains("nafterdatetimeright") && completed.globals.contains("nafterdatetimeleft"),
           "#3887: statements after character/Date/DateTime plus faults should execute");

    fs::remove_all(temp_root, ignored);
}

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

void test_string_minus_rejects_mixed_operand_types_without_ending_the_session()
{
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_string_minus_type_mismatch";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "mixed_minus.prg";
    write_text(
        main_path,
        "cLeft = 'abc' - 1\n"
        "nAfterLeft = 1\n"
        "cRight = 1 - 'abc'\n"
        "nAfterRight = 2\n"
        "RETURN\n");

    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path.string(), temp_root.string()));

    auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#3875: character-minus-numeric should pause with a runtime error");
    expect(state.location.line == 1U, "#3875: character-minus-numeric should identify the faulting line");
    expect(state.message.find("Operator/operand type mismatch.") != std::string::npos,
           "#3875: mixed string-minus operands should report VFP Error 107 prose");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.reason == copperfin::runtime::DebugPauseReason::error,
           "#3875: numeric-minus-character should also pause with a runtime error");
    expect(state.location.line == 3U, "#3875: the second mixed-type fault should identify its source line");

    state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(state.completed, "#3875: continuing after mixed string-minus faults should keep the session alive");
    expect(state.globals.contains("nafterleft") && state.globals.contains("nafterright"),
           "#3875: statements after trapped mixed-type faults should execute");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_string_plus_preserves_typed_operands_and_rejects_mixed_types();
    test_string_plus_type_mismatch_uses_the_selected_locale();
    test_string_plus_rejects_character_date_time_mixes();
    test_string_minus_uses_vfp_trailing_space_concatenation();
    test_string_minus_rejects_mixed_operand_types_without_ending_the_session();

    if (copperfin::test_support::test_failures() != 0)
    {
        std::cerr << copperfin::test_support::test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
