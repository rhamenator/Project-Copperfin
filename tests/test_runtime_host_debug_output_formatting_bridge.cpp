// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_host_debug_output_support.h"
#include "test_process_capture_support.h"

#include "copperfin/platform/path.h"

void test_runtime_host_writes_bridge_response_artifact(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_response_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeResponse\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 7,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "7",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-response stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-response stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: [Unicode path omitted]\n";
    }

    expect(process.exit_code == 0,
           "runtime host should accept wrapper-emitted bridge descriptor and response arguments");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode");
    expect(process.stdout_text.find("bridge.library_export: AddNumbers") != std::string::npos,
           "runtime host should preserve bridge export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the PRG return value in bridge diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host bridge mode should invoke exported routines through a bootstrap");
    expect(fs::exists(response_path),
           "runtime host should write the requested bridge response artifact");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "runtime host bridge response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "runtime host bridge response should include the evaluated PRG return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "runtime host bridge response should echo the expected response media type");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "runtime host bridge response should echo the requested schema version");
    expect(response_document.find("\"diagnostics\": \"bridge_response_written\"") != std::string::npos,
           "runtime host bridge response should include diagnostics");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_security_enabled_bridge_source_stays_inside_verified_package(
    const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            "copperfin_runtime_host_secure_bridge_source-\xD0\x9F\xD1\x83\xD1\x82\xD1\x8C");
    const fs::path content_root = temp_root / "content";
    const fs::path startup_path = content_root / "startup.prg";
    const fs::path source_path = content_root / "exports" / "exports.prg";
    const fs::path include_path = content_root / "shared" / "bridge_value.h";
    const fs::path outside_path = temp_root.parent_path() / "copperfin_external_bridge_source.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "GetAnswer.response.json";
    const fs::path locale_root = temp_root / "locales";
    const fs::path deployed_runtime_host = deployed_runtime_host_path(temp_root, runtime_host_path);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::remove(outside_path, ignored);
    fs::create_directories(content_root);
    fs::create_directories(source_path.parent_path());
    fs::create_directories(include_path.parent_path());
    write_runtime_host_usage_catalogs(locale_root);
    write_text(startup_path, "RETURN\n");
    write_text(
        source_path,
        "#INCLUDE '../shared/BRIDGE_VALUE.H'\n"
        "PROCEDURE GetAnswer\n"
        "RETURN BRIDGE_VALUE\n"
        "ENDPROC\n");
    write_text(include_path, "#DEFINE BRIDGE_VALUE 42\n");
    write_text(outside_path, "PROCEDURE GetAnswer\nRETURN 99\nENDPROC\n");
    fs::copy_file(runtime_host_path, deployed_runtime_host, fs::copy_options::overwrite_existing);
#if defined(__unix__) || defined(__APPLE__)
    fs::permissions(
        deployed_runtime_host,
        fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
        fs::perm_options::add,
        ignored);
#endif

    const auto runtime_host_hash =
        copperfin::security::sha256_hex_for_file(copperfin::platform::path_to_utf8_string(deployed_runtime_host));
    const auto startup_hash = copperfin::security::sha256_hex_for_file(
        copperfin::platform::path_to_utf8_string(startup_path));
    const auto source_hash = copperfin::security::sha256_hex_for_file(
        copperfin::platform::path_to_utf8_string(source_path));
    const auto include_hash = copperfin::security::sha256_hex_for_file(
        copperfin::platform::path_to_utf8_string(include_path));
    expect(runtime_host_hash.ok && startup_hash.ok && source_hash.ok && include_hash.ok,
           "secure bridge fixture should hash host, startup, export source, and include");
    if (!runtime_host_hash.ok || !startup_hash.ok || !source_hash.ok || !include_hash.ok) {
        fs::remove_all(temp_root, ignored);
        fs::remove(outside_path, ignored);
        return;
    }

    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=SecureBridgeSource\n"
        "package_root=" + copperfin::platform::path_to_utf8_string(temp_root) + "\n"
        "content_root=" + copperfin::platform::path_to_utf8_string(content_root) + "\n"
        "working_directory=" + copperfin::platform::path_to_utf8_string(content_root) + "\n"
        "startup_item=startup.prg\n"
        "startup_source=" + copperfin::platform::path_to_utf8_string(startup_path) + "\n"
        "security_enabled=true\n"
        "security_role=runtime-operator\n"
        "security_mode=native\n"
        "runtime_host_sha256=" + runtime_host_hash.hex_digest + "\n"
        "asset=1|startup.prg|" + copperfin::platform::path_to_utf8_string(startup_path) +
            "|Program|false|true|" + startup_hash.hex_digest + "|true\n"
        "asset=2|exports.prg|" + copperfin::platform::path_to_utf8_string(source_path) +
            "|Program|false|true|" + source_hash.hex_digest + "|true\n"
        "extension_payload=" + copperfin::platform::path_to_utf8_string(include_path) + "|" +
        include_hash.hex_digest + "\n"
        "dotnet_story=none\n");

    const auto write_request = [&](const fs::path& requested_source) {
        const std::string escaped_source_path = json_escape_string(
            copperfin::platform::path_to_utf8_string(requested_source));
        write_text(
            request_path,
            std::string("{\n"
            "  \"export_name\": \"GetAnswer\",\n"
            "  \"routine_kind\": \"procedure\",\n"
            "  \"source_path\": \"") + escaped_source_path + "\",\n"
            "  \"source_line\": 1,\n"
            "  \"parameter_declaration\": \"LPARAMETERS\",\n"
            "  \"parameter_names\": \"\",\n"
            "  \"parameter_count\": 0,\n"
            "  \"schema_version\": \"v1\",\n"
            "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
            "  \"parameters\": []\n"
            "}\n");
    };
    const auto invoke = [&](const fs::path& requested_source) {
        write_request(requested_source);
        fs::remove(response_path, ignored);
        const auto captured = copperfin::test_support::run_process_capture(
            deployed_runtime_host,
            {
                "--manifest", copperfin::platform::path_to_utf8_string(manifest_path),
                "--library-export", "GetAnswer",
                "--routine-kind", "procedure",
                "--source-path", copperfin::platform::path_to_utf8_string(requested_source),
                "--source-line", "1",
                "--parameter-declaration", "LPARAMETERS",
                "--parameter-names", "",
                "--parameter-count", "0",
                "--request-path", copperfin::platform::path_to_utf8_string(request_path),
                "--response-path", copperfin::platform::path_to_utf8_string(response_path),
                "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
                "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
                "--schema-version", "v1"
            },
            temp_root);
        return ProcessResult{
            .exit_code = captured.started ? captured.exit_code : -1,
            .stdout_text = captured.stdout_text,
            .stderr_text = captured.stderr_text};
    };

    const auto original_locale_dir = copperfin::platform::read_environment_path(
        "COPPERFIN_LOCALE_DIR");
    const bool locale_write_succeeded = copperfin::platform::write_environment_path(
        "COPPERFIN_LOCALE_DIR",
        locale_root);
    expect(locale_write_succeeded,
           "verified bridge fixture should set its locale directory override");
    const auto packaged_process = invoke(source_path);
    expect(packaged_process.exit_code == 0,
           "security-enabled bridge invocation should execute its verified packaged source bytes");
    expect(packaged_process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "security-enabled bridge invocation should resolve verified includes case-insensitively from memory");

    const auto external_process = invoke(outside_path);
    expect(external_process.exit_code == 4,
           "security-enabled bridge invocation should reject an external source path");
    expect(external_process.stdout_text.find(
               "error: Bridge routine source is missing from the package: copperfin_external_bridge_source.prg") !=
               std::string::npos,
           "external bridge-source rejection should use the localized package-boundary diagnostic");
    expect(external_process.stdout_text.find("bridge.return_value: 99") == std::string::npos,
           "security-enabled bridge invocation must not execute external source content");

    if (original_locale_dir.has_value()) {
        (void)copperfin::platform::write_environment_path(
            "COPPERFIN_LOCALE_DIR",
            *original_locale_dir);
    } else {
        (void)copperfin::platform::clear_environment_path("COPPERFIN_LOCALE_DIR");
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
        fs::remove(outside_path, ignored);
    }
}

void test_runtime_host_invokes_zero_argument_bridge_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_zero_arg_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeZeroArgExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-zero-arg-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-zero-arg-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should invoke zero-argument bridge exports through a bootstrap PRG");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should report bridge invocation mode for zero-argument exports");
    expect(process.stdout_text.find("bridge.library_export: GetAnswer") != std::string::npos,
           "runtime host should preserve zero-argument export metadata in diagnostics");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for zero-argument exports");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the zero-argument export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for zero-argument exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "zero-argument bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "zero-argument bridge export response should include the exported routine return value");
    expect(response_document.find("\"schema_version\": \"v1\"") != std::string::npos,
           "zero-argument bridge export response should echo the requested schema version");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_removes_bridge_routine_bootstrap_after_execution(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_bootstrap_cleanup_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeBootstrapCleanup\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-bootstrap-cleanup stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-bootstrap-cleanup stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should complete bridge routine invocation before bootstrap cleanup assertion");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for cleanup coverage");
    const std::string execution_source = output_line_value(process.stdout_text, "bridge.execution_source: ");
    expect(!execution_source.empty(),
           "runtime host should report the materialized bootstrap execution source");
    if (!execution_source.empty()) {
        expect(execution_source.find("copperfin_bridge_GetAnswer_") != std::string::npos,
               "runtime host should report the generated bridge bootstrap path");
        expect(!fs::exists(execution_source),
               "runtime host should remove the generated bridge bootstrap after execution");
    }
    expect(fs::exists(response_path),
           "runtime host should still write the bridge response after bootstrap cleanup");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_unescapes_bridge_descriptor_string_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_escaped_descriptor_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports\\escaped.prg";
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeEscapedDescriptor\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");

    const std::string escaped_source_path = json_escape_string(source_path.string());
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"GetAnswer\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + escaped_source_path + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"\",\n"
        "  \"parameter_count\": 0,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": []\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-escaped-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-escaped-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should decode escaped descriptor strings before bridge validation");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should reach routine bootstrap execution after escaped descriptor validation");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should execute the escaped-path descriptor source");
    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "escaped descriptor bridge response should include the exported routine return value");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_decodes_unicode_bridge_descriptor_paths(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        copperfin::platform::path_from_utf8_string(
            "copperfin_runtime_host_bridge_unicode_descriptor_tests-\xC3\xA9-\xF0\x9F\x9A\x80");
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_name = copperfin::platform::path_from_utf8_string(
        "exports-\xC3\xA9-\xF0\x9F\x9A\x80.prg");
    const fs::path source_path = temp_root / "content" / source_name;
    const fs::path request_path = temp_root / "GetAnswer.request.json";
    const fs::path response_path = temp_root / "nested" / "GetAnswer.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeUnicodeDescriptor\n"
        "startup_item=startup.prg\n"
        "startup_source=") + copperfin::platform::path_to_utf8_string(startup_path) + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE GetAnswer\n"
        "RETURN 42\n"
        "ENDPROC\n");

    const std::string source_path_utf8 = copperfin::platform::path_to_utf8_string(source_path);
    std::string escaped_source_path = json_escape_string(source_path_utf8);
    const std::string source_name_utf8 = copperfin::platform::path_to_utf8_string(source_name);
    const auto source_name_offset = escaped_source_path.find(source_name_utf8);
    expect(source_name_offset != std::string::npos,
           "Unicode bridge fixture should find its UTF-8 source name in the escaped path");
    if (source_name_offset == std::string::npos) {
        fs::remove_all(temp_root, ignored);
        return;
    }
    const std::string escaped_source_name = "exports-\\u00E9-\\uD83D\\uDE80.prg";
    escaped_source_path.replace(source_name_offset, source_name_utf8.size(), escaped_source_name);
    const auto write_request = [&](const std::string& encoded_source_path) {
        write_text(
            request_path,
            std::string("{\n"
            "  \"payload_shape\": \"bridge_request_v1\",\n"
            "  \"export_name\": \"GetAnswer\",\n"
            "  \"routine_kind\": \"procedure\",\n"
            "  \"source_path\": \"") + encoded_source_path + "\",\n"
            "  \"source_line\": 1,\n"
            "  \"parameter_declaration\": \"LPARAMETERS\",\n"
            "  \"parameter_names\": \"\",\n"
            "  \"parameter_count\": 0,\n"
            "  \"schema_version\": \"v1\",\n"
            "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
            "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
            "  \"parameters\": []\n"
            "}\n");
    };
    write_request(escaped_source_path);

    const std::vector<std::string> process_arguments{
        "--manifest", copperfin::platform::path_to_utf8_string(manifest_path),
        "--library-export", "GetAnswer",
        "--routine-kind", "procedure",
        "--source-path", source_path_utf8,
        "--source-line", "1",
        "--parameter-declaration", "LPARAMETERS",
        "--parameter-names", "",
        "--parameter-count", "0",
        "--request-path", copperfin::platform::path_to_utf8_string(request_path),
        "--response-path", copperfin::platform::path_to_utf8_string(response_path),
        "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
        "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
        "--schema-version", "v1"
    };
    const auto process = copperfin::test_support::run_process_capture(
        runtime_host_path,
        process_arguments,
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-unicode-descriptor exit code: " << process.exit_code << "\n";
        std::cerr << "bridge-unicode-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-unicode-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: [Unicode path omitted]\n";
    }
    std::cerr << "UNICODE: first invocation result captured\n";
    expect(process.exit_code == 0,
           "runtime host should decode Unicode bridge descriptor paths before validation");
    std::cerr << "UNICODE: first exit assertion complete\n";
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should execute a bridge source path decoded from Unicode escapes");
    std::cerr << "UNICODE: first output assertion complete\n";

    std::cerr << "UNICODE: writing malformed request\n";
    write_request(json_escape_string(source_path_utf8).replace(
        source_name_offset, source_name_utf8.size(), "exports-\\uD800.prg"));
    std::cerr << "UNICODE: malformed request written\n";
    const auto malformed_process = copperfin::test_support::run_process_capture(
        runtime_host_path,
        {
            "--manifest", copperfin::platform::path_to_utf8_string(manifest_path),
            "--library-export", "GetAnswer",
            "--routine-kind", "procedure",
            "--source-path", source_path_utf8,
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", copperfin::platform::path_to_utf8_string(request_path),
            "--response-path", copperfin::platform::path_to_utf8_string(response_path),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);
    if (malformed_process.exit_code != 6) {
        std::cerr << "bridge-unicode-malformed exit code: " << malformed_process.exit_code << "\n";
        std::cerr << "bridge-unicode-malformed stdout:\n" << malformed_process.stdout_text << "\n";
        std::cerr << "bridge-unicode-malformed stderr:\n" << malformed_process.stderr_text << "\n";
    }
    std::cerr << "UNICODE: malformed invocation result captured\n";
    expect(malformed_process.exit_code == 6,
           "runtime host should reject an unpaired Unicode bridge escape");
    std::cerr << "UNICODE: malformed exit assertion complete\n";
    expect(malformed_process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "malformed Unicode bridge escapes should preserve descriptor mismatch diagnostics");
    std::cerr << "UNICODE: malformed output assertion complete\n";

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_passes_bridge_request_parameters_to_export(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_export_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path startup_path = temp_root / "content" / "startup.prg";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterExport\n"
        "startup_item=startup.prg\n"
        "startup_source=") + startup_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(startup_path, "RETURN 7\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "bridge-parameter-export stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-export stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0,
           "runtime host should pass bridge request parameter values to exported routines");
    expect(process.stdout_text.find("bridge.routine_bootstrap: true") != std::string::npos,
           "runtime host should report routine bootstrap execution for parameterized exports");
    expect(process.stdout_text.find("bridge.parameter_count: 2") != std::string::npos,
           "runtime host should preserve the parameter count in diagnostics");
    expect(process.stdout_text.find("bridge.return_value: 42") != std::string::npos,
           "runtime host should report the parameterized export return value in diagnostics");
    expect(fs::exists(response_path),
           "runtime host should write the bridge response for parameterized exports");

    const std::string response_document = read_text(response_path);
    expect(response_document.find("\"status\": \"ok\"") != std::string::npos,
           "parameterized bridge export response should include ok status");
    expect(response_document.find("\"return_value\": \"42\"") != std::string::npos,
           "parameterized bridge export response should include the exported routine return value");
    expect(response_document.find("\"response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\"") != std::string::npos,
           "parameterized bridge export response should echo the expected response media type");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_count_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_mismatch_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "nested" / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeParameterMismatch\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight|tnExtra\",\n"
        "  \"parameter_count\": 3,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
        "  ]\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight|tnExtra",
            "--parameter-count", "3",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter count mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter count mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report bridge parameter count mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter counts mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_array_tests";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path source_path = temp_root / "content" / "exports.prg";
    const fs::path request_path = temp_root / "AddNumbers.request.json";
    const fs::path response_path = temp_root / "AddNumbers.response.json";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(source_path.parent_path());

    write_text(
        manifest_path,
        std::string("manifest_version=1\n"
        "project_title=BridgeNestedParameters\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(
        source_path,
        "PROCEDURE AddNumbers\n"
        "LPARAMETERS tnLeft, tnRight\n"
        "RETURN tnLeft + tnRight\n"
        "ENDPROC\n"
        "RETURN 7\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameter_shadow\": {\n"
        "    \"parameters\": [\n"
        "      {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "      {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
        "    ]\n"
        "  }\n"
        "}\n");

    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--library-export", "AddNumbers",
            "--routine-kind", "procedure",
            "--source-path", source_path.string(),
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "tnLeft|tnRight",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-array stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-array stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter arrays for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-array errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when no top-level parameter payload exists");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameters are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
