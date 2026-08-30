// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <locale>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace copperfin::runtime {
namespace {

std::atomic<unsigned long long> native_wrapper_build_sequence{0};

bool path_exists_without_error(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::exists(path, error) && !error;
}

}  // namespace

RuntimeBuildResult build_runtime_package_primary_output(
    const RuntimePackagePlan& plan,
    const security::NativeSecurityProfile& security_profile,
    const platform::ExtensibilityProfile& extensibility_profile) {
    std::string error;
    if (!validate_public_output_artifact_name(plan, error)) {
        return {.ok = false, .error = error};
    }
    if (!plan.ok) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PlanInvalid")};
    }
    if (!is_library_output_kind(plan.output_kind)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.PrimaryOutputRequiresLibraryOutput")};
    }
    if (!path_exists_without_error(
            copperfin::platform::path_from_utf8_string(plan.native_wrapper_cmake_path))) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperCMakeMissing")};
    }

    RuntimePackagePlan built_plan = plan;
    const std::filesystem::path source_root =
        copperfin::platform::path_from_utf8_string(plan.native_wrapper_cmake_path).parent_path();
    const std::filesystem::path original_build_root = source_root / "cmake_pipeline_build";
    const std::filesystem::path staging_root =
        std::filesystem::temp_directory_path() /
        ("copperfin-native-wrapper-" +
#if defined(_WIN32)
            std::to_string(static_cast<unsigned long long>(::GetCurrentProcessId())) +
#else
            std::to_string(static_cast<unsigned long long>(::getpid())) +
#endif
            "-" +
            std::to_string(++native_wrapper_build_sequence));
    const std::filesystem::path staging_package_root = staging_root / "package";
    const std::filesystem::path staging_source_root = staging_package_root / "wrapper";
    const std::filesystem::path build_root = staging_source_root / "cmake_pipeline_build";
    const std::filesystem::path configure_log_path = build_root / "cmake-configure.log";
    const std::filesystem::path build_log_path = build_root / "cmake-build.log";
    const std::filesystem::path staged_output_path = staging_package_root /
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path).filename();
    // CMake-generated Makefiles interpret shell-like path text; build from a
    // private safe-path copy and publish only the requested primary artifact.
    std::error_code ignored;
    std::filesystem::remove_all(original_build_root, ignored);
    std::filesystem::remove_all(staging_root, ignored);
    std::filesystem::remove(
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
        ignored);
    std::filesystem::create_directories(staging_package_root, ignored);
    std::filesystem::copy(
        source_root,
        staging_source_root,
        std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing,
        ignored);
    if (!ignored) {
        std::filesystem::create_directories(build_root, ignored);
    }
    if (!ignored) {
        const std::filesystem::path module_definition_path =
            copperfin::platform::path_from_utf8_string(plan.module_definition_path);
        if (path_exists_without_error(module_definition_path)) {
            std::filesystem::copy_file(
                module_definition_path,
                staging_package_root / module_definition_path.filename(),
                std::filesystem::copy_options::overwrite_existing,
                ignored);
        }
    }
    if (ignored) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }

    const NativeWrapperProcessResult configure_result = run_native_wrapper_process(
#if defined(_WIN32)
        "cmake.exe",
#else
        "cmake",
#endif
        {
            "-S",
            copperfin::platform::path_to_utf8_string(staging_source_root),
            "-B",
            copperfin::platform::path_to_utf8_string(build_root)},
        configure_log_path);
    if (!configure_result.started || configure_result.exit_code != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputConfigureFailed");
        if (path_exists_without_error(configure_log_path)) {
            error += ":\n" + read_text_file(configure_log_path);
        }
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = error};
    }

    const NativeWrapperProcessResult build_result = run_native_wrapper_process(
#if defined(_WIN32)
        "cmake.exe",
#else
        "cmake",
#endif
        {
            "--build",
            copperfin::platform::path_to_utf8_string(build_root)},
        build_log_path);
    if (!build_result.started || build_result.exit_code != 0) {
        error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputBuildFailed");
        if (path_exists_without_error(build_log_path)) {
            error += ":\n" + read_text_file(build_log_path);
        }
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = error};
    }

    if (!path_exists_without_error(staged_output_path)) {
        std::filesystem::remove_all(staging_root, ignored);
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    std::error_code copy_error;
    std::filesystem::copy_file(
        staged_output_path,
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
        std::filesystem::copy_options::overwrite_existing,
        copy_error);
    std::error_code cleanup_error;
    std::filesystem::remove_all(staging_root, cleanup_error);
    if (copy_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    return finalize_runtime_package_primary_output(
        built_plan,
        security_profile,
        extensibility_profile);
}

}  // namespace copperfin::runtime
