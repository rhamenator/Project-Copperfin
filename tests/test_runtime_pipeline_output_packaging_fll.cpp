// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_pipeline_output_packaging_support.h"

namespace cf_test_runtime_pipeline {

void test_fll_output_package_emits_api_manifest_from_prg_routines() {
    namespace fs = std::filesystem;
    const fs::path temp_root = fs::temp_directory_path() / "copperfin_runtime_pipeline_fll_contract";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "librarymain.prg",
               "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(project_dir / "helper.prg",
               "FUNCTION AddNumbers\nPARAMETERS tnLeft, tnRight\nRETURN 1\nENDFUNC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "librarydemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LibraryDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LibraryDemo";
    workspace.build_plan.output_path = (output_dir / "LibraryDemo.fll").string();
    workspace.build_plan.output_kind = "fll";
    workspace.build_plan.build_target = "x64 Visual FoxPro library";
    workspace.build_plan.startup_item = "librarymain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "librarymain.prg", .relative_path = "librarymain.prg", .type_title = "Program"},
        {.record_index = 2U, .name = "helper.prg", .relative_path = "helper.prg", .type_title = "Program"}
    };

    const auto plan = copperfin::runtime::create_runtime_package_plan(
        document,
        workspace,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        output_dir.string(),
        copperfin::runtime::BuildConfiguration::debug,
        false,
        true);
    const std::string librarymain_source = plan.assets.at(0U).source_path;
    const std::string helper_source = plan.assets.at(1U).source_path;

    expect(plan.ok, "fll-output plan should be created");
    expect(plan.output_kind == copperfin::runtime::BuildOutputKind::fll,
           "fll-output plan should preserve FLL output kind");
    expect(!plan.emit_dotnet_launcher,
           "fll-output plan should not route through .NET launcher emission");
    expect(plan.launcher_mode == "foxpro_library_definition",
           "fll-output plan should switch to the library-definition packaging mode");
    expect(plan.launcher_fallback == "library_binary_generation_pending",
           "fll-output plan should record the honest non-binary fallback state");
    expect(fs::path(plan.launcher_output_path).filename() == "LibraryDemo.fll",
           "fll-output plan should preserve the requested output filename");
    expect(fs::path(plan.module_definition_path).filename() == "LibraryDemo.def",
           "fll-output plan should derive a matching module-definition filename");
    expect(fs::path(plan.native_wrapper_source_path).filename() == "LibraryDemo_wrapper.cpp",
           "fll-output plan should derive a matching native-wrapper source filename");
    expect(fs::path(plan.native_wrapper_cmake_path).filename() == "CMakeLists.txt",
           "fll-output plan should derive a native-wrapper CMake filename");
    expect(fs::path(plan.native_wrapper_build_script_path).filename() == "build_wrapper.sh",
           "fll-output plan should derive a native-wrapper shell build script filename");
    expect(fs::path(plan.native_wrapper_build_powershell_path).filename() == "build_wrapper.ps1",
           "fll-output plan should derive a native-wrapper PowerShell build script filename");
    expect(fs::path(plan.fll_api_manifest_path).filename() == "LibraryDemo.fll.api",
           "fll-output plan should derive a matching API-manifest filename");
    expect(plan.exported_symbols.size() == 2U,
           "fll-output plan should discover routine exports from PRG assets");

    const auto result = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());

    expect_materialization(result, "fll-output package should materialize");
    if (result.ok) {
        expect(fs::exists(result.plan.module_definition_path),
               "fll-output package should emit a module-definition file");
        expect(fs::exists(result.plan.native_wrapper_source_path),
               "fll-output package should emit a native-wrapper source scaffold");
        expect(fs::exists(result.plan.native_wrapper_cmake_path),
               "fll-output package should emit native-wrapper build metadata");
        expect(fs::exists(result.plan.native_wrapper_build_script_path),
               "fll-output package should emit a native-wrapper shell build script");
        expect(fs::exists(result.plan.native_wrapper_build_powershell_path),
               "fll-output package should emit a native-wrapper PowerShell build script");
        expect(fs::exists(result.plan.fll_api_manifest_path),
               "fll-output package should emit an API manifest");
        expect(!fs::exists(result.plan.launcher_output_path),
               "fll-output package should not fake an FLL binary");
        expect(fs::exists(result.plan.runtime_host_destination_path),
               "fll-output package should stage the runtime host beside the generated wrapper contract");
        expect(read_text(result.plan.runtime_host_destination_path) == "runtime-host",
               "fll-output package should preserve the runtime host payload bytes");
        expect(!result.plan.primary_output_materialized,
               "fll-output package should report that the primary FLL binary is not yet materialized");

        const std::string module_definition = read_text(result.plan.module_definition_path);
        expect(module_definition.find("LIBRARY LibraryDemo") != std::string::npos,
               "fll-output module-definition file should declare the library name");
        expect(module_definition.find("InitLibrary") != std::string::npos,
               "fll-output module-definition file should export discovered procedure names");
        expect(module_definition.find("AddNumbers") != std::string::npos,
               "fll-output module-definition file should export discovered function names");
        const std::string wrapper_source = read_text(result.plan.native_wrapper_source_path);
        expect(wrapper_source.find("Generated Copperfin native wrapper scaffold") != std::string::npos,
               "fll-output wrapper source should identify the generated scaffold");
        expect(wrapper_source.find("static std::filesystem::path copperfin_wrapper_module_path(void* symbol_address)") != std::string::npos,
               "fll-output wrapper source should derive its loaded module path");
        expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_manifest_path(void* symbol_address)") != std::string::npos,
               "fll-output wrapper source should derive a sibling manifest path");
        expect(wrapper_source.find("static std::filesystem::path copperfin_runtime_host_path(void* symbol_address)") != std::string::npos,
               "fll-output wrapper source should derive a sibling runtime-host path");
        expect(wrapper_source.find("runtime_host_sha256") != std::string::npos,
               "fll-output wrapper source should read the packaged runtime-host digest");
        expect(wrapper_source.find("copperfin_runtime_bridge_sha256_bytes") != std::string::npos,
               "fll-output wrapper source should carry a self-contained runtime-host hash verifier");
        expect(wrapper_source.find("copperfin_runtime_bridge_read_verified_host") != std::string::npos,
               "fll-output wrapper source should verify the sibling host before launch");
        expect(wrapper_source.find("copperfin_runtime_bridge_sha256_bytes") <
                   wrapper_source.find("copperfin_runtime_bridge_read_verified_host"),
               "fll-output wrapper source should emit host digest verification before verified-host admission");
        expect(wrapper_source.find("FILE_FLAG_OPEN_REPARSE_POINT") != std::string::npos,
               "fll-output wrapper source should reject Windows reparse-point host redirection");
        expect(wrapper_source.find("O_NOFOLLOW") != std::string::npos,
               "fll-output wrapper source should reject POSIX symlink host redirection");
        expect(wrapper_source.find("fexecve(verified_runtime_host") != std::string::npos,
               "fll-output wrapper source should execute the verified Linux host descriptor");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeDescriptor") != std::string::npos,
               "fll-output wrapper source should declare a shared bridge-descriptor surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeDescriptor copperfin_build_runtime_bridge_descriptor(") != std::string::npos,
               "fll-output wrapper source should declare a bridge-descriptor helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeInvocation") != std::string::npos,
               "fll-output wrapper source should declare a shared bridge-invocation surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeInvocation copperfin_build_runtime_bridge_invocation(") != std::string::npos,
               "fll-output wrapper source should declare a bridge-invocation helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
               "fll-output wrapper source should declare manifest flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_manifest_flag()") != std::string::npos,
               "fll-output wrapper source should route manifest flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
               "fll-output wrapper source should declare library-export flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_library_export_flag()") != std::string::npos,
               "fll-output wrapper source should route library-export flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
               "fll-output wrapper source should declare routine-kind flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_flag()") != std::string::npos,
               "fll-output wrapper source should route routine-kind flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
               "fll-output wrapper source should declare source-path flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_source_path_flag()") != std::string::npos,
               "fll-output wrapper source should route source-path flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
               "fll-output wrapper source should declare source-line flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_source_line_flag()") != std::string::npos,
               "fll-output wrapper source should route source-line flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
               "fll-output wrapper source should declare parameter-declaration flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_parameter_declaration_flag()") != std::string::npos,
               "fll-output wrapper source should route parameter-declaration flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
               "fll-output wrapper source should declare parameter-names flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_parameter_names_flag()") != std::string::npos,
               "fll-output wrapper source should route parameter-names flag through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
               "fll-output wrapper source should declare parameter-count flag helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_flag()") != std::string::npos,
               "fll-output wrapper source should route parameter-count flag through helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeParameter") != std::string::npos,
               "fll-output wrapper source should declare a bridge-parameter surface");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeCall") != std::string::npos,
               "fll-output wrapper source should declare a bridge-call surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeCall copperfin_build_runtime_bridge_call(") != std::string::npos,
               "fll-output wrapper source should declare a bridge-call helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturn") != std::string::npos,
               "fll-output wrapper source should declare a return-binding surface");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeResult") != std::string::npos,
               "fll-output wrapper source should declare a bridge-result surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResult copperfin_build_runtime_bridge_result(") != std::string::npos,
               "fll-output wrapper source should declare a bridge-result helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturn copperfin_build_runtime_bridge_placeholder_return_binding(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder return-binding helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeEnvironmentVariable") != std::string::npos,
               "fll-output wrapper source should declare a launch-environment surface");
        expect(wrapper_source.find("std::vector<CopperfinRuntimeBridgeEnvironmentVariable> environment;") != std::string::npos,
               "fll-output wrapper source should carry launch environment entries through dispatch and launch surfaces.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeLaunchPlan") != std::string::npos,
               "fll-output wrapper source should declare a launch-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeLaunchPlan copperfin_build_runtime_bridge_launch_plan(") != std::string::npos,
               "fll-output wrapper source should declare a launch-plan helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
               "fll-output wrapper source should declare library-export env-var helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_library_export_env_var()") != std::string::npos,
               "fll-output wrapper source should route library-export env-var through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
               "fll-output wrapper source should declare routine-kind env-var helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_routine_kind_env_var()") != std::string::npos,
               "fll-output wrapper source should route routine-kind env-var through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
               "fll-output wrapper source should declare source-path env-var helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_source_path_env_var()") != std::string::npos,
               "fll-output wrapper source should route source-path env-var through helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
               "fll-output wrapper source should declare parameter-count env-var helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_parameter_count_env_var()") != std::string::npos,
               "fll-output wrapper source should route parameter-count env-var through helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeObservationPlan") != std::string::npos,
               "fll-output wrapper source should declare an observation-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeObservationPlan copperfin_build_runtime_bridge_observation_plan(") != std::string::npos,
               "fll-output wrapper source should declare an observation-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeExecutionPlan") != std::string::npos,
               "fll-output wrapper source should declare an execution-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeExecutionPlan copperfin_build_runtime_bridge_execution_plan(") != std::string::npos,
               "fll-output wrapper source should declare an execution-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeTransportPlan") != std::string::npos,
               "fll-output wrapper source should declare a transport-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeTransportPlan copperfin_build_runtime_bridge_transport_plan(") != std::string::npos,
               "fll-output wrapper source should declare a transport-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeSerializationPlan") != std::string::npos,
               "fll-output wrapper source should declare a serialization-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeSerializationPlan copperfin_build_runtime_bridge_serialization_plan(") != std::string::npos,
               "fll-output wrapper source should declare a serialization-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeDispatchPlan") != std::string::npos,
               "fll-output wrapper source should declare a dispatch-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchPlan copperfin_build_runtime_bridge_dispatch_plan(") != std::string::npos,
               "fll-output wrapper source should declare a dispatch-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeDispatchExecution copperfin_runtime_bridge_execute_dispatch(") != std::string::npos,
               "fll-output wrapper source should declare a shared dispatch-execution helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(") != std::string::npos,
               "fll-output wrapper source should declare a shared process-launch helper.");
        const auto launch_declaration = wrapper_source.find(
            "static CopperfinRuntimeBridgeProcessLaunch copperfin_runtime_bridge_launch_process(\n"
            "    const CopperfinRuntimeBridgeDispatchExecution& dispatch_execution);\n");
        const auto launch_test_hook = wrapper_source.find(
            "COPPERFIN_EXPORT int copperfin_runtime_bridge_test_launch_environment(");
        expect(launch_declaration != std::string::npos &&
                   launch_test_hook != std::string::npos &&
                   launch_declaration < launch_test_hook,
               "fll-output wrapper source should declare process launch before its test hook on every platform.");
        expect(wrapper_source.find("#include <windows.h>") != std::string::npos &&
                   wrapper_source.find("#include <unistd.h>") != std::string::npos,
               "fll-output wrapper source should include native process-launch support.");
        expect(wrapper_source.find("launch_plan.environment") != std::string::npos,
               "fll-output wrapper source should carry launch environment entries into dispatch execution.");
        expect(wrapper_source.find("const auto environment_entries = copperfin_runtime_bridge_windows_environment(dispatch_execution.environment);") != std::string::npos &&
                   wrapper_source.find("const auto environment_values = copperfin_runtime_bridge_posix_environment(dispatch_execution.environment);") != std::string::npos,
               "fll-output wrapper source should apply launch environment entries through native process APIs.");
        expect(wrapper_source.find(
                   "    environment_block.push_back(L'\\0');\n"
                   "    if (environment_entries.empty()) {\n"
                   "        environment_block.push_back(L'\\0');\n"
                   "    }\n") != std::string::npos,
               "fll-output Windows environment blocks should double-terminate the empty-environment case.");
        expect(wrapper_source.find("#include <mutex>") != std::string::npos &&
                   wrapper_source.find("static std::mutex environment_mutex;") != std::string::npos &&
                   wrapper_source.find("const std::lock_guard<std::mutex> environment_lock(environment_mutex);") != std::string::npos,
               "fll-output POSIX environment snapshots should use a bounded synchronization boundary.");
        expect(wrapper_source.find("CreateProcessW(") != std::string::npos &&
                   wrapper_source.find("execve(") != std::string::npos &&
                   wrapper_source.find("std::system(") == std::string::npos &&
                   wrapper_source.find("copperfin_runtime_bridge_build_process_command(") == std::string::npos,
               "fll-output wrapper source should launch without shell command construction.");
        expect(wrapper_source.find("STARTUPINFOEXW startup_info{};") != std::string::npos &&
                   wrapper_source.find("PROC_THREAD_ATTRIBUTE_HANDLE_LIST") != std::string::npos &&
                   wrapper_source.find("UpdateProcThreadAttribute(") != std::string::npos &&
                   wrapper_source.find("DuplicateHandle(") != std::string::npos &&
                   wrapper_source.find("SetHandleInformation(duplicate, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)") != std::string::npos,
               "fll-output Windows bridge should restrict child handle inheritance to duplicated standard streams.");
        expect(wrapper_source.find("const bool launch_succeeded = launch_attempted && process_created && exit_code == dispatch_execution.expected_exit_code;") != std::string::npos,
               "fll-output wrapper source should compare runtime-host exit code with the expected exit code.");
        expect(wrapper_source.find("        false,\n        false,\n        dispatch_execution.expected_exit_code,\n        dispatch_execution.expected_exit_code") == std::string::npos,
               "fll-output wrapper source should not keep the deterministic process-launch failure placeholder.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeHostFailureEvaluation copperfin_runtime_bridge_evaluate_host_failure(") != std::string::npos,
               "fll-output wrapper source should declare a shared host-failure evaluation helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeMissingResponseEvaluation copperfin_runtime_bridge_evaluate_missing_response(") != std::string::npos,
               "fll-output wrapper source should declare a shared missing-response evaluation helper.");
        expect(wrapper_source.find("const CopperfinRuntimeBridgeResponseReadPlan& response_read_plan,\n    const std::string& response_document) {") != std::string::npos,
               "fll-output wrapper source should pass response documents into missing-response evaluation.");
        expect(wrapper_source.find("const bool response_missing = response_read_plan.require_existing_response && response_document.empty();") != std::string::npos,
               "fll-output wrapper source should detect missing responses from the read response document.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationEvaluation copperfin_runtime_bridge_evaluate_response_validation(") != std::string::npos,
               "fll-output wrapper source should declare a shared response-validation evaluation helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_response_document_has_required_fields(") != std::string::npos,
               "fll-output wrapper source should declare a required response-field validation helper.");
        expect(wrapper_source.find("response_validation_plan.required_response_fields") != std::string::npos,
               "fll-output wrapper source should validate the declared required response fields.");
        expect(wrapper_source.find("!required_response_fields_present") != std::string::npos,
               "fll-output wrapper source should fail response validation when required response fields are absent.");
        expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_response_media_type_field_name())") != std::string::npos,
               "fll-output wrapper source should read response media type during response validation.");
        expect(wrapper_source.find("response_media_type == response_validation_plan.expected_response_media_type") != std::string::npos,
               "fll-output wrapper source should compare response media type with the expected response media type.");
        expect(wrapper_source.find("!response_media_type_matches") != std::string::npos,
               "fll-output wrapper source should fail response validation when response media type mismatches.");
        expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(\n        response_document,\n        copperfin_build_runtime_bridge_schema_version_field_name())") != std::string::npos,
               "fll-output wrapper source should read response schema version during response validation.");
        expect(wrapper_source.find("response_schema_version == response_validation_plan.expected_schema_version") != std::string::npos,
               "fll-output wrapper source should compare response schema version with the expected schema version.");
        expect(wrapper_source.find("!response_schema_version_matches") != std::string::npos,
               "fll-output wrapper source should fail response validation when response schema version mismatches.");
        expect(wrapper_source.find("bool response_document_available = false;") != std::string::npos,
               "fll-output wrapper source should track response-document availability in response-validation evaluation.");
        expect(wrapper_source.find("const std::string& response_document) {") != std::string::npos,
               "fll-output wrapper source should pass response documents into response-validation evaluation.");
        expect(wrapper_source.find("const bool response_document_available = !response_document.empty();") != std::string::npos,
               "fll-output wrapper source should derive response-document availability during response-validation evaluation.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgePayloadPlan") != std::string::npos,
               "fll-output wrapper source should declare a payload-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgePayloadPlan copperfin_build_runtime_bridge_payload_plan(") != std::string::npos,
               "fll-output wrapper source should declare a payload-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretationPlan") != std::string::npos,
               "fll-output wrapper source should declare an interpretation-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretationPlan copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
               "fll-output wrapper source should declare an interpretation-plan helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeFailurePolicyPlan") != std::string::npos,
               "fll-output wrapper source should declare a failure-policy surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeFailurePolicyPlan copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
               "fll-output wrapper source should declare a failure-policy helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-status field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-value field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-diagnostics field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared request payload-shape helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response payload-shape helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared export-name field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared routine-kind field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared source-path field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared source-line field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-declaration field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-names field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-count field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared schema-version field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameters field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared request-media-type field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared request-fields contract helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared expected-response media-type helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-fields contract helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-media-type field helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared payload-shape field helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_payload_shape_field_name()") != std::string::npos,
               "fll-output wrapper source should route payload-shape field through helper in request document");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-name field helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_name_field_name()") != std::string::npos,
               "fll-output wrapper source should route parameter-name field through helper in request document");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-value field helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_value_field_name()") != std::string::npos,
               "fll-output wrapper source should route parameter-value field through helper in request document");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared parameter-surface field helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_surface_field_name()") != std::string::npos,
               "fll-output wrapper source should route parameter-surface field through helper in request document");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared failure-diagnostics token helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared success-status token helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseValidationPlan") != std::string::npos,
               "fll-output wrapper source should declare a response-validation surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseValidationPlan copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
               "fll-output wrapper source should declare a response-validation helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestArtifact") != std::string::npos,
               "fll-output wrapper source should declare a request-artifact surface");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_document(") != std::string::npos,
               "fll-output wrapper source should declare a request-document helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestArtifact copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
               "fll-output wrapper source should declare a request-artifact helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeRequestWritePlan") != std::string::npos,
               "fll-output wrapper source should declare a request-write-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeRequestWritePlan copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
               "fll-output wrapper source should declare a request-write-plan helper");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_execute_write_request(") != std::string::npos,
               "fll-output wrapper source should declare a shared request-write execution helper.");
        expect(wrapper_source.find("out << plan.request_artifact.request_document;") != std::string::npos,
               "fll-output wrapper source should stage request-document writes through the shared request-write execution helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseReadPlan") != std::string::npos,
               "fll-output wrapper source should declare a response-read-plan surface");
        expect(wrapper_source.find("bool request_write_succeeded = false;") != std::string::npos,
               "fll-output wrapper source should carry request-write success on the response-read plan.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseReadPlan copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
               "fll-output wrapper source should declare a response-read-plan helper");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_execute_read_response(") != std::string::npos,
               "fll-output wrapper source should declare a shared response-read execution helper.");
        expect(wrapper_source.find("if (!plan.request_write_succeeded)") != std::string::npos,
               "fll-output wrapper source should fall back when request writing failed before reading a response.");
        expect(wrapper_source.find("response_document << input.rdbuf();") != std::string::npos,
               "fll-output wrapper source should stage response-document reads through the shared response-read execution helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseArtifact") != std::string::npos,
               "fll-output wrapper source should declare a response-artifact surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseArtifact copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
               "fll-output wrapper source should declare a response-artifact helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeResponseParsePlan") != std::string::npos,
               "fll-output wrapper source should declare a response-parse-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParsePlan copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
               "fll-output wrapper source should declare a response-parse-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeResponseParseAdmission copperfin_runtime_bridge_admit_response_parse(") != std::string::npos,
               "fll-output wrapper source should declare a shared response-parse admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeParsedResponse copperfin_runtime_bridge_execute_parse_response(") != std::string::npos,
               "fll-output wrapper source should declare a shared response-parse execution helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_extract_json_field(response_document, plan.status_field)") != std::string::npos,
               "fll-output wrapper source should stage response field extraction through the shared response-parse execution helper.");
        expect(wrapper_source.find("object_depth == 1U && array_depth == 0U") != std::string::npos,
               "fll-output wrapper source should validate required response fields as top-level object fields");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_find_json_field_value_start(") != std::string::npos,
               "fll-output wrapper source should declare a shared response field-value scanner");
        expect(wrapper_source.find("copperfin_runtime_bridge_find_json_field_value_start(response_document, field_name, value_start)") != std::string::npos,
               "fll-output wrapper source should extract response field values through the shared scanner");
        expect(wrapper_source.find("response_document.compare(index, field_token.size(), field_token) == 0") != std::string::npos,
               "fll-output wrapper source should compare required response fields through the scanner token match");
        expect(wrapper_source.find("return response_document.find(field_token) != std::string::npos;") == std::string::npos,
               "fll-output wrapper source should not validate required response fields with raw token search");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeInterpretedResultPlan") != std::string::npos,
               "fll-output wrapper source should declare an interpreted-result-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultPlan copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
               "fll-output wrapper source should declare an interpreted-result-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResultAdmission copperfin_runtime_bridge_admit_interpreted_result(") != std::string::npos,
               "fll-output wrapper source should declare a shared interpreted-result admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeInterpretedResult copperfin_runtime_bridge_execute_interpreted_result(") != std::string::npos,
               "fll-output wrapper source should declare a shared interpreted-result execution helper.");
        expect(wrapper_source.find("parsed_response.status_value == plan.success_status_value") != std::string::npos,
               "fll-output wrapper source should stage interpreted-result selection through the shared execution helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeNativeReturnPlan") != std::string::npos,
               "fll-output wrapper source should declare a native-return-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnPlan copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
               "fll-output wrapper source should declare a native-return-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturnAdmission copperfin_runtime_bridge_admit_native_return(") != std::string::npos,
               "fll-output wrapper source should declare a shared native-return admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeNativeReturn copperfin_runtime_bridge_execute_native_return(") != std::string::npos,
               "fll-output wrapper source should declare a shared native-return execution helper.");
        expect(wrapper_source.find("interpreted_result.matched_success_status") != std::string::npos,
               "fll-output wrapper source should stage native-return selection through the shared execution helper.");
        expect(wrapper_source.find("static int copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
               "fll-output wrapper source should declare an integer return-representation parser");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_parse_json_string_at(") != std::string::npos,
               "fll-output wrapper source should declare a JSON string escape decoder for response parsing");
        expect(wrapper_source.find("copperfin_runtime_bridge_parse_json_string_at(response_document, value_start, string_end, decoded_value)") != std::string::npos,
               "fll-output wrapper source should decode escaped response string fields before interpreting returns");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_default_int_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared parsed-int default sentinel helper");
        expect(wrapper_source.find("copperfin_runtime_bridge_default_int_value()") != std::string::npos,
               "fll-output wrapper source should route the parsed-int default sentinel through the shared helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeOutcomeSelectionPlan") != std::string::npos,
               "fll-output wrapper source should declare an outcome-selection-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionPlan copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
               "fll-output wrapper source should declare an outcome-selection-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelectionAdmission copperfin_runtime_bridge_admit_outcome_selection(") != std::string::npos,
               "fll-output wrapper source should declare a shared outcome-selection admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeOutcomeSelection copperfin_runtime_bridge_execute_outcome_selection(") != std::string::npos,
               "fll-output wrapper source should declare a shared outcome-selection execution helper.");
        expect(wrapper_source.find("native_return.matched_success_status") != std::string::npos,
               "fll-output wrapper source should stage outcome selection through the shared execution helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnMaterializationPlan") != std::string::npos,
               "fll-output wrapper source should declare a return-materialization-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationPlan copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
               "fll-output wrapper source should declare a return-materialization-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterializationAdmission copperfin_runtime_bridge_admit_return_materialization(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-materialization admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnMaterialization copperfin_runtime_bridge_execute_return_materialization(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-materialization execution helper.");
        expect(wrapper_source.find("const auto& outcome_selection = plan.outcome_selection") != std::string::npos,
               "fll-output wrapper source should consume explicit outcome selection while materializing returns.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
               "fll-output wrapper source should declare a shared native-int return-surface helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_native_int_return_surface()") != std::string::npos,
               "fll-output wrapper source should route native-int return-surface comparisons through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_native_int_placeholder_signature_token()") != std::string::npos,
               "fll-output wrapper source should declare a shared native-int placeholder-signature helper.");
        expect(wrapper_source.find("find(copperfin_build_runtime_bridge_native_int_placeholder_signature_token())") != std::string::npos,
               "fll-output wrapper source should route native-int placeholder-signature matching through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
               "fll-output wrapper source should declare a shared native return-statement framing helper.");
        expect(wrapper_source.find("return copperfin_build_runtime_bridge_return_statement_from_expression(") != std::string::npos,
               "fll-output wrapper source should route native return-statement framing through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
               "fll-output wrapper source should declare a shared typed native return-expression helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_typed_native_return_expression(") != std::string::npos,
               "fll-output wrapper source should route typed native return-expression construction through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
               "fll-output wrapper source should declare a shared stdout log-file suffix helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
               "fll-output wrapper source should route stdout log-file suffix through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
               "fll-output wrapper source should declare a shared stderr log-file suffix helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
               "fll-output wrapper source should route stderr log-file suffix through the shared helper.");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
               "fll-output wrapper source should declare a shared expected-exit-code helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_expected_exit_code()") != std::string::npos,
               "fll-output wrapper source should route expected-exit-code through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should declare a shared request artifact suffix helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should route request artifact suffix through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should declare a shared response artifact suffix helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should route response artifact suffix through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared activates-adopted-return policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_activates_adopted_return_policy()") != std::string::npos,
               "fll-output wrapper source should route activates-adopted-return policy through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared capture-stdout policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_capture_stdout_policy()") != std::string::npos,
               "fll-output wrapper source should route capture-stdout policy through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared capture-stderr policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_capture_stderr_policy()") != std::string::npos,
               "fll-output wrapper source should route capture-stderr policy through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared fail-on-nonzero-exit policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_nonzero_exit_policy()") != std::string::npos,
               "fll-output wrapper source should route fail-on-nonzero-exit policy through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared fail-on-missing-response policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_fail_on_missing_response_policy()") != std::string::npos,
               "fll-output wrapper source should route fail-on-missing-response policy through the shared helper.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared ensure-parent-directory policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_ensure_parent_directory_policy()") != std::string::npos,
               "fll-output wrapper source should route ensure-parent-directory policy through the shared helper.");
        expect(wrapper_source.find(
                   "std::error_code parent_directory_error;\n"
                   "        std::filesystem::create_directories(plan.target_path.parent_path(), parent_directory_error);\n"
                   "        if (parent_directory_error) {") != std::string::npos,
               "fll-output wrapper source should keep request-directory status failures in the bridge failure contract.");
        expect(wrapper_source.find(
                   "std::error_code response_exists_error;\n"
                   "    if (plan.require_existing_response &&\n"
                   "        (!std::filesystem::exists(plan.source_path, response_exists_error) || response_exists_error)) {") != std::string::npos,
               "fll-output wrapper source should keep response-path status failures in the bridge fallback contract.");
        expect(wrapper_source.find("static bool copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
               "fll-output wrapper source should declare a shared require-existing-response policy helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_require_existing_response_policy()") != std::string::npos,
               "fll-output wrapper source should route require-existing-response policy through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
               "fll-output wrapper source should declare a shared replace-placeholder-return adoption-mode helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_replace_placeholder_return_mode()") != std::string::npos,
               "fll-output wrapper source should route replace-placeholder-return mode token through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
               "fll-output wrapper source should declare a shared planned-activation-pending activation-mode helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_planned_activation_pending_mode()") != std::string::npos,
               "fll-output wrapper source should route planned-activation-pending mode token through the shared helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnEmissionPlan") != std::string::npos,
               "fll-output wrapper source should declare a return-emission-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionPlan copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
               "fll-output wrapper source should declare a return-emission-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmissionAdmission copperfin_runtime_bridge_admit_return_emission(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-emission admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnEmission copperfin_runtime_bridge_execute_return_emission(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-emission execution helper.");
        expect(wrapper_source.find("const auto& return_materialization = plan.return_materialization") != std::string::npos,
               "fll-output wrapper source should consume explicit materialized return while emitting returns.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeFinalReturnAdoptionPlan") != std::string::npos,
               "fll-output wrapper source should declare a final-return-adoption-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionPlan copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
               "fll-output wrapper source should declare a final-return-adoption-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoptionAdmission copperfin_runtime_bridge_admit_final_return_adoption(") != std::string::npos,
               "fll-output wrapper source should declare a shared final-return-adoption admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeFinalReturnAdoption copperfin_runtime_bridge_execute_final_return_adoption(") != std::string::npos,
               "fll-output wrapper source should declare a shared final-return-adoption execution helper.");
        expect(wrapper_source.find("const auto& return_emission = plan.return_emission") != std::string::npos,
               "fll-output wrapper source should consume explicit return emission while adopting final returns.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_placeholder_return_statement(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder return-statement helper");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeReturnActivationPlan") != std::string::npos,
               "fll-output wrapper source should declare a return-activation-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationPlan copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
               "fll-output wrapper source should declare a return-activation-plan helper");
        expect(wrapper_source.find("CopperfinRuntimeBridgeStubEmissionWrapper stub_emission_wrapper;") != std::string::npos,
               "fll-output wrapper source should carry the stub-emission wrapper contract through the descriptor plan.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivationAdmission copperfin_runtime_bridge_admit_return_activation(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-activation admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeReturnActivation copperfin_runtime_bridge_execute_return_activation(") != std::string::npos,
               "fll-output wrapper source should declare a shared return-activation execution helper.");
        expect(wrapper_source.find("const auto& final_return_adoption = plan.final_return_adoption") != std::string::npos,
               "fll-output wrapper source should consume explicit final-return adoption while activating returns.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubReturnPlan") != std::string::npos,
               "fll-output wrapper source should declare a stub-return-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnPlan copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
               "fll-output wrapper source should declare a stub-return-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturnAdmission copperfin_runtime_bridge_admit_stub_return(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-return admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubReturn copperfin_runtime_bridge_execute_stub_return(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-return execution helper.");
        expect(wrapper_source.find("const auto& return_activation = plan.return_activation") != std::string::npos,
               "fll-output wrapper source should consume explicit return activation while routing stub returns.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgePlaceholderReturnValuePlan") != std::string::npos,
               "fll-output wrapper source should declare a placeholder-return-value-plan surface");
        expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValuePlan copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
               "fll-output wrapper source should declare a placeholder-return-value-plan helper");
        expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValueAdmission copperfin_runtime_bridge_admit_placeholder_return_value(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder-return-value admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnValue copperfin_runtime_bridge_execute_placeholder_return_value(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder-return-value execution helper.");
        expect(wrapper_source.find("const auto& stub_return = plan.stub_return") != std::string::npos,
               "fll-output wrapper source should consume explicit stub return while planning placeholder return values.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgePlaceholderReturnIntAdmission copperfin_runtime_bridge_admit_placeholder_return_int(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder-return-int admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionAdmission copperfin_runtime_bridge_admit_stub_emission(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission admission helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmission copperfin_runtime_bridge_execute_stub_emission(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission execution helper.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionReturnSurface copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission return-surface helper.");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_apply_stub_emission_output(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission output-application helper.");
        expect(wrapper_source.find("struct CopperfinRuntimeBridgeStubEmissionWrapper") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission wrapper surface.");
        expect(wrapper_source.find("static CopperfinRuntimeBridgeStubEmissionWrapper copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
               "fll-output wrapper source should declare a shared stub-emission wrapper helper.");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return_shared(") == std::string::npos,
               "fll-output wrapper source should apply stub-emission output at generated call-sites instead of a shared emitter helper.");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_execute_placeholder_return_int(") != std::string::npos,
               "fll-output wrapper source should declare a shared placeholder-return-int execution helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_value)};") == std::string::npos,
               "fll-output wrapper source should not hide placeholder-return-int execution inside the shared stub-emission helper.");
        expect(wrapper_source.find("const auto stub_emission_wrapper =\n        copperfin_runtime_bridge_build_stub_emission_wrapper(") != std::string::npos,
               "fll-output wrapper source should build a shared stub-emission wrapper before building the descriptor plan.");
        expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,") != std::string::npos,
               "fll-output wrapper source should route FLL stub emission through the generated output-application call-site.");
        expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface") != std::string::npos,
               "fll-output wrapper source should read the stub-emission return surface through the descriptor plan.");
        expect(wrapper_source.find("final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter") != std::string::npos,
               "fll-output wrapper source should read the stub-emission return adapter through the descriptor plan.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface(),") != std::string::npos,
               "fll-output wrapper source should pass the FLL int return-surface contract into the shared wrapper helper.");
        expect(wrapper_source.find("_RetInt);") != std::string::npos,
               "fll-output wrapper source should pass the `_RetInt` adapter into the shared wrapper helper.");
        expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
               "fll-output wrapper source should pass the built wrapper into the descriptor-plan builder.");
        expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n        interpretation_plan,\n        placeholder_return_binding.value_representation);") != std::string::npos,
               "fll-output wrapper source should build the failure-policy plan from the enriched interpretation plan.");
        expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n        failure_policy);") != std::string::npos,
               "fll-output wrapper source should build the response-validation plan from the enriched failure-policy plan.");
        expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n        response_validation);") != std::string::npos,
               "fll-output wrapper source should build the request artifact directly from the response-validation plan once the wrapper contract is upstream.");
        expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n        request_artifact);") != std::string::npos,
               "fll-output wrapper source should build the request-write plan directly from the request artifact once the wrapper contract is upstream.");
        const auto request_write_position = wrapper_source.find("const auto request_write_execution =");
        const auto process_launch_position = wrapper_source.find("const auto process_launch = request_write_execution");
        const auto response_read_position = wrapper_source.find("const auto response_read_plan =");
        expect(request_write_position != std::string::npos && process_launch_position != std::string::npos &&
                   response_read_position != std::string::npos && request_write_position < process_launch_position &&
                   process_launch_position < response_read_position,
               "fll-output wrapper source should write the request before launching and reading the response");
        expect(wrapper_source.find("copperfin_runtime_bridge_failed_process_launch(dispatch_execution);") != std::string::npos,
               "fll-output wrapper source should avoid launching when request preparation fails");
        expect(wrapper_source.find("copperfin_runtime_bridge_cleanup_artifacts(response_read_plan);") != std::string::npos,
               "fll-output wrapper source should clean request, response, and log artifacts after each call");
        expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n        request_write_plan,\n        request_write_execution);") != std::string::npos,
               "fll-output wrapper source should build the response-read plan from the request-write plan and executed write result.");
        expect(wrapper_source.find("const auto response_document =\n        copperfin_runtime_bridge_execute_read_response(response_read_plan);") != std::string::npos,
               "fll-output wrapper source should execute the response-read plan before building the response artifact.");
        expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n        response_read_plan,\n        response_document);") != std::string::npos,
               "fll-output wrapper source should build the response artifact from the response-read plan and executed response document.");
        expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n        response_artifact);") != std::string::npos,
               "fll-output wrapper source should build the response-parse plan directly from the response artifact once the wrapper contract is upstream.");
        expect(wrapper_source.find("const auto parsed_response =\n        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);") != std::string::npos,
               "fll-output wrapper source should execute the response-parse plan before building the interpreted-result plan.");
        expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n        response_parse_plan,\n        parsed_response);") != std::string::npos,
               "fll-output wrapper source should build the interpreted-result plan from the response-parse plan and parsed response.");
        expect(wrapper_source.find("const auto interpreted_result =\n        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);") != std::string::npos,
               "fll-output wrapper source should execute the interpreted-result plan before building the native-return plan.");
        expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n        result,\n        interpreted_result_plan,\n        interpreted_result);") != std::string::npos,
               "fll-output wrapper source should build the native-return plan from the interpreted-result plan and interpreted result.");
        expect(wrapper_source.find("const auto native_return =\n        copperfin_runtime_bridge_execute_native_return(native_return_plan);") != std::string::npos,
               "fll-output wrapper source should execute the native-return plan before building the outcome-selection plan.");
        expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n        native_return_plan,\n        native_return);") != std::string::npos,
               "fll-output wrapper source should build the outcome-selection plan from the native-return plan and native return.");
        expect(wrapper_source.find("const auto outcome_selection =\n        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);") != std::string::npos,
               "fll-output wrapper source should execute the outcome-selection plan before building the return-materialization plan.");
        expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n        outcome_selection_plan,\n        outcome_selection);") != std::string::npos,
               "fll-output wrapper source should build the return-materialization plan from the outcome-selection plan and outcome selection.");
        expect(wrapper_source.find("const auto return_materialization =\n        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);") != std::string::npos,
               "fll-output wrapper source should execute the return-materialization plan before building the return-emission plan.");
        expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n        return_materialization_plan,\n        return_materialization);") != std::string::npos,
               "fll-output wrapper source should build the return-emission plan from the return-materialization plan and materialized return.");
        expect(wrapper_source.find("const auto return_emission =\n        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);") != std::string::npos,
               "fll-output wrapper source should execute the return-emission plan before building the final-return-adoption plan.");
        expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n        return_emission_plan,\n        return_emission,\n        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));") != std::string::npos,
               "fll-output wrapper source should build the final-return-adoption plan from the return-emission plan and emitted return.");
        expect(wrapper_source.find("const auto final_return_adoption =\n        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);") != std::string::npos,
               "fll-output wrapper source should execute the final-return-adoption plan before building the return-activation plan.");
        expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n        final_return_adoption_plan,\n        final_return_adoption);") != std::string::npos,
               "fll-output wrapper source should build the return-activation plan from the final-return-adoption plan and adopted return.");
        expect(wrapper_source.find("const auto return_activation =\n        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);") != std::string::npos,
               "fll-output wrapper source should execute the return-activation plan before building the stub-return plan.");
        expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n        return_activation_plan,\n        return_activation);") != std::string::npos,
               "fll-output wrapper source should build the stub-return plan from the return-activation plan and activated return.");
        expect(wrapper_source.find("static int copperfin_runtime_bridge_emit_stub_return(\n") == std::string::npos,
               "fll-output wrapper source should no longer declare an output-specific stub-return wrapper helper once the plan carries the wrapper contract.");
        expect(wrapper_source.find("app.cfmanifest") != std::string::npos,
               "fll-output wrapper source should target the packaged manifest filename");
        expect(wrapper_source.find("copperfin_runtime_host") != std::string::npos,
               "fll-output wrapper source should target the packaged runtime-host filename");
        expect(wrapper_source.find("struct ParamBlk") != std::string::npos,
               "fll-output wrapper source should declare a ParamBlk-shaped callable surface");
        expect(wrapper_source.find("static int _RetInt(int value)") != std::string::npos,
               "fll-output wrapper source should declare the default return helper");
        expect(wrapper_source.find("int InitLibrary(ParamBlk* parm)") != std::string::npos,
               "fll-output wrapper source should scaffold ParamBlk procedure entrypoints");
        expect(wrapper_source.find("int AddNumbers(ParamBlk* parm)") != std::string::npos,
               "fll-output wrapper source should scaffold ParamBlk function entrypoints");
        expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"InitLibrary\"") != std::string::npos,
               "fll-output wrapper source should build a bridge descriptor for InitLibrary");
        expect(wrapper_source.find("\"lparameters\", \"tcMode\", 1U, reinterpret_cast<void*>(&InitLibrary), stub_emission_wrapper);") != std::string::npos,
               "fll-output wrapper source should preserve InitLibrary bridge metadata");
        expect(wrapper_source.find("const auto invocation = copperfin_build_runtime_bridge_invocation(\n        descriptor);") != std::string::npos,
               "fll-output wrapper source should build a bridge invocation from the descriptor");
        expect(wrapper_source.find("const auto call = copperfin_build_runtime_bridge_call(") != std::string::npos,
               "fll-output wrapper source should build a bridge call from the invocation");
        expect(wrapper_source.find("const auto result = copperfin_build_runtime_bridge_result(\n        call,\n        placeholder_return_binding);") != std::string::npos,
               "fll-output wrapper source should build a bridge result from the enriched call");
        expect(wrapper_source.find("const auto placeholder_return_binding =") != std::string::npos,
               "fll-output wrapper source should build a shared placeholder return binding before building the result");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
               "fll-output wrapper source should declare a shared FLL int return-surface helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_fll_int_return_surface()") != std::string::npos,
               "fll-output wrapper source should route FLL return surface through helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_binding(\n            copperfin_build_runtime_bridge_fll_int_return_surface())") != std::string::npos,
               "fll-output wrapper source should build the FLL placeholder return binding through the shared helper");
        expect(wrapper_source.find("const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n        result);") != std::string::npos,
               "fll-output wrapper source should build a launch plan from the result");
        expect(wrapper_source.find("const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n        launch_plan);") != std::string::npos,
               "fll-output wrapper source should build an observation plan from the launch plan");
        expect(wrapper_source.find("const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n        observation_plan);") != std::string::npos,
               "fll-output wrapper source should build an execution plan from the observation plan");
        expect(wrapper_source.find("const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n        execution_plan);") != std::string::npos,
               "fll-output wrapper source should build a transport plan from the execution plan");
        expect(wrapper_source.find("const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n        transport_plan);") != std::string::npos,
               "fll-output wrapper source should build a serialization plan from the transport plan");
        expect(wrapper_source.find("const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n        serialization_plan);") != std::string::npos,
               "fll-output wrapper source should build a dispatch plan from the serialization plan");
        expect(wrapper_source.find("const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);") != std::string::npos,
               "fll-output wrapper source should route the dispatch plan through the shared dispatch-execution helper.");
        expect(wrapper_source.find("(void)dispatch_execution;") == std::string::npos,
               "fll-output wrapper source should consume dispatch execution when launching the process.");
        expect(wrapper_source.find("copperfin_runtime_bridge_launch_process(dispatch_execution)") != std::string::npos,
               "fll-output wrapper source should route dispatch execution through the shared process-launch helper.");
        expect(wrapper_source.find("(void)process_launch;") == std::string::npos,
               "fll-output wrapper source should consume process launch when evaluating host failure.");
        expect(wrapper_source.find("const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n        dispatch_plan);") != std::string::npos,
               "fll-output wrapper source should build a payload plan from the dispatch plan");
        expect(wrapper_source.find("const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(") != std::string::npos,
               "fll-output wrapper source should build an interpretation plan from the payload plan");
        expect(wrapper_source.find("const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(") != std::string::npos,
               "fll-output wrapper source should build a failure policy from the interpretation plan");
        expect(wrapper_source.find("const auto host_failure =") != std::string::npos,
               "fll-output wrapper source should evaluate staged host failure from the process-launch helper.");
        expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);") != std::string::npos,
               "fll-output wrapper source should route process-launch output through the shared host-failure evaluation helper.");
        expect(wrapper_source.find("(void)host_failure;") == std::string::npos,
               "fll-output wrapper source should consume host-failure evaluation when evaluating missing response.");
        expect(wrapper_source.find("const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(") != std::string::npos,
               "fll-output wrapper source should build a response-validation plan from the failure policy");
        expect(wrapper_source.find("const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(") != std::string::npos,
               "fll-output wrapper source should build a request artifact from the response validation plan");
        expect(wrapper_source.find("const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(") != std::string::npos,
               "fll-output wrapper source should build a request write plan from the request artifact");
        expect(wrapper_source.find("const auto request_write_execution =\n        copperfin_runtime_bridge_execute_write_request(request_write_plan);") != std::string::npos,
               "fll-output wrapper source should execute the request-write plan through the shared helper.");
        expect(wrapper_source.find("(void)request_write_execution;") == std::string::npos,
               "fll-output wrapper source should consume request-write execution when planning response reads.");
        expect(wrapper_source.find("const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(") != std::string::npos,
               "fll-output wrapper source should build a response read plan from the request write plan");
        expect(wrapper_source.find("const auto missing_response =") != std::string::npos,
               "fll-output wrapper source should evaluate staged missing-response policy from the host-failure and response-read helpers.");
        expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_missing_response(\n            host_failure,\n            response_read_plan,\n            response_document);") != std::string::npos,
               "fll-output wrapper source should route host-failure output and the response document through the shared missing-response evaluation helper.");
        expect(wrapper_source.find("(void)missing_response;") == std::string::npos,
               "fll-output wrapper source should consume missing-response evaluation when evaluating response validation.");
        expect(wrapper_source.find("const auto response_validation_evaluation =") != std::string::npos,
               "fll-output wrapper source should evaluate staged response-validation policy from the missing-response, validation, and response-document helpers.");
        expect(wrapper_source.find("copperfin_runtime_bridge_evaluate_response_validation(\n            missing_response,\n            response_validation,\n            response_document);") != std::string::npos,
               "fll-output wrapper source should route missing-response output and the response document through the shared response-validation evaluation helper.");
        expect(wrapper_source.find("(void)response_validation_evaluation;") == std::string::npos,
               "fll-output wrapper source should consume response-validation evaluation when admitting response parsing.");
        expect(wrapper_source.find("const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(") != std::string::npos,
               "fll-output wrapper source should build a response artifact from the response read plan");
        expect(wrapper_source.find("const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(") != std::string::npos,
               "fll-output wrapper source should build a response parse plan from the response artifact");
        expect(wrapper_source.find("const auto response_parse_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged response parsing from the response-validation evaluation and parse plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);") != std::string::npos,
               "fll-output wrapper source should route response-validation evaluation through the shared response-parse admission helper.");
        expect(wrapper_source.find("(void)response_parse_admission;") == std::string::npos,
               "fll-output wrapper source should consume response-parse admission when admitting interpreted result.");
        expect(wrapper_source.find("const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(") != std::string::npos,
               "fll-output wrapper source should build an interpreted result plan from the response parse plan");
        expect(wrapper_source.find("const auto interpreted_result_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged interpreted-result selection from the response-parse admission and interpreted-result plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);") != std::string::npos,
               "fll-output wrapper source should route response-parse admission through the shared interpreted-result admission helper.");
        expect(wrapper_source.find("(void)interpreted_result_admission;") == std::string::npos,
               "fll-output wrapper source should consume interpreted-result admission when admitting native return.");
        expect(wrapper_source.find("const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(") != std::string::npos,
               "fll-output wrapper source should build a native return plan from the interpreted result plan");
        expect(wrapper_source.find("const auto native_return_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged native-return selection from the interpreted-result admission and native-return plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);") != std::string::npos,
               "fll-output wrapper source should route interpreted-result admission through the shared native-return admission helper.");
        expect(wrapper_source.find("(void)native_return_admission;") == std::string::npos,
               "fll-output wrapper source should consume native-return admission when admitting outcome selection.");
        expect(wrapper_source.find("const auto success_value_representation = interpreted_result.selected_return_value_representation;") != std::string::npos,
               "fll-output wrapper source should route interpreted response return values into native-return planning");
        expect(wrapper_source.find("const int success_int_value = copperfin_parse_runtime_bridge_int_value_representation(\n        success_value_representation);") != std::string::npos,
               "fll-output wrapper source should parse the typed success integer value from the interpreted response representation");
        expect(wrapper_source.find("const int fallback_int_value = copperfin_parse_runtime_bridge_int_value_representation(") != std::string::npos,
               "fll-output wrapper source should parse the typed fallback integer value from the fallback representation");
        expect(wrapper_source.find("const auto int_value_representation = std::to_string(int_value);") != std::string::npos,
               "fll-output wrapper source should build typed return statements from parsed integer values");
        expect(wrapper_source.find("native_return_plan.success_int_value,") != std::string::npos,
               "fll-output wrapper source should materialize success returns from the parsed success integer value");
        expect(wrapper_source.find("native_return_plan.fallback_int_value,") != std::string::npos,
               "fll-output wrapper source should materialize fallback returns from the parsed fallback integer value");
        expect(wrapper_source.find("\"else { \" + return_materialization.fallback_return_statement + \" }\";") != std::string::npos,
               "fll-output wrapper source should record an explicit fallback else-branch statement");
        expect(wrapper_source.find("success_branch_statement + \" \" + fallback_branch_statement;") != std::string::npos,
               "fll-output wrapper source should compose the emitted return block from the explicit branch statements");
        expect(wrapper_source.find("const auto active_return_block = final_return_adoption.adopted_return_block;") != std::string::npos,
               "fll-output wrapper source should seed the inactive active-return block from the adopted return block");
        expect(wrapper_source.find(": return_activation.active_return_block;") != std::string::npos,
               "fll-output wrapper source should route the deferred stub-return block through the activation metadata");
        expect(wrapper_source.find("int placeholder_fallback_int_value = -1;") != std::string::npos,
               "fll-output wrapper source should record placeholder fallback integers in the stub-return plan");
        expect(wrapper_source.find("std::string placeholder_fallback_value_representation;") != std::string::npos,
               "fll-output wrapper source should record placeholder fallback representations in the stub-return plan");
        expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
               "fll-output wrapper source should record placeholder-emission flags in the placeholder-return-value plan");
        expect(wrapper_source.find("std::string emitted_return_statement;") != std::string::npos,
               "fll-output wrapper source should record placeholder emitted-return statements in the placeholder-return-value plan");
        expect(wrapper_source.find("std::string deferred_return_block;") != std::string::npos,
               "fll-output wrapper source should record deferred return blocks in the placeholder-return-value plan");
        expect(wrapper_source.find("stub_return.emits_placeholder_return,") != std::string::npos,
               "fll-output wrapper source should feed placeholder-emission flags from stub-return metadata");
        expect(wrapper_source.find("stub_return.emitted_return_statement,") != std::string::npos,
               "fll-output wrapper source should feed emitted placeholder-return statements from stub-return metadata");
        expect(wrapper_source.find("stub_return.deferred_return_block,") != std::string::npos,
               "fll-output wrapper source should feed deferred return blocks from stub-return metadata");
        expect(wrapper_source.find("stub_return.activation_mode,") != std::string::npos,
               "fll-output wrapper source should feed activation modes from stub-return metadata");
        expect(wrapper_source.find("stub_return.adoption_mode,") != std::string::npos,
               "fll-output wrapper source should feed adoption modes from stub-return metadata");
        expect(wrapper_source.find("stub_return.keeps_placeholder_return_active,") != std::string::npos,
               "fll-output wrapper source should feed placeholder-helper active-policy booleans from stub-return metadata");
        expect(wrapper_source.find("stub_return.adopts_placeholder_replacement,") != std::string::npos,
               "fll-output wrapper source should feed placeholder-helper replacement-policy booleans from stub-return metadata");
        expect(wrapper_source.find("stub_return.placeholder_fallback_int_value,") != std::string::npos,
               "fll-output wrapper source should feed placeholder fallback integers from stub-return metadata");
        expect(wrapper_source.find("stub_return.placeholder_fallback_value_representation};") != std::string::npos,
               "fll-output wrapper source should feed placeholder fallback representations from stub-return metadata");
        expect(wrapper_source.find("const bool keeps_placeholder_return_active =") != std::string::npos,
               "fll-output wrapper source should derive placeholder-helper active-policy booleans upstream");
        expect(wrapper_source.find("const bool adopts_placeholder_replacement =") != std::string::npos,
               "fll-output wrapper source should derive placeholder-helper replacement-policy booleans upstream");
        expect(wrapper_source.find("(void)placeholder_return_value.emitted_return_statement;") == std::string::npos,
               "fll-output wrapper source should not have the int helper consume placeholder-value return-statement contracts");
        expect(wrapper_source.find("(void)placeholder_return_value.deferred_return_block;") == std::string::npos,
               "fll-output wrapper source should not have the int helper consume placeholder-value deferred return-block contracts");
        expect(wrapper_source.find("placeholder_return_int_admission.should_return_int") != std::string::npos,
               "fll-output wrapper source should have the int helper consume the admitted int-return policy boolean");
        expect(wrapper_source.find("placeholder_return_int_admission.selected_int_value") != std::string::npos,
               "fll-output wrapper source should have the int helper consume the admitted selected integer value");
        expect(wrapper_source.find("const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(") != std::string::npos,
               "fll-output wrapper source should build an outcome selection plan from the native return plan");
        expect(wrapper_source.find("const auto outcome_selection_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged outcome selection from the native-return admission and outcome-selection plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);") != std::string::npos,
               "fll-output wrapper source should route native-return admission through the shared outcome-selection admission helper.");
        expect(wrapper_source.find("(void)outcome_selection_admission;") == std::string::npos,
               "fll-output wrapper source should consume outcome-selection admission when admitting return materialization.");
        expect(wrapper_source.find("const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(") != std::string::npos,
               "fll-output wrapper source should build a return materialization plan from the outcome selection plan");
        expect(wrapper_source.find("const auto return_materialization_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged return materialization from the outcome-selection admission and return-materialization plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);") != std::string::npos,
               "fll-output wrapper source should route outcome-selection admission through the shared return-materialization admission helper.");
        expect(wrapper_source.find("(void)return_materialization_admission;") == std::string::npos,
               "fll-output wrapper source should consume return-materialization admission when admitting return emission.");
        expect(wrapper_source.find("const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(") != std::string::npos,
               "fll-output wrapper source should build a return emission plan from the return materialization plan");
        expect(wrapper_source.find("const auto return_emission_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged return emission from the return-materialization admission and return-emission plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);") != std::string::npos,
               "fll-output wrapper source should route return-materialization admission through the shared return-emission admission helper.");
        expect(wrapper_source.find("(void)return_emission_admission;") == std::string::npos,
               "fll-output wrapper source should consume return-emission admission when admitting final-return adoption.");
        expect(wrapper_source.find("const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(") != std::string::npos,
               "fll-output wrapper source should build a final return adoption plan from the return emission plan");
        expect(wrapper_source.find("const auto final_return_adoption_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged final return adoption from the return-emission admission and final-return-adoption plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);") != std::string::npos,
               "fll-output wrapper source should route return-emission admission through the shared final-return-adoption admission helper.");
        expect(wrapper_source.find("(void)final_return_adoption_admission;") == std::string::npos,
               "fll-output wrapper source should consume final-return-adoption admission when admitting return activation.");
        expect(wrapper_source.find("const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(") != std::string::npos,
               "fll-output wrapper source should build a return activation plan from the final return adoption plan");
        expect(wrapper_source.find("const auto return_activation_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged return activation from the final-return-adoption admission and return-activation plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);") != std::string::npos,
               "fll-output wrapper source should route final-return-adoption admission through the shared return-activation admission helper.");
        expect(wrapper_source.find("(void)return_activation_admission;") == std::string::npos,
               "fll-output wrapper source should consume return-activation admission when admitting stub-return routing.");
        expect(wrapper_source.find("const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(") != std::string::npos,
               "fll-output wrapper source should build a stub return plan from the return activation plan");
        expect(wrapper_source.find("const auto stub_return_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged stub-return routing from the return-activation admission and stub-return plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);") != std::string::npos,
               "fll-output wrapper source should route return-activation admission through the shared stub-return admission helper.");
        expect(wrapper_source.find("(void)stub_return_admission;") == std::string::npos,
               "fll-output wrapper source should consume stub-return admission when admitting placeholder-return-value routing.");
        expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(") != std::string::npos,
               "fll-output wrapper source should build a placeholder-return-value plan from the stub return plan");
        expect(wrapper_source.find("const auto stub_return =\n        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);") != std::string::npos,
               "fll-output wrapper source should execute the stub-return plan before building the placeholder-return-value plan.");
        expect(wrapper_source.find("const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n        stub_return_plan,\n        stub_return);") != std::string::npos,
               "fll-output wrapper source should build the placeholder-return-value plan from the stub-return plan and stub return.");
        expect(wrapper_source.find("const auto placeholder_return_value_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged placeholder-return-value routing from the stub-return admission and placeholder-return-value plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);") != std::string::npos,
               "fll-output wrapper source should route stub-return admission through the shared placeholder-return-value admission helper.");
        expect(wrapper_source.find("(void)placeholder_return_value_admission;") == std::string::npos,
               "fll-output wrapper source should consume placeholder-return-value admission when admitting placeholder-return-int routing.");
        expect(wrapper_source.find("const auto placeholder_return_value =\n        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);") != std::string::npos,
               "fll-output wrapper source should execute the placeholder-return-value plan before shared stub emission.");
        expect(wrapper_source.find("const auto placeholder_return_int_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged placeholder-return-int routing from the placeholder-return-value admission and placeholder-return-value plan.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);") != std::string::npos,
               "fll-output wrapper source should route placeholder-return-value admission through the shared placeholder-return-int admission helper.");
        expect(wrapper_source.find("(void)placeholder_return_int_admission;") == std::string::npos,
               "fll-output wrapper source should consume placeholder-return-int admission when executing placeholder-return-int output.");
        expect(wrapper_source.find("const auto placeholder_return_int =\n        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);") != std::string::npos,
               "fll-output wrapper source should execute placeholder-return-int routing from the admitted surface before stub emission.");
        expect(wrapper_source.find("const auto stub_emission_admission =") != std::string::npos,
               "fll-output wrapper source should admit staged stub emission from the placeholder-return-int admission.");
        expect(wrapper_source.find("copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);") != std::string::npos,
               "fll-output wrapper source should route explicit placeholder-return-int output through the shared stub-emission admission helper.");
        expect(wrapper_source.find("(void)stub_emission_admission;") == std::string::npos,
               "fll-output wrapper source should consume stub-emission admission when executing stub emission.");
        expect(wrapper_source.find("const auto stub_emission =\n        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);") != std::string::npos,
               "fll-output wrapper source should execute stub emission from explicit admission output.");
        expect(wrapper_source.find("const auto stub_emission_return_surface =\n        copperfin_runtime_bridge_build_stub_emission_return_surface(") != std::string::npos,
               "fll-output wrapper source should build the stub-emission return surface before direct output application.");
        expect(wrapper_source.find("native_return_plan.fallback_int_value") != std::string::npos,
               "fll-output wrapper source should propagate the typed native fallback integer value downstream");
        expect(wrapper_source.find("return copperfin_runtime_bridge_apply_stub_emission_output(\n        stub_emission_return_surface,\n        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);") != std::string::npos,
               "fll-output wrapper source should route the placeholder return through the generated stub-emission output-application call-site.");
        expect(wrapper_source.find("\"--library-export\"") != std::string::npos,
               "fll-output wrapper source should encode the export name into the bridge invocation plan");
        expect(wrapper_source.find("(void)parm;") == std::string::npos,
               "fll-output wrapper source should consume ParamBlk through bridge call bindings.");
        expect(wrapper_source.find("{{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}}") != std::string::npos,
               "fll-output wrapper source should preserve the ParamBlk call-surface binding");
        expect(wrapper_source.find(", stub_emission_wrapper);") != std::string::npos,
               "fll-output wrapper source should feed the bridge result from the enriched descriptor and shared placeholder return binding");
        expect(wrapper_source.find("{copperfin_runtime_bridge_library_export_env_var(), result.call.invocation.descriptor.export_name}") != std::string::npos,
               "fll-output wrapper source should preserve launch environment export metadata");
        expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_stdout_log_suffix()") != std::string::npos,
               "fll-output wrapper source should derive unique stdout observation paths");
        expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_stderr_log_suffix()") != std::string::npos,
               "fll-output wrapper source should derive unique stderr observation paths");
        expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.descriptor.runtime_host_path") != std::string::npos,
               "fll-output wrapper source should preserve the runtime-host executable path in the execution plan");
        expect(wrapper_source.find("observation_plan.launch_plan.result.call.invocation.arguments") != std::string::npos,
               "fll-output wrapper source should preserve the bridge invocation arguments in the execution plan");
        expect(wrapper_source.find("const std::string artifact_stem = std::string(export_name) + \".\" + invocation_identity;") != std::string::npos,
               "fll-output wrapper source should derive a per-call transport artifact stem");
        expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_request_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should derive unique request transport paths");
        expect(wrapper_source.find("artifact_stem + copperfin_runtime_bridge_response_artifact_suffix()") != std::string::npos,
               "fll-output wrapper source should derive unique response transport paths");
        expect(wrapper_source.find("static std::atomic<unsigned long long> invocation_sequence{0};") != std::string::npos,
               "fll-output wrapper source should serialize per-process invocation identities");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared request serialization media-type helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared response serialization media-type helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
               "fll-output wrapper source should declare a shared serialization schema-version helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared request-path dispatch helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-path dispatch helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared request-media-type dispatch helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared response-media-type dispatch helper");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
               "fll-output wrapper source should declare a shared schema-version dispatch helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_value()") != std::string::npos,
               "fll-output wrapper source should route the request serialization media type through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_value()") != std::string::npos,
               "fll-output wrapper source should route the response serialization media type through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_value()") != std::string::npos,
               "fll-output wrapper source should route the serialization schema version through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_path_argument_name()") != std::string::npos,
               "fll-output wrapper source should route the request-path dispatch argument through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_path_argument_name()") != std::string::npos,
               "fll-output wrapper source should route the response-path dispatch argument through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_argument_name()") != std::string::npos,
               "fll-output wrapper source should route the request-media-type dispatch argument through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_argument_name()") != std::string::npos,
               "fll-output wrapper source should route the response-media-type dispatch argument through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_argument_name()") != std::string::npos,
               "fll-output wrapper source should route the schema-version dispatch argument through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_payload_shape_name()") != std::string::npos,
               "fll-output wrapper source should route the request payload shape through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_payload_shape_name()") != std::string::npos,
               "fll-output wrapper source should route the response payload shape through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_export_name_field_name()") != std::string::npos,
               "fll-output wrapper source should route the export-name field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_routine_kind_field_name()") != std::string::npos,
               "fll-output wrapper source should route the routine-kind field through the shared helper");
        expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.routine_kind)") != std::string::npos,
               "fll-output wrapper source should serialize routine-kind metadata into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_source_path_field_name()") != std::string::npos,
               "fll-output wrapper source should route the source-path field through the shared helper");
        expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.source_path)") != std::string::npos,
               "fll-output wrapper source should serialize source-path metadata into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_source_line_field_name()") != std::string::npos,
               "fll-output wrapper source should route the source-line field through the shared helper");
        expect(wrapper_source.find("call.invocation.descriptor.source_line") != std::string::npos,
               "fll-output wrapper source should serialize source-line metadata into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_declaration_field_name()") != std::string::npos,
               "fll-output wrapper source should route the parameter-declaration field through the shared helper");
        expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_declaration_kind)") != std::string::npos,
               "fll-output wrapper source should serialize parameter-declaration metadata into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_names_field_name()") != std::string::npos,
               "fll-output wrapper source should route the parameter-names field through the shared helper");
        expect(wrapper_source.find("copperfin_escape_runtime_bridge_json_string(call.invocation.descriptor.parameter_names)") != std::string::npos,
               "fll-output wrapper source should serialize parameter-name metadata into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameter_count_field_name()") != std::string::npos,
               "fll-output wrapper source should route the parameter-count field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_schema_version_field_name()") != std::string::npos,
               "fll-output wrapper source should route the schema-version field through the shared helper");
        expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.schema_version") != std::string::npos,
               "fll-output wrapper source should serialize schema-version metadata into the request document");
        expect(wrapper_source.find("{copperfin_build_runtime_bridge_export_name_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_routine_kind_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_source_path_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_source_line_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_parameter_declaration_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_parameter_names_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_parameter_count_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_parameters_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_request_media_type_field_name()}") != std::string::npos,
               "fll-output wrapper source should declare descriptor metadata in the request-field contract");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_parameters_field_name()") != std::string::npos,
               "fll-output wrapper source should route the parameters field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should route the request-media-type field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_fields_field_name()") != std::string::npos,
               "fll-output wrapper source should route the request-fields contract through the shared helper");
        expect(wrapper_source.find("payload_plan.request_fields.size()") != std::string::npos,
               "fll-output wrapper source should serialize the request-field contract list");
        expect(wrapper_source.find("payload_plan.request_fields[index]") != std::string::npos,
               "fll-output wrapper source should serialize each request-field contract item");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_expected_response_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should route expected response media-type through the shared helper");
        expect(wrapper_source.find("payload_plan.dispatch_plan.serialization_plan.response_media_type") != std::string::npos,
               "fll-output wrapper source should serialize expected response media type into the request document");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_fields_field_name()") != std::string::npos,
               "fll-output wrapper source should route the response-fields contract through the shared helper");
        expect(wrapper_source.find("payload_plan.response_fields.size()") != std::string::npos,
               "fll-output wrapper source should serialize the response-field contract list");
        expect(wrapper_source.find("payload_plan.response_fields[index]") != std::string::npos,
               "fll-output wrapper source should serialize each response-field contract item");
        expect(wrapper_source.find("{copperfin_build_runtime_bridge_status_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_return_value_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_response_media_type_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_schema_version_field_name(),\n"
                                   "         copperfin_build_runtime_bridge_diagnostics_field_name()}") != std::string::npos,
               "fll-output wrapper source should declare schema version in the response-field contract");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_return_value_field_name()") != std::string::npos,
               "fll-output wrapper source should route the response value field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_media_type_field_name()") != std::string::npos,
               "fll-output wrapper source should route the response-media-type field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_status_field_name()") != std::string::npos,
               "fll-output wrapper source should route the response status field through the shared helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_diagnostics_field_name()") != std::string::npos,
               "fll-output wrapper source should route the response diagnostics field through the shared helper");
        expect(wrapper_source.find("        copperfin_build_runtime_bridge_fll_int_return_surface());") != std::string::npos,
               "fll-output wrapper source should preserve the FLL wrapper return surface");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_failure_diagnostics_value()") != std::string::npos,
               "fll-output wrapper source should declare the diagnostics fallback policy through the shared token helper");
        expect(wrapper_source.find("placeholder_return_binding.value_representation);") != std::string::npos,
               "fll-output wrapper source should declare the fallback return value policy through the shared binding");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
               "fll-output wrapper source should derive the placeholder return statement from the shared binding helper");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_success_status_value()") != std::string::npos,
               "fll-output wrapper source should declare the success-status expectation through the shared token helper");
        expect(wrapper_source.find("std::string request_document;") != std::string::npos,
               "fll-output wrapper source should record the request document payload.");
        expect(wrapper_source.find("std::filesystem::path target_path;") != std::string::npos,
               "fll-output wrapper source should record the request write target path.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
               "fll-output wrapper source should declare a shared request write-mode helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_request_write_mode()") != std::string::npos,
               "fll-output wrapper source should route the request write mode through the shared helper.");
        expect(wrapper_source.find("std::filesystem::path source_path;") != std::string::npos,
               "fll-output wrapper source should record the response read source path.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
               "fll-output wrapper source should declare a shared response read-mode helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_read_mode()") != std::string::npos,
               "fll-output wrapper source should route the response read mode through the shared helper.");
        expect(wrapper_source.find("std::string response_document;") != std::string::npos,
               "fll-output wrapper source should record the response document payload.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
               "fll-output wrapper source should declare a shared empty response-document helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_empty_response_document()") != std::string::npos,
               "fll-output wrapper source should route the empty response-document token through the shared helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
               "fll-output wrapper source should declare a shared response parse-kind helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_response_parse_kind()") != std::string::npos,
               "fll-output wrapper source should route the response parse kind through the shared helper.");
        expect(wrapper_source.find("std::string wrapper_return_surface;") != std::string::npos,
               "fll-output wrapper source should record the wrapper return surface.");
        expect(wrapper_source.find("std::string native_return_surface;") != std::string::npos,
               "fll-output wrapper source should record the native return surface.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
               "fll-output wrapper source should declare a shared success-comparator helper.");
        expect(wrapper_source.find("static std::string copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
               "fll-output wrapper source should declare a shared fallback-comparator helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_success_comparator_token()") != std::string::npos,
               "fll-output wrapper source should route the success comparator through the shared helper.");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_fallback_comparator_token()") != std::string::npos,
               "fll-output wrapper source should route the fallback comparator through the shared helper.");
        expect(wrapper_source.find("std::string success_condition;") != std::string::npos,
               "fll-output wrapper source should record the outcome success condition.");
        expect(wrapper_source.find("std::string success_return_statement;") != std::string::npos,
               "fll-output wrapper source should record the success return statement.");
        expect(wrapper_source.find("std::string emitted_return_block;") != std::string::npos,
               "fll-output wrapper source should record the emitted return block.");
        expect(wrapper_source.find("std::string placeholder_return_statement;") != std::string::npos,
               "fll-output wrapper source should record the placeholder return statement.");
        expect(wrapper_source.find("bool activates_adopted_return = false;") != std::string::npos,
               "fll-output wrapper source should record the inactive return-activation flag.");
        expect(wrapper_source.find("bool emits_placeholder_return = true;") != std::string::npos,
               "fll-output wrapper source should record the placeholder-emission flag.");
        expect(wrapper_source.find("int fallback_int_value = -1;") != std::string::npos,
               "fll-output wrapper source should record the placeholder fallback integer value.");
        expect(wrapper_source.find("int success_int_value = -1;") != std::string::npos,
               "fll-output wrapper source should record the typed native success integer value.");
        expect(wrapper_source.find("const auto descriptor = copperfin_build_runtime_bridge_descriptor(\"AddNumbers\"") != std::string::npos,
               "fll-output wrapper source should build a bridge descriptor for AddNumbers");
        expect(wrapper_source.find("\"parameters\", \"tnLeft\\|tnRight\", 2U, reinterpret_cast<void*>(&AddNumbers), stub_emission_wrapper);") != std::string::npos,
               "fll-output wrapper source should preserve AddNumbers bridge metadata");
        expect(wrapper_source.find("copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding)") != std::string::npos,
               "fll-output wrapper source should route stub returns through the shared placeholder return-statement helper");
        expect(wrapper_source.find("struct CopperfinFoxInfoRecord") != std::string::npos,
               "fll-output wrapper source should emit FoxInfo registration metadata");
        expect(wrapper_source.find("struct CopperfinFoxTableRecord") != std::string::npos,
               "fll-output wrapper source should emit FoxTable registration metadata");
        expect(wrapper_source.find("const char* routine_kind;") != std::string::npos,
               "fll-output wrapper source should record routine kind fields in the FoxInfo table");
        expect(wrapper_source.find("const char* source_path;") != std::string::npos,
               "fll-output wrapper source should record source-path fields in the FoxInfo table");
        expect(wrapper_source.find("unsigned int source_line;") != std::string::npos,
               "fll-output wrapper source should record source-line fields in the FoxInfo table");
        expect(wrapper_source.find("const char* parameter_declaration_kind;") != std::string::npos,
               "fll-output wrapper source should record parameter-declaration fields in the FoxInfo table");
        expect(wrapper_source.find("const char* parameter_names;") != std::string::npos,
               "fll-output wrapper source should record parameter-name fields in the FoxInfo table");
        expect(wrapper_source.find("const CopperfinFoxTableRecord _FoxTable") != std::string::npos,
               "fll-output wrapper source should export the FoxTable registration symbol");
        expect(wrapper_source.find("{\"InitLibrary\", &InitLibrary, \"procedure\", \"" + quote_manifest_value(librarymain_source) + "\", 1U, \"lparameters\", \"tcMode\", 1U}") != std::string::npos,
               "fll-output wrapper source should record InitLibrary metadata in the FoxInfo table");
        expect(wrapper_source.find("{\"AddNumbers\", &AddNumbers, \"function\", \"" + quote_manifest_value(helper_source) + "\", 1U, \"parameters\", \"tnLeft|tnRight\", 2U}") != std::string::npos,
               "fll-output wrapper source should record AddNumbers metadata in the FoxInfo table");
        expect(wrapper_source.find("const CopperfinFoxTableRecord* FoxInfo()") != std::string::npos,
               "fll-output wrapper source should scaffold the FoxInfo entrypoint");
        const std::string wrapper_cmake = read_text(result.plan.native_wrapper_cmake_path);
        expect(wrapper_cmake.find("add_library(LibraryDemo SHARED LibraryDemo_wrapper.cpp)") != std::string::npos,
               "fll-output wrapper CMake should declare a shared library target");
        expect(wrapper_cmake.find("target_link_libraries(LibraryDemo PRIVATE dl)") != std::string::npos,
               "fll-output wrapper CMake should link dl on supported Unix hosts for module-path discovery");
        expect(wrapper_cmake.find("PREFIX \"\" SUFFIX \".fll\"") != std::string::npos,
               "fll-output wrapper CMake should preserve the requested FLL filename shape");
        expect(wrapper_cmake.find("LIBRARY_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route built libraries to the package root");
        expect(wrapper_cmake.find("RUNTIME_OUTPUT_DIRECTORY \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route built runtime artifacts to the package root");
        expect(wrapper_cmake.find("foreach(COPPERFIN_CONFIGURATION IN LISTS CMAKE_CONFIGURATION_TYPES)") != std::string::npos,
               "fll-output wrapper CMake should enumerate multi-config generator configurations");
        expect(wrapper_cmake.find("\"LIBRARY_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route every configured library artifact to the package root");
        expect(wrapper_cmake.find("\"RUNTIME_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route every configured runtime artifact to the package root");
        expect(wrapper_cmake.find("\"ARCHIVE_OUTPUT_DIRECTORY_${COPPERFIN_CONFIGURATION_UPPER}\" \"${CMAKE_CURRENT_SOURCE_DIR}/..\"") != std::string::npos,
               "fll-output wrapper CMake should route every configured archive artifact to the package root");
        expect(wrapper_cmake.find("/DEF:${CMAKE_CURRENT_SOURCE_DIR}/../LibraryDemo.def") != std::string::npos,
               "fll-output wrapper CMake should forward the module-definition file on MSVC");
        const std::string wrapper_shell_script = read_text(result.plan.native_wrapper_build_script_path);
        expect(wrapper_shell_script.find("cmake -S \"$SCRIPT_DIR\" -B \"$SCRIPT_DIR/build\"") != std::string::npos,
               "fll-output wrapper shell script should configure the emitted CMake project");
        expect(wrapper_shell_script.find("cmake --build \"$SCRIPT_DIR/build\"") != std::string::npos,
               "fll-output wrapper shell script should build the emitted CMake project");
        const std::string wrapper_powershell_script = read_text(result.plan.native_wrapper_build_powershell_path);
        expect(wrapper_powershell_script.find("cmake -S $scriptDir -B $buildDir") != std::string::npos,
               "fll-output wrapper PowerShell script should configure the emitted CMake project");
        expect(wrapper_powershell_script.find("cmake --build $buildDir") != std::string::npos,
               "fll-output wrapper PowerShell script should build the emitted CMake project");
        if (native_cxx_is_available()) {
            fs::path compiled_wrapper_path;
            std::string compile_error;
            const bool compiled = compile_native_wrapper_scaffold(
                result.plan.native_wrapper_source_path,
                compiled_wrapper_path,
                compile_error);
            if (!compiled && !compile_error.empty()) {
                std::cerr << "FAIL: " << compile_error << "\n";
            }
            expect(compiled,
                   "fll-output wrapper scaffold should compile under the host C++ toolchain");
            if (compiled) {
                test_generated_bridge_runtime_host_verification(compiled_wrapper_path);
            }
            if (compiled && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(compiled_wrapper_path, symbol_error);
                const std::set<std::string> declared_module_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(result.plan.fll_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols.contains("InitLibrary"),
                       "fll-output compiled wrapper should export InitLibrary");
                expect(exported_symbols.contains("AddNumbers"),
                       "fll-output compiled wrapper should export AddNumbers");
                expect(exported_symbols.contains("FoxInfo"),
                       "fll-output compiled wrapper should export FoxInfo");
                expect(exported_symbols.contains("_FoxTable"),
                       "fll-output compiled wrapper should export the FoxTable registration symbol");
                expect(exported_symbols == declared_module_symbols,
                       "fll-output compiled wrapper exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "fll-output compiled wrapper exports should stay synchronized with the API manifest contract");
            }
        }
        if (cmake_is_available() && shell_is_available()) {
            std::string script_error;
            const bool script_built = build_native_wrapper_with_script(
                result.plan.native_wrapper_build_script_path,
                result.plan.launcher_output_path,
                script_error);
            if (!script_built && !script_error.empty()) {
                std::cerr << "FAIL: " << script_error << "\n";
            }
            expect(script_built,
                   "fll-output wrapper shell script should build the requested primary output");
        }
        if (cmake_is_available()) {
            fs::path cmake_output_path;
            std::string cmake_error;
            const bool cmake_built = build_native_wrapper_with_cmake(
                result.plan.native_wrapper_cmake_path,
                result.plan.launcher_output_path,
                cmake_output_path,
                cmake_error);
            if (!cmake_built && !cmake_error.empty()) {
                std::cerr << "FAIL: " << cmake_error << "\n";
            }
            expect(cmake_built,
                   "fll-output wrapper CMake metadata should configure and build under CMake");
            if (cmake_built) {
                expect(cmake_output_path == result.plan.launcher_output_path,
                       "fll-output generated-CMake artifact should materialize the requested primary output path");
            }
            if (cmake_built && native_symbol_dump_is_available()) {
                std::string symbol_error;
                const std::set<std::string> exported_symbols = read_native_exported_symbols(cmake_output_path, symbol_error);
                const std::set<std::string> declared_module_symbols = read_module_definition_exports(result.plan.module_definition_path);
                const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(result.plan.fll_api_manifest_path);
                if (exported_symbols.empty() && !symbol_error.empty()) {
                    std::cerr << "FAIL: " << symbol_error << "\n";
                }
                expect(exported_symbols == declared_module_symbols,
                       "fll-output generated-CMake artifact exports should stay synchronized with the module-definition contract");
                expect(exported_symbols == declared_api_symbols,
                       "fll-output generated-CMake artifact exports should stay synchronized with the API manifest contract");
            }
        }
        if (ninja_multi_config_is_available()) {
            std::string multi_config_error;
            const bool multi_config_built = build_native_wrapper_with_ninja_multi_config(
                result.plan.native_wrapper_cmake_path,
                result.plan.launcher_output_path,
                multi_config_error);
            if (!multi_config_built && !multi_config_error.empty()) {
                std::cerr << "FAIL: " << multi_config_error << "\n";
            }
            expect(multi_config_built,
                   "fll-output wrapper should materialize the requested FLL for Debug and Release multi-config builds");
        }

        const std::string api_manifest = read_text(result.plan.fll_api_manifest_path);
        expect(api_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output API manifest should declare the FLL output kind");
        expect(api_manifest.find("library_file=LibraryDemo.fll") != std::string::npos,
               "fll-output API manifest should name the requested FLL file");
        expect(api_manifest.find("registration_model=FoxInfo/FoxTable") != std::string::npos,
               "fll-output API manifest should declare the FoxInfo/FoxTable registration model");
        expect(api_manifest.find("registration_command=SET LIBRARY TO") != std::string::npos,
               "fll-output API manifest should declare the registration command");
        expect(api_manifest.find("release_command=RELEASE LIBRARY") != std::string::npos,
               "fll-output API manifest should declare the release command");
        expect(api_manifest.find("additive_supported=true") != std::string::npos,
               "fll-output API manifest should declare additive loading support");
        expect(api_manifest.find("loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output API manifest should declare the loader entrypoint");
        expect(api_manifest.find("registration_symbol=_FoxTable") != std::string::npos,
               "fll-output API manifest should declare the FoxTable registration symbol");
        expect(api_manifest.find("callable_signature=ParamBlk*") != std::string::npos,
               "fll-output API manifest should declare the ParamBlk callable signature");
        expect(api_manifest.find("default_return_helper=_RetInt") != std::string::npos,
               "fll-output API manifest should declare the default return helper");
        expect(api_manifest.find("function=InitLibrary") != std::string::npos,
               "fll-output API manifest should list discovered procedure names");
        expect(api_manifest.find("function=AddNumbers") != std::string::npos,
               "fll-output API manifest should list discovered function names");
        expect(api_manifest.find("function_arity=InitLibrary|1") != std::string::npos,
               "fll-output API manifest should declare InitLibrary arity");
        expect(api_manifest.find("function_arity=AddNumbers|2") != std::string::npos,
               "fll-output API manifest should declare AddNumbers arity");
        expect(api_manifest.find("function_kind=InitLibrary|procedure") != std::string::npos,
               "fll-output API manifest should record InitLibrary routine kind");
        expect(api_manifest.find("function_kind=AddNumbers|function") != std::string::npos,
               "fll-output API manifest should record AddNumbers routine kind");
        expect(api_manifest.find("function_source=InitLibrary|" + quote_manifest_value(librarymain_source) + "|1") != std::string::npos,
               "fll-output API manifest should record InitLibrary source provenance");
        expect(api_manifest.find("function_source=AddNumbers|" + quote_manifest_value(helper_source) + "|1") != std::string::npos,
               "fll-output API manifest should record AddNumbers source provenance");
        expect(api_manifest.find("function_parameters=InitLibrary|tcMode") != std::string::npos,
               "fll-output API manifest should record InitLibrary parameter names");
        expect(api_manifest.find("function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "fll-output API manifest should record AddNumbers parameter names");
        expect(api_manifest.find("function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "fll-output API manifest should record InitLibrary parameter declaration style");
        expect(api_manifest.find("function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "fll-output API manifest should record AddNumbers parameter declaration style");
        expect(api_manifest.find("function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output API manifest should declare InitLibrary callable surface");
        expect(api_manifest.find("function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output API manifest should declare AddNumbers callable surface");

        const std::string runtime_manifest = read_text(result.plan.manifest_path);
        const std::string debug_manifest = read_text(result.plan.debug_manifest_path);
        expect(runtime_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output manifest should record FLL output kind");
        expect(runtime_manifest.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output manifest should record the FLL loader entrypoint");
        expect(runtime_manifest.find("fll_registration_symbol=_FoxTable") != std::string::npos,
               "fll-output manifest should record the FoxTable registration symbol");
        expect(runtime_manifest.find("fll_callable_signature=ParamBlk*") != std::string::npos,
               "fll-output manifest should record the ParamBlk callable signature");
        expect(runtime_manifest.find("fll_default_return_helper=_RetInt") != std::string::npos,
               "fll-output manifest should record the default return helper");
        expect(lines_with_prefix(runtime_manifest, "library_function_").empty(),
               "fll-output runtime manifest should omit library-function inventory from the execution contract");
        expect(lines_with_prefix(runtime_manifest, "feature_flag=").empty(),
               "fll-output runtime manifest should omit feature-flag inventory from the execution contract");
        expect(runtime_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "fll-output manifest should record the project title");
        expect(runtime_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "fll-output manifest should record the package root");
        expect(runtime_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "fll-output manifest should record the content root");
        expect(runtime_manifest.find("configuration=debug") != std::string::npos,
               "fll-output manifest should record the debug build configuration");
        expect(runtime_manifest.find("security_enabled=false") != std::string::npos,
               "fll-output manifest should record the disabled security state");
        expect(runtime_manifest.find("security_role=" + quote_manifest_value(result.plan.security_role)) != std::string::npos,
               "fll-output manifest should record the effective security role");
        expect(runtime_manifest.find("security_mode=" + quote_manifest_value(copperfin::security::default_native_security_profile().mode)) != std::string::npos,
               "fll-output manifest should record the security mode");
        expect(runtime_manifest.find("audit_log_path=" + quote_manifest_value(result.plan.audit_log_path)) != std::string::npos,
               "fll-output manifest should record the audit log path");
        expect(runtime_manifest.find("runtime_host_sha256=" + quote_manifest_value(result.plan.runtime_host_sha256)) != std::string::npos,
               "fll-output manifest should record the runtime host SHA-256 digest");
        expect(runtime_manifest.find("extension_payload=" + quote_manifest_value(result.plan.runtime_host_destination_path) + "|") != std::string::npos,
               "fll-output manifest should record the staged runtime host as an extension payload");
        expect(runtime_manifest.find("security_roles=") == std::string::npos,
               "fll-output runtime manifest should omit the security-role count summary");
        expect_manifest_omits_keys(
            runtime_manifest,
            {
                "project_path",
                "ast_manifest_path",
                "ir_manifest_path",
                "transpiled_csharp_path",
                "primary_output_path",
                "primary_output_materialized",
                "module_definition_path",
                "library_api_manifest_path",
                "fll_api_manifest_path",
                "fxp_token_manifest_path",
                "app_archive_manifest_path",
                "native_wrapper_source_path",
                "native_wrapper_cmake_path",
                "native_wrapper_build_script_path",
                "native_wrapper_build_powershell_path"
            },
            "fll-output runtime manifest");
        const std::vector<std::string> runtime_asset_lines = lines_with_prefix(runtime_manifest, "asset=");
        expect(!runtime_asset_lines.empty(),
               "fll-output manifest should record staged asset inventory");
        expect(debug_manifest.find("output_kind=fll") != std::string::npos,
               "fll-output debug manifest should record FLL output kind");
        expect(debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
               "fll-output debug manifest should record the project title");
        expect(debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
               "fll-output debug manifest should record the project path");
        expect(debug_manifest.find("package_root=" + quote_manifest_value(result.plan.package_root)) != std::string::npos,
               "fll-output debug manifest should record the package root");
        expect(debug_manifest.find("content_root=" + quote_manifest_value(result.plan.content_root)) != std::string::npos,
               "fll-output debug manifest should record the content root");
        expect(debug_manifest.find("ast_manifest_path=" + quote_manifest_value(result.plan.ast_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the AST manifest path");
        expect(debug_manifest.find("ir_manifest_path=" + quote_manifest_value(result.plan.ir_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the IR manifest path");
        expect(debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(result.plan.transpiled_csharp_path)) != std::string::npos,
               "fll-output debug manifest should record the transpiled C# path");
        expect(debug_manifest.find("configuration=debug") != std::string::npos,
               "fll-output debug manifest should record the debug build configuration");
        expect(debug_manifest.find("security_enabled=false") != std::string::npos,
               "fll-output debug manifest should record the disabled security state");
        expect(debug_manifest.find("security_role=" + quote_manifest_value(result.plan.security_role)) != std::string::npos,
               "fll-output debug manifest should record the effective security role");
        expect(debug_manifest.find("security_mode=" + quote_manifest_value(copperfin::security::default_native_security_profile().mode)) != std::string::npos,
               "fll-output debug manifest should record the security mode");
        expect(debug_manifest.find("audit_log_path=" + quote_manifest_value(result.plan.audit_log_path)) != std::string::npos,
               "fll-output debug manifest should record the audit log path");
        expect(debug_manifest.find("runtime_host_sha256=" + quote_manifest_value(result.plan.runtime_host_sha256)) != std::string::npos,
               "fll-output debug manifest should record the runtime host SHA-256 digest");
        expect(debug_manifest.find("extension_payload=" + quote_manifest_value(result.plan.runtime_host_destination_path) + "|") != std::string::npos,
               "fll-output debug manifest should record the staged runtime host as an extension payload");
        expect(debug_manifest.find("security_roles=" + std::to_string(copperfin::security::default_native_security_profile().roles.size())) != std::string::npos,
               "fll-output debug manifest should record the security-role count");
        const std::vector<std::string> fll_dotnet_summary_keys{
            "dotnet_story"};
        for (const auto& key : fll_dotnet_summary_keys) {
            const std::string value = manifest_value_for_key(runtime_manifest, key);
            expect(!value.empty(),
                   "fll-output runtime manifest should provide " + key + " for debug-manifest mirroring");
            expect(debug_manifest.find(key + "=" + value) != std::string::npos,
                   "fll-output debug manifest should mirror " + key);
        }
        expect(manifest_value_for_key(runtime_manifest, "dotnet_enabled").empty(),
               "fll-output runtime manifest should omit the .NET availability summary");
        expect(manifest_value_for_key(runtime_manifest, "dotnet_policy_allowlist").empty(),
               "fll-output runtime manifest should omit the .NET allowlist summary");
        expect(manifest_value_for_key(runtime_manifest, "dotnet_policy_denylist").empty(),
               "fll-output runtime manifest should omit the .NET denylist summary");
        expect(manifest_value_for_key(runtime_manifest, "dotnet_parity_matrix_entries").empty(),
               "fll-output runtime manifest should omit the .NET parity summary");
        expect(manifest_value_for_key(runtime_manifest, "dotnet_gateway_task_primitives").empty(),
               "fll-output runtime manifest should omit the .NET gateway allow decision");
        expect(manifest_value_for_key(runtime_manifest, "dotnet_gateway_unsafe_reflection").empty(),
               "fll-output runtime manifest should omit the .NET gateway deny decision");
        expect(lines_with_prefix(runtime_manifest, "dotnet_policy_allowlist_item=").empty(),
               "fll-output runtime manifest should omit the .NET allowlist items");
        expect(lines_with_prefix(runtime_manifest, "dotnet_policy_denylist_item=").empty(),
               "fll-output runtime manifest should omit the .NET denylist items");
        expect(lines_with_prefix(runtime_manifest, "dotnet_parity_matrix_item=").empty(),
               "fll-output runtime manifest should omit the .NET parity entries");
        expect(!lines_with_prefix(debug_manifest, "dotnet_policy_allowlist_item=").empty(),
               "fll-output debug manifest should preserve the .NET allowlist items");
        expect(!lines_with_prefix(debug_manifest, "dotnet_policy_denylist_item=").empty(),
               "fll-output debug manifest should preserve the .NET denylist items");
        expect(!lines_with_prefix(debug_manifest, "dotnet_parity_matrix_item=").empty(),
               "fll-output debug manifest should preserve the .NET parity entries");
        expect(debug_manifest.find("dotnet_enabled=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET availability summary");
        expect(debug_manifest.find("dotnet_policy_allowlist=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET allowlist summary");
        expect(debug_manifest.find("dotnet_policy_denylist=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET denylist summary");
        expect(debug_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET parity summary");
        expect(debug_manifest.find("dotnet_gateway_task_primitives=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET gateway allow decision");
        expect(debug_manifest.find("dotnet_gateway_unsafe_reflection=") != std::string::npos,
               "fll-output debug manifest should preserve the .NET gateway deny decision");
        const std::vector<std::string> fll_extensibility_summary_keys{
            "language_integration_count",
            "ai_feature_count",
            "extensibility_guardrail_count",
            "language_integrations",
            "ai_features"};
        for (const auto& key : fll_extensibility_summary_keys) {
            expect(manifest_value_for_key(runtime_manifest, key).empty(),
                   "fll-output runtime manifest should omit " + key);
            expect(!manifest_value_for_key(debug_manifest, key).empty(),
                   "fll-output debug manifest should preserve " + key);
        }
        expect(lines_with_prefix(runtime_manifest, "language_integration=").empty(),
               "fll-output runtime manifest should omit language integration entries");
        expect(lines_with_prefix(runtime_manifest, "ai_feature=").empty(),
               "fll-output runtime manifest should omit AI feature entries");
        expect(lines_with_prefix(runtime_manifest, "extensibility_guardrail=").empty(),
               "fll-output runtime manifest should omit extensibility guardrails");
        expect(!lines_with_prefix(debug_manifest, "language_integration=").empty(),
               "fll-output debug manifest should preserve language integration entries");
        expect(!lines_with_prefix(debug_manifest, "ai_feature=").empty(),
               "fll-output debug manifest should preserve AI feature entries");
        expect(!lines_with_prefix(debug_manifest, "extensibility_guardrail=").empty(),
               "fll-output debug manifest should preserve extensibility guardrails");
        expect(!lines_with_prefix(debug_manifest, "feature_flag=").empty(),
               "fll-output debug manifest should preserve feature-flag inventory");
        expect(debug_manifest.find("primary_output_path=" + quote_manifest_value(result.plan.launcher_output_path)) != std::string::npos,
               "fll-output debug manifest should record the requested FLL output path");
        expect(debug_manifest.find("primary_output_materialized=false") != std::string::npos,
               "fll-output debug manifest should record the honest non-materialized FLL state");
        expect(debug_manifest.find("module_definition_path=" + quote_manifest_value(result.plan.module_definition_path)) != std::string::npos,
               "fll-output debug manifest should record the module-definition path");
        expect(debug_manifest.find("fll_api_manifest_path=" + quote_manifest_value(result.plan.fll_api_manifest_path)) != std::string::npos,
               "fll-output debug manifest should record the emitted API-manifest path");
        expect(debug_manifest.find("fll_loader_entrypoint=FoxInfo") != std::string::npos,
               "fll-output debug manifest should record the FLL loader entrypoint");
        expect(debug_manifest.find("fll_registration_symbol=_FoxTable") != std::string::npos,
               "fll-output debug manifest should record the FoxTable registration symbol");
        expect(debug_manifest.find("fll_callable_signature=ParamBlk*") != std::string::npos,
               "fll-output debug manifest should record the ParamBlk callable signature");
        expect(debug_manifest.find("fll_default_return_helper=_RetInt") != std::string::npos,
               "fll-output debug manifest should record the default return helper");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.module_definition_path) + "|") != std::string::npos,
               "fll-output debug manifest should record the module-definition compiler-contract digest");
        expect(debug_manifest.find("compiler_contract=" + quote_manifest_value(result.plan.fll_api_manifest_path) + "|") != std::string::npos,
               "fll-output debug manifest should record the API-manifest compiler-contract digest");
        expect(debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the library-contract feature flag");
        expect(debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the native-wrapper feature flag");
        expect(debug_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
               "fll-output debug manifest should expose the FLL API-contract feature flag");
        expect(debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
               "fll-output debug manifest should record discovered FLL routine export symbols");
        expect(debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
               "fll-output debug manifest should record all FLL export symbols");
        expect(debug_manifest.find("library_function_kind=InitLibrary|procedure") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary routine kind");
        expect(debug_manifest.find("library_function_kind=AddNumbers|function") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers routine kind");
        expect(debug_manifest.find("library_function_source=InitLibrary|" + quote_manifest_value(librarymain_source) + "|1") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary source provenance");
        expect(debug_manifest.find("library_function_source=AddNumbers|" + quote_manifest_value(helper_source) + "|1") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers source provenance");
        expect(debug_manifest.find("library_function_parameters=InitLibrary|tcMode") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary parameter names");
        expect(debug_manifest.find("library_function_parameters=AddNumbers|tnLeft|tnRight") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers parameter names");
        expect(debug_manifest.find("library_function_parameter_declaration=InitLibrary|lparameters") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary parameter declaration style");
        expect(debug_manifest.find("library_function_parameter_declaration=AddNumbers|parameters") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers parameter declaration style");
        expect(debug_manifest.find("library_function_call_surface=InitLibrary|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output debug manifest should mirror InitLibrary callable surface");
        expect(debug_manifest.find("library_function_call_surface=AddNumbers|ParamBlk*|_RetInt") != std::string::npos,
               "fll-output debug manifest should mirror AddNumbers callable surface");
        expect(debug_manifest.find("native_wrapper_source_path=" + quote_manifest_value(result.plan.native_wrapper_source_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper source path");
        expect(debug_manifest.find("native_wrapper_cmake_path=" + quote_manifest_value(result.plan.native_wrapper_cmake_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper CMake path");
        expect(debug_manifest.find("native_wrapper_build_script_path=" + quote_manifest_value(result.plan.native_wrapper_build_script_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper shell build script path");
        expect(debug_manifest.find("native_wrapper_build_powershell_path=" + quote_manifest_value(result.plan.native_wrapper_build_powershell_path)) != std::string::npos,
               "fll-output debug manifest should record the wrapper PowerShell build script path");
        for (const auto& asset_line : runtime_asset_lines) {
            expect(debug_manifest.find(asset_line) != std::string::npos,
                   "fll-output debug manifest should mirror each staged asset line from the runtime manifest");
        }

        if (runtime_pipeline_primary_output_build_supported()) {
            const auto build_result = copperfin::runtime::build_runtime_package_primary_output(
                result.plan,
                copperfin::security::default_native_security_profile(),
                copperfin::platform::default_extensibility_profile());
            if (!build_result.ok && !build_result.error.empty()) {
                std::cerr << "FAIL: " << build_result.error << "\n";
            }
            expect(build_result.ok,
                   "fll-output runtime pipeline should build the requested primary output");
            if (build_result.ok) {
                expect(build_result.plan.primary_output_materialized,
                       "fll-output runtime pipeline should mark the primary output as materialized");
                expect(fs::exists(build_result.plan.launcher_output_path),
                       "fll-output runtime pipeline should materialize the requested FLL output");
                expect(fs::exists(build_result.plan.runtime_host_destination_path),
                       "fll-output runtime pipeline should preserve the staged runtime host after building the FLL");
                const std::string built_runtime_manifest = read_text(build_result.plan.manifest_path);
                const std::string built_debug_manifest = read_text(build_result.plan.debug_manifest_path);
                expect(built_runtime_manifest.find("primary_output_materialized=") == std::string::npos,
                       "fll-output runtime pipeline should keep the materialized primary output state out of the runtime manifest");
                expect(built_runtime_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.runtime_host_destination_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the staged runtime host as an extension payload");
                expect(built_runtime_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should record the built FLL as an extension payload");
                expect(built_debug_manifest.find("primary_output_path=" + quote_manifest_value(build_result.plan.launcher_output_path)) != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with the materialized FLL output path");
                expect(built_debug_manifest.find("project_title=LibraryDemo") != std::string::npos,
                       "fll-output runtime pipeline should preserve the project title in the rewritten debug manifest");
                expect(built_debug_manifest.find("project_path=" + quote_manifest_value((project_dir / "librarydemo.pjx").string())) != std::string::npos,
                       "fll-output runtime pipeline should preserve the project path in the rewritten debug manifest");
                expect(built_debug_manifest.find("package_root=" + quote_manifest_value(build_result.plan.package_root)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the package root in the rewritten debug manifest");
                expect(built_debug_manifest.find("content_root=" + quote_manifest_value(build_result.plan.content_root)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the content root in the rewritten debug manifest");
                expect(built_debug_manifest.find("ast_manifest_path=" + quote_manifest_value(build_result.plan.ast_manifest_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the AST manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("ir_manifest_path=" + quote_manifest_value(build_result.plan.ir_manifest_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the IR manifest path in the rewritten debug manifest");
                expect(built_debug_manifest.find("transpiled_csharp_path=" + quote_manifest_value(build_result.plan.transpiled_csharp_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the transpiled C# path in the rewritten debug manifest");
                expect(built_debug_manifest.find("configuration=debug") != std::string::npos,
                       "fll-output runtime pipeline should preserve the debug build configuration in the rewritten debug manifest");
                expect(built_debug_manifest.find("security_enabled=false") != std::string::npos,
                       "fll-output runtime pipeline should preserve the disabled security state in the rewritten debug manifest");
                expect(built_debug_manifest.find("security_role=" + quote_manifest_value(build_result.plan.security_role)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the effective security role in the rewritten debug manifest");
                expect(built_debug_manifest.find("security_mode=" + quote_manifest_value(copperfin::security::default_native_security_profile().mode)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the security mode in the rewritten debug manifest");
                expect(built_debug_manifest.find("audit_log_path=" + quote_manifest_value(build_result.plan.audit_log_path)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the audit log path in the rewritten debug manifest");
                expect(built_debug_manifest.find("runtime_host_sha256=" + quote_manifest_value(build_result.plan.runtime_host_sha256)) != std::string::npos,
                       "fll-output runtime pipeline should preserve the runtime host SHA-256 digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.runtime_host_destination_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the staged runtime host in the rewritten debug manifest");
                expect(built_debug_manifest.find("security_roles=" + std::to_string(copperfin::security::default_native_security_profile().roles.size())) != std::string::npos,
                       "fll-output runtime pipeline should preserve the security-role count in the rewritten debug manifest");
                for (const auto& key : fll_dotnet_summary_keys) {
                    const std::string value = manifest_value_for_key(built_runtime_manifest, key);
                    expect(!value.empty(),
                           "fll-output rewritten runtime manifest should provide " + key + " for debug-manifest mirroring");
                    expect(built_debug_manifest.find(key + "=" + value) != std::string::npos,
                           "fll-output runtime pipeline should preserve " + key + " in the rewritten debug manifest");
                }
                expect(manifest_value_for_key(built_runtime_manifest, "dotnet_enabled").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET availability summary");
                expect(manifest_value_for_key(built_runtime_manifest, "dotnet_gateway_task_primitives").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET gateway allow decision");
                expect(manifest_value_for_key(built_runtime_manifest, "dotnet_gateway_unsafe_reflection").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET gateway deny decision");
                expect(lines_with_prefix(built_runtime_manifest, "dotnet_policy_allowlist_item=").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET allowlist items");
                expect(lines_with_prefix(built_runtime_manifest, "dotnet_policy_denylist_item=").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET denylist items");
                expect(lines_with_prefix(built_runtime_manifest, "dotnet_parity_matrix_item=").empty(),
                       "fll-output rewritten runtime manifest should omit the .NET parity entries");
                expect(!lines_with_prefix(built_debug_manifest, "dotnet_policy_allowlist_item=").empty(),
                       "fll-output rewritten debug manifest should preserve the .NET allowlist items");
                expect(!lines_with_prefix(built_debug_manifest, "dotnet_policy_denylist_item=").empty(),
                       "fll-output rewritten debug manifest should preserve the .NET denylist items");
                expect(!lines_with_prefix(built_debug_manifest, "dotnet_parity_matrix_item=").empty(),
                       "fll-output rewritten debug manifest should preserve the .NET parity entries");
                expect(built_debug_manifest.find("dotnet_enabled=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET availability summary");
                expect(built_debug_manifest.find("dotnet_policy_allowlist=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET allowlist summary");
                expect(built_debug_manifest.find("dotnet_policy_denylist=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET denylist summary");
                expect(built_debug_manifest.find("dotnet_parity_matrix_entries=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET parity summary");
                expect(built_debug_manifest.find("dotnet_gateway_task_primitives=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET gateway allow decision");
                expect(built_debug_manifest.find("dotnet_gateway_unsafe_reflection=") != std::string::npos,
                       "fll-output rewritten debug manifest should preserve the .NET gateway deny decision");
                for (const auto& key : fll_extensibility_summary_keys) {
                    expect(manifest_value_for_key(built_runtime_manifest, key).empty(),
                           "fll-output rewritten runtime manifest should omit " + key);
                    expect(!manifest_value_for_key(built_debug_manifest, key).empty(),
                           "fll-output runtime pipeline should preserve " + key + " in the rewritten debug manifest");
                }
                expect(lines_with_prefix(built_runtime_manifest, "language_integration=").empty(),
                       "fll-output rewritten runtime manifest should omit language integration entries");
                expect(lines_with_prefix(built_runtime_manifest, "ai_feature=").empty(),
                       "fll-output rewritten runtime manifest should omit AI feature entries");
                expect(lines_with_prefix(built_runtime_manifest, "extensibility_guardrail=").empty(),
                       "fll-output rewritten runtime manifest should omit extensibility guardrails");
                expect(!lines_with_prefix(built_debug_manifest, "language_integration=").empty(),
                       "fll-output runtime pipeline should preserve language integration entries in the rewritten debug manifest");
                expect(!lines_with_prefix(built_debug_manifest, "ai_feature=").empty(),
                       "fll-output runtime pipeline should preserve AI feature entries in the rewritten debug manifest");
                expect(!lines_with_prefix(built_debug_manifest, "extensibility_guardrail=").empty(),
                       "fll-output runtime pipeline should preserve extensibility guardrails in the rewritten debug manifest");
                expect(lines_with_prefix(built_runtime_manifest, "feature_flag=").empty(),
                       "fll-output rewritten runtime manifest should omit feature-flag inventory");
                expect(!lines_with_prefix(built_debug_manifest, "feature_flag=").empty(),
                       "fll-output rewritten debug manifest should preserve feature-flag inventory");
                expect(lines_with_prefix(built_runtime_manifest, "library_function_").empty(),
                       "fll-output rewritten runtime manifest should omit library-function inventory");
                expect(built_debug_manifest.find("primary_output_materialized=true") != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with a materialized primary output state");
                expect(built_debug_manifest.find("extension_payload=" + quote_manifest_value(build_result.plan.launcher_output_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should rewrite the debug manifest with the built FLL extension-payload digest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.module_definition_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the module-definition compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("compiler_contract=" + quote_manifest_value(build_result.plan.fll_api_manifest_path) + "|") != std::string::npos,
                       "fll-output runtime pipeline should preserve the API-manifest compiler-contract digest in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.library_contract|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the library-contract feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.native_library_wrapper|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the native-wrapper feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("feature_flag=build.output.fll_api_contract|true|build_output") != std::string::npos,
                       "fll-output runtime pipeline should preserve the FLL API-contract feature flag in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=InitLibrary") != std::string::npos,
                       "fll-output runtime pipeline should preserve discovered FLL routine export symbols in the rewritten debug manifest");
                expect(built_debug_manifest.find("export_symbol=AddNumbers") != std::string::npos,
                       "fll-output runtime pipeline should preserve all FLL export symbols in the rewritten debug manifest");
                expect(!lines_with_prefix(built_debug_manifest, "library_function_").empty(),
                       "fll-output runtime pipeline should preserve library-function inventory in the rewritten debug manifest");
                const std::vector<std::string> built_runtime_asset_lines = lines_with_prefix(built_runtime_manifest, "asset=");
                expect(!built_runtime_asset_lines.empty(),
                       "fll-output runtime pipeline should preserve staged asset inventory in the rewritten runtime manifest");
                for (const auto& asset_line : built_runtime_asset_lines) {
                    expect(built_debug_manifest.find(asset_line) != std::string::npos,
                           "fll-output runtime pipeline should preserve each staged asset line in the rewritten debug manifest");
                }
                if (native_symbol_dump_is_available()) {
                    std::string symbol_error;
                    const std::set<std::string> exported_symbols = read_native_exported_symbols(build_result.plan.launcher_output_path, symbol_error);
                    const std::set<std::string> declared_module_symbols = read_module_definition_exports(build_result.plan.module_definition_path);
                    const std::set<std::string> declared_api_symbols = read_fll_api_declared_symbols(build_result.plan.fll_api_manifest_path);
                    if (exported_symbols.empty() && !symbol_error.empty()) {
                        std::cerr << "FAIL: " << symbol_error << "\n";
                    }
                    expect(exported_symbols == declared_module_symbols,
                           "fll-output runtime pipeline build should preserve the module-definition export contract");
                    expect(exported_symbols == declared_api_symbols,
                           "fll-output runtime pipeline build should preserve the API-manifest export contract");
                }
            }
        }
    }

    fs::remove_all(temp_root, ignored);
}



}  // namespace cf_test_runtime_pipeline
