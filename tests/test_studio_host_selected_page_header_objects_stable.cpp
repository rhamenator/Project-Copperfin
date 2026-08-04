// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

using copperfin::test_support::ScopedDefaultLocaleCatalogEnvironment;

int failures = 0;

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

bool dbf_record_deleted(const std::filesystem::path& table_path, std::size_t record_index) {
    const auto table_result =
        copperfin::vfp::parse_dbf_table_from_file(table_path.string(), record_index + 1U);
    if (!table_result.ok || record_index >= table_result.table.records.size()) {
        return false;
    }
    return table_result.table.records[record_index].deleted;
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
    expect(create_result.ok, "#2825: layout fixture should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 6U, true);
    expect(delete_result.ok, "#2825: layout fixture should mark deleted layout objects");
}

void run_page_header_object_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_json(asset_path);

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected page-header object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected page-header object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                    issue_prefix + " should preserve live object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                    issue_prefix + " should preserve deleted object counts");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should preserve deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " should advertise containing-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSection\": {",
                    issue_prefix + " should expose containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
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
            "\"expression\": \"\\\"Invoice\\\"\"",
            "\"expressionFieldIndex\": 2",
            "\"highlightCount\": 1"
        },
        issue_prefix + " should expose selected object metadata and provenance");
    expect_contains(object_process.stdout_text, "\"left\": 900",
                    issue_prefix + " should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 100",
                    issue_prefix + " should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"width\": 1800",
                    issue_prefix + " should expose selected-object widths");
    expect_contains(object_process.stdout_text, "\"right\": 2700",
                    issue_prefix + " should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"height\": 350",
                    issue_prefix + " should expose selected-object heights");
    expect_contains(object_process.stdout_text, "\"bottom\": 450",
                    issue_prefix + " should expose selected-object bottom bounds");
    expect_contains(
        object_process.stdout_text,
        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
        issue_prefix + " should expose selected-object expression provenance");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        issue_prefix + " should expose containing page-header metadata");
}

void run_deleted_page_header_object_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_layout_json(asset_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 4U, true);
    expect(delete_result.ok && dbf_record_deleted(asset_path, 4U),
           issue_prefix + " should mark the object deleted");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable selected deleted page-header object stdout:\n"
                  << object_process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable selected deleted page-header object stderr:\n"
                  << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should return refreshed report-layout JSON");
    if (asset_path.extension() == ".lbx") {
        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                    issue_prefix + " should advertise selected-object availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                    issue_prefix + " should expose object selection kind");
    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 2",
                    issue_prefix + " should update live object counts");
    expect_contains(object_process.stdout_text, "\"placedObjectCount\": 1",
                    issue_prefix + " should update live placed object counts");
    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 2",
                    issue_prefix + " should update deleted object counts");
    expect_contains(object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                    issue_prefix + " should expose deleted placed object counts");
    expect_contains(object_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    issue_prefix + " should preserve deleted unplaced object counts");
    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 900",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 100",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1800",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2800",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    issue_prefix + " should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    issue_prefix + " should serialize null selected sections");
    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                    issue_prefix + " should advertise containing-section availability");
    expect_contains_in_order(
        object_process.stdout_text,
        {
            "\"selectedReportObjectSection\": {",
            "\"id\": \"page_header_1\"",
            "\"bandKind\": \"page_header\"",
            "\"recordIndex\": 1",
            "\"deleted\": false"
        },
        issue_prefix + " should expose live containing-section JSON");
    expect_contains_in_order(
        object_process.stdout_text,
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
            "\"expression\": \"\\\"Invoice\\\"\"",
            "\"expressionFieldIndex\": 2"
        },
        issue_prefix + " should expose selected object metadata with containing-section membership");
    expect_contains(object_process.stdout_text, "\"left\": 900",
                    issue_prefix + " should expose selected-object left bounds");
    expect_contains(object_process.stdout_text, "\"top\": 100",
                    issue_prefix + " should expose selected-object top bounds");
    expect_contains(object_process.stdout_text, "\"width\": 1800",
                    issue_prefix + " should expose selected-object widths");
    expect_contains(object_process.stdout_text, "\"right\": 2700",
                    issue_prefix + " should expose selected-object right bounds");
    expect_contains(object_process.stdout_text, "\"height\": 350",
                    issue_prefix + " should expose selected-object heights");
    expect_contains(object_process.stdout_text, "\"bottom\": 450",
                    issue_prefix + " should expose selected-object bottom bounds");
    expect_contains(
        object_process.stdout_text,
        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
        issue_prefix + " should expose selected-object expression provenance");
}

void test_studio_host_json_preserves_selected_page_header_objects_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_page_header_objects_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_page_header_object_stable.frx",
        "report",
        "#2825: stable live report page-header object selection");
    run_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_page_header_object_stable.lbx",
        "label",
        "#2825: stable live label page-header object selection");
    run_deleted_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_header_object_stable.frx",
        "report",
        "#2825: stable deleted report page-header object selection");
    run_deleted_page_header_object_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_page_header_object_stable.lbx",
        "label",
        "#2825: stable deleted label page-header object selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_page_header_objects_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_page_header_objects_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
