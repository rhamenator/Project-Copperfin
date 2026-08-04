// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_detail_header_footer_object_containment(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_objects = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " detail header/footer object summary stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " detail header/footer object summary stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1764: detail header/footer object containment JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1764: detail header/footer object containment JSON should return report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1764: detail header/footer object label layouts should retain label identity");
        }
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1764: detail header/footer object JSON should summarize live sections");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 2",
                        "#1764: detail header/footer object JSON should summarize live objects");
        expect_contains(process.stdout_text, "\"placedObjectCount\": 2",
                        "#1764: detail header/footer object JSON should count both objects as placed");
        expect_contains(process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1764: detail header/footer object JSON should not fabricate unplaced objects");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2310: detail header/footer object JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2310: detail header/footer object JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 550",
                        "#2310: detail header/footer object JSON should preserve live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 550",
                        "#2310: detail header/footer object JSON should preserve live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#2310: detail header/footer object JSON should not fabricate deleted preview bounds");
        expect_contains(process.stdout_text,
                        "\"sectionKindCounts\": [\n"
                        "        {\"kind\": \"detail_footer\", \"count\": 1},\n"
                        "        {\"kind\": \"detail_header\", \"count\": 1}\n"
                        "      ]",
                        "#1764: detail header/footer object JSON should preserve section-kind buckets");

        const auto expect_selected_object = [&](const std::string& unique_id,
                                                const std::string& record_index,
                                                const std::string& object_kind,
                                                const std::string& containing_section_id,
                                                const std::string& containing_section_record_index,
                                                const std::string& relative_top,
                                                const std::string& relative_bottom,
                                                const std::string& selection_label) {
            const auto object_process = run_process_capture(
                studio_host_path,
                {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                temp_root);

            if (object_process.exit_code != 0) {
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stdout:\n" << object_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " selected " << selection_label
                          << " stderr:\n" << object_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(object_process.exit_code == 0,
                   "#1764: selected detail header/footer object JSON should exit successfully");
            expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1764: selected detail header/footer object JSON should advertise selected objects");
            expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1764: selected detail header/footer object JSON should expose object selection kind");
            expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1764: selected detail header/footer object JSON should expose containing sections");
            expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2310: selected detail header/footer object JSON should preserve live preview availability");
            expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2310: selected detail header/footer object JSON should preserve live preview top bounds");
            expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2310: selected detail header/footer object JSON should preserve live preview bottom bounds");
            expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2310: selected detail header/footer object JSON should preserve live preview heights");
            expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2310: selected detail header/footer object JSON should not fabricate deleted preview bounds");
            expect_contains_in_order(
                object_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": " + record_index,
                    "\"deleted\": false",
                    "\"containingSectionId\": \"" + containing_section_id + "\"",
                    "\"containingSectionRecordIndex\": " + containing_section_record_index,
                    "\"sectionRelativeTop\": " + relative_top,
                    "\"sectionRelativeBottom\": " + relative_bottom,
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"" + object_kind + "\""
                },
                "#1764: selected " + selection_label + " JSON should expose containing-section metadata");
            expect_contains_in_order(
                object_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"" + containing_section_id + "\"",
                    "\"recordIndex\": " + containing_section_record_index,
                    "\"sectionIndex\": ",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1764: selected " + selection_label + " JSON should expose containing-section object counts");
        };

        expect_selected_object("detail-header-label-guid",
                               "1",
                               "label",
                               "detail-header-guid",
                               "0",
                               "50",
                               "170",
                               "detail-header label");
        expect_selected_object("detail-footer-field-guid",
                               "3",
                               "field",
                               "detail-footer-guid",
                               "2",
                               "60",
                               "160",
                               "detail-footer field");
    };

    run_detail_header_footer_objects(temp_root / "detail_header_footer_objects.frx",
                                     "detail_header_footer_objects.frx",
                                     "report");
    run_detail_header_footer_objects(temp_root / "detail_header_footer_objects.lbx",
                                     "detail_header_footer_objects.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_expressions =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto expect_selected_object_expression =
                [&](const std::string& unique_id,
                    const std::string& record_index,
                    const std::string& object_kind,
                    const std::string& containing_section_id,
                    const std::string& containing_section_record_index,
                    const std::string& relative_top,
                    const std::string& relative_bottom,
                    const std::string& expression,
                    const std::string& memo_block,
                    const std::string& selection_label) {
                    const auto object_process = run_process_capture(
                        studio_host_path,
                        {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                        temp_root);

                    if (object_process.exit_code != 0) {
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " expression stdout:\n" << object_process.stdout_text << "\n";
                        std::cerr << "studio host " << label << " stable selected " << selection_label
                                  << " expression stderr:\n" << object_process.stderr_text << "\n";
                        std::cerr << "fixture root: " << temp_root << "\n";
                    }

                    expect(object_process.exit_code == 0,
                           "#1769: selected detail header/footer object expression JSON should exit successfully");
                    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1769: selected detail header/footer object expression JSON should preserve titles");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                                        "#1769: selected detail header/footer label object expression JSON should retain identity");
                    }
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                                    "#1769: selected detail header/footer object expressions should select report objects");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                                    "#1769: selected detail header/footer object expressions should expose object selection kind");
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                                    "#1769: selected detail header/footer object expressions should expose containing sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                                    "#1769: selected detail header/footer object expressions should not select sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1769: selected detail header/footer object expressions should not select settings");
                    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2279: selected detail header/footer object expressions should preserve live preview availability");
                    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2279: selected detail header/footer object expressions should preserve live preview top bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#2279: selected detail header/footer object expressions should preserve live preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#2279: selected detail header/footer object expressions should preserve live preview heights");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                                    "#2279: selected detail header/footer object expressions should not fabricate deleted preview availability");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObject\": {",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": false",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + memo_block
                        },
                        "#1769: stable selected " + selection_label + " should expose selected-object expression metadata");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObjectSection\": {",
                            "\"id\": \"" + containing_section_id + "\"",
                            "\"recordIndex\": " + containing_section_record_index,
                            "\"sectionCount\": 2",
                            "\"objectCount\": 1"
                        },
                        "#1769: stable selected " + selection_label + " should expose containing-section metadata");
                };

            expect_selected_object_expression("detail-header-label-guid",
                                              "1",
                                              "label",
                                              "detail-header-guid",
                                              "0",
                                              "50",
                                              "170",
                                              "\\\"Header label\\\"",
                                              "2",
                                              "detail-header label");
            expect_selected_object_expression("detail-footer-field-guid",
                                              "3",
                                              "field",
                                              "detail-footer-guid",
                                              "2",
                                              "60",
                                              "160",
                                              "footer.total",
                                              "4",
                                              "detail-footer field");
        };

    run_detail_header_footer_object_expressions(temp_root / "detail_header_footer_object_expressions.frx",
                                                "detail_header_footer_object_expressions.frx",
                                                "report");
    run_detail_header_footer_object_expressions(temp_root / "detail_header_footer_object_expressions.lbx",
                                                "detail_header_footer_object_expressions.lbx",
                                                "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_detail_header_footer_object_expression_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "EXPR",
                    "--property-value", "\"Updated header label\"",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1770: detail-header object expression update should exit successfully");
            const auto updated_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "EXPR"
            });
            expect(updated_expr.ok && updated_expr.exists && updated_expr.value == "\"Updated header label\"",
                   "#1770: detail-header object expression update should persist the EXPR memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1770: detail-header object expression update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1770: detail-header label object expression update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1770: detail-header object expression update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1770: detail-header object expression update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1770: detail-header object expression update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2307: detail-header object expression update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2307: detail-header object expression update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2307: detail-header object expression update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2307: detail-header object expression update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2307: detail-header object expression update should not fabricate deleted preview bounds");
            expect_contains(update_process.stdout_text, "\"dryRun\": false",
                            "#2260: detail-header object expression update JSON should expose committed execution");
            expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                            "#2260: detail-header object expression update JSON should expose mutation state");
            expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                            "#2260: detail-header object expression update JSON should expose undo availability");
            expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2260: detail-header object expression update JSON should expose expression undo labels");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Updated header label\\\"\"",
                    "\"expressionFieldIndex\": 2",
                    "\"expressionMemoBlockNumber\": 5"
                },
                "#1770: detail-header object expression update should refresh selected-object expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1770: detail-header object expression update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1770: detail-footer object expression clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                            "#1770: detail-footer object expression clear should blank the EXPR memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1770: detail-footer object expression clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1770: detail-footer label object expression clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1770: detail-footer object expression clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1770: detail-footer object expression clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1770: detail-footer object expression clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2307: detail-footer object expression clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2307: detail-footer object expression clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2307: detail-footer object expression clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2307: detail-footer object expression clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2307: detail-footer object expression clear should not fabricate deleted preview bounds");
            expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                            "#2260: detail-footer object expression clear JSON should expose committed execution");
            expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                            "#2260: detail-footer object expression clear JSON should expose mutation state");
            expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                            "#2260: detail-footer object expression clear JSON should expose undo availability");
            expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2260: detail-footer object expression clear JSON should expose expression undo labels");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": 2"
                },
                "#1770: detail-footer object expression clear should refresh selected-object expression metadata");
            expect_not_contains(clear_process.stdout_text, "\"expression\": \"footer.total\"",
                                "#1770: detail-footer object expression clear should not leak stale expressions");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1770: detail-footer object expression clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_expression_edits(temp_root / "detail_header_footer_object_expression_edits.frx",
                                                     "detail_header_footer_object_expression_edits.frx",
                                                     "report");
    run_detail_header_footer_object_expression_edits(temp_root / "detail_header_footer_object_expression_edits.lbx",
                                                     "detail_header_footer_object_expression_edits.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_deleted_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_expression_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_object_expressions =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1771: detail-header object expression fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1771: detail-footer object expression fixture should mark the footer object deleted");

            const auto expect_deleted_selected_object_expression =
                [&](const std::string& unique_id,
                    const std::string& record_index,
                    const std::string& object_kind,
                    const std::string& containing_section_id,
                    const std::string& containing_section_record_index,
                    const std::string& relative_top,
                    const std::string& relative_bottom,
                    const std::string& expression,
                    const std::string& memo_block,
                    const std::string& left,
                    const std::string& top,
                    const std::string& width,
                    const std::string& right,
                    const std::string& height,
                    const std::string& bottom,
                    const std::string& selection_label) {
                    const auto object_process = run_process_capture(
                        studio_host_path,
                        {"--path", asset_path.string(), "--unique-id", unique_id, "--json"},
                        temp_root);

                    if (object_process.exit_code != 0) {
                        std::cerr << "studio host " << label << " stable selected deleted " << selection_label
                                  << " expression stdout:\n" << object_process.stdout_text << "\n";
                        std::cerr << "studio host " << label << " stable selected deleted " << selection_label
                                  << " expression stderr:\n" << object_process.stderr_text << "\n";
                        std::cerr << "fixture root: " << temp_root << "\n";
                    }

                    expect(object_process.exit_code == 0,
                           "#1771: selected deleted detail header/footer object expression JSON should exit successfully");
                    expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                                    "#1771: selected deleted detail header/footer object expression JSON should preserve titles");
                    if (asset_path.extension() == ".lbx") {
                        expect_contains(object_process.stdout_text, "\"isLabel\": true",
                                        "#1771: selected deleted detail header/footer label object expression JSON should retain identity");
                    }
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                                    "#1771: selected deleted detail header/footer object expressions should select report objects");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                                    "#1771: selected deleted detail header/footer object expressions should advertise report selection");
                    expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                                    "#1771: selected deleted detail header/footer object expressions should expose object selection kind");
                    expect_contains(object_process.stdout_text, "\"liveObjectCount\": 0",
                                    "#1771: selected deleted detail header/footer object expressions should remove live object counts");
                    expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 2",
                                    "#1771: selected deleted detail header/footer object expressions should preserve deleted object counts");
                    expect_contains(object_process.stdout_text, "\"deletedPlacedObjectCount\": 2",
                                    "#1771: selected deleted detail header/footer object expressions should expose deleted placed object counts");
                    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                                    "#1771: selected deleted detail header/footer object expressions should not select sections");
                    expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1771: selected deleted detail header/footer object expressions should not select settings");
                    expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                                    "#1771: selected deleted detail header/footer object expressions should preserve containing sections");
                    expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                                    "#2309: selected deleted detail header/footer object expressions should preserve live preview availability");
                    expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                                    "#2309: selected deleted detail header/footer object expressions should preserve live preview top bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 550",
                                    "#2309: selected deleted detail header/footer object expressions should preserve live preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 550",
                                    "#2309: selected deleted detail header/footer object expressions should preserve live preview heights");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                                    "#2309: selected deleted detail header/footer object expressions should expose deleted preview availability");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                                    "#2309: selected deleted detail header/footer object expressions should preserve deleted preview top bounds");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                                    "#2309: selected deleted detail header/footer object expressions should preserve deleted preview bottom bounds");
                    expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                                    "#2309: selected deleted detail header/footer object expressions should preserve deleted preview heights");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"deletedObjects\": [",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + memo_block
                        },
                        "#1771: stable selected deleted " + selection_label + " should expose deleted-object expression metadata");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObject\": {",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": true",
                            "\"containingSectionId\": \"" + containing_section_id + "\"",
                            "\"containingSectionRecordIndex\": " + containing_section_record_index,
                            "\"sectionRelativeTop\": " + relative_top,
                            "\"sectionRelativeBottom\": " + relative_bottom,
                            "\"sectionObjectIndex\": 0",
                            "\"sectionObjectCount\": 1",
                            "\"objectKind\": \"" + object_kind + "\"",
                            "\"expression\": \"" + expression + "\"",
                            "\"expressionFieldIndex\": 2",
                            "\"expressionMemoBlockNumber\": " + memo_block
                        },
                        "#1771: stable selected deleted " + selection_label + " should expose selected-object expression metadata");
                    expect_contains(object_process.stdout_text, "\"left\": " + left,
                                    "#1771: selected deleted detail header/footer object expressions should expose left bounds");
                    expect_contains(object_process.stdout_text, "\"top\": " + top,
                                    "#1771: selected deleted detail header/footer object expressions should expose top bounds");
                    expect_contains(object_process.stdout_text, "\"width\": " + width,
                                    "#1771: selected deleted detail header/footer object expressions should expose widths");
                    expect_contains(object_process.stdout_text, "\"right\": " + right,
                                    "#1771: selected deleted detail header/footer object expressions should expose right bounds");
                    expect_contains(object_process.stdout_text, "\"height\": " + height,
                                    "#1771: selected deleted detail header/footer object expressions should expose heights");
                    expect_contains(object_process.stdout_text, "\"bottom\": " + bottom,
                                    "#1771: selected deleted detail header/footer object expressions should expose bottom bounds");
                    expect_contains(object_process.stdout_text,
                                    "\"name\": \"EXPR\", \"recordIndex\": " + record_index +
                                        ", \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": " +
                                        memo_block + ", \"value\": \"" + expression + "\"",
                                    "#1771: stable selected deleted " + selection_label +
                                        " should expose selected-object expression provenance");
                    expect_contains_in_order(
                        object_process.stdout_text,
                        {
                            "\"selectedReportObjectSection\": {",
                            "\"id\": \"" + containing_section_id + "\"",
                            "\"recordIndex\": " + containing_section_record_index,
                            "\"sectionCount\": 2",
                            "\"objectCount\": 0",
                            "\"deletedObjectCount\": 1"
                        },
                        "#1771: stable selected deleted " + selection_label +
                            " should expose containing-section metadata");
                };

            expect_deleted_selected_object_expression("detail-header-label-guid",
                                                      "1",
                                                      "label",
                                                      "detail-header-guid",
                                                      "0",
                                                      "50",
                                                      "170",
                                                      "\\\"Header label\\\"",
                                                      "2",
                                                      "100",
                                                      "50",
                                                      "700",
                                                      "800",
                                                      "120",
                                                      "170",
                                                      "detail-header label");
            expect_deleted_selected_object_expression("detail-footer-field-guid",
                                                      "3",
                                                      "field",
                                                      "detail-footer-guid",
                                                      "2",
                                                      "60",
                                                      "160",
                                                      "footer.total",
                                                      "4",
                                                      "140",
                                                      "360",
                                                      "900",
                                                      "1040",
                                                      "100",
                                                      "460",
                                                      "detail-footer field");
        };

    run_deleted_detail_header_footer_object_expressions(
        temp_root / "deleted_detail_header_footer_object_expressions.frx",
        "deleted_detail_header_footer_object_expressions.frx",
        "report");
    run_deleted_detail_header_footer_object_expressions(
        temp_root / "deleted_detail_header_footer_object_expressions.lbx",
        "deleted_detail_header_footer_object_expressions.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_expressions_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_object_expression_edit_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_detail_header_footer_object_expression_edits =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1772: deleted detail-header object expression fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1772: deleted detail-footer object expression fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "EXPR",
                    "--property-value", "\"Updated deleted header label\"",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object expression update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object expression update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1772: deleted detail-header object expression update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1772: deleted detail-header object expression update should preserve deleted state");
            const auto updated_expr = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "EXPR"
            });
            expect(updated_expr.ok && updated_expr.exists && updated_expr.value == "\"Updated deleted header label\"",
                   "#1772: deleted detail-header object expression update should persist the EXPR memo field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1772: deleted detail-header object expression update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1772: deleted detail-header label object expression update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1772: deleted detail-header object expression update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1772: deleted detail-header object expression update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1772: deleted detail-header object expression update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1772: deleted detail-header object expression update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2308: deleted detail-header object expression update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2308: deleted detail-header object expression update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2308: deleted detail-header object expression update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2308: deleted detail-header object expression update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2308: deleted detail-header object expression update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2308: deleted detail-header object expression update should preserve deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2308: deleted detail-header object expression update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2308: deleted detail-header object expression update should preserve deleted preview heights");
            expect_contains(update_process.stdout_text, "\"dryRun\": false",
                            "#2261: deleted detail-header object expression update JSON should expose committed execution");
            expect_contains(update_process.stdout_text, "\"mutatesAsset\": true",
                            "#2261: deleted detail-header object expression update JSON should expose mutation state");
            expect_contains(update_process.stdout_text, "\"undoAvailable\": true",
                            "#2261: deleted detail-header object expression update JSON should expose undo availability");
            expect_contains(update_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2261: deleted detail-header object expression update JSON should expose expression undo labels");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Updated deleted header label\\\"\"",
                    "\"expressionFieldIndex\": 2"
                },
                "#1772: deleted detail-header object expression update should refresh deleted-object expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 50",
                    "\"sectionRelativeBottom\": 170",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"expression\": \"\\\"Updated deleted header label\\\"\"",
                    "\"expressionFieldIndex\": 2"
                },
                "#1772: deleted detail-header object expression update should refresh selected-object expression metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1772: deleted detail-header object expression update should preserve containing-section metadata");
            expect_not_contains(update_process.stdout_text, "\"expression\": \"\\\"Header label\\\"\"",
                                "#1772: deleted detail-header object expression update should not leak stale expressions");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "EXPR",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object expression clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object expression clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1772: deleted detail-footer object expression clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1772: deleted detail-footer object expression clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"EXPR\", \"type\": \"M\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 2",
                            "#1772: deleted detail-footer object expression clear should blank the EXPR memo field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1772: deleted detail-footer object expression clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1772: deleted detail-footer label object expression clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1772: deleted detail-footer object expression clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1772: deleted detail-footer object expression clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1772: deleted detail-footer object expression clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1772: deleted detail-footer object expression clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2308: deleted detail-footer object expression clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2308: deleted detail-footer object expression clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2308: deleted detail-footer object expression clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2308: deleted detail-footer object expression clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2308: deleted detail-footer object expression clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                            "#2308: deleted detail-footer object expression clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2308: deleted detail-footer object expression clear should preserve deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 410",
                            "#2308: deleted detail-footer object expression clear should preserve deleted preview heights");
            expect_contains(clear_process.stdout_text, "\"dryRun\": false",
                            "#2261: deleted detail-footer object expression clear JSON should expose committed execution");
            expect_contains(clear_process.stdout_text, "\"mutatesAsset\": true",
                            "#2261: deleted detail-footer object expression clear JSON should expose mutation state");
            expect_contains(clear_process.stdout_text, "\"undoAvailable\": true",
                            "#2261: deleted detail-footer object expression clear JSON should expose undo availability");
            expect_contains(clear_process.stdout_text, "\"undoLabel\": \"Property EXPR\"",
                            "#2261: deleted detail-footer object expression clear JSON should expose expression undo labels");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": 2"
                },
                "#1772: deleted detail-footer object expression clear should refresh deleted-object expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 160",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"expression\": \"\"",
                    "\"expressionFieldIndex\": 2"
                },
                "#1772: deleted detail-footer object expression clear should refresh selected-object expression metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"deletedObjectCount\": 1"
                },
                "#1772: deleted detail-footer object expression clear should preserve containing-section metadata");
            expect_not_contains(clear_process.stdout_text, "\"expression\": \"footer.total\"",
                                "#1772: deleted detail-footer object expression clear should not leak stale expressions");
        };

    run_deleted_detail_header_footer_object_expression_edits(
        temp_root / "deleted_detail_header_footer_object_expression_edits.frx",
        "deleted_detail_header_footer_object_expression_edits.frx",
        "report");
    run_deleted_detail_header_footer_object_expression_edits(
        temp_root / "deleted_detail_header_footer_object_expression_edits.lbx",
        "deleted_detail_header_footer_object_expression_edits.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
