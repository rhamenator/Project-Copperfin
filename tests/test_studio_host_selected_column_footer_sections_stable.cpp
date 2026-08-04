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

void write_synthetic_report_table_for_stable_column_section_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "", "", ""},
        {"9", "2", "0", "450", "column-header-section-guid"},
        {"9", "4", "450", "2600", ""},
        {"9", "6", "3050", "400", "column-footer-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2810: stable column-footer section fixture should be created");
}

void run_column_footer_section_stable_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_column_section_json(asset_path);

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "column-footer-section-guid", "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable column-footer section stdout:\n"
                  << process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable column-footer section stderr:\n"
                  << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should preserve document titles");
    if (asset_path.extension() == ".lbx") {
        expect_contains(process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should preserve section selection classification");
    expect_contains(process.stdout_text, "\"sectionCount\": 3",
                    issue_prefix + " should preserve live sibling section counts");
    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " should preserve deleted section counts");
    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should expose live preview availability");
    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3450",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3450",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview availability");
    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3"
        },
        issue_prefix + " should expose sibling section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column-footer-section-guid\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": false",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 3",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        issue_prefix + " should expose selected section metadata");
}

void run_deleted_column_footer_section_stable_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_column_section_json(asset_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
    expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
           issue_prefix + " fixture should mark the column-footer section deleted");

    const auto process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--unique-id", "column-footer-section-guid", "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host " << label << " stable deleted column-footer section stdout:\n"
                  << process.stdout_text << "\n";
        std::cerr << "studio host " << label << " stable deleted column-footer section stderr:\n"
                  << process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(process.exit_code == 0, issue_prefix + " should exit successfully");
    expect_contains(process.stdout_text, "\"documentTitle\": \"" + file_name + "\"",
                    issue_prefix + " should preserve document titles");
    if (asset_path.extension() == ".lbx") {
        expect_contains(process.stdout_text, "\"isLabel\": true",
                        issue_prefix + " should retain label identity");
    }
    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    issue_prefix + " should advertise selected-section availability");
    expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    issue_prefix + " should advertise report-selection availability");
    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    issue_prefix + " should preserve section selection classification");
    expect_contains(process.stdout_text, "\"sectionCount\": 2",
                    issue_prefix + " should preserve live sibling section counts");
    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                    issue_prefix + " should expose deleted section counts");
    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                    issue_prefix + " should preserve live preview left bounds");
    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                    issue_prefix + " should preserve live preview right bounds");
    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3050",
                    issue_prefix + " should refresh live preview bottom bounds");
    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                    issue_prefix + " should preserve live preview widths");
    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3050",
                    issue_prefix + " should refresh live preview heights");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    issue_prefix + " should preserve deleted preview left bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3050",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    issue_prefix + " should preserve deleted preview right bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3450",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    issue_prefix + " should preserve deleted preview widths");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    issue_prefix + " should not advertise selected object-section availability");
    expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                    issue_prefix + " should serialize null selected object sections");
    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"column-footer-section-guid\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true"
        },
        issue_prefix + " should expose deleted section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"column-footer-section-guid\"",
            "\"bandKind\": \"column_footer\"",
            "\"recordIndex\": 3",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"top\": 3050",
            "\"height\": 400",
            "\"bottom\": 3450"
        },
        issue_prefix + " should expose selected section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"sections\": [",
            "\"bandKind\": \"column_header\"",
            "\"recordIndex\": 1",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 2"
        },
        issue_prefix + " should preserve live sibling metadata");
}

void test_studio_host_json_preserves_selected_column_footer_sections_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_column_footer_sections_stable_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_column_footer_section_stable_selection(
        studio_host_path,
        temp_root,
        "stable_column_footer_sections.frx",
        "report",
        "#2810: stable-selected live report column-footer section selection");
    run_column_footer_section_stable_selection(
        studio_host_path,
        temp_root,
        "stable_column_footer_sections.lbx",
        "label",
        "#2810: stable-selected live label column-footer section selection");
    run_deleted_column_footer_section_stable_selection(
        studio_host_path,
        temp_root,
        "stable_deleted_column_footer_sections.frx",
        "report",
        "#2810: stable-selected deleted report column-footer section selection");
    run_deleted_column_footer_section_stable_selection(
        studio_host_path,
        temp_root,
        "stable_deleted_column_footer_sections.lbx",
        "label",
        "#2810: stable-selected deleted label column-footer section selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_column_footer_sections_stable <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_column_footer_sections_stable_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
