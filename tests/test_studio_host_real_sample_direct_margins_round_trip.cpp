// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
                    "#3761: real-sample direct-margin round trip should preserve document title");
    if (sample.is_label) {
        expect_contains(json_text,
                        "\"isLabel\": true",
                        "#3761: label direct-margin round trip should preserve label identity");
    } else {
        expect_contains(json_text,
                        "\"isLabel\": false",
                        "#3761: report direct-margin round trip should preserve report identity");
    }
    expect_contains(json_text,
                    "\"selectedReportSettingsAvailable\": true",
                    "#3761: real-sample direct-margin round trip should preserve selected-settings availability");
    expect_contains(json_text,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    "#3761: real-sample direct-margin round trip should preserve settings selection kind");
    expect_contains(json_text,
                    "\"pageSetupAvailable\": true",
                    "#3761: real-sample direct-margin round trip should preserve page setup availability");
    expect_contains(json_text,
                    "\"orientationCode\": 0",
                    "#3761: real-sample direct-margin round trip should preserve orientation");
    expect_contains(json_text,
                    "\"paperSizeCode\": 1",
                    "#3761: real-sample direct-margin round trip should preserve paper size");
    expect_contains(json_text,
                    "\"gridVerticalAvailable\": true",
                    "#3761: real-sample direct-margin round trip should preserve grid-vertical availability");
    expect_contains(json_text,
                    "\"gridVertical\": 12",
                    "#3761: real-sample direct-margin round trip should preserve grid-vertical value");
}

void exercise_real_sample_direct_margins_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSamplePair& sample) {
    namespace fs = std::filesystem;

    constexpr const char* kTopMarginValue = "10";
    constexpr const char* kBottomMarginValue = "20";

    const fs::path copied_primary = temp_root / sample.primary.filename();
    const fs::path copied_sidecar = temp_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3761: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3761: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3761: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3761: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto set_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "TOPMARGIN",
            "--property-value", kTopMarginValue,
            "--json"
        },
        temp_root);
    if (set_top_process.exit_code != 0) {
        std::cerr << "studio host sample TOPMARGIN update stdout:\n" << set_top_process.stdout_text << "\n";
        std::cerr << "studio host sample TOPMARGIN update stderr:\n" << set_top_process.stderr_text << "\n";
    }
    expect(set_top_process.exit_code == 0, "#3761: real sample TOPMARGIN update should succeed");

    const auto top_margin_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "TOPMARGIN"
    });
    expect(top_margin_property.ok && top_margin_property.exists && top_margin_property.value == kTopMarginValue,
           "#3761: real sample TOPMARGIN update should persist the direct field");
    expect(read_binary(copied_primary) != original_primary_bytes,
           "#3761: real sample TOPMARGIN update should change the primary asset bytes");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3761: real sample TOPMARGIN update should preserve sidecar bytes");

    const auto reopen_after_top = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_top.exit_code != 0) {
        std::cerr << "studio host sample TOPMARGIN reopen stdout:\n" << reopen_after_top.stdout_text << "\n";
        std::cerr << "studio host sample TOPMARGIN reopen stderr:\n" << reopen_after_top.stderr_text << "\n";
    }
    expect(reopen_after_top.exit_code == 0, "#3761: real sample reopen after TOPMARGIN update should succeed");
    expect_common_reopen_json(reopen_after_top.stdout_text, sample);
    expect_contains(reopen_after_top.stdout_text,
                    "\"topMarginAvailable\": true",
                    "#3761: real sample TOPMARGIN update should expose top-margin availability");
    expect_contains(reopen_after_top.stdout_text,
                    "\"topMargin\": 10",
                    "#3761: real sample TOPMARGIN update should expose the updated top margin");
    expect_contains(reopen_after_top.stdout_text,
                    "\"bottomMarginAvailable\": false",
                    "#3761: real sample TOPMARGIN update should keep bottom-margin unavailable");
    expect_contains(reopen_after_top.stdout_text,
                    "\"bottomMargin\": 0",
                    "#3761: real sample TOPMARGIN update should keep bottom-margin inert");
    expect_contains(reopen_after_top.stdout_text,
                    "\"settingCount\":",
                    "#3761: real sample TOPMARGIN update should expose a live root setting count");
    expect_contains(reopen_after_top.stdout_text,
                    "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 62, \"sourceLineIndex\": null",
                    "#3761: real sample TOPMARGIN update should expose direct TOPMARGIN provenance");

    const std::string after_top_primary_bytes = read_binary(copied_primary);

    const auto set_bottom_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", "BOTMARGIN",
            "--property-value", kBottomMarginValue,
            "--json"
        },
        temp_root);
    if (set_bottom_process.exit_code != 0) {
        std::cerr << "studio host sample BOTMARGIN update stdout:\n" << set_bottom_process.stdout_text << "\n";
        std::cerr << "studio host sample BOTMARGIN update stderr:\n" << set_bottom_process.stderr_text << "\n";
    }
    expect(set_bottom_process.exit_code == 0, "#3761: real sample BOTMARGIN update should succeed");

    const auto bottom_margin_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "BOTMARGIN"
    });
    expect(bottom_margin_property.ok && bottom_margin_property.exists &&
               bottom_margin_property.value == kBottomMarginValue,
           "#3761: real sample BOTMARGIN update should persist the direct field");
    expect(read_binary(copied_primary) != after_top_primary_bytes,
           "#3761: real sample BOTMARGIN update should change the primary asset bytes again");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3761: real sample BOTMARGIN update should preserve sidecar bytes");

    const auto reopen_after_bottom = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_bottom.exit_code != 0) {
        std::cerr << "studio host sample BOTMARGIN reopen stdout:\n" << reopen_after_bottom.stdout_text << "\n";
        std::cerr << "studio host sample BOTMARGIN reopen stderr:\n" << reopen_after_bottom.stderr_text << "\n";
    }
    expect(reopen_after_bottom.exit_code == 0, "#3761: real sample reopen after BOTMARGIN update should succeed");
    expect_common_reopen_json(reopen_after_bottom.stdout_text, sample);
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"topMarginAvailable\": true",
                    "#3761: real sample BOTMARGIN update should preserve top-margin availability");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"topMargin\": 10",
                    "#3761: real sample BOTMARGIN update should preserve the updated top margin");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"bottomMarginAvailable\": true",
                    "#3761: real sample BOTMARGIN update should expose bottom-margin availability");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"bottomMargin\": 20",
                    "#3761: real sample BOTMARGIN update should expose the updated bottom margin");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"settingCount\":",
                    "#3761: real sample BOTMARGIN update should expose a live root setting count");
    expect(copperfin::test_support::json_integer_delta(
               reopen_after_bottom.stdout_text,
               reopen_after_top.stdout_text,
               "\"settingCount\"",
               1),
           "#3761: real sample BOTMARGIN update should add one setting after TOPMARGIN");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 62, \"sourceLineIndex\": null",
                    "#3761: real sample BOTMARGIN update should preserve TOPMARGIN provenance");
    expect_contains(reopen_after_bottom.stdout_text,
                    "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 63, \"sourceLineIndex\": null",
                    "#3761: real sample BOTMARGIN update should expose BOTMARGIN provenance");

    const std::string after_bottom_primary_bytes = read_binary(copied_primary);

    const auto clear_bottom_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "BOTMARGIN",
            "--json"
        },
        temp_root);
    if (clear_bottom_process.exit_code != 0) {
        std::cerr << "studio host sample BOTMARGIN clear stdout:\n" << clear_bottom_process.stdout_text << "\n";
        std::cerr << "studio host sample BOTMARGIN clear stderr:\n" << clear_bottom_process.stderr_text << "\n";
    }
    expect(clear_bottom_process.exit_code == 0, "#3761: real sample BOTMARGIN clear should succeed");

    const auto cleared_bottom_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "BOTMARGIN"
    });
    expect(cleared_bottom_property.ok && cleared_bottom_property.exists && cleared_bottom_property.value.empty(),
           "#3761: real sample BOTMARGIN clear should blank the direct field");
    expect(read_binary(copied_primary) != after_bottom_primary_bytes,
           "#3761: real sample BOTMARGIN clear should change the primary asset bytes again");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3761: real sample BOTMARGIN clear should preserve sidecar bytes");

    const auto clear_top_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--clear-property",
            "--record", "0",
            "--property-name", "TOPMARGIN",
            "--json"
        },
        temp_root);
    if (clear_top_process.exit_code != 0) {
        std::cerr << "studio host sample TOPMARGIN clear stdout:\n" << clear_top_process.stdout_text << "\n";
        std::cerr << "studio host sample TOPMARGIN clear stderr:\n" << clear_top_process.stderr_text << "\n";
    }
    expect(clear_top_process.exit_code == 0, "#3761: real sample TOPMARGIN clear should succeed");

    const auto cleared_top_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "TOPMARGIN"
    });
    expect(cleared_top_property.ok && cleared_top_property.exists && cleared_top_property.value.empty(),
           "#3761: real sample TOPMARGIN clear should blank the direct field");
    expect(read_binary(copied_sidecar) == original_sidecar_bytes,
           "#3761: real sample TOPMARGIN clear should preserve sidecar bytes");

    const auto reopen_after_clear = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        temp_root);
    if (reopen_after_clear.exit_code != 0) {
        std::cerr << "studio host sample margin clear reopen stdout:\n" << reopen_after_clear.stdout_text << "\n";
        std::cerr << "studio host sample margin clear reopen stderr:\n" << reopen_after_clear.stderr_text << "\n";
    }
    expect(reopen_after_clear.exit_code == 0, "#3761: real sample reopen after direct-margin clear should succeed");
    expect_common_reopen_json(reopen_after_clear.stdout_text, sample);
    expect_contains(reopen_after_clear.stdout_text,
                    "\"topMarginAvailable\": false",
                    "#3761: real sample direct-margin clear should remove top-margin availability");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"topMargin\": 0",
                    "#3761: real sample direct-margin clear should clear the top margin");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"bottomMarginAvailable\": false",
                    "#3761: real sample direct-margin clear should remove bottom-margin availability");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"bottomMargin\": 0",
                    "#3761: real sample direct-margin clear should clear the bottom margin");
    expect_contains(reopen_after_clear.stdout_text,
                    "\"settingCount\":",
                    "#3761: real sample direct-margin clear should preserve a live root setting count");
    expect(copperfin::test_support::json_integer_delta(
               reopen_after_top.stdout_text,
               reopen_after_clear.stdout_text,
               "\"settingCount\"",
               1),
           "#3761: real sample direct-margin clear should restore the count after removing two settings");
    const std::string selected_settings = selected_settings_segment(reopen_after_clear.stdout_text);
    expect(!selected_settings.empty(),
           "#3761: real sample direct-margin clear should expose a selected-settings JSON block");
    if (!selected_settings.empty()) {
        expect_not_contains(selected_settings,
                            "\"name\": \"TOPMARGIN\"",
                            "#3761: real sample direct-margin clear should remove TOPMARGIN from selected settings");
        expect_not_contains(selected_settings,
                            "\"name\": \"BOTMARGIN\"",
                            "#3761: real sample direct-margin clear should remove BOTMARGIN from selected settings");
    }
}

void test_real_vfp9_report_and_label_direct_margins_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3761 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_direct_margins_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    exercise_real_sample_direct_margins_round_trip(
        studio_host_path,
        temp_root,
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .title = "invoice.frx",
            .is_label = false
        });
    exercise_real_sample_direct_margins_round_trip(
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
        std::cerr << "usage: test_studio_host_real_sample_direct_margins_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_direct_margins_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
