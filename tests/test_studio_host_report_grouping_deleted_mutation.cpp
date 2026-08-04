// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

void test_studio_host_json_updates_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_update = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_deleted_group_section_expression_fixture(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1571: deleted report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1571: deleted report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1571: deleted report/label group section expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1571: deleted group section expression updates should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1571: deleted group section expression updates should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group section expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2270: deleted group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group section expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2270: deleted group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group section expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2270: deleted group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group section expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2270: deleted group section expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group section expression update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2270: deleted group section expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1571: deleted report/label group section update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1571: deleted report/label group section update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
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
            "#1571: deleted group section expression update should preserve live sibling section metadata");
    };

    run_deleted_group_expression_update(temp_root / "deleted_group_update.frx", "deleted_group_update.frx", "report");
    run_deleted_group_expression_update(temp_root / "deleted_group_update.lbx", "deleted_group_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_expression_clear = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_deleted_group_section_expression_fixture(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1572: deleted report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1572: deleted report/label group section expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1572: deleted group section expression clears should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1572: deleted group section expression clears should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group section expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 600",
                        "#2270: deleted group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group section expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 4100",
                        "#2270: deleted group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group section expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2270: deleted group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group section expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 600",
                        "#2270: deleted group section expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group section expression clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 600",
                        "#2270: deleted group section expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1572: deleted report/label group section clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1572: deleted report/label group section clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
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
            "#1572: deleted group section expression clear should preserve live sibling section metadata");
    };

    run_deleted_group_expression_clear(temp_root / "deleted_group_clear.frx", "deleted_group_clear.frx", "report");
    run_deleted_group_expression_clear(temp_root / "deleted_group_clear.lbx", "deleted_group_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_update = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_deleted_group_footer_expression_fixture(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--property-value", "customer.region",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1574: deleted report/label group footer expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.region",
               "#1574: deleted report/label group footer expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1574: deleted report/label group footer expression update should return refreshed layout JSON");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1574: deleted group footer expression updates should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1574: deleted group footer expression updates should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group footer expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group footer expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2270: deleted group footer expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group footer expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2270: deleted group footer expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group footer expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2270: deleted group footer expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group footer expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2270: deleted group footer expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2270: deleted group footer expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group footer expression update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2270: deleted group footer expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1574: deleted report/label group footer update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.region\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1574: deleted report/label group footer update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
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
            "#1574: deleted group footer expression update should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_update(temp_root / "deleted_group_footer_update.frx",
                                               "deleted_group_footer_update.frx",
                                               "report");
    run_deleted_group_footer_expression_update(temp_root / "deleted_group_footer_update.lbx",
                                               "deleted_group_footer_update.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_group_footer_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_group_footer_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_footer_expression_clear = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_deleted_group_footer_expression_fixture(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted group footer expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted group footer expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1573: deleted report/label group footer expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1573: deleted report/label group footer expression clear should return refreshed layout JSON");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1573: deleted group footer expression clears should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1573: deleted group footer expression clears should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2270: deleted group footer expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3600",
                        "#2270: deleted group footer expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2270: deleted group footer expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3600",
                        "#2270: deleted group footer expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2270: deleted group footer expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 3600",
                        "#2270: deleted group footer expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                        "#2270: deleted group footer expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#2270: deleted group footer expression clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 500",
                        "#2270: deleted group footer expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1573: deleted report/label group footer clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0"
            },
            "#1573: deleted report/label group footer clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
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
            "#1573: deleted group footer expression clear should preserve live sibling section metadata");
    };

    run_deleted_group_footer_expression_clear(temp_root / "deleted_group_footer_clear.frx",
                                              "deleted_group_footer_clear.frx",
                                              "report");
    run_deleted_group_footer_expression_clear(temp_root / "deleted_group_footer_clear.lbx",
                                              "deleted_group_footer_clear.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_grouping_deleted_mutation <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_group_section_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_group_section_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_group_footer_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_group_footer_expressions(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
