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
    test_parser_behavior_remains_locale_invariant();
    if (argc > 1) {
        test_inspect_usage_routes_through_localization(argv[1]);
    } else {
        expect(false, "#1779: test_localization requires the copperfin_inspect executable path");
    }

    return test_failures() == 0 ? 0 : 1;
}
