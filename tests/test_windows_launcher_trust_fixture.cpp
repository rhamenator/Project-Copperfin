// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/package/launcher_inventory_trust.h"
#include "copperfin/security/sha256.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace {

constexpr char kDependenciesFixturePayload[] =
    "{\"fixture\":\"dependencies\"}\n";
constexpr char kRuntimeConfigurationFixturePayload[] =
    "{\"fixture\":\"runtime-configuration\"}\n";

#if !defined(_WIN32)
constexpr const char* kMarkerEnvironment = "COPPERFIN_TEST_LAUNCHER_TRUST_MARKER";
#endif

void write_text(const std::filesystem::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!output) {
        throw std::runtime_error("failed to write fixture file: " + path.string());
    }
}

std::string file_digest(const std::filesystem::path& path) {
    const auto digest = copperfin::security::sha256_hex_for_file(path.string());
    if (!digest.ok) {
        throw std::runtime_error("failed to hash fixture file: " + path.string());
    }
    return digest.hex_digest;
}

std::optional<std::filesystem::path> marker_path_from_environment() {
#if defined(_WIN32)
    constexpr wchar_t marker_environment[] =
        L"COPPERFIN_TEST_LAUNCHER_TRUST_MARKER";
    const DWORD required = GetEnvironmentVariableW(marker_environment, nullptr, 0U);
    if (required <= 1U) {
        return std::nullopt;
    }
    std::wstring value(required, L'\0');
    const DWORD written = GetEnvironmentVariableW(
        marker_environment,
        value.data(),
        required);
    if (written == 0U || written >= required) {
        return std::nullopt;
    }
    value.resize(written);
    return std::filesystem::path(value);
#else
    const char* value = std::getenv(kMarkerEnvironment);
    if (value == nullptr || *value == '\0') {
        return std::nullopt;
    }
    return std::filesystem::path(value);
#endif
}

int run_internal_target(const std::filesystem::path& marker) {
    write_text(marker, "managed-apphost-started\n");
    return 0;
}

int prepare_fixture(
    const std::filesystem::path& self,
    const std::filesystem::path& package_root,
    const std::filesystem::path& guard,
    const std::string& signer_key_id) {
    namespace fs = std::filesystem;
    if (!fs::is_regular_file(self) || !fs::is_regular_file(guard) ||
        signer_key_id.empty()) {
        std::cerr << "invalid launcher trust fixture input\n";
        return 2;
    }

    std::error_code error;
    fs::remove_all(package_root, error);
    error.clear();
    fs::create_directories(package_root, error);
    if (error) {
        std::cerr << "failed to create launcher trust fixture root\n";
        return 2;
    }

    const std::string public_name = "Copperfin.TrustValidation.exe";
    const std::string internal_name = "Copperfin.GeneratedLauncher.apphost.exe";
    fs::copy_file(guard, package_root / public_name, fs::copy_options::overwrite_existing, error);
    if (error) {
        std::cerr << "failed to copy enforced launcher guard\n";
        return 2;
    }
    error.clear();
    fs::copy_file(self, package_root / internal_name, fs::copy_options::overwrite_existing, error);
    if (error) {
        std::cerr << "failed to copy internal apphost fixture\n";
        return 2;
    }
    write_text(package_root / "Copperfin.GeneratedLauncher.dll", "launcher dll fixture\n");
    write_text(package_root / "Copperfin.GeneratedLauncher.deps.json",
               kDependenciesFixturePayload);
    write_text(package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json",
               kRuntimeConfigurationFixturePayload);

    std::vector<copperfin::package_trust::LauncherInventoryArtifact> artifacts{
        {"public_apphost", public_name, file_digest(package_root / public_name)},
        {"runtime_required", internal_name, file_digest(package_root / internal_name)},
        {"runtime_required", "Copperfin.GeneratedLauncher.dll",
         file_digest(package_root / "Copperfin.GeneratedLauncher.dll")},
        {"runtime_required", "Copperfin.GeneratedLauncher.deps.json",
         file_digest(package_root / "Copperfin.GeneratedLauncher.deps.json")},
        {"runtime_required", "Copperfin.GeneratedLauncher.runtimeconfig.json",
         file_digest(package_root / "Copperfin.GeneratedLauncher.runtimeconfig.json")}
    };

    std::string manifest = "manifest_version=3\n";
    for (const auto& artifact : artifacts) {
        manifest += "launcher_artifact=" + artifact.package_relative_path + "|" +
            artifact.role + "|" + artifact.sha256 + "\n";
    }
    write_text(package_root / "app.cfmanifest", manifest);

    const std::string envelope =
        copperfin::package_trust::canonical_launcher_inventory_envelope(
            signer_key_id,
            artifacts);
    if (envelope.empty()) {
        std::cerr << "failed to create canonical launcher trust envelope\n";
        return 2;
    }
    write_text(package_root / "app.cftrust", envelope);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    const auto marker = marker_path_from_environment();
    if (marker.has_value()) {
        return run_internal_target(*marker);
    }
    if (argc != 5 || std::string(argv[1]) != "--prepare") {
        std::cerr << "usage: test_windows_launcher_trust_fixture --prepare <package-root> <guard> <signer-key-id>\n";
        return 2;
    }
    try {
        return prepare_fixture(
            std::filesystem::absolute(argv[0]),
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]),
            argv[4]);
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 2;
    }
}
