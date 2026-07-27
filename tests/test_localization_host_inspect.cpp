// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_localization_support.h"

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

void test_inspect_accepts_posix_locale_suffixes(const std::string& inspect_path) {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_posix_suffix_cli_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    seed_test_catalogs(temp_root);
    write_catalog(
        temp_root,
        "pt-BR",
        "{\n"
        "  \"Inspect.Usage\": \"Uso-sufixo: {commandName} [{localeOption} {localeValue}] {assetPathArgument}\"\n"
        "}\n");

    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE_DIR", temp_root.string(), true);

    set_env_value("COPPERFIN_LOCALE", "", false);
    const std::string explicit_output = run_command_capture(
        shell_quote(inspect_path) + " --locale pt-BR.UTF-8 2>&1");
    expect(
        explicit_output.find("Uso-sufixo:") != std::string::npos &&
            explicit_output.find("--locale") != std::string::npos,
        "#3915: explicit dotted POSIX locale should select the regional catalog without changing CLI tokens");

    for (const std::string& selected_locale : {
             std::string("pt-BR.UTF-8"),
             std::string("pt_BR@latin"),
             std::string("pt_BR.UTF-8@latin")}) {
        set_env_value("COPPERFIN_LOCALE", selected_locale, true);
        const std::string environment_output = run_command_capture(shell_quote(inspect_path) + " 2>&1");
        expect(
            environment_output.find("Uso-sufixo:") != std::string::npos &&
                environment_output.find("--locale") != std::string::npos,
            "#3915: COPPERFIN_LOCALE POSIX form '" + selected_locale +
                "' should select the regional catalog without changing CLI tokens");
    }

    fs::remove_all(temp_root, ignored);
}

void test_inspect_explicit_locale_routes_dbf_version_display(const std::string& inspect_path) {
    namespace fs = std::filesystem;
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_inspect_version_tests";
    const fs::path dbf_path = temp_root / "version.dbf";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    write_catalog(
        temp_root,
        "en-US",
        "{\n"
        "  \"Vfp.DbfHeader.Version.VisualFoxPro\": \"English VFP\"\n"
        "}\n");
    write_catalog(
        temp_root,
        "es-419",
        "{\n"
        "  \"Vfp.DbfHeader.Version.VisualFoxPro\": \"Espanol VFP\"\n"
        "}\n");
    write_catalog(
        temp_root,
        "pt-BR",
        "{\n"
        "  \"Vfp.DbfHeader.Version.VisualFoxPro\": \"Portugues VFP\"\n"
        "}\n");
    write_catalog(
        temp_root,
        "qps-ploc",
        "{\n"
        "  \"Vfp.DbfHeader.Version.VisualFoxPro\": \"Pseudo VFP\"\n"
        "}\n");

    std::string dbf_bytes(33U, '\0');
    dbf_bytes[0] = static_cast<char>(0x30U);
    dbf_bytes[1] = static_cast<char>(126U);
    dbf_bytes[2] = static_cast<char>(7U);
    dbf_bytes[3] = static_cast<char>(26U);
    dbf_bytes[8] = static_cast<char>(33U);
    dbf_bytes[10] = static_cast<char>(1U);
    dbf_bytes[32] = static_cast<char>(0x0DU);
    {
        std::ofstream stream(dbf_path, std::ios::binary);
        stream.write(dbf_bytes.data(), static_cast<std::streamsize>(dbf_bytes.size()));
    }

    set_env_value("COPPERFIN_LOCALE_DIR", temp_root.string(), true);
    set_env_value("COPPERFIN_LOCALE", "", false);
    for (const auto& locale_case : {
             std::pair<std::string, std::string>{"es-419", "Espanol VFP"},
             std::pair<std::string, std::string>{"pt-BR", "Portugues VFP"}}) {
        const std::string output = run_command_capture(
            shell_quote(inspect_path) + " " + shell_quote(dbf_path.string()) +
            " --locale " + locale_case.first + " 2>&1");
        expect(
            output.find("header.version_description: " + locale_case.second) != std::string::npos,
            "#4733: explicit " + locale_case.first + " should localize DBF version display");
        expect(
            output.find("header.version_description: English VFP") == std::string::npos,
            "#4733: explicit " + locale_case.first + " should not use the environment/default DBF catalog");
    }

    const std::string pseudo_output = run_command_capture(
        shell_quote(inspect_path) + " " + shell_quote(dbf_path.string()) +
        " --locale qps-ploc 2>&1");
    expect(
        pseudo_output.find("header.version_description: [!! ") != std::string::npos,
        "#4733: explicit qps-ploc should pseudo-localize DBF version display");

    fs::remove_all(temp_root, ignored);
}

void test_inspect_license_status_preserves_machine_contracts(const std::string& inspect_path) {
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR");
    set_env_value("COPPERFIN_LOCALE_DIR", "", false);

    const auto expect_license_status_contract = [&](const std::string& selected_locale, const std::string& issue_tag) {
        set_env_value("COPPERFIN_LOCALE", selected_locale, true);
        const std::string output = run_command_capture(shell_quote(inspect_path) + " --license-status 2>&1");

        const std::size_t status_position = output.find("status: ok");
        const std::size_t state_position = output.find("state: ");
        expect(status_position != std::string::npos,
               issue_tag + ": copperfin_inspect --license-status should preserve the machine-readable status line");
        expect(state_position != std::string::npos,
               issue_tag + ": copperfin_inspect --license-status should preserve the machine-readable state line");
        if (status_position != std::string::npos && state_position != std::string::npos) {
            expect(status_position < state_position,
                   issue_tag + ": copperfin_inspect --license-status should print status before state");
        }
    };

    expect_license_status_contract("es-419", "#3816");
    expect_license_status_contract("qps-ploc", "#3816");
    expect_license_status_contract("pt_BR.UTF-8@latin", "#3915");
}

void test_runtime_package_warnings_pseudo_localize() {
    namespace fs = std::filesystem;

    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    copperfin::test_support::ScopedTestLocaleCatalogDirectory locale_dir;
    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);

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
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE");
    copperfin::test_support::ScopedTestLocaleCatalogDirectory locale_dir;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_localization_inspect_error_prefix_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    set_env_value("COPPERFIN_LOCALE", "qps-ploc", true);
    const std::string missing_asset = (temp_root / "missing.dbf").string();
    const std::string output = run_command_capture(
        shell_quote(inspect_path) + " " + shell_quote(missing_asset) + " 2>&1");

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
