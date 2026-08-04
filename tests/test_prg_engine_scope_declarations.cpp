// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <iostream>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

void test_public_rejects_non_public_existing_bindings()
{
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scope_declarations";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "public_collision.prg";
    write_text(
        main_path,
        "DO caller_scope\n"
        "RETURN\n"
        "PROCEDURE caller_scope\n"
        "PRIVATE collision_value\n"
        "collision_value = 'private'\n"
        "DO public_scope\n"
        "RETURN\n"
        "PROCEDURE public_scope\n"
        "PUBLIC collision_value\n"
        "RETURN\n");

    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "PUBLIC collision should fail instead of silently changing scope");
    expect(state.message == "Illegal redefinition of variable \"collision_value\"",
           "PUBLIC collision should use the localized Error 1960 contract");
    fs::remove_all(temp_root, ignored);
}

void test_public_rejects_existing_implicit_global_binding()
{
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_prg_engine_scope_global";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path main_path = temp_root / "public_existing.prg";
    write_text(main_path, "existing_value = 'assigned'\nPUBLIC existing_value\nRETURN\n");
    auto session = copperfin::runtime::PrgRuntimeSession::create(
        make_runtime_session_options(main_path.string(), temp_root.string()));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    expect(!state.completed, "PUBLIC after assignment should fail under VFP Error 1960 semantics");
    expect(state.message == "Illegal redefinition of variable \"existing_value\"",
           "implicit global PUBLIC collision should preserve the diagnostic identity");
    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main()
{
    test_public_rejects_non_public_existing_bindings();
    test_public_rejects_existing_implicit_global_binding();
    if (test_failures() != 0)
    {
        std::cerr << test_failures() << " test(s) failed.\n";
        return 1;
    }
    return 0;
}
