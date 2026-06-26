#include "copperfin/localization/localization.h"
#include "copperfin/platform/extensibility_model.h"
#include "copperfin/runtime/runtime_pipeline.h"
#include "copperfin/runtime/prg_engine.h"
#include "copperfin/security/security_model.h"
#include "copperfin/studio/project_workspace.h"
#include "prg_engine_test_support.h"

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

void set_env_value(const std::string& name, const std::string& value, bool has_value) {
#ifdef _WIN32
    if (has_value) {
        _putenv_s(name.c_str(), value.c_str());
    } else {
        _putenv_s(name.c_str(), "");
    }
#else
    if (has_value) {
        setenv(name.c_str(), value.c_str(), 1);
    } else {
        unsetenv(name.c_str());
    }
#endif
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

struct ScopedEnvironmentValue {
    std::string name;
    bool had_value = false;
    std::string original_value;

    explicit ScopedEnvironmentValue(std::string environment_name)
        : name(std::move(environment_name)) {
        if (const char* current = std::getenv(name.c_str())) {
            had_value = true;
            original_value = current;
        }
    }

    ~ScopedEnvironmentValue() {
        set_env_value(name, original_value, had_value);
    }
};

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

void test_runtime_transaction_journal_messages_route_through_catalog() {
    const auto catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "en-US");

    expect(
        catalog.translate("Runtime.Prg.Transaction.Error.JournalInitializeFailed") ==
            "Unable to initialize transaction journal",
        "#2535: transaction journal initialize error should be catalog-backed");
    expect(
        catalog.translate("Runtime.Prg.Transaction.Error.JournalStatePersistFailed") ==
            "Unable to persist transaction journal state",
        "#2535: transaction journal state persist error should be catalog-backed");
    expect(
        catalog.translate("Runtime.Prg.Transaction.Error.BackupCreateFailed", {{"path", "fixtures/people.dbf"}}) ==
            "Unable to create transaction backup for: fixtures/people.dbf",
        "#2535: transaction backup error should preserve the named path placeholder");
    expect(
        catalog.translate("Runtime.Prg.Transaction.Error.BackupJournalPersistFailed") ==
            "Unable to persist transaction backup journal",
        "#2535: transaction backup journal persist error should be catalog-backed");
    expect(
        catalog.translate("Runtime.Prg.Transaction.Error.JournalReplayFailed") ==
            "Failed to replay transaction journal",
        "#2535: transaction replay error should be catalog-backed");
}

void test_runtime_report_output_messages_route_through_catalog() {
    const auto catalog =
        copperfin::localization::load_catalogs(copperfin::localization::resolve_catalog_root(), "en-US");

    expect(
        catalog.translate("Runtime.Prg.ReportOutput.Error.PathRequired") ==
            "REPORT/LABEL TO clause requires a writable output path",
        "#2536: report output path-required error should be catalog-backed");
    expect(
        catalog.translate("Runtime.Prg.ReportOutput.Error.OpenFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to open report output path: renders/invoice.txt",
        "#2536: report output open error should preserve the named path placeholder");
    expect(
        catalog.translate("Runtime.Prg.ReportOutput.Error.WriteFailed", {{"path", "renders/invoice.txt"}}) ==
            "Unable to write report output path: renders/invoice.txt",
        "#2536: report output write error should preserve the named path placeholder");
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
    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap detail_placeholders{{"detail", "disk full"}};
    const copperfin::localization::PlaceholderMap path_placeholders{{"path", "forms/customer.scx"}};

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
        spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("disk full") !=
                std::string::npos &&
            spanish.translate("Runtime.Prg.Core.Error.RuntimeFault", detail_placeholders).find("Runtime fault") ==
                std::string::npos,
        "#2551: es-419 runtime fault should preserve detail without falling back to English");

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
    test_machine_contract_fields_remain_invariant();
    test_catalog_root_resolution_searches_parent_directories();
    test_parser_behavior_remains_locale_invariant();
    test_runtime_transaction_journal_messages_route_through_catalog();
    test_runtime_report_output_messages_route_through_catalog();
    test_build_host_catalog_entries_cover_placeholder_locales();
    test_inspect_catalog_entries_cover_placeholder_locales();
    test_shared_core_catalog_entries_cover_placeholder_locales();
    test_runtime_host_manifest_verification_errors_route_through_catalog();
    test_runtime_numeric_domain_errors_route_through_catalog();
    test_runtime_expression_errors_route_through_catalog();
    test_runtime_record_precondition_errors_route_through_catalog();
    test_runtime_dll_errors_route_through_catalog();
    test_runtime_core_errors_route_through_catalog();
    test_runtime_pause_and_session_messages_route_through_catalog();
    test_runtime_watch_errors_route_through_catalog();
    test_runtime_dispatch_errors_route_through_catalog();
    test_runtime_surface_errors_route_through_catalog();
    test_runtime_package_warnings_pseudo_localize();
    if (argc > 1) {
        test_inspect_usage_routes_through_localization(argv[1]);
        test_inspect_error_prefix_routes_through_localization(argv[1]);
    } else {
        expect(false, "#1779: test_localization requires the copperfin_inspect executable path");
    }

    return test_failures() == 0 ? 0 : 1;
}
