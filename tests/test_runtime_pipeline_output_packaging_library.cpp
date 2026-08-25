// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_runtime_pipeline_output_packaging_support.h"

namespace cf_test_runtime_pipeline {

#include "test_runtime_pipeline_output_packaging_library_definition_contracts.inl"

void test_native_wrapper_primary_output_handles_literal_shell_paths() {
    namespace fs = std::filesystem;
    if (!cmake_is_available()) {
        return;
    }

    const fs::path temp_root = fs::temp_directory_path() /
        "copperfin runtime pipeline $(literal) %TEMP%";
    const fs::path project_dir = temp_root / "project";
    const fs::path output_dir = temp_root / "output";
    const fs::path runtime_host = runtime_host_fixture_path(temp_root);
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(project_dir);

    write_text(project_dir / "literalpathmain.prg",
               "PROCEDURE InitLibrary\nLPARAMETERS tcMode\nRETURN\nENDPROC\n");
    write_text(runtime_host, "runtime-host");

    copperfin::studio::StudioDocumentModel document;
    document.path = (project_dir / "literalpathdemo.pjx").string();

    copperfin::studio::StudioProjectWorkspace workspace;
    workspace.available = true;
    workspace.project_title = "LiteralPathDemo";
    workspace.home_directory = project_dir.string();
    workspace.build_plan.available = true;
    workspace.build_plan.can_build = true;
    workspace.build_plan.project_title = "LiteralPathDemo";
    workspace.build_plan.output_path = (output_dir / "LiteralPathDemo.dll").string();
    workspace.build_plan.output_kind = "dll";
    workspace.build_plan.build_target = "x64 Windows dynamic-link library";
    workspace.build_plan.startup_item = "literalpathmain.prg";
    workspace.build_plan.startup_record_index = 1U;
    workspace.entries = {
        {.record_index = 1U, .name = "literalpathmain.prg", .relative_path = "literalpathmain.prg", .type_title = "Program"}
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
    expect(plan.ok, "literal-shell-path library plan should be created");
    if (!plan.ok) {
        fs::remove_all(temp_root, ignored);
        return;
    }

    const auto materialized = copperfin::runtime::materialize_runtime_package(
        plan,
        copperfin::security::default_native_security_profile(),
        copperfin::platform::default_extensibility_profile(),
        runtime_host.string());
    expect_materialization(materialized, "literal-shell-path library package should materialize");
    if (materialized.ok) {
        const auto built = copperfin::runtime::build_runtime_package_primary_output(
            materialized.plan,
            copperfin::security::default_native_security_profile(),
            copperfin::platform::default_extensibility_profile());
        if (!built.ok && !built.error.empty()) {
            std::cerr << "FAIL: " << built.error << "\n";
        }
        expect(built.ok,
               "native-wrapper primary output should treat shell metacharacters as literal path data");
        if (built.ok) {
            expect(fs::exists(built.plan.launcher_output_path),
                   "literal-shell-path native-wrapper build should materialize the requested DLL");
            expect(built.plan.launcher_output_path.find("%TEMP%") != std::string::npos,
                   "literal-shell-path native-wrapper build should preserve percent path text");
        }
    }

    fs::remove_all(temp_root, ignored);
}



}  // namespace cf_test_runtime_pipeline
