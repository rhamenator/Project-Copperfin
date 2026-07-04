// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {

int failures = 0;
using copperfin::test_support::ScopedEnvironmentValue;
using copperfin::test_support::getenv_value;
using copperfin::test_support::set_env_value;

struct ScopedDefaultLocaleCatalogEnvironment {
    ScopedEnvironmentValue locale;
    ScopedEnvironmentValue locale_dir;

    ScopedDefaultLocaleCatalogEnvironment()
        : locale("COPPERFIN_LOCALE"),
          locale_dir("COPPERFIN_LOCALE_DIR") {
        set_env_value("COPPERFIN_LOCALE", "en-US", true);
        set_env_value(
            "COPPERFIN_LOCALE_DIR",
            [] {
                std::filesystem::path ancestor = std::filesystem::absolute(std::filesystem::current_path());
                for (;;) {
                    const auto candidate = ancestor / "resources" / "locales";
                    if (std::filesystem::exists(candidate)) {
                        return candidate.lexically_normal().string();
                    }
                    const auto parent = ancestor.parent_path();
                    if (parent == ancestor) {
                        return candidate.lexically_normal().string();
                    }
                    ancestor = parent;
                }
            }(),
            true);
    }
};

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

std::string quote_command_argument(const std::string& value) {
    std::string quoted = "\"";
    quoted.reserve(value.size() + 2U);
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted.push_back(ch);
        }
    }
    quoted.push_back('"');
    return quoted;
}

std::string read_binary(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

struct ProcessResult {
    int exit_code = -1;
    std::string stdout_text;
    std::string stderr_text;
};

ProcessResult run_process_capture(
    const std::string& executable_path,
    const std::vector<std::string>& arguments,
    const std::filesystem::path& working_directory) {
    namespace fs = std::filesystem;

    const fs::path resolved_executable_path = fs::absolute(executable_path);
    const fs::path stdout_path = working_directory / "studio_host_stdout.log";
    const fs::path stderr_path = working_directory / "studio_host_stderr.log";

    std::string command = quote_command_argument(resolved_executable_path.string());
    for (const auto& argument : arguments) {
        command += " ";
        command += quote_command_argument(argument);
    }
    command += " > ";
    command += quote_command_argument(stdout_path.string());
    command += " 2> ";
    command += quote_command_argument(stderr_path.string());

    const fs::path original_directory = fs::current_path();
    fs::current_path(working_directory);
    const int raw_exit_code = std::system(command.c_str());
    fs::current_path(original_directory);

    ProcessResult result;
    if (fs::exists(stdout_path)) {
        result.stdout_text = read_binary(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_binary(stderr_path);
    }

#if defined(_WIN32)
    result.exit_code = raw_exit_code;
#else
    if (raw_exit_code != -1 && WIFEXITED(raw_exit_code)) {
        result.exit_code = WEXITSTATUS(raw_exit_code);
    } else {
        result.exit_code = raw_exit_code;
    }
#endif
    return result;
}

struct RealSamplePair {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::string title;
    bool is_label = false;
};

std::filesystem::path find_vfp9_reports_root() {
    namespace fs = std::filesystem;

    if (const std::string override_root = getenv_value("COPPERFIN_VFP9_REPORTS_ROOT");
        !override_root.empty()) {
        const fs::path candidate = fs::path(override_root);
        if (fs::exists(candidate / "invoice.frx") && fs::exists(candidate / "cust.lbx")) {
            return candidate;
        }
    }

    const fs::path windows_candidate =
        R"(C:\Program Files (x86)\Microsoft Visual FoxPro 9\Samples\Solution\Reports)";
    if (fs::exists(windows_candidate / "invoice.frx") && fs::exists(windows_candidate / "cust.lbx")) {
        return windows_candidate;
    }

    const std::vector<fs::path> media_roots{
        "/run/media",
        "/media"
    };
    for (const auto& media_root : media_roots) {
        std::error_code error;
        if (!fs::exists(media_root, error)) {
            continue;
        }
        for (const auto& user_entry : fs::directory_iterator(media_root, error)) {
            if (error) {
                break;
            }
            const fs::path candidate =
                user_entry.path() / "VFPPROD1" / "program files" / "microsoft visual foxpro 9" /
                "samples" / "solution" / "reports";
            if (fs::exists(candidate / "invoice.frx") && fs::exists(candidate / "cust.lbx")) {
                return candidate;
            }
        }
    }

    return {};
}

bool make_writable(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_write,
        std::filesystem::perm_options::add,
        error);
    return !error;
}

void exercise_real_sample_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSamplePair& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3526: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3526: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3526: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3526: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto no_op_process = run_process_capture(
        studio_host_path,
        {"--json", copied_primary.string()},
        temp_root);
    if (no_op_process.exit_code != 0) {
        std::cerr << "studio host no-op sample read stdout:\n" << no_op_process.stdout_text << "\n";
        std::cerr << "studio host no-op sample read stderr:\n" << no_op_process.stderr_text << "\n";
    }
    expect(no_op_process.exit_code == 0, "#3526: real sample no-op read should succeed");
    expect(read_binary(copied_primary) == original_primary_bytes,
           "#3526: real sample no-op read should preserve primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3526: real sample no-op read should preserve sidecar bytes");

    const auto update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "GRIDV",
            "--property-value", "16",
            "--json"
        },
        temp_root);
    if (update_process.exit_code != 0) {
        std::cerr << "studio host sample GRIDV update stdout:\n" << update_process.stdout_text << "\n";
        std::cerr << "studio host sample GRIDV update stderr:\n" << update_process.stderr_text << "\n";
    }
    expect(update_process.exit_code == 0, "#3526: real sample GRIDV update should succeed");

    const auto grid_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDV"
    });
    expect(grid_property.ok && grid_property.exists && grid_property.value == "16",
           "#3526: real sample GRIDV update should persist the direct field");

    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3526: real sample GRIDV update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3526: real sample GRIDV update should preserve sidecar bytes");

    const auto reopen_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_process.exit_code != 0) {
        std::cerr << "studio host sample reopen stdout:\n" << reopen_process.stdout_text << "\n";
        std::cerr << "studio host sample reopen stderr:\n" << reopen_process.stderr_text << "\n";
    }
    expect(reopen_process.exit_code == 0, "#3526: real sample reopen after GRIDV update should succeed");
    expect_contains(reopen_process.stdout_text,
                    "\"documentTitle\": \"" + sample.title + "\"",
                    "#3526: real sample reopen should preserve document title");
    if (sample.is_label) {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": true",
                        "#3526: label sample reopen should preserve label identity");
    } else {
        expect_contains(reopen_process.stdout_text,
                        "\"isLabel\": false",
                        "#3526: report sample reopen should preserve report identity");
    }
    expect_contains(reopen_process.stdout_text,
                    "\"selectedReportSettingsAvailable\": true",
                    "#3526: real sample reopen should preserve selected-settings availability");
    expect_contains(reopen_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    "#3526: real sample reopen should preserve settings selection kind");
    expect_contains(reopen_process.stdout_text,
                    "\"pageSetupAvailable\": true",
                    "#3526: real sample reopen should preserve page-setup availability");
    expect_contains(reopen_process.stdout_text,
                    "\"orientationCode\": 0",
                    "#3526: real sample reopen should preserve orientation");
    expect_contains(reopen_process.stdout_text,
                    "\"paperSizeCode\": 1",
                    "#3526: real sample reopen should preserve paper size");
    expect_contains(reopen_process.stdout_text,
                    "\"gridVerticalAvailable\": true",
                    "#3526: real sample reopen should preserve grid-vertical availability");
    expect_contains(reopen_process.stdout_text,
                    "\"gridVertical\": 16",
                    "#3526: real sample reopen should expose the updated GRIDV value");
}

void test_real_vfp9_report_and_label_samples_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3526 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_sample_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .title = "invoice.frx",
            .is_label = false
        });
    exercise_real_sample_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .title = "cust.lbx",
            .is_label = true
        });

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_samples_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
