// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"

#include <filesystem>
#include <string>

namespace {

using namespace copperfin::test_support;
namespace fs = std::filesystem;

copperfin::runtime::RuntimePauseState run_program(
    const fs::path &root,
    const std::string &name,
    const std::string &source,
    copperfin::runtime::RuntimeSessionOptions options = {})
{
    const fs::path program = root / name;
    write_text(program, source);
    options.startup_path = program.string();
    options.working_directory = root.string();
    options.temp_directory = (root / "runtime-temp").string();
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    return session.run(copperfin::runtime::DebugResumeAction::continue_run);
}

std::string global_text(
    const copperfin::runtime::RuntimePauseState &state,
    const std::string &name)
{
    const auto found = state.globals.find(name);
    expect(found != state.globals.end(), "verified DBF script should define global " + name);
    return found == state.globals.end()
        ? std::string{}
        : copperfin::runtime::format_value(found->second);
}

void test_initial_use_reads_verified_dbf_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_use";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 24U}},
        {{"Ada"}, {"Grace"}});
    expect(created.ok, "verified DBF fixture should be created");
    const std::string verified_bytes = read_text(table_path);
    write_text(table_path, "tampered after package verification");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "nRows = RECCOUNT('customers')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict verified DBF USE should complete from the immutable snapshot: " + state.message);
    expect(global_text(state, "nrows") == "2",
           "strict verified DBF USE should preserve the verified record count");
    fs::remove_all(root, ignored);
}

void test_initial_use_fails_closed_without_verified_dbf_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_unverified_dbf_use";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 24U}},
        {{"Ada"}});
    expect(created.ok, "unverified DBF fixture should be created");

    copperfin::runtime::RuntimeSessionOptions options;
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "unverified.prg",
        "USE '" + table_path.string() + "' ALIAS customers\nRETURN\n",
        options);

    expect(!state.completed,
           "strict verified DBF USE should fail closed without package bytes");
    expect(state.message.find("Verified package bytes are unavailable for database component") != std::string::npos,
           "strict DBF rejection should retain the localized verified-byte diagnostic: " + state.message);
    fs::remove_all(root, ignored);
}

}  // namespace

int main()
{
    test_initial_use_reads_verified_dbf_bytes();
    test_initial_use_fails_closed_without_verified_dbf_bytes();
    return test_failures() == 0 ? 0 : 1;
}
