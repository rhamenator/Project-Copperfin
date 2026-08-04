// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_unresolved_geometry_memo_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'C', .length = 24U},
        {.name = "VPOS", .type = 'C', .length = 24U},
        {.name = "WIDTH", .type = 'C', .length = 24U},
        {.name = "HEIGHT", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "detail.placeholder.geometry", "", "<memo block 70>", "", "<memo block 71>"},
        {"8", "0", "customer.unresolved.geometry", "<memo block 72>", "<memo block 73>",
         "<memo block 74>", "<memo block 75>"},
        {"5", "", "\"Deleted unresolved geometry\"", "<memo block 76>", "<memo block 77>",
         "<memo block 78>", "<memo block 79>"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1740: synthetic report table with unresolved geometry memo placeholders should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1740: synthetic report table should mark unresolved geometry object deleted");
}

void write_synthetic_report_table_for_missing_section_geometry_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 48U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "1", "page.header.missing.geometry", "missing-geometry-live-section-guid"},
        {"9", "8", "summary.missing.geometry", "missing-geometry-deleted-section-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1727: synthetic report table without section geometry schema should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1727: synthetic report table should mark the no-geometry section deleted");
}

#if !defined(COPPERFIN_DETAIL_HEADER_FOOTER_OBJECT_GEOMETRY_SKIP_STABLE)
void test_studio_host_json_updates_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_object_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_object_geometry =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "VPOS",
                    "--property-value", "90",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header object geometry update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header object geometry update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1780: detail-header object geometry update should exit successfully");
            const auto updated_vpos = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "VPOS"
            });
            expect(updated_vpos.ok && updated_vpos.exists && updated_vpos.value == "90",
                   "#1780: detail-header object geometry update should persist the VPOS field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1780: detail-header object geometry update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1780: detail-header label object geometry update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1780: detail-header object geometry update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1780: detail-header object geometry update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1780: detail-header object geometry update should preserve containing-section availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2286: detail-header object geometry update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2286: detail-header object geometry update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2286: detail-header object geometry update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2286: detail-header object geometry update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2286: detail-header object geometry update should not fabricate deleted preview availability");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1780: detail-header object geometry update should refresh selected-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1780: detail-header object geometry update should preserve containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer object geometry clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer object geometry clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1780: detail-footer object geometry clear should exit successfully");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"HEIGHT\", \"type\": \"N\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 6",
                            "#1780: detail-footer object geometry clear should blank the HEIGHT field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1780: detail-footer object geometry clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1780: detail-footer label object geometry clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1780: detail-footer object geometry clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1780: detail-footer object geometry clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1780: detail-footer object geometry clear should preserve containing-section availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2286: detail-footer object geometry clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2286: detail-footer object geometry clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2286: detail-footer object geometry clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2286: detail-footer object geometry clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2286: detail-footer object geometry clear should not fabricate deleted preview availability");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1780: detail-footer object geometry clear should refresh selected-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 1"
                },
                "#1780: detail-footer object geometry clear should preserve containing-section metadata");
        };

    run_detail_header_footer_object_geometry(temp_root / "detail_header_footer_object_geometry.frx",
                                             "detail_header_footer_object_geometry.frx",
                                             "report");
    run_detail_header_footer_object_geometry(temp_root / "detail_header_footer_object_geometry.lbx",
                                             "detail_header_footer_object_geometry.lbx",
                                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_detail_header_footer_object_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_object_geometry =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
            const auto delete_header_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
            expect(delete_header_object.ok && dbf_record_deleted(asset_path, 1U),
                   "#1781: deleted detail-header object geometry fixture should mark the header object deleted");
            const auto delete_footer_object = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
            expect(delete_footer_object.ok && dbf_record_deleted(asset_path, 3U),
                   "#1781: deleted detail-footer object geometry fixture should mark the footer object deleted");

            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-label-guid",
                    "--property-name", "VPOS",
                    "--property-value", "90",
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header object geometry update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header object geometry update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1781: deleted detail-header object geometry update should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1781: deleted detail-header object geometry update should preserve deleted state");
            const auto updated_vpos = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-header-label-guid",
                .property_name = "VPOS"
            });
            expect(updated_vpos.ok && updated_vpos.exists && updated_vpos.value == "90",
                   "#1781: deleted detail-header object geometry update should persist the VPOS field");
            expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1781: deleted detail-header object geometry update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_process.stdout_text, "\"isLabel\": true",
                                "#1781: deleted detail-header label object geometry update should retain label identity");
            }
            expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1781: deleted detail-header object geometry update should preserve deleted object counts");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1781: deleted detail-header object geometry update should preserve selected object availability");
            expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1781: deleted detail-header object geometry update should preserve object selection kind");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1781: deleted detail-header object geometry update should preserve containing sections");
            expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1781: deleted detail-header object geometry update should serialize containing-section JSON");
            expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2287: deleted detail-header object geometry update should preserve live preview availability");
            expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2287: deleted detail-header object geometry update should preserve live preview top bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2287: deleted detail-header object geometry update should preserve live preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2287: deleted detail-header object geometry update should preserve live preview heights");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2287: deleted detail-header object geometry update should expose deleted preview availability");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 90",
                            "#2287: deleted detail-header object geometry update should refresh deleted preview top bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 460",
                            "#2287: deleted detail-header object geometry update should preserve deleted preview bottom bounds");
            expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 370",
                            "#2287: deleted detail-header object geometry update should refresh deleted preview heights");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1781: deleted detail-header object geometry update should refresh deleted-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionRelativeTop\": 90",
                    "\"sectionRelativeBottom\": 210",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"label\"",
                    "\"left\": 100",
                    "\"top\": 90",
                    "\"width\": 700",
                    "\"right\": 800",
                    "\"height\": 120",
                    "\"bottom\": 210"
                },
                "#1781: deleted detail-header object geometry update should refresh selected-object geometry metadata");
            expect_contains_in_order(
                update_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-header-guid\"",
                    "\"recordIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1781: deleted detail-header object geometry update should preserve deleted containing-section metadata");

            const auto clear_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-field-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer object geometry clear stdout:\n"
                          << clear_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer object geometry clear stderr:\n"
                          << clear_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_process.exit_code == 0,
                   "#1781: deleted detail-footer object geometry clear should exit successfully");
            expect(dbf_record_deleted(asset_path, 3U),
                   "#1781: deleted detail-footer object geometry clear should preserve deleted state");
            expect_contains(clear_process.stdout_text,
                            "{\"name\": \"HEIGHT\", \"type\": \"N\", \"isNull\": false, \"value\": \"\", \"fieldIndex\": 6",
                            "#1781: deleted detail-footer object geometry clear should blank the HEIGHT field");
            expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1781: deleted detail-footer object geometry clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                                "#1781: deleted detail-footer label object geometry clear should retain label identity");
            }
            expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 2",
                            "#1781: deleted detail-footer object geometry clear should preserve deleted object counts");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                            "#1781: deleted detail-footer object geometry clear should preserve selected object availability");
            expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                            "#1781: deleted detail-footer object geometry clear should preserve object selection kind");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                            "#1781: deleted detail-footer object geometry clear should preserve containing sections");
            expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": {",
                            "#1781: deleted detail-footer object geometry clear should serialize containing-section JSON");
            expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview availability");
            expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview top bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2287: deleted detail-footer object geometry clear should preserve live preview heights");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2287: deleted detail-footer object geometry clear should expose deleted preview availability");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 90",
                            "#2287: deleted detail-footer object geometry clear should preserve deleted preview top bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 360",
                            "#2287: deleted detail-footer object geometry clear should refresh deleted preview bottom bounds");
            expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 270",
                            "#2287: deleted detail-footer object geometry clear should refresh deleted preview heights");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"deletedObjects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1781: deleted detail-footer object geometry clear should refresh deleted-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObject\": {",
                    "\"recordIndex\": 3",
                    "\"deleted\": true",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionRelativeTop\": 60",
                    "\"sectionRelativeBottom\": 60",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1",
                    "\"objectKind\": \"field\"",
                    "\"left\": 140",
                    "\"top\": 360",
                    "\"width\": 900",
                    "\"right\": 1040",
                    "\"height\": 0",
                    "\"bottom\": 360"
                },
                "#1781: deleted detail-footer object geometry clear should refresh selected-object geometry metadata");
            expect_contains_in_order(
                clear_process.stdout_text,
                {
                    "\"selectedReportObjectSection\": {",
                    "\"id\": \"detail-footer-guid\"",
                    "\"recordIndex\": 2",
                    "\"sectionCount\": 2",
                    "\"objectCount\": 0",
                    "\"deletedObjectCount\": 1"
                },
                "#1781: deleted detail-footer object geometry clear should preserve deleted containing-section metadata");
        };

    run_deleted_detail_header_footer_object_geometry(temp_root / "deleted_detail_header_footer_object_geometry.frx",
                                                     "deleted_detail_header_footer_object_geometry.frx",
                                                     "report");
    run_deleted_detail_header_footer_object_geometry(temp_root / "deleted_detail_header_footer_object_geometry.lbx",
                                                     "deleted_detail_header_footer_object_geometry.lbx",
                                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_UNRESOLVED_MEMO_SKIP_HOST_SMOKE)
void test_studio_host_json_defaults_unresolved_report_geometry_memo_placeholders(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_unresolved_report_geometry_memo_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_unresolved_geometry_memo_layout = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_unresolved_geometry_memo_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " unresolved geometry memo summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " unresolved geometry memo summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1740: unresolved geometry memo placeholders should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1740: unresolved geometry memo layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1740: unresolved geometry memo label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1740: unresolved geometry memo layouts should expose defaulted preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1740: unresolved geometry memo live preview width should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1740: unresolved geometry memo live preview height should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1740: unresolved geometry memo layouts should expose deleted defaulted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1740: unresolved geometry memo deleted preview width should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1740: unresolved geometry memo deleted preview height should stay non-inverted");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2336: unresolved geometry memo summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve section rows");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1740: unresolved geometry memo layouts should preserve deleted object counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"detail_0\"",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved section geometry memo placeholders should default to zero with field provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"objects\": [",
                "\"recordIndex\": 1",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved live object geometry memo placeholders should default to zero with field provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 2",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved deleted object geometry memo placeholders should default to zero with field provenance");
        expect_not_contains(summary_process.stdout_text, "<memo block",
                            "#1740: unresolved geometry memo placeholders should not leak into summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1740: unresolved live object geometry memo selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1740: unresolved live object geometry memo selection should advertise selected objects");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2336: selected unresolved live geometry memo JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 1",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved live object geometry memo selection should expose zero non-inverted geometry");
        expect_not_contains(live_process.stdout_text, "<memo block",
                            "#1740: unresolved live object geometry memo placeholders should not leak into selection JSON");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1740: unresolved deleted object geometry memo selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1740: unresolved deleted object geometry memo selection should advertise selected objects");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"left\": 0",
                "\"leftFieldIndex\": 3",
                "\"top\": 0",
                "\"topFieldIndex\": 4",
                "\"width\": 0",
                "\"widthFieldIndex\": 5",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": 6",
                "\"bottom\": 0"
            },
            "#1740: unresolved deleted object geometry memo selection should expose zero non-inverted geometry");
        expect_not_contains(deleted_process.stdout_text, "<memo block",
                            "#1740: unresolved deleted object geometry memo placeholders should not leak into selection JSON");
    };

    run_unresolved_geometry_memo_layout(temp_root / "unresolved_geometry_memo.frx",
                                        "unresolved_geometry_memo.frx",
                                        "report");
    run_unresolved_geometry_memo_layout(temp_root / "unresolved_geometry_memo.lbx",
                                        "unresolved_geometry_memo.lbx",
                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

#if !defined(COPPERFIN_REPORT_SCHEMA_FALLBACK_SKIP_HOST_SMOKE)
void test_studio_host_json_defaults_report_sections_without_geometry_schema(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_section_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_section_geometry_layout = [&](const fs::path& asset_path,
                                                         const std::string& title,
                                                         const std::string& label) {
        write_synthetic_report_table_for_missing_section_geometry_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing section geometry summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing section geometry summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1727: missing section geometry schema should keep report/label inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1727: missing section geometry layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1727: missing section geometry label layouts should retain label identity");
        }
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2341: missing section geometry summary JSON");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1727: missing section geometry layouts should preserve live section counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1727: missing section geometry layouts should preserve deleted section counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1727: missing live section geometry should default height totals to zero");
        expect_contains(summary_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                        "#1727: missing deleted section geometry should default height totals to zero");
        expect_contains(summary_process.stdout_text, "\"sectionKindCounts\": [\n        {\"kind\": \"page_header\", \"count\": 1}\n      ]",
                        "#1727: missing live section geometry should preserve band-kind counts");
        expect_contains(summary_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"summary\", \"count\": 1}\n      ]",
                        "#1727: missing deleted section geometry should preserve band-kind counts");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"missing-geometry-live-section-guid\"",
                "\"title\": \"Page Header\"",
                "\"bandKind\": \"page_header\"",
                "\"expression\": \"page.header.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 1",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing live section geometry should serialize zero geometry with null provenance");
        expect_contains_in_order(
            summary_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"missing-geometry-deleted-section-guid\"",
                "\"title\": \"Summary\"",
                "\"bandKind\": \"summary\"",
                "\"expression\": \"summary.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 8",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing deleted section geometry should serialize zero geometry with null provenance");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "0", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1727: missing section geometry live selection should keep inspection non-failing");
        expect_contains(live_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1727: missing section geometry live selection should advertise selected sections");
        expect_contains(live_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1727: missing section geometry live selection should expose section selection kind");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2341: selected missing live section geometry JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"missing-geometry-live-section-guid\"",
                "\"title\": \"Page Header\"",
                "\"bandKind\": \"page_header\"",
                "\"expression\": \"page.header.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 0",
                "\"deleted\": false",
                "\"objectCode\": 1",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing live section geometry selection should expose zero geometry with null provenance");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "1", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1727: missing section geometry deleted selection should keep inspection non-failing");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1727: missing section geometry deleted selection should advertise selected sections");
        expect_contains(deleted_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1727: missing section geometry deleted selection should expose section selection kind");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2341: selected missing deleted section geometry JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"missing-geometry-deleted-section-guid\"",
                "\"title\": \"Summary\"",
                "\"bandKind\": \"summary\"",
                "\"expression\": \"summary.missing.geometry\"",
                "\"expressionFieldIndex\": 2",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCode\": 8",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1727: missing deleted section geometry selection should expose zero geometry with null provenance");
    };

    run_missing_section_geometry_layout(temp_root / "missing_section_geometry.frx",
                                        "missing_section_geometry.frx",
                                        "report");
    run_missing_section_geometry_layout(temp_root / "missing_section_geometry.lbx",
                                        "missing_section_geometry.lbx",
                                        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif

}  // namespace cf_test_studio_host_json
