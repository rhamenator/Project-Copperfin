// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "test_environment_support.h"
#include "prg_engine_test_support.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <string>
#include <thread>

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
    options.startup_path = copperfin::platform::path_to_utf8_string(program);
    options.working_directory = copperfin::platform::path_to_utf8_string(root);
    options.temp_directory = copperfin::platform::path_to_utf8_string(root / "runtime-temp");
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

void test_filetostr_reads_verified_bytes_and_fails_closed_without_admission()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_filetostr";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path payload_path = root / "payload.txt";
    write_text(payload_path, "TAMPERED\nPHYSICAL");

    copperfin::runtime::RuntimeSessionOptions verified_options;
    verified_options.verified_file_byte_overrides.emplace(payload_path.string(), "VERIFIED\nSECOND");
    verified_options.require_verified_file_byte_overrides = true;
    const auto verified_state = run_program(
        root,
        "verified_filetostr.prg",
        "hRead = FOPEN('payload.txt', 0)\n"
        "cChunk = FREAD(hRead, 8)\n"
        "nTell = FTELL(hRead)\n"
        "nSeekStart = FSEEK(hRead, 0, 0)\n"
        "cLine = FGETS(hRead, 64)\n"
        "nSeekTail = FSEEK(hRead, -6, 2)\n"
        "cTail = FREAD(hRead, 6)\n"
        "cAfterEof = FREAD(hRead, 1)\n"
        "lEof = FEOF(hRead)\n"
        "nClose = FCLOSE(hRead)\n"
        "cValue = FILETOSTR('payload.txt')\nRETURN\n",
        verified_options);

    expect(verified_state.completed,
           "strict FILETOSTR should complete from admitted bytes: " + verified_state.message);
    expect(global_text(verified_state, "cvalue") == "VERIFIED\nSECOND",
           "strict FILETOSTR should ignore tampered disk bytes");
    expect(global_text(verified_state, "cchunk") == "VERIFIED",
           "strict FREAD should ignore tampered disk bytes");
    expect(global_text(verified_state, "ntell") == "8",
           "strict FTELL should track admitted-byte position");
    expect(global_text(verified_state, "cline") == "VERIFIED",
           "strict FGETS should read admitted lines");
    expect(global_text(verified_state, "ctail") == "SECOND",
           "strict FSEEK and FREAD should use admitted bytes");
    expect(global_text(verified_state, "caftereof").empty(),
           "strict FREAD should return an empty value after admitted bytes are exhausted");
    expect(global_text(verified_state, "leof") == "true",
           "strict FEOF should report the admitted-byte end");
    expect(global_text(verified_state, "nclose") == "0",
           "strict verified read handles should close successfully");

    copperfin::runtime::RuntimeSessionOptions missing_options;
    missing_options.require_verified_file_byte_overrides = true;
    const auto missing_state = run_program(
        root,
        "missing_filetostr.prg",
        "hRead = FOPEN('payload.txt', 0)\n"
        "nError = FERROR()\n"
        "cChunk = FREAD(hRead, 3)\n"
        "cValue = FILETOSTR('payload.txt')\nRETURN\n",
        missing_options);

    expect(missing_state.completed,
           "strict FILETOSTR without admission should fail safely: " + missing_state.message);
    expect(global_text(missing_state, "cvalue").empty(),
           "strict FILETOSTR without admission should return no file bytes");
    expect(global_text(missing_state, "hread") == "-1",
           "strict read-only FOPEN without admission should fail closed");
    expect(global_text(missing_state, "nerror") == "5",
           "strict read-only FOPEN should expose the verified-byte access error");
    expect(std::any_of(missing_state.events.begin(), missing_state.events.end(), [](const auto& event)
    {
        return event.category == "runtime.warning" &&
               event.detail.find("Verified package bytes are unavailable for FILETOSTR() input") != std::string::npos;
    }),
           "strict FILETOSTR rejection should emit the verified-byte warning");

    const auto non_strict_state = run_program(
        root,
        "non_strict_filetostr.prg",
        "hRead = FOPEN('payload.txt', 0)\n"
        "cChunk = FREAD(hRead, 3)\n"
        "nClose = FCLOSE(hRead)\n"
        "cValue = FILETOSTR('payload.txt')\nRETURN\n");
    expect(non_strict_state.completed,
           "non-strict FILETOSTR should preserve ordinary disk reads: " + non_strict_state.message);
    expect(global_text(non_strict_state, "cvalue") == "TAMPERED\nPHYSICAL",
           "non-strict FILETOSTR should retain ordinary filesystem behavior");
    expect(global_text(non_strict_state, "cchunk") == "TAM",
           "non-strict FREAD should retain ordinary filesystem behavior");
    fs::remove_all(root, ignored);
}

void test_strict_fopen_uses_admitted_bytes_during_replacement()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_fopen_replacement";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path payload_path = root / "payload.txt";
    write_text(payload_path, "DISK-PAYLOAD");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(payload_path.string(), "ADMITTED-PAYLOAD");
    options.require_verified_file_byte_overrides = true;

    std::atomic<bool> stop_writer{false};
    std::atomic<unsigned int> writes{0U};
    std::thread replacement_writer([&]() {
        while (!stop_writer.load(std::memory_order_relaxed))
        {
            write_text(payload_path, "REPLACED-PAYLOAD");
            writes.fetch_add(1U, std::memory_order_relaxed);
            std::this_thread::yield();
        }
    });
    for (unsigned int attempt = 0U;
         attempt < 10000U && writes.load(std::memory_order_relaxed) == 0U;
         ++attempt)
    {
        std::this_thread::yield();
    }

    const auto state = run_program(
        root,
        "verified_fopen_replacement.prg",
        "hRead = FOPEN('payload.txt', 0)\n"
        "cValue = FREAD(hRead, 64)\n"
        "nClose = FCLOSE(hRead)\n"
        "RETURN\n",
        options);
    stop_writer.store(true, std::memory_order_relaxed);
    replacement_writer.join();

    expect(state.completed,
           "strict FOPEN should complete from admitted bytes during replacement: " + state.message);
    expect(global_text(state, "cvalue") == "ADMITTED-PAYLOAD",
           "strict FOPEN should not observe a concurrently replaced physical payload");
    expect(global_text(state, "nclose") == "0",
           "strict admitted FOPEN handles should close successfully");
    expect(writes.load(std::memory_order_relaxed) > 0U,
           "strict FOPEN replacement coverage should perform a physical pathname replacement");

    copperfin::runtime::RuntimeSessionOptions missing_options;
    missing_options.require_verified_file_byte_overrides = true;
    const auto missing_state = run_program(
        root,
        "missing_fopen_replacement.prg",
        "hRead = FOPEN('payload.txt', 0)\n"
        "nError = FERROR()\n"
        "RETURN\n",
        missing_options);
    expect(missing_state.completed,
           "strict FOPEN without admission should fail safely: " + missing_state.message);
    expect(global_text(missing_state, "hread") == "-1",
           "strict FOPEN without admission should fail closed");
    expect(global_text(missing_state, "nerror") == "5",
           "strict FOPEN without admission should preserve the access error contract");

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

void test_database_index_admission_preserves_platform_case_rules()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_index_case";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const fs::path index_path = root / "customers.cdx";
    const fs::path differently_cased_index_path = root / "CUSTOMERS.CDX";
    write_simple_dbf(table_path, {"Ada", "Grace"});
    write_synthetic_cdx(index_path, "NAME", "UPPER(NAME)");
    const std::string verified_table_bytes = read_text(table_path);
    const std::string verified_index_bytes = read_text(index_path);
    fs::remove(index_path, ignored);

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_table_bytes);
    options.verified_file_byte_overrides.emplace(differently_cased_index_path.string(), verified_index_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_index_case.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "cTag = TAG('customers.cdx', 1, 'customers')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict DBF index case-rule fixture should complete: " + state.message);
#if defined(_WIN32)
    expect(global_text(state, "ctag") == "NAME",
           "Windows strict DBF admission should retain VFP case-insensitive index lookup");
#else
    expect(global_text(state, "ctag").empty(),
           "POSIX strict DBF admission should reject a differently-cased index override");
#endif
    fs::remove_all(root, ignored);
}

void test_database_memo_admission_preserves_platform_case_rules()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_dbf_memo_case";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "notes.dbf";
    const fs::path memo_path = root / "notes.fpt";
    const fs::path differently_cased_memo_path = root / "NOTES.FPT";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NOTE", .type = 'M', .length = 10U}},
        {{"Verified memo"}});
    expect(created.ok, "verified DBF memo fixture should be created");
    const std::string verified_table_bytes = read_text(table_path);
    const std::string verified_memo_bytes = read_text(memo_path);
    fs::remove(memo_path, ignored);

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_table_bytes);
    options.verified_file_byte_overrides.emplace(differently_cased_memo_path.string(), verified_memo_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_memo_case.prg",
        "USE '" + table_path.string() + "' ALIAS notes\n"
        "cNote = notes.NOTE\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict DBF memo case-rule fixture should complete: " + state.message);
#if defined(_WIN32)
    expect(global_text(state, "cnote") == "Verified memo",
           "Windows strict DBF admission should retain VFP case-insensitive memo lookup");
#else
    expect(global_text(state, "cnote") != "Verified memo",
           "POSIX strict DBF admission should reject a differently-cased memo override");
#endif
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

void test_typed_append_from_reads_verified_destination_schema()
{
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_typed_append_destination_schema";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path destination_path = root / "destination.dbf";
    const fs::path source_path = root / "source.csv";
    const auto verified_created = copperfin::vfp::create_dbf_table_file(
        destination_path.string(),
        {{.name = "NAME", .type = 'C', .length = 12U}, {.name = "AGE", .type = 'N', .length = 3U}},
        {{"Existing", "7"}});
    expect(verified_created.ok, "verified typed APPEND FROM destination fixture should be created");
    const std::string verified_destination_bytes = read_text(destination_path);
    const auto tampered_created = copperfin::vfp::create_dbf_table_file(
        destination_path.string(),
        {{.name = "AGE", .type = 'N', .length = 3U}, {.name = "NAME", .type = 'C', .length = 12U}},
        {{"99", "Tampered"}});
    expect(tampered_created.ok, "tampered typed APPEND FROM destination fixture should remain valid");
    write_text(source_path, "NAME,AGE\nAda,42\n");
    const std::string verified_source_bytes = read_text(source_path);
    write_text(source_path, "NAME,AGE\nTampered,0\n");

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(destination_path.string(), verified_destination_bytes);
    options.verified_file_byte_overrides.emplace(source_path.string(), verified_source_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_typed_append_destination_schema.prg",
        "USE '" + destination_path.string() + "' ALIAS destination\n"
        "APPEND FROM '" + source_path.string() + "' TYPE CSV\n"
        "nRows = RECCOUNT('destination')\n"
        "RETURN\n",
        options);

    expect(state.completed,
           "strict typed APPEND FROM should use the admitted destination schema: " + state.message);
    expect(global_text(state, "nrows") == "2",
           "strict typed APPEND FROM should skip the admitted CSV header exactly once");
    const auto persisted = copperfin::vfp::parse_dbf_table_from_file(destination_path.string(), 10U);
    expect(persisted.ok && persisted.table.records.size() == 2U,
           "strict typed APPEND FROM should persist one row using the admitted destination schema");
    if (persisted.ok && persisted.table.records.size() == 2U && persisted.table.records[1].values.size() == 2U)
    {
        expect(persisted.table.records[1].values[0].display_value == "42" &&
                   persisted.table.records[1].values[1].display_value == "Ada",
               "strict typed APPEND FROM should map CSV columns through admitted field names");
    }
    fs::remove_all(root, ignored);
}

void test_verified_alter_table_fails_closed_without_mutation()
{
    ScopedEnvironmentValue scoped_locale("COPPERFIN_LOCALE", "qps-ploc");
    const fs::path root = fs::temp_directory_path() / "copperfin_verified_alter_table_mutation";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path table_path = root / "customers.dbf";
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(),
        {{.name = "NAME", .type = 'C', .length = 12U}},
        {{"Ada"}});
    expect(created.ok, "verified ALTER TABLE fixture should be created");
    const std::string verified_bytes = read_text(table_path);

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_path.string(), verified_bytes);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_alter_table.prg",
        "USE '" + table_path.string() + "' ALIAS customers\n"
        "ALTER TABLE '" + table_path.string() + "' ADD COLUMN STATUS C(8)\n"
        "RETURN\n",
        options);

    expect(!state.completed, "strict ALTER TABLE should fail closed for an admitted DBF");
    expect(state.message.find("[!! ") == 0U &&
               state.message.find("ALTER TABLE") != std::string::npos &&
               state.message.find("verified package DBFs") == std::string::npos,
           "strict ALTER TABLE rejection should preserve the localized mutation contract: " + state.message);
    const auto unchanged = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 10U);
    expect(unchanged.ok && unchanged.table.fields.size() == 1U && unchanged.table.records.size() == 1U,
           "strict ALTER TABLE rejection should leave the physical DBF unchanged");
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

void test_verified_prg_data_readers_preserve_unicode_paths()
{
    const fs::path root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string("copperfin_verified_prg_data_\xE2\x98\x83");
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);

    const fs::path restore_path = root / "state.mem";
    const std::string verified_restore_bytes = "saved_value=C:verified\n";
    write_text(restore_path, verified_restore_bytes);
    write_text(restore_path, "saved_value=C:tampered\n");

    copperfin::runtime::RuntimeSessionOptions restore_options;
    restore_options.verified_file_byte_overrides.emplace(
        copperfin::platform::path_to_utf8_string(restore_path),
        verified_restore_bytes);
    restore_options.require_verified_file_byte_overrides = true;
    const std::string restore_utf8_path = copperfin::platform::path_to_utf8_string(restore_path);
    const auto restore_state = run_program(
        root,
        "unicode_verified_restore.prg",
        "RESTORE FROM '" + restore_utf8_path + "'\n"
        "restored_value = saved_value\n"
        "RETURN\n",
        restore_options);

    expect(restore_state.completed,
           "#4250: strict RESTORE FROM should resolve a Unicode admitted path: " + restore_state.message);
    expect(global_text(restore_state, "restored_value") == "verified",
           "#4250: strict RESTORE FROM should consume bytes admitted under a Unicode path");

    const fs::path destination_path = root / "destination.dbf";
    const fs::path append_source_path = root / "source.json";
    write_simple_dbf(destination_path, {"Existing"});
    write_text(append_source_path, "[{\"NAME\":\"Ada\"},{\"NAME\":\"Grace\"}]\n");
    const std::string verified_destination_bytes = read_text(destination_path);
    const std::string verified_append_bytes = read_text(append_source_path);
    write_text(append_source_path, "[{\"NAME\":\"Tampered\"}]\n");

    copperfin::runtime::RuntimeSessionOptions append_options;
    append_options.verified_file_byte_overrides.emplace(
        copperfin::platform::path_to_utf8_string(destination_path),
        verified_destination_bytes);
    append_options.verified_file_byte_overrides.emplace(
        copperfin::platform::path_to_utf8_string(append_source_path),
        verified_append_bytes);
    append_options.require_verified_file_byte_overrides = true;
    const std::string append_destination_utf8 = copperfin::platform::path_to_utf8_string(destination_path);
    const std::string append_source_utf8 = copperfin::platform::path_to_utf8_string(append_source_path);
    const auto append_state = run_program(
        root,
        "unicode_verified_append.prg",
        "USE '" + append_destination_utf8 + "' ALIAS destination\n"
        "APPEND FROM '" + append_source_utf8 + "' TYPE JSON\n"
        "nRows = RECCOUNT('destination')\n"
        "RETURN\n",
        append_options);

    expect(append_state.completed,
           "#4250: strict typed APPEND FROM should resolve a Unicode admitted path: " + append_state.message);
    expect(global_text(append_state, "nrows") == "3",
           "#4250: strict typed APPEND FROM should consume admitted bytes under a Unicode path");
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
    const std::string table_utf8_path = copperfin::platform::path_to_utf8_string(table_path);
    const std::string query_utf8_path = copperfin::platform::path_to_utf8_string(query_path);
    const std::string verified_query_text =
        "SELECT name FROM customers WHERE name = 'Ada' INTO CURSOR temp2\n";
    write_text(query_path, verified_query_text);

    copperfin::runtime::RuntimeSessionOptions options;
    options.verified_file_byte_overrides.emplace(table_utf8_path, verified_table_bytes);
    options.verified_file_byte_overrides.emplace(query_utf8_path, verified_query_text);
    options.require_verified_file_byte_overrides = true;
    const auto state = run_program(
        root,
        "verified_query_file.prg",
        "USE '" + table_utf8_path + "' ALIAS customers\n"
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
    test_filetostr_reads_verified_bytes_and_fails_closed_without_admission();
    test_strict_fopen_uses_admitted_bytes_during_replacement();
    test_initial_use_reads_verified_index_metadata();
    test_database_index_admission_preserves_platform_case_rules();
    test_database_memo_admission_preserves_platform_case_rules();
    test_runtime_surface_reads_verified_code_page();
    test_append_from_reads_verified_source_rows();
    test_typed_append_from_reads_verified_json_bytes();
    test_typed_append_from_reads_verified_delimited_bytes();
    test_typed_append_from_reads_verified_destination_schema();
    test_verified_alter_table_fails_closed_without_mutation();
    test_typed_append_from_fails_closed_without_verified_source_bytes();
    test_restore_from_reads_verified_mem_bytes();
    test_restore_from_fails_closed_without_verified_mem_bytes();
    test_verified_prg_data_readers_preserve_unicode_paths();
    test_buffered_append_blank_reads_verified_dbf_rows();
    test_append_from_array_reads_verified_destination_schema();
    test_list_query_file_requery_reads_verified_bytes();
    return test_failures() == 0 ? 0 : 1;
}
