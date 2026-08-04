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

}  // namespace

void test_studio_host_json_exposes_report_group_footer_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_footer_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_footer_expression_json = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_stable_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1667: stable selected report/label group-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1667: stable selected group-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1667: stable selected label group-footer section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1667: stable selected group-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1667: stable selected group-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1667: stable selected group-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1667: stable selected group-footer section JSON should preserve sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1667: stable selected group-footer section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1929: stable selected group-footer section JSON should preserve preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1929: stable selected group-footer section JSON should preserve preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1929: stable selected group-footer section JSON should preserve preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#1929: stable selected group-footer section JSON should preserve preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1929: stable selected group-footer section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1667: stable selected group-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1667: stable selected group-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1667: stable selected group-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1667: stable selected group-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1667: stable selected group-footer section JSON should expose sibling section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"sectionIndex\": 2",
                "\"sectionCount\": 3",
                "\"top\": 3600",
                "\"height\": 500",
                "\"bottom\": 4100"
            },
            "#1667: stable selected group-footer sections should expose selected expression metadata");
    };

    run_group_footer_expression_json(temp_root / "stable_group_footer.frx",
                                     "stable_group_footer.frx",
                                     "report");
    run_group_footer_expression_json(temp_root / "stable_group_footer.lbx",
                                     "stable_group_footer.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_json = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_stable_group_section_expression_fixture(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1668: stable deleted group-header fixture should mark the group-header section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-header-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1668: stable selected deleted report/label group-header section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1668: stable selected deleted group-header section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1668: stable selected deleted label group-header section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1668: stable selected deleted group-header sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1668: stable selected deleted group-header sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1668: stable selected deleted group-header sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1668: stable selected deleted group-header section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1668: stable selected deleted group-header section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                        "#1930: stable selected deleted group-header section JSON should refresh live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#1930: stable selected deleted group-header section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-header section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#1930: stable selected deleted group-header section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1668: stable selected deleted group-header sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1668: stable selected deleted group-header sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group-header-guid\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true"
            },
            "#1668: stable selected deleted group-header section JSON should expose deleted section metadata");
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
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 600",
                "\"bottom\": 600"
            },
            "#1668: stable selected deleted group-header sections should expose selected expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"recordIndex\": 3"
            },
            "#1668: stable selected deleted group-header section JSON should preserve live sibling metadata");
    };

    run_deleted_group_expression_json(temp_root / "stable_deleted_group_header.frx",
                                      "stable_deleted_group_header.frx",
                                      "report");
    run_deleted_group_expression_json(temp_root / "stable_deleted_group_header.lbx",
                                      "stable_deleted_group_header.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_report_group_footer_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_json = [&](const fs::path& asset_path,
                                                              const std::string& title,
                                                              const std::string& label) {
        write_stable_group_section_expression_fixture(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
               "#1669: stable deleted group-footer fixture should mark the group-footer section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "group-footer-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted group footer expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted group footer expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1669: stable selected deleted report/label group-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1669: stable selected deleted group-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1669: stable selected deleted label group-footer section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1669: stable selected deleted group-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1669: stable selected deleted group-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1669: stable selected deleted group-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1669: stable selected deleted group-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1669: stable selected deleted group-footer section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should refresh live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1930: stable selected deleted group-footer section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#1930: stable selected deleted group-footer section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1669: stable selected deleted group-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1669: stable selected deleted group-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"group-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true"
            },
            "#1669: stable selected deleted group-footer section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"group-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 3600",
                "\"height\": 500",
                "\"bottom\": 4100"
            },
            "#1669: stable selected deleted group-footer sections should expose selected expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"group_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1669: stable selected deleted group-footer section JSON should preserve live sibling metadata");
    };

    run_deleted_group_footer_expression_json(temp_root / "stable_deleted_group_footer.frx",
                                             "stable_deleted_group_footer.frx",
                                             "report");
    run_deleted_group_footer_expression_json(temp_root / "stable_deleted_group_footer.lbx",
                                             "stable_deleted_group_footer.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_report_group_section_expressions(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_group_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_expression_json = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_group_section_expression_fixture(asset_path);
        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " group section expression stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " group section expression stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1566: report/label group section expression JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1566: report/label group section expression JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#2268: record-selected label group section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 3",
                        "#1566: report/label group section expression JSON should preserve group/detail/footer sections");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2268: record-selected group section JSON should preserve deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2268: record-selected group section JSON should expose live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2268: record-selected group section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2268: record-selected group section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2268: record-selected group section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2268: record-selected group section JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2268: record-selected group section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                        "#2268: record-selected group section JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2268: record-selected group section JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2268: record-selected group section JSON should expose selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#2268: record-selected group section JSON should expose report-selection availability");
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
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"recordIndex\": 2",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 3"
            },
            "#1566: report/label group section expression JSON should expose full section expression metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.country\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1566: selected report/label group sections should expose expression metadata");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1566: selected group sections should preserve section selection classification");
    };

    run_group_expression_json(temp_root / "group_sections.frx", "group_sections.frx", "report");
    run_group_expression_json(temp_root / "group_sections.lbx", "group_sections.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_grouping_remaining_exposure <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_exposes_report_group_footer_expressions_by_stable_selection(
        argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_exposes_deleted_report_group_section_expressions_by_stable_selection(argv[1]);
    cf_test_studio_host_json::
        test_studio_host_json_exposes_deleted_report_group_footer_expressions_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_exposes_report_group_section_expressions(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
