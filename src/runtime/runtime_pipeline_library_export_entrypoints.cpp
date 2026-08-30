// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#include <locale>

namespace copperfin::runtime {

namespace runtime_pipeline_detail {

void append_native_wrapper_library_entrypoint_source(std::ostringstream& stream, const RuntimePackagePlan& plan) {
    if (plan.output_kind == BuildOutputKind::fll) {
        const auto parameter_counts = collect_library_export_parameter_counts(plan);
        const auto parameter_names = collect_library_export_parameter_names(plan);
        const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
        const auto routine_kinds = collect_library_export_routine_kinds(plan);
        const auto routine_locations = collect_library_export_routine_locations(plan);
        stream << "struct ParamBlk {\n";
        stream << "    int reserved = 0;\n";
        stream << "};\n\n";
        stream << "static std::string copperfin_build_runtime_bridge_fll_int_return_surface() {\n";
        stream << "    return \"" << kFllDefaultReturnHelper << "(int)\";\n";
        stream << "}\n\n";
        stream << "static int " << kFllDefaultReturnHelper << "(int value) {\n";
        stream << "    return value;\n";
        stream << "}\n\n";
        stream << "using CopperfinFllEntryPoint = int (*)(ParamBlk*);\n\n";
        stream << "struct CopperfinFoxInfoRecord {\n";
        stream << "    const char* function_name;\n";
        stream << "    CopperfinFllEntryPoint entrypoint;\n";
        stream << "    const char* routine_kind;\n";
        stream << "    const char* source_path;\n";
        stream << "    unsigned int source_line;\n";
        stream << "    const char* parameter_declaration_kind;\n";
        stream << "    const char* parameter_names;\n";
        stream << "    unsigned int parameter_count;\n";
        stream << "};\n\n";
        stream << "struct CopperfinFoxTableRecord {\n";
        stream << "    const CopperfinFoxTableRecord* previous;\n";
        stream << "    unsigned int entry_count;\n";
        stream << "    const CopperfinFoxInfoRecord* entries;\n";
        stream << "};\n\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_name_manifest =
                names_found == parameter_names.end()
                    ? std::string{}
                    : build_manifest_parameter_names(names_found->second);
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            stream << "COPPERFIN_EXPORT int " << symbol << "(ParamBlk* parm) {\n";
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface(),\n";
            stream << "            " << kFllDefaultReturnHelper << ");\n";
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "), stub_emission_wrapper);\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(\n";
            stream << "        descriptor);\n";
            stream << "    const auto call = copperfin_build_runtime_bridge_call(\n";
            stream << "        invocation,\n";
            stream << "        {{\"parm\", std::to_string(static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(parm))), \"ParamBlk*\"}});\n";
            stream << "    const auto placeholder_return_binding =\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_binding(\n";
            stream << "            copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto result = copperfin_build_runtime_bridge_result(\n";
            stream << "        call,\n";
            stream << "        placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n";
            stream << "        result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n";
            stream << "        launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n";
            stream << "        observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n";
            stream << "        execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n";
            stream << "        transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n";
            stream << "        serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n";
            stream << "        dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_fll_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto request_write_execution =\n";
            stream << "        copperfin_runtime_bridge_execute_write_request(request_write_plan);\n";
            stream << "    const auto process_launch = request_write_execution\n";
            stream << "        ? copperfin_runtime_bridge_launch_process(dispatch_execution)\n";
            stream << "        : copperfin_runtime_bridge_failed_process_launch(dispatch_execution);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan,\n";
            stream << "        request_write_execution);\n";
            stream << "    const auto response_document =\n";
            stream << "        copperfin_runtime_bridge_execute_read_response(response_read_plan);\n";
            stream << "    copperfin_runtime_bridge_cleanup_artifacts(response_read_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(\n";
            stream << "            host_failure,\n";
            stream << "            response_read_plan,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(\n";
            stream << "            missing_response,\n";
            stream << "            response_validation,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan,\n";
            stream << "        response_document);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    const auto parsed_response =\n";
            stream << "        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan,\n";
            stream << "        parsed_response);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    const auto interpreted_result =\n";
            stream << "        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan,\n";
            stream << "        interpreted_result);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    const auto native_return =\n";
            stream << "        copperfin_runtime_bridge_execute_native_return(native_return_plan);\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan,\n";
            stream << "        native_return);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    const auto outcome_selection =\n";
            stream << "        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan,\n";
            stream << "        outcome_selection);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    const auto return_materialization =\n";
            stream << "        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan,\n";
            stream << "        return_materialization);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    const auto return_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        return_emission,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    const auto final_return_adoption =\n";
            stream << "        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan,\n";
            stream << "        final_return_adoption);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    const auto return_activation =\n";
            stream << "        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan,\n";
            stream << "        return_activation);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    const auto stub_return =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_return);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_value =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);\n";
            stream << "    const auto stub_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);\n";
            stream << "    const auto stub_emission_return_surface =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
            stream << "            stub_emission,\n";
            stream << "            placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface);\n";
            stream << "    return copperfin_runtime_bridge_apply_stub_emission_output(\n";
            stream << "        stub_emission_return_surface,\n";
            stream << "        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);\n";
            stream << "}\n\n";
        }
        stream << "static const CopperfinFoxInfoRecord kCopperfinFoxInfo[] = {\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_name_manifest =
                names_found == parameter_names.end()
                    ? std::string{}
                    : build_manifest_parameter_names(names_found->second);
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            stream << "    {\"" << symbol << "\", &" << symbol
                   << ", \"" << routine_kind << "\""
                   << ", \"" << quote_manifest_value(location.file_path) << "\""
                   << ", " << location.line << "U"
                   << ", \"" << parameter_declaration_kind << "\""
                   << ", \"" << parameter_name_manifest << "\""
                   << ", " << parameter_count << "U},\n";
        }
        stream << "};\n\n";
        stream << "COPPERFIN_EXPORT const CopperfinFoxTableRecord " << kFllRegistrationSymbol << " = {\n";
        stream << "    nullptr,\n";
        stream << "    static_cast<unsigned int>(sizeof(kCopperfinFoxInfo) / sizeof(kCopperfinFoxInfo[0])),\n";
        stream << "    kCopperfinFoxInfo\n";
        stream << "};\n\n";
        stream << "COPPERFIN_EXPORT const CopperfinFoxTableRecord* " << kFllLoaderEntrypoint << "() {\n";
        stream << "    return &" << kFllRegistrationSymbol << ";\n";
        stream << "}\n";
        return;
    }

    if (plan.output_kind == BuildOutputKind::dll || plan.output_kind == BuildOutputKind::ocx) {
        const auto parameter_counts = collect_library_export_parameter_counts(plan);
        const auto parameter_names = collect_library_export_parameter_names(plan);
        const auto parameter_declaration_kinds = collect_library_export_parameter_declaration_kinds(plan);
        const auto routine_kinds = collect_library_export_routine_kinds(plan);
        const auto routine_locations = collect_library_export_routine_locations(plan);
        stream << "#if defined(_WIN32) && defined(_M_IX86)\n";
        stream << "#define COPPERFIN_VFP_DLL_CALL __stdcall\n";
        stream << "#else\n";
        stream << "#define COPPERFIN_VFP_DLL_CALL\n";
        stream << "#endif\n\n";
        stream << "static int copperfin_runtime_bridge_return_native_int(int value) {\n";
        stream << "    return value;\n";
        stream << "}\n\n";
        for (const auto& symbol : plan.exported_symbols) {
            const auto found = parameter_counts.find(symbol);
            const std::size_t parameter_count = found == parameter_counts.end() ? 0U : found->second;
            const auto names_found = parameter_names.find(symbol);
            const auto declaration_kind_found = parameter_declaration_kinds.find(symbol);
            const auto kind_found = routine_kinds.find(symbol);
            const auto location_found = routine_locations.find(symbol);
            const std::vector<std::string> effective_names =
                names_found == parameter_names.end()
                    ? std::vector<std::string>(parameter_count, std::string{})
                    : names_found->second;
            const std::string routine_kind =
                kind_found == routine_kinds.end() ? std::string("function") : kind_found->second;
            const SourceLocation location =
                location_found == routine_locations.end() ? SourceLocation{} : location_found->second;
            const std::string parameter_declaration_kind =
                declaration_kind_found == parameter_declaration_kinds.end()
                    ? std::string{}
                    : declaration_kind_found->second;
            const std::string parameter_name_manifest = build_manifest_parameter_names(effective_names);
            stream << "COPPERFIN_EXPORT int COPPERFIN_VFP_DLL_CALL " << symbol << "("
                   << build_placeholder_int_parameter_list(effective_names) << ") {\n";
            stream << "    const auto stub_emission_wrapper =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_wrapper(\n";
            stream << "            copperfin_build_runtime_bridge_native_int_return_surface(),\n";
            stream << "            copperfin_runtime_bridge_return_native_int);\n";
            stream << "    const auto descriptor = copperfin_build_runtime_bridge_descriptor(\""
                   << quote_manifest_value(symbol) << "\", \"" << quote_manifest_value(routine_kind)
                   << "\", \"" << quote_manifest_value(location.file_path) << "\", " << location.line
                   << "U, \"" << quote_manifest_value(parameter_declaration_kind) << "\", \""
                   << quote_manifest_value(parameter_name_manifest) << "\", " << parameter_count
                   << "U, reinterpret_cast<void*>(&" << symbol << "), stub_emission_wrapper);\n";
            stream << "    const auto invocation = copperfin_build_runtime_bridge_invocation(\n";
            stream << "        descriptor);\n";
            stream << "    const auto call = copperfin_build_runtime_bridge_call(\n";
            stream << "        invocation,\n";
            stream << "        {";
            for (std::size_t index = 0; index < effective_names.size(); ++index) {
                if (index > 0U) {
                    stream << ", ";
                }
                const std::string parameter_name = effective_names[index];
                const std::string sanitized_name = sanitize_cpp_identifier(effective_names[index], index);
                stream << "{\"" << quote_manifest_value(parameter_name) << "\", std::to_string(" << sanitized_name
                       << "), \"int\"}";
            }
            stream << "});\n";
            stream << "    const auto placeholder_return_binding =\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_binding(\"int\");\n";
            stream << "    const auto result = copperfin_build_runtime_bridge_result(\n";
            stream << "        call,\n";
            stream << "        placeholder_return_binding);\n";
            stream << "    const auto launch_plan = copperfin_build_runtime_bridge_launch_plan(\n";
            stream << "        result);\n";
            stream << "    const auto observation_plan = copperfin_build_runtime_bridge_observation_plan(\n";
            stream << "        launch_plan);\n";
            stream << "    const auto execution_plan = copperfin_build_runtime_bridge_execution_plan(\n";
            stream << "        observation_plan);\n";
            stream << "    const auto transport_plan = copperfin_build_runtime_bridge_transport_plan(\n";
            stream << "        execution_plan);\n";
            stream << "    const auto serialization_plan = copperfin_build_runtime_bridge_serialization_plan(\n";
            stream << "        transport_plan);\n";
            stream << "    const auto dispatch_plan = copperfin_build_runtime_bridge_dispatch_plan(\n";
            stream << "        serialization_plan);\n";
            stream << "    const auto dispatch_execution = copperfin_runtime_bridge_execute_dispatch(dispatch_plan);\n";
            stream << "    const auto payload_plan = copperfin_build_runtime_bridge_payload_plan(\n";
            stream << "        dispatch_plan);\n";
            stream << "    const auto interpretation_plan = copperfin_build_runtime_bridge_interpretation_plan(\n";
            stream << "        payload_plan,\n";
            stream << "        copperfin_build_runtime_bridge_native_int_return_surface());\n";
            stream << "    const auto failure_policy = copperfin_build_runtime_bridge_failure_policy_plan(\n";
            stream << "        interpretation_plan,\n";
            stream << "        placeholder_return_binding.value_representation);\n";
            stream << "    const auto response_validation = copperfin_build_runtime_bridge_response_validation_plan(\n";
            stream << "        failure_policy);\n";
            stream << "    const auto request_artifact = copperfin_build_runtime_bridge_request_artifact(\n";
            stream << "        response_validation);\n";
            stream << "    const auto request_write_plan = copperfin_build_runtime_bridge_request_write_plan(\n";
            stream << "        request_artifact);\n";
            stream << "    const auto request_write_execution =\n";
            stream << "        copperfin_runtime_bridge_execute_write_request(request_write_plan);\n";
            stream << "    const auto process_launch = request_write_execution\n";
            stream << "        ? copperfin_runtime_bridge_launch_process(dispatch_execution)\n";
            stream << "        : copperfin_runtime_bridge_failed_process_launch(dispatch_execution);\n";
            stream << "    const auto host_failure =\n";
            stream << "        copperfin_runtime_bridge_evaluate_host_failure(process_launch, failure_policy);\n";
            stream << "    const auto response_read_plan = copperfin_build_runtime_bridge_response_read_plan(\n";
            stream << "        request_write_plan,\n";
            stream << "        request_write_execution);\n";
            stream << "    const auto response_document =\n";
            stream << "        copperfin_runtime_bridge_execute_read_response(response_read_plan);\n";
            stream << "    copperfin_runtime_bridge_cleanup_artifacts(response_read_plan);\n";
            stream << "    const auto missing_response =\n";
            stream << "        copperfin_runtime_bridge_evaluate_missing_response(\n";
            stream << "            host_failure,\n";
            stream << "            response_read_plan,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_validation_evaluation =\n";
            stream << "        copperfin_runtime_bridge_evaluate_response_validation(\n";
            stream << "            missing_response,\n";
            stream << "            response_validation,\n";
            stream << "            response_document);\n";
            stream << "    const auto response_artifact = copperfin_build_runtime_bridge_response_artifact(\n";
            stream << "        response_read_plan,\n";
            stream << "        response_document);\n";
            stream << "    const auto response_parse_plan = copperfin_build_runtime_bridge_response_parse_plan(\n";
            stream << "        response_artifact);\n";
            stream << "    const auto response_parse_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_response_parse(response_validation_evaluation, response_parse_plan);\n";
            stream << "    const auto parsed_response =\n";
            stream << "        copperfin_runtime_bridge_execute_parse_response(response_parse_plan);\n";
            stream << "    const auto interpreted_result_plan = copperfin_build_runtime_bridge_interpreted_result_plan(\n";
            stream << "        response_parse_plan,\n";
            stream << "        parsed_response);\n";
            stream << "    const auto interpreted_result_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_interpreted_result(response_parse_admission, interpreted_result_plan);\n";
            stream << "    const auto interpreted_result =\n";
            stream << "        copperfin_runtime_bridge_execute_interpreted_result(interpreted_result_plan);\n";
            stream << "    const auto native_return_plan = copperfin_build_runtime_bridge_native_return_plan(\n";
            stream << "        result,\n";
            stream << "        interpreted_result_plan,\n";
            stream << "        interpreted_result);\n";
            stream << "    const auto native_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_native_return(interpreted_result_admission, native_return_plan);\n";
            stream << "    const auto native_return =\n";
            stream << "        copperfin_runtime_bridge_execute_native_return(native_return_plan);\n";
            stream << "    const auto outcome_selection_plan = copperfin_build_runtime_bridge_outcome_selection_plan(\n";
            stream << "        native_return_plan,\n";
            stream << "        native_return);\n";
            stream << "    const auto outcome_selection_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_outcome_selection(native_return_admission, outcome_selection_plan);\n";
            stream << "    const auto outcome_selection =\n";
            stream << "        copperfin_runtime_bridge_execute_outcome_selection(outcome_selection_plan);\n";
            stream << "    const auto return_materialization_plan = copperfin_build_runtime_bridge_return_materialization_plan(\n";
            stream << "        outcome_selection_plan,\n";
            stream << "        outcome_selection);\n";
            stream << "    const auto return_materialization_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_materialization(outcome_selection_admission, return_materialization_plan);\n";
            stream << "    const auto return_materialization =\n";
            stream << "        copperfin_runtime_bridge_execute_return_materialization(return_materialization_plan);\n";
            stream << "    const auto return_emission_plan = copperfin_build_runtime_bridge_return_emission_plan(\n";
            stream << "        return_materialization_plan,\n";
            stream << "        return_materialization);\n";
            stream << "    const auto return_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_emission(return_materialization_admission, return_emission_plan);\n";
            stream << "    const auto return_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_return_emission(return_emission_plan);\n";
            stream << "    const auto final_return_adoption_plan = copperfin_build_runtime_bridge_final_return_adoption_plan(\n";
            stream << "        return_emission_plan,\n";
            stream << "        return_emission,\n";
            stream << "        copperfin_build_runtime_bridge_placeholder_return_statement(placeholder_return_binding));\n";
            stream << "    const auto final_return_adoption_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_final_return_adoption(return_emission_admission, final_return_adoption_plan);\n";
            stream << "    const auto final_return_adoption =\n";
            stream << "        copperfin_runtime_bridge_execute_final_return_adoption(final_return_adoption_plan);\n";
            stream << "    const auto return_activation_plan = copperfin_build_runtime_bridge_return_activation_plan(\n";
            stream << "        final_return_adoption_plan,\n";
            stream << "        final_return_adoption);\n";
            stream << "    const auto return_activation_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_return_activation(final_return_adoption_admission, return_activation_plan);\n";
            stream << "    const auto return_activation =\n";
            stream << "        copperfin_runtime_bridge_execute_return_activation(return_activation_plan);\n";
            stream << "    const auto stub_return_plan = copperfin_build_runtime_bridge_stub_return_plan(\n";
            stream << "        return_activation_plan,\n";
            stream << "        return_activation);\n";
            stream << "    const auto stub_return_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_return(return_activation_admission, stub_return_plan);\n";
            stream << "    const auto stub_return =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_return(stub_return_plan);\n";
            stream << "    const auto placeholder_return_value_plan = copperfin_build_runtime_bridge_placeholder_return_value_plan(\n";
            stream << "        stub_return_plan,\n";
            stream << "        stub_return);\n";
            stream << "    const auto placeholder_return_value_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_value(stub_return_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_value =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_value(placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_placeholder_return_int(placeholder_return_value_admission, placeholder_return_value_plan);\n";
            stream << "    const auto placeholder_return_int =\n";
            stream << "        copperfin_runtime_bridge_execute_placeholder_return_int(placeholder_return_int_admission);\n";
            stream << "    const auto stub_emission_admission =\n";
            stream << "        copperfin_runtime_bridge_admit_stub_emission(placeholder_return_int_admission, placeholder_return_int);\n";
            stream << "    const auto stub_emission =\n";
            stream << "        copperfin_runtime_bridge_execute_stub_emission(stub_emission_admission);\n";
            stream << "    const auto stub_emission_return_surface =\n";
            stream << "        copperfin_runtime_bridge_build_stub_emission_return_surface(\n";
            stream << "            stub_emission,\n";
            stream << "            placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.native_return_surface);\n";
            stream << "    return copperfin_runtime_bridge_apply_stub_emission_output(\n";
            stream << "        stub_emission_return_surface,\n";
            stream << "        placeholder_return_value_plan.stub_return_plan.return_activation_plan.final_return_adoption_plan.return_emission_plan.return_materialization_plan.outcome_selection_plan.native_return_plan.interpreted_result_plan.response_parse_plan.response_artifact.response_read_plan.request_write_plan.request_artifact.response_validation_plan.failure_policy_plan.interpretation_plan.payload_plan.dispatch_plan.serialization_plan.transport_plan.execution_plan.observation_plan.launch_plan.result.call.invocation.descriptor.stub_emission_wrapper.return_adapter);\n";
            stream << "}\n\n";
        }
        return;
    }

    for (const auto& symbol : plan.exported_symbols) {
        stream << "COPPERFIN_EXPORT int " << symbol << "() {\n";
        stream << "    return -1;\n";
        stream << "}\n\n";
    }
}

}  // namespace runtime_pipeline_detail

}  // namespace copperfin::runtime
