// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_runtime_host_debug_output_support.h"

// Safety-relevant coverage: these tests exercise immutable audit-chain and integrity contracts.

void test_security_enabled_report_and_label_execute_verified_snapshots(const std::string& runtime_host_path);
void test_security_enabled_query_file_uses_verified_snapshot(const std::string& runtime_host_path);
void test_security_enabled_form_class_and_menu_companion_integrity(const std::string& runtime_host_path);
void test_runtime_host_preserves_logical_identity_across_nested_directory_aliases(const std::string& runtime_host_path);
void test_app_cfdebug_preserves_external_xasset_source_compatibility(const std::string& runtime_host_path);
void test_app_cfdebug_rejects_inaccessible_external_startup_source(const std::string& runtime_host_path);
void test_app_cfdebug_rejects_file_valued_working_directory(const std::string& runtime_host_path);
void test_security_enabled_writable_package_data_contract(const std::string& runtime_host_path);
void test_runtime_host_rejects_extension_payload_basename_fallback(const std::string& runtime_host_path);
void test_runtime_host_accepts_escaped_manifest_pipe_fields(const std::string& runtime_host_path);
void test_runtime_host_preserves_escaped_pipe_in_direct_manifest_paths(const std::string& runtime_host_path);
void test_runtime_host_manifest_verification_errors_localize_without_changing_contracts(const std::string& runtime_host_path);
void test_runtime_host_rejects_audit_paths_outside_the_direct_package(const std::string& runtime_host_path);
void test_runtime_host_rejects_malformed_security_enabled_before_startup(const std::string& runtime_host_path);
void test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts(const std::string& runtime_host_path);
void test_runtime_host_validates_manifest_versions_without_changing_error_contracts(const std::string& runtime_host_path);
void test_runtime_host_debug_privileges_require_debug_document_contract(const std::string& runtime_host_path);
void test_runtime_host_rejects_ai_federation_planning_without_ai_permission(const std::string& runtime_host_path);

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
    run_runtime_host_test("verified report and label snapshots", runtime_host_path, test_security_enabled_report_and_label_execute_verified_snapshots);
    run_runtime_host_test("verified query-file snapshot", runtime_host_path, test_security_enabled_query_file_uses_verified_snapshot);
    run_runtime_host_test("verified xAsset companions", runtime_host_path, test_security_enabled_form_class_and_menu_companion_integrity);
    run_runtime_host_test("nested directory alias identity", runtime_host_path, test_runtime_host_preserves_logical_identity_across_nested_directory_aliases);
    run_runtime_host_test("external xAsset debug source", runtime_host_path, test_app_cfdebug_preserves_external_xasset_source_compatibility);
    run_runtime_host_test("inaccessible external debug source", runtime_host_path, test_app_cfdebug_rejects_inaccessible_external_startup_source);
    run_runtime_host_test("file-valued working directory", runtime_host_path, test_app_cfdebug_rejects_file_valued_working_directory);
    run_runtime_host_test("writable package data", runtime_host_path, test_security_enabled_writable_package_data_contract);
    run_runtime_host_test("extension payload basename fallback", runtime_host_path, test_runtime_host_rejects_extension_payload_basename_fallback);
    run_runtime_host_test("escaped manifest pipe fields", runtime_host_path, test_runtime_host_accepts_escaped_manifest_pipe_fields);
    run_runtime_host_test("escaped direct manifest paths", runtime_host_path, test_runtime_host_preserves_escaped_pipe_in_direct_manifest_paths);
    run_runtime_host_test("manifest versions", runtime_host_path, test_runtime_host_validates_manifest_versions_without_changing_error_contracts);
    run_runtime_host_test("debug privilege document", runtime_host_path, test_runtime_host_debug_privileges_require_debug_document_contract);
    run_runtime_host_test("manifest verification localization", runtime_host_path, test_runtime_host_manifest_verification_errors_localize_without_changing_contracts);
    run_runtime_host_test("audit package boundary", runtime_host_path, test_runtime_host_rejects_audit_paths_outside_the_direct_package);
    run_runtime_host_test("malformed security metadata", runtime_host_path, test_runtime_host_rejects_malformed_security_enabled_before_startup);
    run_runtime_host_test("security audit localization", runtime_host_path, test_runtime_host_security_denial_audit_details_localize_without_changing_audit_contracts);
    run_runtime_host_test("AI federation permission", runtime_host_path, test_runtime_host_rejects_ai_federation_planning_without_ai_permission);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed\n";
        return 1;
    }

    std::cout << "All runtime host audit-stream tests passed\n";
    return 0;
}
