// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_host_debug_output_support.h"

void test_runtime_host_preserves_debug_state_across_prg_fault(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_debug_fault_recovery_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "fault_recovery.prg";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    write_text(
        source_path,
        "before_fault = \"kept\"\n"
        "fault_value = LOG(-1)\n"
        "after_fault = \"continued\"\n"
        "READ EVENTS\n");
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=DebugFaultRecovery\n"
        "startup_item=fault_recovery.prg\n"
        "startup_source=" + source_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto process = run_process_capture(
        runtime_host_path,
        {
            "--manifest", manifest_path.string(),
            "--debug",
            "--breakpoint", source_path.string() + ":2",
            "--debug-command", "continue",
            "--debug-command", "watch:before_fault",
            "--debug-command", "continue",
            "--debug-command", "watch:before_fault",
            "--debug-command", "continue",
            "--debug-command", "watch:after_fault"
        },
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "debug fault recovery stdout:\n" << process.stdout_text << "\n";
        std::cerr << "debug fault recovery stderr:\n" << process.stderr_text << "\n";
    }

    expect(process.exit_code == 0,
           "runtime-host debug fault recovery should exit cleanly after continuing the same session");
    expect(process.stdout_text.find("debug.breakpoint[0]: " + source_path.string() + ":2") != std::string::npos,
           "runtime-host debug fault recovery should preserve the breakpoint inventory");
    expect(process.stdout_text.find("debug.reason: breakpoint") != std::string::npos,
           "runtime-host debug fault recovery should pause at the configured breakpoint");
    expect(process.stdout_text.find("status: error") != std::string::npos,
           "runtime-host debug fault recovery should expose a structured runtime fault");
    expect(process.stdout_text.find("debug.reason: error") != std::string::npos,
           "runtime-host debug fault recovery should preserve the error pause reason");
    expect(process.stdout_text.find("debug.location: " + source_path.string() + ":2") != std::string::npos,
           "runtime-host debug fault recovery should preserve the faulting source location");
    expect(process.stdout_text.find("debug.frame[0]: main@" + source_path.string() + ":2") != std::string::npos,
           "runtime-host debug fault recovery should preserve the faulting stack frame");
    expect(process.stdout_text.find("debug.watch.value: kept") != std::string::npos,
           "runtime-host debug fault recovery should keep watch evaluation available at the fault pause");
    expect(process.stdout_text.find("debug.reason: event_loop") != std::string::npos,
           "runtime-host debug fault recovery should return to the event loop after continue");
    expect(process.stdout_text.find("debug.watch.value: continued") != std::string::npos,
           "runtime-host debug fault recovery should execute post-fault code in the same session");
    expect(process.stdout_text.find("terminate called") == std::string::npos,
           "runtime-host debug fault recovery should not terminate the host process");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_contains_unexpected_process_fault(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_unexpected_fault_containment_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    const fs::path locale_root = temp_root / "locales";
    write_runtime_host_usage_catalogs(locale_root);

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    ScopedEnvironmentValue injected_fault("COPPERFIN_TEST_THROW_RUNTIME_HOST", "1");
    const auto process = run_process_capture(
        runtime_host_path,
        {"--manifest", (temp_root / "unused.cfmanifest").string()},
        temp_root);

    expect(process.exit_code == 5,
           "unexpected runtime-host exceptions should use the runtime error exit code");
    expect(process.stdout_text.find("status: error") != std::string::npos,
           "unexpected runtime-host exceptions should preserve machine-readable error status");
    expect(process.stdout_text.find(
               "error: Runtime host fault was contained: test-injected host fault") != std::string::npos,
           "unexpected runtime-host exceptions should emit the localized containment diagnostic");
    expect(process.stdout_text.find("terminate called") == std::string::npos,
           "contained runtime-host exceptions should not terminate without a diagnostic");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_parameter_value_tests";
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
        "project_title=BridgeNestedParameterValues\n"
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
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value_shadow\": {\"value\": \"40\"}, \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value_shadow\": {\"value\": \"2\"}, \"surface\": \"int\"}\n"
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

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-parameter-values stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-parameter-values stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge parameter values for nonzero arity");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested parameter-value errors");
    expect(process.stdout_text.find("error: Bridge request parameter count mismatch.") != std::string::npos,
           "runtime host should report a parameter count mismatch when parameter values are nested");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when nonzero bridge parameter values are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_parameter_name_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_parameter_name_mismatch_tests";
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
        "project_title=BridgeParameterNameMismatch\n"
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
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnRight\", \"value\": \"40\", \"surface\": \"dll-int\"},\n"
        "    {\"name\": \"tnLeft\", \"value\": \"2\", \"surface\": \"dll-int\"}\n"
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

    if (process.exit_code != 6) {
        std::cerr << "bridge-parameter-name-mismatch stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-parameter-name-mismatch stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge parameter name mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on parameter name mismatches");
    expect(process.stdout_text.find("error: Bridge request parameter name mismatch.") != std::string::npos,
           "runtime host should report bridge parameter name mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge parameter names mismatch");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_request_contract_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_request_contract_tests";
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
        "project_title=BridgeRequestContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        "{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.bad-request+json\",\n"
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
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-request-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-request-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge request media-type mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on request contract errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should report the request media-type mismatch");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when the request contract mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_nested_bridge_descriptor_fields(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_nested_descriptor_tests";
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
        "project_title=BridgeNestedDescriptor\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"descriptor_shadow\": {\n"
        "    \"export_name\": \"AddNumbers\",\n"
        "    \"routine_kind\": \"procedure\",\n"
        "    \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "    \"parameter_count\": 0,\n"
        "    \"schema_version\": \"v1\",\n"
        "    \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\"\n"
        "  },\n"
        "  \"parameters\": []\n"
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
            "--parameter-names", "",
            "--parameter-count", "0",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-nested-descriptor stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-nested-descriptor stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject nested bridge descriptor fields before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on nested descriptor errors");
    expect(process.stdout_text.find("error: Bridge request media type mismatch.") != std::string::npos,
           "runtime host should not accept nested request-media fields as top-level contract fields");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when bridge descriptor fields are nested");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_identity_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_contract_tests";
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
        "project_title=BridgeDescriptorContract\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"WrongExport\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"LPARAMETERS\",\n"
        "  \"parameter_names\": \"left,right\",\n"
        "  \"parameter_count\": 2,\n"
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
            "--source-line", "1",
            "--parameter-declaration", "LPARAMETERS",
            "--parameter-names", "left,right",
            "--parameter-count", "2",
            "--request-path", request_path.string(),
            "--response-path", response_path.string(),
            "--request-media-type", "application/vnd.copperfin.runtime-bridge-request+json",
            "--response-media-type", "application/vnd.copperfin.runtime-bridge-response+json",
            "--schema-version", "v1"
        },
        temp_root);

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-contract stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-contract stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor identity mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor contract errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor identity mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor identity mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_runtime_host_rejects_bridge_descriptor_metadata_mismatch(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_host_bridge_descriptor_metadata_tests";
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
        "project_title=BridgeDescriptorMetadata\n"
        "startup_item=exports.prg\n"
        "startup_source=") + source_path.string() + "\n"
        "security_enabled=false\n"
        "dotnet_story=none\n");
    write_text(source_path, "RETURN 42\n");
    write_text(
        request_path,
        std::string("{\n"
        "  \"payload_shape\": \"bridge_request_v1\",\n"
        "  \"export_name\": \"AddNumbers\",\n"
        "  \"routine_kind\": \"procedure\",\n"
        "  \"source_path\": \"") + json_escape_string(source_path.string()) + "\",\n"
        "  \"source_line\": 1,\n"
        "  \"parameter_declaration\": \"PARAMETERS\",\n"
        "  \"parameter_names\": \"tnLeft|tnRight\",\n"
        "  \"parameter_count\": 2,\n"
        "  \"schema_version\": \"v1\",\n"
        "  \"request_media_type\": \"application/vnd.copperfin.runtime-bridge-request+json\",\n"
        "  \"expected_response_media_type\": \"application/vnd.copperfin.runtime-bridge-response+json\",\n"
        "  \"parameters\": [\n"
        "    {\"name\": \"tnLeft\", \"value\": \"40\", \"surface\": \"int\"},\n"
        "    {\"name\": \"tnRight\", \"value\": \"2\", \"surface\": \"int\"}\n"
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

    if (process.exit_code != 6) {
        std::cerr << "bridge-descriptor-metadata stdout:\n" << process.stdout_text << "\n";
        std::cerr << "bridge-descriptor-metadata stderr:\n" << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 6,
           "runtime host should reject bridge descriptor metadata mismatches before execution");
    expect(process.stdout_text.find("runtime.mode: bridge-invocation") != std::string::npos,
           "runtime host should keep bridge mode visible on descriptor metadata errors");
    expect(process.stdout_text.find("error: Bridge request descriptor mismatch.") != std::string::npos,
           "runtime host should report descriptor metadata mismatches");
    expect(!fs::exists(response_path),
           "runtime host should not write a success response when descriptor metadata mismatches");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
