// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_host_debug_output_support.h"

void test_runtime_host_usage_text_localizes_without_changing_cli_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_usage_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    std::cerr << "USAGE: fixture root ready\n";
#if defined(_WIN32)
    const fs::path locale_root = temp_root / fs::path(L"locales_\u0416_\u6F22");
#else
    const fs::path locale_root = temp_root / "locales_\xD0\x96_\xE6\xBC\xA2";
#endif
    write_runtime_host_usage_catalogs(locale_root);
    std::cerr << "USAGE: catalogs ready\n";

    {
        std::cerr << "USAGE: BEGIN en-US\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: runtime host without manifest should keep the usage exit code");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2349: runtime host en-US usage should remain stable");
        expect(process.stdout_text.find("--federation-backend <sqlite|postgresql|sqlserver|oracle>") != std::string::npos,
               "#2349: runtime host en-US usage should preserve federation CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-enable", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: runtime host should reject invalid federation planning booleans");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-enable value must be true or false.") != std::string::npos,
               "#3791: invalid federation planning booleans should localize the en-US parse error");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-enable") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("<true|false>") != std::string::npos,
               "#3791: invalid federation planning booleans should preserve invariant CLI tokens in usage output");
        std::cerr << "USAGE: END en-US\n";
    }

    {
        std::cerr << "USAGE: BEGIN es-419\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2585: es-419 runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#2585: es-419 runtime host usage should localize manifest usage prose");
        expect(process.stdout_text.find("   o: copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--federation-backend") != std::string::npos &&
                   process.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: es-419 runtime host usage should localize alternate usage prose while preserving CLI tokens");
        expect(process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#2585: es-419 runtime host usage should not fall back to raw English prose");

        const auto slash_locale_process = run_process_capture(runtime_host_path, {"/locale", "es-419"}, temp_root);
        expect(slash_locale_process.exit_code == 2,
               "#3752: /locale should keep the normal usage exit code when no manifest is available");
        expect(slash_locale_process.stdout_text.find("Uso: copperfin_runtime_host --manifest <path> [--debug]") != std::string::npos,
               "#3752: /locale should select the same localized catalog as --locale");
        expect(slash_locale_process.stdout_text.find("Usage: copperfin_runtime_host --manifest <path> [--debug]") == std::string::npos,
               "#3752: /locale should not fall back to raw English prose");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-require", "quizas"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: es-419 invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: El valor de --federation-planning-require debe ser true o false.") != std::string::npos,
               "#3791: es-419 invalid federation planning booleans should localize parse errors while preserving option tokens");
        expect(invalid_federation_bool.stdout_text.find(
                   "error: The --federation-planning-require value must be true or false.") == std::string::npos,
               "#3791: es-419 invalid federation planning booleans should not fall back to raw English prose");
        std::cerr << "USAGE: END es-419\n";
    }

    {
        std::cerr << "USAGE: BEGIN qps-ploc\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(runtime_host_path, {}, temp_root);
        expect(process.exit_code == 2,
               "#2349: pseudo-localized runtime host usage should keep the usage exit code");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should decorate prose");
        expect(process.stdout_text.find("copperfin_runtime_host") != std::string::npos &&
                   process.stdout_text.find("--manifest") != std::string::npos &&
                   process.stdout_text.find("--debug-command") != std::string::npos &&
                   process.stdout_text.find("<continue|step|next|out|watch:<expr>|select:<action-id>|invoke:<action-id>|break:add:<file:line>|break:remove:<file:line>|break:add-action:<action-id>|break:remove-action:<action-id>|break:clear|break:list>") != std::string::npos,
               "#2349: pseudo-localized runtime host usage should preserve CLI and debug-command tokens");

        const auto slash_debug = run_process_capture(runtime_host_path, {"/debug"}, temp_root);
        expect(slash_debug.exit_code == 2,
               "#3752: /debug should keep the normal usage exit code when no manifest is available");
        expect(slash_debug.stdout_text.find("status: error") == std::string::npos,
               "#3752: /debug should be accepted as a host alias instead of surfacing an unknown-argument contract");
        expect(slash_debug.stdout_text.find("[!! ") != std::string::npos,
               "#3752: /debug acceptance should still honor the selected pseudo-localized catalog");
        expect(slash_debug.stdout_text.find("--debug-command") != std::string::npos,
               "#3752: /debug acceptance should preserve ordinary usage/debug token output");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2351: pseudo-localized runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized runtime host errors should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should decorate prose");
        expect(unknown_argument.stdout_text.find("--unknown-option") != std::string::npos,
               "#2351: pseudo-localized runtime host unknown arguments should preserve CLI tokens");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2351: pseudo-localized federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("status: error") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve machine-readable status");
        expect(missing_federation_argument.stdout_text.find("[!! ") != std::string::npos,
               "#2351: pseudo-localized federation validation should decorate prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2351: pseudo-localized federation validation should preserve CLI tokens");

        const auto invalid_federation_bool = run_process_capture(
            runtime_host_path,
            {
                "--federation-backend", "sqlite",
                "--federation-query", "SELECT * FROM customer",
                "--federation-planning-audit", "maybe"
            },
            temp_root);
        expect(invalid_federation_bool.exit_code == 2,
               "#3791: pseudo-localized invalid federation planning booleans should keep the usage exit code");
        expect(invalid_federation_bool.stdout_text.find("status: error") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve machine-readable status");
        expect(invalid_federation_bool.stdout_text.find("[!! ") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should decorate prose");
        expect(invalid_federation_bool.stdout_text.find("--federation-planning-audit") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("true") != std::string::npos &&
                   invalid_federation_bool.stdout_text.find("false") != std::string::npos,
               "#3791: pseudo-localized invalid federation planning booleans should preserve invariant boolean tokens");

        const fs::path bridge_manifest_path = temp_root / "bridge.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge.prg";
        const fs::path bridge_response_path = temp_root / "bridge.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalization\n"
            "startup_item=bridge.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2352: pseudo-localized bridge errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2352: pseudo-localized bridge errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("[!! ") != std::string::npos,
               "#2352: pseudo-localized bridge errors should decorate prose");
        std::cerr << "USAGE: END qps-ploc\n";
    }

    {
        std::cerr << "USAGE: BEGIN es-419 bridge\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");

        const fs::path bridge_manifest_path = temp_root / "bridge_es.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_es.prg";
        const fs::path bridge_response_path = temp_root / "bridge_es.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationSpanish\n"
            "startup_item=bridge_es.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.es.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: es-419 bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("error: No se encontro el artefacto de solicitud bridge.") != std::string::npos,
               "#2587: es-419 bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: es-419 bridge request-artifact errors should not fall back to raw English prose");
        std::cerr << "USAGE: END es-419 bridge\n";
    }

    {
        std::cerr << "USAGE: BEGIN pt-BR\n";
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");

        const auto unknown_argument = run_process_capture(runtime_host_path, {"--unknown-option"}, temp_root);
        expect(unknown_argument.exit_code == 2,
               "#2585: pt-BR runtime host unknown arguments should keep the usage exit code");
        expect(unknown_argument.stdout_text.find("status: error") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should preserve machine-readable status");
        expect(unknown_argument.stdout_text.find("erro: Argumento desconhecido: --unknown-option") != std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should localize prefixed error prose");
        expect(unknown_argument.stdout_text.find("error: Unknown argument: --unknown-option") == std::string::npos,
               "#2585: pt-BR runtime host unknown arguments should not fall back to raw English prose");

        const auto missing_federation_argument = run_process_capture(
            runtime_host_path,
            {"--federation-backend", "sqlite"},
            temp_root);
        expect(missing_federation_argument.exit_code == 2,
               "#2585: pt-BR federation validation should keep the usage exit code");
        expect(missing_federation_argument.stdout_text.find("erro: --federation-backend e --federation-query sao obrigatorios no modo de federacao.") != std::string::npos,
               "#2585: pt-BR federation validation should localize required-option prose");
        expect(missing_federation_argument.stdout_text.find("--federation-backend") != std::string::npos &&
                   missing_federation_argument.stdout_text.find("--federation-query") != std::string::npos,
               "#2585: pt-BR federation validation should preserve CLI tokens");
        expect(missing_federation_argument.stdout_text.find("error: --federation-backend and --federation-query are both required in federation mode.") == std::string::npos,
               "#2585: pt-BR federation validation should not fall back to raw English prose");

        const fs::path bridge_manifest_path = temp_root / "bridge_pt.cfmanifest";
        const fs::path bridge_source_path = temp_root / "bridge_pt.prg";
        const fs::path bridge_response_path = temp_root / "bridge_pt.response.json";
        write_text(
            bridge_manifest_path,
            std::string("manifest_version=1\n"
            "project_title=BridgeLocalizationPortuguese\n"
            "startup_item=bridge_pt.prg\n"
            "startup_source=") + bridge_source_path.string() + "\n"
            "security_enabled=false\n"
            "dotnet_story=none\n");
        write_text(bridge_source_path, "RETURN 1\n");

        const auto bridge_error = run_process_capture(
            runtime_host_path,
            {
                "--manifest", bridge_manifest_path.string(),
                "--request-path", (temp_root / "missing.request.pt.json").string(),
                "--response-path", bridge_response_path.string(),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        expect(bridge_error.exit_code == 6,
               "#2587: pt-BR bridge request-artifact errors should keep the bridge validation exit code");
        expect(bridge_error.stdout_text.find("status: error") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve machine-readable status");
        expect(bridge_error.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should preserve bridge runtime mode");
        expect(bridge_error.stdout_text.find("erro: Artefato de solicitacao bridge nao encontrado.") != std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should localize prose");
        expect(bridge_error.stdout_text.find("error: Bridge request artifact not found.") == std::string::npos,
               "#2587: pt-BR bridge request-artifact errors should not fall back to raw English prose");
        std::cerr << "USAGE: END pt-BR\n";
    }

    fs::remove_all(temp_root, ignored);
}
void test_runtime_host_debug_errors_localize_without_changing_command_tokens(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_debug_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DebugErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: en-US invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: en-US invalid breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") !=
                std::string::npos,
            "#2391: en-US invalid breakpoint diagnostics should remain stable");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:remove:2"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: es-419 unknown breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should preserve machine-readable status");
        expect(
            process.stdout_text.find("Breakpoint desconocido: " + startup_path.string() + ":2") != std::string::npos,
            "#2586: es-419 unknown breakpoint diagnostics should localize the error body while preserving path and line");
        expect(process.stdout_text.find("Unknown breakpoint: " + startup_path.string() + ":2") == std::string::npos,
               "#2586: es-419 unknown breakpoint diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2566: pt-BR invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error prefix");
        expect(process.stdout_text.find("Comando de breakpoint invalido: break:add:not-a-breakpoint") != std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should localize the error body");
        expect(process.stdout_text.find("error: Invalid breakpoint command") == std::string::npos,
               "#2566: pt-BR invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "watch:nValue"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2586: pt-BR watch diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2586: pt-BR watch diagnostics should preserve machine-readable status");
        expect(process.stdout_text.find("erro: ") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the error prefix");
        expect(process.stdout_text.find("A avaliacao de watch requer um estado pausado ativo.") != std::string::npos,
               "#2586: pt-BR watch diagnostics should localize the paused-state error");
        expect(process.stdout_text.find("Watch evaluation requires an active paused state.") == std::string::npos,
               "#2586: pt-BR watch diagnostics should not fall back to the raw English error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "break:add:not-a-breakpoint"
            },
            temp_root);
        expect(process.exit_code == 5,
               "#2391: pseudo-localized invalid breakpoint diagnostics should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve machine-readable status");
        const std::string pseudo_error_prefix =
            copperfin::localization::load_catalogs(locale_root, "qps-ploc").translate("RuntimeHost.Prefix.Error");
        expect(process.stdout_text.find(pseudo_error_prefix) != std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should route the error prefix through qps-ploc");
        expect(process.stdout_text.find("[!! ") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should decorate prose");
        expect(process.stdout_text.find("break:add:not-a-breakpoint") != std::string::npos,
               "#2391: pseudo-localized invalid breakpoint diagnostics should preserve debug command tokens");
        expect(process.stdout_text.find("error: Invalid breakpoint command: break:add:not-a-breakpoint") == std::string::npos,
               "#2566: pseudo-localized invalid breakpoint diagnostics should not fall back to the raw English prefixed error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_xasset_open_errors_follow_explicit_locale(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_xasset_open_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path sidecar_path = temp_root / "startup.sct";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(sidecar_path, "not-a-valid-primary-document\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=XAssetOpenLocalization\n"
        "startup_item=startup.sct\n"
        "startup_source=" + sidecar_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=developer\n"
        "security_mode=off\n"
        "dotnet_story=none\n");

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    ScopedEnvironmentValue environment_locale("COPPERFIN_LOCALE", "en-US");
    const auto process = run_process_capture(
        runtime_host_path,
        {"--locale", "qps-ploc", "--manifest", manifest_path.string()},
        temp_root);

    expect(process.exit_code == 0,
           "#4737: xAsset fallback should preserve the compatibility-launcher exit code");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "#4737: xAsset fallback should preserve the machine-readable success status");
    expect(process.stdout_text.find("runtime.mode: compatibility-launcher") != std::string::npos,
           "#4737: xAsset open failure should preserve the compatibility-launcher mode");
    expect(process.stdout_text.find(
               "[!! Ţhë prïmåry døçümëñţ før sïdëçår '") != std::string::npos &&
               process.stdout_text.find("startup.sct' wås ñøţ føüñd. !!]") != std::string::npos,
           "#4737: xAsset open diagnostics should follow the explicit pseudo-locale");
    expect(process.stdout_text.find(
               "The primary document for sidecar '" + sidecar_path.string() +
               "' was not found.") == std::string::npos,
           "#4737: xAsset open diagnostics should not fall back to COPPERFIN_LOCALE");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_session_messages_follow_explicit_locale(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_session_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(startup_path, "nValue = 1\nRETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=SessionLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    ScopedEnvironmentValue environment_locale("COPPERFIN_LOCALE", "en-US");
    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--locale", "qps-ploc",
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-stop-on-entry",
            "--debug-command", "continue"
        },
        temp_root);

    expect(process.exit_code == 0,
           "#4738: explicit-locale runtime sessions should preserve the runtime-host success exit code");
    expect(process.stdout_text.find("status: ok") != std::string::npos,
           "#4738: explicit-locale runtime sessions should preserve machine-readable success status");
    expect(process.stdout_text.find("debug.reason: entry") != std::string::npos,
           "#4738: explicit-locale runtime sessions should preserve the invariant entry pause reason");
    expect(process.stdout_text.find("debug.message: [!! Sţøppëd øñ ëñţry. !!]") != std::string::npos,
           "#4738: session-produced runtime messages should follow the explicit pseudo-locale");
    expect(process.stdout_text.find("debug.message: Stopped on entry.") == std::string::npos,
           "#4738: session-produced runtime messages should not fall back to COPPERFIN_LOCALE");

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_rejects_invalid_debug_command_without_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_invalid_debug_command_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);
    const fs::path startup_path = temp_root / "main.prg";
    const fs::path marker_path = temp_root / "marker.txt";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(startup_path, "STRTOFILE('ran', 'marker.txt')\nRETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=InvalidDebugCommand\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string(), "--debug", "--debug-command", "contnue"},
            temp_root);
        expect(process.exit_code == 5,
               "#4206: invalid debug commands should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#4206: invalid debug commands should preserve machine-readable error status");
        expect(process.stdout_text.find("Invalid debug command: contnue") != std::string::npos,
               "#4206: invalid debug commands should use the localized error catalog");
        expect(!fs::exists(marker_path),
               "#4206: invalid debug commands must not execute the startup PRG");
    }

    {
        fs::remove(marker_path, ignored);
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string(), "--debug", "--debug-command", "contnue"},
            temp_root);
        expect(process.exit_code == 5,
               "#4206: es-419 invalid debug commands should keep the debug error exit code");
        expect(process.stdout_text.find("Comando de depuracion invalido: contnue") != std::string::npos,
               "#4206: es-419 invalid debug commands should localize the error body");
        expect(process.stdout_text.find("Invalid debug command: contnue") == std::string::npos,
               "#4206: es-419 invalid debug commands should not fall back to English");
        expect(!fs::exists(marker_path),
               "#4206: es-419 invalid debug commands must not execute the startup PRG");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_rejects_invalid_startup_breakpoint_without_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_invalid_startup_breakpoint_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);
    const fs::path startup_path = temp_root / "main.prg";
    const fs::path marker_path = temp_root / "marker.txt";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(startup_path, "STRTOFILE('ran', 'marker.txt')\nRETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=InvalidStartupBreakpoint\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string(), "--debug", "--breakpoint", "main.prg:abc"},
            temp_root);
        expect(process.exit_code == 5,
               "#4240: malformed startup breakpoints should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#4240: malformed startup breakpoints should preserve machine-readable error status");
        expect(process.stdout_text.find("Invalid breakpoint command: main.prg:abc") != std::string::npos,
               "#4240: malformed startup breakpoints should use the localized diagnostic catalog");
        expect(!fs::exists(marker_path),
               "#4240: malformed startup breakpoints must not execute the startup PRG");
    }

    {
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string(), "--debug", "--breakpoint", "main.prg:abc"},
            temp_root);
        expect(process.exit_code == 5,
               "#4240: es-419 malformed startup breakpoints should keep the debug error exit code");
        expect(process.stdout_text.find("Comando de breakpoint invalido: main.prg:abc") != std::string::npos,
               "#4240: es-419 malformed startup breakpoints should localize the error body");
        expect(process.stdout_text.find("Invalid breakpoint command: main.prg:abc") == std::string::npos,
               "#4240: es-419 malformed startup breakpoints should not fall back to English");
        expect(!fs::exists(marker_path),
               "#4240: es-419 malformed startup breakpoints must not execute the startup PRG");
    }

    {
        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string(), "--debug", "--breakpoint", "main.prg:abc"},
            temp_root);
        expect(process.exit_code == 5,
               "#4240: pseudo-locale malformed startup breakpoints should keep the debug error exit code");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "#4240: pseudo-locale malformed startup breakpoints should preserve machine-readable status");
        expect(process.stdout_text.find("main.prg:abc") != std::string::npos,
               "#4240: pseudo-locale malformed startup breakpoints should preserve the invalid token");
        expect(!fs::exists(marker_path),
               "#4240: pseudo-locale malformed startup breakpoints must not execute the startup PRG");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_pause_messages_localize_without_changing_pause_reasons(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_pause_message_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=PauseMessageLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        write_text(startup_path, "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: en-US completed pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("status: ok") != std::string::npos,
               "#2589: en-US completed pause messages should preserve machine-readable ok status");
        expect(process.stdout_text.find("debug.reason: completed") != std::string::npos,
               "#2589: en-US completed pause messages should preserve the completed pause reason");
        expect(process.stdout_text.find("debug.message: Execution completed.") != std::string::npos,
               "#2589: en-US completed pause messages should remain stable");
    }

    {
        write_text(
            startup_path,
            "nValue = 1\n"
            "nValue = 2\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-stop-on-entry",
                "--debug-command", "continue",
                "--debug-command", "step"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#4262: entry-stop debug sessions should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.command[0]: continue") != std::string::npos &&
                   process.stdout_text.find("debug.reason: entry") != std::string::npos,
               "#4262: entry-stop debug sessions should expose the initial entry pause");
        expect(process.stdout_text.find("debug.command[1]: step") != std::string::npos &&
                   process.stdout_text.find("debug.reason: step") != std::string::npos,
               "#4262: entry-stop debug sessions should preserve the next step pause");
    }

    {
        write_text(startup_path, "STRTOFILE('live', 'live-session-marker.txt')\nRETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-server"
            },
            temp_root,
            std::string("continue\nstep\nexit\n"));
        expect(process.exit_code == 0,
               "#4296: persistent debug server should preserve a successful exit code after an explicit exit");
        expect(process.stdout_text.find("debug.server.ready: true") != std::string::npos,
               "#4296: persistent debug server should publish an invariant readiness marker");
        expect(process.stdout_text.find("debug.response.begin") != std::string::npos &&
                   process.stdout_text.find("debug.response.end") != std::string::npos,
               "#4296: persistent debug server should frame each command response");
        expect(process.stdout_text.find("debug.command[0]: continue") != std::string::npos &&
                   process.stdout_text.find("debug.reason: entry") != std::string::npos,
               "#4296: persistent debug server should execute continue in the initialized session");
        expect(process.stdout_text.find("debug.command[2]: exit") != std::string::npos &&
                   process.stdout_text.find("debug.exit: true") != std::string::npos,
               "#4296: persistent debug server should support deterministic session shutdown");
        expect(fs::exists(temp_root / "live-session-marker.txt"),
               "#4296: persistent debug server should execute startup side effects in the live process");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "step"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: es-419 step pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: step") != std::string::npos,
               "#2589: es-419 step pause messages should preserve the step pause reason");
        expect(process.stdout_text.find("debug.message: El paso se completo.") != std::string::npos,
               "#2589: es-419 step pause messages should localize the step-completed prose");
        expect(process.stdout_text.find("debug.message: Step completed.") == std::string::npos,
               "#2589: es-419 step pause messages should not fall back to the raw English step-completed prose");
    }

    {
        write_text(
            startup_path,
            "READ EVENTS\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: pt-BR READ EVENTS pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should preserve the event-loop pause reason");
        expect(process.stdout_text.find("debug.message: O runtime esta aguardando em READ EVENTS.") !=
                   std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should localize prose while preserving the READ EVENTS token");
        expect(process.stdout_text.find("The runtime is waiting in READ EVENTS.") == std::string::npos,
               "#2589: pt-BR READ EVENTS pause messages should not fall back to the raw English prose");
    }

    {
        write_text(
            startup_path,
            "LOCAL nValue\n"
            "nValue = 1\n"
            "RETURN\n");
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2589: qps-ploc breakpoint pause messages should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.message: [!! ") != std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should pseudo-localize the debug message");
        expect(process.stdout_text.find("Breakpoint hit.") == std::string::npos,
               "#2589: qps-ploc breakpoint pause messages should not fall back to the raw English breakpoint prose");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_watch_errors_localize_without_changing_watch_fields(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_watch_error_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "main.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=WatchErrorLocalization\n"
        "startup_item=main.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: pt-BR watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.expression: ") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.expression field");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: pt-BR watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: A expressao de watch esta vazia.") !=
                   std::string::npos,
               "#2590: pt-BR watch errors should localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: pt-BR watch errors should not fall back to the raw English watch error");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {
                "--manifest", manifest_path.string(),
                "--debug",
                "--breakpoint", startup_path.string() + ":2",
                "--debug-command", "continue",
                "--debug-command", "watch:"
            },
            temp_root);
        expect(process.exit_code == 0,
               "#2590: qps-ploc watch errors should keep the runtime-host success exit code");
        expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the breakpoint pause reason");
        expect(process.stdout_text.find("debug.watch.ok: false") != std::string::npos,
               "#2590: qps-ploc watch errors should preserve the debug.watch.ok field");
        expect(process.stdout_text.find("debug.watch.error: [!! ") != std::string::npos,
               "#2590: qps-ploc watch errors should pseudo-localize the watch error prose");
        expect(process.stdout_text.find("debug.watch.error: Watch expression is empty.") ==
                   std::string::npos,
               "#2590: qps-ploc watch errors should not fall back to the raw English watch error");
    }

    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_escapes_multiline_debug_values(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_multiline_debug_value_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "multiline.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    write_text(
        startup_path,
        "PUBLIC cGlobal\n"
        "LOCAL cLocal\n"
        "cGlobal = \"line1\" + CHR(13) + CHR(10) + CHR(9) + \"\\tail\"\n"
        "cLocal = \"line1\" + CHR(13) + CHR(10) + CHR(9) + \"\\tail\"\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=MultilineDebugValues\n"
        "startup_item=multiline.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--debug-server",
            "--breakpoint", startup_path.string() + ":5"
        },
        temp_root,
        "continue\ncontinue\nwatch:cLocal\nwatch:cGlobal\nexit\n");
    const std::string escaped_value = "line1\\r\\n\\t\\\\tail";
    const std::string escaped_watch_value = "debug.watch.value: " + escaped_value;
    const std::string escaped_local_value = "debug.frame[0].local.clocal: " + escaped_value;
    const std::string escaped_global_value = "debug.global.cglobal: " + escaped_value;
    expect(process.exit_code == 0,
           "#4300: multiline debug values should keep the persistent debug server exit contract");
    expect(process.stdout_text.find(escaped_watch_value) != std::string::npos,
           "#4300: watch values should escape CR, LF, tab, and backslash characters");
    expect(process.stdout_text.find(escaped_local_value) != std::string::npos,
           "#4300: local values should escape CR, LF, tab, and backslash characters");
    expect(process.stdout_text.find(escaped_global_value) != std::string::npos,
           "#4300: global values should escape CR, LF, tab, and backslash characters");
    expect(process.stdout_text.find("debug.watch.value: line1\r") == std::string::npos &&
               process.stdout_text.find("debug.watch.value: line1\n") == std::string::npos,
           "#4300: escaped debug values should not inject raw line breaks into responses");
    fs::remove_all(temp_root, ignored);
}

void test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_quit_prompt_localization_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    const fs::path startup_path = temp_root / "quit_prompt.prg";
    const fs::path manifest_path = temp_root / "quit_prompt.cfmanifest";
    write_text(
        startup_path,
        "LOCAL nValue\n"
        "QUIT\n"
        "nValue = 1\n"
        "RETURN\n");
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=QuitPromptLocalization\n"
        "startup_item=quit_prompt.prg\n"
        "startup_source=" + startup_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Do you want to quit this application? [y/N]: ") != std::string::npos,
               "#2591: runtime-host quit prompt should preserve the en-US confirmation prompt");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "es-419");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: es-419 runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Desea salir de esta aplicacion? [y/N]: ") != std::string::npos,
               "#2591: es-419 runtime-host quit prompt should localize the prompt prose");
        expect(process.stderr_text.find("Do you want to quit this application?") == std::string::npos,
               "#2591: es-419 runtime-host quit prompt should not fall back to raw English prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "pt-BR");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: pt-BR runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("Deseja sair deste aplicativo? [y/N]: ") != std::string::npos,
               "#2591: pt-BR runtime-host quit prompt should localize the prompt prose");
    }

    {
        ScopedEnvironmentValue locale_dir("COPPERFIN_LOCALE_DIR", locale_root.string());
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "qps-ploc");
        const auto process = run_process_capture(
            runtime_host_path,
            {"--manifest", manifest_path.string()},
            temp_root,
            std::string("n\n"));
        expect(process.exit_code == 0,
               "#2591: qps-ploc runtime-host quit prompt should keep the normal success exit code when quit is cancelled");
        expect(process.stderr_text.find("[!! ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should pseudo-localize the prompt prose");
        expect(process.stderr_text.find("[y/N]: ") != std::string::npos,
               "#2591: qps-ploc runtime-host quit prompt should preserve confirmation tokens");
    }

    fs::remove_all(temp_root, ignored);
}
