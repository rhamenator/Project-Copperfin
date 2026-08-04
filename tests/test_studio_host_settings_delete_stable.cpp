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


void expect_settings_selection_metadata(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"selectedReportSettingsAvailable\": true",
                    prefix + " should advertise selected-settings availability");
    expect_contains(text, "\"selectedReportSelectionAvailable\": true",
                    prefix + " should advertise report-selection availability");
    expect_contains(text, "\"selectedReportSelectionKind\": \"settings\"",
                    prefix + " should expose settings selection kind");
    expect_contains(text, "\"selectedReportSectionAvailable\": false",
                    prefix + " should not advertise selected-section availability");
    expect_contains(text, "\"selectedReportSection\": null",
                    prefix + " should serialize null selected sections");
    expect_contains(text, "\"selectedReportObjectAvailable\": false",
                    prefix + " should not advertise selected-object availability");
    expect_contains(text, "\"selectedReportObject\": null",
                    prefix + " should serialize null selected objects");
    expect_contains(text, "\"selectedReportObjectSectionAvailable\": false",
                    prefix + " should not advertise containing-section availability");
    expect_contains(text, "\"selectedReportObjectSection\": null",
                    prefix + " should serialize null containing sections");
}

void expect_supported_deleted_setting_payloads(const std::string& text, const std::string& prefix) {
    const std::vector<std::string> expected_settings{
        "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
        "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
        "\"name\": \"BOTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"20\"",
        "\"name\": \"GRIDV\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3, \"memoBlockNumber\": 1, \"value\": \"4\"",
        "\"name\": \"GRIDH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4, \"memoBlockNumber\": 1, \"value\": \"8\"",
        "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\""
    };

    std::vector<std::string> deleted_payload{"\"deletedSettings\": ["};
    deleted_payload.insert(deleted_payload.end(), expected_settings.begin(), expected_settings.end());
    expect_contains_in_order(text, deleted_payload,
                             prefix + " should expose complete deleted-setting provenance");

    std::vector<std::string> selected_payload{"\"selectedReportSettings\": ["};
    selected_payload.insert(selected_payload.end(), expected_settings.begin(), expected_settings.end());
    expect_contains_in_order(text, selected_payload,
                             prefix + " should expose complete selected deleted-setting provenance");
}

void expect_unset_deleted_page_summary_fields(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"paperLengthAvailable\": false",
                    prefix + " should keep paper length unavailable");
    expect_contains(text, "\"paperLength\": 0",
                    prefix + " should keep paper length inert");
    expect_contains(text, "\"paperWidthAvailable\": false",
                    prefix + " should keep paper width unavailable");
    expect_contains(text, "\"paperWidth\": 0",
                    prefix + " should keep paper width inert");
    expect_contains(text, "\"leftMarginAvailable\": false",
                    prefix + " should keep left margin unavailable");
    expect_contains(text, "\"leftMargin\": 0",
                    prefix + " should keep left margin inert");
    expect_contains(text, "\"rightMarginAvailable\": false",
                    prefix + " should keep right margin unavailable");
    expect_contains(text, "\"rightMargin\": 0",
                    prefix + " should keep right margin inert");
    expect_contains(text, "\"copiesAvailable\": false",
                    prefix + " should keep copies unavailable");
    expect_contains(text, "\"copies\": 0",
                    prefix + " should keep copies inert");
    expect_contains(text, "\"driverAvailable\": false",
                    prefix + " should keep driver unavailable");
    expect_contains(text, "\"driver\": \"\"",
                    prefix + " should keep driver empty");
    expect_contains(text, "\"deviceAvailable\": false",
                    prefix + " should keep device unavailable");
    expect_contains(text, "\"device\": \"\"",
                    prefix + " should keep device empty");
    expect_contains(text, "\"outputAvailable\": false",
                    prefix + " should keep output unavailable");
    expect_contains(text, "\"output\": \"\"",
                    prefix + " should keep output empty");
    expect_contains(text, "\"defaultSourceAvailable\": false",
                    prefix + " should keep default source unavailable");
    expect_contains(text, "\"defaultSource\": 0",
                    prefix + " should keep default source inert");
    expect_contains(text, "\"printQualityAvailable\": false",
                    prefix + " should keep print quality unavailable");
    expect_contains(text, "\"printQuality\": 0",
                    prefix + " should keep print quality inert");
    expect_contains(text, "\"yResolutionAvailable\": false",
                    prefix + " should keep Y resolution unavailable");
    expect_contains(text, "\"yResolution\": 0",
                    prefix + " should keep Y resolution inert");
    expect_contains(text, "\"trueTypeOptionAvailable\": false",
                    prefix + " should keep TrueType options unavailable");
    expect_contains(text, "\"trueTypeOption\": 0",
                    prefix + " should keep TrueType options inert");
    expect_contains(text, "\"asciiAvailable\": false",
                    prefix + " should keep ASCII mode unavailable");
    expect_contains(text, "\"ascii\": 0",
                    prefix + " should keep ASCII mode inert");
    expect_contains(text, "\"collateAvailable\": false",
                    prefix + " should keep collate unavailable");
    expect_contains(text, "\"collate\": 0",
                    prefix + " should keep collate inert");
    expect_contains(text, "\"columnSetupAvailable\": false",
                    prefix + " should keep column setup unavailable");
    expect_contains(text, "\"columnCountAvailable\": false",
                    prefix + " should keep column count unavailable");
    expect_contains(text, "\"columnCount\": 0",
                    prefix + " should keep column count inert");
    expect_contains(text, "\"columnWidthAvailable\": false",
                    prefix + " should keep column width unavailable");
    expect_contains(text, "\"columnWidth\": 0",
                    prefix + " should keep column width inert");
    expect_contains(text, "\"columnSpacingAvailable\": false",
                    prefix + " should keep column spacing unavailable");
    expect_contains(text, "\"columnSpacing\": 0",
                    prefix + " should keep column spacing inert");
    expect_contains(text, "\"sortExpressionAvailable\": false",
                    prefix + " should keep sort expression unavailable");
    expect_contains(text, "\"sortExpression\": \"\"",
                    prefix + " should keep sort expression empty");
}

void expect_supported_deleted_page_summary(const std::string& text, const std::string& prefix) {
    expect_unset_deleted_page_summary_fields(text, prefix);
    expect_contains(text, "\"pageSetupAvailable\": true",
                    prefix + " should expose effective deleted-root page setup");
    expect_contains(text, "\"orientationAvailable\": true",
                    prefix + " should expose orientation availability");
    expect_contains(text, "\"orientationCode\": 0",
                    prefix + " should expose the orientation code");
    expect_contains(text, "\"paperSizeAvailable\": true",
                    prefix + " should expose paper-size availability");
    expect_contains(text, "\"paperSizeCode\": 1",
                    prefix + " should expose the paper-size code");
    expect_contains(text, "\"topMarginAvailable\": true",
                    prefix + " should expose top-margin availability");
    expect_contains(text, "\"topMargin\": 10",
                    prefix + " should expose the top margin");
    expect_contains(text, "\"bottomMarginAvailable\": true",
                    prefix + " should expose bottom-margin availability");
    expect_contains(text, "\"bottomMargin\": 20",
                    prefix + " should expose the bottom margin");
    expect_contains(text, "\"gridVerticalAvailable\": true",
                    prefix + " should expose vertical-grid availability");
    expect_contains(text, "\"gridVertical\": 4",
                    prefix + " should expose vertical grid spacing");
    expect_contains(text, "\"gridHorizontalAvailable\": true",
                    prefix + " should expose horizontal-grid availability");
    expect_contains(text, "\"gridHorizontal\": 8",
                    prefix + " should expose horizontal grid spacing");
    expect_contains(text, "\"colorAvailable\": false",
                    prefix + " should keep color unavailable");
    expect_contains(text, "\"color\": 0",
                    prefix + " should keep color inert");
}

void expect_unsupported_deleted_page_summary(const std::string& text, const std::string& prefix) {
    expect_unset_deleted_page_summary_fields(text, prefix);
    expect_contains(text, "\"pageSetupAvailable\": true",
                    prefix + " should expose effective deleted-root page setup");
    expect_contains(text, "\"orientationAvailable\": true",
                    prefix + " should expose orientation availability");
    expect_contains(text, "\"orientationCode\": 0",
                    prefix + " should expose the orientation code");
    expect_contains(text, "\"paperSizeAvailable\": false",
                    prefix + " should keep paper size unavailable");
    expect_contains(text, "\"paperSizeCode\": 0",
                    prefix + " should keep the paper-size code inert");
    expect_contains(text, "\"topMarginAvailable\": true",
                    prefix + " should expose direct top-margin availability");
    expect_contains(text, "\"topMargin\": 10",
                    prefix + " should expose the direct top margin");
    expect_contains(text, "\"bottomMarginAvailable\": false",
                    prefix + " should keep bottom margin unavailable");
    expect_contains(text, "\"bottomMargin\": 0",
                    prefix + " should keep the bottom margin inert");
    expect_contains(text, "\"gridVerticalAvailable\": false",
                    prefix + " should keep vertical grid unavailable");
    expect_contains(text, "\"gridVertical\": 0",
                    prefix + " should keep vertical grid spacing inert");
    expect_contains(text, "\"gridHorizontalAvailable\": false",
                    prefix + " should keep horizontal grid unavailable");
    expect_contains(text, "\"gridHorizontal\": 0",
                    prefix + " should keep horizontal grid spacing inert");
    expect_contains(text, "\"colorAvailable\": true",
                    prefix + " should expose color availability");
    expect_contains(text, "\"color\": 1",
                    prefix + " should expose the color value");
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
    expect(create_result.ok, "#2795: synthetic report/label layout settings fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2795: synthetic report/label layout settings fixture should mark deleted objects");
}

void write_synthetic_report_table_for_stable_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(unique_id_result.ok, "#2795: stable settings fixture should seed a settings unique id");
    expect(!dbf_record_deleted(report_path, 0U),
           "#2795: stable settings fixture should preserve the live settings state");
}

void write_synthetic_report_table_for_stable_unsupported_expr_settings_json(
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
        {"1", "53", "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1", "", "", "", "", "", "10", "unsupported-settings-id"},
        {"9", "1", "", "", "0", "", "2000", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#3095: stable settings delete fixture should create unsupported EXPR layout");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#3095: stable settings delete fixture should mark deleted objects");
    expect(!dbf_record_deleted(report_path, 0U),
           "#3095: stable settings delete fixture should preserve the live settings state");
}

void run_settings_delete_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_settings_json(asset_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--delete-object",
            "--unique-id", "settings-guid",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable settings delete stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable settings delete stderr:\n"
                  << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(dbf_record_deleted(asset_path, 0U), issue_prefix + " should mark the settings record deleted");
    expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_full_report_layout_preview_bounds(delete_process.stdout_text, issue_prefix);
    expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                    issue_prefix + " should remove settings from live counts");
    expect_supported_deleted_page_summary(delete_process.stdout_text, issue_prefix);
    expect_settings_selection_metadata(delete_process.stdout_text, issue_prefix);
    expect_supported_deleted_setting_payloads(delete_process.stdout_text, issue_prefix);
    expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 6",
                    issue_prefix + " should expose deleted setting counts");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSettings\": [",
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
        issue_prefix + " should move root settings into deleted-setting metadata");
    expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains_in_order(
        delete_process.stdout_text,
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
        issue_prefix + " should expose selected deleted-settings metadata");
    expect_contains(delete_process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " should preserve live section metadata");
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object metadata");
}

void run_unsupported_expr_settings_delete_case(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_unsupported_expr_settings_json(asset_path);

    const auto delete_process = run_process_capture(
        studio_host_path,
        {
            "--path", asset_path.string(),
            "--delete-object",
            "--unique-id", "unsupported-settings-id",
            "--json"
        },
        temp_root);

    if (delete_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable unsupported EXPR settings delete stdout:\n"
                  << delete_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable unsupported EXPR settings delete stderr:\n"
                  << delete_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(delete_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect(dbf_record_deleted(asset_path, 0U), issue_prefix + " should mark the settings record deleted");
    expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    const auto expr_property = copperfin::vfp::query_visual_object_property({
        .path = asset_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "unsupported-settings-id",
        .property_name = "EXPR"
    });
    expect(expr_property.ok && expr_property.exists,
           issue_prefix + " should leave the EXPR memo queryable after delete");
    expect(normalize_line_endings(expr_property.value) ==
               "ORIENTATION=0\n* keep-this-comment\n\nXUSER=keepme\nCOLOR=1",
           issue_prefix + " should preserve raw unsupported EXPR lines across delete");
    expect_contains(delete_process.stdout_text, "\"settingCount\": 0",
                    issue_prefix + " should remove settings from live counts");
    expect_contains(delete_process.stdout_text, "\"deletedSettingCount\": 4",
                    issue_prefix + " should expose deleted key/value settings only");
    expect_contains(delete_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    issue_prefix + " should advertise selected-settings availability");
    expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    issue_prefix + " should expose settings selection kind");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"deletedSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " should preserve deleted parsed setting source-line gaps around unsupported EXPR lines");
    expect_contains_in_order(
        delete_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0",
            "\"name\": \"XUSER\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3",
            "\"name\": \"COLOR\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 4",
            "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null"
        },
        issue_prefix + " should expose selected deleted parsed settings after delete");
    expect_unsupported_deleted_page_summary(delete_process.stdout_text, issue_prefix);
    expect_settings_selection_metadata(delete_process.stdout_text, issue_prefix);
    expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object metadata");
    expect_contains(delete_process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " should preserve live section metadata");
    expect(delete_process.stdout_text.find("\"name\": \"* keep-this-comment\"") == std::string::npos,
           issue_prefix + " should not fabricate comment lines as deleted settings");
}

void test_studio_host_json_preserves_settings_delete_stable_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_settings_delete_stable_selection_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_settings_delete_case(
        studio_host_path,
        temp_root,
        "settings_delete_stable.frx",
        "report",
        "#2795: stable-selected report settings delete");
    run_settings_delete_case(
        studio_host_path,
        temp_root,
        "settings_delete_stable.lbx",
        "label",
        "#2795: stable-selected label settings delete");
    run_unsupported_expr_settings_delete_case(
        studio_host_path,
        temp_root,
        "settings_delete_unsupported_stable.frx",
        "report",
        "#3095: stable-selected report settings delete should preserve unsupported EXPR lines");
    run_unsupported_expr_settings_delete_case(
        studio_host_path,
        temp_root,
        "settings_delete_unsupported_stable.lbx",
        "label",
        "#3095: stable-selected label settings delete should preserve unsupported EXPR lines");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_settings_delete_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_settings_delete_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
