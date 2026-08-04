// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/visual_asset_editor.h"
#include "test_environment_support.h"
#include "test_locale_catalog_environment_support.h"
#include "test_studio_host_real_sample_support.h"

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
using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;
using copperfin::test_support::getenv_value;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

void expect_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) != std::string::npos, message);
}

void expect_not_contains(const std::string& text, const std::string& needle, const std::string& message) {
    expect(text.find(needle) == std::string::npos, message);
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
    const int raw_exit_code = copperfin::test_support::run_shell_command(command);
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

std::string selected_settings_segment(const std::string& json_text) {
    const std::string start_marker = "\"selectedReportSettings\": [";
    const std::string end_marker = "\n    \"selectedReportSelectionKind\":";
    const std::size_t start = json_text.find(start_marker);
    if (start == std::string::npos) {
        return {};
    }

    const std::size_t end = json_text.find(end_marker, start);
    if (end == std::string::npos) {
        return json_text.substr(start);
    }

    return json_text.substr(start, end - start);
}

struct RealSamplePair {
    std::filesystem::path primary;
    std::filesystem::path sidecar;
    std::string title;
    bool is_label = false;
};

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

    const auto original_gridv = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDV"
    });
    expect(original_gridv.ok && original_gridv.exists && !original_gridv.value.empty(),
           "#4253: real sample no-op property write should find the existing GRIDV value");
    if (original_gridv.ok && original_gridv.exists && !original_gridv.value.empty()) {
        const auto no_op_write_process = run_process_capture(
            studio_host_path,
            {
                "--path", copied_primary.string(),
                "--set-property",
                "--record", "0",
                "--property-name", "GRIDV",
                "--property-value", original_gridv.value,
                "--json"
            },
            temp_root);
        if (no_op_write_process.exit_code != 0) {
            std::cerr << "studio host sample no-op GRIDV write stdout:\n"
                      << no_op_write_process.stdout_text << "\n";
            std::cerr << "studio host sample no-op GRIDV write stderr:\n"
                      << no_op_write_process.stderr_text << "\n";
        }
        expect(no_op_write_process.exit_code == 0,
               "#4253: real sample no-op GRIDV write should succeed");
        expect_contains(no_op_write_process.stdout_text,
                        "\"gridVertical\": " + original_gridv.value,
                        "#4253: real sample no-op GRIDV write should return the existing value");
        expect_contains(no_op_write_process.stdout_text,
                        "\"documentTitle\": \"" + sample.title + "\"",
                        "#4253: real sample no-op GRIDV write should preserve document identity");
        expect_contains(
            no_op_write_process.stdout_text,
            sample.is_label ? "\"isLabel\": true" : "\"isLabel\": false",
            "#4253: real sample no-op GRIDV write should preserve report/label identity");
        expect_contains(no_op_write_process.stdout_text,
                        "\"selectedReportSelectionKind\": \"settings\"",
                        "#4253: real sample no-op GRIDV write should preserve settings selection kind");
        const std::string no_op_selected_settings =
            selected_settings_segment(no_op_write_process.stdout_text);
        expect_contains(no_op_selected_settings,
                        "\"name\": \"GRIDV\"",
                        "#4253: real sample no-op GRIDV write should preserve selected-property provenance");
        expect_contains(no_op_selected_settings,
                        "\"value\": \"" + original_gridv.value + "\"",
                        "#4253: real sample no-op GRIDV write should preserve selected-property value");
        expect(read_binary(copied_primary) == original_primary_bytes,
               "#4253: real sample no-op GRIDV write should preserve primary asset bytes");
        expect(read_binary(copied_sidecar) == original_sidecar_bytes,
               "#4253: real sample no-op GRIDV write should preserve sidecar bytes");
    }

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
    expect_contains(reopen_process.stdout_text,
                    "\"documentTitleFieldIndex\": 5",
                    "#4248: real sample document titles should preserve the header NAME field provenance");
    expect_contains(reopen_process.stdout_text,
                    "\"documentTitleMemoBlockNumber\": 0",
                    "#4248: real sample document titles should preserve the header NAME memo provenance");
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

    const std::string post_gridv_primary_bytes = read_binary(copied_primary);
    const auto original_gridh_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDH"
    });
    expect(original_gridh_property.ok && original_gridh_property.exists && original_gridh_property.value == "12",
           "#3711: mounted real samples should start with GRIDH set to 12");
    const std::string gridh_target_value = "13";
    const auto gridh_update_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "GRIDH",
            "--property-value", gridh_target_value,
            "--json"
        },
        temp_root);
    if (gridh_update_process.exit_code != 0) {
        std::cerr << "studio host sample GRIDH update stdout:\n" << gridh_update_process.stdout_text << "\n";
        std::cerr << "studio host sample GRIDH update stderr:\n" << gridh_update_process.stderr_text << "\n";
    }
    expect(gridh_update_process.exit_code == 0, "#3711: real sample GRIDH update should succeed");

    const auto gridh_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDH"
    });
    expect(gridh_property.ok && gridh_property.exists && gridh_property.value == gridh_target_value,
           "#3711: real sample GRIDH update should persist the direct field");

    expect(read_binary(copied_primary) != post_gridv_primary_bytes,
           "#3711: real sample GRIDH update should change the primary asset bytes again");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3711: real sample GRIDH update should preserve sidecar bytes");

    const auto reopen_gridh_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_gridh_process.exit_code != 0) {
        std::cerr << "studio host sample reopen after GRIDH stdout:\n" << reopen_gridh_process.stdout_text << "\n";
        std::cerr << "studio host sample reopen after GRIDH stderr:\n" << reopen_gridh_process.stderr_text << "\n";
    }
    expect(reopen_gridh_process.exit_code == 0, "#3711: real sample reopen after GRIDH update should succeed");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"documentTitle\": \"" + sample.title + "\"",
                    "#3711: real sample reopen after GRIDH should preserve document title");
    if (sample.is_label) {
        expect_contains(reopen_gridh_process.stdout_text,
                        "\"isLabel\": true",
                        "#3711: label sample reopen after GRIDH should preserve label identity");
    } else {
        expect_contains(reopen_gridh_process.stdout_text,
                        "\"isLabel\": false",
                        "#3711: report sample reopen after GRIDH should preserve report identity");
    }
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"selectedReportSettingsAvailable\": true",
                    "#3711: real sample reopen after GRIDH should preserve selected-settings availability");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    "#3711: real sample reopen after GRIDH should preserve settings selection kind");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"pageSetupAvailable\": true",
                    "#3711: real sample reopen after GRIDH should preserve page-setup availability");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"gridHorizontalAvailable\": true",
                    "#3711: real sample reopen should preserve grid-horizontal availability");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"gridHorizontal\": " + gridh_target_value,
                    "#3711: real sample reopen should expose the updated GRIDH value");
    expect_contains(reopen_gridh_process.stdout_text,
                    "\"gridVertical\": 16",
                    "#3711: real sample reopen after GRIDH should preserve the earlier GRIDV update");

    const std::string after_gridh_primary_bytes = read_binary(copied_primary);

    const auto clear_gridv_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "GRIDV",
            "--json"
        },
        temp_root);
    if (clear_gridv_process.exit_code != 0) {
        std::cerr << "studio host sample GRIDV clear stdout:\n" << clear_gridv_process.stdout_text << "\n";
        std::cerr << "studio host sample GRIDV clear stderr:\n" << clear_gridv_process.stderr_text << "\n";
    }
    expect(clear_gridv_process.exit_code == 0, "#3797: real sample GRIDV clear should succeed");

    const auto cleared_gridv_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDV"
    });
    expect(cleared_gridv_property.ok &&
               cleared_gridv_property.value.empty(),
           "#3797: real sample GRIDV clear should clear the persisted setting state");
    expect(read_binary(copied_primary) != after_gridh_primary_bytes,
           "#3797: real sample GRIDV clear should change the primary asset bytes again");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3797: real sample GRIDV clear should preserve sidecar bytes");

    const auto reopen_after_gridv_clear = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_gridv_clear.exit_code != 0) {
        std::cerr << "studio host sample reopen after GRIDV clear stdout:\n"
                  << reopen_after_gridv_clear.stdout_text << "\n";
        std::cerr << "studio host sample reopen after GRIDV clear stderr:\n"
                  << reopen_after_gridv_clear.stderr_text << "\n";
    }
    expect(reopen_after_gridv_clear.exit_code == 0, "#3797: real sample reopen after GRIDV clear should succeed");
    expect_contains(reopen_after_gridv_clear.stdout_text,
                    "\"gridVerticalAvailable\": false",
                    "#3797: real sample GRIDV clear should remove grid-vertical availability");
    expect_contains(reopen_after_gridv_clear.stdout_text,
                    "\"gridVertical\": 0",
                    "#3797: real sample GRIDV clear should clear the grid-vertical value");
    expect_contains(reopen_after_gridv_clear.stdout_text,
                    "\"gridHorizontalAvailable\": true",
                    "#3797: real sample GRIDV clear should preserve grid-horizontal availability");
    expect_contains(reopen_after_gridv_clear.stdout_text,
                    "\"gridHorizontal\": " + gridh_target_value,
                    "#3797: real sample GRIDV clear should preserve the updated GRIDH value");
    const std::string gridv_cleared_settings = selected_settings_segment(reopen_after_gridv_clear.stdout_text);
    expect(!gridv_cleared_settings.empty(),
           "#3797: real sample GRIDV clear should expose a selected-settings JSON block");
    if (!gridv_cleared_settings.empty()) {
        expect_not_contains(gridv_cleared_settings,
                            "\"name\": \"GRIDV\"",
                            "#3797: real sample GRIDV clear should remove GRIDV from selected settings");
    }

    const std::string after_gridv_clear_primary_bytes = read_binary(copied_primary);

    const auto clear_gridh_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "GRIDH",
            "--json"
        },
        temp_root);
    if (clear_gridh_process.exit_code != 0) {
        std::cerr << "studio host sample GRIDH clear stdout:\n" << clear_gridh_process.stdout_text << "\n";
        std::cerr << "studio host sample GRIDH clear stderr:\n" << clear_gridh_process.stderr_text << "\n";
    }
    expect(clear_gridh_process.exit_code == 0, "#3797: real sample GRIDH clear should succeed");

    const auto cleared_gridh_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "GRIDH"
    });
    expect(cleared_gridh_property.ok &&
               cleared_gridh_property.value.empty(),
           "#3797: real sample GRIDH clear should clear the persisted setting state");
    expect(read_binary(copied_primary) != after_gridv_clear_primary_bytes,
           "#3797: real sample GRIDH clear should change the primary asset bytes again");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3797: real sample GRIDH clear should preserve sidecar bytes");

    const auto reopen_after_gridh_clear = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_gridh_clear.exit_code != 0) {
        std::cerr << "studio host sample reopen after GRIDH clear stdout:\n"
                  << reopen_after_gridh_clear.stdout_text << "\n";
        std::cerr << "studio host sample reopen after GRIDH clear stderr:\n"
                  << reopen_after_gridh_clear.stderr_text << "\n";
    }
    expect(reopen_after_gridh_clear.exit_code == 0, "#3797: real sample reopen after GRIDH clear should succeed");
    expect_contains(reopen_after_gridh_clear.stdout_text,
                    "\"gridVerticalAvailable\": false",
                    "#3797: real sample GRIDH clear should preserve cleared grid-vertical availability");
    expect_contains(reopen_after_gridh_clear.stdout_text,
                    "\"gridHorizontalAvailable\": false",
                    "#3797: real sample GRIDH clear should remove grid-horizontal availability");
    expect_contains(reopen_after_gridh_clear.stdout_text,
                    "\"gridHorizontal\": 0",
                    "#3797: real sample GRIDH clear should clear the grid-horizontal value");
    const std::string cleared_settings = selected_settings_segment(reopen_after_gridh_clear.stdout_text);
    expect(!cleared_settings.empty(),
           "#3797: real sample GRIDH clear should expose a selected-settings JSON block");
    if (!cleared_settings.empty()) {
        expect_not_contains(cleared_settings,
                            "\"name\": \"GRIDV\"",
                            "#3797: real sample GRIDH clear should keep GRIDV removed from selected settings");
        expect_not_contains(cleared_settings,
                            "\"name\": \"GRIDH\"",
                            "#3797: real sample GRIDH clear should remove GRIDH from selected settings");
    }
}

void test_real_vfp9_report_and_label_samples_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
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
