// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

void expect_common_reopen_json(const std::string& json_text, const RealSamplePair& sample) {
    expect_contains(json_text,
                    "\"documentTitle\": \"" + sample.title + "\"",
                    "#3713: real-sample column setup round trip should preserve document title");
    if (sample.is_label) {
        expect_contains(json_text,
                        "\"isLabel\": true",
                        "#3713: label column setup round trip should preserve label identity");
    } else {
        expect_contains(json_text,
                        "\"isLabel\": false",
                        "#3713: report column setup round trip should preserve report identity");
    }
    expect_contains(json_text,
                    "\"selectedReportSettingsAvailable\": true",
                    "#3713: real-sample column setup round trip should preserve selected-settings availability");
    expect_contains(json_text,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    "#3713: real-sample column setup round trip should preserve settings selection kind");
    expect_contains(json_text,
                    "\"pageSetupAvailable\": true",
                    "#3713: real-sample column setup round trip should preserve page setup availability");
    expect_contains(json_text,
                    "\"orientationCode\": 0",
                    "#3713: real-sample column setup round trip should preserve orientation");
    expect_contains(json_text,
                    "\"paperSizeCode\": 1",
                    "#3713: real-sample column setup round trip should preserve paper size");
    expect_contains(json_text,
                    "\"gridVerticalAvailable\": true",
                    "#3713: real-sample column setup round trip should preserve grid-vertical availability");
    expect_contains(json_text,
                    "\"gridVertical\": 12",
                    "#3713: real-sample column setup round trip should preserve grid-vertical value");
}

void exercise_real_sample_column_setup_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSamplePair& sample) {
    namespace fs = std::filesystem;

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3713: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3713: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3713: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3713: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto set_cols_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "COLS",
            "--property-value", "2",
            "--json"
        },
        temp_root);
    if (set_cols_process.exit_code != 0) {
        std::cerr << "studio host sample COLS update stdout:\n" << set_cols_process.stdout_text << "\n";
        std::cerr << "studio host sample COLS update stderr:\n" << set_cols_process.stderr_text << "\n";
    }
    expect(set_cols_process.exit_code == 0, "#3713: real sample COLS update should succeed");

    const auto cols_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "COLS"
    });
    expect(cols_property.ok && cols_property.exists && cols_property.value == "2",
           "#3713: real sample COLS update should persist the memo-backed setting");

    const std::string after_cols_primary_bytes = read_binary(copied_primary);
    const std::string after_cols_sidecar_bytes = read_binary(copied_sidecar);
    expect(after_cols_primary_bytes != original_primary_bytes,
           "#3713: real sample COLS update should change the primary asset bytes");
    expect(after_cols_sidecar_bytes != original_sidecar_bytes,
           "#3713: real sample COLS update should change the memo sidecar bytes");

    const auto reopen_after_cols = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_cols.exit_code != 0) {
        std::cerr << "studio host sample COLS reopen stdout:\n" << reopen_after_cols.stdout_text << "\n";
        std::cerr << "studio host sample COLS reopen stderr:\n" << reopen_after_cols.stderr_text << "\n";
    }
    expect(reopen_after_cols.exit_code == 0, "#3713: real sample reopen after COLS update should succeed");
    expect_common_reopen_json(reopen_after_cols.stdout_text, sample);
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnCountAvailable\": true",
                    "#3713: real sample COLS update should expose column-count availability");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnCount\": 2",
                    "#3713: real sample COLS update should expose the updated column count");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnSpacingAvailable\": false",
                    "#3713: real sample COLS update should keep column spacing unavailable");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnSpacing\": 0",
                    "#3713: real sample COLS update should keep column spacing inert");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnWidthAvailable\": false",
                    "#3720: real sample COLS update should keep column width unavailable");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"columnWidth\": 0",
                    "#3720: real sample COLS update should keep column width inert");
    expect_contains(reopen_after_cols.stdout_text,
                    "\"name\": \"COLS\"",
                    "#3713: real sample COLS update should expose COLS provenance");

    const auto set_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "COLWIDTH",
            "--property-value", "4800",
            "--json"
        },
        temp_root);
    if (set_width_process.exit_code != 0) {
        std::cerr << "studio host sample COLWIDTH update stdout:\n" << set_width_process.stdout_text << "\n";
        std::cerr << "studio host sample COLWIDTH update stderr:\n" << set_width_process.stderr_text << "\n";
    }
    expect(set_width_process.exit_code == 0, "#3720: real sample COLWIDTH update should succeed");

    const auto width_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "COLWIDTH"
    });
    expect(width_property.ok && width_property.exists && width_property.value == "4800",
           "#3720: real sample COLWIDTH update should persist the memo-backed setting");

    const std::string after_width_primary_bytes = read_binary(copied_primary);
    const std::string after_width_sidecar_bytes = read_binary(copied_sidecar);
    expect(after_width_primary_bytes != after_cols_primary_bytes,
           "#3720: real sample COLWIDTH update should change the primary asset bytes again");
    expect(after_width_sidecar_bytes != after_cols_sidecar_bytes,
           "#3720: real sample COLWIDTH update should change the memo sidecar bytes again");

    const auto reopen_after_width = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_width.exit_code != 0) {
        std::cerr << "studio host sample COLWIDTH reopen stdout:\n" << reopen_after_width.stdout_text << "\n";
        std::cerr << "studio host sample COLWIDTH reopen stderr:\n" << reopen_after_width.stderr_text << "\n";
    }
    expect(reopen_after_width.exit_code == 0, "#3720: real sample reopen after COLWIDTH update should succeed");
    expect_common_reopen_json(reopen_after_width.stdout_text, sample);
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnCountAvailable\": true",
                    "#3720: real sample COLWIDTH update should preserve column-count availability");
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnCount\": 2",
                    "#3720: real sample COLWIDTH update should preserve the updated column count");
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnSpacingAvailable\": false",
                    "#3720: real sample COLWIDTH update should keep column spacing unavailable");
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnSpacing\": 0",
                    "#3720: real sample COLWIDTH update should keep column spacing inert");
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnWidthAvailable\": true",
                    "#3720: real sample COLWIDTH update should expose column-width availability");
    expect_contains(reopen_after_width.stdout_text,
                    "\"columnWidth\": 4800",
                    "#3720: real sample COLWIDTH update should expose the updated column width");
    expect_contains(reopen_after_width.stdout_text,
                    "\"name\": \"COLS\"",
                    "#3720: real sample COLWIDTH update should preserve COLS provenance");
    expect_contains(reopen_after_width.stdout_text,
                    "\"name\": \"COLWIDTH\"",
                    "#3720: real sample COLWIDTH update should expose COLWIDTH provenance");

    const auto set_spacing_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "COLSPACING",
            "--property-value", "120",
            "--json"
        },
        temp_root);
    if (set_spacing_process.exit_code != 0) {
        std::cerr << "studio host sample COLSPACING update stdout:\n" << set_spacing_process.stdout_text << "\n";
        std::cerr << "studio host sample COLSPACING update stderr:\n" << set_spacing_process.stderr_text << "\n";
    }
    expect(set_spacing_process.exit_code == 0, "#3713: real sample COLSPACING update should succeed");

    const auto spacing_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "COLSPACING"
    });
    expect(spacing_property.ok && spacing_property.exists && spacing_property.value == "120",
           "#3713: real sample COLSPACING update should persist the memo-backed setting");

    const std::string after_spacing_primary_bytes = read_binary(copied_primary);
    const std::string after_spacing_sidecar_bytes = read_binary(copied_sidecar);
    expect(after_spacing_primary_bytes != after_width_primary_bytes,
           "#3713: real sample COLSPACING update should change the primary asset bytes again");
    expect(after_spacing_sidecar_bytes != after_width_sidecar_bytes,
           "#3713: real sample COLSPACING update should change the memo sidecar bytes again");

    const auto reopen_after_spacing = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_spacing.exit_code != 0) {
        std::cerr << "studio host sample COLSPACING reopen stdout:\n" << reopen_after_spacing.stdout_text << "\n";
        std::cerr << "studio host sample COLSPACING reopen stderr:\n" << reopen_after_spacing.stderr_text << "\n";
    }
    expect(reopen_after_spacing.exit_code == 0,
           "#3713: real sample reopen after COLSPACING update should succeed");
    expect_common_reopen_json(reopen_after_spacing.stdout_text, sample);
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnCountAvailable\": true",
                    "#3713: real sample COLSPACING update should preserve column-count availability");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnCount\": 2",
                    "#3713: real sample COLSPACING update should preserve the updated column count");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnSpacingAvailable\": true",
                    "#3713: real sample COLSPACING update should expose column-spacing availability");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnSpacing\": 120",
                    "#3713: real sample COLSPACING update should expose the updated column spacing");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnWidthAvailable\": true",
                    "#3720: real sample COLSPACING update should preserve column-width availability");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"columnWidth\": 4800",
                    "#3720: real sample COLSPACING update should preserve the updated column width");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"name\": \"COLS\"",
                    "#3713: real sample COLSPACING update should preserve COLS provenance");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"name\": \"COLSPACING\"",
                    "#3713: real sample COLSPACING update should expose COLSPACING provenance");
    expect_contains(reopen_after_spacing.stdout_text,
                    "\"name\": \"COLWIDTH\"",
                    "#3720: real sample COLSPACING update should preserve COLWIDTH provenance");

    const auto clear_spacing_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "COLSPACING",
            "--json"
        },
        temp_root);
    if (clear_spacing_process.exit_code != 0) {
        std::cerr << "studio host sample COLSPACING clear stdout:\n" << clear_spacing_process.stdout_text << "\n";
        std::cerr << "studio host sample COLSPACING clear stderr:\n" << clear_spacing_process.stderr_text << "\n";
    }
    expect(clear_spacing_process.exit_code == 0, "#3713: real sample COLSPACING clear should succeed");

    const auto clear_width_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "COLWIDTH",
            "--json"
        },
        temp_root);
    if (clear_width_process.exit_code != 0) {
        std::cerr << "studio host sample COLWIDTH clear stdout:\n" << clear_width_process.stdout_text << "\n";
        std::cerr << "studio host sample COLWIDTH clear stderr:\n" << clear_width_process.stderr_text << "\n";
    }
    expect(clear_width_process.exit_code == 0, "#3720: real sample COLWIDTH clear should succeed");

    const auto clear_cols_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "COLS",
            "--json"
        },
        temp_root);
    if (clear_cols_process.exit_code != 0) {
        std::cerr << "studio host sample COLS clear stdout:\n" << clear_cols_process.stdout_text << "\n";
        std::cerr << "studio host sample COLS clear stderr:\n" << clear_cols_process.stderr_text << "\n";
    }
    expect(clear_cols_process.exit_code == 0, "#3713: real sample COLS clear should succeed");

    const std::string cleared_primary_bytes = read_binary(copied_primary);
    const std::string cleared_sidecar_bytes = read_binary(copied_sidecar);
    expect(cleared_primary_bytes != after_spacing_primary_bytes,
           "#3713: real sample column setup clear should change the primary asset bytes again");
    expect(cleared_sidecar_bytes != after_spacing_sidecar_bytes,
           "#3713: real sample column setup clear should change the memo sidecar bytes again");
    expect(cleared_primary_bytes != original_primary_bytes,
           "#3713: real sample column setup clear should not rewind the primary bytes to the original sample");
    expect(cleared_sidecar_bytes != original_sidecar_bytes,
           "#3713: real sample column setup clear should not rewind the sidecar bytes to the original sample");

    const auto reopen_after_clear = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_clear.exit_code != 0) {
        std::cerr << "studio host sample column setup clear reopen stdout:\n" << reopen_after_clear.stdout_text
                  << "\n";
        std::cerr << "studio host sample column setup clear reopen stderr:\n" << reopen_after_clear.stderr_text
                  << "\n";
    }
    expect(reopen_after_clear.exit_code == 0,
           "#3713: real sample reopen after column setup clear should succeed");
    expect_common_reopen_json(reopen_after_clear.stdout_text, sample);
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnCountAvailable\": false",
                    "#3713: real sample column setup clear should remove column-count availability");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnCount\": 0",
                    "#3713: real sample column setup clear should clear the column count");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnSpacingAvailable\": false",
                    "#3713: real sample column setup clear should remove column-spacing availability");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnSpacing\": 0",
                    "#3713: real sample column setup clear should clear the column spacing");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnWidthAvailable\": false",
                    "#3720: real sample column setup clear should remove column-width availability");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"columnWidth\": 0",
                    "#3720: real sample column setup clear should clear the column width");
    const std::string selected_settings = selected_settings_segment(reopen_after_clear.stdout_text);
    expect(!selected_settings.empty(),
           "#3713: real sample column setup clear should expose a selected-settings JSON block");
    expect_not_contains(selected_settings,
                        "\"name\": \"COLS\"",
                        "#3713: real sample column setup clear should remove COLS from selected settings");
    expect_not_contains(selected_settings,
                        "\"name\": \"COLSPACING\"",
                        "#3713: real sample column setup clear should remove COLSPACING from selected settings");
    expect_not_contains(selected_settings,
                        "\"name\": \"COLWIDTH\"",
                        "#3720: real sample column setup clear should remove COLWIDTH from selected settings");
}

void test_real_vfp9_report_and_label_column_setup_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3713 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_column_setup_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_column_setup_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .title = "invoice.frx",
            .is_label = false
        });
    exercise_real_sample_column_setup_round_trip(
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
        std::cerr << "usage: test_studio_host_real_sample_column_setup_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_column_setup_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
