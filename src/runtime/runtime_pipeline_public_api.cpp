// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "runtime_pipeline_support.h"
#include "copperfin/platform/private_directory.h"

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
#include "runtime_pipeline_test_hooks.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <exception>
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
#include <bcrypt.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#if defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__)
#include <cstdlib>
#endif
#endif

namespace copperfin::runtime {
namespace {

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
std::atomic<bool> fail_native_wrapper_staging_create_once{false};
#endif

bool fill_staging_random_bytes(std::array<unsigned char, 16>& bytes) noexcept {
#if defined(_WIN32)
    return ::BCryptGenRandom(nullptr, bytes.data(), static_cast<ULONG>(bytes.size()),
                             BCRYPT_USE_SYSTEM_PREFERRED_RNG) >= 0;
#elif defined(__linux__)
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const ssize_t count = ::getrandom(bytes.data() + offset, bytes.size() - offset, 0);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return false;
        offset += static_cast<std::size_t>(count);
    }
    return true;
#elif defined(__APPLE__)
    ::arc4random_buf(bytes.data(), bytes.size());
    return true;
#else
    return false;
#endif
}

bool create_native_wrapper_staging_root(std::filesystem::path& root) {
    std::error_code error;
    const auto configured_root = std::filesystem::temp_directory_path(error);
    if (error || configured_root.empty()) return false;
    // Canonicalization permits ordinary /tmp aliases while the private-directory
    // API checks the resolved parent's authority and creates one exclusive leaf.
    const auto parent = std::filesystem::canonical(configured_root, error);
    if (error || parent.empty()) return false;
#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
    if (fail_native_wrapper_staging_create_once.exchange(false)) return false;
#endif
    constexpr char hex[] = "0123456789abcdef";
    for (int attempt = 0; attempt < 8; ++attempt) {
        std::array<unsigned char, 16> bytes{};
        if (!fill_staging_random_bytes(bytes)) return false;
        std::string leaf = ".copperfin-native-wrapper-";
        for (const auto byte : bytes) {
            leaf.push_back(hex[byte >> 4U]);
            leaf.push_back(hex[byte & 0x0fU]);
        }
        const auto candidate = parent / leaf;
        root = candidate;
        const auto created = platform::create_private_directory(candidate);
        if (created.ok) {
            return true;
        }
        root.clear();
        if (created.failure != platform::PrivateDirectoryFailure::already_exists) return false;
    }
    return false;
}

struct StagingRootCleanup {
    std::filesystem::path path;
    ~StagingRootCleanup() {
        if (!path.empty()) {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    }
};

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
    std::filesystem::path staging_root;
    bool staging_created = false;
    try {
        staging_created = create_native_wrapper_staging_root(staging_root);
    } catch (const std::exception&) {
        staging_created = false;
    }
    if (!staging_created) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }
    StagingRootCleanup staging_cleanup{staging_root};
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
    if (ignored) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.CreateNativeWrapperBuildDirectoryFailed")};
    }
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
    // Keep the prior primary output until publication.

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
        return {.ok = false, .error = error};
    }

    if (!path_exists_without_error(staged_output_path)) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    std::error_code copy_error;
    std::filesystem::copy_file(
        staged_output_path,
        copperfin::platform::path_from_utf8_string(plan.launcher_output_path),
        std::filesystem::copy_options::overwrite_existing,
        copy_error);
    if (copy_error) {
        return {.ok = false, .error = runtime_text("Runtime.Package.Error.NativeWrapperPrimaryOutputMissing")};
    }

    return finalize_runtime_package_primary_output(
        built_plan,
        security_profile,
        extensibility_profile);
}

#if defined(COPPERFIN_ENABLE_RUNTIME_PIPELINE_TEST_HOOKS)
namespace test_hooks {
void force_native_wrapper_staging_create_failure_once() {
    fail_native_wrapper_staging_create_once.store(true);
}
}  // namespace test_hooks
#endif

}  // namespace copperfin::runtime
