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

void test_typed_append_from_reads_verified_json_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_typed_append_json";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path destination_path = root / "destination.dbf";
    const fs::path source_path = root / "source.json";
    write_simple_dbf(destination_path, {"Existing"});
    write_text(source_path, "[{\"NAME\":\"Ada\"},{\"NAME\":\"Grace\"}]\n");
    const std::string verified_destination_bytes = read_text(destination_path);
    const std::string verified_source_bytes = read_text(source_path);
    write_text(source_path, "[{\"NAME\":\"Tampered\"}]\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(destination_path.string(), verified_destination_bytes);
    options.verified_file_byte_overrides.emplace(source_path.string(), verified_source_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_typed_append_json.prg",
        "USE '" + destination_path.string() + "' ALIAS destination\n"
        "APPEND FROM '" + source_path.string() + "' TYPE JSON\n"
        "nRows = RECCOUNT('destination')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict typed APPEND FROM JSON should read admitted source bytes: " + state.message);
    expect(global_text(state, "nrows") == "3",
           "strict typed APPEND FROM JSON should append both admitted rows");
    const auto json_persisted = copperfin::vfp::parse_dbf_table_from_file(destination_path.string(), 10U);
    expect(json_persisted.ok && json_persisted.table.records.size() == 3U,
           "strict typed APPEND FROM JSON should persist both admitted rows");
    if (json_persisted.ok && json_persisted.table.records.size() == 3U)
    {
        expect(json_persisted.table.records[1].values.front().display_value == "Ada" &&
                   json_persisted.table.records[2].values.front().display_value == "Grace",
               "strict typed APPEND FROM JSON should ignore the replaced physical source");
    }
    fs::remove_all(root, ignored);
}

void test_typed_append_from_reads_verified_delimited_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_typed_append_csv";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path destination_path = root / "destination.dbf";
    const fs::path source_path = root / "source.csv";
    write_simple_dbf(destination_path, {"Existing"});
    write_text(source_path, "NAME\nAda\nGrace\n");
    const std::string verified_destination_bytes = read_text(destination_path);
    const std::string verified_source_bytes = read_text(source_path);
    write_text(source_path, "NAME\nTampered\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(destination_path.string(), verified_destination_bytes);
    options.verified_file_byte_overrides.emplace(source_path.string(), verified_source_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_typed_append_csv.prg",
        "USE '" + destination_path.string() + "' ALIAS destination\n"
        "APPEND FROM '" + source_path.string() + "' TYPE CSV\n"
        "nRows = RECCOUNT('destination')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict typed APPEND FROM CSV should read admitted source bytes: " + state.message);
    expect(global_text(state, "nrows") == "3",
           "strict typed APPEND FROM CSV should append both admitted rows");
    const auto csv_persisted = copperfin::vfp::parse_dbf_table_from_file(destination_path.string(), 10U);
    expect(csv_persisted.ok && csv_persisted.table.records.size() == 3U,
           "strict typed APPEND FROM CSV should persist both admitted rows");
    if (csv_persisted.ok && csv_persisted.table.records.size() == 3U)
    {
        expect(csv_persisted.table.records[1].values.front().display_value == "Ada" &&
                   csv_persisted.table.records[2].values.front().display_value == "Grace",
               "strict typed APPEND FROM CSV should ignore the replaced physical source");
    }
    fs::remove_all(root, ignored);
}

void test_typed_append_from_fails_closed_without_verified_source_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_unverified_typed_append";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path destination_path = root / "destination.dbf";
    const fs::path source_path = root / "source.json";
    write_simple_dbf(destination_path, {"Existing"});
    write_text(source_path, "[{\"NAME\":\"Ada\"}]\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(destination_path.string(), read_text(destination_path));
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "unverified_typed_append.prg",
        "USE '" + destination_path.string() + "' ALIAS destination\n"
        "APPEND FROM '" + source_path.string() + "' TYPE JSON\n"
        "RETURN\n",
        options);

    expect(!state.completed,
           "strict typed APPEND FROM should fail closed without admitted source bytes");
    expect(state.message.find("APPEND FROM TYPE JSON") != std::string::npos,
           "strict typed APPEND FROM rejection should preserve the localized type diagnostic: " + state.message);
    fs::remove_all(root, ignored);
}

void test_restore_from_reads_verified_mem_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_restore_mem";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path source_path = root / "state.mem";
    const std::string verified_bytes = "saved_value=C:verified\n";
    write_text(source_path, verified_bytes);
    write_text(source_path, "saved_value=C:tampered\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(source_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_restore_mem.prg",
        "RESTORE FROM '" + source_path.string() + "'\n"
        "restored_value = saved_value\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict RESTORE FROM should read admitted .mem bytes: " + state.message);
    expect(global_text(state, "restored_value") == "verified",
           "strict RESTORE FROM should ignore the replaced physical .mem file");
    fs::remove_all(root, ignored);
}

void test_restore_from_fails_closed_without_verified_mem_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_unverified_restore_mem";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path source_path = root / "state.mem";
    write_text(source_path, "saved_value=C:physical\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "unverified_restore_mem.prg",
        "RESTORE FROM '" + source_path.string() + "'\n"
        "RETURN\n",
        options);

    expect(!state.completed,
           "strict RESTORE FROM should fail closed without admitted .mem bytes");
    expect(state.message == "RESTORE FROM: unable to open source file",
           "strict RESTORE FROM rejection should preserve the localized open diagnostic: " + state.message);
    fs::remove_all(root, ignored);
}

void test_buffered_append_blank_reads_verified_dbf_rows()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_buffered_append";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    write_simple_dbf(table_path, {"Ada", "Grace"});
    const std::string verified_bytes = read_text(table_path);
    write_simple_dbf(table_path, {"Tampered"});

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_buffered_append.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "lSet = CURSORSETPROP('Buffering', 5, 'customers')\n"
        "APPEND BLANK\n"
        "nRows = RECCOUNT('customers')\n"
        "nRecord = RECNO('customers')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict buffered APPEND BLANK should complete from the verified DBF snapshot: " + state.message);
    expect(global_text(state, "lset") == "true",
           "strict buffered APPEND BLANK should enable table buffering");
    expect(global_text(state, "nrows") == "3",
           "strict buffered APPEND BLANK should count the verified persisted rows plus the pending row");
    expect(global_text(state, "nrecord") == "3",
           "strict buffered APPEND BLANK should assign the pending row after the verified rows");
    fs::remove_all(root, ignored);
}

void test_append_from_array_reads_verified_destination_schema()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_append_from_array";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const auto verified_created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 12U}, {.name = "AGE", .type = 'N', .length = 3U}},
        {{"Existing", "7"}});
    expect(verified_created.ok, "verified APPEND FROM ARRAY destination fixture should be created");
    const std::string verified_bytes = read_text(table_path);
    const auto tampered_created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "AGE", .type = 'N', .length = 3U}, {.name = "NAME", .type = 'C', .length = 12U}},
        {{"99", "Tampered"}});
    expect(tampered_created.ok, "tampered APPEND FROM ARRAY destination fixture should remain valid");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_append_from_array.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "DIMENSION aRows[1,2]\n"
        "aRows[1,1] = 'Grace'\n"
        "aRows[1,2] = 42\n"
        "APPEND FROM ARRAY aRows\n"
        "nRows = RECCOUNT('customers')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict APPEND FROM ARRAY should use the verified destination schema: " + state.message);
    expect(global_text(state, "nrows") == "2",
           "strict APPEND FROM ARRAY should append one row to the verified destination count");
    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(persisted.ok && persisted.table.records.size() == 2U,
           "strict APPEND FROM ARRAY should persist the appended row");
    if (persisted.ok && persisted.table.records.size() == 2U && persisted.table.records[1].values.size() == 2U)
    {
        expect(persisted.table.records[1].values[0].display_value == "42" &&
                   persisted.table.records[1].values[1].display_value == "Grace",
               "strict APPEND FROM ARRAY should map values using the verified field order");
    }
    fs::remove_all(root, ignored);
}

void test_list_query_file_requery_reads_verified_bytes()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_query_file_requery";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    write_simple_dbf(table_path, {"Ada", "Grace"});
    const std::string verified_table_bytes = read_text(table_path);
    const fs::path query_path = root / "names.qpr";
    const std::string verified_query_text =
        "SELECT name FROM customers WHERE name = 'Ada' INTO CURSOR temp2\n";
    write_text(query_path, verified_query_text);

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_table_bytes);
    options.verified_file_byte_overrides.emplace(query_path.string(), verified_query_text);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_query_file.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "oList = CREATEOBJECT('ListBox')\n"
        "oList.RowSourceType = 4\n"
        "oList.RowSource = 'names.qpr'\n"
        "oList.Requery()\n"
        "cBefore = oList.List(1)\n"
        "nTampered = STRTOFILE(\"SELECT name FROM customers WHERE name = 'Grace' INTO CURSOR temp2\", 'names.qpr')\n"
        "oList.Requery()\n"
        "cAfter = oList.List(1)\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict query-file RowSource should complete from the verified query snapshot: " + state.message);
    const std::string before = global_text(state, "cbefore");
    const std::string tampered = global_text(state, "ntampered");
    const std::string after = global_text(state, "cafter");
    expect(before == "Ada",
           "strict query-file RowSource should initially use the admitted query bytes, got " + before);
    expect(tampered != "0",
           "query-file security fixture should replace the physical query after initial load");
    expect(after == "Ada",
           "strict query-file Requery() should ignore a replaced physical query file, got " + after);
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
    test_typed_append_from_reads_verified_json_bytes();
    test_typed_append_from_reads_verified_delimited_bytes();
    test_typed_append_from_fails_closed_without_verified_source_bytes();
    test_restore_from_reads_verified_mem_bytes();
    test_restore_from_fails_closed_without_verified_mem_bytes();
    test_buffered_append_blank_reads_verified_dbf_rows();
    test_append_from_array_reads_verified_destination_schema();
    test_list_query_file_requery_reads_verified_bytes();
    return test_failures() == 0 ? 0 : 1;
}
