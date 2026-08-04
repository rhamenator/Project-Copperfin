// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

void write_group_section_expression_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", ""},
        {"9", "3", "customer.country", "0", "600"},
        {"9", "4", "", "600", "3000"},
        {"9", "5", "customer.country", "3600", "500"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1566: synthetic report table for group section expression JSON should be created");
}

void write_deleted_group_section_expression_fixture(const std::filesystem::path& report_path) {
    write_group_section_expression_fixture(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1569: synthetic report table should mark group section deleted");
}

void write_deleted_group_footer_expression_fixture(const std::filesystem::path& report_path) {
    write_group_section_expression_fixture(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1570: synthetic report table should mark group footer section deleted");
}

}  // namespace

void test_studio_host_json_exposes_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_deleted_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1569: deleted report/label group section expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1569: deleted report/label group section expression JSON should preserve document titles");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1569: deleted report/label group section expression JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1569: deleted report/label group section expression JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1569: deleted group section selections should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1569: deleted group section selections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2271: deleted group section expression JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2271: deleted group section expression JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2271: deleted group section expression JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2271: deleted group section expression JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2271: deleted group section expression JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2271: deleted group section expression JSON should preserve deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2271: deleted group section expression JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2271: deleted group section expression JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2271: deleted group section expression JSON should preserve deleted preview heights");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1569: deleted report/label group section JSON should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1569: selected deleted group sections should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1569: deleted group section JSON should preserve live sibling section metadata");
    };

    run_deleted_group_expression_json(temp_root / "deleted_group.frx", "deleted_group.frx", "report");
    run_deleted_group_expression_json(temp_root / "deleted_group.lbx", "deleted_group.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_json = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_deleted_group_footer_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1570: deleted report/label group footer expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1570: deleted report/label group footer expression JSON should preserve document titles");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1570: deleted report/label group footer expression JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1570: deleted report/label group footer expression JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1570: deleted group footer selections should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1570: deleted group footer selections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2271: deleted group footer expression JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2271: deleted group footer expression JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2271: deleted group footer expression JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2271: deleted group footer expression JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2271: deleted group footer expression JSON should preserve deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2271: deleted group footer expression JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2271: deleted group footer expression JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2271: deleted group footer expression JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2271: deleted group footer expression JSON should preserve deleted preview heights");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1570: deleted report/label group footer JSON should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1570: selected deleted group footers should expose expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1570: deleted group footer JSON should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_json(temp_root / "deleted_group_footer.frx",
                                             "deleted_group_footer.frx",
                                             "report");
    run_deleted_group_footer_expression_json(temp_root / "deleted_group_footer.lbx",
                                             "deleted_group_footer.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_grouping_deleted_exposure <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_exposes_deleted_report_group_section_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_deleted_report_group_footer_expressions(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
