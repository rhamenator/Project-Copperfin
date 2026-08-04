// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_environment_support.h"
#include "test_studio_host_real_sample_support.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

}  // namespace

int main() {
    namespace fs = std::filesystem;
    using copperfin::test_support::ScopedEnvironmentValue;

    const fs::path temp_root = fs::temp_directory_path() /
        ("copperfin_vfp9_sample_discovery_" +
         std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::error_code error;
    fs::create_directories(temp_root, error);
    expect(!error, "create sample discovery fixture directory");

    {
        ScopedEnvironmentValue override_root("COPPERFIN_VFP9_REPORTS_ROOT", temp_root.string());
        const fs::path discovered = copperfin::test_support::find_vfp9_reports_root();
        expect(discovered.empty(), "incomplete override is ignored without throwing");

        std::ofstream(temp_root / "invoice.frx").put('\n');
        std::ofstream(temp_root / "cust.lbx").put('\n');
        expect(
            copperfin::test_support::find_vfp9_reports_root().empty(),
            "primary report and label files are incomplete without memo sidecars");

        std::ofstream(temp_root / "invoice.frt").put('\n');
        expect(
            copperfin::test_support::find_vfp9_reports_root().empty(),
            "a report memo sidecar alone does not make the sample complete");

        std::ofstream(temp_root / "cust.lbt").put('\n');
        expect(
            copperfin::test_support::find_vfp9_reports_root() == temp_root,
            "report, label, and both memo sidecars select the complete override");

        fs::remove(temp_root / "invoice.frt", error);
        expect(
            copperfin::test_support::find_vfp9_reports_root().empty(),
            "missing report memo sidecar rejects the sample root");
        std::ofstream(temp_root / "invoice.frt").put('\n');

        fs::remove(temp_root / "cust.lbt", error);
        expect(
            copperfin::test_support::find_vfp9_reports_root().empty(),
            "missing label memo sidecar rejects the sample root");
    }

    const fs::path invalid_override = temp_root / "not-a-directory";
    std::ofstream(invalid_override).put('\n');
    {
        ScopedEnvironmentValue override_root(
            "COPPERFIN_VFP9_REPORTS_ROOT", invalid_override.string());
        const fs::path discovered = copperfin::test_support::find_vfp9_reports_root();
        expect(
            discovered.empty(),
            "regular-file override is authoritative and rejects discovery without throwing");
    }

    fs::remove_all(temp_root, error);
    return failures == 0 ? 0 : 1;
}
