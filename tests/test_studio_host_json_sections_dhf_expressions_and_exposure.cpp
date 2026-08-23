// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_ONLY_EXPRESSIONS)
void test_studio_host_json_exposes_detail_header_footer_section_kinds(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_kind_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_sections = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail header/footer section summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail header/footer section summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1763: detail header/footer report/label section-kind JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1763: detail header/footer section-kind JSON should return report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1763: detail header/footer label layouts should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1763: detail header/footer section-kind JSON should summarize live sections");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1763: detail header/footer section-kind JSON should summarize deleted sections");
        expect_contains(process.stdout_text, "\"sectionKindCount\": 2",
                        "#1763: detail header/footer section-kind JSON should expose live kind bucket count");
        expect_contains(process.stdout_text,
                        "\"sectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1},\n"
                        "        {\"kind\": \"detail_header\", \"count\": 1}\n"
                        "      ]",
                        "#1763: detail header/footer section-kind JSON should count live detail header/footer buckets");
        expect_contains(process.stdout_text, "\"deletedSectionKindCount\": 1",
                        "#1763: detail header/footer section-kind JSON should expose deleted kind bucket count");
        expect_contains(process.stdout_text,
                        "\"deletedSectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1}\n"
                        "      ]",
                        "#1763: detail header/footer section-kind JSON should count deleted detail footer buckets");
        expect_contains(process.stdout_text, "\"sectionHeightTotal\": 550",
                        "#1763: detail header/footer section-kind JSON should sum live section heights");
        expect_contains(process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                        "#1763: detail header/footer section-kind JSON should sum deleted section heights");

        const auto expect_selected_section = [&](const std::string& unique_id,
                                                 const std::string& record_index,
                                                 const std::string& object_code,
                                                 const std::string& band_title,
                                                 const std::string& band_kind,
                                                 const std::string& selection_label) {
            const auto section_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                temp_root);

            if (section_process.exit_code != 0) {
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stdout:\n" << section_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stderr:\n" << section_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(section_process.exit_code == 0,
                   "#1763: selected detail header/footer section JSON should exit successfully");
            expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1763: selected detail header/footer JSON should advertise selected sections");
            expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1763: selected detail header/footer JSON should expose section selection kind");
            expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2275: selected detail header/footer section JSON should preserve live preview availability");
            expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview left bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview top bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview right bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2275: selected detail header/footer section JSON should preserve live preview bottom bounds");
            expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 0",
                            "#2275: selected detail header/footer section JSON should preserve live preview widths");
            expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2275: selected detail header/footer section JSON should preserve live preview heights");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview availability");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview left bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview top bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview right bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview bottom bounds");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview widths");
            expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2275: selected detail header/footer section JSON should preserve deleted preview heights");
            expect_contains_in_order(
                section_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"" + band_title + "\"",
                    "\"bandKind\": \"" + band_kind + "\"",
                    "\"recordIndex\": " + record_index,
                    "\"objectCode\": " + object_code
                },
                "#1763: selected " + selection_label + " JSON should expose detail header/footer band metadata");
        };

        expect_selected_section("detail-header-guid", "0", "9", "Detail Header", "detail_header", "detail header");
        expect_selected_section("detail-footer-guid", "1", "10", "Detail Footer", "detail_footer", "detail footer");
        expect_selected_section("deleted-detail-footer-guid",
                                "2",
                                "10",
                                "Detail Footer",
                                "detail_footer",
                                "deleted detail footer");
    };

    run_detail_header_footer_sections(temp_root / "detail_header_footer_sections.frx",
                                      "detail_header_footer_sections.frx",
                                      "report");
    run_detail_header_footer_sections(temp_root / "detail_header_footer_sections.lbx",
                                      "detail_header_footer_sections.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_SKIP_EXPRESSIONS)

void test_studio_host_json_updates_detail_header_footer_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_expression_edits = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "0",
                "--property-name", "EXPR",
                "--property-value", "detail header updated",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail-header expression update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail-header expression update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1765: detail-header report/label expression update should exit successfully");
        const auto updated_expr = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = {},
            .property_name = "EXPR"
        });
        expect(updated_expr.ok && updated_expr.exists && updated_expr.value == "detail header updated",
               "#1765: detail-header expression update should persist the EXPR memo field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1765: detail-header expression update should return refreshed layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1765: detail-header expression update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1765: detail-header expression update should preserve section selection");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1765: detail-header expression update should preserve selection kind");
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2262: detail-header expression update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2262: detail-header expression update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2262: detail-header expression update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2262: detail-header expression update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2262: detail-header expression update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2262: detail-header expression update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2262: detail-header expression update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"dryRun\": false",
                        "#2262: detail-header expression update JSON should expose committed execution");
        expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                        "#2262: detail-header expression update JSON should expose mutation state");
        expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                        "#2262: detail-header expression update JSON should expose undo availability");
        expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2262: detail-header expression update JSON should expose expression undo labels");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"title\": \"Detail Header\"",
                "\"bandKind\": \"detail_header\"",
                "\"expression\": \"detail header updated\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 0",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1765: detail-header expression update should refresh selected-section expression metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"title\": \"Detail Footer\"",
                "\"bandKind\": \"detail_footer\"",
                "\"expression\": \"detail footer expression\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 2",
                "\"recordIndex\": 1"
            },
            "#1765: detail-header expression update should preserve sibling detail-footer expression metadata");

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
            std::cerr << "studio host " << label << " detail-footer expression clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail-footer expression clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1765: detail-footer report/label expression clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1765: detail-footer expression clear should return refreshed layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1765: detail-footer expression clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1765: detail-footer expression clear should preserve section selection");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1765: detail-footer expression clear should preserve selection kind");
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2262: detail-footer expression clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2262: detail-footer expression clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2262: detail-footer expression clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#2262: detail-footer expression clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2262: detail-footer expression clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#2262: detail-footer expression clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2262: detail-footer expression clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                        "#2262: detail-footer expression clear JSON should expose committed execution");
        expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                        "#2262: detail-footer expression clear JSON should expose mutation state");
        expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                        "#2262: detail-footer expression clear JSON should expose undo availability");
        expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                        "#2262: detail-footer expression clear JSON should expose expression undo labels");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"title\": \"Detail Footer\"",
                "\"bandKind\": \"detail_footer\"",
                "\"expression\": \"\"",
                "\"expressionFieldIndex\": null",
                "\"expressionMemoBlockNumber\": 0",
                "\"recordIndex\": 1",
                "\"top\": 300",
                "\"height\": 250",
                "\"bottom\": 550"
            },
            "#1765: detail-footer expression clear should refresh selected-section expression metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"title\": \"Detail Header\"",
                "\"bandKind\": \"detail_header\"",
                "\"expression\": \"detail header updated\"",
                "\"expressionFieldIndex\": 2",
                "\"expressionMemoBlockNumber\": 4",
                "\"recordIndex\": 0"
            },
            "#1765: detail-footer expression clear should preserve sibling detail-header expression metadata");
    };

    run_detail_header_footer_expression_edits(temp_root / "detail_header_footer_expression.frx",
                                              "detail_header_footer_expression.frx",
                                              "report");
    run_detail_header_footer_expression_edits(temp_root / "detail_header_footer_expression.lbx",
                                              "detail_header_footer_expression.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_stable_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_stable_expression_selection =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto expect_live_section = [&](const std::string& unique_id,
                                                 const std::string& section_title,
                                                 const std::string& band_kind,
                                                 const std::string& expression,
                                                 const std::string& memo_block,
                                                 const std::string& record_index,
                                                 const std::string& section_index,
                                                 const std::string& object_code,
                                                 const std::string& top,
                                                 const std::string& height,
                                                 const std::string& bottom,
                                                 const std::string& selection_label) {
                const auto process = run_process_capture(
                    studio_host_path,
                    {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                    temp_root);

                if (process.exit_code != 0) {
                    std::cerr << "studio host " << label << " stable live " << selection_label
                              << " stdout:\n" << process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " stable live " << selection_label
                              << " stderr:\n" << process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(process.exit_code == 0,
                       "#1768: stable detail header/footer section expression selection should exit successfully");
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                "#1768: stable detail header/footer expression selection should preserve document titles");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1768: stable detail header/footer expression label selection should retain identity");
                }
                expect_contains(process.stdout_text, "\"sectionCount\": 2",
                                "#1768: stable detail header/footer expression selection should preserve live sections");
                expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1768: stable detail header/footer expression selection should preserve deleted sections");
                expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#2267: stable detail header/footer expression selection should preserve live preview availability");
                expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview left bounds");
                expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview top bounds");
                expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview right bounds");
                expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                                "#2267: stable detail header/footer expression selection should preserve live preview bottom bounds");
                expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                "#2267: stable detail header/footer expression selection should preserve live preview widths");
                expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                                "#2267: stable detail header/footer expression selection should preserve live preview heights");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview availability");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview left bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview top bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview right bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview bottom bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview widths");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                "#2267: stable detail header/footer expression selection should preserve deleted preview heights");
                expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1768: stable detail header/footer expression selection should expose selected sections");
                expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1768: stable detail header/footer expression selection should expose section selections");
                expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not select report objects");
                expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not fabricate object sections");
                expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1768: stable detail header/footer expression selection should not select settings");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1768: stable live " + selection_label + " should expose section expression metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1768: stable live " + selection_label + " should expose selected-section expression metadata");
            };

            expect_live_section("detail-header-guid",
                                "Detail Header",
                                "detail_header",
                                "detail header expression",
                                "1",
                                "0",
                                "0",
                                "9",
                                "0",
                                "300",
                                "300",
                                "detail header");
            expect_live_section("detail-footer-guid",
                                "Detail Footer",
                                "detail_footer",
                                "detail footer expression",
                                "2",
                                "1",
                                "1",
                                "10",
                                "300",
                                "250",
                                "550",
                                "detail footer");
        };

    run_detail_header_footer_stable_expression_selection(
        temp_root / "detail_header_footer_stable_expression.frx",
        "detail_header_footer_stable_expression.frx",
        "report");
    run_detail_header_footer_stable_expression_selection(
        temp_root / "detail_header_footer_stable_expression.lbx",
        "detail_header_footer_stable_expression.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& section_index,
                    const std::string& object_code,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1810: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#1810: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 2",
                                    "#1810: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                                    "#1810: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#1927: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#1927: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#1927: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#1927: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#1927: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#1927: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#1927: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#1927: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview left bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                                    "#1927: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview right bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                                    "#1927: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                    "#1927: " + operation_label + " should preserve deleted preview widths");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                                    "#1927: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1810: " + operation_label + " should preserve section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1810: " + operation_label + " should preserve selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1810: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1810: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2263: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2263: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2263: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                                    "#2263: " + operation_label + " JSON should expose expression undo labels");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": 2",
                            "\"objectCode\": " + object_code,
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#1810: " + operation_label + " should refresh selected-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "detail header stable updated",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1810: stable-selected detail-header expression update should exit successfully");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "detail header stable updated",
                   "#1810: stable-selected detail-header expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_header_process,
                                               "Detail Header",
                                               "detail_header",
                                               "detail header stable updated",
                                               "2",
                                               "4",
                                               "0",
                                               "0",
                                               "9",
                                               "0",
                                               "300",
                                               "300",
                                               "stable-selected detail-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"detail footer expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 2",
                    "\"recordIndex\": 1"
                },
                "#1810: stable-selected detail-header expression update should preserve sibling footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "detail footer stable updated",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1810: stable-selected detail-footer expression update should exit successfully");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "detail footer stable updated",
                   "#1810: stable-selected detail-footer expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_footer_process,
                                               "Detail Footer",
                                               "detail_footer",
                                               "detail footer stable updated",
                                               "2",
                                               "5",
                                               "1",
                                               "1",
                                               "10",
                                               "300",
                                               "250",
                                               "550",
                                               "stable-selected detail-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"detail header stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 0"
                },
                "#1810: stable-selected detail-footer expression update should preserve sibling header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1810: stable-selected detail-header expression clear should exit successfully");
            expect_selected_section_expression(clear_header_process,
                                               "Detail Header",
                                               "detail_header",
                                               "",
                                               "null",
                                               "0",
                                               "0",
                                               "0",
                                               "9",
                                               "0",
                                               "300",
                                               "300",
                                               "stable-selected detail-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"detail footer stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 1"
                },
                "#1810: stable-selected detail-header expression clear should preserve sibling footer metadata");
            expect_not_contains(clear_header_process.stdout_text,
                                "\"expression\": \"detail header stable updated\"",
                                "#1810: stable-selected detail-header expression clear should not leak stale selected expressions");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable detail-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable detail-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1810: stable-selected detail-footer expression clear should exit successfully");
            expect_selected_section_expression(clear_footer_process,
                                               "Detail Footer",
                                               "detail_footer",
                                               "",
                                               "null",
                                               "0",
                                               "1",
                                               "1",
                                               "10",
                                               "300",
                                               "250",
                                               "550",
                                               "stable-selected detail-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 0"
                },
                "#1810: stable-selected detail-footer expression clear should preserve sibling header metadata");
            expect_not_contains(clear_footer_process.stdout_text,
                                "\"expression\": \"detail footer stable updated\"",
                                "#1810: stable-selected detail-footer expression clear should not leak stale selected expressions");
        };

    run_detail_header_footer_section_expression_stable_edits(
        temp_root / "detail_header_footer_section_stable_expression_edits.frx",
        "detail_header_footer_section_stable_expression_edits.frx",
        "report");
    run_detail_header_footer_section_expression_stable_edits(
        temp_root / "detail_header_footer_section_stable_expression_edits.lbx",
        "detail_header_footer_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_expression_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "1",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail header updated",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1766: deleted detail-header report/label expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2264: deleted detail-header expression update should preserve deleted state");
            const auto updated_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = {},
                .property_name = "EXPR"
            });
            expect(updated_expr.ok && updated_expr.exists &&
                       updated_expr.value == "deleted detail header updated",
                   "#1766: deleted detail-header expression update should persist the EXPR memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1766: deleted detail-header expression update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1766: deleted detail-header expression update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"sectionCount\": 1",
                            "#1766: deleted detail-header expression update should preserve live section count");
            expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1766: deleted detail-header expression update should preserve deleted section count");
            expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1766: deleted detail-header expression update should preserve section selection");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1766: deleted detail-header expression update should preserve selection kind");
            expect_contains(update_process.stdout_text, "\"dryRun\": false",
                            "#2264: deleted detail-header expression update JSON should expose committed execution");
            expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                            "#2264: deleted detail-header expression update JSON should expose mutation state");
            expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                            "#2264: deleted detail-header expression update JSON should expose undo availability");
            expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2264: deleted detail-header expression update JSON should expose expression undo labels");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"deleted detail footer expression\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2"
                },
                "#1766: deleted detail-header update should refresh deleted-section expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"objectCodeFieldIndex\": 1",
                    "\"objectCodeMemoBlockNumber\": 0",
                    "\"top\": 500",
                    "\"height\": 300",
                    "\"bottom\": 800"
                },
                "#1766: deleted detail-header update should refresh selected-section expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"title\": \"Detail\"",
                    "\"bandKind\": \"detail\"",
                    "\"expression\": \"live detail expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 1",
                    "\"recordIndex\": 0"
                },
                "#1766: deleted detail-header expression update should preserve live detail metadata");

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
                std::cerr << "studio host " << label << " deleted detail-footer expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1766: deleted detail-footer report/label expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#2264: deleted detail-footer expression clear should preserve deleted state");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1766: deleted detail-footer expression clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1766: deleted detail-footer expression clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"sectionCount\": 1",
                            "#2264: deleted detail-footer expression clear should preserve live section count");
            expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#2264: deleted detail-footer expression clear should preserve deleted section count");
            expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1766: deleted detail-footer expression clear should preserve section selection");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1766: deleted detail-footer expression clear should preserve selection kind");
            expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                            "#2264: deleted detail-footer expression clear JSON should expose committed execution");
            expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                            "#2264: deleted detail-footer expression clear JSON should expose mutation state");
            expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                            "#2264: deleted detail-footer expression clear JSON should expose undo availability");
            expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2264: deleted detail-footer expression clear JSON should expose expression undo labels");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"expression\": \"deleted detail header updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2"
                },
                "#1766: deleted detail-footer clear should refresh deleted-section expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"objectCodeFieldIndex\": 1",
                    "\"objectCodeMemoBlockNumber\": 0",
                    "\"top\": 800",
                    "\"height\": 250",
                    "\"bottom\": 1050"
                },
                "#1766: deleted detail-footer clear should refresh selected-section expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"title\": \"Detail\"",
                    "\"bandKind\": \"detail\"",
                    "\"expression\": \"live detail expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 1",
                    "\"recordIndex\": 0"
                },
                "#1766: deleted detail-footer expression clear should preserve live detail metadata");
        };

    run_deleted_detail_header_footer_expression_edits(
        temp_root / "deleted_detail_header_footer_expression.frx",
        "deleted_detail_header_footer_expression.frx",
        "report");
    run_deleted_detail_header_footer_expression_edits(
        temp_root / "deleted_detail_header_footer_expression.lbx",
        "deleted_detail_header_footer_expression.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& object_code,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1811: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#1811: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                    "#1811: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                    "#1811: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#1928: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#1928: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#1928: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#1928: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 500",
                                    "#1928: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#1928: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 500",
                                    "#1928: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#1928: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview left bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                    "#1928: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview right bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                    "#1928: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                    "#1928: " + operation_label + " should preserve deleted preview widths");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                    "#1928: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1811: " + operation_label + " should preserve deleted section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1811: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1811: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1811: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2265: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2265: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2265: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                                    "#2265: " + operation_label + " JSON should expose expression undo labels");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"objectCode\": " + object_code,
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#1811: " + operation_label + " should refresh selected deleted-section expression metadata");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"sections\": [",
                            "\"title\": \"Detail\"",
                            "\"bandKind\": \"detail\"",
                            "\"expression\": \"live detail expression\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": 1",
                            "\"recordIndex\": 0"
                        },
                        "#1811: " + operation_label + " should preserve live detail metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail header stable updated",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-header expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1811: stable-selected deleted detail-header expression update should preserve deleted state");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "deleted detail header stable updated",
                   "#1811: stable-selected deleted detail-header expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_header_process,
                                                       "Detail Header",
                                                       "detail_header",
                                                       "deleted detail header stable updated",
                                                       "2",
                                                       "4",
                                                       "1",
                                                       "9",
                                                       "500",
                                                       "300",
                                                       "800",
                                                       "stable-selected deleted detail-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"deleted detail header stable updated\"",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer expression\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-header update should preserve deleted footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "deleted detail footer stable updated",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-footer expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1811: stable-selected deleted detail-footer expression update should preserve deleted state");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "deleted detail footer stable updated",
                   "#1811: stable-selected deleted detail-footer expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_footer_process,
                                                       "Detail Footer",
                                                       "detail_footer",
                                                       "deleted detail footer stable updated",
                                                       "2",
                                                       "5",
                                                       "2",
                                                       "10",
                                                       "800",
                                                       "250",
                                                       "1050",
                                                       "stable-selected deleted detail-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"deleted detail header stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer stable updated\"",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-footer update should preserve deleted header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-header expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1811: stable-selected deleted detail-header expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_header_process,
                                                       "Detail Header",
                                                       "detail_header",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "1",
                                                       "9",
                                                       "500",
                                                       "300",
                                                       "800",
                                                       "stable-selected deleted detail-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"deleted detail footer stable updated\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-header clear should preserve deleted footer metadata");
            expect_not_contains(clear_header_process.stdout_text,
                                "\"expression\": \"deleted detail header stable updated\"",
                                "#1811: stable-selected deleted detail-header clear should not leak stale selected expressions");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted detail-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted detail-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1811: stable-selected deleted detail-footer expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1811: stable-selected deleted detail-footer expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_footer_process,
                                                       "Detail Footer",
                                                       "detail_footer",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "2",
                                                       "10",
                                                       "800",
                                                       "250",
                                                       "1050",
                                                       "stable-selected deleted detail-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"title\": \"Detail Footer\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 2"
                },
                "#1811: stable-selected deleted detail-footer clear should preserve deleted header metadata");
            expect_not_contains(clear_footer_process.stdout_text,
                                "\"expression\": \"deleted detail footer stable updated\"",
                                "#1811: stable-selected deleted detail-footer clear should not leak stale selected expressions");
        };

    run_deleted_detail_header_footer_section_expression_stable_edits(
        temp_root / "deleted_detail_header_footer_section_stable_expression_edits.frx",
        "deleted_detail_header_footer_section_stable_expression_edits.frx",
        "report");
    run_deleted_detail_header_footer_section_expression_stable_edits(
        temp_root / "deleted_detail_header_footer_section_stable_expression_edits.lbx",
        "deleted_detail_header_footer_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_stable_section_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_stable_selection =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto expect_deleted_section = [&](const std::string& unique_id,
                                                    const std::string& section_title,
                                                    const std::string& band_kind,
                                                    const std::string& expression,
                                                    const std::string& memo_block,
                                                    const std::string& record_index,
                                                    const std::string& object_code,
                                                    const std::string& top,
                                                    const std::string& height,
                                                    const std::string& bottom,
                                                    const std::string& selection_label) {
                const auto process = run_process_capture(
                    studio_host_path,
                    {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                    temp_root);

                if (process.exit_code != 0) {
                    std::cerr << "studio host " << label << " stable deleted " << selection_label
                              << " stdout:\n" << process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " stable deleted " << selection_label
                              << " stderr:\n" << process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(process.exit_code == 0,
                       "#1767: stable deleted detail header/footer section selection should exit successfully");
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                "#1767: stable deleted detail header/footer selection should preserve document titles");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1767: stable deleted detail header/footer label selection should retain label identity");
                }
                expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                "#1767: stable deleted detail header/footer selection should preserve live sections");
                expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                "#1767: stable deleted detail header/footer selection should expose deleted sections");
                expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                "#2266: stable deleted detail header/footer selection should preserve live preview availability");
                expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview left bounds");
                expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview top bounds");
                expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview right bounds");
                expect_contains(process.stdout_text, "\"previewBoundsBottom\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve live preview bottom bounds");
                expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve live preview widths");
                expect_contains(process.stdout_text, "\"previewBoundsHeight\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve live preview heights");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview availability");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview left bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview top bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview right bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview bottom bounds");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview widths");
                expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                                "#2266: stable deleted detail header/footer selection should preserve deleted preview heights");
                expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                "#1767: stable deleted detail header/footer selection should expose selected sections");
                expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                "#1767: stable deleted detail header/footer selection should expose section selections");
                expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not select report objects");
                expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not fabricate object sections");
                expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                "#1767: stable deleted detail header/footer selection should not select settings");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1767: stable deleted " + selection_label + " should expose deleted-section metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"selectedReportSection\": {",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"expression\": \"" + expression + "\"",
                        "\"expressionFieldIndex\": 2",
                        "\"expressionMemoBlockNumber\": " + memo_block,
                        "\"recordIndex\": " + record_index,
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCode\": " + object_code,
                        "\"top\": " + top,
                        "\"height\": " + height,
                        "\"bottom\": " + bottom
                    },
                    "#1767: stable deleted " + selection_label + " should expose selected-section metadata");
                expect_contains_in_order(
                    process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"Detail\"",
                        "\"bandKind\": \"detail\"",
                        "\"expression\": \"live detail expression\"",
                        "\"recordIndex\": 0"
                    },
                    "#1767: stable deleted " + selection_label + " should preserve live detail metadata");
            };

            expect_deleted_section("deleted-detail-header-guid",
                                   "Detail Header",
                                   "detail_header",
                                   "deleted detail header expression",
                                   "2",
                                   "1",
                                   "9",
                                   "500",
                                   "300",
                                   "800",
                                   "detail header");
            expect_deleted_section("deleted-detail-footer-guid",
                                   "Detail Footer",
                                   "detail_footer",
                                   "deleted detail footer expression",
                                   "3",
                                   "2",
                                   "10",
                                   "800",
                                   "250",
                                   "1050",
                                   "detail footer");
        };

    run_deleted_detail_header_footer_stable_selection(
        temp_root / "deleted_detail_header_footer_stable_sections.frx",
        "deleted_detail_header_footer_stable_sections.frx",
        "report");
    run_deleted_detail_header_footer_stable_selection(
        temp_root / "deleted_detail_header_footer_stable_sections.lbx",
        "deleted_detail_header_footer_stable_sections.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
