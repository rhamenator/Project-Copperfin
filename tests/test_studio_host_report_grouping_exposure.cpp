// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

void write_stable_group_section_expression_fixture(const std::filesystem::path& report_path) {
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
        {"9", "3", "customer.country", "0", "600", "group-header-guid"},
        {"9", "4", "", "600", "3000", ""},
        {"9", "5", "customer.country", "3600", "500", "group-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1666: synthetic report table for stable group section expression JSON should be created");
}

void write_stable_blank_group_footer_expression_fixture(const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "3", "customer.country", "", "0", "", "600", "group-header-guid"},
        {"9", "4", "", "", "600", "", "3000", ""},
        {"5", "", "\"Detail label\"", "120", "900", "1400", "240", "detail-object-guid"},
        {"9", "5", "", "", "3600", "", "500", "group-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#2687: synthetic report table for stable blank group-footer expression JSON should be created");
}

void write_deleted_blank_group_footer_expression_fixture(const std::filesystem::path& report_path) {
    write_stable_blank_group_footer_expression_fixture(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok,
           "#2687: synthetic report table should mark the blank group-footer section deleted");
}

void write_stable_nested_group_section_expression_fixture(const std::filesystem::path& report_path) {
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
    expect(create_result.ok,
           "#2680: synthetic report table for stable nested group section expression JSON should be created");
}

void write_deleted_nested_group_footer_expression_fixture(const std::filesystem::path& report_path) {
    write_stable_nested_group_section_expression_fixture(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 4U, true);
    expect(delete_result.ok,
           "#2681: synthetic report table should mark the nested group footer section deleted");
}

}  // namespace

void test_studio_host_json_exposes_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_json = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_stable_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(process.exit_code == 0,
               "#1666: stable selected report/label group-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1666: stable selected group-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1666: stable selected label group-header section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1666: stable selected group-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1666: stable selected group-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1666: stable selected group-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1666: stable selected group-header section JSON should preserve sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1666: stable selected group-header section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1929: stable selected group-header section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1929: stable selected group-header section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1929: stable selected group-header section JSON should preserve preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#1929: stable selected group-header section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1929: stable selected group-header section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1666: stable selected group-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1666: stable selected group-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1666: stable selected group-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1666: stable selected group-header sections should serialize null selected settings");
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
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 3"
            },
            "#1666: stable selected group-header section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group-header-guid\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 3",
                "\"top\": 0",
                "\"height\": 600",
                "\"bottom\": 600"
            },
            "#1666: stable selected group-header sections should expose selected expression metadata");
    };

    run_group_expression_json(temp_root / "stable_group_sections.frx",
                              "stable_group_sections.frx",
                              "report");
    run_group_expression_json(temp_root / "stable_group_sections.lbx",
                              "stable_group_sections.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_nested_report_group_section_ordering_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_json = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_stable_nested_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "country-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable nested group section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable nested group section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(process.exit_code == 0,
               "#2680: stable selected nested report/label group section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2680: stable selected nested group section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2680: stable selected nested label group section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 5",
                        "#2680: stable selected nested group section JSON should preserve both group levels");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2680: stable selected nested group section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2680: stable selected nested group section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2680: stable selected nested group section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2680: stable selected nested group section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2680: stable selected nested group section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2680: stable selected nested group section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2680: stable selected nested group section JSON should advertise section selection");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2680: stable selected nested group section JSON should advertise report selection");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2680: stable selected nested group section JSON should preserve selection classification");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#2680: stable selected nested group section JSON should keep selected object unavailable");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#2680: stable selected nested group section JSON should keep selected settings unavailable");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"region-header-guid\"",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 1",
                "\"id\": \"country-header-guid\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"country-footer-guid\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 4",
                "\"id\": \"region-footer-guid\"",
                "\"expression\": \"customer.region\"",
                "\"recordIndex\": 5"
            },
            "#2680: stable selected nested group section JSON should preserve nested sibling ordering and expressions");
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
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2680: stable selected nested group section JSON should expose selected inner-group metadata");
    };

    run_nested_group_expression_json(temp_root / "nested_group_sections.frx",
                                     "nested_group_sections.frx",
                                     "report");
    run_nested_group_expression_json(temp_root / "nested_group_sections.lbx",
                                     "nested_group_sections.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_groupings_in_layout_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_groupings_summary_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_groupings_summary_json = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_stable_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " report grouping summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " report grouping summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(process.exit_code == 0,
               "#2685: report/label grouping summary JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2685: report/label grouping summary JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2685: label grouping summary JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"groupingCount\": 1",
                        "#2685: report/label grouping summary JSON should expose one grouping");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#2685: report/label grouping summary JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2685: report/label grouping summary JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#2685: report/label grouping summary JSON should preserve no-selection state");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#2685: report/label grouping summary JSON should classify the summary launch as no selection");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"groupings\": [",
                "\"groupingIndex\": 0",
                "\"nestingDepth\": 0",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"headerSectionId\": \"group-header-guid\"",
                "\"headerRecordIndex\": 1",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"group-footer-guid\"",
                "\"footerRecordIndex\": 3",
                "\"footerDeleted\": false"
            },
            "#2685: report/label grouping summary JSON should expose explicit group-header/footer pairing metadata");
    };

    run_groupings_summary_json(temp_root / "groupings_summary.frx",
                               "groupings_summary.frx",
                               "report");
    run_groupings_summary_json(temp_root / "groupings_summary.lbx",
                               "groupings_summary.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_nested_mixed_state_groupings_in_layout_summary(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_groupings_summary_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_groupings_summary_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_deleted_nested_group_footer_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested grouping summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested grouping summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(process.exit_code == 0,
               "#2685: nested mixed-state report/label grouping summary JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2685: nested mixed-state grouping summary JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2685: nested mixed-state label grouping summary JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"groupingCount\": 2",
                        "#2685: nested mixed-state grouping summary JSON should expose both grouping levels");
        expect_contains(process.stdout_text, "\"sectionCount\": 4",
                        "#2685: nested mixed-state grouping summary JSON should preserve live section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2685: nested mixed-state grouping summary JSON should preserve deleted section counts");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"groupings\": [",
                "\"groupingIndex\": 0",
                "\"nestingDepth\": 0",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"headerSectionId\": \"region-header-guid\"",
                "\"headerRecordIndex\": 1",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"region-footer-guid\"",
                "\"footerRecordIndex\": 5",
                "\"footerDeleted\": false",
                "\"groupingIndex\": 1",
                "\"nestingDepth\": 1",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"headerSectionId\": \"country-header-guid\"",
                "\"headerRecordIndex\": 2",
                "\"headerDeleted\": false",
                "\"footerSectionId\": \"country-footer-guid\"",
                "\"footerRecordIndex\": 4",
                "\"footerDeleted\": true"
            },
            "#2685: nested mixed-state grouping summary JSON should pair live headers with deleted nested footers");
    };

    run_nested_groupings_summary_json(temp_root / "nested_groupings_summary.frx",
                                      "nested_groupings_summary.frx",
                                      "report");
    run_nested_groupings_summary_json(temp_root / "nested_groupings_summary.lbx",
                                      "nested_groupings_summary.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_resolved_grouping_expression_for_blank_footer_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_blank_group_footer_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_blank_footer_grouping_expression_json = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label,
                                                               bool deleted_footer) {
        if (deleted_footer) {
            write_deleted_blank_group_footer_expression_fixture(asset_path);
        } else {
            write_stable_blank_group_footer_expression_fixture(asset_path);
        }

        const auto footer_record = deleted_footer ? "4" : "4";
        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", footer_record, "--json"},
            temp_root);

        if (section_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank group footer expression stdout:\n"
                      << section_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank group footer expression stderr:\n"
                      << section_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }
        expect(section_process.exit_code == 0,
               "#2687: blank group-footer selection JSON should exit successfully");
        expect_contains(section_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2687: blank group-footer selection JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(section_process.stdout_text, "\"isLabel\": true",
                            "#2687: blank group-footer label selection JSON should retain label identity");
        }
        expect_contains_in_order(
            section_process.stdout_text,
            {
                deleted_footer ? "\"deletedSections\": [" : "\"sections\": [",
                "\"id\": \"group-footer-guid\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"groupingExpression\": \"customer.country\"",
                "\"groupingExpressionFieldIndex\": 2",
                "\"groupingExpressionMemoBlockNumber\": 2"
            },
            "#2687: blank group-footer section arrays should expose resolved grouping-expression metadata without fabricating direct EXPR");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group-footer-guid\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"groupRole\": \"footer\"",
                "\"groupingExpression\": \"customer.country\"",
                "\"groupingExpressionFieldIndex\": 2",
                "\"groupingExpressionMemoBlockNumber\": 2"
            },
            "#2687: selected blank group footers should expose resolved grouping-expression provenance from the paired header");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " blank group footer object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " blank group footer object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#2687: blank group-footer selected object JSON should exit successfully");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"groupingContextAvailable\": false",
                "\"groupingExpression\": null",
                "\"groupingExpressionFieldIndex\": null",
                "\"groupingExpressionMemoBlockNumber\": 0"
            },
            "#2687: non-group selected object sections should keep resolved grouping-expression metadata absent");
    };

    run_blank_footer_grouping_expression_json(temp_root / "blank_group_footer.frx",
                                              "blank_group_footer.frx",
                                              "report",
                                              false);
    run_blank_footer_grouping_expression_json(temp_root / "blank_group_footer.lbx",
                                              "blank_group_footer.lbx",
                                              "label",
                                              false);
    run_blank_footer_grouping_expression_json(temp_root / "deleted_blank_group_footer.frx",
                                              "deleted_blank_group_footer.frx",
                                              "deleted report",
                                              true);
    run_blank_footer_grouping_expression_json(temp_root / "deleted_blank_group_footer.lbx",
                                              "deleted_blank_group_footer.lbx",
                                              "deleted label",
                                              true);

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <studio-host>\n";
        return 1;
    }

    cf_test_studio_host_json::test_studio_host_json_exposes_report_group_section_expressions_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_nested_report_group_section_ordering_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_report_groupings_in_layout_summary(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_nested_mixed_state_groupings_in_layout_summary(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_resolved_grouping_expression_for_blank_footer_sections(argv[1]);
    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
