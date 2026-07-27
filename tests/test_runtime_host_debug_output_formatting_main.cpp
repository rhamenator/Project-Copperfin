// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_runtime_host_debug_output_support.h"

void test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback(const std::string& runtime_host_path);
void test_runtime_host_supports_breakpoint_management_commands(const std::string& runtime_host_path);
void test_runtime_host_supports_single_breakpoint_removal(const std::string& runtime_host_path);
void test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches(const std::string& runtime_host_path);
void test_runtime_host_reports_xasset_pause_identity(const std::string& runtime_host_path);
void test_runtime_host_supports_xasset_action_breakpoint_commands(const std::string& runtime_host_path);
void test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output(const std::string& runtime_host_path);
void test_runtime_host_removes_xasset_bootstrap_after_execution(const std::string& runtime_host_path);
void test_runtime_host_cleans_failed_xasset_bootstrap_write(const std::string& runtime_host_path);
void test_runtime_host_writes_bridge_response_artifact(const std::string& runtime_host_path);
void test_security_enabled_bridge_source_stays_inside_verified_package(const std::string& runtime_host_path);
void test_runtime_host_invokes_zero_argument_bridge_export(const std::string& runtime_host_path);
void test_runtime_host_removes_bridge_routine_bootstrap_after_execution(const std::string& runtime_host_path);
void test_runtime_host_unescapes_bridge_descriptor_string_fields(const std::string& runtime_host_path);
void test_runtime_host_decodes_unicode_bridge_descriptor_paths(const std::string& runtime_host_path);
void test_runtime_host_passes_bridge_request_parameters_to_export(const std::string& runtime_host_path);
void test_runtime_host_rejects_bridge_parameter_count_mismatch(const std::string& runtime_host_path);
void test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity(const std::string& runtime_host_path);
void test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity(const std::string& runtime_host_path);
void test_runtime_host_rejects_bridge_parameter_name_mismatch(const std::string& runtime_host_path);
void test_runtime_host_rejects_bridge_request_contract_mismatch(const std::string& runtime_host_path);
void test_runtime_host_rejects_nested_bridge_descriptor_fields(const std::string& runtime_host_path);
void test_runtime_host_rejects_bridge_descriptor_identity_mismatch(const std::string& runtime_host_path);
void test_runtime_host_rejects_bridge_descriptor_metadata_mismatch(const std::string& runtime_host_path);
void test_runtime_host_usage_text_localizes_without_changing_cli_tokens(const std::string& runtime_host_path);
void test_runtime_host_debug_errors_localize_without_changing_command_tokens(const std::string& runtime_host_path);
void test_runtime_host_xasset_open_errors_follow_explicit_locale(const std::string& runtime_host_path);
void test_runtime_host_rejects_invalid_debug_command_without_execution(const std::string& runtime_host_path);
void test_runtime_host_rejects_invalid_startup_breakpoint_without_execution(const std::string& runtime_host_path);
void test_runtime_host_pause_messages_localize_without_changing_pause_reasons(const std::string& runtime_host_path);
void test_runtime_host_watch_errors_localize_without_changing_watch_fields(const std::string& runtime_host_path);
void test_runtime_host_escapes_multiline_debug_values(const std::string& runtime_host_path);
void test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens(const std::string& runtime_host_path);
void test_runtime_host_preserves_debug_state_across_prg_fault(const std::string& runtime_host_path);
void test_runtime_host_contains_executable_xasset_action_faults(const std::string& runtime_host_path);
void test_runtime_host_contains_report_label_action_faults(const std::string& runtime_host_path);
void test_runtime_host_contains_unexpected_process_fault(const std::string& runtime_host_path);

void run_runtime_host_test(
    const char* name,
    const std::string& runtime_host_path,
    void (*test)(const std::string&)) {
    std::cerr << "BEGIN: " << name << '\n';
    test(runtime_host_path);
    std::cerr << "END: " << name << '\n';
}
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "FAIL: runtime host executable path argument is required\n";
        return 1;
    }

    const std::string runtime_host_path = argv[1];
    run_runtime_host_test("breakpoint management", runtime_host_path, test_runtime_host_supports_breakpoint_management_commands);
    run_runtime_host_test("single breakpoint removal", runtime_host_path, test_runtime_host_supports_single_breakpoint_removal);
    run_runtime_host_test("implicit debug manifest", runtime_host_path, test_runtime_host_prefers_debug_manifest_for_implicit_debug_launches);
    run_runtime_host_test("compatibility launcher note", runtime_host_path, test_runtime_host_compatibility_launcher_note_reflects_xasset_fallback);
    run_runtime_host_test("xAsset pause identity", runtime_host_path, test_runtime_host_reports_xasset_pause_identity);
    run_runtime_host_test("xAsset action breakpoints", runtime_host_path, test_runtime_host_supports_xasset_action_breakpoint_commands);
    run_runtime_host_test("xAsset breakpoint metadata", runtime_host_path, test_runtime_host_surfaces_xasset_breakpoint_metadata_in_pause_output);
    run_runtime_host_test("xAsset bootstrap cleanup", runtime_host_path, test_runtime_host_removes_xasset_bootstrap_after_execution);
    run_runtime_host_test("xAsset failed-write cleanup", runtime_host_path, test_runtime_host_cleans_failed_xasset_bootstrap_write);
    run_runtime_host_test("bridge response", runtime_host_path, test_runtime_host_writes_bridge_response_artifact);
    run_runtime_host_test("verified bridge source", runtime_host_path, test_security_enabled_bridge_source_stays_inside_verified_package);
    run_runtime_host_test("zero-argument bridge export", runtime_host_path, test_runtime_host_invokes_zero_argument_bridge_export);
    run_runtime_host_test("bridge bootstrap cleanup", runtime_host_path, test_runtime_host_removes_bridge_routine_bootstrap_after_execution);
    run_runtime_host_test("escaped bridge descriptor", runtime_host_path, test_runtime_host_unescapes_bridge_descriptor_string_fields);
    run_runtime_host_test("Unicode bridge descriptor", runtime_host_path, test_runtime_host_decodes_unicode_bridge_descriptor_paths);
    run_runtime_host_test("bridge parameters", runtime_host_path, test_runtime_host_passes_bridge_request_parameters_to_export);
    run_runtime_host_test("bridge parameter count mismatch", runtime_host_path, test_runtime_host_rejects_bridge_parameter_count_mismatch);
    run_runtime_host_test("nested bridge parameter array", runtime_host_path, test_runtime_host_rejects_nested_bridge_parameter_array_for_nonzero_arity);
    run_runtime_host_test("nested bridge parameter values", runtime_host_path, test_runtime_host_rejects_nested_bridge_parameter_values_for_nonzero_arity);
    run_runtime_host_test("bridge parameter name mismatch", runtime_host_path, test_runtime_host_rejects_bridge_parameter_name_mismatch);
    run_runtime_host_test("bridge request contract", runtime_host_path, test_runtime_host_rejects_bridge_request_contract_mismatch);
    run_runtime_host_test("nested bridge descriptor", runtime_host_path, test_runtime_host_rejects_nested_bridge_descriptor_fields);
    run_runtime_host_test("bridge descriptor identity", runtime_host_path, test_runtime_host_rejects_bridge_descriptor_identity_mismatch);
    run_runtime_host_test("bridge descriptor metadata", runtime_host_path, test_runtime_host_rejects_bridge_descriptor_metadata_mismatch);
    run_runtime_host_test("usage localization", runtime_host_path, test_runtime_host_usage_text_localizes_without_changing_cli_tokens);
    run_runtime_host_test("debug error localization", runtime_host_path, test_runtime_host_debug_errors_localize_without_changing_command_tokens);
    run_runtime_host_test("xAsset open localization", runtime_host_path, test_runtime_host_xasset_open_errors_follow_explicit_locale);
    run_runtime_host_test("invalid debug command rejection", runtime_host_path, test_runtime_host_rejects_invalid_debug_command_without_execution);
    run_runtime_host_test("invalid startup breakpoint rejection", runtime_host_path, test_runtime_host_rejects_invalid_startup_breakpoint_without_execution);
    run_runtime_host_test("pause localization", runtime_host_path, test_runtime_host_pause_messages_localize_without_changing_pause_reasons);
    run_runtime_host_test("watch localization", runtime_host_path, test_runtime_host_watch_errors_localize_without_changing_watch_fields);
    run_runtime_host_test("multiline debug values", runtime_host_path, test_runtime_host_escapes_multiline_debug_values);
    run_runtime_host_test("quit prompt localization", runtime_host_path, test_runtime_host_quit_prompt_localizes_without_changing_confirmation_tokens);
    run_runtime_host_test("PRG fault debugger recovery", runtime_host_path, test_runtime_host_preserves_debug_state_across_prg_fault);
    run_runtime_host_test("xAsset action fault containment", runtime_host_path, test_runtime_host_contains_executable_xasset_action_faults);
    run_runtime_host_test("report/label action fault containment", runtime_host_path, test_runtime_host_contains_report_label_action_faults);
    run_runtime_host_test("unexpected process fault containment", runtime_host_path, test_runtime_host_contains_unexpected_process_fault);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host debug-output formatting tests passed\n";
    return 0;
}
