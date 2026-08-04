// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_localization_support.h"

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

void test_catalog_file_accepts_one_leading_utf8_bom() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_bom_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const std::string bom = "\xEF\xBB\xBF";
    write_catalog(
        temp_root,
        "en-US",
        "{\"Fallback.Message\":\"English fallback\"}\n");
    write_catalog(
        temp_root,
        "es-419",
        bom + "{\"Localized.Message\":\"Espa\xC3\xB1ol {name}\"}\n");

    const auto direct = copperfin::localization::load_catalog_file(
        temp_root / "es-419" / "strings.json");
    expect(
        direct.ok && direct.entries.contains("Localized.Message") &&
            direct.entries.at("Localized.Message") == "Espa\xC3\xB1ol {name}",
        "#3893: catalog file loading should accept one leading UTF-8 BOM and preserve UTF-8 values");

    const auto spanish_mexico = copperfin::localization::load_catalogs(temp_root, "es-MX");
    expect(
        spanish_mexico.translate("Localized.Message", {{"name", "Ana"}}) ==
            "Espa\xC3\xB1ol Ana" &&
            spanish_mexico.translate("Fallback.Message") == "English fallback",
        "#3893: a BOM-prefixed regional catalog should participate in normal locale fallback");

    write_catalog(temp_root, "double-bom", bom + bom + "{}\n");
    const auto double_bom = copperfin::localization::load_catalog_file(
        temp_root / "double-bom" / "strings.json");
    expect(
        !double_bom.ok && double_bom.error == "Catalog.Json.ExpectedObject",
        "#3893: catalog file loading should not accept more than one leading UTF-8 BOM");

    write_catalog(temp_root, "indented-bom", " " + bom + "{}\n");
    const auto indented_bom = copperfin::localization::load_catalog_file(
        temp_root / "indented-bom" / "strings.json");
    expect(
        !indented_bom.ok && indented_bom.error == "Catalog.Json.ExpectedObject",
        "#3893: a UTF-8 BOM should only be accepted at the first file byte");

    write_catalog(temp_root, "trailing-bom", "{}" + bom);
    const auto trailing_bom = copperfin::localization::load_catalog_file(
        temp_root / "trailing-bom" / "strings.json");
    expect(
        !trailing_bom.ok && trailing_bom.error == "Catalog.Json.TrailingContent",
        "#3893: a trailing UTF-8 BOM should remain invalid JSON content");

    const auto parser_result = copperfin::localization::parse_catalog_json(bom + "{}");
    expect(
        !parser_result.ok && parser_result.error == "Catalog.Json.ExpectedObject",
        "#3893: the JSON parser should remain strict when it is called directly");

    fs::remove_all(temp_root, ignored);
}

void test_bcp47_script_locale_normalization_and_catalog_fallback() {
    namespace fs = std::filesystem;
    using copperfin::localization::locale_fallback_chain;
    using copperfin::localization::normalize_locale;

    expect(
        normalize_locale("ZH_hANT_tw") == "zh-Hant-TW" &&
            normalize_locale("SR_lATN") == "sr-Latn" &&
            normalize_locale("zh-cmn-hANS-cn") == "zh-cmn-Hans-CN",
        "#3894: language, script, extlang, and region subtags should use canonical BCP 47 casing");
    expect(
        normalize_locale("sl-ROZAJ-BISKE") == "sl-rozaj-biske" &&
            normalize_locale("en-latn-us-u-CA-GREGORY-x-PRIVATE") ==
                "en-Latn-US-u-ca-gregory-x-private",
        "#3894: variants, extensions, and private-use subtags should remain deterministically lowercase");
    expect(
        normalize_locale("qPs-pLoC") == "qps-ploc" &&
            normalize_locale("ES_419") == "es-419" &&
            normalize_locale("PT_br") == "pt-BR",
        "#3894: canonicalization should preserve pseudo-locale and existing regional directory contracts");

    const std::vector<std::string> script_chain = locale_fallback_chain("ZH_hANT_tw");
    expect(
        script_chain == std::vector<std::string>({"zh-Hant-TW", "zh-Hant", "zh", "en-US"}),
        "#3894: a script-plus-region locale should retain its script parent before language fallback");
    const std::vector<std::string> script_only_chain = locale_fallback_chain("SR_lATN");
    expect(
        script_only_chain == std::vector<std::string>({"sr-Latn", "sr", "en-US"}),
        "#3894: a script-only locale should fall back through its language and the source locale");
    const std::vector<std::string> spanish_script_chain = locale_fallback_chain("ES_lATN_mx");
    expect(
        spanish_script_chain ==
            std::vector<std::string>({"es-Latn-MX", "es-Latn", "es-419", "es", "en-US"}),
        "#3894: script-qualified Spanish should preserve the Latin American fallback contract");
    const std::vector<std::string> pseudo_chain = locale_fallback_chain("qPs-pLoC");
    expect(
        pseudo_chain == std::vector<std::string>({"qps-ploc", "qps", "en-US"}),
        "#3894: pseudo-locale fallback should preserve its established normalized identity");
    const std::vector<std::string> extension_chain =
        locale_fallback_chain("de-Latn-DE-u-co-phonebk");
    expect(
        extension_chain == std::vector<std::string>({
            "de-Latn-DE-u-co-phonebk",
            "de-Latn-DE-u-co",
            "de-Latn-DE",
            "de-Latn",
            "de",
            "en-US"}),
        "#3894: extension fallback should truncate deterministically without retaining a lone singleton");

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_script_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    write_catalog(
        temp_root,
        "en-US",
        "{\"Fallback.Message\":\"English fallback\"}\n");
    write_catalog(
        temp_root,
        "zh-Hant",
        "{\"Script.Message\":\"Traditional script parent\"}\n");
    write_catalog(
        temp_root,
        "zh-Hant-TW",
        "{\"Regional.Message\":\"Taiwan exact locale\"}\n");
    write_catalog(
        temp_root,
        "sr-Latn",
        "{\"Script.Message\":\"Serbian Latin exact locale\"}\n");

    const auto traditional_taiwan =
        copperfin::localization::load_catalogs(temp_root, "ZH_hANT_tw");
    expect(
        traditional_taiwan.requested_locale == "zh-Hant-TW" &&
            traditional_taiwan.catalogs.contains("zh-Hant-TW") &&
            traditional_taiwan.catalogs.contains("zh-Hant") &&
            traditional_taiwan.translate("Regional.Message") == "Taiwan exact locale" &&
            traditional_taiwan.translate("Script.Message") == "Traditional script parent" &&
            traditional_taiwan.translate("Fallback.Message") == "English fallback",
        "#3894: canonical script-plus-region directories should load with parent and source fallback");
    const auto serbian_latin = copperfin::localization::load_catalogs(temp_root, "SR_lATN");
    expect(
        serbian_latin.requested_locale == "sr-Latn" &&
            serbian_latin.catalogs.contains("sr-Latn") &&
            serbian_latin.translate("Script.Message") == "Serbian Latin exact locale" &&
            serbian_latin.translate("Fallback.Message") == "English fallback",
        "#3894: canonical script-only directories should load on case-sensitive filesystems");

    fs::remove_all(temp_root, ignored);
}

void test_posix_locale_suffixes_normalize_before_catalog_fallback() {
    expect(
        copperfin::localization::normalize_locale("pt-BR.UTF-8") == "pt-BR",
        "#3915: POSIX codeset suffix should not become part of the regional locale identifier");
    expect(
        copperfin::localization::normalize_locale("pt_BR@latin") == "pt-BR",
        "#3915: POSIX modifier suffix should be stripped before underscore normalization");
    expect(
        copperfin::localization::normalize_locale(" pt_BR.UTF-8@latin ") == "pt-BR",
        "#3915: combined POSIX codeset/modifier suffixes should normalize deterministically");
    expect(
        copperfin::localization::normalize_locale(".UTF-8") == "en-US" &&
            copperfin::localization::normalize_locale("@latin") == "en-US",
        "#3915: suffix-only malformed locale values should fall back to the default locale");

    const auto portuguese_chain = copperfin::localization::locale_fallback_chain("pt_BR.UTF-8@latin");
    expect(
        portuguese_chain.size() >= 3U && portuguese_chain[0] == "pt-BR" &&
            portuguese_chain[1] == "pt" && portuguese_chain.back() == "en-US",
        "#3915: normalized regional locale should preserve the existing language/default fallback order");
    const auto spanish_chain = copperfin::localization::locale_fallback_chain("es_MX.UTF-8");
    expect(
        spanish_chain.size() >= 4U && spanish_chain[0] == "es-MX" &&
            spanish_chain[1] == "es-419" && spanish_chain[2] == "es" &&
            spanish_chain.back() == "en-US",
        "#3915: POSIX suffix removal should preserve the Spanish regional fallback contract");
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
        copperfin::localization::pseudo_localize("[!! already pseudo-localized !!]") ==
            "[!! already pseudo-localized !!]",
        "#4378: pseudo-localization should not wrap an already decorated catalog value twice");

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

void test_catalog_json_rejects_literal_string_control_characters() {
    for (const std::string& malformed : std::vector<std::string>{
             "{\"key\":\"line1\nline2\"}",
             "{\"key\":\"left\tright\"}",
             std::string{"{\"key\":\"before"} + '\0' + "after\"}"}) {
        const auto parsed = copperfin::localization::parse_catalog_json(malformed);
        expect(!parsed.ok && parsed.error == "Catalog.Json.InvalidControlCharacter",
               "#3908: literal control bytes inside catalog JSON strings should be rejected");
    }

    const auto escaped = copperfin::localization::parse_catalog_json(
        "{\"key\":\"\\b\\f\\n\\r\\t\\u0001\"}");
    expect(escaped.ok && escaped.entries.contains("key") &&
               escaped.entries.at("key") == std::string{"\b\f\n\r\t\x01"},
           "#3908: legal JSON control escapes should remain accepted and decoded");
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

std::vector<std::string> placeholder_names(std::string_view pattern) {
    std::vector<std::string> names;
    std::size_t offset = 0U;
    while (offset < pattern.size()) {
        const std::size_t open = pattern.find('{', offset);
        if (open == std::string_view::npos) {
            break;
        }
        const std::size_t close = pattern.find('}', open + 1U);
        if (close == std::string_view::npos) {
            break;
        }
        names.emplace_back(pattern.substr(open + 1U, close - open - 1U));
        offset = close + 1U;
    }
    std::sort(names.begin(), names.end());
    return names;
}

void test_product_locale_catalogs_have_key_parity() {
    namespace fs = std::filesystem;
    const fs::path catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english = copperfin::localization::load_catalog_file(
        catalog_root / "en-US" / "strings.json");
    expect(english.ok, "product en-US localization catalog should parse");
    if (!english.ok) {
        return;
    }

    for (const std::string locale : {"es-419", "pt-BR", "qps-ploc"}) {
        const auto localized = copperfin::localization::load_catalog_file(
            catalog_root / locale / "strings.json");
        expect(localized.ok, "product " + locale + " localization catalog should parse");
        if (!localized.ok) {
            continue;
        }
        expect(
            localized.entries.size() == english.entries.size(),
            "product " + locale + " catalog should contain the same number of keys as en-US");
        for (const auto& entry : english.entries) {
            const std::string& key = entry.first;
            expect(
                localized.entries.contains(key),
                "product " + locale + " catalog should contain en-US key " + key);
            const auto localized_entry = localized.entries.find(key);
            if (localized_entry != localized.entries.end()) {
                expect(
                    std::any_of(
                        localized_entry->second.begin(),
                        localized_entry->second.end(),
                        [](const char value) {
                            return std::isspace(static_cast<unsigned char>(value)) == 0;
                        }),
                    "product " + locale + " catalog value should not be blank for key " + key);
                expect(
                    placeholder_names(entry.second) == placeholder_names(localized_entry->second),
                    "product " + locale + " catalog should preserve placeholders for key " + key);
            }
        }
        for (const auto& entry : localized.entries) {
            const std::string& key = entry.first;
            expect(
                english.entries.contains(key),
                "product " + locale + " catalog should not add a key absent from en-US: " + key);
        }
    }

    const auto pseudo = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const std::string rushmore_status =
        pseudo.translate("StudioHost.RushmoreExplain.Status.Ok");
    expect(
        rushmore_status == "[!! šţåţµš: øƙ !!]" &&
            rushmore_status.find("[!! ", 4U) == std::string::npos,
        "#4378: pre-decorated qps-ploc Rushmore entries should not receive nested markers");
    for (const auto& entry : english.entries) {
        copperfin::localization::PlaceholderMap placeholders;
        for (const std::string& name : placeholder_names(entry.second)) {
            placeholders.emplace(name, "PLACEHOLDER_" + name);
        }
        const std::string localized = pseudo.translate(entry.first, placeholders);
        expect(
            localized.starts_with("[!! ") && localized.ends_with(" !!]"),
            "qps-ploc should pseudo-localize catalog key " + entry.first);
        for (const auto& placeholder : placeholders) {
            expect(
                localized.find(placeholder.second) != std::string::npos,
                "qps-ploc should preserve placeholder value for key " + entry.first);
        }
    }
}

void test_catalog_root_auto_discovery_skips_invalid_candidates() {
    namespace fs = std::filesystem;
    ScopedTempDirectory temp_directory("copperfin_localization_invalid_discovery_tests");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path executable_root = temp_directory.path() / "layout" / "bin";
    const fs::path executable_path = executable_root / "copperfin_runtime_host";
    fs::create_directories(executable_root);
    write_text(executable_path, "");

    const fs::path empty_candidate =
        executable_root / ".." / "share" / "copperfin" / "locales";
    fs::create_directories(empty_candidate);

    const fs::path file_candidate =
        executable_root / "share" / "copperfin" / "locales";
    fs::create_directories(file_candidate.parent_path());
    write_text(file_candidate, "not a locale directory\n");

    const fs::path missing_default_candidate =
        executable_root / ".." / "resources" / "locales";
    write_catalog(missing_default_candidate, "es-419", "{}\n");

    const fs::path directory_catalog_candidate =
        executable_root / ".." / ".." / "resources" / "locales";
    write_catalog(directory_catalog_candidate, "en-US", "{}\n");
    const fs::path directory_catalog =
        directory_catalog_candidate / "en-US" / "strings.json";
    fs::remove(directory_catalog);
    fs::create_directory(directory_catalog);

    const fs::path working_directory = temp_directory.path() / "working";
    const fs::path valid_root = working_directory / "resources" / "locales";
    fs::create_directories(working_directory);
    write_catalog(valid_root, "en-US", "{\"Discovery.Source\":\"working directory\"}\n");

    fs::path resolved_root;
    {
        ScopedCurrentPath current_path(working_directory);
        resolved_root = copperfin::localization::resolve_catalog_root(executable_path);
    }
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");

    expect(
        fs::equivalent(resolved_root, valid_root) &&
            catalog.translate("Discovery.Source") == "working directory",
        "#4088: auto-discovery should skip empty, file-shaped, incomplete, and directory-shaped candidates");
}

void test_catalog_root_auto_discovery_skips_malformed_default_catalog() {
    namespace fs = std::filesystem;
    ScopedTempDirectory temp_directory("copperfin_localization_malformed_discovery_tests");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path executable_root = temp_directory.path() / "layout" / "bin";
    const fs::path executable_path = executable_root / "copperfin_runtime_host";
    fs::create_directories(executable_root);
    write_text(executable_path, "");

    const fs::path malformed_root =
        executable_root / ".." / "share" / "copperfin" / "locales";
    const fs::path later_valid_root =
        executable_root / "share" / "copperfin" / "locales";
    write_catalog(malformed_root, "en-US", "{not valid JSON}\n");
    write_catalog(later_valid_root, "en-US", "{\"Discovery.Source\":\"later executable root\"}\n");

    const fs::path resolved_root =
        copperfin::localization::resolve_catalog_root(executable_path);
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");
    expect(
        fs::equivalent(resolved_root, later_valid_root) &&
            catalog.translate("Discovery.Source") == "later executable root",
        "#4088: discovery should skip a malformed default catalog and continue to a valid root");
}

#if !defined(_WIN32)
void test_catalog_root_auto_discovery_skips_unreadable_default_catalog_when_enforced() {
    namespace fs = std::filesystem;
    ScopedTempDirectory temp_directory("copperfin_localization_unreadable_discovery_tests");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path executable_root = temp_directory.path() / "layout" / "bin";
    const fs::path executable_path = executable_root / "copperfin_runtime_host";
    fs::create_directories(executable_root);
    write_text(executable_path, "");

    const fs::path unreadable_root =
        executable_root / ".." / "share" / "copperfin" / "locales";
    const fs::path later_valid_root =
        executable_root / "share" / "copperfin" / "locales";
    write_catalog(unreadable_root, "en-US", "{}\n");
    write_catalog(later_valid_root, "en-US", "{\"Discovery.Source\":\"later executable root\"}\n");

    const fs::path unreadable_catalog = unreadable_root / "en-US" / "strings.json";
    ScopedPathPermissions catalog_permissions(unreadable_catalog);
    const bool permissions_changed = catalog_permissions.replace(fs::perms::none);
    expect(
        permissions_changed,
        "#4088: POSIX unreadable-catalog fixture should remove file permissions");
    if (permissions_changed) {
        std::ifstream permission_probe(unreadable_catalog, std::ios::binary);
        if (!permission_probe.good()) {
            const fs::path resolved_root =
                copperfin::localization::resolve_catalog_root(executable_path);
            expect(
                fs::equivalent(resolved_root, later_valid_root),
                "#4088: discovery should skip an unreadable regular default catalog when the host enforces permissions");
        }
    }
}
#endif

void test_catalog_root_auto_discovery_preserves_executable_candidate_precedence() {
    namespace fs = std::filesystem;
    ScopedTempDirectory temp_directory("copperfin_localization_discovery_precedence_tests");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path executable_root =
        temp_directory.path() / path_from_utf8_string("app_\xD0\x96_\xE6\xBC\xA2") / "bin";
    const fs::path executable_path = executable_root / "copperfin_inspect";
    fs::create_directories(executable_root);
    write_text(executable_path, "");

    const fs::path installed_root =
        executable_root / ".." / "share" / "copperfin" / "locales";
    const fs::path bundled_root =
        executable_root / "share" / "copperfin" / "locales";
    write_catalog(installed_root, "en-US", "{\"Discovery.Source\":\"installed\"}\n");
    write_catalog(bundled_root, "en-US", "{\"Discovery.Source\":\"bundled\"}\n");

    const fs::path working_directory = temp_directory.path() / "working";
    const fs::path developer_root = working_directory / "resources" / "locales";
    fs::create_directories(working_directory);
    write_catalog(developer_root, "en-US", "{\"Discovery.Source\":\"developer\"}\n");

    fs::path resolved_root;
    {
        ScopedCurrentPath current_path(working_directory);
        resolved_root = copperfin::localization::resolve_catalog_root(executable_path);
    }
    const auto catalog = copperfin::localization::load_catalogs(resolved_root, "en-US");

    expect(
        fs::equivalent(resolved_root, installed_root) &&
            catalog.translate("Discovery.Source") == "installed",
        "#4088: a valid installed share root should retain executable-candidate precedence");
}

void test_catalog_root_explicit_override_remains_authoritative() {
    namespace fs = std::filesystem;
    ScopedTempDirectory temp_directory("copperfin_localization_explicit_override_tests");
    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR");

    const fs::path explicit_override =
        temp_directory.path() / path_from_utf8_string("override_\xD0\x96_\xE6\xBC\xA2");
    write_text(explicit_override, "explicit overrides need not be catalog directories\n");
    locale_dir.set(explicit_override);

    const fs::path executable_root = temp_directory.path() / "app" / "bin";
    const fs::path executable_path = executable_root / "copperfin_inspect";
    const fs::path installed_root =
        executable_root / ".." / "share" / "copperfin" / "locales";
    fs::create_directories(executable_root);
    write_text(executable_path, "");
    write_catalog(installed_root, "en-US", "{}\n");

    expect(
        copperfin::localization::resolve_catalog_root(executable_path) == explicit_override,
        "#4088: explicit COPPERFIN_LOCALE_DIR should remain authoritative without auto-discovery validation");
}

void test_catalog_root_resolution_searches_parent_directories() {
    namespace fs = std::filesystem;
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
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
#if defined(_WIN32)
    ScopedEnvironmentPath search_path("PATH", false);
#else
    ScopedEnvironmentValue search_path("PATH", false);
#endif
#if defined(_WIN32)
    ScopedEnvironmentValue path_extensions("PATHEXT", ".EXE;.COM;.BAT;.CMD");
#endif

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_path_catalog_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root / "resources" / "locales");
    seed_test_catalogs(temp_root / "resources" / "locales");

#if defined(_WIN32)
    const std::string executable_file_name = "copperfin_build_host.exe";
    const std::string invocation_name = "copperfin_build_host";
#else
    const std::string executable_file_name = "copperfin_build_host";
    const std::string invocation_name = executable_file_name;
#endif
    const fs::path unicode_build_root =
        temp_root / path_from_utf8_string("build_\xD0\x96_\xE6\xBC\xA2");
#if defined(_WIN32)
    const fs::path executable_directory = unicode_build_root / "Release";
#else
    const fs::path executable_directory = unicode_build_root / "Release ";
#endif
    const fs::path executable_path = executable_directory / executable_file_name;
    fs::create_directories(executable_path.parent_path());
    write_text(executable_path, "");
    fs::create_directories(
        executable_directory / ".." / "share" / "copperfin" / "locales");
#if !defined(_WIN32)
    fs::permissions(
        executable_path,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

#if defined(_WIN32)
    const std::filesystem::path original_path = getenv_path("PATH");
    std::wstring seeded_path = executable_path.parent_path().native();
    if (!original_path.empty()) {
        seeded_path += L';';
        seeded_path += original_path.native();
    }
#else
    const std::string original_path = getenv_value("PATH");
    const char path_separator = ':';
    std::string seeded_path = executable_path.parent_path().string() +
        (original_path.empty() ? std::string() : std::string(1U, path_separator) + original_path);
    const fs::path shadow_root = temp_root / "non-executable-shadow";
    fs::create_directories(shadow_root);
    write_text(shadow_root / invocation_name, "not executable");
    seeded_path = shadow_root.string() + std::string(1U, path_separator) + seeded_path;
#endif
#if defined(_WIN32)
    search_path.set(fs::path(seeded_path));
#else
    search_path.set(seeded_path);
#endif

    const fs::path nested_working_directory = temp_root / "cwd" / "nested";
    fs::create_directories(nested_working_directory);
    const fs::path previous_working_directory = fs::current_path();
    fs::current_path(nested_working_directory);
    const fs::path resolved_root = copperfin::localization::resolve_catalog_root(invocation_name);
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
