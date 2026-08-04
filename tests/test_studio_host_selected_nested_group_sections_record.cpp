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

void write_synthetic_report_table_for_stable_nested_group_section_expression_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", ""},
        {"9", "3", "customer.region", "0", "400", "region-header-guid"},
        {"9", "3", "customer.country", "400", "300", "country-header-guid"},
        {"9", "4", "", "700", "2200", ""},
        {"9", "5", "customer.country", "2900", "250", "country-footer-guid"},
        {"9", "5", "customer.region", "3150", "350", "region-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#2828: stable nested group section fixture should be created");
}

void write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_nested_group_section_expression_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok,
           "#2828: deleted nested group footer fixture should mark the inner group footer deleted");
}

void run_nested_group_section_record_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_stable_nested_group_section_expression_json(asset_path);
    const auto process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "2", "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host " << label << " record-selected nested group section stdout:\n"
                  << process.stdout_text << "\n";
        std::cerr << "studio host " << label << " record-selected nested group section stderr:\n"
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
                    issue_prefix + " should preserve section selection kind");
    expect_contains(process.stdout_text, "\"sectionCount\": 5",
                    issue_prefix + " should preserve live section counts");
    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                    issue_prefix + " should preserve deleted section counts");
    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                    issue_prefix + " should not fabricate deleted preview bounds");
    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"sections\": [",
            "\"id\": \"region-header-guid\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.region\"",
            "\"recordIndex\": 1",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 0",
            "\"groupingNestingDepth\": 0",
            "\"groupRole\": \"header\"",
            "\"groupPartnerSectionId\": \"region-footer-guid\"",
            "\"groupPartnerRecordIndex\": 5",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should expose the outer group-header section");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"id\": \"country-header-guid\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 2",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"header\"",
            "\"groupPartnerSectionId\": \"country-footer-guid\"",
            "\"groupPartnerRecordIndex\": 4",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should expose the inner group-header section");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"bandKind\": \"detail\"",
            "\"expression\": \"\"",
            "\"recordIndex\": 3",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": false",
            "\"groupingIndex\": null",
            "\"groupingNestingDepth\": null",
            "\"groupRole\": null",
            "\"groupPartnerSectionId\": null",
            "\"groupPartnerRecordIndex\": null",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should preserve the detail section between nested group pairs");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"id\": \"country-footer-guid\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 4",
            "\"sectionIndex\": 3",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"footer\"",
            "\"groupPartnerSectionId\": \"country-header-guid\"",
            "\"groupPartnerRecordIndex\": 2",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should expose the inner group-footer section");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"id\": \"region-footer-guid\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.region\"",
            "\"recordIndex\": 5",
            "\"sectionIndex\": 4",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 0",
            "\"groupingNestingDepth\": 0",
            "\"groupRole\": \"footer\"",
            "\"groupPartnerSectionId\": \"region-header-guid\"",
            "\"groupPartnerRecordIndex\": 1",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should expose the outer group-footer section");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"country-header-guid\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 3",
            "\"recordIndex\": 2",
            "\"deleted\": false",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 5",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"header\"",
            "\"groupingExpression\": \"customer.country\"",
            "\"groupingExpressionFieldIndex\": 2",
            "\"groupingExpressionMemoBlockNumber\": 3",
            "\"groupPartnerSectionId\": \"country-footer-guid\"",
            "\"groupPartnerRecordIndex\": 4",
            "\"groupPartnerDeleted\": false",
            "\"top\": 400",
            "\"height\": 300",
            "\"bottom\": 700"
        },
        issue_prefix + " should expose selected inner-group metadata");
}

void run_deleted_nested_group_section_record_selection(
    const std::string& studio_host_path,
    const std::filesystem::path& temp_root,
    const std::string& file_name,
    const std::string& label,
    const std::string& issue_prefix) {
    const std::filesystem::path asset_path = temp_root / file_name;
    write_synthetic_report_table_for_deleted_nested_group_footer_expression_json(asset_path);
    const auto process = run_process_capture(
        studio_host_path,
        {"--path", asset_path.string(), "--record", "4", "--json"},
        temp_root);

    if (process.exit_code != 0) {
        std::cerr << "studio host " << label << " record-selected deleted nested group section stdout:\n"
                  << process.stdout_text << "\n";
        std::cerr << "studio host " << label << " record-selected deleted nested group section stderr:\n"
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
                    issue_prefix + " should preserve selection kind");
    expect_contains(process.stdout_text, "\"sectionCount\": 4",
                    issue_prefix + " should preserve live section counts");
    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                    issue_prefix + " should expose deleted section counts");
    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                    issue_prefix + " should preserve live preview availability");
    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                    issue_prefix + " should preserve live preview top bounds");
    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                    issue_prefix + " should preserve live preview bottom bounds");
    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                    issue_prefix + " should preserve live preview heights");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    issue_prefix + " should expose deleted preview availability");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                    issue_prefix + " should preserve deleted preview top bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                    issue_prefix + " should preserve deleted preview bottom bounds");
    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                    issue_prefix + " should preserve deleted preview heights");
    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    issue_prefix + " should not advertise selected-object availability");
    expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                    issue_prefix + " should serialize null selected objects");
    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    issue_prefix + " should not advertise selected-settings availability");
    expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                    issue_prefix + " should serialize null selected settings");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"deletedSections\": [",
            "\"id\": \"country-footer-guid\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 4",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"footer\"",
            "\"groupingExpression\": \"customer.country\"",
            "\"groupingExpressionFieldIndex\": 2",
            "\"groupingExpressionMemoBlockNumber\": 3",
            "\"groupPartnerSectionId\": \"country-header-guid\"",
            "\"groupPartnerRecordIndex\": 2",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should expose deleted nested section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"id\": \"country-footer-guid\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.country\"",
            "\"expressionFieldIndex\": 2",
            "\"expressionMemoBlockNumber\": 4",
            "\"recordIndex\": 4",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"footer\"",
            "\"groupingExpression\": \"customer.country\"",
            "\"groupingExpressionFieldIndex\": 2",
            "\"groupingExpressionMemoBlockNumber\": 3",
            "\"groupPartnerSectionId\": \"country-header-guid\"",
            "\"groupPartnerRecordIndex\": 2",
            "\"groupPartnerDeleted\": false",
            "\"top\": 2900",
            "\"height\": 250",
            "\"bottom\": 3150"
        },
        issue_prefix + " should expose selected deleted-section metadata");
    expect_contains_in_order(
        process.stdout_text,
        {
            "\"sections\": [",
            "\"id\": \"region-header-guid\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.region\"",
            "\"recordIndex\": 1",
            "\"sectionIndex\": 0",
            "\"sectionCount\": 4",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 0",
            "\"groupingNestingDepth\": 0",
            "\"groupRole\": \"header\"",
            "\"groupPartnerSectionId\": \"region-footer-guid\"",
            "\"groupPartnerRecordIndex\": 5",
            "\"groupPartnerDeleted\": false",
            "\"id\": \"country-header-guid\"",
            "\"bandKind\": \"group_header\"",
            "\"expression\": \"customer.country\"",
            "\"recordIndex\": 2",
            "\"sectionIndex\": 1",
            "\"sectionCount\": 4",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 1",
            "\"groupingNestingDepth\": 1",
            "\"groupRole\": \"header\"",
            "\"groupPartnerSectionId\": \"country-footer-guid\"",
            "\"groupPartnerRecordIndex\": 4",
            "\"groupPartnerDeleted\": true",
            "\"bandKind\": \"detail\"",
            "\"expression\": \"\"",
            "\"recordIndex\": 3",
            "\"sectionIndex\": 2",
            "\"sectionCount\": 4",
            "\"groupingContextAvailable\": false",
            "\"groupingIndex\": null",
            "\"groupingNestingDepth\": null",
            "\"groupRole\": null",
            "\"groupPartnerSectionId\": null",
            "\"groupPartnerRecordIndex\": null",
            "\"groupPartnerDeleted\": false",
            "\"id\": \"region-footer-guid\"",
            "\"bandKind\": \"group_footer\"",
            "\"expression\": \"customer.region\"",
            "\"recordIndex\": 5",
            "\"sectionIndex\": 3",
            "\"sectionCount\": 4",
            "\"groupingContextAvailable\": true",
            "\"groupingIndex\": 0",
            "\"groupingNestingDepth\": 0",
            "\"groupRole\": \"footer\"",
            "\"groupPartnerSectionId\": \"region-header-guid\"",
            "\"groupPartnerRecordIndex\": 1",
            "\"groupPartnerDeleted\": false"
        },
        issue_prefix + " should preserve unaffected live sibling expressions");
}

void test_studio_host_json_preserves_selected_nested_group_sections_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_nested_group_sections_record_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    run_nested_group_section_record_selection(
        studio_host_path,
        temp_root,
        "selected_nested_group_sections_record.frx",
        "report",
        "#2828: record-selected live report nested group section selection");
    run_nested_group_section_record_selection(
        studio_host_path,
        temp_root,
        "selected_nested_group_sections_record.lbx",
        "label",
        "#2828: record-selected live label nested group section selection");
    run_deleted_nested_group_section_record_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_nested_group_sections_record.frx",
        "report",
        "#2828: record-selected deleted report nested group section selection");
    run_deleted_nested_group_section_record_selection(
        studio_host_path,
        temp_root,
        "selected_deleted_nested_group_sections_record.lbx",
        "label",
        "#2828: record-selected deleted label nested group section selection");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: test_studio_host_selected_nested_group_sections_record <studio_host_path>\n";
        return 1;
    }

    test_studio_host_json_preserves_selected_nested_group_sections_record_selection(argv[1]);

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return 1;
    }

    std::cout << "All tests passed.\n";
    return 0;
}
