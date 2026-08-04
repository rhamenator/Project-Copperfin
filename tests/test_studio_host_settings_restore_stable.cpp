// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

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

void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
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

void write_synthetic_report_table_for_layout_json(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2796: synthetic report/label layout settings fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2796: synthetic report/label layout settings fixture should mark deleted objects");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2796: deleted settings fixture should mark report settings deleted");
}

void write_synthetic_report_table_for_stable_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_settings_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-settings-guid"
    });
    expect(unique_id_result.ok, "#2796: stable deleted settings fixture should seed a deleted settings unique id");
    expect(dbf_record_deleted(report_path, 0U),
           "#2796: stable deleted settings fixture should preserve the deleted settings state");
}

void write_synthetic_report_table_for_stable_deleted_unsupported_expr_settings_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "TOPMARGIN", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1", "", "", "", "", "", "10", "unsupported-deleted-id"},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3095: stable deleted settings restore fixture should create unsupported EXPR layout");
    const auto object_delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(object_delete_result.ok, "#3095: stable deleted settings restore fixture should mark deleted objects");
    const auto settings_delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(settings_delete_result.ok, "#3095: stable deleted settings restore fixture should mark settings deleted");
    expect(dbf_record_deleted(report_path, 0U),
           "#3095: stable deleted settings restore fixture should preserve the deleted settings state");
}

void run_settings_restore_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_deleted_settings_json(asset_path);
    expect(dbf_record_deleted(asset_path, 0U), issue_prefix + " fixture should start with deleted settings");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--restore-object",
            "--unique-id", "deleted-settings-guid",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable settings restore stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable settings restore stderr:\n"
                  << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(!dbf_record_deleted(asset_path, 0U), issue_prefix + " should clear the settings record delete flag");
    expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_full_report_layout_preview_bounds(restore_process.stdout_text, issue_prefix);
    expect_contains(restore_process.stdout_text, "\"settingCount\": 6",
                    issue_prefix + " should restore live setting counts");
    expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                    issue_prefix + " should clear deleted setting counts");
    expect_contains(restore_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " should restore live page setup summaries");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        issue_prefix + " should move root settings back into live-setting metadata");
    expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0"
        },
        issue_prefix + " should rehydrate selected-settings metadata");
    expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " should preserve live section metadata");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object metadata");
}

void run_unsupported_expr_settings_restore_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_deleted_unsupported_expr_settings_json(asset_path);
    expect(dbf_record_deleted(asset_path, 0U), issue_prefix + " fixture should start with deleted settings");

    const auto restore_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--restore-object",
            "--unique-id", "unsupported-deleted-id",
            "--json"
        },
        temp_root);

    if (restore_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable unsupported EXPR settings restore stdout:\n"
                  << restore_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable unsupported EXPR settings restore stderr:\n"
                  << restore_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(restore_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(!dbf_record_deleted(asset_path, 0U), issue_prefix + " should clear the settings record delete flag");
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "unsupported-deleted-id",
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists,
           issue_prefix + " should leave the EXPR memo queryable after restore");
    expect(normalize_line_endings(expr_property.value) ==
               "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1",
           issue_prefix + " should preserve raw unsupported EXPR lines across restore");
    expect_contains(restore_process.stdout_text, "\"settingCount\": 4",
                    issue_prefix + " should restore live key/value settings only");
    expect_contains(restore_process.stdout_text, "\"deletedSettingCount\": 0",
                    issue_prefix + " should clear deleted setting counts");
    expect_contains(restore_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"settings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " should restore parsed setting source-line gaps around unsupported EXPR lines");
    expect_contains_in_order(
        restore_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " should rehydrate selected parsed settings after restore");
    expect_contains(restore_process.stdout_text, "\"pageSetupAvailable\": true",
                    issue_prefix + " should restore live page setup availability");
    expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object metadata");
    expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " should preserve live section metadata");
    expect(restore_process.stdout_text.find("\"name\": \"* keep-this-comment\"") == std::string::npos,
           issue_prefix + " should not fabricate comment lines as live settings");
}

void test_studio_host_json_preserves_settings_restore_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_settings_restore_stable_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_settings_restore_case(
        studio_host_path,
        temp_root,
        "settings_restore_stable.frx",
        "report",
        "#2796: stable-selected report settings restore");
    run_settings_restore_case(
        studio_host_path,
        temp_root,
        "settings_restore_stable.lbx",
        "label",
        "#2796: stable-selected label settings restore");
    run_unsupported_expr_settings_restore_case(
        studio_host_path,
        temp_root,
        "settings_restore_unsupported_stable.frx",
        "report",
        "#3095: stable-selected report settings restore should preserve unsupported EXPR lines");
    run_unsupported_expr_settings_restore_case(
        studio_host_path,
        temp_root,
        "settings_restore_unsupported_stable.lbx",
        "label",
        "#3095: stable-selected label settings restore should preserve unsupported EXPR lines");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_settings_restore_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_settings_restore_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
