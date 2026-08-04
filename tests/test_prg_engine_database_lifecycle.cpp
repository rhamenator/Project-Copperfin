// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/runtime/prg_engine.h"
#include "copperfin/vfp/dbf_table.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace {

using namespace copperfin::test_support;

struct DatabaseFixture {
    std::filesystem::path dbc;
    std::filesystem::path dct;
    std::filesystem::path dcx;
};

void write_synthetic_database_index(const std::filesystem::path& path) {
    std::vector<std::uint8_t> bytes(16U * 512U, 0U);
    bytes[0] = 0x00U;
    bytes[1] = 0x04U;
    bytes[12] = 0x0AU;
    bytes[14] = 0xE0U;
    bytes[15] = 0x01U;
    bytes[1024U] = 0x03U;
    write_le_u16(bytes, 1026U, 1U);
    write_le_u32(bytes, 1028U, 4U * 512U);
    write_le_u16(bytes, 4U * 512U, 0x0003U);
    write_le_u16(bytes, (4U * 512U) + 2U, 1U);
    const std::string tag_name = "NAME";
    std::copy(tag_name.begin(), tag_name.end(), bytes.begin() + static_cast<std::ptrdiff_t>((3U * 512U) - 10U));
    const std::string expression = "UPPER(NAME)";
    std::copy(expression.begin(), expression.end(), bytes.begin() + static_cast<std::ptrdiff_t>((4U * 512U) + 24U));
    write_text(path, std::string(bytes.begin(), bytes.end()));
}

DatabaseFixture create_database_fixture(
    const std::filesystem::path& directory,
    const std::string& stem,
    bool uppercase_extensions = false) {
    std::filesystem::create_directories(directory);
    DatabaseFixture fixture{
        .dbc = directory / (stem + (uppercase_extensions ? ".DBC" : ".dbc")),
        .dct = directory / (stem + ".dct"),
        .dcx = directory / (stem + (uppercase_extensions ? ".DCX" : ".dcx")),
    };
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .length = 12U},
        {.name = "OBJECTNAME", .type = 'C', .length = 40U},
        {.name = "PARENTNAME", .type = 'C', .length = 40U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U},
        {.name = "CODE", .type = 'M', .length = 4U},
    };
    const auto result = copperfin::vfp::create_dbf_table_file(
        fixture.dbc.string(),
        fields,
        {{"Database", stem, "", "Database fixture", "PUBLIC gDbcStoredCode\ngDbcStoredCode = .T."}});
    expect(result.ok, "DBC fixture should be created");

    write_synthetic_database_index(fixture.dcx);
    return fixture;
}

#if !defined(_WIN32)
bool directory_supports_distinct_case_entries(const std::filesystem::path& directory) {
    namespace fs = std::filesystem;
    const fs::path lowercase_probe = directory / "copperfin_case_probe";
    const fs::path uppercase_probe = directory / "COPPERFIN_CASE_PROBE";
    std::error_code ignored;
    fs::remove(lowercase_probe, ignored);
    fs::remove(uppercase_probe, ignored);
    write_text(lowercase_probe, "probe");

    ignored.clear();
    const bool uppercase_exists = fs::exists(uppercase_probe, ignored);
    const bool supports_distinct_entries = !ignored && !uppercase_exists;
    fs::remove(lowercase_probe, ignored);
    fs::remove(uppercase_probe, ignored);
    return supports_distinct_entries;
}
#endif

copperfin::runtime::RuntimePauseState run_program(
    const std::filesystem::path& root,
    const std::string& name,
    const std::string& source,
    copperfin::runtime::RuntimeSessionOptions options = {}) {
    const std::filesystem::path program = root / name;
    write_text(program, source);
    options.startup_path = program.string();
    options.working_directory = root.string();
    if (options.temp_directory.empty()) {
        options.temp_directory = (root / "runtime-temp").string();
    }
    auto session = copperfin::runtime::PrgRuntimeSession::create(options);
    return session.run(copperfin::runtime::DebugResumeAction::continue_run);
}

std::string global_text(
    const copperfin::runtime::RuntimePauseState& state,
    const std::string& name) {
    const auto found = state.globals.find(name);
    expect(found != state.globals.end(), "runtime script should define global " + name);
    return found == state.globals.end()
        ? std::string{}
        : copperfin::runtime::format_value(found->second);
}

void test_open_select_modes_and_functions() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_database_lifecycle_open";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const auto first = create_database_fixture(root, "first");
    const auto second = create_database_fixture(root, "second");

    const auto state = run_program(
        root,
        "open.prg",
        "nOpenTargetCalls = 0\n"
        "cFirst = '" + first.dbc.string() + "'\n"
        "cSecondPath = '" + second.dbc.string() + "'\n"
        "OPEN DATABASE (resolve_database(cFirst)) SHARED NOUPDATE\n"
        "cFirstPath = DBC()\n"
        "lFirstUsed = DBUSED('first')\n"
        "OPEN DATABASE (cSecondPath) EXCLUSIVE\n"
        "cSecondPathFromSet = SET('DATABASE')\n"
        "SET DATABASE TO (cFirst)\n"
        "cSelectedPath = DBC()\n"
        "OPEN DATABASE (cFirst) EXCLUSIVE\n"
        "SET DATABASE TO\n"
        "cClearedPath = DBC()\n"
        "SET DATABASE TO first\n"
        "RETURN\n"
        "FUNCTION resolve_database\n"
        "LPARAMETERS value\n"
        "nOpenTargetCalls = nOpenTargetCalls + 1\n"
        "RETURN value\n"
        "ENDFUNC\n");

    expect(state.completed, "OPEN/SET DATABASE lifecycle script should complete: " + state.message);
    expect(state.databases.size() == 2U, "two open databases should be visible in the runtime snapshot");
    if (state.databases.size() == 2U) {
        expect(!state.databases[0].exclusive, "SHARED database should retain its initial mode");
        expect(state.databases[0].read_only, "NOUPDATE database should be marked read-only");
        expect(state.databases[0].current, "SET DATABASE should select the requested open database");
        expect(state.databases[1].exclusive, "EXCLUSIVE database should retain its mode");
        expect(!state.databases[1].current, "only one database should be current");
    }
    expect(global_text(state, "cfirstpath") == first.dbc.string(),
           "DBC() should return the current database full path");
    expect(global_text(state, "csecondpathfromset") == second.dbc.string(),
           "SET('DATABASE') should return the same stable path contract as DBC()");
    expect(global_text(state, "cselectedpath") == first.dbc.string(),
           "SET DATABASE TO should update DBC()");
    expect(global_text(state, "lfirstused") == "true",
           "DBUSED(name) should find an open database by stem");
    expect(global_text(state, "nopentargetcalls") == "1",
           "OPEN DATABASE should evaluate a UDF-produced target exactly once");
    expect(global_text(state, "cclearedpath").empty(),
           "empty SET DATABASE TO should clear the current database without closing it");
    expect(has_runtime_event(state.events, "runtime.database.open", first.dbc.string()),
           "OPEN DATABASE should emit a stable runtime event");
    expect(!state.globals.contains("gdbcstoredcode"),
           "OPEN DATABASE should never execute code-like DBC memo payloads");
    fs::remove_all(root, ignored);
}

void test_data_sessions_close_and_stack_frugality() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_database_lifecycle_sessions";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const auto first = create_database_fixture(root, "first");
    const auto second = create_database_fixture(root, "second");

    const auto state = run_program(
        root,
        "sessions.prg",
        "cFirst = '" + first.dbc.string() + "'\n"
        "cSecond = '" + second.dbc.string() + "'\n"
        "OPEN DATABASE (cFirst) SHARED\n"
        "OPEN DATABASE (cSecond) SHARED\n"
        "SET DATASESSION TO 2\n"
        "OPEN DATABASE (cFirst) SHARED\n"
        "lSession2 = DBUSED('first')\n"
        "SET DATASESSION TO 1\n"
        "SET DATABASE TO first\n"
        "CLOSE DATABASE\n"
        "lFirstClosed = DBUSED('first')\n"
        "lSecondStillOpen = DBUSED('second')\n"
        "SET DATABASE TO second\n"
        "FOR nLoop = 1 TO 2000\n"
        "  OPEN DATABASE (cSecond) SHARED\n"
        "  lLoopUsed = DBUSED('second')\n"
        "ENDFOR\n"
        "CLOSE DATABASES ALL\n"
        "SET DATASESSION TO 2\n"
        "lSession2Preserved = DBUSED('first')\n"
        "CLOSE ALL\n"
        "lSession2Closed = DBUSED('first')\n"
        "RETURN\n");

    expect(state.completed, "data-session and repeated database lifecycle script should complete");
    expect(state.databases.empty(), "CLOSE DATABASES ALL should clear open database state");
    expect(global_text(state, "lsession2") == "true",
           "DBUSED(name) should inspect the current data session");
    expect(global_text(state, "lfirstclosed") == "false",
           "CLOSE DATABASE should close only the current database");
    expect(global_text(state, "lsecondstillopen") == "true",
           "CLOSE DATABASE should preserve another open database");
    expect(global_text(state, "lloopused") == "true",
           "repeated OPEN DATABASE should remain functional without recursive runtime frames");
    expect(global_text(state, "lsession2preserved") == "true",
           "CLOSE DATABASES ALL should preserve databases open in another data session");
    expect(global_text(state, "lsession2closed") == "false",
           "CLOSE ALL should clear database state across data sessions");
    fs::remove_all(root, ignored);
}

void test_path_resolution_casefold_and_verified_bytes() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_database_lifecycle_paths";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const auto fixture = create_database_fixture(root / "Data", "Catalog", true);

    const auto resolved = run_program(
        root,
        "paths.prg",
        "SET PATH TO 'Data'\n"
        "OPEN DATABASE catalog SHARED\n"
        "cResolved = DBC()\n"
        "RETURN\n");
    expect(resolved.completed, "SET PATH and case-insensitive database resolution should complete");
    expect(global_text(resolved, "cresolved") == fixture.dbc.string(),
           "OPEN DATABASE should resolve omitted extension and actual companion casing");

    copperfin::runtime::RuntimeSessionOptions trusted;
    trusted.verified_file_byte_overrides = {
        {fixture.dbc.string(), read_text(fixture.dbc)},
        {fixture.dct.string(), read_text(fixture.dct)},
        {fixture.dcx.string(), read_text(fixture.dcx)},
    };
    trusted.require_verified_file_byte_overrides = true;
    write_text(fixture.dbc, "tampered after verification");
    const auto immutable = run_program(
        root,
        "immutable.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n",
        trusted);
    expect(immutable.completed,
           "verified DBC bytes should be parsed instead of reopened disk bytes: " + immutable.message);

    const auto verified_components = trusted.verified_file_byte_overrides;
    std::size_t component_index = 0U;
    for (const auto& [component_path, _] : verified_components) {
        trusted.verified_file_byte_overrides = verified_components;
        trusted.verified_file_byte_overrides.erase(component_path);
        const auto missing_verified = run_program(
            root,
            "missing_verified_" + std::to_string(component_index++) + ".prg",
            "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n",
            trusted);
        expect(!missing_verified.completed,
               "strict package execution should reject each unverified DBC component");
        expect(missing_verified.message.find("Verified package bytes are unavailable") != std::string::npos,
               "strict package rejection should identify unavailable verified bytes: " + missing_verified.message);
        expect(missing_verified.message.find(component_path) != std::string::npos,
               "strict package rejection should preserve the missing component path");
    }

#if !defined(_WIN32)
    trusted.verified_file_byte_overrides = verified_components;
    trusted.verified_file_byte_overrides.erase(fixture.dbc.string());
    std::string colliding_key = fixture.dbc.string();
    colliding_key.replace(colliding_key.find("Catalog.DBC"), std::string("Catalog.DBC").size(), "catalog.dbc");
    trusted.verified_file_byte_overrides.emplace(colliding_key, verified_components.at(fixture.dbc.string()));
    const auto case_collision = run_program(
        root,
        "case_collision.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n",
        trusted);
    expect(!case_collision.completed,
           "POSIX strict byte admission should not borrow bytes from a differently cased path");
    expect(case_collision.message.find("Verified package bytes are unavailable") != std::string::npos,
           "POSIX case-collision denial should retain the verified-byte diagnostic");

    write_text(fixture.dbc, verified_components.at(fixture.dbc.string()));
    const bool supports_distinct_case_entries =
        directory_supports_distinct_case_entries(fixture.dbc.parent_path());
    if (supports_distinct_case_entries) {
        const fs::path extension_case_sibling = fixture.dbc.parent_path() / "Catalog.dbc";
        write_text(extension_case_sibling, verified_components.at(fixture.dbc.string()));
        const auto exact_extension_case = run_program(
            root,
            "exact_extension_case.prg",
            "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\n"
            "cExactExtensionCase = DBC()\n"
            "RETURN\n");
        expect(exact_extension_case.completed,
               "an exact DBC filename should win over an extension-case sibling");
        expect(global_text(exact_extension_case, "cexactextensioncase") == fixture.dbc.string(),
               "exact DBC filename precedence should preserve the requested on-disk path");
        fs::remove(extension_case_sibling, ignored);
    }

    const auto non_listed_fixture = create_database_fixture(root / "NoList", "exact_access");
    std::error_code permission_error;
    fs::permissions(
        non_listed_fixture.dbc.parent_path(),
        fs::perms::owner_exec,
        fs::perm_options::replace,
        permission_error);
    expect(!permission_error,
           "exact-path fallback fixture should remove parent-directory listing permission");
    const auto non_listed = run_program(
        root,
        "non_listed_parent.prg",
        "OPEN DATABASE '" + non_listed_fixture.dbc.string() + "' SHARED\nRETURN\n");
    fs::permissions(
        non_listed_fixture.dbc.parent_path(),
        fs::perms::owner_all,
        fs::perm_options::replace,
        permission_error);
    expect(!permission_error,
           "exact-path fallback fixture should restore parent-directory permissions");
    expect(non_listed.completed,
           "an exact readable DBC path should survive unavailable parent enumeration: " +
               non_listed.message);

    if (supports_distinct_case_entries) {
        const auto lowercase_fixture = create_database_fixture(root / "Data", "catalog");
        const auto distinct_case = run_program(
            root,
            "distinct_case.prg",
            "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\n"
            "OPEN DATABASE '" + lowercase_fixture.dbc.string() + "' SHARED\n"
            "RETURN\n");
        expect(distinct_case.completed,
               "case-sensitive POSIX paths should admit distinct database names: " +
                   distinct_case.message);
        expect(distinct_case.databases.size() == 2U,
               "database identity should preserve distinct case-sensitive paths");
    }
#endif
    fs::remove_all(root, ignored);
}

void test_exclusive_conflicts_and_malformed_components() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_database_lifecycle_errors";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const auto fixture = create_database_fixture(root, "catalog");
    const std::string valid_dbc = read_text(fixture.dbc);
    const std::string valid_dct = read_text(fixture.dct);
    const std::string valid_dcx = read_text(fixture.dcx);

    const auto conflict = run_program(
        root,
        "conflict.prg",
        "cDatabase = '" + fixture.dbc.string() + "'\n"
        "OPEN DATABASE (cDatabase) EXCLUSIVE\n"
        "SET DATASESSION TO 2\n"
        "OPEN DATABASE (cDatabase) SHARED\n"
        "RETURN\n");
    expect(!conflict.completed, "an exclusive database should reject another data-session open");
    expect(conflict.message.find("already open in another data session") != std::string::npos,
           "exclusive database conflicts should use the localized diagnostic");

    write_text(fixture.dbc, "malformed database container");
    const auto malformed = run_program(
        root,
        "malformed.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n");
    expect(!malformed.completed, "a malformed DBC should fail before lifecycle admission");
    expect(malformed.message.find("Database container is malformed") != std::string::npos,
           "malformed DBC failures should use the localized diagnostic");

    write_text(fixture.dbc, valid_dbc);
    write_text(fixture.dct, "malformed memo");
    const auto malformed_dct = run_program(
        root,
        "malformed_dct.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n");
    expect(!malformed_dct.completed, "a malformed DCT should fail before lifecycle admission");
    expect(malformed_dct.message.find(fixture.dct.string()) != std::string::npos,
           "malformed DCT failures should preserve the component path");

    write_text(fixture.dct, valid_dct);
    write_text(fixture.dcx, "malformed index");
    const auto malformed_dcx = run_program(
        root,
        "malformed_dcx.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "' SHARED\nRETURN\n");
    expect(!malformed_dcx.completed, "a malformed DCX should fail before lifecycle admission");
    expect(malformed_dcx.message.find(fixture.dcx.string()) != std::string::npos,
           "malformed DCX failures should preserve the component path");
    write_text(fixture.dcx, valid_dcx);
    fs::remove_all(root, ignored);
}

void test_database_errors_are_pseudo_localized() {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "copperfin_database_lifecycle_localized";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const auto fixture = create_database_fixture(root, "missing_index");
    fs::remove(fixture.dcx, ignored);

    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    for (const auto& [locale_name, expected_text] :
         std::vector<std::pair<std::string, std::string>>{
             {"es-419", "Falta el componente de la base de datos"},
             {"pt-BR", "O componente do banco de dados esta ausente"}}) {
        set_env_value("COPPERFIN_LOCALE", locale_name, true);
        const auto localized = run_program(
            root,
            "localized_" + locale_name + ".prg",
            "OPEN DATABASE '" + fixture.dbc.string() + "'\nRETURN\n");
        expect(!localized.completed, locale_name + " missing DBC companion should fail");
        expect(localized.message.find(expected_text) != std::string::npos,
               locale_name + " database failure should use localized prose: " + localized.message);
        expect(localized.message.find(fixture.dcx.string()) != std::string::npos,
               locale_name + " database failure should preserve path placeholders");
    }

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const auto state = run_program(
        root,
        "localized.prg",
        "OPEN DATABASE '" + fixture.dbc.string() + "'\nRETURN\n");
    expect(!state.completed, "missing DBC companion should fail");
    expect(state.message.find("Runtime.Prg.Database.Error") == std::string::npos,
           "database failures should not leak localization keys");
    expect(state.message.starts_with("[!! "),
           "qps-ploc database failures should run through the pseudo-localization transform");
    expect(state.message.find("Database component is missing") == std::string::npos,
           "qps-ploc database failures should not leak raw English prose");
    expect(state.message.find(fixture.dcx.string()) != std::string::npos,
           "localized database failures should preserve path placeholders");
    fs::remove_all(root, ignored);
}

}  // namespace

int main() {
    test_open_select_modes_and_functions();
    test_data_sessions_close_and_stack_frugality();
    test_path_resolution_casefold_and_verified_bytes();
    test_exclusive_conflicts_and_malformed_components();
    test_database_errors_are_pseudo_localized();

    if (test_failures() != 0) {
        std::cerr << test_failures() << " test(s) failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
