// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
namespace {

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

void test_studio_host_json_updates_nested_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_update = [&](const fs::path& asset_path,
                                                        const std::string& title,
                                                        const std::string& label) {
        write_stable_nested_group_section_expression_fixture(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "2",
                "--property-name", "EXPR",
                "--property-value", "customer.state",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2682: nested report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 2U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
               "#2682: nested report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: nested group section expression update should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2682: nested label group section expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: nested group section expression update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: nested group section expression update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 5",
                        "#2682: nested group section expression update should preserve live section counts");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2682: nested group section expression update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: nested group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: nested group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: nested group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: nested group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2682: nested group section expression update should not fabricate deleted preview bounds");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"region-header-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"country-header-guid\"",
                "\"expression\": \"customer.state\"",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"country-footer-guid\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"id\": \"region-footer-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: nested group section expression update should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"country-header-guid\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2682: nested group section expression update should refresh selected inner-group metadata");
    };

    run_nested_group_expression_update(temp_root / "nested_expression_update.frx",
                                       "nested_expression_update.frx",
                                       "report");
    run_nested_group_expression_update(temp_root / "nested_expression_update.lbx",
                                       "nested_expression_update.lbx",
                                       "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_nested_report_group_section_expressions_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_clear = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_stable_nested_group_section_expression_fixture(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "2",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " nested group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " nested group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2682: nested report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: nested group section expression clear should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2682: nested label group section expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: nested group section expression clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: nested group section expression clear should preserve section selection kind");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 5",
                        "#2682: nested group section expression clear should preserve live section counts");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#2682: nested group section expression clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: nested group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: nested group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: nested group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: nested group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2682: nested group section expression clear should not fabricate deleted preview bounds");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"region-header-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"country-header-guid\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"country-footer-guid\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 4",
                "\"id\": \"region-footer-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: nested group section expression clear should preserve nested sibling ordering and expressions");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"country-header-guid\"",
                "\"bandKind\": \"group_header\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 5",
                "\"top\": 400",
                "\"height\": 300",
                "\"bottom\": 700"
            },
            "#2682: nested group section expression clear should refresh selected inner-group metadata");
    };

    run_nested_group_expression_clear(temp_root / "nested_expression_clear.frx",
                                      "nested_expression_clear.frx",
                                      "report");
    run_nested_group_expression_clear(temp_root / "nested_expression_clear.lbx",
                                      "nested_expression_clear.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_nested_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_nested_group_section_expression_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_update = [&](const fs::path& asset_path,
                                                                const std::string& title,
                                                                const std::string& label) {
        write_deleted_nested_group_footer_expression_fixture(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "4",
                "--property-name", "EXPR",
                "--property-value", "customer.state",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted nested group section expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted nested group section expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#2682: deleted nested report/label group section expression update should exit successfully");
        const auto expr_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
               "#2682: deleted nested report/label group section expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: deleted nested group section expression update should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#2682: deleted nested label group section expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: deleted nested group section expression update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 4",
                        "#2682: deleted nested group section expression update should preserve live section counts");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2682: deleted nested group section expression update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: deleted nested group section expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: deleted nested group section expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: deleted nested group section expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                        "#2682: deleted nested group section expression update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                        "#2682: deleted nested group section expression update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2682: deleted nested group section expression update should preserve deleted preview heights");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"country-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#2682: deleted nested group section expression update should refresh deleted-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"country-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"customer.state\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 6",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2900",
                "\"height\": 250",
                "\"bottom\": 3150"
            },
            "#2682: deleted nested group section expression update should refresh selected deleted-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"region-header-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"country-header-guid\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"region-footer-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: deleted nested group section expression update should preserve unaffected live sibling expressions");
    };

    run_deleted_nested_group_expression_update(temp_root / "nested_deleted_expression_update.frx",
                                               "nested_deleted_expression_update.frx",
                                               "report");
    run_deleted_nested_group_expression_update(temp_root / "nested_deleted_expression_update.lbx",
                                               "nested_deleted_expression_update.lbx",
                                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_nested_report_group_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_nested_group_section_expression_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_clear = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_deleted_nested_group_footer_expression_fixture(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "4",
                "--property-name", "EXPR",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted nested group section expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted nested group section expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#2682: deleted nested report/label group section expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#2682: deleted nested group section expression clear should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2682: deleted nested label group section expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#2682: deleted nested group section expression clear should preserve section selection kind");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 4",
                        "#2682: deleted nested group section expression clear should preserve live section counts");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#2682: deleted nested group section expression clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2682: deleted nested group section expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 3500",
                        "#2682: deleted nested group section expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 3500",
                        "#2682: deleted nested group section expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2682: deleted nested group section expression clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                        "#2682: deleted nested group section expression clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                        "#2682: deleted nested group section expression clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                        "#2682: deleted nested group section expression clear should preserve deleted preview heights");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"country-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 4",
                "\"deleted\": true"
            },
            "#2682: deleted nested group section expression clear should refresh deleted-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"country-footer-guid\"",
                "\"bandKind\": \"group_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 4",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2900",
                "\"height\": 250",
                "\"bottom\": 3150"
            },
            "#2682: deleted nested group section expression clear should refresh selected deleted-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"region-header-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1",
                "\"id\": \"country-header-guid\"",
                "\"expression\": \"customer.country\"",
                "\"expressionMemoBlockNumber\": 3",
                "\"recordIndex\": 2",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 3",
                "\"id\": \"region-footer-guid\"",
                "\"expression\": \"customer.region\"",
                "\"expressionMemoBlockNumber\": 5",
                "\"recordIndex\": 5"
            },
            "#2682: deleted nested group section expression clear should preserve unaffected live sibling expressions");
    };

    run_deleted_nested_group_expression_clear(temp_root / "nested_deleted_expression_clear.frx",
                                              "nested_deleted_expression_clear.frx",
                                              "report");
    run_deleted_nested_group_expression_clear(temp_root / "nested_deleted_expression_clear.lbx",
                                              "nested_deleted_expression_clear.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_nested_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_nested_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_nested_group_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_stable_nested_group_section_expression_fixture(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2684: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2684: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 5",
                                    "#2684: " + operation_label + " should preserve live section counts");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                                    "#2684: " + operation_label + " should preserve deleted section counts");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2684: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2684: " + operation_label + " should not fabricate deleted preview bounds");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2684: " + operation_label + " should preserve selected-section availability");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2684: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2684: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2684: " + operation_label + " should not select report settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2684: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2684: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2684: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"country-header-guid\"",
                            "\"bandKind\": \"group_header\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": 2",
                            "\"deleted\": false",
                            "\"sectionIndex\": 1",
                            "\"sectionCount\": 5",
                            "\"top\": 400",
                            "\"height\": 300",
                            "\"bottom\": 700"
                        },
                        "#2684: " + operation_label + " should refresh selected nested section metadata");
                };

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "country-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.state",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable nested group section expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable nested group section expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#2684: stable-selected nested report/label group section expression update should exit successfully");
            const auto expr_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "country-header-guid",
                .property_name = "EXPR"
            });
            expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
                   "#2684: stable-selected nested report/label group section expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_process,
                                               "customer.state",
                                               "2",
                                               "6",
                                               "stable-selected nested group section expression update");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"region-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"country-header-guid\"",
                    "\"expression\": \"customer.state\"",
                    "\"expressionMemoBlockNumber\": 6",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"country-footer-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 4",
                    "\"id\": \"region-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected nested group section expression update should preserve unaffected nested sibling expressions");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "country-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable nested group section expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable nested group section expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#2684: stable-selected nested report/label group section expression clear should exit successfully");
            expect_selected_section_expression(clear_process,
                                               "",
                                               "null",
                                               "0",
                                               "stable-selected nested group section expression clear");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"region-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"country-header-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"country-footer-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 4",
                    "\"id\": \"region-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected nested group section expression clear should preserve unaffected nested sibling expressions");
        };

    run_nested_group_expression_stable_edits(temp_root / "nested_stable_expression_update.frx",
                                             "nested_stable_expression_update.frx",
                                             "report");
    run_nested_group_expression_stable_edits(temp_root / "nested_stable_expression_update.lbx",
                                             "nested_stable_expression_update.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_nested_report_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_nested_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_nested_group_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_deleted_nested_group_footer_expression_fixture(asset_path);

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2684: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2684: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 4",
                                    "#2684: " + operation_label + " should preserve live section counts");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                    "#2684: " + operation_label + " should preserve deleted section counts");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2684: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3500",
                                    "#2684: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2684: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 2900",
                                    "#2684: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3150",
                                    "#2684: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                                    "#2684: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2684: " + operation_label + " should preserve selected-section availability");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2684: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2684: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2684: " + operation_label + " should not select report settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2684: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2684: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2684: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"country-footer-guid\"",
                            "\"bandKind\": \"group_footer\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": 4",
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"top\": 2900",
                            "\"height\": 250",
                            "\"bottom\": 3150"
                        },
                        "#2684: " + operation_label + " should refresh selected deleted nested section metadata");
                };

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "country-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.state",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted nested group section expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted nested group section expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#2684: stable-selected deleted nested report/label group section expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 4U),
                   "#2684: stable-selected deleted nested report/label group section expression update should preserve deleted state");
            const auto expr_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 4U,
                .object_name = {},
                .unique_id = "country-footer-guid",
                .property_name = "EXPR"
            });
            expect(expr_property.ok && expr_property.exists && expr_property.value == "customer.state",
                   "#2684: stable-selected deleted nested report/label group section expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_process,
                                                       "customer.state",
                                                       "2",
                                                       "6",
                                                       "stable-selected deleted nested group section expression update");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"country-footer-guid\"",
                    "\"expression\": \"customer.state\"",
                    "\"expressionMemoBlockNumber\": 6",
                    "\"recordIndex\": 4"
                },
                "#2684: stable-selected deleted nested group section expression update should refresh deleted section metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"region-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"country-header-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"region-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected deleted nested group section expression update should preserve unaffected live sibling expressions");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "country-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted nested group section expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted nested group section expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#2684: stable-selected deleted nested report/label group section expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 4U),
                   "#2684: stable-selected deleted nested report/label group section expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_process,
                                                       "",
                                                       "null",
                                                       "0",
                                                       "stable-selected deleted nested group section expression clear");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"country-footer-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 4"
                },
                "#2684: stable-selected deleted nested group section expression clear should refresh deleted section metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"region-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1",
                    "\"id\": \"country-header-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 3",
                    "\"id\": \"region-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 5"
                },
                "#2684: stable-selected deleted nested group section expression clear should preserve unaffected live sibling expressions");
        };

    run_deleted_nested_group_expression_stable_edits(temp_root / "nested_deleted_stable_expression_update.frx",
                                                     "nested_deleted_stable_expression_update.frx",
                                                     "report");
    run_deleted_nested_group_expression_stable_edits(temp_root / "nested_deleted_stable_expression_update.lbx",
                                                     "nested_deleted_stable_expression_update.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_grouping_nested_mutation <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_nested_report_group_section_expressions_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_nested_report_group_section_expressions_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_nested_report_group_section_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_nested_report_group_section_expressions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_nested_report_group_section_expressions_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_nested_report_group_section_expressions_by_stable_selection(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
