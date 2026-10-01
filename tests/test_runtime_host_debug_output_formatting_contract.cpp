// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_host_debug_output_support.h"

void test_runtime_host_surfaces_copy_omission_warning_metadata(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin_runtime_host_copy_omission_warning_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path source_path = temp_root / "copy_warning.prg";
    const fs::path table_path = temp_root / "source.dbf";
    const fs::path output_path = temp_root / "output.txt";
    const fs::path manifest_path = temp_root / "app.cfmanifest";
    const fs::path locale_root = temp_root / "locales";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "GENERAL", .type = 'G', .length = 4U},
        {.name = "CODE", .type = 'C', .length = 2U},
    };
    const auto created = copperfin::vfp::create_dbf_table_file(
        table_path.string(), fields, {{"", "OK"}});
    expect(created.ok, "runtime-host COPY warning fixture should be created");
    write_text(
        source_path,
        "USE '" + table_path.string() + "'\n"
        "COPY TO '" + output_path.string() + "' TYPE TAB FIELDS GENERAL, CODE\n"
        "RETURN\n");
    write_runtime_host_usage_catalogs(locale_root);
    write_text(
        manifest_path,
        "manifest_version=1\n"
        "project_title=CopyWarningMetadata\n"
        "startup_item=copy_warning.prg\n"
        "startup_source=" + source_path.string() + "\n"
        "working_directory=" + temp_root.string() + "\n"
        "security_enabled=false\n"
        "security_role=\n"
        "security_mode=native\n"
        "dotnet_story=none\n");

    ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
    ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
    const auto debug_process = run_process_capture(
        runtime_host_path, {"--manifest", manifest_path.string(), "--debug"}, temp_root);
    expect(debug_process.exit_code == 0, "runtime-host debug COPY warning should complete");
    expect(debug_process.stdout_text.find(
               "].metadata.warning_id: copy_to.omitted_fields.v1") != std::string::npos &&
           debug_process.stdout_text.find(
               "].metadata.output_type: TAB") != std::string::npos &&
           debug_process.stdout_text.find(
               "].metadata.omitted_field.0.name: GENERAL") != std::string::npos,
           "runtime-host debug protocol should expose structured COPY omission metadata");

    const auto cli_process = run_process_capture(
        runtime_host_path, {"--manifest", manifest_path.string()}, temp_root);
    expect(cli_process.exit_code == 0, "runtime-host headless COPY warning should complete");
    expect(cli_process.stdout_text.find(
               "].metadata.warning_id: copy_to.omitted_fields.v1") != std::string::npos &&
           cli_process.stdout_text.find(
               "].metadata.output_type: TAB") != std::string::npos,
           "runtime-host headless protocol should expose structured COPY omission metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

// #5729/#5730: a missing active #INCLUDE and an unbalanced conditional stop the program. The host
// must report `status: error` with the diagnostic, exit non-zero, and run nothing. The host test's
// minimal locale directory carries no runtime catalog, so the message shows as its localization key;
// the file-and-line placeholders are verified by test_prg_engine_preprocessor_diagnostics.
void test_runtime_host_rejects_missing_include_and_unbalanced_conditionals(const std::string& runtime_host_path) {
    namespace fs = std::filesystem;

    struct Case {
        std::string name;
        std::string main_source;
        std::string header_name;
        std::string header_source;
        std::string expected_text;
    };
    const std::vector<Case> cases = {
        {"missing_include", "#INCLUDE \"absent-header.h\"\nSTRTOFILE('continued', 'marker.txt')\nRETURN\n", "", "",
         "Runtime.Prg.Parser.Error.IncludeFileNotFound"},
        {"cross_file_endif", "#INCLUDE \"bad.h\"\nSTRTOFILE('should-run', 'marker.txt')\nRETURN\n", "bad.h",
         "#IFDEF NEVER_DEFINED\n", "Runtime.Prg.Parser.Error.UnterminatedConditional"},
        {"stray_endif", "x = 1\n#ENDIF\nSTRTOFILE('no', 'marker.txt')\nRETURN\n", "", "", "Runtime.Prg.Parser.Error.MismatchedConditional"},
    };
    for (const Case& c : cases) {
        const fs::path temp_root = fs::temp_directory_path() / ("copperfin_runtime_host_preprocessor_" + c.name);
        std::error_code ignored;
        fs::remove_all(temp_root, ignored);
        fs::create_directories(temp_root);
        const fs::path source_path = temp_root / "main.prg";
        const fs::path manifest_path = temp_root / "app.cfmanifest";
        const fs::path locale_root = temp_root / "locales";
        write_text(source_path, c.main_source);
        if (!c.header_name.empty()) {
            write_text(temp_root / c.header_name, c.header_source);
        }
        write_runtime_host_usage_catalogs(locale_root);
        write_text(
            manifest_path,
            "manifest_version=1\n"
            "project_title=PreprocessorStructure\n"
            "startup_item=main.prg\n"
            "startup_source=" + source_path.string() + "\n"
            "working_directory=" + temp_root.string() + "\n"
            "security_enabled=false\n"
            "security_role=\n"
            "security_mode=native\n"
            "dotnet_story=none\n");

        ScopedEnvironmentPath locale_dir("COPPERFIN_LOCALE_DIR", locale_root);
        ScopedEnvironmentValue locale("COPPERFIN_LOCALE", "en-US");
        const auto process = run_process_capture(runtime_host_path, {"--manifest", manifest_path.string()}, temp_root);
        expect(process.exit_code != 0, "runtime-host " + c.name + ": a structure error must exit non-zero");
        expect(process.stdout_text.find("status: error") != std::string::npos,
               "runtime-host " + c.name + ": should report status: error");
        expect(process.stdout_text.find(c.expected_text) != std::string::npos,
               "runtime-host " + c.name + ": the diagnostic should be " + c.expected_text + ", got: " + process.stdout_text);
        expect(process.stdout_text.find("runtime.completed: true") == std::string::npos,
               "runtime-host " + c.name + ": must not report runtime completion");
        expect(!fs::exists(temp_root / "marker.txt"), "runtime-host " + c.name + ": no statement may run");
        if (failures == 0) {
            fs::remove_all(temp_root, ignored);
        }
    }
}

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
