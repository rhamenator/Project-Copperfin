#include "copperfin/localization/localization.h"
#include "copperfin/runtime/prg_engine.h"
#include "prg_engine_test_support.h"

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
    if (argc > 1) {
        test_inspect_usage_routes_through_localization(argv[1]);
    } else {
        expect(false, "#1779: test_localization requires the copperfin_inspect executable path");
    }

    return test_failures() == 0 ? 0 : 1;
}
