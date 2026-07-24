// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

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
        {.name = "UNIQUEID", .type = 'C', .length = 24U},
        {.name = "FLOAT", .type = 'L', .length = 1U},
        {.name = "NOREPEAT", .type = 'L', .length = 1U},
        {.name = "STRETCH", .type = 'L', .length = 1U},
        {.name = "STRETCHTOP", .type = 'L', .length = 1U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0\nPAPERSIZE=1\nBOTMARGIN=20\nGRIDV=4\nGRIDH=8", "", "", "", "", "", "10", "", "", "", "", ""},
        {"9", "1", "", "", "0", "", "2000", "", "", "", "", "", "", ""},
        {"9", "4", "", "", "2000", "", "5000", "", "", "", "", "", "", ""},
        {"8", "0", "customer.company", "1200", "2600", "4000", "450", "Segoe UI", "", "field-guid", "T", "F", "T", "F"},
        {"5", "", "\"Invoice\"", "900", "100", "1800", "350", "", "", "label-guid", "", "", "", ""},
        {"6", "", "", "50", "8000", "100", "100", "", "", "", "", "", "", ""},
        {"5", "", "\"Deleted label\"", "1000", "2600", "1200", "300", "", "", "", "F", "T", "F", "T"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2831: layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2831: layout fixture should mark deleted layout objects");
}

void test_studio_host_json_preserves_selected_report_objects(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host selected report object stdout:\n" << object_process.stdout_text << "\n";
        std::cerr << "studio host selected report object stderr:\n" << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1454: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1454: report object selections should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1457: report object selections should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1457: report object selections should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"selectedReportObject\": {",
                    "#1454: report object selections should expose selected-object JSON");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1964: selected report object JSON should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1964: selected report object JSON should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1964: selected report object JSON should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1964: selected report object JSON should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1964: selected report object JSON should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1964: selected report object JSON should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1964: selected report object JSON should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1964: selected report object JSON should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1964: selected report object JSON should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1964: selected report object JSON should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1964: selected report object JSON should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1964: selected report object JSON should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1964: selected report object JSON should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1964: selected report object JSON should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1509: selected report objects should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1509: selected report objects should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1509: selected report objects should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1509: selected report objects should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"recordIndex\": 3",
                    "#1454: selected report object JSON should expose the selected object record index");
    expect_contains(object_process.stdout_text, "\"objectTypeCode\": 8",
                    "#1454: selected report object JSON should expose raw report object type codes");
    expect_contains(object_process.stdout_text, "\"objectKind\": \"field\"",
                    "#1454: selected report object JSON should expose report object kind");
    expect_contains(object_process.stdout_text, "\"containingSectionId\": \"detail_2\"",
                    "#1458: selected report object JSON should expose containing section ids");
    expect_contains(object_process.stdout_text, "\"containingSectionRecordIndex\": 2",
                    "#1458: selected report object JSON should expose containing section record indexes");
    expect_contains(object_process.stdout_text, "\"sectionRelativeTop\": 600",
                    "#1458: selected report object JSON should expose top coordinates relative to containing sections");
    expect_contains(object_process.stdout_text, "\"sectionRelativeBottom\": 1050",
                    "#1461: selected report object JSON should expose bottom coordinates relative to containing sections");
    expect_contains(object_process.stdout_text, "\"sectionObjectIndex\": 0",
                    "#1459: selected report object JSON should expose object order inside containing sections");
    expect_contains(object_process.stdout_text, "\"sectionObjectCount\": 1",
                    "#1459: selected report object JSON should expose containing section object counts");
    expect_contains(object_process.stdout_text, "\"expression\": \"customer.company\"",
                    "#1454: selected report object JSON should expose report object expressions");
    expect_contains(object_process.stdout_text, "\"expressionFieldIndex\": 2",
                    "#1454: selected report object JSON should expose expression field provenance");
    expect_contains(object_process.stdout_text, "\"right\": 5200",
                    "#1462: selected report object JSON should expose object right-edge coordinates");
    expect_contains(object_process.stdout_text, "\"bottom\": 3050",
                    "#1461: selected report object JSON should expose object bottom-edge coordinates");
    expect_contains(object_process.stdout_text, "\"highlightCount\": 6",
                    "#1454: selected report object JSON should expose highlight counts");
    expect_contains(object_process.stdout_text,
                    "\"name\": \"FLOAT\", \"recordIndex\": 3, \"fieldIndex\": 10, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"true\"",
                    "#4554: selected report object JSON should expose FLOAT highlight provenance");
    expect_contains(object_process.stdout_text,
                    "\"name\": \"NOREPEAT\", \"recordIndex\": 3, \"fieldIndex\": 11, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"false\"",
                    "#4554: selected report object JSON should expose NOREPEAT highlight provenance");
    expect_contains(object_process.stdout_text,
                    "\"name\": \"STRETCH\", \"recordIndex\": 3, \"fieldIndex\": 12, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"true\"",
                    "#4554: selected report object JSON should expose STRETCH highlight provenance");
    expect_contains(object_process.stdout_text,
                    "\"name\": \"STRETCHTOP\", \"recordIndex\": 3, \"fieldIndex\": 13, \"sourceLineIndex\": null, \"memoBlockNumber\": 0, \"value\": \"false\"",
                    "#4554: selected report object JSON should expose STRETCHTOP highlight provenance");
    expect_contains(object_process.stdout_text, "\"name\": \"FONTFACE\", \"recordIndex\": 3, \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": 3, \"value\": \"Segoe UI\"",
                    "#1454: selected report object JSON should expose highlight provenance");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1455: section-contained report objects should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1455: section-contained report objects should expose containing-section JSON");
    expect_contains(object_process.stdout_text, "\"id\": \"detail_2\"",
                    "#1455: containing-section JSON should expose selected object section ids");
    expect_contains(object_process.stdout_text, "\"bandKind\": \"detail\"",
                    "#1455: containing-section JSON should expose selected object band kinds");

    const auto page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "4", "--json"},
        temp_root);

    if (page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected page-header report object stdout:\n"
                  << page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected page-header report object stderr:\n"
                  << page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(page_header_object_process.exit_code == 0,
           "#1972: selected page-header report object JSON should exit successfully");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1972: page-header report object selections should advertise selected-object availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1972: page-header report object selections should advertise report-selection availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1972: page-header report object selections should expose object selection kind");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1972: selected page-header report object JSON should expose live preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1972: selected page-header report object JSON should preserve live preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1972: selected page-header report object JSON should preserve live preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1972: selected page-header report object JSON should preserve live preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1972: selected page-header report object JSON should preserve live preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1972: selected page-header report object JSON should preserve live preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1972: selected page-header report object JSON should preserve live preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1972: selected page-header report object JSON should expose deleted preview availability");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1972: selected page-header report object JSON should preserve deleted preview left bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1972: selected page-header report object JSON should preserve deleted preview top bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1972: selected page-header report object JSON should preserve deleted preview right bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1972: selected page-header report object JSON should preserve deleted preview bottom bounds");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1972: selected page-header report object JSON should preserve deleted preview widths");
    expect_contains(page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1972: selected page-header report object JSON should preserve deleted preview heights");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1972: selected page-header report objects should not advertise selected-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1972: selected page-header report objects should serialize null selected sections");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1972: selected page-header report objects should not advertise selected-settings availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1972: selected page-header report objects should serialize null selected settings");
    expect_contains_in_order(
        page_header_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": false",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1972: page-header report object selections should expose selected-object metadata");
    expect_contains(page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1972: selected page-header report object JSON should expose object right-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1972: selected page-header report object JSON should expose object bottom-edge coordinates");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1972: page-header report objects should advertise containing-section availability");
    expect_contains(page_header_object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    "#1972: page-header report objects should expose containing-section JSON");
    expect_contains(page_header_object_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1972: containing-section JSON should expose selected page-header object section ids");
    expect_contains(page_header_object_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#1972: containing-section JSON should expose selected page-header object band kinds");

    const fs::path deleted_page_header_object_path = temp_root / "deleted_page_header_object.frx";
    write_synthetic_report_table_for_layout_json(deleted_page_header_object_path);
    const auto delete_page_header_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_page_header_object_path.string(), 4U, true);
    expect(delete_page_header_object_result.ok,
           "#1974: selected deleted page-header report object fixture should mark the page-header object deleted");

    const auto deleted_page_header_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_page_header_object_path.string(), "--record", "4", "--json"},
        temp_root);

    if (deleted_page_header_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted page-header report object stdout:\n"
                  << deleted_page_header_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted page-header report object stderr:\n"
                  << deleted_page_header_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_page_header_object_process.exit_code == 0,
           "#1974: selected deleted page-header report object JSON should exit successfully");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1974: deleted page-header report object selections should advertise selected-object availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1974: deleted page-header report object selections should advertise report-selection availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1974: deleted page-header report object selections should expose object selection kind");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1974: selected deleted page-header report object JSON should preserve live preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1974: selected deleted page-header report object JSON should preserve live preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1974: selected deleted page-header report object JSON should preserve live preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1974: selected deleted page-header report object JSON should preserve live preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1974: selected deleted page-header report object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1974: selected deleted page-header report object JSON should preserve live preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1974: selected deleted page-header report object JSON should preserve live preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1974: selected deleted page-header report object JSON should expose deleted preview availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview left bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview top bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview right bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1974: selected deleted page-header report object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1800",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview widths");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2800",
                    "#1974: selected deleted page-header report object JSON should expand deleted preview heights");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1974: selected deleted page-header report objects should not advertise selected-section availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1974: selected deleted page-header report objects should serialize null selected sections");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1974: selected deleted page-header report objects should not advertise selected-settings availability");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1974: selected deleted page-header report objects should serialize null selected settings");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should summarize remaining live objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should summarize deleted objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                    "#1974: selected deleted page-header report object JSON should count deleted placed objects");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1974: selected deleted page-header report object JSON should not fabricate deleted unplaced objects");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"containingSectionId\": \"page_header_1\"",
            "\"containingSectionRecordIndex\": 1",
            "\"sectionRelativeTop\": 100",
            "\"sectionRelativeBottom\": 450",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"objectTypeCode\": 5",
            "\"objectKind\": \"label\"",
            "\"expression\": \"\\\"Invoice\\\"\""
        },
        "#1974: deleted page-header report object selections should expose selected-object metadata with containing-section membership");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"right\": 2700",
                    "#1974: selected deleted page-header report object JSON should expose object right-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"bottom\": 450",
                    "#1974: selected deleted page-header report object JSON should expose object bottom-edge coordinates");
    expect_contains(deleted_page_header_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1974: deleted page-header report objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_page_header_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        "#1974: deleted page-header report objects should expose live containing-section JSON");

    const auto deleted_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "6", "--json"},
        temp_root);

    if (deleted_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report object stdout:\n"
                  << deleted_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report object stderr:\n"
                  << deleted_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_object_process.exit_code == 0,
           "#1479: selected deleted report object JSON should exit successfully");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1479: deleted report object selections should advertise selected-object availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1479: deleted report object selections should advertise report-selection availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1479: deleted report object selections should expose object selection kind");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1965: selected deleted report object JSON should preserve live preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1965: selected deleted report object JSON should preserve live preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1965: selected deleted report object JSON should preserve live preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1965: selected deleted report object JSON should preserve live preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1965: selected deleted report object JSON should preserve live preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1965: selected deleted report object JSON should preserve live preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1965: selected deleted report object JSON should preserve live preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1965: selected deleted report object JSON should expose deleted preview availability");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1965: selected deleted report object JSON should preserve deleted preview left bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1965: selected deleted report object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1965: selected deleted report object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1965: selected deleted report object JSON should preserve deleted preview bottom bounds");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1965: selected deleted report object JSON should preserve deleted preview widths");
    expect_contains(deleted_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1965: selected deleted report object JSON should preserve deleted preview heights");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1510: selected deleted report objects should not advertise selected-section availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1510: selected deleted report objects should serialize null selected sections");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1510: selected deleted report objects should not advertise selected-settings availability");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1510: selected deleted report objects should serialize null selected settings");
    expect_contains(deleted_object_process.stdout_text, "\"deletedObjectCount\": 1",
                    "#1479: deleted selected report object JSON should expose deleted object counts");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 6",
            "\"deleted\": true",
            "\"containingSectionId\": \"detail_2\"",
            "\"containingSectionRecordIndex\": 2",
            "\"sectionRelativeTop\": 600",
            "\"sectionRelativeBottom\": 900",
            "\"sectionObjectIndex\": 0",
            "\"sectionObjectCount\": 1",
            "\"expression\": \"\\\"Deleted label\\\"\""
        },
        "#1479: deleted report object selections should expose deleted selected-object metadata");
    expect_contains(deleted_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    "#1479: deleted report objects should advertise selected containing-section availability");
    expect_contains_in_order(
        deleted_object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"detail_2\"",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"deleted\": false"
        },
        "#1479: deleted report objects should expose live containing-section JSON");

    const auto unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "5", "--json"},
        temp_root);

    if (unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected unplaced report object stdout:\n"
                  << unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected unplaced report object stderr:\n"
                  << unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(unplaced_object_process.exit_code == 0,
           "#1480: selected unplaced report object JSON should exit successfully");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1480: unplaced report object selections should advertise selected-object availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1480: unplaced report object selections should advertise report-selection availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1480: unplaced report object selections should expose object selection kind");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1968: selected unplaced report object JSON should expose live preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1968: selected unplaced report object JSON should preserve live preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1968: selected unplaced report object JSON should preserve live preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1968: selected unplaced report object JSON should preserve live preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1968: selected unplaced report object JSON should preserve live preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1968: selected unplaced report object JSON should preserve live preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1968: selected unplaced report object JSON should preserve live preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1968: selected unplaced report object JSON should expose deleted preview availability");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1968: selected unplaced report object JSON should preserve deleted preview left bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1968: selected unplaced report object JSON should preserve deleted preview top bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1968: selected unplaced report object JSON should preserve deleted preview right bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1968: selected unplaced report object JSON should preserve deleted preview bottom bounds");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1968: selected unplaced report object JSON should preserve deleted preview widths");
    expect_contains(unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1968: selected unplaced report object JSON should preserve deleted preview heights");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1511: selected unplaced report objects should not advertise selected-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1511: selected unplaced report objects should serialize null selected sections");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1511: selected unplaced report objects should not advertise selected-settings availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1511: selected unplaced report objects should serialize null selected settings");
    expect_contains(unplaced_object_process.stdout_text, "\"unplacedObjectCount\": 1",
                    "#1480: unplaced selected report object JSON should expose unplaced object counts");
    expect_contains_in_order(
        unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": false",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectKind\": \"line\""
        },
        "#1480: unplaced report object selections should expose selected-object metadata without section membership");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1480: unplaced report objects should not advertise selected containing-section availability");
    expect_contains(unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1480: unplaced report objects should serialize null selected containing-section JSON");

    const fs::path deleted_unplaced_object_path = temp_root / "deleted_unplaced_object.frx";
    write_synthetic_report_table_for_layout_json(deleted_unplaced_object_path);
    const auto delete_unplaced_object_result =
        copperfin::vfp::set_record_deleted_flag(deleted_unplaced_object_path.string(), 5U, true);
    expect(delete_unplaced_object_result.ok,
           "#1970: selected deleted unplaced report object fixture should mark the unplaced object deleted");

    const auto deleted_unplaced_object_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_unplaced_object_path.string(), "--record", "5", "--json"},
        temp_root);

    if (deleted_unplaced_object_process.exit_code != 0) {
        std::cerr << "studio host selected deleted unplaced report object stdout:\n"
                  << deleted_unplaced_object_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted unplaced report object stderr:\n"
                  << deleted_unplaced_object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_unplaced_object_process.exit_code == 0,
           "#1970: selected deleted unplaced report object JSON should exit successfully");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    "#1970: deleted unplaced report object selections should advertise selected-object availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1970: deleted unplaced report object selections should advertise report-selection availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    "#1970: deleted unplaced report object selections should expose object selection kind");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsBottom\": 7000",
                    "#1970: selected deleted unplaced report object JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1970: selected deleted unplaced report object JSON should preserve live preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"previewBoundsHeight\": 7000",
                    "#1970: selected deleted unplaced report object JSON should preserve remaining live preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1970: selected deleted unplaced report object JSON should expose deleted preview availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 50",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1970: selected deleted unplaced report object JSON should preserve deleted preview top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1970: selected deleted unplaced report object JSON should preserve deleted preview right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 8100",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview bottom bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2150",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview widths");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5500",
                    "#1970: selected deleted unplaced report object JSON should expand deleted preview heights");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1970: selected deleted unplaced report objects should not advertise selected-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1970: selected deleted unplaced report objects should serialize null selected sections");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1970: selected deleted unplaced report objects should not advertise selected-settings availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1970: selected deleted unplaced report objects should serialize null selected settings");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"liveObjectCount\": 2",
                    "#1970: selected deleted unplaced report object JSON should summarize remaining live objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedObjectCount\": 2",
                    "#1970: selected deleted unplaced report object JSON should summarize deleted objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                    "#1970: selected deleted unplaced report object JSON should retain deleted placed object counts");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                    "#1970: selected deleted unplaced report object JSON should count deleted unplaced objects");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"left\": 50",
                    "#1970: deleted unplaced report object selections should expose selected-object left bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"top\": 8000",
                    "#1970: deleted unplaced report object selections should expose selected-object top bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"right\": 150",
                    "#1970: deleted unplaced report object selections should expose selected-object right bounds");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"bottom\": 8100",
                    "#1970: deleted unplaced report object selections should expose selected-object bottom bounds");
    expect_contains_in_order(
        deleted_unplaced_object_process.stdout_text,
        {
            "\"selectedReportObject\": {",
            "\"recordIndex\": 5",
            "\"deleted\": true",
            "\"containingSectionId\": \"\"",
            "\"containingSectionRecordIndex\": null",
            "\"sectionObjectIndex\": null",
            "\"sectionObjectCount\": 0",
            "\"objectKind\": \"line\""
        },
        "#1970: deleted unplaced report object selections should expose selected-object metadata without section membership");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1970: deleted unplaced report objects should not advertise selected containing-section availability");
    expect_contains(deleted_unplaced_object_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1970: deleted unplaced report objects should serialize null selected containing-section JSON");

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host selected report section stdout:\n" << section_process.stdout_text << "\n";
        std::cerr << "studio host selected report section stderr:\n" << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1454: selected report section JSON smoke should exit successfully");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1454: non-object report selections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1454: non-object report selections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1455: non-object report selections should not advertise containing-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1455: non-object report selections should serialize null containing sections");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_report_objects <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_report_objects(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
