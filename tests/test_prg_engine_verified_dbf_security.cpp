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
    const auto tampered = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 24U}},
        {{"Tampered"}});
    expect(tampered.ok, "tampered DBF fixture should remain structurally valid");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "nRows = RECCOUNT('customers')\n"
        "cName = customers.NAME\n"
        "GO TOP\n"
        "cNameAfterGo = customers.NAME\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict verified DBF USE should complete from the immutable snapshot: " + state.message);
    expect(global_text(state, "nrows") == "2",
           "strict verified DBF USE should preserve the verified record count");
    expect(global_text(state, "cname") == "Ada",
           "strict verified DBF field reads should preserve the verified current record");
    expect(global_text(state, "cnameaftergo") == "Ada",
           "strict verified DBF navigation reads should preserve verified bytes after GO TOP");
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

void test_initial_use_reads_verified_index_metadata()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_index";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const fs::path index_path = root / "customers.cdx";
    write_simple_dbf(table_path, {"Ada", "Grace", "Linus"});
    write_synthetic_cdx(index_path, "NAME", "UPPER(NAME)");
    const std::string verified_table_bytes = read_text(table_path);
    const std::string verified_index_bytes = read_text(index_path);
    write_synthetic_cdx(index_path, "TAMPERED", "NAME");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_table_bytes);
    options.verified_file_byte_overrides.emplace(index_path.string(), verified_index_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_index.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "SET ORDER TO TAG NAME\n"
        "cOrder = ORDER('customers')\n"
        "cOrderPath = ORDER('customers', 1)\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict verified DBF USE should inspect verified index metadata: " + state.message);
    expect(global_text(state, "corder") == "NAME",
           "strict verified DBF USE should expose the verified tag name");
    expect(global_text(state, "corderpath").find("CUSTOMERS.CDX") != std::string::npos,
           "strict verified DBF ORDER path should preserve the logical index identity");
    fs::remove_all(root, ignored);
}

void test_runtime_surface_reads_verified_code_page()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_code_page";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 24U}},
        {{"Ada"}, {"Grace"}});
    expect(created.ok, "verified code-page DBF fixture should be created");
    std::string verified_bytes = read_text(table_path);
    verified_bytes[29U] = static_cast<char>(0x03U);
    write_simple_dbf(table_path, {"Tampered"});

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_code_page.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "nCodePage = CPDBF('customers')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict verified CPDBF should inspect the verified DBF header: " + state.message);
    expect(global_text(state, "ncodepage") == "1252",
           "strict verified CPDBF should preserve the verified code-page mark");
    fs::remove_all(root, ignored);
}

void test_append_from_reads_verified_source_rows()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_append_from";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path destination_path = root / "destination.dbf";
    const fs::path source_path = root / "source.dbf";
    write_simple_dbf(destination_path, {"Existing"});
    write_simple_dbf(source_path, {"Ada", "Grace"});
    const std::string verified_destination_bytes = read_text(destination_path);
    const std::string verified_source_bytes = read_text(source_path);
    write_simple_dbf(source_path, {"Tampered"});

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(destination_path.string(), verified_destination_bytes);
    options.verified_file_byte_overrides.emplace(source_path.string(), verified_source_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_append_from.prg",
        "USE '" + destination_path.string() + "' ALIAS destination\n"
        "APPEND FROM '" + source_path.string() + "'\n"
        "nRows = RECCOUNT('destination')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict verified APPEND FROM should read the verified source table: " + state.message);
    const std::string rows = global_text(state, "nrows");
    expect(rows == "3",
           "strict verified APPEND FROM should append every verified source row, got " + rows);
    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(destination_path.string(), 10U);
    expect(persisted.ok && persisted.table.records.size() == 3U,
           "strict verified APPEND FROM should persist the expected destination row count");
    if (persisted.ok && persisted.table.records.size() == 3U &&
        !persisted.table.records[1].values.empty() &&
        !persisted.table.records[2].values.empty())
    {
        expect(persisted.table.records[1].values.front().display_value == "Ada" &&
                   persisted.table.records[2].values.front().display_value == "Grace",
               "strict verified APPEND FROM should persist verified source rows instead of the replacement");
    }
    fs::remove_all(root, ignored);
}

}  // namespace

int main()
{
    test_initial_use_reads_verified_dbf_bytes();
    test_initial_use_fails_closed_without_verified_dbf_bytes();
    test_initial_use_reads_verified_index_metadata();
    test_runtime_surface_reads_verified_code_page();
    test_append_from_reads_verified_source_rows();
    return test_failures() == 0 ? 0 : 1;
}
