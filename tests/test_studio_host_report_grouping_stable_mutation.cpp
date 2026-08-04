// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

}  // namespace

void test_studio_host_json_updates_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_group_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_stable_group_section_expression_fixture(asset_path);

            const auto expect_selected_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_id,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& section_index,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2683: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2683: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 3",
                                    "#2683: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 0",
                                    "#2683: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                                    "#2683: " + operation_label + " should preserve live preview left bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2683: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                                    "#2683: " + operation_label + " should preserve live preview right bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 4100",
                                    "#2683: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                                    "#2683: " + operation_label + " should preserve live preview widths");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 4100",
                                    "#2683: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2683: " + operation_label + " should not fabricate deleted preview bounds");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2683: " + operation_label + " should preserve section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2683: " + operation_label + " should preserve selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2683: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2683: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2683: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2683: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2683: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"" + section_id + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": 3",
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#2683: " + operation_label + " should refresh selected-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#2683: stable-selected group-header expression update should exit successfully");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "group-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "customer.region",
                   "#2683: stable-selected group-header expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_header_process,
                                               "group-header-guid",
                                               "group_header",
                                               "customer.region",
                                               "2",
                                               "4",
                                               "1",
                                               "0",
                                               "0",
                                               "600",
                                               "600",
                                               "stable-selected group-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-header expression update should preserve sibling footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#2683: stable-selected group-footer expression update should exit successfully");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 3U,
                .object_name = {},
                .unique_id = "group-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "customer.region",
                   "#2683: stable-selected group-footer expression update should persist the EXPR memo field");
            expect_selected_section_expression(update_footer_process,
                                               "group-footer-guid",
                                               "group_footer",
                                               "customer.region",
                                               "2",
                                               "5",
                                               "3",
                                               "2",
                                               "3600",
                                               "500",
                                               "4100",
                                               "stable-selected group-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-footer expression update should preserve sibling header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#2683: stable-selected group-header expression clear should exit successfully");
            expect_selected_section_expression(clear_header_process,
                                               "group-header-guid",
                                               "group_header",
                                               "",
                                               "null",
                                               "0",
                                               "1",
                                               "0",
                                               "0",
                                               "600",
                                               "600",
                                               "stable-selected group-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-header expression clear should preserve sibling footer metadata");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable group-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable group-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#2683: stable-selected group-footer expression clear should exit successfully");
            expect_selected_section_expression(clear_footer_process,
                                               "group-footer-guid",
                                               "group_footer",
                                               "",
                                               "null",
                                               "0",
                                               "3",
                                               "2",
                                               "3600",
                                               "500",
                                               "4100",
                                               "stable-selected group-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"sections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"bandKind\": \"detail\"",
                    "\"recordIndex\": 2",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected group-footer expression clear should preserve sibling header metadata");
        };

    run_group_section_expression_stable_edits(temp_root / "group_section_stable_expression_edits.frx",
                                              "group_section_stable_expression_edits.frx",
                                              "report");
    run_group_section_expression_stable_edits(temp_root / "group_section_stable_expression_edits.lbx",
                                              "group_section_stable_expression_edits.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_group_section_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_group_section_stable_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_group_section_expression_stable_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_stable_group_section_expression_fixture(asset_path);

            const auto delete_header_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_result.ok && dbf_record_deleted(asset_path, 1U),
                   "#2683: deleted group-header fixture should mark the group-header section deleted");
            const auto delete_footer_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_result.ok && dbf_record_deleted(asset_path, 3U),
                   "#2683: deleted group-footer fixture should mark the group-footer section deleted");

            const auto expect_selected_deleted_section_expression =
                [&](const ProcessResult& process,
                    const std::string& section_id,
                    const std::string& band_kind,
                    const std::string& expression,
                    const std::string& field_index,
                    const std::string& memo_block,
                    const std::string& record_index,
                    const std::string& top,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#2683: " + operation_label + " should return refreshed layout JSON");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(process.stdout_text, "\"isLabel\": true",
                                        "#2683: " + operation_label + " should retain label identity");
                    }
                    expect_contains(process.stdout_text, "\"sectionCount\": 1",
                                    "#2683: " + operation_label + " should preserve live section count");
                    expect_contains(process.stdout_text, "\"deletedSectionCount\": 2",
                                    "#2683: " + operation_label + " should preserve deleted section count");
                    expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve live preview availability");
                    expect_contains(process.stdout_text, "\"previewBoundsTop\": 600",
                                    "#2683: " + operation_label + " should preserve live preview top bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3600",
                                    "#2683: " + operation_label + " should preserve live preview bottom bounds");
                    expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3000",
                                    "#2683: " + operation_label + " should preserve live preview heights");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2683: " + operation_label + " should preserve deleted preview availability");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                                    "#2683: " + operation_label + " should preserve deleted preview top bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 4100",
                                    "#2683: " + operation_label + " should preserve deleted preview bottom bounds");
                    expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 4100",
                                    "#2683: " + operation_label + " should preserve deleted preview heights");
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#2683: " + operation_label + " should preserve deleted section selection");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#2683: " + operation_label + " should preserve section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#2683: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#2683: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2683: " + operation_label + " JSON should expose committed execution");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2683: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": true",
                                    "#2683: " + operation_label + " JSON should expose undo availability");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"id\": \"" + section_id + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": " + field_index,
                            "\"expressionMemoBlockNumber\": " + memo_block,
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"sectionIndex\": null",
                            "\"sectionCount\": 0",
                            "\"top\": " + top,
                            "\"height\": " + height,
                            "\"bottom\": " + bottom
                        },
                        "#2683: " + operation_label + " should refresh selected deleted-section expression metadata");
                };

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-header expression update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-header expression update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#2683: stable-selected deleted group-header expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2683: stable-selected deleted group-header expression update should preserve deleted state");
            const auto updated_header_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "group-header-guid",
                .property_name = "EXPR"
            });
            expect(updated_header_expr.ok && updated_header_expr.exists &&
                       updated_header_expr.value == "customer.region",
                   "#2683: stable-selected deleted group-header expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_header_process,
                                                       "group-header-guid",
                                                       "group_header",
                                                       "customer.region",
                                                       "2",
                                                       "4",
                                                       "1",
                                                       "0",
                                                       "600",
                                                       "600",
                                                       "stable-selected deleted group-header expression update");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.country\"",
                    "\"expressionMemoBlockNumber\": 3",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-header update should preserve deleted footer metadata");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--property-value", "customer.region",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-footer expression update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-footer expression update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#2683: stable-selected deleted group-footer expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#2683: stable-selected deleted group-footer expression update should preserve deleted state");
            const auto updated_footer_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 3U,
                .object_name = {},
                .unique_id = "group-footer-guid",
                .property_name = "EXPR"
            });
            expect(updated_footer_expr.ok && updated_footer_expr.exists &&
                       updated_footer_expr.value == "customer.region",
                   "#2683: stable-selected deleted group-footer expression update should persist the EXPR memo field");
            expect_selected_deleted_section_expression(update_footer_process,
                                                       "group-footer-guid",
                                                       "group_footer",
                                                       "customer.region",
                                                       "2",
                                                       "5",
                                                       "3",
                                                       "3600",
                                                       "500",
                                                       "4100",
                                                       "stable-selected deleted group-footer expression update");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 4",
                    "\"recordIndex\": 1",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-footer update should preserve deleted header metadata");

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-header-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-header expression clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-header expression clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#2683: stable-selected deleted group-header expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#2683: stable-selected deleted group-header expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_header_process,
                                                       "group-header-guid",
                                                       "group_header",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "1",
                                                       "0",
                                                       "600",
                                                       "600",
                                                       "stable-selected deleted group-header expression clear");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"customer.region\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-header clear should preserve deleted footer metadata");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "group-footer-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted group-footer expression clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted group-footer expression clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#2683: stable-selected deleted group-footer expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#2683: stable-selected deleted group-footer expression clear should preserve deleted state");
            expect_selected_deleted_section_expression(clear_footer_process,
                                                       "group-footer-guid",
                                                       "group_footer",
                                                       "",
                                                       "null",
                                                       "0",
                                                       "3",
                                                       "3600",
                                                       "500",
                                                       "4100",
                                                       "stable-selected deleted group-footer expression clear");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"id\": \"group-header-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 1",
                    "\"id\": \"group-footer-guid\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": null",
                    "\"expressionMemoBlockNumber\": 0",
                    "\"recordIndex\": 3"
                },
                "#2683: stable-selected deleted group-footer clear should preserve deleted header metadata");
        };

    run_deleted_group_section_expression_stable_edits(
        temp_root / "deleted_group_section_stable_expression_edits.frx",
        "deleted_group_section_stable_expression_edits.frx",
        "report");
    run_deleted_group_section_expression_stable_edits(
        temp_root / "deleted_group_section_stable_expression_edits.lbx",
        "deleted_group_section_stable_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_grouping_stable_mutation <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_group_section_expressions_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_group_section_expressions_by_stable_selection(argv[1]);

    if (cf_test_studio_host_json::failures != 0) {
        return 1;
    }

    return 0;
}
