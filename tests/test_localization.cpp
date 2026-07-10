// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "prg_engine_test_support.h"
#include "test_environment_support.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#if defined(_WIN32)
#include <cstdio>
#else
#include <cstdio>
#endif

namespace {

using namespace copperfin::test_support;

void write_catalog(const std::filesystem::path& root, const std::string& locale, const std::string& json) {
    const std::filesystem::path locale_root = root / locale;
    std::filesystem::create_directories(locale_root);
    write_text(locale_root / "strings.json", json);
}

std::string shell_quote(const std::string& value) {
#ifdef _WIN32
    return "\"" + value + "\"";
#else
    std::string quoted = "'";
    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted += "'";
    return quoted;
#endif
}

std::string run_command_capture(const std::string& command) {
    std::string output;
#ifdef _WIN32
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) {
        return output;
    }
    char buffer[256];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output;
}

void seed_test_catalogs(const std::filesystem::path& root) {
    write_catalog(
        root,
        "en-US",
        "{\n"
        "  \"Command.Inspect\": \"Inspect\",\n"
        "  \"Diagnostic.ExpectedTokenBeforeToken\": \"Expected {expectedToken} before {actualToken}.\",\n"
        "  \"Help.LocaleOption\": \"Select the user-interface locale.\",\n"
        "  \"Inspect.Usage\": \"Usage: {commandName} [{localeOption} {localeValue}] {assetPathArgument}\",\n"
        "  \"Test.Unicode\": \"Cafe \xC3\xA9 \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 {name}\"\n"
        "}\n");
    write_catalog(root, "es-419", "{}\n");
    write_catalog(root, "pt-BR", "{}\n");
    write_catalog(root, "qps-ploc", "{}\n");
}

void test_catalog_loading_and_fallback() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    seed_test_catalogs(temp_root);

    const auto english = copperfin::localization::load_catalogs(temp_root, "en-US");
    expect(
        english.translate("Command.Inspect") == "Inspect",
        "#1779: en-US catalog loading should return source strings");

    const auto spanish_mexico = copperfin::localization::load_catalogs(temp_root, "es-MX");
    expect(
        spanish_mexico.fallback_chain.size() >= 3U &&
            spanish_mexico.fallback_chain[0] == "es-MX" &&
            spanish_mexico.fallback_chain[1] == "es-419" &&
            spanish_mexico.translate("Help.LocaleOption") == "Select the user-interface locale.",
        "#1779: es-MX should deterministically fall back through es-419 to en-US");

    const auto portuguese_portugal = copperfin::localization::load_catalogs(temp_root, "pt-PT");
    expect(
        portuguese_portugal.fallback_chain.size() >= 3U &&
            portuguese_portugal.fallback_chain[0] == "pt-PT" &&
            portuguese_portugal.fallback_chain[1] == "pt" &&
            portuguese_portugal.translate("Command.Inspect") == "Inspect",
        "#1779: pt-PT should deterministically fall back through pt to en-US");

    const auto unknown = copperfin::localization::load_catalogs(temp_root, "zz-ZZ");
    expect(
        unknown.translate("Command.Inspect") == "Inspect",
        "#1779: unknown locales should fall back to en-US");

    fs::remove_all(temp_root, ignored);
}

void test_placeholders_pseudo_locale_and_unicode() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_format_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    seed_test_catalogs(temp_root);

    const auto english = copperfin::localization::load_catalogs(temp_root, "en-US");
    expect(
        english.translate(
            "Diagnostic.ExpectedTokenBeforeToken",
            {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}}) == "Expected ENDSCAN before ENDIF.",
        "#1779: named placeholders should format diagnostic prose without changing VFP tokens");

    const auto pseudo = copperfin::localization::load_catalogs(temp_root, "qps-ploc");
    const std::string pseudo_message = pseudo.translate(
        "Diagnostic.ExpectedTokenBeforeToken",
        {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}});
    expect(
        pseudo_message.find("[!! ") == 0U && pseudo_message.find("ENDSCAN") != std::string::npos &&
            pseudo_message.find("ENDIF") != std::string::npos &&
            pseudo_message.find("{expectedToken}") == std::string::npos,
        "#1779: pseudo-localization should decorate prose while preserving placeholder replacement values");

    expect(
        english.translate("Test.Unicode", {{"name", "Zo\xC3\xAB"}}) ==
            "Cafe \xC3\xA9 \xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 Zo\xC3\xAB",
        "#1779: UTF-8 catalog values and placeholders should round trip non-ASCII text");

    fs::remove_all(temp_root, ignored);
}

void test_catalog_json_unicode_escapes_support_surrogate_pairs() {
    const auto parsed = copperfin::localization::parse_catalog_json(
        "{\"emoji\":\"\\uD83D\\uDE03\",\"clef\":\"\\uD834\\uDD1E\"}");
    expect(parsed.ok, "#3213: catalog JSON parser should accept valid surrogate-pair escapes");
    if (parsed.ok) {
        const auto emoji = parsed.entries.find("emoji");
        const auto clef = parsed.entries.find("clef");
        expect(
            emoji != parsed.entries.end() && emoji->second == "\xF0\x9F\x98\x83",
            "#3213: surrogate-pair emoji escapes should decode to four-byte UTF-8");
        expect(
            clef != parsed.entries.end() && clef->second == "\xF0\x9D\x84\x9E",
            "#3213: surrogate-pair music-symbol escapes should decode to four-byte UTF-8");
    }

    const auto lone_high = copperfin::localization::parse_catalog_json("{\"emoji\":\"\\uD83D\"}");
    expect(
        !lone_high.ok && lone_high.error == "Catalog.Json.IncompleteUnicodeEscape",
        "#3213: a trailing high surrogate should surface the existing incomplete-unicode diagnostic");

    const auto lone_low = copperfin::localization::parse_catalog_json("{\"emoji\":\"\\uDE03\"}");
    expect(
        !lone_low.ok && lone_low.error == "Catalog.Json.InvalidUnicodeEscape",
        "#3213: an unpaired low surrogate should be rejected");

    const auto mismatched_pair = copperfin::localization::parse_catalog_json("{\"emoji\":\"\\uD83D\\u0041\"}");
    expect(
        !mismatched_pair.ok && mismatched_pair.error == "Catalog.Json.InvalidUnicodeEscape",
        "#3213: a high surrogate followed by a non-low-surrogate escape should be rejected");
}

void test_machine_contract_fields_remain_invariant() {
    const std::string diagnostic_code = "CFP1007";
    const std::string severity = "error";
    const std::string expected_token = "ENDSCAN";
    const std::string actual_token = "ENDIF";
    const std::string localized_message = copperfin::localization::format_named_placeholders(
        "Expected {expectedToken} before {actualToken}.",
        {{"expectedToken", expected_token}, {"actualToken", actual_token}});

    expect(diagnostic_code == "CFP1007", "#1779: diagnostic codes should stay locale-invariant");
    expect(severity == "error", "#1779: severity values should stay machine-readable");
    expect(expected_token == "ENDSCAN" && actual_token == "ENDIF", "#1779: VFP tokens should stay English");
    expect(
        localized_message == "Expected ENDSCAN before ENDIF.",
        "#1779: only diagnostic prose should be localized");
}

void test_catalog_root_resolution_searches_parent_directories() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_parent_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "resources" / "locales");
    seed_test_catalogs(temp_root / "resources" / "locales");

    const fs::path nested_working_directory = temp_root / ".tmp" / "fixture";
    fs::create_directories(nested_working_directory);

    const fs::path previous_working_directory = fs::current_path();
    fs::current_path(nested_working_directory);
    const fs::path resolved_root = copperfin::localization::resolve_catalog_root();
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");
    fs::current_path(previous_working_directory);

    expect(
        fs::equivalent(resolved_root, temp_root / "resources" / "locales") &&
            catalog.translate("Command.Inspect") == "Inspect",
        "#2361: catalog root resolution should find repo-local resources from nested temp directories");

    fs::remove_all(temp_root, ignored);
}

void test_catalog_root_resolution_finds_repo_build_output_layout_from_executable_path() {
    namespace fs = std::filesystem;
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_executable_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "resources" / "locales");
    seed_test_catalogs(temp_root / "resources" / "locales");

    const fs::path executable_path = temp_root / "build" / "Release" / "copperfin_runtime_host";
    fs::create_directories(executable_path.parent_path());
    write_text(executable_path, "");

    const fs::path resolved_root = copperfin::localization::resolve_catalog_root(executable_path);
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");

    expect(
        fs::equivalent(resolved_root, temp_root / "resources" / "locales") &&
            catalog.translate("Command.Inspect") == "Inspect",
        "#3645: catalog root resolution should find repo build-output resources from the executable path");

    fs::remove_all(temp_root, ignored);
}

void test_catalog_root_resolution_finds_repo_build_output_layout_from_path_launched_basename() {
    namespace fs = std::filesystem;
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    ScopedEnvironmentValue search_path("PATH", false);

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_path_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "resources" / "locales");
    seed_test_catalogs(temp_root / "resources" / "locales");

#if defined(_WIN32)
    const std::string executable_name = "copperfin_build_host.exe";
#else
    const std::string executable_name = "copperfin_build_host";
#endif
    const fs::path executable_path = temp_root / "build" / "Release" / executable_name;
    fs::create_directories(executable_path.parent_path());
    write_text(executable_path, "");

    const std::string original_path = getenv_value("PATH");
#if defined(_WIN32)
    const char path_separator = ';';
#else
    const char path_separator = ':';
#endif
    const std::string seeded_path =
        executable_path.parent_path().string() +
        (original_path.empty() ? std::string() : std::string(1U, path_separator) + original_path);
    search_path.set(seeded_path);

    const fs::path nested_working_directory = temp_root / "cwd" / "nested";
    fs::create_directories(nested_working_directory);
    const fs::path previous_working_directory = fs::current_path();
    fs::current_path(nested_working_directory);
    const fs::path resolved_root = copperfin::localization::resolve_catalog_root(executable_name);
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");
    fs::current_path(previous_working_directory);

    expect(
        fs::equivalent(resolved_root, temp_root / "resources" / "locales") &&
            catalog.translate("Command.Inspect") == "Inspect",
        "#3810: catalog root resolution should find build-output resources from a PATH-launched basename");

    fs::remove_all(temp_root, ignored);
}

void test_parser_behavior_remains_locale_invariant() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_parser_invariant_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path prg_path = temp_root / "scan.prg";
    write_text(
        prg_path,
        "x = 0\n"
        "FOR i = 1 TO 2\n"
        "    x = x + i\n"
        "ENDFOR\n");

    set_env_value("COPPERFIN_LOCALE", "es-419", true);
    copperfin::runtime::PrgRuntimeSession session =
        copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(prg_path, temp_root));
    const auto state = session.run(copperfin::runtime::DebugResumeAction::continue_run);
    set_env_value("COPPERFIN_LOCALE", "", false);

    const auto value = state.globals.find("x");
    expect(state.completed, "#1779: non-English locale should not affect parser execution");
    expect(
        value != state.globals.end() && copperfin::runtime::format_value(value->second) == "3",
        "#1779: VFP parser keywords and runtime semantics should remain locale-invariant");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_session_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.CommandUndo.Error.NoCommand") == "No command to UNDO",
        "#2607: command UNDO empty-stack error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.CriticalSection.Error.UnknownSection", {{"section", "orders"}}) ==
            "Unknown critical section: orders",
        "#2607: critical-section unknown-name error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalInitializeFailed") ==
            "Unable to initialize transaction journal",
        "#2607: transaction journal initialize error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalStatePersistFailed") ==
            "Unable to persist transaction journal state",
        "#2607: transaction journal state persist error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}}) ==
            "Unable to create transaction backup for: fixtures/people.dbf",
        "#2607: transaction backup error should preserve the named path placeholder in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.BackupJournalPersistFailed") ==
            "Unable to persist transaction backup journal",
        "#2607: transaction backup journal persist error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Transaction.Error.JournalReplayFailed") ==
            "Failed to replay transaction journal",
        "#2607: transaction replay error should remain catalog-backed in en-US");

    const std::string spanish_command_undo =
        spanish.translate("Runtime.Prg.CommandUndo.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}});
    expect(
        spanish_command_undo ==
            "No se pudo crear el respaldo de undo del comando para: fixtures/people.dbf",
        "#2607: es-419 command UNDO backup error should localize the prose while preserving the path");
    expect(
        spanish_command_undo.find("fixtures/people.dbf") != std::string::npos &&
            spanish_command_undo.find("Unable to create command undo backup") == std::string::npos,
        "#2607: es-419 command UNDO backup error should preserve the path without falling back to English prose");

    const std::string spanish_critical_section = spanish.translate(
        "Runtime.Prg.CriticalSection.Error.BlockingOperation",
        {{"operation", "WAIT"}, {"section", "orders"}});
    expect(
        spanish_critical_section ==
            "La operacion bloqueante WAIT no esta permitida mientras se mantiene la seccion CRITICAL orders",
        "#2607: es-419 critical-section blocking-operation error should localize the prose while preserving invariant tokens");
    expect(
        spanish_critical_section.find("WAIT") != std::string::npos &&
            spanish_critical_section.find("CRITICAL") != std::string::npos &&
            spanish_critical_section.find("Blocking operation") == std::string::npos,
        "#2607: es-419 critical-section blocking-operation error should preserve invariant tokens without falling back to English prose");

    const std::string portuguese_command_undo =
        portuguese.translate("Runtime.Prg.CommandUndo.Error.NoCommand");
    expect(
        portuguese_command_undo == "Nao ha comando para UNDO",
        "#2607: pt-BR command UNDO empty-stack error should localize the prose");

    const std::string portuguese_transaction =
        portuguese.translate("Runtime.Prg.Transaction.Error.JournalReplayFailed");
    expect(
        portuguese_transaction == "Falha ao reproduzir o diario da transacao",
        "#2607: pt-BR transaction replay error should localize the prose");

    const std::string pseudo_critical_section = pseudo.translate(
        "Runtime.Prg.CriticalSection.Error.MutexNotFound",
        {{"section", "orders"}});
    expect(
        pseudo_critical_section.find("[!! ") == 0U &&
            pseudo_critical_section.find("orders") != std::string::npos &&
            pseudo_critical_section.find("Critical section mutex not found: orders") == std::string::npos,
        "#2607: qps-ploc critical-section mutex error should pseudo-localize prose while preserving the section name");

    const std::string pseudo_transaction =
        pseudo.translate("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}});
    expect(
        pseudo_transaction.find("[!! ") == 0U &&
            pseudo_transaction.find("fixtures/people.dbf") != std::string::npos &&
        pseudo_transaction.find("Unable to create transaction backup for: fixtures/people.dbf") == std::string::npos,
        "#2607: qps-ploc transaction backup error should pseudo-localize prose while preserving the path");
}

void test_runtime_cursor_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Cursor.Error.AliasAlreadyOpen", {{"alias", "People"}}) ==
            "Alias already open in this data session: People",
        "#2608: cursor duplicate-alias error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Cursor.Error.TableAlreadyOpenUseAgainRequired", {{"path", "fixtures/people.dbf"}}) ==
            "Table already open in this data session; USE AGAIN is required: fixtures/people.dbf",
        "#2608: cursor duplicate-table error should preserve the named path placeholder in en-US");
    expect(
        english.translate("Runtime.Prg.Cursor.Error.UseTargetWorkAreaNotFound", {{"target", "42"}}) ==
            "USE target work area not found: 42",
        "#2608: cursor missing-work-area error should remain catalog-backed in en-US");

    const std::string spanish_alias =
        spanish.translate("Runtime.Prg.Cursor.Error.AliasAlreadyOpen", {{"alias", "People"}});
    expect(
        spanish_alias == "El alias ya esta abierto en esta sesion de datos: People",
        "#2608: es-419 cursor duplicate-alias error should localize the prose while preserving the alias");
    expect(
        spanish_alias.find("People") != std::string::npos &&
            spanish_alias.find("Alias already open in this data session") == std::string::npos,
        "#2608: es-419 cursor duplicate-alias error should preserve the alias without falling back to English prose");

    const std::string spanish_seek =
        spanish.translate("Runtime.Prg.Cursor.Error.SeekRequiresActiveOrder");
    expect(
        spanish_seek == "SEEK requiere un orden activo",
        "#2608: es-419 cursor active-order error should localize the prose");
    expect(
        spanish_seek.find("SEEK") != std::string::npos &&
            spanish_seek.find("requires an active order") == std::string::npos,
        "#2608: es-419 cursor active-order error should preserve SEEK without falling back to English prose");

    const std::string portuguese_use_target =
        portuguese.translate("Runtime.Prg.Cursor.Error.UseTargetResolveFailed", {{"path", "fixtures/missing.dbf"}});
    expect(
        portuguese_use_target == "Nao foi possivel resolver o destino de USE: fixtures/missing.dbf",
        "#2608: pt-BR cursor USE-target resolve error should localize the prose while preserving the path");

    const std::string portuguese_local_seek =
        portuguese.translate("Runtime.Prg.Cursor.Error.SeekRequiresLocalTableBackedCursor");
    expect(
        portuguese_local_seek == "SEEK exige um cursor local com suporte de tabela",
        "#2608: pt-BR cursor local-table requirement should localize the prose");

    const std::string pseudo_use_again = pseudo.translate(
        "Runtime.Prg.Cursor.Error.TableAlreadyOpenUseAgainRequired",
        {{"path", "fixtures/people.dbf"}});
    expect(
        pseudo_use_again.find("[!! ") == 0U &&
            pseudo_use_again.find("fixtures/people.dbf") != std::string::npos &&
            pseudo_use_again.find("Table already open in this data session; USE AGAIN is required: fixtures/people.dbf") == std::string::npos,
        "#2608: qps-ploc cursor duplicate-table error should pseudo-localize prose while preserving the path");

    const std::string pseudo_work_area =
        pseudo.translate("Runtime.Prg.Cursor.Error.UseTargetWorkAreaNotFound", {{"target", "42"}});
    expect(
        pseudo_work_area.find("[!! ") == 0U &&
            pseudo_work_area.find("42") != std::string::npos &&
        pseudo_work_area.find("USE target work area not found: 42") == std::string::npos,
        "#2608: qps-ploc cursor missing-work-area error should pseudo-localize prose while preserving the target");
}

void test_runtime_total_diagnostics_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Total.Error.RequiresToTarget") == "TOTAL requires a TO target",
        "#2609: TOTAL missing-target error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Total.Error.RequiresOnField") == "TOTAL requires an ON field",
        "#2609: TOTAL missing-ON-field error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}}) ==
            "TOTAL field was not found: MISSING",
        "#2609: TOTAL missing-field error should preserve the named field placeholder in en-US");

    const std::string spanish_target =
        spanish.translate("Runtime.Prg.Total.Error.RequiresSelectedWorkArea");
    expect(
        spanish_target == "TOTAL requiere un area de trabajo seleccionada",
        "#2609: es-419 TOTAL missing-work-area error should localize the prose");
    expect(
        spanish_target.find("TOTAL") != std::string::npos &&
            spanish_target.find("requires a selected work area") == std::string::npos,
        "#2609: es-419 TOTAL missing-work-area error should preserve TOTAL without falling back to English prose");

    const std::string spanish_field =
        spanish.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}});
    expect(
        spanish_field == "No se encontro el campo de TOTAL: MISSING",
        "#2609: es-419 TOTAL missing-field error should localize the prose while preserving the field name");
    expect(
        spanish_field.find("MISSING") != std::string::npos &&
            spanish_field.find("TOTAL field was not found") == std::string::npos,
        "#2609: es-419 TOTAL missing-field error should preserve the field name without falling back to English prose");

    const std::string portuguese_numeric =
        portuguese.translate("Runtime.Prg.Total.Error.RequiresNumericField");
    expect(
        portuguese_numeric == "TOTAL exige pelo menos um campo numerico para totalizar",
        "#2609: pt-BR TOTAL numeric-field requirement should localize the prose");

    const std::string portuguese_local_cursor =
        portuguese.translate("Runtime.Prg.Total.Error.RequiresLocalTableBackedCursor");
    expect(
        portuguese_local_cursor == "TOTAL exige um cursor local com suporte de tabela",
        "#2609: pt-BR TOTAL local-cursor requirement should localize the prose");

    const std::string pseudo_only_numeric =
        pseudo.translate("Runtime.Prg.Total.Error.OnlyNumericFields");
    expect(
        pseudo_only_numeric.find("[!! ") == 0U &&
            pseudo_only_numeric.find("TOTAL only supports numeric FIELDS in the first pass") == std::string::npos,
        "#2609: qps-ploc TOTAL numeric-fields error should pseudo-localize prose");

    const std::string pseudo_missing_field =
        pseudo.translate("Runtime.Prg.Total.Error.FieldNotFound", {{"fieldName", "MISSING"}});
    expect(
        pseudo_missing_field.find("[!! ") == 0U &&
            pseudo_missing_field.find("MISSING") != std::string::npos &&
            pseudo_missing_field.find("TOTAL field was not found: MISSING") == std::string::npos,
        "#2609: qps-ploc TOTAL missing-field error should pseudo-localize prose while preserving the field name");
}

void test_runtime_report_output_messages_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.PathRequired") ==
            "REPORT/LABEL TO clause requires a writable output path",
        "#2536: report output path-required error should be catalog-backed");
    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to open report output path: renders/invoice.txt",
        "#2536: report output open error should preserve the named path placeholder");
    expect(
        english.translate("Runtime.Prg.ReportOutput.Error.WriteFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to write report output path: renders/invoice.txt",
        "#2536: report output write error should preserve the named path placeholder");
    expect(
        english.translate("Runtime.Prg.ReportAsset.Error.ResolveFailed", {{"path", "reports/missing_invoice.frx"}}) ==
            "Unable to resolve report asset: reports/missing_invoice.frx",
        "#2597: report asset resolve error should be catalog-backed");

    const std::string spanish_path_required =
        spanish.translate("Runtime.Prg.ReportOutput.Error.PathRequired");
    expect(
        spanish_path_required == "La clausula TO de REPORT/LABEL requiere una ruta de salida escribible",
        "#2597: es-419 report output path-required error should localize the prose");
    expect(
        spanish_path_required.find("REPORT/LABEL") != std::string::npos &&
            spanish_path_required.find("TO") != std::string::npos &&
            spanish_path_required.find("requires a writable output path") == std::string::npos,
        "#2597: es-419 report output path-required error should preserve command tokens without falling back to English prose");

    const std::string portuguese_resolve =
        portuguese.translate("Runtime.Prg.ReportAsset.Error.ResolveFailed", {{"path", "reports/missing_invoice.frx"}});
    expect(
        portuguese_resolve == "Nao foi possivel resolver o asset do relatorio: reports/missing_invoice.frx",
        "#2597: pt-BR report asset resolve error should localize the prose while preserving the path");

    const std::string pseudo_open =
        pseudo.translate("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", "renders/invoice.txt"}});
    expect(
        pseudo_open.find("[!! ") == 0U &&
            pseudo_open.find("renders/invoice.txt") != std::string::npos &&
            pseudo_open.find("Unable to open report output path") == std::string::npos,
        "#2597: qps-ploc report output open error should pseudo-localize prose while preserving the path");
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_synthetic_vfp_asset(const std::filesystem::path& path) {
    const auto bytes = make_vfp_header();
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

void test_runtime_report_output_errors_localize_without_changing_runtime_behavior() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_report_output_localization";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_vfp_asset(report_path);

    auto run_script = [&](const std::string& stem, const std::string& script) {
        const fs::path main_path = temp_root / (stem + ".prg");
        write_text(main_path, script);
        copperfin::runtime::PrgRuntimeSession session =
            copperfin::runtime::PrgRuntimeSession::create(make_runtime_session_options(main_path, temp_root));
        return session.run(copperfin::runtime::DebugResumeAction::continue_run);
    };

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto state = run_script(
            "report_missing_path_es",
            "REPORT FORM '" + report_path.string() + "' TO FILE\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: es-419 report output missing-path script should pause with an error");
        expect(state.message == "La clausula TO de REPORT/LABEL requiere una ruta de salida escribible",
               "#2597: es-419 report output missing-path error should localize the prose (got '" + state.message + "')");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "pt-BR", true);
        const fs::path blocking_parent = temp_root / "not_a_directory";
        write_text(blocking_parent, "block");
        const auto state = run_script(
            "report_open_failed_pt",
            "REPORT FORM '" + report_path.string() + "' TO FILE '" + (blocking_parent / "invoice.txt").string() + "'\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: pt-BR report output open-failed script should pause with an error");
        expect(state.message == "Nao foi possivel abrir o caminho de saida do relatorio: " + (blocking_parent / "invoice.txt").string(),
               "#2597: pt-BR report output open-failed error should localize the prose while preserving the path");
    }

    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
        const auto state = run_script(
            "report_missing_asset_qps",
            "REPORT FORM '" + (temp_root / "missing_invoice.frx").string() + "' PREVIEW\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: qps-ploc missing report asset script should pause with an error");
        expect(state.message.find("[!! ") == 0U &&
                   state.message.find((temp_root / "missing_invoice.frx").string()) != std::string::npos &&
                   state.message.find("Unable to resolve report asset") == std::string::npos,
               "#2597: qps-ploc missing report asset error should pseudo-localize prose while preserving the path");
    }

#if defined(__linux__)
    {
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
        set_env_value("COPPERFIN_LOCALE", "es-419", true);
        const auto state = run_script(
            "report_write_failed_es",
            "REPORT FORM '" + report_path.string() + "' TO FILE '/dev/full'\n");
        expect(state.reason == copperfin::runtime::DebugPauseReason::error,
               "#2597: es-419 report output write-failed script should pause with an error on /dev/full");
        expect(state.message == "No se pudo escribir la ruta de salida del reporte: /dev/full",
               "#2597: es-419 report output write-failed error should localize the prose while preserving the path");
    }
#endif

    fs::remove_all(temp_root, ignored);
}

void test_runtime_aggregate_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap function_placeholders{{"function", "SUM"}};
    const copperfin::localization::PlaceholderMap to_placeholders{
        {"function", "SUM"},
        {"toKeyword", "TO"},
    };
    const copperfin::localization::PlaceholderMap to_array_placeholders{
        {"function", "SUM"},
        {"toKeyword", "TO"},
        {"arrayKeyword", "ARRAY"},
    };

    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAssignments") ==
            "CALCULATE requires one or more aggregate TO/INTO assignments",
        "#2595: aggregate missing-assignment error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAggregateExpression") ==
            "CALCULATE requires aggregate expressions like COUNT() or SUM(field)",
        "#2595: aggregate malformed-expression error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.CountToSingleTarget") ==
            "COUNT TO only accepts a single variable target",
        "#2595: count multi-target error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.RequiresSelectedWorkArea", function_placeholders) ==
            "SUM requires a selected work area",
        "#2721: aggregate no-work-area error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.TargetWorkAreaNotFound", function_placeholders) ==
            "SUM target work area not found",
        "#2721: aggregate missing-target-work-area error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToArrayRequiresTargetArrayName", to_array_placeholders) ==
            "SUM TO ARRAY requires a target array name",
        "#2721: aggregate TO ARRAY missing-target error should preserve invariant command tokens");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToArraySingleTargetOnly", to_array_placeholders) ==
            "SUM TO ARRAY accepts exactly one array target",
        "#2721: aggregate TO ARRAY multi-target error should preserve invariant command tokens");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.RequiresExpressions", function_placeholders) ==
            "SUM requires one or more expressions",
        "#2721: aggregate missing-expressions error should preserve the invariant command token");
    expect(
        english.translate("Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression", to_placeholders) ==
            "SUM TO requires one variable per aggregate expression",
        "#2721: aggregate TO target-count mismatch error should preserve invariant command tokens");

    const std::string spanish_assignments =
        spanish.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAssignments");
    expect(
        spanish_assignments == "CALCULATE requiere una o mas asignaciones agregadas TO/INTO",
        "#2595: es-419 aggregate missing-assignment error should localize the prose");
    expect(
        spanish_assignments.find("CALCULATE") != std::string::npos &&
            spanish_assignments.find("TO/INTO") != std::string::npos &&
            spanish_assignments.find("requires one or more") == std::string::npos,
        "#2595: es-419 aggregate missing-assignment error should preserve invariant command tokens without falling back to English prose");

    const std::string portuguese_expression =
        portuguese.translate("Runtime.Prg.Aggregate.Error.CalculateRequiresAggregateExpression");
    expect(
        portuguese_expression == "CALCULATE exige expressoes agregadas como COUNT() ou SUM(field)",
        "#2595: pt-BR aggregate malformed-expression error should localize the prose");
    expect(
        spanish.translate("Runtime.Prg.Aggregate.Error.RequiresSelectedWorkArea", function_placeholders)
                .find("requires a selected work area") == std::string::npos,
        "#2721: es-419 aggregate no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Aggregate.Error.ToRequiresVariablePerAggregateExpression", to_placeholders)
                .find("one variable per aggregate expression") == std::string::npos,
        "#2721: pt-BR aggregate TO target-count mismatch error should not fall back to raw English");

    const std::string pseudo_count =
        pseudo.translate("Runtime.Prg.Aggregate.Error.CountToSingleTarget");
    expect(
        pseudo_count ==
            copperfin::localization::pseudo_localize("COUNT TO only accepts a single variable target"),
        "#2595: qps-ploc aggregate count-target error should resolve through the pseudo-localization transform");
    const std::string pseudo_to_array =
        pseudo.translate("Runtime.Prg.Aggregate.Error.ToArrayRequiresTargetArrayName", to_array_placeholders);
    expect(
        pseudo_to_array.find("[!! ") == 0U &&
            pseudo_to_array.find("SUM") != std::string::npos &&
            pseudo_to_array.find("TO ARRAY") != std::string::npos &&
            pseudo_to_array.find("requires a target array name") == std::string::npos,
        "#2721: qps-ploc aggregate TO ARRAY error should pseudo-localize prose while preserving invariant command tokens");
}

void test_runtime_sql_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}}) ==
            "SQL handle not found: 7",
        "#2596: SQL missing-handle error should remain catalog-backed in en-US");
    expect(
        english.translate("Runtime.Prg.Sql.Error.SqlExecRequiresCommandOrPreparedStatement") ==
            "SQLEXEC requires a command or a prepared SQL statement",
        "#2596: SQLEXEC missing-command error should remain catalog-backed in en-US");

    const std::string spanish_handle =
        spanish.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}});
    expect(
        spanish_handle == "No se encontro el handle SQL: 7",
        "#2596: es-419 SQL missing-handle error should localize the prose");
    expect(
        spanish_handle.find("7") != std::string::npos &&
            spanish_handle.find("SQL") != std::string::npos &&
            spanish_handle.find("SQL handle not found") == std::string::npos,
        "#2596: es-419 SQL missing-handle error should preserve invariant handle and SQL token values without falling back to English prose");

    const std::string portuguese_sqlexec =
        portuguese.translate("Runtime.Prg.Sql.Error.SqlExecRequiresCommandOrPreparedStatement");
    expect(
        portuguese_sqlexec == "SQLEXEC exige um comando ou uma instrucao SQL preparada",
        "#2596: pt-BR SQLEXEC missing-command error should localize the prose");

    const std::string pseudo_handle =
        pseudo.translate("Runtime.Prg.Sql.Error.HandleNotFound", {{"handle", "7"}});
    expect(
        pseudo_handle.find("[!! ") == 0U &&
            pseudo_handle.find("7") != std::string::npos &&
            pseudo_handle.find("SQL handle not found: 7") == std::string::npos,
        "#2596: qps-ploc SQL missing-handle error should pseudo-localize prose while preserving the handle value");
}

void test_build_host_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap usage_placeholders{
        {"buildCommand", "build"},
        {"commandName", "copperfin_build_host"},
        {"configurationOption", "--configuration"},
        {"configurationValue", "debug|release"},
        {"emitDotnetLauncherOption", "--emit-dotnet-launcher"},
        {"enableSecurityOption", "--enable-security"},
        {"outputDirOption", "--output-dir"},
        {"outputDirValue", "<directory>"},
        {"projectOption", "--project"},
        {"projectValue", "<path-to-pjx>"},
        {"runtimeHostOption", "--runtime-host"},
        {"runtimeHostValue", "<path>"}
    };

    const std::string english_usage = english.translate("BuildHost.Usage", usage_placeholders);
    const std::string spanish_usage = spanish.translate("BuildHost.Usage", usage_placeholders);
    const std::string portuguese_usage = portuguese.translate("BuildHost.Usage", usage_placeholders);
    const std::string pseudo_usage = pseudo.translate("BuildHost.Usage", usage_placeholders);

    expect(
        english.translate("BuildHost.Warning.ProcessHardening", {{"message", "harden"}}) == "warning: harden",
        "#2539: build-host process-hardening warning label should be catalog-backed");
    expect(
        spanish_usage.find("Uso: copperfin_build_host build") != std::string::npos &&
            spanish_usage.find("--project") != std::string::npos &&
            spanish_usage.find("debug|release") != std::string::npos,
        "#2539: es-419 build-host usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        spanish_usage != english_usage && spanish_usage.find("Usage: copperfin_build_host") == std::string::npos,
        "#2539: es-419 build-host usage should not fall back to raw English prose");
    expect(
        portuguese.translate("BuildHost.Error.UnknownOrIncompleteArgument", {{"argument", "--project"}}) ==
            "Argumento desconhecido ou incompleto: --project",
        "#2539: pt-BR build-host parse diagnostics should preserve CLI placeholders");
    expect(
        portuguese_usage != english_usage && portuguese_usage.find("Usage: copperfin_build_host") == std::string::npos,
        "#2539: pt-BR build-host usage should not fall back to raw English prose");
    expect(
        pseudo_usage.find("[!! ") == 0U &&
            pseudo_usage.find("copperfin_build_host") != std::string::npos &&
            pseudo_usage.find("--output-dir") != std::string::npos,
        "#2539: qps-ploc build-host usage should pseudo-localize prose while preserving placeholder values");
}

void test_inspect_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap usage_placeholders{
        {"assetPathArgument", "<path-to-vfp-asset>"},
        {"commandName", "copperfin_inspect"},
        {"localeOption", "--locale"},
        {"localeValue", "<locale>"}
    };

    const std::string english_usage = english.translate("Inspect.Usage", usage_placeholders);
    const std::string spanish_usage = spanish.translate("Inspect.Usage", usage_placeholders);
    const std::string portuguese_usage = portuguese.translate("Inspect.Usage", usage_placeholders);
    const std::string pseudo_usage = pseudo.translate("Inspect.Usage", usage_placeholders);

    expect(
        spanish_usage.find("Uso: copperfin_inspect") != std::string::npos &&
            spanish_usage.find("--locale") != std::string::npos &&
            spanish_usage.find("<path-to-vfp-asset>") != std::string::npos,
        "#2579: es-419 inspect usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        spanish_usage != english_usage && spanish_usage.find("Usage: copperfin_inspect") == std::string::npos,
        "#2579: es-419 inspect usage should not fall back to raw English prose");
    expect(
        portuguese_usage.find("Uso: copperfin_inspect") != std::string::npos &&
            portuguese_usage.find("--locale") != std::string::npos &&
            portuguese_usage.find("<path-to-vfp-asset>") != std::string::npos,
        "#2579: pt-BR inspect usage should preserve CLI invariants while routing prose through the catalog");
    expect(
        portuguese_usage != english_usage && portuguese_usage.find("Usage: copperfin_inspect") == std::string::npos,
        "#2579: pt-BR inspect usage should not fall back to raw English prose");
    expect(
        pseudo_usage.find("[!! ") == 0U &&
            pseudo_usage.find("copperfin_inspect") != std::string::npos &&
            pseudo_usage.find("--locale") != std::string::npos,
        "#2579: qps-ploc inspect usage should pseudo-localize prose while preserving placeholder values");
}

void test_shared_core_catalog_entries_cover_placeholder_locales() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        spanish.translate("Command.Build") == "Compilar",
        "#2584: es-419 shared command labels should localize build labels");
    expect(
        portuguese.translate("Command.Inspect") == "Inspecionar",
        "#2584: pt-BR shared command labels should localize inspect labels");
    expect(
        spanish.translate("Help.LocaleOption") != english.translate("Help.LocaleOption") &&
            spanish.translate("Help.LocaleOption").find("Select the user-interface locale.") == std::string::npos,
        "#2584: es-419 shared help prose should not fall back to raw English");

    const std::string spanish_diagnostic = spanish.translate(
        "Diagnostic.ExpectedTokenBeforeToken",
        {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}});
    expect(
        spanish_diagnostic == "Se esperaba ENDSCAN antes de ENDIF.",
        "#2584: es-419 shared diagnostics should preserve parser tokens while localizing prose");

    const std::string portuguese_error =
        portuguese.translate("Error.UnknownLocale", {{"locale", "zz-ZZ"}});
    expect(
        portuguese_error == "Localidade desconhecida: zz-ZZ",
        "#2584: pt-BR shared error prose should preserve locale placeholders");

    const std::string pseudo_diagnostic = pseudo.translate(
        "Diagnostic.ExpectedTokenBeforeToken",
        {{"expectedToken", "ENDSCAN"}, {"actualToken", "ENDIF"}});
    expect(
        pseudo_diagnostic.find("[!! ") == 0U &&
            pseudo_diagnostic.find("ENDSCAN") != std::string::npos &&
            pseudo_diagnostic.find("ENDIF") != std::string::npos &&
            pseudo_diagnostic.find("{expectedToken}") == std::string::npos,
        "#2584: qps-ploc shared diagnostics should pseudo-localize prose while preserving placeholders");
}

void test_runtime_host_manifest_verification_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap file_name{{"fileName", "helper.dll"}};

    expect(
        english.translate("RuntimeHost.Error.ManifestMissingRuntimeHostSha256") ==
            "Security-enabled manifest is missing runtime_host_sha256.",
        "#2588: runtime-host manifest verification should preserve the en-US missing runtime_host_sha256 output");
    expect(
        english.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name) ==
            "Extension payload is missing from the package: helper.dll",
        "#2588: runtime-host manifest verification should preserve the en-US payload-path placeholder output");

    const std::string spanish_missing_sha256 =
        spanish.translate("RuntimeHost.Error.ManifestMissingRuntimeHostSha256");
    expect(
        spanish_missing_sha256 ==
            "Al manifiesto con seguridad habilitada le falta runtime_host_sha256.",
        "#2588: es-419 runtime-host manifest verification should localize missing runtime_host_sha256 prose");
    expect(
        spanish_missing_sha256.find("Security-enabled manifest is missing") == std::string::npos,
        "#2588: es-419 runtime-host manifest verification should not fall back to raw English prose");

    const std::string portuguese_missing_payload =
        portuguese.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name);
    expect(
        portuguese_missing_payload ==
            "O payload de extensao esta ausente do pacote: helper.dll",
        "#2588: pt-BR runtime-host manifest verification should localize payload-path prose while preserving the file name");
    expect(
        portuguese_missing_payload.find("helper.dll") != std::string::npos &&
            portuguese_missing_payload.find("Extension payload is missing") == std::string::npos,
        "#2588: pt-BR runtime-host manifest verification should preserve payload placeholders without falling back to English");

    const std::string pseudo_missing_payload =
        pseudo.translate("RuntimeHost.Error.ExtensionPayloadMissingFromPackage", file_name);
    expect(
        pseudo_missing_payload.find("[!! ") == 0U &&
            pseudo_missing_payload.find("helper.dll") != std::string::npos &&
            pseudo_missing_payload.find("{fileName}") == std::string::npos,
        "#2588: qps-ploc runtime-host manifest verification should pseudo-localize prose while preserving payload placeholders");
}

void test_runtime_host_quit_prompt_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap confirmation_tokens{
        {"yesToken", "y"},
        {"defaultNoToken", "N"}
    };

    expect(
        english.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens) ==
            "Do you want to quit this application? [y/N]: ",
        "#2591: runtime-host quit prompt should preserve the en-US confirmation prompt");

    const std::string spanish_prompt = spanish.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        spanish_prompt == "Desea salir de esta aplicacion? [y/N]: ",
        "#2591: es-419 runtime-host quit prompt should localize the prose");
    expect(
        spanish_prompt.find("Do you want to quit this application?") == std::string::npos &&
            spanish_prompt.find("[y/N]: ") != std::string::npos,
        "#2591: es-419 runtime-host quit prompt should preserve confirmation tokens without falling back to English");

    const std::string portuguese_prompt =
        portuguese.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        portuguese_prompt == "Deseja sair deste aplicativo? [y/N]: ",
        "#2591: pt-BR runtime-host quit prompt should localize the prose");

    const std::string pseudo_prompt = pseudo.translate("RuntimeHost.Prompt.QuitConfirm", confirmation_tokens);
    expect(
        pseudo_prompt.find("[!! ") == 0U &&
            pseudo_prompt.find("[y/N]: ") != std::string::npos &&
            pseudo_prompt.find("Do you want to quit this application?") == std::string::npos,
        "#2591: qps-ploc runtime-host quit prompt should pseudo-localize prose while preserving confirmation tokens");
}

void test_runtime_host_security_policy_denial_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap denial_placeholders{
        {"permission", "runtime.admin"},
        {"role", "developer"}
    };

    expect(
        english.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders) ==
            "Security policy denied runtime.admin for role 'developer'.",
        "#2592: runtime-host security denial should preserve the en-US default output");

    const std::string spanish_denial =
        spanish.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        spanish_denial == "La politica de seguridad denego runtime.admin para el rol 'developer'.",
        "#2592: es-419 runtime-host security denial should localize the prose");
    expect(
        spanish_denial.find("Security policy denied") == std::string::npos &&
            spanish_denial.find("runtime.admin") != std::string::npos &&
            spanish_denial.find("developer") != std::string::npos,
        "#2592: es-419 runtime-host security denial should preserve invariant permission and role ids without falling back to English");

    const std::string portuguese_denial =
        portuguese.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        portuguese_denial == "A politica de seguranca negou runtime.admin para a funcao 'developer'.",
        "#2592: pt-BR runtime-host security denial should localize the prose");

    const std::string pseudo_denial =
        pseudo.translate("RuntimeHost.Error.SecurityPolicyDenied", denial_placeholders);
    expect(
        pseudo_denial.find("[!! ") == 0U &&
            pseudo_denial.find("runtime.admin") != std::string::npos &&
            pseudo_denial.find("developer") != std::string::npos &&
            pseudo_denial.find("Security policy denied") == std::string::npos,
        "#2592: qps-ploc runtime-host security denial should pseudo-localize prose while preserving invariant ids");
}

void test_platform_federation_ai_planner_fallback_routes_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::string english_translation_error =
        english.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string spanish_translation_error =
        spanish.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string portuguese_translation_error =
        portuguese.translate("Platform.QueryTranslator.Error.SelectFromOnly");
    const std::string pseudo_translation_error =
        pseudo.translate("Platform.QueryTranslator.Error.SelectFromOnly");

    const copperfin::localization::PlaceholderMap english_placeholders{
        {"planMode", "optional"},
        {"translationError", english_translation_error}
    };
    const copperfin::localization::PlaceholderMap spanish_placeholders{
        {"planMode", "optional"},
        {"translationError", spanish_translation_error}
    };
    const copperfin::localization::PlaceholderMap portuguese_placeholders{
        {"planMode", "optional"},
        {"translationError", portuguese_translation_error}
    };
    const copperfin::localization::PlaceholderMap pseudo_placeholders{
        {"planMode", "optional"},
        {"translationError", pseudo_translation_error}
    };

    expect(
        english.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", english_placeholders) ==
            "Planner is not yet implemented for optional AI policy. Deterministic translation failed: "
            "Only first-pass SELECT...FROM SQL translation is supported.",
        "#2593: federation AI planner fallback should preserve the en-US default output");

    const std::string spanish_message =
        spanish.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", spanish_placeholders);
    expect(
        spanish_message ==
            "El planner aun no esta implementado para la politica de IA optional. La traduccion deterministica fallo: "
            "Solo se admite la traduccion SQL deterministica de primera pasada de SELECT...FROM.",
        "#2594: es-419 federation AI planner fallback should localize both the wrapper and translator payload");
    expect(
        spanish_message.find("Planner is not yet implemented") == std::string::npos &&
            spanish_message.find("optional") != std::string::npos &&
            spanish_message.find("Only first-pass SELECT...FROM SQL translation is supported.") == std::string::npos &&
            spanish_message.find("SELECT...FROM") != std::string::npos,
        "#2594: es-419 federation AI planner fallback should preserve invariant SQL tokens without falling back to English payload prose");

    const std::string portuguese_message =
        portuguese.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", portuguese_placeholders);
    expect(
        portuguese_message ==
            "O planner ainda nao esta implementado para a politica de IA optional. A traducao deterministica falhou: "
            "Somente a traducao SQL deterministica de primeira passagem de SELECT...FROM e suportada.",
        "#2594: pt-BR federation AI planner fallback should localize both the wrapper and translator payload");

    const std::string pseudo_message =
        pseudo.translate("Platform.FederationExecution.Error.AiPlannerNotImplemented", pseudo_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find(pseudo_translation_error) != std::string::npos &&
            pseudo_message.find("optional") != std::string::npos &&
            pseudo_message.find("Platform.QueryTranslator.Error.SelectFromOnly") == std::string::npos &&
            pseudo_message.find("Only first-pass SELECT...FROM SQL translation is supported.") == std::string::npos &&
            pseudo_message.find("Planner is not yet implemented") == std::string::npos,
        "#2594: qps-ploc federation AI planner fallback should pseudo-localize both the wrapper and translator payload without leaking raw English or unresolved keys");
}

void test_runtime_numeric_domain_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap log_placeholders{
        {"function", "LOG()"},
        {"value", "0.000000"}
    };
    const copperfin::localization::PlaceholderMap asin_placeholders{
        {"function", "ASIN()"},
        {"value", "2.000000"}
    };

    expect(
        english.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders) ==
            "LOG() requires a positive argument (got 0.000000)",
        "#2540: positive-argument numeric error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired", asin_placeholders) ==
            "ASIN() requires an argument between -1 and 1 (got 2.000000)",
        "#2540: unit-range numeric error should preserve en-US default output");

    const std::string spanish_log =
        spanish.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders);
    expect(
        spanish_log.find("LOG()") != std::string::npos &&
            spanish_log.find("0.000000") != std::string::npos &&
            spanish_log.find("requires a positive argument") == std::string::npos,
        "#2540: es-419 numeric error should preserve placeholders without falling back to English prose");

    const std::string portuguese_asin =
        portuguese.translate("Runtime.Prg.Numeric.Error.UnitRangeArgumentRequired", asin_placeholders);
    expect(
        portuguese_asin.find("ASIN()") != std::string::npos &&
            portuguese_asin.find("2.000000") != std::string::npos &&
            portuguese_asin.find("requires an argument between") == std::string::npos,
        "#2540: pt-BR numeric error should preserve placeholders without falling back to English prose");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.Numeric.Error.PositiveArgumentRequired", log_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("LOG()") != std::string::npos &&
            pseudo_message.find("0.000000") != std::string::npos &&
            pseudo_message.find("{function}") == std::string::npos,
        "#2540: qps-ploc numeric error should pseudo-localize prose while preserving placeholders");
}

void test_runtime_expression_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero") ==
            "Division by zero in integer expression",
        "#2541: integer division-by-zero error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Expression.Error.DivisionByZero") == "Division by zero",
        "#2541: division-by-zero error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument") == "Expected function argument",
        "#2541: expected-function-argument error should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.Expression.Error.DivisionByZero") !=
            english.translate("Runtime.Prg.Expression.Error.DivisionByZero"),
        "#2541: es-419 expression division error should not fall back to raw English prose");
    expect(
        portuguese.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument") !=
            english.translate("Runtime.Prg.Expression.Error.ExpectedFunctionArgument"),
        "#2541: pt-BR expected-argument error should not fall back to raw English prose");

    const std::string pseudo_message = pseudo.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero");
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message != english.translate("Runtime.Prg.Expression.Error.IntegerDivisionByZero"),
        "#2541: qps-ploc expression error should pseudo-localize prose");
}

void test_runtime_record_precondition_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap replace_placeholders{{"command", "REPLACE"}};
    const copperfin::localization::PlaceholderMap field_placeholders{{"fieldName", "CustomerID"}};
    const copperfin::localization::PlaceholderMap append_blank_placeholders{{"command", "APPEND BLANK"}};
    const copperfin::localization::PlaceholderMap constraint_placeholders{
        {"constraint", "NOT NULL"},
        {"fieldName", "CustomerID"}
    };
    const copperfin::localization::PlaceholderMap insert_placeholders{{"command", "INSERT INTO"}};

    expect(
        english.translate("Runtime.Prg.Records.Error.RequiresLocalTableBackedCursor") ==
            "This command requires a local table-backed cursor",
        "#2542: generic local table-backed cursor error should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentRemoteRecord", replace_placeholders) ==
            "REPLACE requires a current remote record",
        "#2542: command-specific remote record error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord", replace_placeholders) ==
            "REPLACE requires a current local record",
        "#2542: command-specific local record error should preserve command placeholder");
    expect(
        english.translate(
            "Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor",
            append_blank_placeholders) == "APPEND BLANK requires a local table-backed cursor",
        "#2547: command-specific local table-backed cursor error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders) ==
            "Field not found on remote SQL cursor: CustomerID",
        "#2542: remote SQL field error should preserve field-name placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.LockRetryCancelled") == "Lock retry cancelled.",
        "#2542: lock retry cancellation should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Records.Error.ConstraintFieldNotFound", constraint_placeholders) ==
            "NOT NULL field not found: CustomerID",
        "#2546: NOT NULL field-not-found error should preserve constraint and field placeholders");
    expect(
        english.translate("Runtime.Prg.Records.Error.ConstraintFailedForField", constraint_placeholders) ==
            "NOT NULL constraint failed for field: CustomerID",
        "#2546: NOT NULL constraint error should preserve constraint and field placeholders");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertTargetFieldsResolveFailed", insert_placeholders) ==
            "INSERT INTO could not resolve target field names",
        "#2546: INSERT INTO field resolution error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertRequiresTargetField", insert_placeholders) ==
            "INSERT INTO requires at least one target field",
        "#2546: INSERT INTO target-field error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Records.Error.InsertFieldValueCountMismatch", insert_placeholders) ==
            "INSERT INTO field/value counts do not match",
        "#2546: INSERT INTO count mismatch error should preserve command placeholder");

    const std::string spanish_field =
        spanish.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders);
    expect(
        spanish_field.find("CustomerID") != std::string::npos &&
            spanish_field.find("Field not found") == std::string::npos,
        "#2542: es-419 remote SQL field error should preserve field name without falling back to English");

    const std::string portuguese_replace =
        portuguese.translate("Runtime.Prg.Records.Error.CommandRequiresCurrentLocalRecord", replace_placeholders);
    expect(
        portuguese_replace.find("REPLACE") != std::string::npos &&
            portuguese_replace.find("requires a current local record") == std::string::npos,
        "#2542: pt-BR command-specific record error should preserve command without falling back to English");
    const std::string spanish_append =
        spanish.translate("Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor", append_blank_placeholders);
    expect(
        spanish_append.find("APPEND BLANK") != std::string::npos &&
            spanish_append.find("requires a local table-backed cursor") == std::string::npos,
        "#2547: es-419 table-backed cursor error should preserve command without falling back to English");
    const std::string spanish_insert =
        spanish.translate("Runtime.Prg.Records.Error.InsertFieldValueCountMismatch", insert_placeholders);
    expect(
        spanish_insert.find("INSERT INTO") != std::string::npos &&
            spanish_insert.find("field/value counts do not match") == std::string::npos,
        "#2546: es-419 INSERT INTO error should preserve command without falling back to English");
    const std::string portuguese_constraint =
        portuguese.translate("Runtime.Prg.Records.Error.ConstraintFailedForField", constraint_placeholders);
    expect(
        portuguese_constraint.find("NOT NULL") != std::string::npos &&
            portuguese_constraint.find("CustomerID") != std::string::npos &&
            portuguese_constraint.find("constraint failed") == std::string::npos,
        "#2546: pt-BR NOT NULL error should preserve invariant placeholders without falling back to English");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.Records.Error.RemoteSqlFieldNotFound", field_placeholders);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("CustomerID") != std::string::npos &&
            pseudo_message.find("{fieldName}") == std::string::npos,
        "#2542: qps-ploc record precondition error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_constraint =
        pseudo.translate("Runtime.Prg.Records.Error.ConstraintFieldNotFound", constraint_placeholders);
    expect(
        pseudo_constraint.find("[!! ") == 0U &&
            pseudo_constraint.find("NOT NULL") != std::string::npos &&
            pseudo_constraint.find("CustomerID") != std::string::npos &&
            pseudo_constraint.find("{constraint}") == std::string::npos &&
            pseudo_constraint.find("{fieldName}") == std::string::npos,
        "#2546: qps-ploc NOT NULL error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_append =
        pseudo.translate("Runtime.Prg.Records.Error.CommandRequiresLocalTableBackedCursor", append_blank_placeholders);
    expect(
        pseudo_append.find("[!! ") == 0U &&
            pseudo_append.find("APPEND BLANK") != std::string::npos &&
            pseudo_append.find("{command}") == std::string::npos,
        "#2547: qps-ploc table-backed cursor error should pseudo-localize prose while preserving command");
}

void test_runtime_dll_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap handle_placeholders{{"handle", "42"}};
    const copperfin::localization::PlaceholderMap hresult_placeholders{{"hresult", "-2146232576"}};
    const copperfin::localization::PlaceholderMap assembly_placeholders{
        {"hresult", "-2147024894"},
        {"path", "bin/Interop.dll"}
    };
    const copperfin::localization::PlaceholderMap type_placeholders{{"typeName", "Copperfin.Tools.Loader"}};
    const copperfin::localization::PlaceholderMap function_placeholders{{"functionName", "GetVersion"}};

    expect(
        english.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded") == "FOXTOOLS is not loaded",
        "#2548: FOXTOOLS load precondition should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Dll.Error.RegisteredApiHandleNotFound", handle_placeholders) ==
            "Registered API handle not found: 42",
        "#2548: API handle error should preserve handle placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.ClrCreateInstanceFailed", hresult_placeholders) ==
            "CLRCreateInstance failed: -2146232576",
        "#2549: CLRCreateInstance error should preserve HRESULT placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed", assembly_placeholders) ==
            "Could not load .NET assembly: bin/Interop.dll hr=-2147024894",
        "#2549: .NET assembly load error should preserve path and HRESULT placeholders");
    expect(
        english.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders) ==
            "Type not found: Copperfin.Tools.Loader",
        "#2549: .NET type lookup error should preserve type-name placeholder");
    expect(
        english.translate("Runtime.Prg.Dll.Error.NativeProcAddressMissing", function_placeholders) ==
            "No proc address for: GetVersion",
        "#2550: native proc-address error should preserve function-name placeholder");
    expect(
        spanish.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded").find("FOXTOOLS") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dll.Error.FoxtoolsNotLoaded").find("is not loaded") == std::string::npos,
        "#2548: es-419 FOXTOOLS error should preserve invariant product token without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders)
                    .find("Copperfin.Tools.Loader") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dll.Error.DotNetTypeNotFound", type_placeholders)
                    .find("Type not found") == std::string::npos,
        "#2549: es-419 .NET type error should preserve type name without falling back to English");

    const std::string pseudo_handle =
        pseudo.translate("Runtime.Prg.Dll.Error.RegisteredApiHandleNotFound", handle_placeholders);
    expect(
        pseudo_handle.find("[!! ") == 0U &&
            pseudo_handle.find("42") != std::string::npos &&
            pseudo_handle.find("{handle}") == std::string::npos,
        "#2548: qps-ploc API handle error should pseudo-localize prose while preserving handle");
    const std::string pseudo_assembly =
        pseudo.translate("Runtime.Prg.Dll.Error.DotNetAssemblyLoadFailed", assembly_placeholders);
    expect(
        pseudo_assembly.find("[!! ") == 0U &&
            pseudo_assembly.find("bin/Interop.dll") != std::string::npos &&
            pseudo_assembly.find("-2147024894") != std::string::npos &&
            pseudo_assembly.find("{path}") == std::string::npos &&
            pseudo_assembly.find("{hresult}") == std::string::npos,
        "#2549: qps-ploc .NET assembly error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_function =
        pseudo.translate("Runtime.Prg.Dll.Error.NativeProcAddressMissing", function_placeholders);
    expect(
        pseudo_function.find("[!! ") == 0U &&
            pseudo_function.find("GetVersion") != std::string::npos &&
            pseudo_function.find("{functionName}") == std::string::npos,
        "#2550: qps-ploc native proc-address error should pseudo-localize prose while preserving function name");
}

void test_runtime_core_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap detail_placeholders{{"detail", "disk full"}};
    const copperfin::localization::PlaceholderMap path_placeholders{{"path", "forms/customer.scx"}};
    const copperfin::localization::PlaceholderMap guardrail_placeholders{{"limit", "37"}};

    expect(
        english.translate("Runtime.Prg.Core.Error.AsyncTaskCancelled") == "Async task cancelled.",
        "#2551: async task cancellation should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceOutOfMemory") ==
            "Runtime resource fault: out of memory. Execution paused safely.",
        "#2551: out-of-memory resource fault should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceFilesystemFailure", detail_placeholders) ==
            "Runtime resource fault: filesystem failure: disk full",
        "#2551: filesystem resource fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.ResourceSystemError", detail_placeholders) ==
            "Runtime resource fault: system error: disk full",
        "#2551: system resource fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders) == "Runtime fault: disk full",
        "#2551: generic runtime fault should preserve detail placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.UnknownRuntimeFault") == "Runtime fault: unknown exception",
        "#2551: unknown runtime fault should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed", path_placeholders) ==
            "Unable to materialize xAsset bootstrap for: forms/customer.scx",
        "#2552: xAsset bootstrap error should preserve path placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailCallDepthExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum call depth (37) exceeded.",
        "#2720: call-depth guardrail fault should preserve the numeric limit placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailExecutedStatementsExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum executed statements (37) exceeded.",
        "#2720: executed-statements guardrail fault should preserve the numeric limit placeholder");
    expect(
        english.translate("Runtime.Prg.Core.Error.GuardrailLoopIterationsExceeded", guardrail_placeholders) ==
            "Runtime guardrail: maximum loop iterations (37) exceeded.",
        "#2720: loop-iterations guardrail fault should preserve the numeric limit placeholder");
    expect(
        spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("disk full") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("Runtime fault") ==
                std::string::npos,
        "#2551: es-419 runtime fault should preserve detail without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Core.Error.GuardrailExecutedStatementsExceeded", guardrail_placeholders)
                .find("maximum executed statements") == std::string::npos,
        "#2720: es-419 executed-statements guardrail fault should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Core.Error.GuardrailLoopIterationsExceeded", guardrail_placeholders)
                .find("maximum loop iterations") == std::string::npos,
        "#2720: pt-BR loop-iterations guardrail fault should not fall back to raw English");

    const std::string pseudo_fault =
        pseudo.translate("Runtime.Prg.Core.Error.ResourceFilesystemFailure", detail_placeholders);
    expect(
        pseudo_fault.find("[!! ") == 0U &&
            pseudo_fault.find("disk full") != std::string::npos &&
            pseudo_fault.find("{detail}") == std::string::npos,
        "#2551: qps-ploc runtime resource fault should pseudo-localize prose while preserving detail");
    const std::string pseudo_xasset =
        pseudo.translate("Runtime.Prg.Core.Error.XAssetBootstrapMaterializeFailed", path_placeholders);
    expect(
        pseudo_xasset.find("[!! ") == 0U &&
        pseudo_xasset.find("forms/customer.scx") != std::string::npos &&
            pseudo_xasset.find("{path}") == std::string::npos,
        "#2552: qps-ploc xAsset bootstrap error should pseudo-localize prose while preserving path");
    const std::string pseudo_guardrail =
        pseudo.translate("Runtime.Prg.Core.Error.GuardrailCallDepthExceeded", guardrail_placeholders);
    expect(
        pseudo_guardrail.find("[!! ") == 0U &&
            pseudo_guardrail.find("37") != std::string::npos &&
            pseudo_guardrail.find("maximum call depth") == std::string::npos,
        "#2720: qps-ploc call-depth guardrail fault should pseudo-localize prose while preserving the numeric limit");
}

void test_runtime_pause_and_session_messages_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const copperfin::localization::PlaceholderMap path_placeholders{{"path", "forms/customer.scx"}};

    expect(
        english.translate("Runtime.Prg.Session.Message.StoppedOnEntry") == "Stopped on entry.",
        "#2589: stopped-on-entry pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.WaitingInReadEvents") ==
            "The runtime is waiting in READ EVENTS.",
        "#2589: READ EVENTS pause message should preserve the invariant token in en-US");
    expect(
        english.translate("Runtime.Prg.Session.Message.ExecutionCompleted") == "Execution completed.",
        "#2589: execution-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.BreakpointHit") == "Breakpoint hit.",
        "#2589: breakpoint-hit pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepCompleted") == "Step completed.",
        "#2589: step-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepOverCompleted") == "Step-over completed.",
        "#2589: step-over-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Message.StepOutCompleted") == "Step-out completed.",
        "#2589: step-out-completed pause message should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset", path_placeholders) ==
            "No runnable startup methods were found in asset: forms/customer.scx",
        "#2589: no-runnable-startup message should preserve the asset path placeholder in en-US");

    const std::string spanish_waiting =
        spanish.translate("Runtime.Prg.Session.Message.WaitingInReadEvents");
    expect(
        spanish_waiting.find("READ EVENTS") != std::string::npos &&
            spanish_waiting.find("The runtime is waiting") == std::string::npos,
        "#2589: es-419 READ EVENTS pause message should preserve invariant tokens without falling back to English");

    const std::string portuguese_breakpoint =
        portuguese.translate("Runtime.Prg.Session.Message.BreakpointHit");
    expect(
        portuguese_breakpoint == "Um breakpoint foi atingido.",
        "#2589: pt-BR breakpoint-hit pause message should localize the prose");

    const std::string pseudo_step =
        pseudo.translate("Runtime.Prg.Session.Message.StepOverCompleted");
    expect(
        pseudo_step.find("[!! ") == 0U &&
            pseudo_step.find("Step-over completed.") == std::string::npos,
        "#2589: qps-ploc step-over pause message should pseudo-localize the prose");

    const std::string pseudo_no_runnable =
        pseudo.translate("Runtime.Prg.Session.Error.NoRunnableStartupMethodsFoundInAsset", path_placeholders);
    expect(
        pseudo_no_runnable.find("[!! ") == 0U &&
            pseudo_no_runnable.find("forms/customer.scx") != std::string::npos &&
            pseudo_no_runnable.find("{path}") == std::string::npos,
        "#2589: qps-ploc no-runnable-startup message should pseudo-localize prose while preserving the asset path");
}

void test_runtime_watch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english.translate("Runtime.Prg.Watch.Error.EmptyExpression") == "Watch expression is empty.",
        "#2590: empty watch-expression error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.RequiresPausedFrame") ==
            "Watch evaluation requires a paused runtime frame.",
        "#2590: paused-frame watch error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.OutOfMemory") ==
            "Watch evaluation ran out of memory.",
        "#2590: out-of-memory watch error should preserve the en-US default output");
    expect(
        english.translate("Runtime.Prg.Watch.Error.Failed") == "Watch evaluation failed.",
        "#2590: generic watch failure should preserve the en-US default output");

    const std::string spanish_empty = spanish.translate("Runtime.Prg.Watch.Error.EmptyExpression");
    expect(
        spanish_empty == "La expresion de watch esta vacia.",
        "#2590: es-419 empty watch-expression error should localize the prose");
    expect(
        spanish_empty.find("Watch expression is empty.") == std::string::npos,
        "#2590: es-419 empty watch-expression error should not fall back to raw English prose");

    const std::string portuguese_paused =
        portuguese.translate("Runtime.Prg.Watch.Error.RequiresPausedFrame");
    expect(
        portuguese_paused == "A avaliacao de watch exige um frame de runtime pausado.",
        "#2590: pt-BR paused-frame watch error should localize the prose");

    const std::string pseudo_failed = pseudo.translate("Runtime.Prg.Watch.Error.Failed");
    expect(
        pseudo_failed.find("[!! ") == 0U &&
            pseudo_failed.find("Watch evaluation failed.") == std::string::npos,
        "#2590: qps-ploc generic watch failure should pseudo-localize the prose");
}

void test_runtime_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap yield_placeholders{{"command", "YIELD"}};
    const copperfin::localization::PlaceholderMap go_placeholders{{"command", "GO"}};
    const copperfin::localization::PlaceholderMap replace_placeholders{{"command", "REPLACE"}};
    const copperfin::localization::PlaceholderMap update_placeholders{{"command", "UPDATE"}};
    const copperfin::localization::PlaceholderMap append_blank_placeholders{{"command", "APPEND BLANK"}};
    const copperfin::localization::PlaceholderMap delete_placeholders{{"command", "DELETE"}};
    const copperfin::localization::PlaceholderMap delete_from_placeholders{{"command", "DELETE FROM"}};
    const copperfin::localization::PlaceholderMap recall_placeholders{{"command", "RECALL"}};
    const copperfin::localization::PlaceholderMap insert_into_placeholders{{"command", "INSERT INTO"}};
    const copperfin::localization::PlaceholderMap pack_placeholders{{"command", "PACK"}};
    const copperfin::localization::PlaceholderMap zap_placeholders{{"command", "ZAP"}};
    const copperfin::localization::PlaceholderMap unlock_placeholders{{"command", "UNLOCK"}};
    const copperfin::localization::PlaceholderMap seek_placeholders{{"command", "SEEK"}};
    const copperfin::localization::PlaceholderMap skip_placeholders{{"command", "SKIP"}};
    const copperfin::localization::PlaceholderMap browse_placeholders{{"command", "BROWSE"}};
    const copperfin::localization::PlaceholderMap set_order_placeholders{{"command", "SET ORDER"}};
    const copperfin::localization::PlaceholderMap select_placeholders{{"command", "SELECT"}};
    const copperfin::localization::PlaceholderMap scan_placeholders{{"command", "SCAN"}};
    const copperfin::localization::PlaceholderMap do_target_placeholders{
        {"command", "DO"},
        {"target", "legacy/startup.prg"}
    };
    const copperfin::localization::PlaceholderMap call_target_placeholders{
        {"command", "CALL"},
        {"target", "NativeEntry"}
    };
    const copperfin::localization::PlaceholderMap spawn_placeholders{{"command", "SPAWN"}};
    const copperfin::localization::PlaceholderMap spawn_target_placeholders{
        {"command", "SPAWN"},
        {"target", "workers/process.prg"}
    };
    const copperfin::localization::PlaceholderMap await_placeholders{{"command", "AWAIT"}};
    const copperfin::localization::PlaceholderMap handle_placeholders{{"handle", "42"}};

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandDoesNotTakeArguments", yield_placeholders) ==
            "YIELD does not take arguments",
        "#2553: YIELD argument error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders) ==
            "GO target work area not found",
        "#2556: GO work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders) ==
            "SKIP target work area not found",
        "#2556: SKIP work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders) ==
            "BROWSE target work area not found",
        "#2556: BROWSE work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders) ==
            "SET ORDER target work area not found",
        "#2556: SET ORDER work-area error should localize through catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
            ": MISSING_ALIAS" ==
            "SELECT target work area not found: MISSING_ALIAS",
        "#2556: SELECT work-area error should localize through catalog and preserve selection");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders) ==
            "SEEK target work area not found",
        "#2555: SEEK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", replace_placeholders) ==
            "REPLACE target work area not found",
        "#2557: REPLACE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", update_placeholders) ==
            "UPDATE target work area not found",
        "#2557: UPDATE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", append_blank_placeholders) ==
            "APPEND BLANK target work area not found",
        "#2557: APPEND BLANK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders) ==
            "DELETE target work area not found",
        "#2557: DELETE work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_from_placeholders) ==
            "DELETE FROM target work area not found",
        "#2557: DELETE FROM work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", recall_placeholders) ==
            "RECALL target work area not found",
        "#2557: RECALL work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", insert_into_placeholders) ==
            "INSERT INTO target work area not found",
        "#2557: INSERT INTO work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", pack_placeholders) ==
            "PACK target work area not found",
        "#2557: PACK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", zap_placeholders) ==
            "ZAP target work area not found",
        "#2557: ZAP work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", unlock_placeholders) ==
            "UNLOCK target work area not found",
        "#2557: UNLOCK work-area error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders) ==
            "Unable to resolve DO target: legacy/startup.prg",
        "#2554: DO target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", call_target_placeholders) ==
            "Unable to resolve CALL target: NativeEntry",
        "#2554: CALL target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SpawnRequiresTarget", spawn_placeholders) ==
            "SPAWN requires a target routine or file",
        "#2553: SPAWN missing-target error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders) ==
            "Unable to resolve SPAWN target: workers/process.prg",
        "#2553: SPAWN target resolution error should preserve command and target placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AwaitRequiresTaskHandle", await_placeholders) ==
            "AWAIT requires a task handle",
        "#2553: AWAIT missing-handle error should preserve command placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UnknownTaskHandle", handle_placeholders) ==
            "Unknown task handle: 42",
        "#2553: unknown task-handle error should preserve handle placeholder");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders)
                    .find("workers/process.prg") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders)
                    .find("Unable to resolve") == std::string::npos,
        "#2553: es-419 SPAWN target error should preserve target without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders)
                    .find("legacy/startup.prg") != std::string::npos &&
            spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders)
                    .find("Unable to resolve") == std::string::npos,
        "#2554: es-419 DO target error should preserve target without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders)
                    .find("SEEK") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", seek_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2555: es-419 SEEK work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders).find("DELETE") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", delete_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2557: es-419 DELETE work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders).find("GO") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 GO work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders).find("SKIP") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", skip_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SKIP work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders).find("BROWSE") !=
            std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", browse_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 BROWSE work-area error should preserve command without falling back to English");
    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders)
                    .find("SET ORDER") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", set_order_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SET ORDER work-area error should preserve command without falling back to English");
    expect(
        (spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
         ": MISSING_ALIAS")
                .find("MISSING_ALIAS") != std::string::npos &&
        spanish.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders)
                    .find("target work area not found") == std::string::npos,
        "#2556: es-419 SELECT work-area error should preserve selection without falling back to English");

    const std::string pseudo_spawn =
        pseudo.translate("Runtime.Prg.Dispatch.Error.SpawnTargetResolveFailed", spawn_target_placeholders);
    expect(
        pseudo_spawn.find("[!! ") == 0U &&
            pseudo_spawn.find("SPAWN") != std::string::npos &&
            pseudo_spawn.find("workers/process.prg") != std::string::npos &&
            pseudo_spawn.find("{command}") == std::string::npos &&
            pseudo_spawn.find("{target}") == std::string::npos,
        "#2553: qps-ploc SPAWN target error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_do =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetResolveFailed", do_target_placeholders);
    expect(
        pseudo_do.find("[!! ") == 0U &&
            pseudo_do.find("DO") != std::string::npos &&
            pseudo_do.find("legacy/startup.prg") != std::string::npos &&
            pseudo_do.find("{command}") == std::string::npos &&
            pseudo_do.find("{target}") == std::string::npos,
        "#2554: qps-ploc DO target error should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_scan =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", scan_placeholders);
    expect(
        pseudo_scan.find("[!! ") == 0U &&
            pseudo_scan.find("SCAN") != std::string::npos &&
            pseudo_scan.find("{command}") == std::string::npos,
        "#2555: qps-ploc SCAN work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_go =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", go_placeholders);
    expect(
        pseudo_go.find("[!! ") == 0U &&
            pseudo_go.find("GO") != std::string::npos &&
            pseudo_go.find("{command}") == std::string::npos,
        "#2556: qps-ploc GO work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_replace =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", replace_placeholders);
    expect(
        pseudo_replace.find("[!! ") == 0U &&
            pseudo_replace.find("REPLACE") != std::string::npos &&
            pseudo_replace.find("target work area not found") == std::string::npos &&
            pseudo_replace.find("{command}") == std::string::npos,
        "#2557: qps-ploc REPLACE work-area error should pseudo-localize prose while preserving command");
    const std::string pseudo_select =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CommandTargetWorkAreaNotFound", select_placeholders) +
        ": MISSING_ALIAS";
    expect(
        pseudo_select.find("[!! ") == 0U &&
            pseudo_select.find("SELECT") != std::string::npos &&
            pseudo_select.find("MISSING_ALIAS") != std::string::npos &&
            pseudo_select.find("{command}") == std::string::npos,
        "#2556: qps-ploc SELECT work-area error should pseudo-localize prose while preserving selection");
}

void test_runtime_surface_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap bounds{
        {"maximum", "31"},
        {"minimum", "0"}
    };

    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds) ==
            "Bit position must be between 0 and 31",
        "#2543: bit-position range error should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("0") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("31") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds).find("Bit position") ==
                std::string::npos,
        "#2543: es-419 bit-position range error should preserve bounds without falling back to English");

    const std::string pseudo_message =
        pseudo.translate("Runtime.Prg.RuntimeSurface.Error.BitPositionOutOfRange", bounds);
    expect(
        pseudo_message.find("[!! ") == 0U &&
            pseudo_message.find("0") != std::string::npos &&
            pseudo_message.find("31") != std::string::npos &&
            pseudo_message.find("{minimum}") == std::string::npos,
        "#2543: qps-ploc bit-position range error should pseudo-localize prose while preserving bounds");

    const copperfin::localization::PlaceholderMap object_array_placeholders{
        {"capability", "object/array"},
        {"function", "AMEMBERS()"}
    };
    const copperfin::localization::PlaceholderMap function_placeholders{{"function", "GETPEM()"}};

    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
            object_array_placeholders) == "AMEMBERS() uses stub behavior (no object/array callback)",
        "#2544: object/array runtime-surface warning should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders) ==
            "GETPEM() uses stub behavior (no runtime object callback)",
        "#2544: runtime object callback warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.AmembersFallback",
            {{"function", "AMEMBERS()"}}) ==
            "AMEMBERS() fallback: unable to enumerate members, returning empty array",
        "#2544: AMEMBERS fallback warning should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders)
                .find("GETPEM()") != std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Warning.StubRuntimeObjectCallback", function_placeholders)
                    .find("uses stub behavior") == std::string::npos,
        "#2544: es-419 runtime object callback warning should preserve function name without falling back to English");

    const std::string pseudo_warning = pseudo.translate(
        "Runtime.Prg.RuntimeSurface.Warning.StubCapabilityCallback",
        object_array_placeholders);
    expect(
        pseudo_warning.find("[!! ") == 0U &&
            pseudo_warning.find("AMEMBERS()") != std::string::npos &&
            pseudo_warning.find("object/array") != std::string::npos &&
            pseudo_warning.find("{function}") == std::string::npos &&
            pseudo_warning.find("{capability}") == std::string::npos,
        "#2544: qps-ploc runtime-surface warning should pseudo-localize prose while preserving placeholders");

    const copperfin::localization::PlaceholderMap cursor_snapshot_placeholders{
        {"capability", "cursor snapshot"},
        {"function", "CURSORTOXML()"}
    };
    const copperfin::localization::PlaceholderMap xml_to_cursor_placeholders{{"function", "XMLTOCURSOR()"}};

    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback", cursor_snapshot_placeholders) ==
            "CURSORTOXML() unavailable (no cursor snapshot callback)",
        "#2545: cursor snapshot warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.CursorToXmlWriteFailed",
            {{"function", "CURSORTOXML()"}}) == "CURSORTOXML() failed to write target path",
        "#2545: CURSORTOXML write warning should preserve en-US default output");
    expect(
        english.translate(
            "Runtime.Prg.RuntimeSurface.Warning.XmlToCursorInputAndAliasRequired",
            xml_to_cursor_placeholders) == "XMLTOCURSOR() requires XML input and destination alias",
        "#2545: XMLTOCURSOR input warning should preserve en-US default output");
    expect(
        english.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders) ==
            "XMLTOCURSOR() could not parse the provided XML payload",
        "#2545: XMLTOCURSOR parse warning should preserve en-US default output");
    expect(
        spanish.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders)
                .find("XMLTOCURSOR()") != std::string::npos &&
            spanish.translate("Runtime.Prg.RuntimeSurface.Warning.XmlToCursorParseFailed", xml_to_cursor_placeholders)
                    .find("could not parse") == std::string::npos,
        "#2545: es-419 XMLTOCURSOR warning should preserve function name without falling back to English");

    const std::string pseudo_cursor_warning =
        pseudo.translate("Runtime.Prg.RuntimeSurface.Warning.UnavailableCallback", cursor_snapshot_placeholders);
    expect(
        pseudo_cursor_warning.find("[!! ") == 0U &&
            pseudo_cursor_warning.find("CURSORTOXML()") != std::string::npos &&
            pseudo_cursor_warning.find("cursor snapshot") != std::string::npos &&
            pseudo_cursor_warning.find("{function}") == std::string::npos &&
            pseudo_cursor_warning.find("{capability}") == std::string::npos,
        "#2545: qps-ploc cursor XML warning should pseudo-localize prose while preserving placeholders");
}

void test_runtime_save_restore_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.SaveToFilenameRequired",
        "Runtime.Prg.Dispatch.Error.SaveToOpenFailed",
        "Runtime.Prg.Dispatch.Error.SaveToWriteFailed",
        "Runtime.Prg.Dispatch.Error.RestoreFromFilenameRequired",
        "Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired") == "SAVE TO: filename required",
        "#2705: SAVE TO missing-filename error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToOpenFailed") == "SAVE TO: unable to open output file",
        "#2705: SAVE TO open-failure error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SaveToWriteFailed") == "SAVE TO: unable to write output file",
        "#2705: SAVE TO write-failure error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RestoreFromFilenameRequired") ==
            "RESTORE FROM: filename required",
        "#2705: RESTORE FROM missing-filename error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed") ==
            "RESTORE FROM: unable to open source file",
        "#2705: RESTORE FROM open-failure error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2705: es-419 should define every SAVE/RESTORE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2705: pt-BR should define every SAVE/RESTORE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2705: qps-ploc should define every SAVE/RESTORE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired").find("filename required") ==
            std::string::npos,
        "#2705: es-419 SAVE TO missing-filename error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed").find("unable to open source file") ==
            std::string::npos,
        "#2705: pt-BR RESTORE FROM open-failure error should not fall back to raw English");

    expect(
        pseudo.translate("Runtime.Prg.Dispatch.Error.SaveToFilenameRequired") ==
            copperfin::localization::pseudo_localize("SAVE TO: filename required"),
        "#2705: qps-ploc SAVE TO missing-filename error should match the pseudo-localization transform");
    expect(
        pseudo.translate("Runtime.Prg.Dispatch.Error.RestoreFromOpenFailed") ==
            copperfin::localization::pseudo_localize("RESTORE FROM: unable to open source file"),
        "#2705: qps-ploc RESTORE FROM open-failure error should match the pseudo-localization transform");
}

void test_runtime_file_operation_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap erase_placeholders{
        {"errorMessage", "directory not empty"},
        {"path", "fixtures/nonempty"}
    };
    const copperfin::localization::PlaceholderMap io_placeholders{{"errorMessage", "No such file or directory"}};
    const copperfin::localization::PlaceholderMap exists_placeholders{{"path", "fixtures/existing.txt"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.EraseFailed",
        "Runtime.Prg.Dispatch.Error.CopyFileFailed",
        "Runtime.Prg.Dispatch.Error.RenameFileFailed",
        "Runtime.Prg.Dispatch.Error.RenameFileTargetExists"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders) ==
            "ERASE failed: directory not empty (fixtures/nonempty)",
        "#2706: ERASE failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders) ==
            "COPY FILE failed: No such file or directory",
        "#2706: COPY FILE failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RenameFileFailed", io_placeholders) ==
            "RENAME failed: No such file or directory",
        "#2706: RENAME failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.RenameFileTargetExists", exists_placeholders) ==
            "RENAME failed: destination already exists (fixtures/existing.txt)",
        "#3703: existing-destination RENAME failure should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2706: es-419 should define every file-operation runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2706: pt-BR should define every file-operation runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2706: qps-ploc should define every file-operation runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders).find("failed:") ==
            std::string::npos,
        "#2706: es-419 ERASE failure should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders).find("failed:") ==
            std::string::npos,
        "#2706: pt-BR COPY FILE failure should not fall back to raw English");

    const std::string pseudo_erase =
        pseudo.translate("Runtime.Prg.Dispatch.Error.EraseFailed", erase_placeholders);
    expect(
        pseudo_erase.find("[!! ") == 0U &&
            pseudo_erase.find("directory not empty") != std::string::npos &&
            pseudo_erase.find("fixtures/nonempty") != std::string::npos &&
            pseudo_erase.find("ERASE failed:") == std::string::npos,
        "#2706: qps-ploc ERASE failure should pseudo-localize prose while preserving placeholders");
    const std::string pseudo_copy =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyFileFailed", io_placeholders);
    expect(
        pseudo_copy.find("[!! ") == 0U &&
            pseudo_copy.find("No such file or directory") != std::string::npos &&
            pseudo_copy.find("COPY FILE failed:") == std::string::npos,
        "#2706: qps-ploc COPY FILE failure should pseudo-localize prose while preserving the OS error text");
    const std::string pseudo_rename =
        pseudo.translate("Runtime.Prg.Dispatch.Error.RenameFileFailed", io_placeholders);
    expect(
        pseudo_rename.find("[!! ") == 0U &&
            pseudo_rename.find("No such file or directory") != std::string::npos &&
            pseudo_rename.find("RENAME failed:") == std::string::npos,
        "#2706: qps-ploc RENAME failure should pseudo-localize prose while preserving the OS error text");
    const std::string pseudo_rename_exists =
        pseudo.translate("Runtime.Prg.Dispatch.Error.RenameFileTargetExists", exists_placeholders);
    expect(
        pseudo_rename_exists.find("[!! ") == 0U &&
            pseudo_rename_exists.find("fixtures/existing.txt") != std::string::npos &&
            pseudo_rename_exists.find("destination already exists") == std::string::npos,
        "#3703: qps-ploc existing-destination RENAME failure should pseudo-localize prose while preserving the target path");
}

void test_runtime_copy_to_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap type_placeholders{{"type", "JSON"}};
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "table write failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.CopyToArrayNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.CopyToNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.CopyToSourceCursorSchemaUnavailable",
        "Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed",
        "Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed",
        "Runtime.Prg.Dispatch.Error.CopyToWriteFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToArrayNoCurrentWorkArea") ==
            "COPY TO ARRAY: no current work area",
        "#2707: COPY TO ARRAY precondition error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea") ==
            "COPY TO: no current work area",
        "#2707: COPY TO precondition error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToSourceCursorSchemaUnavailable") ==
            "COPY TO: source cursor schema is unavailable",
        "#2707: COPY TO schema-unavailable error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToNoFieldsMatchFieldsClause") ==
            "COPY TO: no fields match the FIELDS clause",
        "#2707: COPY TO empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders) ==
            "COPY TO TYPE JSON: unable to open output file",
        "#2707: COPY TO TYPE open-failure error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToTypeWriteOutputFailed", type_placeholders) ==
            "COPY TO TYPE JSON: unable to write output file",
        "#2707: COPY TO TYPE write-failure error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CopyToWriteFailed", error_placeholders) ==
            "COPY TO: table write failed",
        "#2707: COPY TO wrapper error should preserve downstream error text");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2707: es-419 should define every COPY TO runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2707: pt-BR should define every COPY TO runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2707: qps-ploc should define every COPY TO runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CopyToNoCurrentWorkArea").find("no current work area") ==
            std::string::npos,
        "#2707: es-419 COPY TO precondition error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders)
                .find("unable to open output file") == std::string::npos,
        "#2707: pt-BR COPY TO TYPE open-failure error should not fall back to raw English");

    const std::string pseudo_type =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyToTypeOpenOutputFailed", type_placeholders);
    expect(
        pseudo_type.find("[!! ") == 0U &&
            pseudo_type.find("JSON") != std::string::npos &&
            pseudo_type.find("unable to open output file") == std::string::npos,
        "#2707: qps-ploc COPY TO TYPE open-failure error should pseudo-localize prose while preserving the type");
    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CopyToWriteFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("table write failed") != std::string::npos &&
            pseudo_wrapper.find("COPY TO:") == std::string::npos,
        "#2707: qps-ploc COPY TO wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_array_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "table header parse failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromArrayFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders) ==
            "APPEND FROM ARRAY: table header parse failed",
        "#2708: APPEND FROM ARRAY wrapper error should preserve downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause") ==
            "APPEND FROM ARRAY: no fields match the FIELDS clause",
        "#2708: APPEND FROM ARRAY empty-fields error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2708: es-419 should define every APPEND FROM ARRAY runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2708: pt-BR should define every APPEND FROM ARRAY runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2708: qps-ploc should define every APPEND FROM ARRAY runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayNoFieldsMatchFieldsClause")
                .find("no fields match the FIELDS clause") == std::string::npos,
        "#2708: es-419 APPEND FROM ARRAY empty-fields error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders) ==
            "APPEND FROM ARRAY: table header parse failed",
        "#2708: pt-BR APPEND FROM ARRAY wrapper error should preserve invariant command text and downstream error text");

    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromArrayFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("table header parse failed") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM ARRAY:") == std::string::npos,
        "#2708: qps-ploc APPEND FROM ARRAY wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "open table failed"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.AppendFromNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromFailed", error_placeholders) ==
            "APPEND FROM: open table failed",
        "#2709: APPEND FROM wrapper error should preserve downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea") ==
            "APPEND FROM: no current work area",
        "#2709: APPEND FROM no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromNoFieldsMatchFieldsClause") ==
            "APPEND FROM: no fields match the FIELDS clause",
        "#2709: APPEND FROM empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType") ==
            "APPEND FROM: selected SQL/result cursor does not support this source type",
        "#2709: APPEND FROM SQL/result source-type error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2709: es-419 should define every shared APPEND FROM runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2709: pt-BR should define every shared APPEND FROM runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2709: qps-ploc should define every shared APPEND FROM runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromNoCurrentWorkArea")
                .find("no current work area") == std::string::npos,
        "#2709: es-419 APPEND FROM no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromSelectedSqlResultCursorUnsupportedSourceType")
                .find("selected SQL/result cursor does not support this source type") == std::string::npos,
        "#2709: pt-BR APPEND FROM SQL/result source-type error should not fall back to raw English");

    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromFailed", error_placeholders);
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("open table failed") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM:") == std::string::npos,
        "#2709: qps-ploc APPEND FROM wrapper error should pseudo-localize prose while preserving downstream error text");
}

void test_runtime_append_from_type_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap type_placeholders{{"type", "JSON"}};
    const copperfin::localization::PlaceholderMap error_placeholders{{"errorMessage", "numeric value too large"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AppendFromTypeFailed",
        "Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause",
        "Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        }) == "APPEND FROM TYPE JSON: numeric value too large",
        "#2711: APPEND FROM TYPE wrapper error should preserve type and downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders) ==
            "APPEND FROM TYPE JSON: unable to open source file",
        "#2710: APPEND FROM TYPE open-source error should preserve the type placeholder");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause", type_placeholders) ==
            "APPEND FROM TYPE JSON: no fields match the FIELDS clause",
        "#2710: APPEND FROM TYPE empty-fields error should preserve the type placeholder");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2710: es-419 should define every APPEND FROM TYPE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2710: pt-BR should define every APPEND FROM TYPE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2710: qps-ploc should define every APPEND FROM TYPE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders)
                .find("unable to open source file") == std::string::npos,
        "#2710: es-419 APPEND FROM TYPE open-source error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeNoFieldsMatchFieldsClause", type_placeholders)
                .find("no fields match the FIELDS clause") == std::string::npos,
        "#2710: pt-BR APPEND FROM TYPE empty-fields error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        }) == "APPEND FROM TYPE JSON: numeric value too large",
        "#2711: pt-BR APPEND FROM TYPE wrapper error should preserve invariant type and downstream error text");

    const std::string pseudo_open =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeOpenSourceFailed", type_placeholders);
    expect(
        pseudo_open.find("[!! ") == 0U &&
            pseudo_open.find("JSON") != std::string::npos &&
            pseudo_open.find("unable to open source file") == std::string::npos,
        "#2710: qps-ploc APPEND FROM TYPE open-source error should pseudo-localize prose while preserving the type");
    const std::string pseudo_wrapper =
        pseudo.translate("Runtime.Prg.Dispatch.Error.AppendFromTypeFailed", {
            {"type", "JSON"},
            {"errorMessage", "numeric value too large"},
        });
    expect(
        pseudo_wrapper.find("[!! ") == 0U &&
            pseudo_wrapper.find("JSON") != std::string::npos &&
            pseudo_wrapper.find("numeric value too large") != std::string::npos &&
            pseudo_wrapper.find("APPEND FROM TYPE JSON:") == std::string::npos,
        "#2711: qps-ploc APPEND FROM TYPE wrapper error should pseudo-localize prose while preserving type and downstream error text");
}

void test_runtime_scatter_gather_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound",
        "Runtime.Prg.Dispatch.Error.GatherNoCurrentRecord",
        "Runtime.Prg.Dispatch.Error.GatherNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.ScatterNoCurrentRecord",
        "Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea",
        "Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea") ==
            "SCATTER: no current work area",
        "#2712: SCATTER no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentRecord") ==
            "SCATTER: no current record",
        "#2712: SCATTER no-current-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause") ==
            "SCATTER: no fields match the FIELDS clause",
        "#2712: SCATTER empty-fields error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNoCurrentWorkArea") ==
            "GATHER: no current work area",
        "#2712: GATHER no-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNoCurrentRecord") ==
            "GATHER: no current record",
        "#2712: GATHER no-current-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound") ==
            "GATHER NAME: object variable not found",
        "#2712: GATHER NAME missing-object error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2712: es-419 should define every SCATTER/GATHER runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2712: pt-BR should define every SCATTER/GATHER runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2712: qps-ploc should define every SCATTER/GATHER runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ScatterNoCurrentWorkArea")
                .find("no current work area") == std::string::npos,
        "#2712: es-419 SCATTER no-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound")
                .find("object variable not found") == std::string::npos,
        "#2712: pt-BR GATHER NAME missing-object error should not fall back to raw English");

    const std::string pseudo_scatter =
        pseudo.translate("Runtime.Prg.Dispatch.Error.ScatterNoFieldsMatchFieldsClause");
    expect(
        pseudo_scatter.find("[!! ") == 0U &&
            pseudo_scatter.find("no fields match the FIELDS clause") == std::string::npos,
        "#2712: qps-ploc SCATTER empty-fields error should pseudo-localize prose");
    const std::string pseudo_gather =
        pseudo.translate("Runtime.Prg.Dispatch.Error.GatherNameObjectVariableNotFound");
    expect(
        pseudo_gather.find("[!! ") == 0U &&
            pseudo_gather.find("object variable not found") == std::string::npos,
        "#2712: qps-ploc GATHER NAME missing-object error should pseudo-localize prose");
}

void test_runtime_dispatch_array_and_object_target_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap copy_placeholders{{"command", "COPY TO ARRAY"}};
    const copperfin::localization::PlaceholderMap scatter_placeholders{{"command", "SCATTER NAME"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.ArrayNameRequired",
        "Runtime.Prg.Dispatch.Error.InvalidArrayName",
        "Runtime.Prg.Dispatch.Error.ObjectTargetRequired",
        "Runtime.Prg.Dispatch.Error.InvalidObjectTarget"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ArrayNameRequired", copy_placeholders) ==
            "COPY TO ARRAY: array name required",
        "#2722: array-name-required helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InvalidArrayName", copy_placeholders) ==
            "COPY TO ARRAY: invalid array name",
        "#2722: invalid-array-name helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ObjectTargetRequired", scatter_placeholders) ==
            "SCATTER NAME: object target required",
        "#2722: object-target-required helper error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InvalidObjectTarget", scatter_placeholders) ==
            "SCATTER NAME: invalid object target",
        "#2722: invalid-object-target helper error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2722: es-419 should define every shared array/object-target helper runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2722: pt-BR should define every shared array/object-target helper runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2722: qps-ploc should define every shared array/object-target helper runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ArrayNameRequired", copy_placeholders)
                .find("array name required") == std::string::npos,
        "#2722: es-419 array-name-required helper error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.InvalidObjectTarget", scatter_placeholders)
                .find("invalid object target") == std::string::npos,
        "#2722: pt-BR invalid-object-target helper error should not fall back to raw English");

    const std::string pseudo_object_target =
        pseudo.translate("Runtime.Prg.Dispatch.Error.ObjectTargetRequired", scatter_placeholders);
    expect(
        pseudo_object_target.find("[!! ") == 0U &&
            pseudo_object_target.find("SCATTER NAME") != std::string::npos &&
            pseudo_object_target.find("object target required") == std::string::npos,
        "#2722: qps-ploc object-target-required helper error should pseudo-localize prose while preserving the command token");
}

void test_runtime_table_structure_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap alter_add_placeholders{{"command", "ALTER TABLE ADD COLUMN"}};
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.AlterTableRequiresTargetTableName",
        "Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly",
        "Runtime.Prg.Dispatch.Error.CreateCursorRequiresNonEmptyAlias",
        "Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration",
        "Runtime.Prg.Dispatch.Error.CreateTableRequiresTargetTableName"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration") ==
            "CREATE CURSOR requires at least one supported field declaration",
        "#2713: CREATE CURSOR field-declaration error should localize through the runtime catalog");
    expect(
        english.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders) ==
            "ALTER TABLE ADD COLUMN requires a supported field declaration",
        "#2723: ALTER TABLE ADD COLUMN field-declaration error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.CreateTableRequiresTargetTableName") ==
            "CREATE TABLE requires a target table name",
        "#2713: CREATE TABLE target-name error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly") ==
            "ALTER TABLE currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only",
        "#2713: ALTER TABLE action-support error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2713: es-419 should define every table-structure runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2713: pt-BR should define every table-structure runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2713: qps-ploc should define every table-structure runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.CreateCursorRequiresSupportedFieldDeclaration")
                .find("requires at least one supported field declaration") == std::string::npos,
        "#2713: es-419 CREATE CURSOR field-declaration error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.AlterTableSupportsAddDropAlterColumnOnly")
                .find("currently supports ADD COLUMN, DROP COLUMN, and ALTER COLUMN only") == std::string::npos,
        "#2713: pt-BR ALTER TABLE action-support error should not fall back to raw English");
    expect(
        spanish.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders)
                .find("requires a supported field declaration") == std::string::npos,
        "#2723: es-419 ALTER TABLE field-declaration error should not fall back to raw English");

    const std::string pseudo_create_table =
        pseudo.translate("Runtime.Prg.Dispatch.Error.CreateTableRequiresSupportedFieldDeclaration");
    expect(
        pseudo_create_table.find("[!! ") == 0U &&
            pseudo_create_table.find("requires at least one supported field declaration") == std::string::npos,
        "#2713: qps-ploc CREATE TABLE field-declaration error should pseudo-localize prose");
    const std::string pseudo_alter_add =
        pseudo.translate(
            "Runtime.Prg.Dispatch.Error.AlterTableRequiresSupportedFieldDeclaration",
            alter_add_placeholders);
    expect(
        pseudo_alter_add.find("[!! ") == 0U &&
            pseudo_alter_add.find("ALTER TABLE ADD COLUMN") != std::string::npos &&
            pseudo_alter_add.find("requires a supported field declaration") == std::string::npos,
        "#2723: qps-ploc ALTER TABLE field-declaration error should pseudo-localize prose while preserving command tokens");
}

void test_runtime_set_filter_dimension_sleep_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions",
        "Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea",
        "Runtime.Prg.Dispatch.Error.SleepInvalidDuration"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea") ==
            "SET FILTER requires a selected work area",
        "#2714: SET FILTER selected-work-area error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions") ==
            "DIMENSION/DECLARE requires array dimensions",
        "#2714: DIMENSION/DECLARE array-dimensions error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SleepInvalidDuration") ==
            "SLEEP: invalid duration",
        "#2714: SLEEP invalid-duration error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2714: es-419 should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2714: pt-BR should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2714: qps-ploc should define every SET FILTER/DIMENSION/SLEEP runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.SetFilterRequiresSelectedWorkArea")
                .find("requires a selected work area") == std::string::npos,
        "#2714: es-419 SET FILTER selected-work-area error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.SleepInvalidDuration")
                .find("invalid duration") == std::string::npos,
        "#2714: pt-BR SLEEP invalid-duration error should not fall back to raw English");

    const std::string pseudo_dimension =
        pseudo.translate("Runtime.Prg.Dispatch.Error.DimensionDeclareRequiresArrayDimensions");
    expect(
        pseudo_dimension.find("[!! ") == 0U &&
            pseudo_dimension.find("requires array dimensions") == std::string::npos,
        "#2714: qps-ploc DIMENSION/DECLARE error should pseudo-localize prose");
}

void test_runtime_declare_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap load_placeholders{
        {"path", "kernel32.dll"},
        {"errorMessage", "Access is denied."}
    };
    const copperfin::localization::PlaceholderMap function_placeholders{
        {"functionName", "MissingSymbol"},
        {"path", "kernel32.dll"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll",
        "Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows",
        "Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll",
        "Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath") ==
            "DECLARE: missing function name or DLL path.",
        "#2715: DECLARE missing-name/path error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll", load_placeholders) ==
            "DECLARE: cannot load 'kernel32.dll': Access is denied.",
        "#2715: DECLARE load-failure error should preserve path and downstream error text");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareFunctionNotFoundInDll", function_placeholders) ==
            "DECLARE: function 'MissingSymbol' not found in 'kernel32.dll'.",
        "#2715: DECLARE function-not-found error should preserve function and path placeholders");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows") ==
            "DECLARE DLL is only supported on Windows.",
        "#2715: DECLARE Windows-only guard should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2715: es-419 should define every DECLARE runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2715: pt-BR should define every DECLARE runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2715: qps-ploc should define every DECLARE runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.DeclareMissingFunctionNameOrDllPath")
                .find("missing function name or DLL path") == std::string::npos,
        "#2715: es-419 DECLARE missing-name/path error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.DeclareDllOnlySupportedOnWindows")
                .find("only supported on Windows") == std::string::npos,
        "#2715: pt-BR DECLARE Windows-only guard should not fall back to raw English");

    const std::string pseudo_load =
        pseudo.translate("Runtime.Prg.Dispatch.Error.DeclareCannotLoadDll", load_placeholders);
    expect(
        pseudo_load.find("[!! ") == 0U &&
            pseudo_load.find("kernel32.dll") != std::string::npos &&
            pseudo_load.find("Access is denied.") != std::string::npos &&
            pseudo_load.find("cannot load") == std::string::npos,
        "#2715: qps-ploc DECLARE load-failure error should pseudo-localize prose while preserving path and downstream error text");
}

void test_runtime_residual_command_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice",
        "Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry",
        "Runtime.Prg.Dispatch.Error.ReplaceRequiresFieldWithExpressionAssignment",
        "Runtime.Prg.Dispatch.Error.UpdateRequiresSetFieldExpressionAssignments",
        "Runtime.Prg.Dispatch.Error.InsertIntoRequiresValuesClause",
        "Runtime.Prg.Dispatch.Error.UnlockRecordTargetRecordNotFound",
        "Runtime.Prg.Dispatch.Error.SleepCancelled"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice") ==
            "TEXT requires TO <variable> in the current runtime slice",
        "#2717: TEXT missing-target error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry") ==
            "TRY block is missing ENDTRY",
        "#2717: TRY missing-ENDTRY error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ReplaceRequiresFieldWithExpressionAssignment") ==
            "REPLACE requires at least one FIELD WITH expression assignment",
        "#2717: REPLACE assignment error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UpdateRequiresSetFieldExpressionAssignments") ==
            "UPDATE requires SET field = expression assignments",
        "#2717: UPDATE assignment error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.InsertIntoRequiresValuesClause") ==
            "INSERT INTO requires a VALUES clause",
        "#2717: INSERT INTO VALUES-clause error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.UnlockRecordTargetRecordNotFound") ==
            "UNLOCK RECORD target record not found",
        "#2717: UNLOCK RECORD target-record error should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.SleepCancelled") ==
            "SLEEP cancelled.",
        "#2717: SLEEP cancellation error should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2717: es-419 should define every residual command runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2717: pt-BR should define every residual command runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2717: qps-ploc should define every residual command runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.TryBlockMissingEndtry").find("missing ENDTRY") ==
            std::string::npos,
        "#2717: es-419 TRY missing-ENDTRY error should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.SleepCancelled").find("cancelled") ==
            std::string::npos,
        "#2717: pt-BR SLEEP cancellation error should not fall back to raw English");

    const std::string pseudo_text =
        pseudo.translate("Runtime.Prg.Dispatch.Error.TextRequiresToVariableInCurrentRuntimeSlice");
    expect(
        pseudo_text.find("[!! ") == 0U &&
            pseudo_text.find("TEXT requires TO <variable> in the current runtime slice") == std::string::npos &&
            pseudo_text.find("<") != std::string::npos,
        "#2717: qps-ploc TEXT missing-target error should pseudo-localize prose while preserving syntax markers");
}

void test_runtime_object_helper_dispatch_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap ole_placeholders{
        {"targetIdentifier", "missingOle.SomeProperty"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed",
        "Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment",
        "Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject"
    };

    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed") ==
            "Object target assignment failed",
        "#2718: object-target assignment failure should localize through the runtime catalog");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment", ole_placeholders) ==
            "OLE object not found for property assignment: missingOle.SomeProperty",
        "#2718: OLE property-assignment miss should preserve the failing target identifier");
    expect(
        english.translate("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject") ==
            "SCATTER NAME: unable to create object",
        "#2718: SCATTER NAME object-creation failure should localize through the runtime catalog");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2718: es-419 should define every object-helper runtime dispatch key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2718: pt-BR should define every object-helper runtime dispatch key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2718: qps-ploc should define every object-helper runtime dispatch key");
    }

    expect(
        spanish.translate("Runtime.Prg.Dispatch.Error.ObjectTargetAssignmentFailed")
                .find("Object target assignment failed") == std::string::npos,
        "#2718: es-419 object-target assignment failure should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Dispatch.Error.ScatterNameUnableToCreateObject")
                .find("unable to create object") == std::string::npos,
        "#2718: pt-BR SCATTER NAME object-creation failure should not fall back to raw English");

    const std::string pseudo_ole =
        pseudo.translate("Runtime.Prg.Dispatch.Error.OleObjectNotFoundForPropertyAssignment", ole_placeholders);
    expect(
        pseudo_ole.find("[!! ") == 0U &&
            pseudo_ole.find("missingOle.SomeProperty") != std::string::npos &&
            pseudo_ole.find("OLE object not found for property assignment") == std::string::npos,
        "#2718: qps-ploc OLE property-assignment miss should pseudo-localize prose while preserving the member path");
}

void test_runtime_ole_invocation_and_property_read_errors_route_through_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap missing_method_placeholders{
        {"targetIdentifier", "missingOle.NoSuchMethod"}
    };
    const copperfin::localization::PlaceholderMap missing_member_method_placeholders{
        {"memberIdentifier", "Scripting.Dictionary.NoSuchMethod"}
    };
    const copperfin::localization::PlaceholderMap missing_property_placeholders{
        {"propertyPath", "missingOle.SomeProperty"}
    };
    const copperfin::localization::PlaceholderMap missing_member_property_placeholders{
        {"memberIdentifier", "Scripting.Dictionary.SomeProperty"}
    };
    const std::vector<std::string> keys{
        "Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation",
        "Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation",
        "Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead",
        "Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead"
    };

    expect(
        english.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders) ==
            "OLE object not found for method invocation: missingOle.NoSuchMethod",
        "#2719: OLE object-missing method invocation fault should preserve the missing target identifier");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForMethodInvocation", missing_member_method_placeholders) ==
            "OLE member not found for method invocation: Scripting.Dictionary.NoSuchMethod",
        "#2719: OLE member-missing method invocation fault should preserve the automation member identifier");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForPropertyRead", missing_property_placeholders) ==
            "OLE object not found for property read: missingOle.SomeProperty",
        "#2719: OLE object-missing property-read fault should preserve the missing property path");
    expect(
        english.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders) ==
            "OLE member not found for property read: Scripting.Dictionary.SomeProperty",
        "#2719: OLE member-missing property-read fault should preserve the automation member identifier");

    for (const std::string& key : keys) {
        expect(
            spanish.catalogs.contains("es-419") && spanish.catalogs.at("es-419").contains(key),
            "#2719: es-419 should define every residual OLE invocation/read runtime key");
        expect(
            portuguese.catalogs.contains("pt-BR") && portuguese.catalogs.at("pt-BR").contains(key),
            "#2719: pt-BR should define every residual OLE invocation/read runtime key");
        expect(
            pseudo.catalogs.contains("qps-ploc") && pseudo.catalogs.at("qps-ploc").contains(key),
            "#2719: qps-ploc should define every residual OLE invocation/read runtime key");
    }

    expect(
        spanish.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders)
                .find("OLE object not found for method invocation") == std::string::npos,
        "#2719: es-419 OLE object-missing method invocation fault should not fall back to raw English");
    expect(
        portuguese.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders)
                .find("OLE member not found for property read") == std::string::npos,
        "#2719: pt-BR OLE member-missing property-read fault should not fall back to raw English");

    const std::string pseudo_missing_method =
        pseudo.translate("Runtime.Prg.Core.Error.OleObjectNotFoundForMethodInvocation", missing_method_placeholders);
    expect(
        pseudo_missing_method.find("[!! ") == 0U &&
            pseudo_missing_method.find("missingOle.NoSuchMethod") != std::string::npos &&
            pseudo_missing_method.find("OLE object not found for method invocation") == std::string::npos,
        "#2719: qps-ploc OLE object-missing method invocation fault should pseudo-localize prose while preserving the target identifier");

    const std::string pseudo_missing_property =
        pseudo.translate("Runtime.Prg.Core.Error.OleMemberNotFoundForPropertyRead", missing_member_property_placeholders);
    expect(
        pseudo_missing_property.find("[!! ") == 0U &&
            pseudo_missing_property.find("Scripting.Dictionary.SomeProperty") != std::string::npos &&
            pseudo_missing_property.find("OLE member not found for property read") == std::string::npos,
        "#2719: qps-ploc OLE member-missing property-read fault should pseudo-localize prose while preserving the member identifier");
}

void test_inspect_usage_routes_through_localization(const std::string& inspect_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_inspect_usage_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    seed_test_catalogs(temp_root);

    set_env_value("COPPERFIN_LOCALE_DIR", temp_root.string(), true);
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const std::string output = run_command_capture(shell_quote(inspect_path) + " 2>&1");
    set_env_value("COPPERFIN_LOCALE", "", false);
    set_env_value("COPPERFIN_LOCALE_DIR", "", false);

    expect(
        output.find("[!! ") != std::string::npos &&
            output.find("copperfin_inspect") != std::string::npos &&
            output.find("--locale") != std::string::npos,
        "#1779: copperfin_inspect usage text should route through localization while preserving CLI tokens");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_package_warnings_pseudo_localize() {
    namespace fs = std::filesystem;

    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    set_env_value("COPPERFIN_LOCALE_DIR", "", false);

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_package_warning_localization";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "runtime_warning_localization.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "RuntimeWarningLocalization";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "RuntimeWarningLocalization";
    workspace.build_plan.output_path = (output_dir / "RuntimeWarningLocalization.exe").string();
    workspace.build_plan.startup_item = "main.prg";
    workspace.build_plan.startup_record_index = 42U;
    workspace.entries = {
        {.record_index = 1U, .name = "missing.prg", .relative_path = "missing.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        false);

    expect(plan.ok, "#2561: qps-ploc runtime package plan should still be created when warning paths trigger");

    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const std::string missing_asset_warning = pseudo_catalog.translate(
        "Runtime.Package.Warning.MissingProjectAsset",
        {{"path", (project_dir / "missing.prg").string()}});
    const std::string startup_warning =
        pseudo_catalog.translate("Runtime.Package.Warning.StartupSourceUnresolved");
    const std::string debug_startup_warning =
        pseudo_catalog.translate("Runtime.Package.Warning.DebugStartupSourceUnresolved");

    expect(std::find(plan.warnings.begin(), plan.warnings.end(), missing_asset_warning) != plan.warnings.end(),
        "#2561: qps-ploc runtime package warnings should pseudo-localize missing-asset prose while preserving paths");
    expect(std::find(plan.warnings.begin(), plan.warnings.end(), startup_warning) != plan.warnings.end(),
        "#2561: qps-ploc runtime package warnings should pseudo-localize startup-resolution prose");
    expect(std::find(plan.warnings.begin(), plan.warnings.end(), debug_startup_warning) != plan.warnings.end(),
        "#2561: qps-ploc runtime package warnings should pseudo-localize debug startup-resolution prose");
    expect(missing_asset_warning.find("[!! ") == 0U,
        "#2561: qps-ploc runtime package missing-asset warning should decorate human-facing prose");

    fs::remove_all(temp_root, ignored);
}

void test_inspect_error_prefix_routes_through_localization(const std::string& inspect_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_inspect_error_prefix_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    set_env_value("COPPERFIN_LOCALE_DIR", "", false);
    const std::string missing_asset = (temp_root / "missing.dbf").string();
    const std::string output = run_command_capture(
        shell_quote(inspect_path) + " " + shell_quote(missing_asset) + " 2>&1");
    set_env_value("COPPERFIN_LOCALE", "", false);
    set_env_value("COPPERFIN_LOCALE_DIR", "", false);

    const auto pseudo_catalog = copperfin::localization::load_catalogs(
        copperfin::localization::resolve_catalog_root(),
        "qps-ploc");
    const std::string pseudo_error_prefix = pseudo_catalog.translate("Inspect.Prefix.Error");

    expect(
        output.find("status: error") != std::string::npos,
        "#2565: inspect error-prefix localization should preserve machine-readable error status");
    expect(
        output.find(pseudo_error_prefix) != std::string::npos,
        "#2565: qps-ploc inspect failures should route the error prefix through localization");
    expect(
        output.find("[!! ") != std::string::npos,
        "#2565: qps-ploc inspect failures should decorate human-facing prose");
    expect(
        output.find("\nerror: ") == std::string::npos,
        "#2565: qps-ploc inspect failures should not fall back to the raw English error prefix");

    fs::remove_all(temp_root, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    test_catalog_loading_and_fallback();
    test_placeholders_pseudo_locale_and_unicode();
    test_catalog_json_unicode_escapes_support_surrogate_pairs();
    test_machine_contract_fields_remain_invariant();
    test_catalog_root_resolution_searches_parent_directories();
    test_catalog_root_resolution_finds_repo_build_output_layout_from_executable_path();
    test_catalog_root_resolution_finds_repo_build_output_layout_from_path_launched_basename();
    test_parser_behavior_remains_locale_invariant();
    test_runtime_session_diagnostics_route_through_catalog();
    test_runtime_cursor_diagnostics_route_through_catalog();
    test_runtime_total_diagnostics_route_through_catalog();
    test_runtime_report_output_messages_route_through_catalog();
    test_runtime_report_output_errors_localize_without_changing_runtime_behavior();
    test_runtime_aggregate_errors_route_through_catalog();
    test_runtime_sql_errors_route_through_catalog();
    test_build_host_catalog_entries_cover_placeholder_locales();
    test_inspect_catalog_entries_cover_placeholder_locales();
    test_shared_core_catalog_entries_cover_placeholder_locales();
    test_runtime_host_manifest_verification_errors_route_through_catalog();
    test_runtime_host_quit_prompt_routes_through_catalog();
    test_runtime_host_security_policy_denial_routes_through_catalog();
    test_platform_federation_ai_planner_fallback_routes_through_catalog();
    test_runtime_numeric_domain_errors_route_through_catalog();
    test_runtime_expression_errors_route_through_catalog();
    test_runtime_record_precondition_errors_route_through_catalog();
    test_runtime_dll_errors_route_through_catalog();
    test_runtime_core_errors_route_through_catalog();
    test_runtime_pause_and_session_messages_route_through_catalog();
    test_runtime_watch_errors_route_through_catalog();
    test_runtime_dispatch_errors_route_through_catalog();
    test_runtime_surface_errors_route_through_catalog();
    test_runtime_save_restore_errors_route_through_catalog();
    test_runtime_file_operation_errors_route_through_catalog();
    test_runtime_copy_to_errors_route_through_catalog();
    test_runtime_append_from_array_errors_route_through_catalog();
    test_runtime_append_from_errors_route_through_catalog();
    test_runtime_append_from_type_errors_route_through_catalog();
    test_runtime_scatter_gather_errors_route_through_catalog();
    test_runtime_dispatch_array_and_object_target_errors_route_through_catalog();
    test_runtime_table_structure_errors_route_through_catalog();
    test_runtime_set_filter_dimension_sleep_errors_route_through_catalog();
    test_runtime_declare_dispatch_errors_route_through_catalog();
    test_runtime_residual_command_dispatch_errors_route_through_catalog();
    test_runtime_object_helper_dispatch_errors_route_through_catalog();
    test_runtime_ole_invocation_and_property_read_errors_route_through_catalog();
    test_runtime_package_warnings_pseudo_localize();
    if (argc > 1) {
        test_inspect_usage_routes_through_localization(argv[1]);
        test_inspect_error_prefix_routes_through_localization(argv[1]);
    } else {
        expect(false, "#1779: test_localization requires the copperfin_inspect executable path");
    }

    return test_failures() == 0 ? 0 : 1;
}
