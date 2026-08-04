// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"
#include "test_locale_catalog_environment_support.h"

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

void expect_contains_in_order(
    const std::string& text,
    const std::vector<std::string>& needles,
    const std::string& message) {
    std::size_t offset = 0U;
    for (const auto& needle : needles) {
        const std::size_t position = text.find(needle, offset);
        if (position == std::string::npos) {
            expect(false, message);
            return;
        }
        offset = position + needle.size();
    }
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

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

std::string normalize_line_endings(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (text[index] == '\r') {
            if (index + 1U < text.size() && text[index + 1U] == '\n') {
                continue;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(text[index]);
        }
    }
    return normalized;
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
        result.stdout_text = read_text(stdout_path);
    }
    if (fs::exists(stderr_path)) {
        result.stderr_text = read_text(stderr_path);
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

void write_sort_settings_fixture(
    const std::filesystem::path& asset_path,
    const std::string& settings_guid) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "TAG", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=1\nPAPERSIZE=9", "customer.country", settings_guid}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(create_result.ok, "#2909: TAG sort-settings fixture should be created");
}

void write_deleted_sort_settings_fixture(
    const std::filesystem::path& asset_path,
    const std::string& settings_guid) {
    write_sort_settings_fixture(asset_path, settings_guid);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 0U, true);
    expect(delete_result.ok, "#2909: deleted TAG sort-settings fixture should mark the root record deleted");
}

void write_unsupported_sort_settings_fixture(
    const std::filesystem::path& asset_path,
    const std::string& settings_guid) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "TAG", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=1\n* keep-this-comment\n\nPAPERSIZE=9\nXUSER=keepme", "customer.country", settings_guid}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
    expect(create_result.ok, "#3099: unsupported TAG sort-settings fixture should be created");
}

void write_deleted_unsupported_sort_settings_fixture(
    const std::filesystem::path& asset_path,
    const std::string& settings_guid) {
    write_unsupported_sort_settings_fixture(asset_path, settings_guid);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 0U, true);
    expect(delete_result.ok, "#3099: deleted unsupported TAG sort-settings fixture should mark the root record deleted");
}

void expect_sort_settings_page_summary(const std::string& text, const std::string& issue_prefix) {
    expect_contains(text, "\"pageSetupAvailable\": true",
                    issue_prefix + " should expose effective page setup availability");
    expect_contains(text, "\"orientationAvailable\": true",
                    issue_prefix + " should expose orientation availability");
    expect_contains(text, "\"orientationCode\": 1",
                    issue_prefix + " should preserve memo-derived orientation");
    expect_contains(text, "\"paperSizeAvailable\": true",
                    issue_prefix + " should expose paper-size availability");
    expect_contains(text, "\"paperSizeCode\": 9",
                    issue_prefix + " should preserve memo-derived paper size");
}

void expect_live_sort_setting_json(
    const std::string& text,
    const std::string& title,
    const std::string& expected_tag,
    const std::string& issue_prefix,
    bool label_asset) {
    expect_contains(text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " should preserve document titles");
    if (label_asset) {
        expect_contains(text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should preserve selected-settings availability");
    expect_contains(text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should preserve settings selection kind");
    expect_sort_settings_page_summary(text, issue_prefix);
    expect_contains(text, "\"settingCount\": 3",
                    issue_prefix + " should expose the TAG direct setting in live counts");
    expect_contains(text, "\"deletedSettingCount\": 0",
                    issue_prefix + " should keep deleted setting counts empty");
    expect_contains_in_order(
        text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should expose refreshed TAG provenance in live settings");
    expect_contains_in_order(
        text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should expose refreshed TAG provenance in selected settings");
}

void expect_deleted_sort_setting_json(
    const std::string& text,
    const std::string& title,
    const std::string& expected_tag,
    const std::string& issue_prefix,
    bool label_asset) {
    expect_contains(text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " should preserve document titles");
    if (label_asset) {
        expect_contains(text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should preserve selected-settings availability");
    expect_contains(text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should preserve settings selection kind");
    expect_sort_settings_page_summary(text, issue_prefix);
    expect_contains(text, "\"settingCount\": 0",
                    issue_prefix + " should keep live setting counts empty");
    expect_contains(text, "\"deletedSettingCount\": 3",
                    issue_prefix + " should expose deleted TAG settings in deleted counts");
    expect_contains_in_order(
        text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should expose refreshed TAG provenance in deleted settings");
    expect_contains_in_order(
        text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should expose refreshed deleted TAG provenance in selected settings");
}

void expect_live_unsupported_sort_setting_json(
    const std::string& text,
    const std::string& title,
    const std::string& expected_tag,
    const std::string& issue_prefix,
    bool label_asset) {
    expect_contains(text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " should preserve document titles");
    if (label_asset) {
        expect_contains(text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should preserve selected-settings availability");
    expect_contains(text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should preserve settings selection kind");
    expect_sort_settings_page_summary(text, issue_prefix);
    expect_contains(text, "\"settingCount\": 4",
                    issue_prefix + " should expose supported settings plus the TAG direct field");
    expect_contains(text, "\"deletedSettingCount\": 0",
                    issue_prefix + " should keep deleted setting counts empty");
    expect_contains_in_order(
        text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should preserve source-line gaps around unsupported EXPR lines");
    expect_contains_in_order(
        text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should preserve selected settings without comment rows");
    expect_not_contains(text,
                        "\"name\": \"* keep-this-comment\"",
                        issue_prefix + " should not fabricate comment rows as settings");
}

void expect_deleted_unsupported_sort_setting_json(
    const std::string& text,
    const std::string& title,
    const std::string& expected_tag,
    const std::string& issue_prefix,
    bool label_asset) {
    expect_contains(text, "\"documentTitle\": \"" + title + "\"",
                    issue_prefix + " should preserve document titles");
    if (label_asset) {
        expect_contains(text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should preserve selected-settings availability");
    expect_contains(text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should preserve settings selection kind");
    expect_sort_settings_page_summary(text, issue_prefix);
    expect_contains(text, "\"settingCount\": 0",
                    issue_prefix + " should keep live setting counts empty");
    expect_contains(text, "\"deletedSettingCount\": 4",
                    issue_prefix + " should expose deleted supported settings plus TAG");
    expect_contains_in_order(
        text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should preserve deleted source-line gaps around unsupported EXPR lines");
    expect_contains_in_order(
        text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3, \"sourceLineIndex\": null",
            "\"value\": \"" + expected_tag + "\""
        },
        issue_prefix + " should preserve selected deleted settings without comment rows");
    expect_not_contains(text,
                        "\"name\": \"* keep-this-comment\"",
                        issue_prefix + " should not fabricate deleted comment rows as settings");
}

void test_updates_report_sort_settings_by_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_sort_settings_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_update = [&](const fs::path& asset_path,
                                const std::string& title,
                                const std::string& label,
                                bool label_asset) {
        write_sort_settings_fixture(asset_path, "settings-guid");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "settings-guid",
                "--property-name", "TAG",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable TAG update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable TAG update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2909: report/label stable TAG update should exit successfully");
        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#2909: report/label stable TAG update reopen should exit successfully");
        expect_live_sort_setting_json(
            reopen_process.stdout_text,
            title,
            "customer.region",
            "#2909: report/label stable TAG update reopen JSON",
            label_asset);
    };

    run_update(temp_root / "sort_update_stable.frx", "sort_update_stable.frx", "report", false);
    run_update(temp_root / "sort_update_stable.lbx", "sort_update_stable.lbx", "label", true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_clears_report_sort_settings_by_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_sort_settings_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_clear = [&](const fs::path& asset_path,
                               const std::string& title,
                               const std::string& label,
                               bool label_asset) {
        write_sort_settings_fixture(asset_path, "settings-guid");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "settings-guid",
                "--property-name", "TAG",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable TAG clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable TAG clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2909: report/label stable TAG clear should exit successfully");
        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#2909: report/label stable TAG clear reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2909: report/label stable TAG clear reopen JSON should preserve document titles");
        if (label_asset) {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#2909: label stable TAG clear reopen JSON should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#2909: report/label stable TAG clear reopen JSON should preserve selected-settings availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#2909: report/label stable TAG clear reopen JSON should preserve settings selection kind");
        expect_sort_settings_page_summary(
            reopen_process.stdout_text,
            "#2909: report/label stable TAG clear reopen JSON");
        expect_contains(reopen_process.stdout_text, "\"settingCount\": 2",
                        "#2909: report/label stable TAG clear reopen JSON should remove TAG from live setting counts");
        expect_contains(reopen_process.stdout_text, "\"deletedSettingCount\": 0",
                        "#2909: report/label stable TAG clear reopen JSON should keep deleted setting counts empty");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#2909: report/label stable TAG clear reopen JSON should preserve remaining selected settings");
        expect_not_contains(reopen_process.stdout_text,
                            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#2909: report/label stable TAG clear reopen JSON should remove TAG provenance");
    };

    run_clear(temp_root / "sort_clear_stable.frx", "sort_clear_stable.frx", "report", false);
    run_clear(temp_root / "sort_clear_stable.lbx", "sort_clear_stable.lbx", "label", true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_updates_deleted_report_sort_settings_by_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_sort_settings_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_update = [&](const fs::path& asset_path,
                                const std::string& title,
                                const std::string& label,
                                bool label_asset) {
        write_deleted_sort_settings_fixture(asset_path, "deleted-settings-guid");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "TAG",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted TAG update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted TAG update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2909: report/label stable deleted TAG update should exit successfully");
        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#2909: report/label stable deleted TAG update reopen should exit successfully");
        expect_deleted_sort_setting_json(
            reopen_process.stdout_text,
            title,
            "customer.region",
            "#2909: report/label stable deleted TAG update reopen JSON",
            label_asset);
    };

    run_update(temp_root / "deleted_sort_update_stable.frx",
               "deleted_sort_update_stable.frx",
               "report",
               false);
    run_update(temp_root / "deleted_sort_update_stable.lbx",
               "deleted_sort_update_stable.lbx",
               "label",
               true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_clears_deleted_report_sort_settings_by_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_sort_settings_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_clear = [&](const fs::path& asset_path,
                               const std::string& title,
                               const std::string& label,
                               bool label_asset) {
        write_deleted_sort_settings_fixture(asset_path, "deleted-settings-guid");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-settings-guid",
                "--property-name", "TAG",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted TAG clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted TAG clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2909: report/label stable deleted TAG clear should exit successfully");
        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#2909: report/label stable deleted TAG clear reopen should exit successfully");
        expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2909: report/label stable deleted TAG clear reopen JSON should preserve document titles");
        if (label_asset) {
            expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                            "#2909: label stable deleted TAG clear reopen JSON should retain label identity");
        }
        expect_contains(reopen_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#2909: report/label stable deleted TAG clear reopen JSON should preserve selected-settings availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#2909: report/label stable deleted TAG clear reopen JSON should preserve settings selection kind");
        expect_sort_settings_page_summary(
            reopen_process.stdout_text,
            "#2909: report/label stable deleted TAG clear reopen JSON");
        expect_contains(reopen_process.stdout_text, "\"settingCount\": 0",
                        "#2909: report/label stable deleted TAG clear reopen JSON should keep live setting counts empty");
        expect_contains(reopen_process.stdout_text, "\"deletedSettingCount\": 2",
                        "#2909: report/label stable deleted TAG clear reopen JSON should remove TAG from deleted setting counts");
        expect_contains_in_order(
            reopen_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1"
            },
            "#2909: report/label stable deleted TAG clear reopen JSON should preserve remaining deleted settings");
        expect_not_contains(reopen_process.stdout_text,
                            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#2909: report/label stable deleted TAG clear reopen JSON should remove deleted TAG provenance");
    };

    run_clear(temp_root / "deleted_sort_clear_stable.frx",
              "deleted_sort_clear_stable.frx",
              "report",
              false);
    run_clear(temp_root / "deleted_sort_clear_stable.lbx",
              "deleted_sort_clear_stable.lbx",
              "label",
              true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_updates_report_sort_settings_preserve_unsupported_expr_lines_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_sort_settings_unsupported_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const std::string expected_expr =
        "ORIENTATION=1\n"
        "* keep-this-comment\n"
        "\n"
        "PAPERSIZE=9\n"
        "XUSER=keepme";

    const auto run_update = [&](const fs::path& asset_path,
                                const std::string& title,
                                const std::string& label,
                                bool label_asset,
                                bool deleted) {
        if (deleted) {
            write_deleted_unsupported_sort_settings_fixture(asset_path, "unsup-del-guid");
        } else {
            write_unsupported_sort_settings_fixture(asset_path, "unsup-sort-guid");
        }

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", deleted ? "unsup-del-guid" : "unsup-sort-guid",
                "--property-name", "TAG",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable unsupported TAG update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable unsupported TAG update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#3099: stable TAG update should exit successfully when unsupported EXPR lines are present");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = deleted ? "unsup-del-guid" : "unsup-sort-guid",
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists,
               "#3099: stable TAG update should keep the EXPR memo queryable");
        expect(normalize_line_endings(expr_property.value) == expected_expr,
               "#3099: stable TAG update should preserve unsupported EXPR comment and blank lines");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#3099: stable TAG update reopen should exit successfully with unsupported EXPR lines");
        if (deleted) {
            expect_deleted_unsupported_sort_setting_json(
                reopen_process.stdout_text,
                title,
                "customer.region",
                "#3099: stable deleted TAG update reopen JSON",
                label_asset);
        } else {
            expect_live_unsupported_sort_setting_json(
                reopen_process.stdout_text,
                title,
                "customer.region",
                "#3099: stable TAG update reopen JSON",
                label_asset);
        }
    };

    run_update(temp_root / "unsupported_sort_update_stable.frx",
               "unsupported_sort_update_stable.frx",
               "report",
               false,
               false);
    run_update(temp_root / "unsupported_sort_update_stable.lbx",
               "unsupported_sort_update_stable.lbx",
               "label",
               true,
               false);
    run_update(temp_root / "deleted_unsupported_sort_update_stable.frx",
               "deleted_unsupported_sort_update_stable.frx",
               "report",
               false,
               true);
    run_update(temp_root / "deleted_unsupported_sort_update_stable.lbx",
               "deleted_unsupported_sort_update_stable.lbx",
               "label",
               true,
               true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_clears_report_sort_settings_preserve_unsupported_expr_lines_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_sort_settings_unsupported_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const std::string expected_expr =
        "ORIENTATION=1\n"
        "* keep-this-comment\n"
        "\n"
        "PAPERSIZE=9\n"
        "XUSER=keepme";

    const auto run_clear = [&](const fs::path& asset_path,
                               const std::string& title,
                               const std::string& label,
                               bool label_asset,
                               bool deleted) {
        if (deleted) {
            write_deleted_unsupported_sort_settings_fixture(asset_path, "unsup-del-guid");
        } else {
            write_unsupported_sort_settings_fixture(asset_path, "unsup-sort-guid");
        }

        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", deleted ? "unsup-del-guid" : "unsup-sort-guid",
                "--property-name", "TAG",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable unsupported TAG clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable unsupported TAG clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#3099: stable TAG clear should exit successfully when unsupported EXPR lines are present");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = deleted ? "unsup-del-guid" : "unsup-sort-guid",
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists,
               "#3099: stable TAG clear should keep the EXPR memo queryable");
        expect(normalize_line_endings(expr_property.value) == expected_expr,
               "#3099: stable TAG clear should preserve unsupported EXPR comment and blank lines");

        const auto reopen_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);
        expect(reopen_process.exit_code == 0,
               "#3099: stable TAG clear reopen should exit successfully with unsupported EXPR lines");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#3099: stable TAG clear reopen JSON should preserve selected-settings availability");
        expect_contains(reopen_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                        "#3099: stable TAG clear reopen JSON should preserve settings selection kind");
        if (deleted) {
            expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#3099: stable deleted TAG clear reopen JSON should preserve document titles");
            if (label_asset) {
                expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                                "#3099: label stable deleted TAG clear reopen JSON should retain label identity");
            }
            expect_sort_settings_page_summary(
                reopen_process.stdout_text,
                "#3099: stable deleted TAG clear reopen JSON");
            expect_contains(reopen_process.stdout_text, "\"settingCount\": 0",
                            "#3099: stable deleted TAG clear reopen JSON should keep live setting counts empty");
            expect_contains(reopen_process.stdout_text, "\"deletedSettingCount\": 3",
                            "#3099: stable deleted TAG clear reopen JSON should preserve supported deleted settings");
            expect_contains_in_order(
                reopen_process.stdout_text,
                {
                    "\"deletedSettings\": [",
                    "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                    "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                    "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
                },
                "#3099: stable deleted TAG clear reopen JSON should preserve deleted source-line gaps");
            expect_contains_in_order(
                reopen_process.stdout_text,
                {
                    "\"selectedReportSettings\": [",
                    "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                    "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                    "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
                },
                "#3099: stable deleted TAG clear reopen JSON should preserve selected deleted settings");
        } else {
            expect_contains(reopen_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#3099: stable TAG clear reopen JSON should preserve document titles");
            if (label_asset) {
                expect_contains(reopen_process.stdout_text, "\"isLabel\": true",
                                "#3099: label stable TAG clear reopen JSON should retain label identity");
            }
            expect_sort_settings_page_summary(
                reopen_process.stdout_text,
                "#3099: stable TAG clear reopen JSON");
            expect_contains(reopen_process.stdout_text, "\"settingCount\": 3",
                            "#3099: stable TAG clear reopen JSON should preserve supported live settings");
            expect_contains(reopen_process.stdout_text, "\"deletedSettingCount\": 0",
                            "#3099: stable TAG clear reopen JSON should keep deleted setting counts empty");
            expect_contains_in_order(
                reopen_process.stdout_text,
                {
                    "\"settings\": [",
                    "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                    "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                    "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
                },
                "#3099: stable TAG clear reopen JSON should preserve live source-line gaps");
            expect_contains_in_order(
                reopen_process.stdout_text,
                {
                    "\"selectedReportSettings\": [",
                    "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
                    "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
                    "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4"
                },
                "#3099: stable TAG clear reopen JSON should preserve selected live settings");
        }
        expect_not_contains(reopen_process.stdout_text,
                            "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 3",
                            "#3099: stable TAG clear reopen JSON should remove TAG provenance");
        expect_not_contains(reopen_process.stdout_text,
                            "\"name\": \"* keep-this-comment\"",
                            "#3099: stable TAG clear reopen JSON should not fabricate comment rows as settings");
    };

    run_clear(temp_root / "unsupported_sort_clear_stable.frx",
              "unsupported_sort_clear_stable.frx",
              "report",
              false,
              false);
    run_clear(temp_root / "unsupported_sort_clear_stable.lbx",
              "unsupported_sort_clear_stable.lbx",
              "label",
              true,
              false);
    run_clear(temp_root / "deleted_unsupported_sort_clear_stable.frx",
              "deleted_unsupported_sort_clear_stable.frx",
              "report",
              false,
              true);
    run_clear(temp_root / "deleted_unsupported_sort_clear_stable.lbx",
              "deleted_unsupported_sort_clear_stable.lbx",
              "label",
              true,
              true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_report_sort_settings_stable <studio_host_path>\n";
        return 1;
    }

    test_updates_report_sort_settings_by_stable_selection(argv[1]);
    test_clears_report_sort_settings_by_stable_selection(argv[1]);
    test_updates_deleted_report_sort_settings_by_stable_selection(argv[1]);
    test_clears_deleted_report_sort_settings_by_stable_selection(argv[1]);
    test_updates_report_sort_settings_preserve_unsupported_expr_lines_by_stable_selection(argv[1]);
    test_clears_report_sort_settings_preserve_unsupported_expr_lines_by_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
