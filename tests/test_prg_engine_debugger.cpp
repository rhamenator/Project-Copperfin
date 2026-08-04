// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "prg_engine_test_support.h"

#include <cstdlib>
#include <filesystem>
#include <system_error>

namespace {

using namespace copperfin::test_support;

void test_breakpoint_path_identity_uses_platform_semantics() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_debugger_breakpoint_case";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "BreakpointCase.prg";
    write_text(main_path, "nValue = 1\nRETURN\n");
    const std::string canonical_path = main_path.string();
    const std::string case_variant_path = uppercase_ascii(canonical_path);
    expect(case_variant_path != canonical_path,
           "breakpoint case-identity fixture must contain an ASCII alphabetic path component");

    auto matching_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    matching_session.add_breakpoint({.file_path = case_variant_path, .line = 1U});
    const auto matching_state = matching_session.run(copperfin::runtime::DebugResumeAction::continue_run);
#if defined(_WIN32)
    expect(matching_state.reason == copperfin::runtime::DebugPauseReason::breakpoint,
           "Windows debugger should hit a breakpoint whose path differs only by case");
#else
    expect(matching_state.completed,
           "POSIX debugger should retain case-sensitive breakpoint path matching");
#endif

    auto identity_session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path, temp_root));
    identity_session.add_breakpoint({.file_path = canonical_path, .line = 1U});
    identity_session.add_breakpoint({.file_path = case_variant_path, .line = 1U});
#if defined(_WIN32)
    expect(identity_session.list_breakpoints().size() == 1U,
           "Windows debugger should not retain duplicate breakpoint paths differing only by case");
#else
    expect(identity_session.list_breakpoints().size() == 2U,
           "POSIX debugger should retain distinct breakpoint paths differing by case");
#endif
    expect(identity_session.remove_breakpoint({.file_path = case_variant_path, .line = 1U}),
           "debugger should remove the requested breakpoint using platform path identity");
#if defined(_WIN32)
    expect(identity_session.list_breakpoints().empty(),
           "Windows debugger should remove the logical breakpoint when casing differs");
#else
    expect(identity_session.list_breakpoints().size() == 1U,
           "POSIX debugger should remove only the exact-case breakpoint path");
#endif

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main() {
    test_breakpoint_path_identity_uses_platform_semantics();
    return test_failures() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
