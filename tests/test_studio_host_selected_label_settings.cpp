// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "copperfin/vfp/dbf_table.h"
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
        {.name = "TAG", .type = 'M', .length = 4U},
        {.name = "LEFTMARGIN", .type = 'N', .length = 10U},
        {.name = "RIGHTMARGI", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nPAPERLENGTH=2794\nPAPERWIDTH=2159\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8\nDEFAULTSOURCE=15\nPRINTQUALITY=600\nYRESOLUTION=600\nTTOPTION=3\nASCII=9\nCOLLATE=1", "", "", "", "", "", "10", "customer.country", "15", "25", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "", "", "", "field-guid"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "", "", "", "label-guid"},
        {"6", "", "", "50", "8000", "100", "100", "", "", "", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", "", "", "", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2833: layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2833: layout fixture should mark deleted layout objects");
}

void write_synthetic_report_table_for_deleted_settings_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(delete_result.ok, "#2833: synthetic report table should mark settings deleted");
}

void test_studio_host_json_preserves_selected_label_settings(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_label_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path label_path = temp_root / "mailing.lbx";
    write_synthetic_report_table_for_layout_json(label_path);

    const auto settings_process = run_process_capture(
        studio_host_path,
        {"--path", label_path.string(), "--record", "0", "--json"},
        temp_root);

    if (settings_process.exit_code != 0) {
        std::cerr << "studio host selected label settings stdout:\n" << settings_process.stdout_text << "\n";
        std::cerr << "studio host selected label settings stderr:\n" << settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(settings_process.exit_code == 0,
           "#1496: selected label settings JSON should exit successfully");
    expect_contains(settings_process.stdout_text, "\"isLabel\": true",
                    "#1496: selected label settings JSON should retain label identity");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1496: label root selections should advertise selected-settings availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1496: label settings selections should advertise report-selection availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1496: label settings selections should expose settings selection kind");
    expect_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                    "#1496: label root selections should expose selected-settings JSON");
    expect_contains(settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1958: selected label settings JSON should expose live preview availability");
    expect_contains(settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1958: selected label settings JSON should preserve live preview left bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1958: selected label settings JSON should preserve live preview top bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1958: selected label settings JSON should preserve live preview right bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1958: selected label settings JSON should preserve live preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1958: selected label settings JSON should preserve live preview widths");
    expect_contains(settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1958: selected label settings JSON should preserve live preview heights");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1958: selected label settings JSON should expose deleted preview availability");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1958: selected label settings JSON should preserve deleted preview left bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1958: selected label settings JSON should preserve deleted preview top bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1958: selected label settings JSON should preserve deleted preview right bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1958: selected label settings JSON should preserve deleted preview bottom bounds");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1958: selected label settings JSON should preserve deleted preview widths");
    expect_contains(settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1958: selected label settings JSON should preserve deleted preview heights");
    expect_contains(settings_process.stdout_text, "\"leftMarginAvailable\": true",
                    "#3742: selected label settings JSON should expose left-margin availability");
    expect_contains(settings_process.stdout_text, "\"leftMargin\": 15",
                    "#3742: selected label settings JSON should preserve left-margin values");
    expect_contains(settings_process.stdout_text, "\"rightMarginAvailable\": true",
                    "#3742: selected label settings JSON should expose right-margin availability");
    expect_contains(settings_process.stdout_text, "\"rightMargin\": 25",
                    "#3742: selected label settings JSON should preserve right-margin values");
    expect_contains(settings_process.stdout_text, "\"paperLengthAvailable\": true",
                    "#3744: selected label settings JSON should expose paper-length availability");
    expect_contains(settings_process.stdout_text, "\"paperLength\": 2794",
                    "#3744: selected label settings JSON should preserve paper-length values");
    expect_contains(settings_process.stdout_text, "\"paperWidthAvailable\": true",
                    "#3744: selected label settings JSON should expose paper-width availability");
    expect_contains(settings_process.stdout_text, "\"paperWidth\": 2159",
                    "#3744: selected label settings JSON should preserve paper-width values");
    expect_contains(settings_process.stdout_text, "\"sortExpressionAvailable\": true",
                    "#3745: selected label settings JSON should expose sort-expression availability");
    expect_contains(settings_process.stdout_text, "\"sortExpression\": \"customer.country\"",
                    "#3745: selected label settings JSON should preserve sort-expression values");
    expect_contains(settings_process.stdout_text, "\"defaultSourceAvailable\": true",
                    "#3818: selected label settings JSON should expose default-source availability");
    expect_contains(settings_process.stdout_text, "\"defaultSource\": 15",
                    "#3818: selected label settings JSON should preserve default-source values");
    expect_contains(settings_process.stdout_text, "\"printQualityAvailable\": true",
                    "#3818: selected label settings JSON should expose print-quality availability");
    expect_contains(settings_process.stdout_text, "\"printQuality\": 600",
                    "#3818: selected label settings JSON should preserve print-quality values");
    expect_contains(settings_process.stdout_text, "\"yResolutionAvailable\": true",
                    "#3818: selected label settings JSON should expose y-resolution availability");
    expect_contains(settings_process.stdout_text, "\"yResolution\": 600",
                    "#3818: selected label settings JSON should preserve y-resolution values");
    expect_contains(settings_process.stdout_text, "\"trueTypeOptionAvailable\": true",
                    "#3818: selected label settings JSON should expose TrueType-option availability");
    expect_contains(settings_process.stdout_text, "\"trueTypeOption\": 3",
                    "#3818: selected label settings JSON should preserve TrueType-option values");
    expect_contains(settings_process.stdout_text, "\"asciiAvailable\": true",
                    "#3818: selected label settings JSON should expose ASCII availability");
    expect_contains(settings_process.stdout_text, "\"ascii\": 9",
                    "#3818: selected label settings JSON should preserve ASCII values");
    expect_contains(settings_process.stdout_text, "\"collateAvailable\": true",
                    "#3818: selected label settings JSON should expose COLLATE availability");
    expect_contains(settings_process.stdout_text, "\"collate\": 1",
                    "#3818: selected label settings JSON should preserve COLLATE values");
    expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1504: selected label settings should not advertise selected-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1504: selected label settings should serialize null selected sections");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1504: selected label settings should not advertise selected-object availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1504: selected label settings should serialize null selected objects");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1504: selected label settings should not advertise containing-object-section availability");
    expect_contains(settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1504: selected label settings should serialize null containing-object sections");
    expect_contains(settings_process.stdout_text, "\"name\": \"ORIENTATION\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 0, \"memoBlockNumber\": 1, \"value\": \"0\"",
                    "#1496: selected label settings should expose memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"PAPERSIZE\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 1, \"memoBlockNumber\": 1, \"value\": \"1\"",
                    "#1496: selected label settings should expose later memo-line setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"PAPERLENGTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 2, \"memoBlockNumber\": 1, \"value\": \"2794\"",
                    "#3744: selected label settings should expose memo-line paper-length provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"PAPERWIDTH\", \"recordIndex\": 0, \"fieldIndex\": 2, \"sourceLineIndex\": 3, \"memoBlockNumber\": 1, \"value\": \"2159\"",
                    "#3744: selected label settings should expose memo-line paper-width provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"TOPMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 8, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"10\"",
                    "#1496: selected label settings should expose direct setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"TAG\", \"recordIndex\": 0, \"fieldIndex\": 9, \"sourceLineIndex\": null, \"memoBlockNumber\": 2, \"value\": \"customer.country\"",
                    "#2908: selected label settings should expose direct TAG sort-setting provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"LEFTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 10, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"15\"",
                    "#3014: selected label settings should expose direct LEFTMARGIN provenance");
    expect_contains(settings_process.stdout_text, "\"name\": \"RIGHTMARGIN\", \"recordIndex\": 0, \"fieldIndex\": 11, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"25\"",
                    "#3014: selected label settings should expose direct RIGHTMARGIN provenance");
    expect_contains(settings_process.stdout_text, "\"sectionCount\": 2",
                    "#1496: selected label settings JSON should preserve live section metadata");
    expect_contains(settings_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1496: selected label settings JSON should preserve unplaced layout object metadata");
    expect_contains(settings_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1496: selected label settings JSON should preserve deleted layout object metadata");

    const fs::path deleted_settings_path = temp_root / "deleted_settings.lbx";
    write_synthetic_report_table_for_deleted_settings_json(deleted_settings_path);
    const auto deleted_settings_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_settings_path.string(), "--record", "0", "--json"},
        temp_root);

    if (deleted_settings_process.exit_code != 0) {
        std::cerr << "studio host selected deleted label settings stdout:\n"
                  << deleted_settings_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted label settings stderr:\n"
                  << deleted_settings_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_settings_process.exit_code == 0,
           "#1497: selected deleted label settings JSON should exit successfully");
    expect_contains(deleted_settings_process.stdout_text, "\"isLabel\": true",
                    "#1497: selected deleted label settings JSON should retain label identity");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                    "#1497: deleted label settings selections should advertise selected-settings availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1497: deleted label settings selections should advertise report-selection availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSelectionKind\": \"settings\"",
                    "#1497: deleted label settings selections should expose settings selection kind");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1959: selected deleted label settings JSON should expose live preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1959: selected deleted label settings JSON should preserve live preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1959: selected deleted label settings JSON should preserve live preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1959: selected deleted label settings JSON should preserve live preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1959: selected deleted label settings JSON should preserve live preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1959: selected deleted label settings JSON should preserve live preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1959: selected deleted label settings JSON should preserve live preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1959: selected deleted label settings JSON should expose deleted preview availability");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1959: selected deleted label settings JSON should preserve deleted preview left bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1959: selected deleted label settings JSON should preserve deleted preview top bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1959: selected deleted label settings JSON should preserve deleted preview right bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1959: selected deleted label settings JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1959: selected deleted label settings JSON should preserve deleted preview widths");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1959: selected deleted label settings JSON should preserve deleted preview heights");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1505: selected deleted label settings should not advertise selected-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportSection\": null",
                    "#1505: selected deleted label settings should serialize null selected sections");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1505: selected deleted label settings should not advertise selected-object availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObject\": null",
                    "#1505: selected deleted label settings should serialize null selected objects");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1505: selected deleted label settings should not advertise containing-object-section availability");
    expect_contains(deleted_settings_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1505: selected deleted label settings should serialize null containing-object sections");
    expect_contains(deleted_settings_process.stdout_text, "\"settingCount\": 0",
                    "#1497: deleted selected label settings JSON should not expose live settings");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedSettingCount\": 17",
                    "#1497: deleted selected label settings JSON should expose deleted setting counts");
    expect_contains(deleted_settings_process.stdout_text, "\"pageSetupAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback page-setup availability");
    expect_contains(deleted_settings_process.stdout_text, "\"orientationAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback orientation availability");
    expect_contains(deleted_settings_process.stdout_text, "\"orientationCode\": 0",
                    "#3815: deleted selected label settings JSON should surface fallback orientation values");
    expect_contains(deleted_settings_process.stdout_text, "\"paperSizeAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback paper-size availability");
    expect_contains(deleted_settings_process.stdout_text, "\"paperSizeCode\": 1",
                    "#3815: deleted selected label settings JSON should surface fallback paper-size values");
    expect_contains(deleted_settings_process.stdout_text, "\"paperLengthAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback paper-length availability");
    expect_contains(deleted_settings_process.stdout_text, "\"paperLength\": 2794",
                    "#3815: deleted selected label settings JSON should surface fallback paper-length values");
    expect_contains(deleted_settings_process.stdout_text, "\"paperWidthAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback paper-width availability");
    expect_contains(deleted_settings_process.stdout_text, "\"paperWidth\": 2159",
                    "#3815: deleted selected label settings JSON should surface fallback paper-width values");
    expect_contains(deleted_settings_process.stdout_text, "\"topMarginAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback top-margin availability");
    expect_contains(deleted_settings_process.stdout_text, "\"topMargin\": 10",
                    "#3815: deleted selected label settings JSON should surface fallback top-margin values");
    expect_contains(deleted_settings_process.stdout_text, "\"bottomMarginAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback bottom-margin availability");
    expect_contains(deleted_settings_process.stdout_text, "\"bottomMargin\": 20",
                    "#3815: deleted selected label settings JSON should surface fallback bottom-margin values");
    expect_contains(deleted_settings_process.stdout_text, "\"leftMarginAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback left-margin availability");
    expect_contains(deleted_settings_process.stdout_text, "\"leftMargin\": 15",
                    "#3815: deleted selected label settings JSON should surface fallback left-margin values");
    expect_contains(deleted_settings_process.stdout_text, "\"rightMarginAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback right-margin availability");
    expect_contains(deleted_settings_process.stdout_text, "\"rightMargin\": 25",
                    "#3815: deleted selected label settings JSON should surface fallback right-margin values");
    expect_contains(deleted_settings_process.stdout_text, "\"gridVerticalAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback vertical-grid availability");
    expect_contains(deleted_settings_process.stdout_text, "\"gridVertical\": 4",
                    "#3815: deleted selected label settings JSON should surface fallback vertical-grid values");
    expect_contains(deleted_settings_process.stdout_text, "\"gridHorizontalAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback horizontal-grid availability");
    expect_contains(deleted_settings_process.stdout_text, "\"gridHorizontal\": 8",
                    "#3815: deleted selected label settings JSON should surface fallback horizontal-grid values");
    expect_contains(deleted_settings_process.stdout_text, "\"sortExpressionAvailable\": true",
                    "#3815: deleted selected label settings JSON should surface fallback sort-expression availability");
    expect_contains(deleted_settings_process.stdout_text, "\"sortExpression\": \"customer.country\"",
                    "#3815: deleted selected label settings JSON should surface fallback sort-expression values");
    expect_contains(deleted_settings_process.stdout_text, "\"defaultSourceAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback default-source availability");
    expect_contains(deleted_settings_process.stdout_text, "\"defaultSource\": 15",
                    "#3818: deleted selected label settings JSON should surface fallback default-source values");
    expect_contains(deleted_settings_process.stdout_text, "\"printQualityAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback print-quality availability");
    expect_contains(deleted_settings_process.stdout_text, "\"printQuality\": 600",
                    "#3818: deleted selected label settings JSON should surface fallback print-quality values");
    expect_contains(deleted_settings_process.stdout_text, "\"yResolutionAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback y-resolution availability");
    expect_contains(deleted_settings_process.stdout_text, "\"yResolution\": 600",
                    "#3818: deleted selected label settings JSON should surface fallback y-resolution values");
    expect_contains(deleted_settings_process.stdout_text, "\"trueTypeOptionAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback TrueType-option availability");
    expect_contains(deleted_settings_process.stdout_text, "\"trueTypeOption\": 3",
                    "#3818: deleted selected label settings JSON should surface fallback TrueType-option values");
    expect_contains(deleted_settings_process.stdout_text, "\"asciiAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback ASCII availability");
    expect_contains(deleted_settings_process.stdout_text, "\"ascii\": 9",
                    "#3818: deleted selected label settings JSON should surface fallback ASCII values");
    expect_contains(deleted_settings_process.stdout_text, "\"collateAvailable\": true",
                    "#3818: deleted selected label settings JSON should surface fallback COLLATE availability");
    expect_contains(deleted_settings_process.stdout_text, "\"collate\": 1",
                    "#3818: deleted selected label settings JSON should surface fallback COLLATE values");
    expect_contains_in_order(
        deleted_settings_process.stdout_text,
        {
            "\"selectedReportSettings\": [",
            "\"name\": \"ORIENTATION\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERSIZE\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERLENGTH\"",
            "\"recordIndex\": 0",
            "\"name\": \"PAPERWIDTH\"",
            "\"recordIndex\": 0",
            "\"name\": \"BOTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDV\"",
            "\"recordIndex\": 0",
            "\"name\": \"GRIDH\"",
            "\"recordIndex\": 0",
            "\"name\": \"TOPMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"LEFTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"RIGHTMARGIN\"",
            "\"recordIndex\": 0",
            "\"name\": \"TAG\"",
            "\"recordIndex\": 0"
        },
        "#3014: deleted label settings selections should expose selected deleted side-margin and TAG provenance");
    expect_contains(deleted_settings_process.stdout_text, "\"sectionCount\": 2",
                    "#1497: deleted selected label settings JSON should preserve live section metadata");
    expect_contains(deleted_settings_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1497: deleted selected label settings JSON should preserve deleted object metadata");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_label_settings <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_label_settings(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
