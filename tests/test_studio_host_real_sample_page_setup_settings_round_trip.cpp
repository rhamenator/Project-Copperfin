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

struct RealSampleSettingCase {
    std::string property_name;
    std::string original_value;
    std::string updated_value;
    std::string updated_provenance_prefix;
    std::vector<std::string> updated_json_needles;
    std::vector<std::string> restored_json_needles;
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

void expect_common_reopen_json(
    const std::string& json_text,
    const RealSamplePair& sample,
    bool expect_undo,
    const std::string& issue_prefix,
    const std::string& context) {
    expect_contains(json_text,
                    "\"documentTitle\": \"" + sample.title + "\"",
                    issue_prefix + ": " + context + " should preserve document title");
    if (sample.is_label) {
        expect_contains(json_text,
                        "\"isLabel\": true",
                        issue_prefix + ": " + context + " should preserve label identity");
    } else {
        expect_contains(json_text,
                        "\"isLabel\": false",
                        issue_prefix + ": " + context + " should preserve report identity");
    }
    expect_contains(json_text,
                    "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + ": " + context + " should preserve selected-settings availability");
    expect_contains(json_text,
                    "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + ": " + context + " should preserve settings selection kind");
    expect_contains(json_text,
                    "\"pageSetupAvailable\": true",
                    issue_prefix + ": " + context + " should preserve page setup availability");
    expect_contains(json_text,
                    "\"gridVerticalAvailable\": true",
                    issue_prefix + ": " + context + " should preserve grid-vertical availability");
    expect_contains(json_text,
                    "\"gridVertical\": 12",
                    issue_prefix + ": " + context + " should preserve grid-vertical value");
    expect_contains(json_text,
                    "\"settingCount\":",
                    issue_prefix + ": " + context + " should expose a live setting count");
    expect_contains(json_text,
                    "\"deletedSettingCount\": 0",
                    issue_prefix + ": " + context + " should preserve deleted setting count");
    expect_contains(
        json_text,
        expect_undo ? "\"commandUndoAvailable\": true" : "\"commandUndoAvailable\": false",
        issue_prefix + ": " + context + " should preserve command undo availability");
    if (!expect_undo) {
        expect_contains(json_text,
                        "\"commandUndoLabel\": \"\"",
                        issue_prefix + ": " + context + " should clear the command undo label");
    }
}

void expect_selected_setting_value(
    const std::string& json_text,
    const RealSampleSettingCase& setting_case,
    const std::string& expected_value,
    const std::string& issue_prefix,
    const std::string& context) {
    expect_contains(json_text,
                    setting_case.updated_provenance_prefix,
                    issue_prefix + ": " + context + " should preserve selected-settings provenance for " +
                        setting_case.property_name);
    const std::string selected_settings = selected_settings_segment(json_text);
    expect(!selected_settings.empty(),
           issue_prefix + ": " + context + " should expose a selected-settings JSON block");
    if (!selected_settings.empty()) {
        expect_contains(selected_settings,
                        "\"name\": \"" + setting_case.property_name + "\"",
                        issue_prefix + ": " + context + " should preserve " + setting_case.property_name +
                            " in selected settings");
        expect_contains(selected_settings,
                        "\"value\": \"" + expected_value + "\"",
                        issue_prefix + ": " + context + " should expose the " + setting_case.property_name +
                            " value through selected settings");
    }
}

void exercise_real_sample_page_setup_setting_round_trip(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const RealSamplePair& sample,
    const RealSampleSettingCase& setting_case) {
    namespace fs = std::filesystem;

    const fs::path work_root = temp_root / sample.primary.stem() / setting_case.property_name;
    std::error_code ignored;
    fs::remove_all(work_root, ignored);
    fs::create_directories(work_root);

    const fs::path copied_primary = work_root / sample.primary.filename();
    const fs::path copied_sidecar = work_root / sample.sidecar.filename();
    std::error_code copy_error;
    fs::copy_file(sample.primary, copied_primary, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3760: should copy the real primary asset into temp space");
    copy_error.clear();
    fs::copy_file(sample.sidecar, copied_sidecar, fs::copy_options::overwrite_existing, copy_error);
    expect(!copy_error, "#3760: should copy the real sidecar asset into temp space");
    expect(make_writable(copied_primary), "#3760: copied primary asset should become writable");
    expect(make_writable(copied_sidecar), "#3760: copied sidecar asset should become writable");

    const std::string original_primary_bytes = read_binary(copied_primary);
    const std::string original_sidecar_bytes = read_binary(copied_sidecar);

    const auto initial_process = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        work_root);
    if (initial_process.exit_code != 0) {
        std::cerr << "studio host sample " << setting_case.property_name << " initial read stdout:\n"
                  << initial_process.stdout_text << "\n";
        std::cerr << "studio host sample " << setting_case.property_name << " initial read stderr:\n"
                  << initial_process.stderr_text << "\n";
    }
    expect(initial_process.exit_code == 0,
           "#3760: real sample " + setting_case.property_name + " initial read should succeed");
    int initial_setting_count = 0;
    expect(copperfin::test_support::extract_json_integer(
               initial_process.stdout_text,
               "\"settingCount\"",
               initial_setting_count),
           "#3760: real sample " + setting_case.property_name +
               " initial read should expose a numeric setting count");

    const auto set_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--set-property",
            "--record", "0",
            "--property-name", setting_case.property_name,
            "--property-value", setting_case.updated_value,
            "--json"
        },
        work_root);
    if (set_process.exit_code != 0) {
        std::cerr << "studio host sample " << setting_case.property_name << " update stdout:\n"
                  << set_process.stdout_text << "\n";
        std::cerr << "studio host sample " << setting_case.property_name << " update stderr:\n"
                  << set_process.stderr_text << "\n";
    }
    expect(set_process.exit_code == 0,
           "#3760: real sample " + setting_case.property_name + " update should succeed");

    const auto updated_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = setting_case.property_name
    });
    expect(updated_property.ok && updated_property.exists && updated_property.value == setting_case.updated_value,
           "#3760: real sample " + setting_case.property_name + " update should persist the memo-backed field");

    const std::string updated_primary_bytes = read_binary(copied_primary);
    const std::string updated_sidecar_bytes = read_binary(copied_sidecar);
    expect(updated_primary_bytes != original_primary_bytes,
           "#3760: real sample " + setting_case.property_name + " update should change the primary asset bytes");
    expect(updated_sidecar_bytes != original_sidecar_bytes,
           "#3760: real sample " + setting_case.property_name + " update should change the memo sidecar bytes");

    const auto reopen_after_set = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        work_root);
    if (reopen_after_set.exit_code != 0) {
        std::cerr << "studio host sample " << setting_case.property_name << " reopen stdout:\n"
                  << reopen_after_set.stdout_text << "\n";
        std::cerr << "studio host sample " << setting_case.property_name << " reopen stderr:\n"
                  << reopen_after_set.stderr_text << "\n";
    }
    expect(reopen_after_set.exit_code == 0,
           "#3760: real sample reopen after " + setting_case.property_name + " update should succeed");
    expect_common_reopen_json(
        reopen_after_set.stdout_text,
        sample,
        true,
        "#3760",
        "reopened updated real sample " + setting_case.property_name + " read");
    for (const auto& needle : setting_case.updated_json_needles) {
        expect_contains(reopen_after_set.stdout_text,
                        needle,
                        "#3760: reopened updated real sample " + setting_case.property_name +
                            " read should preserve page-setup metadata");
    }
    expect_selected_setting_value(
        reopen_after_set.stdout_text,
        setting_case,
        setting_case.updated_value,
        "#3760",
        "reopened updated real sample " + setting_case.property_name + " read");

    const auto undo_process = run_process_capture(
        studio_host_path,
        {
            "--path", copied_primary.string(),
            "--record", "0",
            "--undo-mode", "command",
            "--json"
        },
        work_root);
    if (undo_process.exit_code != 0) {
        std::cerr << "studio host sample " << setting_case.property_name << " undo stdout:\n"
                  << undo_process.stdout_text << "\n";
        std::cerr << "studio host sample " << setting_case.property_name << " undo stderr:\n"
                  << undo_process.stderr_text << "\n";
    }
    expect(undo_process.exit_code == 0,
           "#3760: real sample " + setting_case.property_name + " undo should succeed");
    expect_common_reopen_json(
        undo_process.stdout_text,
        sample,
        false,
        "#3760",
        "undone real sample " + setting_case.property_name + " read");
    for (const auto& needle : setting_case.restored_json_needles) {
        expect_contains(undo_process.stdout_text,
                        needle,
                        "#3760: undone real sample " + setting_case.property_name +
                            " read should restore page-setup metadata");
    }
    expect_selected_setting_value(
        undo_process.stdout_text,
        setting_case,
        setting_case.original_value,
        "#3760",
        "undone real sample " + setting_case.property_name + " read");
    const auto restored_property = copperfin::vfp::query_visual_object_property({
        .path = copied_primary.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = setting_case.property_name
    });
    expect(restored_property.ok && restored_property.exists && restored_property.value == setting_case.original_value,
           "#3760: real sample " + setting_case.property_name + " undo should restore the field");
    const auto reopen_after_undo = run_process_capture(
        studio_host_path,
        {"--path", copied_primary.string(), "--record", "0", "--json"},
        work_root);
    if (reopen_after_undo.exit_code != 0) {
        std::cerr << "studio host sample " << setting_case.property_name << " undo reopen stdout:\n"
                  << reopen_after_undo.stdout_text << "\n";
        std::cerr << "studio host sample " << setting_case.property_name << " undo reopen stderr:\n"
                  << reopen_after_undo.stderr_text << "\n";
    }
    expect(reopen_after_undo.exit_code == 0,
           "#3760: real sample reopen after " + setting_case.property_name + " undo should succeed");
    expect_common_reopen_json(
        reopen_after_undo.stdout_text,
        sample,
        false,
        "#3760",
        "reopened undone real sample " + setting_case.property_name + " read");
    for (const auto& needle : setting_case.restored_json_needles) {
        expect_contains(reopen_after_undo.stdout_text,
                        needle,
                        "#3760: reopened undone real sample " + setting_case.property_name +
                            " read should restore page-setup metadata");
    }
    expect(copperfin::test_support::json_integer_delta(
               reopen_after_set.stdout_text,
               initial_process.stdout_text,
               "\"settingCount\"",
               0),
           "#3760: real sample " + setting_case.property_name +
               " update should preserve the baseline setting count");
    expect(copperfin::test_support::json_integer_delta(
               reopen_after_undo.stdout_text,
               initial_process.stdout_text,
               "\"settingCount\"",
               0),
           "#3760: real sample " + setting_case.property_name +
               " command undo should restore the baseline setting count");
    expect_selected_setting_value(
        reopen_after_undo.stdout_text,
        setting_case,
        setting_case.original_value,
        "#3760",
        "reopened undone real sample " + setting_case.property_name + " read");
}

void test_real_vfp9_report_and_label_page_setup_settings_round_trip(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path reports_root = copperfin::test_support::find_vfp9_reports_root();
    if (reports_root.empty()) {
        std::cerr << "SKIP: #3760 real VFP9 report samples were not found\n";
        return;
    }

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_real_vfp9_page_setup_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const std::vector<RealSamplePair> samples{
        {
            .primary = reports_root / "invoice.frx",
            .sidecar = reports_root / "invoice.frt",
            .title = "invoice.frx",
            .is_label = false
        },
        {
            .primary = reports_root / "cust.lbx",
            .sidecar = reports_root / "cust.lbt",
            .title = "cust.lbx",
            .is_label = true
        }
    };
    const std::vector<RealSampleSettingCase> setting_cases{
        {
            .property_name = "ORIENTATION",
            .original_value = "0",
            .updated_value = "1",
            .updated_provenance_prefix =
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 6, \"sourceLineIndex\": 0",
            .updated_json_needles = {"\"orientationCode\": 1", "\"paperSizeCode\": 1", "\"colorAvailable\": true", "\"color\": 1"},
            .restored_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 1", "\"colorAvailable\": true", "\"color\": 1"}
        },
        {
            .property_name = "PAPERSIZE",
            .original_value = "1",
            .updated_value = "9",
            .updated_provenance_prefix =
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 6, \"sourceLineIndex\": 1",
            .updated_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 9", "\"colorAvailable\": true", "\"color\": 1"},
            .restored_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 1", "\"colorAvailable\": true", "\"color\": 1"}
        },
        {
            .property_name = "COLOR",
            .original_value = "1",
            .updated_value = "0",
            .updated_provenance_prefix =
                "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 6, \"sourceLineIndex\": 2",
            .updated_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 1", "\"colorAvailable\": true", "\"color\": 0"},
            .restored_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 1", "\"colorAvailable\": true", "\"color\": 1"}
        }
    };

    for (const auto& sample : samples) {
        for (const auto& setting_case : setting_cases) {
            RealSampleSettingCase case_for_sample = setting_case;
            if (sample.is_label && case_for_sample.property_name == "PAPERSIZE") {
                case_for_sample.updated_value = "9";
            } else if (!sample.is_label && case_for_sample.property_name == "PAPERSIZE") {
                case_for_sample.updated_value = "5";
                case_for_sample.updated_json_needles = {"\"orientationCode\": 0", "\"paperSizeCode\": 5"};
            }

            exercise_real_sample_page_setup_setting_round_trip(
                studio_host_path,
                temp_root,
                sample,
                case_for_sample);
        }
    }

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: test_studio_host_real_sample_page_setup_settings_round_trip <studio-host>\n";
        return 1;
    }

    test_real_vfp9_report_and_label_page_setup_settings_round_trip(argv[1]);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
    }
    return failures == 0 ? 0 : 1;
}
