#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_exposes_deleted_report_column_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_column_footer_section_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_column_footer_section_json = [&](const fs::path& asset_path,
                                                            const std::string& title,
                                                            const std::string& label) {
        write_synthetic_report_table_for_stable_column_section_json(asset_path);
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(asset_path.string(), 3U, true);
        expect(delete_result.ok && dbf_record_deleted(asset_path, 3U),
               "#1681: stable deleted column-footer fixture should mark the column-footer section deleted");

        const auto process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "column-footer-section-guid", "--json"},
            temp_root);

        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted column-footer section stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted column-footer section stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1681: stable selected deleted report/label column-footer section JSON should exit successfully");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1681: stable selected deleted column-footer section JSON should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1681: stable selected deleted column-footer label section JSON should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1681: stable selected deleted column-footer sections should advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1681: stable selected deleted column-footer sections should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1681: stable selected deleted column-footer sections should preserve section selection classification");
        expect_contains(process.stdout_text, "\"sectionCount\": 2",
                        "#1681: stable selected deleted column-footer section JSON should preserve live sibling section counts");
        expect_contains(process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1681: stable selected deleted column-footer section JSON should expose deleted section counts");
        expect_contains(process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview availability");
        expect_contains(process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview left bounds");
        expect_contains(process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview top bounds");
        expect_contains(process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview right bounds");
        expect_contains(process.stdout_text, "\"previewBoundsBottom\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should refresh live preview bottom bounds");
        expect_contains(process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve live preview widths");
        expect_contains(process.stdout_text, "\"previewBoundsHeight\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should refresh live preview heights");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1940: stable selected deleted column-footer section JSON should expose deleted preview availability");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview left bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsTop\": 3050",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview top bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview right bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsBottom\": 3450",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview bottom bounds");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview widths");
        expect_contains(process.stdout_text, "\"deletedPreviewBoundsHeight\": 400",
                        "#1940: stable selected deleted column-footer section JSON should preserve deleted preview heights");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected object-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected object sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1681: stable selected deleted column-footer sections should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1681: stable selected deleted column-footer sections should serialize null selected settings");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"id\": \"column_footer_3\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": true"
            },
            "#1681: stable selected deleted column-footer section JSON should expose deleted section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"id\": \"column_footer_3\"",
                "\"bandKind\": \"column_footer\"",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 3050",
                "\"height\": 400",
                "\"bottom\": 3450"
            },
            "#1681: stable selected deleted column-footer sections should expose selected section metadata");
        expect_contains_in_order(
            process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"column_header\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2"
            },
            "#1681: stable selected deleted column-footer section JSON should preserve live sibling metadata");
    };

    run_deleted_column_footer_section_json(temp_root / "stable_deleted_column_footer_sections.frx",
                                           "stable_deleted_column_footer_sections.frx",
                                           "report");
    run_deleted_column_footer_section_json(temp_root / "stable_deleted_column_footer_sections.lbx",
                                           "stable_deleted_column_footer_sections.lbx",
                                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_report_sections(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_report_section_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path report_path = temp_root / "summary.frx";
    write_synthetic_report_table_for_layout_json(report_path);

    const auto section_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "1", "--json"},
        temp_root);

    if (section_process.exit_code != 0) {
        std::cerr << "studio host selected report section stdout:\n" << section_process.stdout_text << "\n";
        std::cerr << "studio host selected report section stderr:\n" << section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(section_process.exit_code == 0,
           "#1453: selected report section JSON smoke should exit successfully");
    expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1453: report section selections should advertise selected-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1457: report section selections should advertise report-selection availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1457: report section selections should expose section selection kind");
    expect_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                    "#1453: report section selections should expose selected-section JSON");
    expect_contains(section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1960: selected report section JSON should expose live preview availability");
    expect_contains(section_process.stdout_text, "\"previewBoundsLeft\": 0",
                    "#1960: selected report section JSON should preserve live preview left bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsTop\": 0",
                    "#1960: selected report section JSON should preserve live preview top bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsRight\": 5200",
                    "#1960: selected report section JSON should preserve live preview right bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsBottom\": 8100",
                    "#1960: selected report section JSON should preserve live preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"previewBoundsWidth\": 5200",
                    "#1960: selected report section JSON should preserve live preview widths");
    expect_contains(section_process.stdout_text, "\"previewBoundsHeight\": 8100",
                    "#1960: selected report section JSON should preserve live preview heights");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1960: selected report section JSON should expose deleted preview availability");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                    "#1960: selected report section JSON should preserve deleted preview left bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                    "#1960: selected report section JSON should preserve deleted preview top bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                    "#1960: selected report section JSON should preserve deleted preview right bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                    "#1960: selected report section JSON should preserve deleted preview bottom bounds");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                    "#1960: selected report section JSON should preserve deleted preview widths");
    expect_contains(section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                    "#1960: selected report section JSON should preserve deleted preview heights");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1512: selected report sections should not advertise selected-object availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1512: selected report sections should serialize null selected objects");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1512: selected report sections should not advertise containing-object-section availability");
    expect_contains(section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1512: selected report sections should serialize null containing-object sections");
    expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1512: selected report sections should not advertise selected-settings availability");
    expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1512: selected report sections should serialize null selected settings");
    expect_contains(section_process.stdout_text, "\"id\": \"page_header_1\"",
                    "#1453: selected report section JSON should expose the selected section id");
    expect_contains(section_process.stdout_text, "\"bandKind\": \"page_header\"",
                    "#1453: selected report section JSON should expose the selected band kind");
    expect_contains(section_process.stdout_text, "\"recordIndex\": 1",
                    "#1453: selected report section JSON should expose the selected section record index");
    expect_contains(section_process.stdout_text, "\"sectionIndex\": 0",
                    "#1460: selected report section JSON should expose section order");
    expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                    "#1460: selected report section JSON should expose live section counts");
    expect_contains(section_process.stdout_text, "\"objectCode\": 1",
                    "#1453: selected report section JSON should preserve raw section object codes");
    expect_contains(section_process.stdout_text, "\"bottom\": 2000",
                    "#1461: selected report section JSON should expose section bottom-edge coordinates");
    expect_contains(section_process.stdout_text, "\"objectCount\": 1",
                    "#1453: selected report section JSON should preserve selected section object counts");
    expect_contains(section_process.stdout_text, "\"objectKind\": \"label\"",
                    "#1453: selected report section JSON should include selected section objects");

    const fs::path deleted_section_path = temp_root / "deleted_section.frx";
    write_synthetic_report_table_for_deleted_section_json(deleted_section_path);
    const auto deleted_section_process = run_process_capture(
        studio_host_path,
        {"--path", deleted_section_path.string(), "--record", "1", "--json"},
        temp_root);

    if (deleted_section_process.exit_code != 0) {
        std::cerr << "studio host selected deleted report section stdout:\n"
                  << deleted_section_process.stdout_text << "\n";
        std::cerr << "studio host selected deleted report section stderr:\n"
                  << deleted_section_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(deleted_section_process.exit_code == 0,
           "#1478: selected deleted report section JSON should exit successfully");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                    "#1478: deleted report section selections should advertise selected-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                    "#1478: deleted report section selections should advertise report-selection availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                    "#1478: deleted report section selections should expose section selection kind");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                    "#1513: selected deleted report sections should not advertise selected-object availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObject\": null",
                    "#1513: selected deleted report sections should serialize null selected objects");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                    "#1513: selected deleted report sections should not advertise containing-object-section availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportObjectSection\": null",
                    "#1513: selected deleted report sections should serialize null containing-object sections");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                    "#1513: selected deleted report sections should not advertise selected-settings availability");
    expect_contains(deleted_section_process.stdout_text, "\"selectedReportSettings\": null",
                    "#1513: selected deleted report sections should serialize null selected settings");
    expect_contains(deleted_section_process.stdout_text, "\"sectionCount\": 0",
                    "#1478: deleted selected report section JSON should not expose live sections");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionCount\": 1",
                    "#1478: deleted selected report section JSON should expose deleted section counts");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsAvailable\": true",
                    "#1961: deleted selected report section JSON should preserve live preview availability");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsLeft\": 100",
                    "#1961: deleted selected report section JSON should preserve remaining live preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsTop\": 2600",
                    "#1961: deleted selected report section JSON should preserve remaining live preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsRight\": 150",
                    "#1961: deleted selected report section JSON should preserve remaining live preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsBottom\": 2800",
                    "#1961: deleted selected report section JSON should preserve remaining live preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsWidth\": 50",
                    "#1961: deleted selected report section JSON should preserve remaining live preview widths");
    expect_contains(deleted_section_process.stdout_text, "\"previewBoundsHeight\": 200",
                    "#1961: deleted selected report section JSON should preserve remaining live preview heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                    "#1524: deleted selected report section JSON should expose deleted preview bounds availability");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview left bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                    "#1524: deleted selected report section JSON should expose deleted section preview top bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview right bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                    "#1524: deleted selected report section JSON should expose deleted section preview bottom bounds");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                    "#1524: deleted selected report section JSON should expose deleted section preview width");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                    "#1524: deleted selected report section JSON should expose deleted section preview height");
    expect_contains(deleted_section_process.stdout_text, "\"placedObjectCount\": 0",
                    "#1522: deleted selected report section JSON should not fabricate placed object counts");
    expect_contains(deleted_section_process.stdout_text, "\"deletedPlacedObjectCount\": 0",
                    "#1523: deleted selected report section JSON should not fabricate deleted placed object counts");
    expect_contains(deleted_section_process.stdout_text, "\"deletedUnplacedObjectCount\": 0",
                    "#1523: deleted selected report section JSON should not fabricate deleted unplaced object counts");
    expect_contains(deleted_section_process.stdout_text, "\"sectionKindCount\": 0",
                    "#1520: deleted selected report section JSON should not fabricate live section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCount\": 1",
                    "#1520: deleted selected report section JSON should summarize deleted section band-kind buckets");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionKindCounts\": [\n        {\"kind\": \"detail\", \"count\": 1}\n      ]",
                    "#1520: deleted selected report section JSON should count deleted detail sections");
    expect_contains(deleted_section_process.stdout_text, "\"sectionHeightTotal\": 0",
                    "#1521: deleted selected report section JSON should not fabricate live section heights");
    expect_contains(deleted_section_process.stdout_text, "\"deletedSectionHeightTotal\": 5000",
                    "#1521: deleted selected report section JSON should summarize deleted section heights");
    expect_contains_in_order(
        deleted_section_process.stdout_text,
        {
            "\"selectedReportSection\": {",
            "\"bandKind\": \"detail\"",
            "\"recordIndex\": 1",
            "\"deleted\": true",
            "\"sectionIndex\": null",
            "\"sectionCount\": 0"
        },
        "#1478: deleted report section selections should expose deleted selected-section metadata");

    const auto object_process = run_process_capture(
        studio_host_path,
        {"--path", report_path.string(), "--record", "3", "--json"},
        temp_root);

    if (object_process.exit_code != 0) {
        std::cerr << "studio host selected report object stdout:\n" << object_process.stdout_text << "\n";
        std::cerr << "studio host selected report object stderr:\n" << object_process.stderr_text << "\n";
        std::cerr << "fixture root: " << temp_root << "\n";
    }

    expect(object_process.exit_code == 0,
           "#1453: selected report object JSON smoke should exit successfully");
    expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                    "#1453: non-section report selections should not advertise selected-section availability");
    expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                    "#1453: non-section report selections should serialize null selected sections");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_placement_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_placement_update = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "5",
                "--property-name", "VPOS",
                "--property-value", "2600",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout placement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout placement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1531: report/label layout object placement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2600",
               "#1531: report/label layout object placement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1531: report/label layout object placement update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1531: report/label layout object placement update should increment placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1531: report/label layout object placement update should clear unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1531: report/label layout object placement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1531: report/label layout object placement update should expose selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 700",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"line\""
            },
            "#1531: report/label layout object placement update should refresh selected object section metadata");
    };

    run_placement_update(temp_root / "placement_update.frx", "placement_update.frx", "report");
    run_placement_update(temp_root / "placement_update.lbx", "placement_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_placement_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_placement_update = [&](const fs::path& asset_path,
                                          const std::string& title,
                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto seed_identity = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = {},
            .property_name = "UNIQUEID",
            .property_value = "unplaced-line-guid"
        });
        expect(seed_identity.ok,
               "#1634: report/label layout object stable placement fixture should seed a stable id");

        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "unplaced-line-guid",
                "--property-name", "VPOS",
                "--property-value", "2600",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout placement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout placement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1634: report/label layout object stable placement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 5U,
            .object_name = {},
            .unique_id = "unplaced-line-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2600",
               "#1634: report/label layout object stable placement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1634: report/label layout object stable placement update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1634: label layout object stable placement update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1634: report/label layout object stable placement update should increment placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1634: report/label layout object stable placement update should clear unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1634: report/label layout object stable placement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1634: report/label layout object stable placement update should expose selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 5",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 700",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"line\"",
                "\"title\": \"unplaced-line-guid\""
            },
            "#1634: report/label layout object stable placement update should refresh selected object section metadata");
    };

    run_placement_update(temp_root / "placement_update_stable.frx",
                         "placement_update_stable.frx",
                         "report");
    run_placement_update(temp_root / "placement_update_stable.lbx",
                         "placement_update_stable.lbx",
                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_unplacement_update_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unplacement_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "VPOS",
                "--property-value", "9000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout unplacement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout unplacement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1532: report/label layout object unplacement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "9000",
               "#1532: report/label layout object unplacement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1532: report/label layout object unplacement update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1532: report/label layout object unplacement update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1532: report/label layout object unplacement update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1532: report/label layout object unplacement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1532: report/label layout object unplacement update should clear selected containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1532: report/label layout object unplacement update should serialize null selected containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\""
            },
            "#1532: report/label layout object unplacement update should refresh selected object unplaced metadata");
    };

    run_unplacement_update(temp_root / "unplacement_update.frx", "unplacement_update.frx", "report");
    run_unplacement_update(temp_root / "unplacement_update.lbx", "unplacement_update.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_unplacement_update_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_unplacement_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "VPOS",
                "--property-value", "9000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout unplacement update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout unplacement update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1635: report/label layout object stable unplacement update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "9000",
               "#1635: report/label layout object stable unplacement update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1635: report/label layout object stable unplacement update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1635: label layout object stable unplacement update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1635: report/label layout object stable unplacement update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1635: report/label layout object stable unplacement update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1635: report/label layout object stable unplacement update should preserve selected object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1635: report/label layout object stable unplacement update should clear selected containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1635: report/label layout object stable unplacement update should serialize null selected containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"containingSectionRecordIndex\": null",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"sectionObjectIndex\": null",
                "\"sectionObjectCount\": 0",
                "\"objectKind\": \"field\"",
                "\"title\": \"customer.company\"",
                "\"top\": 9000",
                "\"bottom\": 9450"
            },
            "#1635: report/label layout object stable unplacement update should refresh selected object unplaced metadata");
    };

    run_unplacement_update(temp_root / "unplacement_update_stable.frx",
                           "unplacement_update_stable.frx",
                           "report");
    run_unplacement_update(temp_root / "unplacement_update_stable.lbx",
                           "unplacement_update_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_section_heights_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& updated_height,
                                               const std::string& updated_total,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "HEIGHT",
                "--property-value", updated_height,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " section height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " section height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1525: report/label section height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == updated_height,
               "#1525: report/label section height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1525: report/label section height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1910: label section height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1910: report/label section height update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1910: report/label section height update should preserve preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1910: report/label section height update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1910: report/label section height update should preserve preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1910: report/label section height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1910: report/label section height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1910: report/label section height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1910: report/label section height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1910: report/label section height update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1910: report/label section height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1910: report/label section height update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionHeightTotal\": " + updated_total,
                        "#1525: report/label section height update should refresh section height totals");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1525: report/label section height update should preserve placed object counts");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"recordIndex\": 1",
                "\"height\": " + updated_height,
                "\"bottom\": " + updated_height,
                "\"objectCount\": 1"
            },
            "#1525: report/label section height update should refresh selected section geometry");
    };

    run_section_height_update(temp_root / "section_height.frx", "section_height.frx", "2400", "7400", "report");
    run_section_height_update(temp_root / "section_height.lbx", "section_height.lbx", "2600", "7600", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_section_heights_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_height_update = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_deleted_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "HEIGHT",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted section height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted section height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1575: deleted report/label section height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "2400",
               "#1575: deleted report/label section height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1575: deleted report/label section height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1919: deleted label section height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1919: deleted report/label section height update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1919: deleted report/label section height update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1919: deleted report/label section height update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1919: deleted report/label section height update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1919: deleted report/label section height update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1919: deleted report/label section height update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1919: deleted report/label section height update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1919: deleted report/label section height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1919: deleted report/label section height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1919: deleted report/label section height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1919: deleted report/label section height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4400",
                        "#1919: deleted report/label section height update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1919: deleted report/label section height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2400",
                        "#1919: deleted report/label section height update should refresh deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1575: deleted report/label section height update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1575: deleted report/label section height update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1575: deleted report/label section height update should preserve selected section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1575: deleted report/label section height update should preserve selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 2400",
                "\"bottom\": 4400"
            },
            "#1575: deleted report/label section height update should refresh deleted-section geometry");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 2400",
                "\"bottom\": 4400"
            },
            "#1575: deleted report/label section height update should refresh selected-section geometry");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1575: deleted report/label section height update should preserve unplaced object accounting");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1575: deleted report/label section height update should not fabricate containing sections");
    };

    run_deleted_section_height_update(temp_root / "deleted_section_height.frx",
                                      "deleted_section_height.frx",
                                      "report");
    run_deleted_section_height_update(temp_root / "deleted_section_height.lbx",
                                      "deleted_section_height.lbx",
                                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_section_heights_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_height_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_height_clear = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_deleted_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted section height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted section height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1576: deleted report/label section height clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1576: deleted report/label section height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1920: deleted label section height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1920: deleted report/label section height clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1920: deleted report/label section height clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1920: deleted report/label section height clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1920: deleted report/label section height clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1920: deleted report/label section height clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1920: deleted report/label section height clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1920: deleted report/label section height clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1920: deleted report/label section height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1920: deleted report/label section height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1920: deleted report/label section height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1920: deleted report/label section height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2000",
                        "#1920: deleted report/label section height clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1920: deleted report/label section height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1920: deleted report/label section height clear should refresh deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 0",
                        "#1576: deleted report/label section height clear should not fabricate live sections");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1576: deleted report/label section height clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1576: deleted report/label section height clear should preserve selected section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1576: deleted report/label section height clear should preserve selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 0",
                "\"bottom\": 2000"
            },
            "#1576: deleted report/label section height clear should refresh deleted-section geometry");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 0",
                "\"bottom\": 2000"
            },
            "#1576: deleted report/label section height clear should refresh selected-section geometry");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1576: deleted report/label section height clear should preserve unplaced object accounting");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1576: deleted report/label section height clear should not fabricate containing sections");
    };

    run_deleted_section_height_clear(temp_root / "deleted_section_height_clear.frx",
                                     "deleted_section_height_clear.frx",
                                     "report");
    run_deleted_section_height_clear(temp_root / "deleted_section_height_clear.lbx",
                                     "deleted_section_height_clear.lbx",
                                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_section_heights_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_height_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " section height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " section height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1564: report/label section height clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1564: report/label section height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#2272: label section height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#2272: report/label section height clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#2272: report/label section height clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#2272: report/label section height clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#2272: report/label section height clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#2272: report/label section height clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#2272: report/label section height clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#2272: report/label section height clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#2272: report/label section height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#2272: report/label section height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#2272: report/label section height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#2272: report/label section height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#2272: report/label section height clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#2272: report/label section height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#2272: report/label section height clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionHeightTotal\": 5000",
                        "#1564: report/label section height clear should refresh section height totals");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1564: report/label section height clear should refresh placed object counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1564: report/label section height clear should refresh unplaced object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1564: report/label section height clear should preserve selected section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"sectionIndex\": 0",
                "\"top\": 0",
                "\"height\": 0",
                "\"bottom\": 0",
                "\"objectCount\": 0"
            },
            "#1564: report/label section height clear should refresh selected section geometry");
    };

    run_section_height_clear(temp_root / "section_height_clear.frx", "section_height_clear.frx", "report");
    run_section_height_clear(temp_root / "section_height_clear.lbx", "section_height_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_section_tops_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_top_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "2",
                "--property-name", "VPOS",
                "--property-value", "2500",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " section top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " section top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1526: report/label section top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 2U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2500",
               "#1526: report/label section top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1526: report/label section top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1915: label section top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1915: report/label section top update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1915: report/label section top update should preserve preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1915: report/label section top update should preserve preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1915: report/label section top update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1915: report/label section top update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1915: report/label section top update should preserve preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1915: report/label section top update should preserve preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1915: report/label section top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1915: report/label section top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1915: report/label section top update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1915: report/label section top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1915: report/label section top update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1915: report/label section top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1915: report/label section top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionHeightTotal\": 7000",
                        "#1526: report/label section top update should preserve section height totals");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1526: report/label section top update should preserve placed object counts");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"recordIndex\": 2",
                "\"sectionIndex\": 1",
                "\"top\": 2500",
                "\"height\": 5000",
                "\"bottom\": 7500",
                "\"objectCount\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 550"
            },
            "#1526: report/label section top update should refresh section and relative object geometry");
    };

    run_section_top_update(temp_root / "section_top.frx", "section_top.frx", "report");
    run_section_top_update(temp_root / "section_top.lbx", "section_top.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_section_tops_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_top_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_top_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_deleted_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "1",
                "--property-name", "VPOS",
                "--property-value", "2500",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted section top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted section top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1577: deleted report/label section top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2500",
               "#1577: deleted report/label section top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1577: deleted report/label section top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1917: deleted label section top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1917: deleted report/label section top update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1917: deleted report/label section top update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1917: deleted report/label section top update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1917: deleted report/label section top update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1917: deleted report/label section top update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1917: deleted report/label section top update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1917: deleted report/label section top update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1917: deleted report/label section top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1917: deleted report/label section top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2500",
                        "#1917: deleted report/label section top update should refresh deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1917: deleted report/label section top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7500",
                        "#1917: deleted report/label section top update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1917: deleted report/label section top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1917: deleted report/label section top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1577: deleted report/label section top update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1577: deleted report/label section top update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1577: deleted report/label section top update should preserve selected section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1577: deleted report/label section top update should preserve selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2500",
                "\"height\": 5000",
                "\"bottom\": 7500"
            },
            "#1577: deleted report/label section top update should refresh deleted-section geometry");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2500",
                "\"height\": 5000",
                "\"bottom\": 7500"
            },
            "#1577: deleted report/label section top update should refresh selected-section geometry");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1577: deleted report/label section top update should preserve unplaced object accounting");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1577: deleted report/label section top update should not fabricate containing sections");
    };

    run_deleted_section_top_update(temp_root / "deleted_section_top.frx",
                                   "deleted_section_top.frx",
                                   "report");
    run_deleted_section_top_update(temp_root / "deleted_section_top.lbx",
                                   "deleted_section_top.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_section_tops_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_top_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_top_clear = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_deleted_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "1",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted section top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted section top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1578: deleted report/label section top clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1578: deleted report/label section top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1918: deleted label section top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1918: deleted report/label section top clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1918: deleted report/label section top clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1918: deleted report/label section top clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1918: deleted report/label section top clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1918: deleted report/label section top clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1918: deleted report/label section top clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1918: deleted report/label section top clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1918: deleted report/label section top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1918: deleted report/label section top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1918: deleted report/label section top clear should refresh deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1918: deleted report/label section top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 5000",
                        "#1918: deleted report/label section top clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1918: deleted report/label section top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1918: deleted report/label section top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 0",
                        "#1578: deleted report/label section top clear should not fabricate live sections");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1578: deleted report/label section top clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1578: deleted report/label section top clear should preserve selected section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1578: deleted report/label section top clear should preserve selection kind");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedSections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 5000",
                "\"bottom\": 5000"
            },
            "#1578: deleted report/label section top clear should refresh deleted-section geometry");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 5000",
                "\"bottom\": 5000"
            },
            "#1578: deleted report/label section top clear should refresh selected-section geometry");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1578: deleted report/label section top clear should preserve unplaced object accounting");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1578: deleted report/label section top clear should not fabricate containing sections");
    };

    run_deleted_section_top_clear(temp_root / "deleted_section_top_clear.frx",
                                  "deleted_section_top_clear.frx",
                                  "report");
    run_deleted_section_top_clear(temp_root / "deleted_section_top_clear.lbx",
                                  "deleted_section_top_clear.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_section_tops_by_record_selection(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_top_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "2",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " section top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " section top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1565: report/label section top clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1565: report/label section top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1916: label section top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1916: report/label section top clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1916: report/label section top clear should preserve preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1916: report/label section top clear should preserve preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1916: report/label section top clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1916: report/label section top clear should preserve preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1916: report/label section top clear should preserve preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1916: report/label section top clear should preserve preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1916: report/label section top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1916: report/label section top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1916: report/label section top clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1916: report/label section top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1916: report/label section top clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1916: report/label section top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1916: report/label section top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionHeightTotal\": 7000",
                        "#1565: report/label section top clear should preserve section height totals");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1565: report/label section top clear should preserve placed object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1565: report/label section top clear should preserve selected section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 2",
                "\"sectionIndex\": 1",
                "\"top\": 0",
                "\"height\": 5000",
                "\"bottom\": 5000",
                "\"objectCount\": 1",
                "\"sectionRelativeTop\": 2600",
                "\"sectionRelativeBottom\": 3050"
            },
            "#1565: report/label section top clear should refresh selected section geometry and relative object metadata");
    };

    run_section_top_clear(temp_root / "section_top_clear.frx", "section_top_clear.frx", "report");
    run_section_top_clear(temp_root / "section_top_clear.lbx", "section_top_clear.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_stable_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& updated_height,
                                               const std::string& label) {
        write_synthetic_report_table_for_stable_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "section-guid",
                "--property-name", "HEIGHT",
                "--property-value", updated_height,
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable section height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable section height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        const std::string updated_bottom = std::to_string(2000 + std::stoi(updated_height));
        expect(update_process.exit_code == 0,
               "#1824: report/label stable section height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "section-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == updated_height,
               "#1824: report/label stable section height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1824: report/label stable section height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1824: label stable section height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1911: report/label stable section height update should preserve preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1911: report/label stable section height update should preserve preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1911: report/label stable section height update should preserve preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1911: report/label stable section height update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": " + updated_bottom,
                        "#1911: report/label stable section height update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#1911: report/label stable section height update should preserve preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": " + updated_height,
                        "#1911: report/label stable section height update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1911: report/label stable section height update should not fabricate deleted preview bounds");
        expect_contains(update_process.stdout_text, "\"sectionHeightTotal\": " + updated_height,
                        "#1824: report/label stable section height update should refresh section height totals");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1824: report/label stable section height update should preserve placed object counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1824: report/label stable section height update should preserve unplaced object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1824: report/label stable section height update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1824: report/label stable section height update should preserve section selection kind");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"top\": 2000",
                "\"height\": " + updated_height,
                "\"bottom\": " + updated_bottom,
                "\"objectCount\": 3"
            },
            "#1824: report/label stable section height update should refresh selected-section geometry");
    };

    const auto run_section_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "section-guid",
                "--property-name", "VPOS",
                "--property-value", "2500",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable section top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable section top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1824: report/label stable section top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "section-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2500",
               "#1824: report/label stable section top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1824: report/label stable section top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1824: label stable section top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1913: report/label stable section top update should preserve preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1913: report/label stable section top update should preserve preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2500",
                        "#1913: report/label stable section top update should refresh preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1913: report/label stable section top update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 7500",
                        "#1913: report/label stable section top update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#1913: report/label stable section top update should preserve preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#1913: report/label stable section top update should preserve preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1913: report/label stable section top update should not fabricate deleted preview bounds");
        expect_contains(update_process.stdout_text, "\"sectionHeightTotal\": 5000",
                        "#1824: report/label stable section top update should preserve section height totals");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1824: report/label stable section top update should preserve placed object counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1824: report/label stable section top update should preserve unplaced object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1824: report/label stable section top update should preserve selected-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"top\": 2500",
                "\"height\": 5000",
                "\"bottom\": 7500",
                "\"objectCount\": 3",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 300"
            },
            "#1824: report/label stable section top update should refresh selected-section and relative object geometry");
    };

    run_section_height_update(temp_root / "section_height_stable.frx",
                              "section_height_stable.frx",
                              "2400",
                              "report");
    run_section_height_update(temp_root / "section_height_stable.lbx",
                              "section_height_stable.lbx",
                              "2600",
                              "label");
    run_section_top_update(temp_root / "section_top_stable.frx", "section_top_stable.frx", "report");
    run_section_top_update(temp_root / "section_top_stable.lbx", "section_top_stable.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_section_stable_geometry_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_section_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "section-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable section height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable section height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1824: report/label stable section height clear should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "section-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.direct_field &&
                   height_property.value.empty(),
               "#1824: report/label stable section height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1824: report/label stable section height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1824: label stable section height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1912: report/label stable section height clear should preserve preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1912: report/label stable section height clear should preserve preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#1912: report/label stable section height clear should preserve preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1912: report/label stable section height clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1912: report/label stable section height clear should refresh preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#1912: report/label stable section height clear should preserve preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 800",
                        "#1912: report/label stable section height clear should refresh preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1912: report/label stable section height clear should not fabricate deleted preview bounds");
        expect_contains(clear_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1824: report/label stable section height clear should refresh section height totals");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 0",
                        "#1824: report/label stable section height clear should refresh placed object counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1824: report/label stable section height clear should refresh unplaced object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1824: report/label stable section height clear should preserve selected-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"top\": 2000",
                "\"height\": 0",
                "\"bottom\": 2000",
                "\"objectCount\": 0"
            },
            "#1824: report/label stable section height clear should refresh selected-section geometry");
    };

    const auto run_section_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "section-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable section top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable section top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1824: report/label stable section top clear should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "section-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.direct_field &&
                   top_property.value.empty(),
               "#1824: report/label stable section top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1824: report/label stable section top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1824: label stable section top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1914: report/label stable section top clear should preserve preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1914: report/label stable section top clear should preserve preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1914: report/label stable section top clear should refresh preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1914: report/label stable section top clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 5000",
                        "#1914: report/label stable section top clear should refresh preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 150",
                        "#1914: report/label stable section top clear should preserve preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 5000",
                        "#1914: report/label stable section top clear should preserve preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1914: report/label stable section top clear should not fabricate deleted preview bounds");
        expect_contains(clear_process.stdout_text, "\"sectionHeightTotal\": 5000",
                        "#1824: report/label stable section top clear should preserve section height totals");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 3",
                        "#1824: report/label stable section top clear should preserve placed object counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1824: report/label stable section top clear should preserve unplaced object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1824: report/label stable section top clear should preserve selected-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"recordIndex\": 1",
                "\"deleted\": false",
                "\"sectionIndex\": 0",
                "\"sectionCount\": 1",
                "\"top\": 0",
                "\"height\": 5000",
                "\"bottom\": 5000",
                "\"objectCount\": 3",
                "\"sectionRelativeTop\": 2600",
                "\"sectionRelativeBottom\": 2800"
            },
            "#1824: report/label stable section top clear should refresh selected-section and relative object geometry");
    };

    run_section_height_clear(temp_root / "section_height_clear_stable.frx",
                             "section_height_clear_stable.frx",
                             "report");
    run_section_height_clear(temp_root / "section_height_clear_stable.lbx",
                             "section_height_clear_stable.lbx",
                             "label");
    run_section_top_clear(temp_root / "section_top_clear_stable.frx",
                          "section_top_clear_stable.frx",
                          "report");
    run_section_top_clear(temp_root / "section_top_clear_stable.lbx",
                          "section_top_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_stable_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_height_update = [&](const fs::path& asset_path,
                                                       const std::string& title,
                                                       const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-section-guid",
                "--property-name", "HEIGHT",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted section height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted section height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1825: report/label stable deleted section height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "deleted-section-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "2400",
               "#1825: report/label stable deleted section height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1825: report/label stable deleted section height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1825: label stable deleted section height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1921: report/label stable deleted section height update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1921: report/label stable deleted section height update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1921: report/label stable deleted section height update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1921: report/label stable deleted section height update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1921: report/label stable deleted section height update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1921: report/label stable deleted section height update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1921: report/label stable deleted section height update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1921: report/label stable deleted section height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1921: report/label stable deleted section height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1921: report/label stable deleted section height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1921: report/label stable deleted section height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4400",
                        "#1921: report/label stable deleted section height update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1921: report/label stable deleted section height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2400",
                        "#1921: report/label stable deleted section height update should refresh deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section height update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section height update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1825: report/label stable deleted section height update should preserve unplaced object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section height update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1825: report/label stable deleted section height update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1825: report/label stable deleted section height update should not fabricate containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 2400",
                "\"bottom\": 4400"
            },
            "#1825: report/label stable deleted section height update should refresh selected deleted-section geometry");
    };

    const auto run_deleted_section_top_update = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_section_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-section-guid",
                "--property-name", "VPOS",
                "--property-value", "2500",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted section top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted section top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1825: report/label stable deleted section top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "deleted-section-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "2500",
               "#1825: report/label stable deleted section top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1825: report/label stable deleted section top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1825: label stable deleted section top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1923: report/label stable deleted section top update should preserve live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1923: report/label stable deleted section top update should preserve live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1923: report/label stable deleted section top update should preserve live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1923: report/label stable deleted section top update should preserve live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1923: report/label stable deleted section top update should preserve live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1923: report/label stable deleted section top update should preserve live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1923: report/label stable deleted section top update should preserve live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1923: report/label stable deleted section top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1923: report/label stable deleted section top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2500",
                        "#1923: report/label stable deleted section top update should refresh deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1923: report/label stable deleted section top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7500",
                        "#1923: report/label stable deleted section top update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1923: report/label stable deleted section top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1923: report/label stable deleted section top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section top update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section top update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1825: report/label stable deleted section top update should preserve unplaced object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section top update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1825: report/label stable deleted section top update should not fabricate containing sections");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2500",
                "\"height\": 5000",
                "\"bottom\": 7500"
            },
            "#1825: report/label stable deleted section top update should refresh selected deleted-section geometry");
    };

    run_deleted_section_height_update(temp_root / "deleted_section_height_stable.frx",
                                      "deleted_section_height_stable.frx",
                                      "report");
    run_deleted_section_height_update(temp_root / "deleted_section_height_stable.lbx",
                                      "deleted_section_height_stable.lbx",
                                      "label");
    run_deleted_section_top_update(temp_root / "deleted_section_top_stable.frx",
                                   "deleted_section_top_stable.frx",
                                   "report");
    run_deleted_section_top_update(temp_root / "deleted_section_top_stable.lbx",
                                   "deleted_section_top_stable.lbx",
                                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_stable_geometry_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_section_height_clear = [&](const fs::path& asset_path,
                                                      const std::string& title,
                                                      const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-section-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted section height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted section height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1825: report/label stable deleted section height clear should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "deleted-section-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.direct_field &&
                   height_property.value.empty(),
               "#1825: report/label stable deleted section height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1825: report/label stable deleted section height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1825: label stable deleted section height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1922: report/label stable deleted section height clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1922: report/label stable deleted section height clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1922: report/label stable deleted section height clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1922: report/label stable deleted section height clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1922: report/label stable deleted section height clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1922: report/label stable deleted section height clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1922: report/label stable deleted section height clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1922: report/label stable deleted section height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1922: report/label stable deleted section height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1922: report/label stable deleted section height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1922: report/label stable deleted section height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2000",
                        "#1922: report/label stable deleted section height clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1922: report/label stable deleted section height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1922: report/label stable deleted section height clear should refresh deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section height clear should not fabricate live sections");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section height clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1825: report/label stable deleted section height clear should preserve unplaced object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section height clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1825: report/label stable deleted section height clear should not fabricate containing sections");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 2000",
                "\"height\": 0",
                "\"bottom\": 2000"
            },
            "#1825: report/label stable deleted section height clear should refresh selected deleted-section geometry");
    };

    const auto run_deleted_section_top_clear = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_section_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-section-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted section top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted section top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1825: report/label stable deleted section top clear should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 1U,
            .object_name = {},
            .unique_id = "deleted-section-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.direct_field &&
                   top_property.value.empty(),
               "#1825: report/label stable deleted section top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1825: report/label stable deleted section top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1825: label stable deleted section top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1924: report/label stable deleted section top clear should preserve live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 100",
                        "#1924: report/label stable deleted section top clear should preserve live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 2600",
                        "#1924: report/label stable deleted section top clear should preserve live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 150",
                        "#1924: report/label stable deleted section top clear should preserve live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 2800",
                        "#1924: report/label stable deleted section top clear should preserve live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 50",
                        "#1924: report/label stable deleted section top clear should preserve live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 200",
                        "#1924: report/label stable deleted section top clear should preserve live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1924: report/label stable deleted section top clear should refresh deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 5000",
                        "#1924: report/label stable deleted section top clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section top clear should not fabricate live sections");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section top clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 3",
                        "#1825: report/label stable deleted section top clear should preserve unplaced object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section top clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"\"",
                        "#1825: report/label stable deleted section top clear should not fabricate containing sections");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"sectionIndex\": null",
                "\"sectionCount\": 0",
                "\"top\": 0",
                "\"height\": 5000",
                "\"bottom\": 5000"
            },
            "#1825: report/label stable deleted section top clear should refresh selected deleted-section geometry");
    };

    run_deleted_section_height_clear(temp_root / "deleted_section_height_clear_stable.frx",
                                     "deleted_section_height_clear_stable.frx",
                                     "report");
    run_deleted_section_height_clear(temp_root / "deleted_section_height_clear_stable.lbx",
                                     "deleted_section_height_clear_stable.lbx",
                                     "label");
    run_deleted_section_top_clear(temp_root / "deleted_section_top_clear_stable.frx",
                                  "deleted_section_top_clear_stable.frx",
                                  "report");
    run_deleted_section_top_clear(temp_root / "deleted_section_top_clear_stable.lbx",
                                  "deleted_section_top_clear_stable.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_section_and_settings_selection_for_ambiguous_stable_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_ambiguous_report_selection_category_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " ambiguous report selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " ambiguous report selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1701: ambiguous stable report/label selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1701: ambiguous stable selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1701: ambiguous stable label selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1701: ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1701: ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1701: ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1701: ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1701: ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1701: ambiguous stable selectors should serialize null selected settings");
    };

    const auto run_ambiguous_section_selector = [&](const fs::path& asset_path,
                                                    const std::string& title,
                                                    const std::string& label) {
        write_synthetic_report_table_for_ambiguous_summary_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 3",
                        "#1701: ambiguous section selectors should preserve live section counts");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1701: ambiguous section selectors should preserve deleted section counts");
        expect_contains(section_process.stdout_text, "\"settingCount\": 1",
                        "#1701: ambiguous section selectors should preserve live setting counts");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"sections\": [",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 1",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 2",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 3"
            },
            "#1701: ambiguous section selectors should preserve both duplicate section records in layout JSON");
    };

    const auto run_ambiguous_settings_selector = [&](const fs::path& asset_path,
                                                     const std::string& title,
                                                     const std::string& label) {
        write_synthetic_report_table_for_ambiguous_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1701: ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1701: ambiguous settings selectors should preserve both root setting records");
        expect_contains(settings_process.stdout_text, "\"pageSetupAvailable\": true",
                        "#1701: ambiguous settings selectors should preserve page setup summaries");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"settings\": [",
                "\"name\": \"ORIENTATION\"",
                "\"recordIndex\": 0",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 1"
            },
            "#1701: ambiguous settings selectors should preserve duplicate settings records in layout JSON");
    };

    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.frx",
                                   "ambiguous_section_selector.frx",
                                   "report section");
    run_ambiguous_section_selector(temp_root / "ambiguous_section_selector.lbx",
                                   "ambiguous_section_selector.lbx",
                                   "label section");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.frx",
                                    "ambiguous_settings_selector.frx",
                                    "report settings");
    run_ambiguous_settings_selector(temp_root / "ambiguous_settings_selector.lbx",
                                    "ambiguous_settings_selector.lbx",
                                    "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_selects_deep_report_sections_and_settings_by_stable_selector(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_report_section_settings_selector_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_common_selection = [&](const ProcessResult& process,
                                             const std::string& label,
                                             const std::string& title,
                                             const std::string& selection_kind) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1707: deep stable report/label section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1707: deep stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1707: deep stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1707: deep stable section/settings selectors should advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"" + selection_kind + "\"",
                        "#1707: deep stable section/settings selectors should expose the selected category");
    };

    const auto run_deep_section_selection = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_deep_stable_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-section-guid", "--json"},
            temp_root);

        expect_common_selection(section_process, label, title, "section");
        expect_contains(section_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1707: deep stable section selectors should advertise selected-section availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-object availability");
        expect_contains(section_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable section selectors should serialize null selected objects");
        expect_contains(section_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1707: deep stable section selectors should not advertise selected-settings availability");
        expect_contains(section_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1707: deep stable section selectors should serialize null selected settings");
        expect_contains(section_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable section selectors should parse objects beyond the default preview record limit");
        expect_contains_in_order(
            section_process.stdout_text,
            {
                "\"selectedReportSection\": {",
                "\"bandKind\": \"summary\"",
                "\"recordIndex\": 10",
                "\"deleted\": false",
                "\"sectionIndex\": 1",
                "\"sectionCount\": 2"
            },
            "#1707: deep stable section selectors should expose the selected deep report section");
    };

    const auto run_deep_settings_selection = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_deep_stable_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-settings-guid", "--json"},
            temp_root);

        expect_common_selection(settings_process, label, title, "settings");
        expect_contains(settings_process.stdout_text, "\"selectedReportSettingsAvailable\": true",
                        "#1707: deep stable settings selectors should advertise selected-settings availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-section availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportSection\": null",
                        "#1707: deep stable settings selectors should serialize null selected sections");
        expect_contains(settings_process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1707: deep stable settings selectors should not advertise selected-object availability");
        expect_contains(settings_process.stdout_text, "\"selectedReportObject\": null",
                        "#1707: deep stable settings selectors should serialize null selected objects");
        expect_contains(settings_process.stdout_text, "\"liveObjectCount\": 8",
                        "#1707: deep stable settings selectors should parse objects beyond the default preview record limit");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1707: deep stable settings selectors should preserve all root settings rows");
        expect_contains_in_order(
            settings_process.stdout_text,
            {
                "\"selectedReportSettings\": [",
                "\"name\": \"PAPERSIZE\"",
                "\"recordIndex\": 10",
                "\"value\": \"9\""
            },
            "#1707: deep stable settings selectors should expose the selected deep root settings");
    };

    run_deep_section_selection(temp_root / "deep_section_selector.frx",
                               "deep_section_selector.frx",
                               "report section");
    run_deep_section_selection(temp_root / "deep_section_selector.lbx",
                               "deep_section_selector.lbx",
                               "label section");
    run_deep_settings_selection(temp_root / "deep_settings_selector.frx",
                                "deep_settings_selector.frx",
                                "report settings");
    run_deep_settings_selection(temp_root / "deep_settings_selector.lbx",
                                "deep_settings_selector.lbx",
                                "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_selection_for_deep_ambiguous_section_and_settings_selectors(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deep_ambiguous_report_section_settings_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto expect_no_selection = [&](const ProcessResult& process,
                                         const std::string& label,
                                         const std::string& title) {
        if (process.exit_code != 0) {
            std::cerr << "studio host " << label << " deep ambiguous stable selector stdout:\n"
                      << process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deep ambiguous stable selector stderr:\n"
                      << process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(process.exit_code == 0,
               "#1708: deep ambiguous stable section/settings selectors should keep JSON inspection non-failing");
        expect_contains(process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1708: deep ambiguous stable section/settings selectors should preserve document titles");
        if (title.find(".lbx") != std::string::npos) {
            expect_contains(process.stdout_text, "\"isLabel\": true",
                            "#1708: deep ambiguous stable label section/settings selectors should retain label identity");
        }
        expect_contains(process.stdout_text, "\"selectedReportSelectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise report-selection availability");
        expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"none\"",
                        "#1708: deep ambiguous stable selectors should expose explicit no-selection kind");
        expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-object availability");
        expect_contains(process.stdout_text, "\"selectedReportObject\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected objects");
        expect_contains(process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise containing-section availability");
        expect_contains(process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null containing sections");
        expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-section availability");
        expect_contains(process.stdout_text, "\"selectedReportSection\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected sections");
        expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1708: deep ambiguous stable selectors should not advertise selected-settings availability");
        expect_contains(process.stdout_text, "\"selectedReportSettings\": null",
                        "#1708: deep ambiguous stable selectors should serialize null selected settings");
        expect_contains(process.stdout_text, "\"liveObjectCount\": 8",
                        "#1708: deep ambiguous stable selectors should parse objects beyond the default preview record limit");
    };

    const auto run_deep_ambiguous_section = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_deep_ambiguous_stable_section_json(asset_path);

        const auto section_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-section-guid", "--json"},
            temp_root);

        expect_no_selection(section_process, label, title);
        expect_contains(section_process.stdout_text, "\"sectionCount\": 2",
                        "#1708: deep ambiguous section selectors should preserve both live sections");
        expect_contains(section_process.stdout_text, "\"deletedSectionCount\": 0",
                        "#1708: deep ambiguous section selectors should preserve deleted section counts");
        expect_not_contains(section_process.stdout_text, "\"selectedReportSection\": {",
                            "#1708: deep ambiguous section selectors should not select the preview-window section");
    };

    const auto run_deep_ambiguous_settings = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_deep_ambiguous_stable_settings_json(asset_path);

        const auto settings_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deep-duplicate-settings-guid", "--json"},
            temp_root);

        expect_no_selection(settings_process, label, title);
        expect_contains(settings_process.stdout_text, "\"sectionCount\": 1",
                        "#1708: deep ambiguous settings selectors should preserve live section counts");
        expect_contains(settings_process.stdout_text, "\"settingCount\": 2",
                        "#1708: deep ambiguous settings selectors should preserve both root settings rows");
        expect_not_contains(settings_process.stdout_text, "\"selectedReportSettings\": [",
                            "#1708: deep ambiguous settings selectors should not select the preview-window settings row");
    };

    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.frx",
                               "deep_ambiguous_section.frx",
                               "report section");
    run_deep_ambiguous_section(temp_root / "deep_ambiguous_section.lbx",
                               "deep_ambiguous_section.lbx",
                               "label section");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.frx",
                                "deep_ambiguous_settings.frx",
                                "report settings");
    run_deep_ambiguous_settings(temp_root / "deep_ambiguous_settings.lbx",
                                "deep_ambiguous_settings.lbx",
                                "label settings");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_page_header_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_orphaned_page_header_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orphaned_page_header_object_selection = [&](const fs::path& asset_path,
                                                               const std::string& title,
                                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 1U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 1U),
               "#1671: report/label orphaned page-header object fixture should mark the containing section deleted");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected orphaned page-header object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected orphaned page-header object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1671: stable selected orphaned page-header report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1671: stable selected orphaned page-header report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1671: stable selected orphaned page-header label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1671: stable orphaned page-header object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1671: stable orphaned page-header object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1671: stable orphaned page-header object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1671: stable orphaned page-header object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1671: stable orphaned page-header object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1671: stable orphaned page-header object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"placedObjectCount\": 2",
                        "#2690: live objects inside deleted page-header sections should remain placed");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#2690: live objects inside deleted page-header sections should not be counted as unplaced");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1671: stable orphaned page-header object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1953: stable orphaned page-header object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 2000",
                        "#2690: live deleted-section page-header objects should drop out of live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1953: stable orphaned page-header object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1953: stable orphaned page-header object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1953: stable orphaned page-header object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 6100",
                        "#2690: live deleted-section page-header objects should shrink live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1953: stable orphaned page-header object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2700",
                        "#2690: live deleted-section page-header objects should expand deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2700",
                        "#2690: live deleted-section page-header objects should expand deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2900",
                        "#1953: stable orphaned page-header object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1671: stable orphaned page-header object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1671: stable orphaned page-header object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1671: stable orphaned page-header object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1671: stable orphaned page-header object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2690: live objects inside deleted page-header sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": true",
                "\"objectCount\": 1"
            },
            "#2690: live objects inside deleted page-header sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"deleted\": false",
                "\"containingSectionId\": \"page_header_1\"",
                "\"containingSectionRecordIndex\": 1",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Invoice\\\"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#2690: live objects inside deleted page-header sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 900",
                        "#1671: stable orphaned page-header object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 100",
                        "#1671: stable orphaned page-header object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1800",
                        "#1671: stable orphaned page-header object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2700",
                        "#1671: stable orphaned page-header object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 350",
                        "#1671: stable orphaned page-header object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 450",
                        "#1671: stable orphaned page-header object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 4, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 4, \"value\": \"\\\"Invoice\\\"\"",
                        "#1671: stable orphaned page-header object selections should expose selected-object expression provenance");
    };

    run_orphaned_page_header_object_selection(temp_root / "selected_orphaned_page_header_object_stable.frx",
                                              "selected_orphaned_page_header_object_stable.frx",
                                              "report");
    run_orphaned_page_header_object_selection(temp_root / "selected_orphaned_page_header_object_stable.lbx",
                                              "selected_orphaned_page_header_object_stable.lbx",
                                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_detail_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_orphaned_detail_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_orphaned_detail_object_selection = [&](const fs::path& asset_path,
                                                          const std::string& title,
                                                          const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 2U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 2U),
               "#1672: report/label orphaned detail object fixture should mark the containing section deleted");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "field-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected orphaned detail object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected orphaned detail object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1672: stable selected orphaned detail report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1672: stable selected orphaned detail report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1672: stable selected orphaned detail label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1672: stable orphaned detail object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1672: stable orphaned detail object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1672: stable orphaned detail object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1672: stable orphaned detail object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1672: stable orphaned detail object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1672: stable orphaned detail object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"placedObjectCount\": 2",
                        "#2690: live objects inside deleted detail sections should remain placed");
        expect_contains(object_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#2690: live objects inside deleted detail sections should not be counted as unplaced");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1672: stable orphaned detail object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1954: stable orphaned detail object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1954: stable orphaned detail object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1954: stable orphaned detail object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#2690: live deleted-section detail objects should drop out of live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1954: stable orphaned detail object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1954: stable orphaned detail object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1954: stable orphaned detail object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1954: stable orphaned detail object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 5200",
                        "#2690: live deleted-section detail objects should expand deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2200",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1954: stable orphaned detail object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1672: stable orphaned detail object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1672: stable orphaned detail object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1672: stable orphaned detail object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1672: stable orphaned detail object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2690: live objects inside deleted detail sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"objectCount\": 1"
            },
            "#2690: live objects inside deleted detail sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 8",
                "\"objectKind\": \"field\"",
                "\"expression\": \"customer.company\"",
                "\"expressionFieldIndex\": 2",
                "\"highlightCount\": 2"
            },
            "#2690: live objects inside deleted detail sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 1200",
                        "#1672: stable orphaned detail object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 2600",
                        "#1672: stable orphaned detail object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 4000",
                        "#1672: stable orphaned detail object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 5200",
                        "#1672: stable orphaned detail object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 450",
                        "#1672: stable orphaned detail object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 3050",
                        "#1672: stable orphaned detail object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"FONTFACE\", \"recordIndex\": 3, \"fieldIndex\": 7, \"sourceLineIndex\": null, \"memoBlockNumber\": 3, \"value\": \"Segoe UI\"",
                        "#1672: stable orphaned detail object selections should expose selected-object field provenance");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1672: stable orphaned detail object selections should preserve sibling page-header metadata");
    };

    run_orphaned_detail_object_selection(temp_root / "selected_orphaned_detail_object_stable.frx",
                                         "selected_orphaned_detail_object_stable.frx",
                                         "report");
    run_orphaned_detail_object_selection(temp_root / "selected_orphaned_detail_object_stable.lbx",
                                         "selected_orphaned_detail_object_stable.lbx",
                                         "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_exposes_selected_deleted_detail_report_objects_orphaned_by_deleted_sections(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_selected_deleted_orphaned_detail_report_objects_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_deleted_orphaned_detail_object_selection = [&](const fs::path& asset_path,
                                                                  const std::string& title,
                                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        const auto delete_section_result =
            copperfin::vfp::set_record_deleted_flag(asset_path.string(), 2U, true);
        expect(delete_section_result.ok && dbf_record_deleted(asset_path, 2U),
               "#1673: report/label deleted orphaned detail object fixture should mark the containing section deleted");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1673: report/label deleted orphaned detail object fixture should preserve deleted object state");

        const auto object_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--unique-id", "deleted-label-guid", "--json"},
            temp_root);

        if (object_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable selected deleted orphaned detail object stdout:\n"
                      << object_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable selected deleted orphaned detail object stderr:\n"
                      << object_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(object_process.exit_code == 0,
               "#1673: stable selected deleted orphaned detail report/label object JSON should exit successfully");
        expect_contains(object_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1673: stable selected deleted orphaned detail report/label object JSON should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(object_process.stdout_text, "\"isLabel\": true",
                            "#1673: stable selected deleted orphaned detail label object JSON should retain label identity");
        }
        expect_contains(object_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1673: stable deleted orphaned detail object selections should advertise selected-object availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionAvailable\": true",
                        "#1673: stable deleted orphaned detail object selections should advertise report-selection availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1673: stable deleted orphaned detail object selections should expose object selection kind");
        expect_contains(object_process.stdout_text, "\"sectionCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should update live section counts");
        expect_contains(object_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should expose deleted section counts");
        expect_contains(object_process.stdout_text, "\"liveObjectCount\": 3",
                        "#1673: stable deleted orphaned detail object selections should preserve live object counts");
        expect_contains(object_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1673: stable deleted orphaned detail object selections should preserve deleted object counts");
        expect_contains(object_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview availability");
        expect_contains(object_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview left bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview top bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview right bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview widths");
        expect_contains(object_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1955: stable deleted orphaned detail object JSON should preserve live preview heights");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1955: stable deleted orphaned detail object JSON should expose deleted preview availability");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview left bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview top bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview right bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview bottom bounds");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2200",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview widths");
        expect_contains(object_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1955: stable deleted orphaned detail object JSON should preserve deleted preview heights");
        expect_contains(object_process.stdout_text, "\"selectedReportSectionAvailable\": false",
                        "#1673: stable deleted orphaned detail object selections should not advertise selected-section availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSection\": null",
                        "#1673: stable deleted orphaned detail object selections should serialize null selected sections");
        expect_contains(object_process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                        "#1673: stable deleted orphaned detail object selections should not advertise selected-settings availability");
        expect_contains(object_process.stdout_text, "\"selectedReportSettings\": null",
                        "#1673: stable deleted orphaned detail object selections should serialize null selected settings");
        expect_contains(object_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#2689: deleted objects inside deleted detail sections should advertise containing-section availability");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObjectSection\": {",
                "\"id\": \"detail_2\"",
                "\"bandKind\": \"detail\"",
                "\"recordIndex\": 2",
                "\"deleted\": true",
                "\"objectCount\": 1",
                "\"deletedObjectCount\": 1"
            },
            "#2689: deleted objects inside deleted detail sections should expose deleted containing-section JSON");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 900",
                "\"sectionObjectIndex\": 0",
                "\"sectionObjectCount\": 1",
                "\"objectTypeCode\": 5",
                "\"objectKind\": \"label\"",
                "\"expression\": \"\\\"Deleted label\\\"\"",
                "\"expressionFieldIndex\": 2"
            },
            "#2689: deleted objects inside deleted detail sections should expose selected object metadata with deleted-section membership");
        expect_contains(object_process.stdout_text, "\"left\": 1000",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object left bounds");
        expect_contains(object_process.stdout_text, "\"top\": 2600",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object top bounds");
        expect_contains(object_process.stdout_text, "\"width\": 1200",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object widths");
        expect_contains(object_process.stdout_text, "\"right\": 2200",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object right bounds");
        expect_contains(object_process.stdout_text, "\"height\": 300",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object heights");
        expect_contains(object_process.stdout_text, "\"bottom\": 2900",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object bottom bounds");
        expect_contains(object_process.stdout_text,
                        "\"name\": \"EXPR\", \"recordIndex\": 6, \"fieldIndex\": 2, \"sourceLineIndex\": null, \"memoBlockNumber\": 5, \"value\": \"\\\"Deleted label\\\"\"",
                        "#1673: stable deleted orphaned detail object selections should expose selected-object expression provenance");
        expect_contains_in_order(
            object_process.stdout_text,
            {
                "\"sections\": [",
                "\"id\": \"page_header_1\"",
                "\"bandKind\": \"page_header\"",
                "\"recordIndex\": 1",
                "\"deleted\": false"
            },
            "#1673: stable deleted orphaned detail object selections should preserve sibling page-header metadata");
    };

    run_deleted_orphaned_detail_object_selection(
        temp_root / "selected_deleted_orphaned_detail_object_stable.frx",
        "selected_deleted_orphaned_detail_object_stable.frx",
        "report");
    run_deleted_orphaned_detail_object_selection(
        temp_root / "selected_deleted_orphaned_detail_object_stable.lbx",
        "selected_deleted_orphaned_detail_object_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_detail_header_footer_section_heights_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_height_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section height clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section height clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1805: deleted detail-header section height clear by stable selection should exit successfully");
            const auto header_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "HEIGHT"
            });
            expect(header_height_property.ok && header_height_property.exists &&
                       header_height_property.direct_field && header_height_property.value.empty(),
                   "#1805: deleted detail-header section height clear should blank the HEIGHT field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1805: deleted detail-header section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1805: deleted detail-header label section height clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1805: deleted detail-header section height clear should preserve live section count");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1805: deleted detail-header section height clear should preserve deleted section count");
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1805: deleted detail-header section height clear should preserve live section heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 250",
                            "#1805: deleted detail-header section height clear should refresh deleted section heights");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: deleted detail-header section height clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: deleted detail-header section height clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1822: deleted detail-header section height clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: deleted detail-header section height clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1822: deleted detail-header section height clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#1822: deleted detail-header section height clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 550",
                            "#1822: deleted detail-header section height clear should refresh deleted preview heights");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1805: deleted detail-header section height clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1805: deleted detail-header section height clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2238: deleted detail-header section height clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2238: deleted detail-header section height clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2238: deleted detail-header section height clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2238: deleted detail-header section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 500",
                    "\"height\": 0",
                    "\"bottom\": 500"
                },
                "#1805: deleted detail-header section height clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section height clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section height clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1805: deleted detail-footer section height clear by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.direct_field && footer_height_property.value.empty(),
                   "#1805: deleted detail-footer section height clear should blank the HEIGHT field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1805: deleted detail-footer section height clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1805: deleted detail-footer label section height clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1805: deleted detail-footer section height clear should preserve live section count");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1805: deleted detail-footer section height clear should preserve deleted section count");
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1805: deleted detail-footer section height clear should preserve live section heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 0",
                            "#1805: deleted detail-footer section height clear should refresh deleted section heights");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1822: deleted detail-footer section height clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1822: deleted detail-footer section height clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1822: deleted detail-footer section height clear should preserve live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1822: deleted detail-footer section height clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1822: deleted detail-footer section height clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 800",
                            "#1822: deleted detail-footer section height clear should shrink deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#1822: deleted detail-footer section height clear should shrink deleted preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1805: deleted detail-footer section height clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1805: deleted detail-footer section height clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2238: deleted detail-footer section height clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2238: deleted detail-footer section height clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2238: deleted detail-footer section height clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property HEIGHT\"",
                            "#2238: deleted detail-footer section height clear JSON should expose height undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 800",
                    "\"height\": 0",
                    "\"bottom\": 800"
                },
                "#1805: deleted detail-footer section height clear should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_height_clear(
        temp_root / "deleted_detail_header_footer_section_height_clear_stable.frx",
        "deleted_detail_header_footer_section_height_clear_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_height_clear(
        temp_root / "deleted_detail_header_footer_section_height_clear_stable.lbx",
        "deleted_detail_header_footer_section_height_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_top_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "40",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section top update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section top update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1806: detail-header section top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "40",
                   "#1806: detail-header section top update should persist the VPOS field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1806: detail-header section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1806: detail-header label section top update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1806: detail-header section top update should preserve live section height totals");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1806: detail-header section top update should preserve deleted section height totals");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2277: detail-header section top update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 40",
                            "#2277: detail-header section top update should refresh live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2277: detail-header section top update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsHeight\": 510",
                            "#2277: detail-header section top update should refresh live preview heights");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2277: detail-header section top update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2277: detail-header section top update should preserve deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2277: detail-header section top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2277: detail-header section top update should preserve deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1806: detail-header section top update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1806: detail-header section top update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2233: detail-header section top update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2233: detail-header section top update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2233: detail-header section top update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2233: detail-header section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 40",
                    "\"height\": 300",
                    "\"bottom\": 340"
                },
                "#1806: detail-header section top update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "VPOS",
                    "--property-value", "360",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section top update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section top update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1806: detail-footer section top update by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.value == "360",
                   "#1806: detail-footer section top update should persist the VPOS field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1806: detail-footer section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1806: detail-footer label section top update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1806: detail-footer section top update should preserve live section height totals");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1806: detail-footer section top update should preserve deleted section height totals");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2277: detail-footer section top update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 40",
                            "#2277: detail-footer section top update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 610",
                            "#2277: detail-footer section top update should refresh live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsHeight\": 570",
                            "#2277: detail-footer section top update should refresh live preview heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2277: detail-footer section top update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#2277: detail-footer section top update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#2277: detail-footer section top update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 200",
                            "#2277: detail-footer section top update should preserve deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1806: detail-footer section top update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1806: detail-footer section top update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2233: detail-footer section top update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2233: detail-footer section top update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2233: detail-footer section top update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2233: detail-footer section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 360",
                    "\"height\": 250",
                    "\"bottom\": 610"
                },
                "#1806: detail-footer section top update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_top_update(
        temp_root / "detail_header_footer_section_top_stable.frx",
        "detail_header_footer_section_top_stable.frx",
        "report");
    run_detail_header_footer_section_top_update(
        temp_root / "detail_header_footer_section_top_stable.lbx",
        "detail_header_footer_section_top_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_top_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section top clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section top clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1807: detail-header section top clear by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.direct_field && header_top_property.value.empty(),
                   "#1807: detail-header section top clear should blank the VPOS field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1807: detail-header section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1807: detail-header label section top clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1807: detail-header section top clear should preserve live section height totals");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1807: detail-header section top clear should preserve deleted section height totals");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: detail-header section top clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: detail-header section top clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1823: detail-header section top clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1823: detail-header section top clear should preserve live preview heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: detail-header section top clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1823: detail-header section top clear should preserve deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1823: detail-header section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1807: detail-header section top clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1807: detail-header section top clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2235: detail-header section top clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2235: detail-header section top clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2235: detail-header section top clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2235: detail-header section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1807: detail-header section top clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section top clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section top clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1807: detail-footer section top clear by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.direct_field && footer_top_property.value.empty(),
                   "#1807: detail-footer section top clear should blank the VPOS field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1807: detail-footer section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1807: detail-footer label section top clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 550",
                            "#1807: detail-footer section top clear should preserve live section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 200",
                            "#1807: detail-footer section top clear should preserve deleted section height totals");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: detail-footer section top clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: detail-footer section top clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 300",
                            "#1823: detail-footer section top clear should shrink live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsHeight\": 300",
                            "#1823: detail-footer section top clear should shrink live preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: detail-footer section top clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1823: detail-footer section top clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1823: detail-footer section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1807: detail-footer section top clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1807: detail-footer section top clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2235: detail-footer section top clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2235: detail-footer section top clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2235: detail-footer section top clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2235: detail-footer section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"objectCode\": 10",
                    "\"top\": 0",
                    "\"height\": 250",
                    "\"bottom\": 250"
                },
                "#1807: detail-footer section top clear should refresh selected-section geometry");
        };

    run_detail_header_footer_section_top_clear(
        temp_root / "detail_header_footer_section_top_clear_stable.frx",
        "detail_header_footer_section_top_clear_stable.frx",
        "report");
    run_detail_header_footer_section_top_clear(
        temp_root / "detail_header_footer_section_top_clear_stable.lbx",
        "detail_header_footer_section_top_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_top_update =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "620",
                    "--json"
                },
                temp_root);

            if (update_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section top update stdout:\n"
                          << update_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section top update stderr:\n"
                          << update_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_process.exit_code == 0,
                   "#1808: deleted detail-header section top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "620",
                   "#1808: deleted detail-header section top update should persist the VPOS field");
            expect_contains(update_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1808: deleted detail-header section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_process.stdout_text, "\"isLabel\": true",
                                "#1808: deleted detail-header label section top update should retain label identity");
            }
            expect_contains(update_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1808: deleted detail-header section top update should preserve live section count");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1808: deleted detail-header section top update should preserve deleted section count");
            expect_contains(update_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1808: deleted detail-header section top update should preserve live section heights");
            expect_contains(update_header_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1808: deleted detail-header section top update should preserve deleted section heights");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2237: deleted detail-header section top update should preserve live preview availability");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2237: deleted detail-header section top update should preserve live preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2237: deleted detail-header section top update should preserve live preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2237: deleted detail-header section top update should preserve deleted preview availability");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                            "#2237: deleted detail-header section top update should refresh deleted preview top bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#2237: deleted detail-header section top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 430",
                            "#2237: deleted detail-header section top update should refresh deleted preview heights");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1808: deleted detail-header section top update should preserve selected section availability");
            expect_contains(update_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1808: deleted detail-header section top update should preserve selection kind");
            expect_contains(update_header_process.stdout_text, "\"dryRun\": false",
                            "#2237: deleted detail-header section top update JSON should expose committed state");
            expect_contains(update_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2237: deleted detail-header section top update JSON should expose mutation state");
            expect_contains(update_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2237: deleted detail-header section top update JSON should expose undo availability");
            expect_contains(update_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2237: deleted detail-header section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 620",
                    "\"height\": 300",
                    "\"bottom\": 920"
                },
                "#1808: deleted detail-header section top update should refresh selected-section geometry");

            const auto update_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "VPOS",
                    "--property-value", "940",
                    "--json"
                },
                temp_root);

            if (update_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section top update stdout:\n"
                          << update_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section top update stderr:\n"
                          << update_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_process.exit_code == 0,
                   "#1808: deleted detail-footer section top update by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.value == "940",
                   "#1808: deleted detail-footer section top update should persist the VPOS field");
            expect_contains(update_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1808: deleted detail-footer section top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_process.stdout_text, "\"isLabel\": true",
                                "#1808: deleted detail-footer label section top update should retain label identity");
            }
            expect_contains(update_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1808: deleted detail-footer section top update should preserve live section count");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1808: deleted detail-footer section top update should preserve deleted section count");
            expect_contains(update_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1808: deleted detail-footer section top update should preserve live section heights");
            expect_contains(update_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1808: deleted detail-footer section top update should preserve deleted section heights");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2237: deleted detail-footer section top update should preserve live preview availability");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2237: deleted detail-footer section top update should preserve live preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#2237: deleted detail-footer section top update should preserve live preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2237: deleted detail-footer section top update should preserve deleted preview availability");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 620",
                            "#2237: deleted detail-footer section top update should preserve deleted preview top bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1190",
                            "#2237: deleted detail-footer section top update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 570",
                            "#2237: deleted detail-footer section top update should refresh deleted preview heights");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1808: deleted detail-footer section top update should preserve selected section availability");
            expect_contains(update_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1808: deleted detail-footer section top update should preserve selection kind");
            expect_contains(update_footer_process.stdout_text, "\"dryRun\": false",
                            "#2237: deleted detail-footer section top update JSON should expose committed state");
            expect_contains(update_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2237: deleted detail-footer section top update JSON should expose mutation state");
            expect_contains(update_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2237: deleted detail-footer section top update JSON should expose undo availability");
            expect_contains(update_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2237: deleted detail-footer section top update JSON should expose top undo labels");
            expect_contains_in_order(
                update_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 940",
                    "\"height\": 250",
                    "\"bottom\": 1190"
                },
                "#1808: deleted detail-footer section top update should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_top_update(
        temp_root / "deleted_detail_header_footer_section_top_stable.frx",
        "deleted_detail_header_footer_section_top_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_top_update(
        temp_root / "deleted_detail_header_footer_section_top_stable.lbx",
        "deleted_detail_header_footer_section_top_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_detail_header_footer_section_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_top_clear =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto clear_header_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_header_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section top clear stdout:\n"
                          << clear_header_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section top clear stderr:\n"
                          << clear_header_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_header_process.exit_code == 0,
                   "#1809: deleted detail-header section top clear by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.direct_field && header_top_property.value.empty(),
                   "#1809: deleted detail-header section top clear should blank the VPOS field");
            expect_contains(clear_header_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1809: deleted detail-header section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_header_process.stdout_text, "\"isLabel\": true",
                                "#1809: deleted detail-header label section top clear should retain label identity");
            }
            expect_contains(clear_header_process.stdout_text, "\"sectionCount\": 1",
                            "#1809: deleted detail-header section top clear should preserve live section count");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1809: deleted detail-header section top clear should preserve deleted section count");
            expect_contains(clear_header_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1809: deleted detail-header section top clear should preserve live section heights");
            expect_contains(clear_header_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1809: deleted detail-header section top clear should preserve deleted section heights");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: deleted detail-header section top clear should preserve live preview availability");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: deleted detail-header section top clear should preserve live preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1823: deleted detail-header section top clear should preserve live preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: deleted detail-header section top clear should preserve deleted preview availability");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#1823: deleted detail-header section top clear should refresh deleted preview top bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1050",
                            "#1823: deleted detail-header section top clear should preserve deleted preview bottom bounds");
            expect_contains(clear_header_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1050",
                            "#1823: deleted detail-header section top clear should refresh deleted preview heights");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1809: deleted detail-header section top clear should preserve selected section availability");
            expect_contains(clear_header_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1809: deleted detail-header section top clear should preserve selection kind");
            expect_contains(clear_header_process.stdout_text, "\"dryRun\": false",
                            "#2239: deleted detail-header section top clear JSON should expose committed state");
            expect_contains(clear_header_process.stdout_text, "\"mutatesAsset\": true",
                            "#2239: deleted detail-header section top clear JSON should expose mutation state");
            expect_contains(clear_header_process.stdout_text, "\"undoAvailable\": true",
                            "#2239: deleted detail-header section top clear JSON should expose undo availability");
            expect_contains(clear_header_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2239: deleted detail-header section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_header_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 9",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#1809: deleted detail-header section top clear should refresh selected-section geometry");

            const auto clear_footer_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--clear-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "VPOS",
                    "--json"
                },
                temp_root);

            if (clear_footer_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section top clear stdout:\n"
                          << clear_footer_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section top clear stderr:\n"
                          << clear_footer_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(clear_footer_process.exit_code == 0,
                   "#1809: deleted detail-footer section top clear by stable selection should exit successfully");
            const auto footer_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "VPOS"
            });
            expect(footer_top_property.ok && footer_top_property.exists &&
                       footer_top_property.direct_field && footer_top_property.value.empty(),
                   "#1809: deleted detail-footer section top clear should blank the VPOS field");
            expect_contains(clear_footer_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1809: deleted detail-footer section top clear should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(clear_footer_process.stdout_text, "\"isLabel\": true",
                                "#1809: deleted detail-footer label section top clear should retain label identity");
            }
            expect_contains(clear_footer_process.stdout_text, "\"sectionCount\": 1",
                            "#1809: deleted detail-footer section top clear should preserve live section count");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionCount\": 2",
                            "#1809: deleted detail-footer section top clear should preserve deleted section count");
            expect_contains(clear_footer_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1809: deleted detail-footer section top clear should preserve live section heights");
            expect_contains(clear_footer_process.stdout_text, "\"deletedSectionHeightTotal\": 550",
                            "#1809: deleted detail-footer section top clear should preserve deleted section heights");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1823: deleted detail-footer section top clear should preserve live preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1823: deleted detail-footer section top clear should preserve live preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1823: deleted detail-footer section top clear should preserve live preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1823: deleted detail-footer section top clear should preserve deleted preview availability");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#1823: deleted detail-footer section top clear should preserve deleted preview top bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsBottom\": 300",
                            "#1823: deleted detail-footer section top clear should shrink deleted preview bottom bounds");
            expect_contains(clear_footer_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#1823: deleted detail-footer section top clear should shrink deleted preview heights");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1809: deleted detail-footer section top clear should preserve selected section availability");
            expect_contains(clear_footer_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1809: deleted detail-footer section top clear should preserve selection kind");
            expect_contains(clear_footer_process.stdout_text, "\"dryRun\": false",
                            "#2239: deleted detail-footer section top clear JSON should expose committed state");
            expect_contains(clear_footer_process.stdout_text, "\"mutatesAsset\": true",
                            "#2239: deleted detail-footer section top clear JSON should expose mutation state");
            expect_contains(clear_footer_process.stdout_text, "\"undoAvailable\": true",
                            "#2239: deleted detail-footer section top clear JSON should expose undo availability");
            expect_contains(clear_footer_process.stdout_text, "\"undoLabel\": \"Property VPOS\"",
                            "#2239: deleted detail-footer section top clear JSON should expose top undo labels");
            expect_contains_in_order(
                clear_footer_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"objectCode\": 10",
                    "\"top\": 0",
                    "\"height\": 250",
                    "\"bottom\": 250"
                },
                "#1809: deleted detail-footer section top clear should refresh selected-section geometry");
        };

    run_deleted_detail_header_footer_section_top_clear(
        temp_root / "deleted_detail_header_footer_section_top_clear_stable.frx",
        "deleted_detail_header_footer_section_top_clear_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_top_clear(
        temp_root / "deleted_detail_header_footer_section_top_clear_stable.lbx",
        "deleted_detail_header_footer_section_top_clear_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_and_restores_detail_header_footer_sections_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_detail_header_footer_section_delete_restore_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_delete_restore =
        [&](const fs::path& header_delete_path,
            const fs::path& footer_delete_path,
            const fs::path& header_restore_path,
            const fs::path& footer_restore_path,
            const std::string& label) {
            const auto expect_document_identity = [&](const ProcessResult& process,
                                                      const fs::path& asset_path,
                                                      const std::string& operation_label) {
                expect_contains(process.stdout_text, "\"documentTitle\": \"" + asset_path.filename().string() + "\"",
                                "#1812: " + operation_label + " should return refreshed layout JSON");
                if (asset_path.extension() == ".lbx") {
                    expect_contains(process.stdout_text, "\"isLabel\": true",
                                    "#1812: " + operation_label + " should retain label identity");
                }
            };

            const auto expect_selected_section_state =
                [&](const ProcessResult& process,
                    const std::string& section_title,
                    const std::string& band_kind,
                    const std::string& record_index,
                    const std::string& deleted,
                    const std::string& section_index,
                    const std::string& section_count,
                    const std::string& object_count,
                    const std::string& operation_label) {
                    expect_contains(process.stdout_text, "\"selectedReportSectionAvailable\": true",
                                    "#1812: " + operation_label + " should advertise selected sections");
                    expect_contains(process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                                    "#1812: " + operation_label + " should expose section selection kind");
                    expect_contains(process.stdout_text, "\"selectedReportObjectAvailable\": false",
                                    "#1812: " + operation_label + " should not select report objects");
                    expect_contains(process.stdout_text, "\"selectedReportSettingsAvailable\": false",
                                    "#1812: " + operation_label + " should not select settings");
                    expect_contains(process.stdout_text, "\"dryRun\": false",
                                    "#2240: " + operation_label + " JSON should expose committed state");
                    expect_contains(process.stdout_text, "\"mutatesAsset\": true",
                                    "#2240: " + operation_label + " JSON should expose mutation state");
                    expect_contains(process.stdout_text, "\"undoAvailable\": false",
                                    "#2240: " + operation_label + " JSON should expose undo availability");
                    expect_contains(process.stdout_text, "\"undoLabel\": \"\"",
                                    "#2240: " + operation_label + " JSON should expose empty undo labels");
                    expect_contains_in_order(
                        process.stdout_text,
                        {
                            "\"selectedReportSection\": {",
                            "\"title\": \"" + section_title + "\"",
                            "\"bandKind\": \"" + band_kind + "\"",
                            "\"recordIndex\": " + record_index,
                            "\"deleted\": " + deleted,
                            "\"sectionIndex\": " + section_index,
                            "\"sectionCount\": " + section_count,
                            "\"objectCount\": " + object_count
                        },
                        "#1812: " + operation_label + " should refresh selected-section metadata");
                };

            const auto delete_section = [&](const fs::path& asset_path,
                                            const std::string& unique_id,
                                            std::size_t section_record_index,
                                            const std::string& section_title,
                                            const std::string& band_kind,
                                            const std::string& deleted_section_id,
                                            const std::string& live_sibling_title,
                                            const std::string& live_sibling_band_kind,
                                            const std::string& live_sibling_record_index,
                                            const std::string& orphan_object_record_index,
                                            const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

                const auto delete_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--delete-object",
                        "--unique-id", unique_id,
                        "--json"
                    },
                    temp_root);

                if (delete_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << delete_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << delete_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(delete_process.exit_code == 0,
                       "#1812: " + operation_label + " should exit successfully");
                expect(dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " should mark the section record deleted");
                expect_document_identity(delete_process, asset_path, operation_label);
                expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                                "#1812: " + operation_label + " should preserve the remaining live section");
                expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                                "#1812: " + operation_label + " should expose the deleted section count");
                expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                                "#1812: " + operation_label + " should keep sibling section objects placed");
                expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                                "#1812: " + operation_label + " should not orphan former section objects");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + live_sibling_title + "\"",
                        "\"bandKind\": \"" + live_sibling_band_kind + "\"",
                        "\"recordIndex\": " + live_sibling_record_index,
                        "\"deleted\": false",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should preserve live sibling section metadata");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"recordIndex\": " + std::to_string(section_record_index),
                        "\"deleted\": true",
                        "\"sectionIndex\": null",
                        "\"sectionCount\": 0",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should move the section to deleted-section metadata");
                expect_contains_in_order(
                    delete_process.stdout_text,
                    {
                        "\"deletedSections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"objects\": [",
                        "\"recordIndex\": " + orphan_object_record_index,
                        "\"deleted\": false",
                        "\"containingSectionId\": \"" + deleted_section_id + "\""
                    },
                    "#1812: " + operation_label + " should retain former section object containment inside deleted sections");
                expect_selected_section_state(delete_process,
                                              section_title,
                                              band_kind,
                                              std::to_string(section_record_index),
                                              "true",
                                              "null",
                                              "0",
                                              "1",
                                              operation_label);
            };

            const auto restore_section = [&](const fs::path& asset_path,
                                             const std::string& unique_id,
                                             std::size_t section_record_index,
                                             const std::string& section_title,
                                             const std::string& band_kind,
                                             const std::string& section_index,
                                             const std::string& containing_section_id,
                                             const std::string& placed_object_record_index,
                                             const std::string& operation_label) {
                write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);
                const auto delete_result =
                    copperfin::vfp::set_record_deleted_flag(asset_path.string(), section_record_index, true);
                expect(delete_result.ok && dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " fixture should start with a deleted section");

                const auto restore_process = run_process_capture(
                    studio_host_path,
                    {
                        "--path", asset_path.string(),
                        "--restore-object",
                        "--unique-id", unique_id,
                        "--json"
                    },
                    temp_root);

                if (restore_process.exit_code != 0) {
                    std::cerr << "studio host " << label << " " << operation_label << " stdout:\n"
                              << restore_process.stdout_text << "\n";
                    std::cerr << "studio host " << label << " " << operation_label << " stderr:\n"
                              << restore_process.stderr_text << "\n";
                    std::cerr << "fixture root: " << temp_root << "\n";
                }

                expect(restore_process.exit_code == 0,
                       "#1812: " + operation_label + " should exit successfully");
                expect(!dbf_record_deleted(asset_path, section_record_index),
                       "#1812: " + operation_label + " should clear the section deleted state");
                expect_document_identity(restore_process, asset_path, operation_label);
                expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                                "#1812: " + operation_label + " should restore live section counts");
                expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                                "#1812: " + operation_label + " should clear deleted section counts");
                expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                                "#1812: " + operation_label + " should restore placed object counts");
                expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                                "#1812: " + operation_label + " should clear unplaced object counts");
                expect_contains_in_order(
                    restore_process.stdout_text,
                    {
                        "\"sections\": [",
                        "\"title\": \"" + section_title + "\"",
                        "\"bandKind\": \"" + band_kind + "\"",
                        "\"recordIndex\": " + std::to_string(section_record_index),
                        "\"deleted\": false",
                        "\"sectionIndex\": " + section_index,
                        "\"sectionCount\": 2",
                        "\"objectCount\": 1"
                    },
                    "#1812: " + operation_label + " should move the section back to live metadata");
                expect_contains_in_order(
                    restore_process.stdout_text,
                    {
                        "\"objects\": [",
                        "\"recordIndex\": " + placed_object_record_index,
                        "\"deleted\": false",
                        "\"containingSectionId\": \"" + containing_section_id + "\"",
                        "\"containingSectionRecordIndex\": " + std::to_string(section_record_index),
                        "\"sectionObjectIndex\": 0",
                        "\"sectionObjectCount\": 1"
                    },
                    "#1812: " + operation_label + " should restore object containing-section metadata");
                expect_selected_section_state(restore_process,
                                              section_title,
                                              band_kind,
                                              std::to_string(section_record_index),
                                              "false",
                                              section_index,
                                              "2",
                                              "1",
                                              operation_label);
            };

            delete_section(header_delete_path,
                           "detail-header-guid",
                           0U,
                           "Detail Header",
                           "detail_header",
                           "detail-header-guid",
                           "Detail Footer",
                           "detail_footer",
                           "2",
                           "1",
                           "stable detail-header section delete");
            delete_section(footer_delete_path,
                           "detail-footer-guid",
                           2U,
                           "Detail Footer",
                           "detail_footer",
                           "detail-footer-guid",
                           "Detail Header",
                           "detail_header",
                           "0",
                           "3",
                           "stable detail-footer section delete");
            restore_section(header_restore_path,
                            "detail-header-guid",
                            0U,
                            "Detail Header",
                            "detail_header",
                            "0",
                            "detail-header-guid",
                            "1",
                            "stable detail-header section restore");
            restore_section(footer_restore_path,
                            "detail-footer-guid",
                            2U,
                            "Detail Footer",
                            "detail_footer",
                            "1",
                            "detail-footer-guid",
                            "3",
                            "stable detail-footer section restore");
        };

    run_detail_header_footer_section_delete_restore(
        temp_root / "detail_header_section_delete.frx",
        temp_root / "detail_footer_section_delete.frx",
        temp_root / "detail_header_section_restore.frx",
        temp_root / "detail_footer_section_restore.frx",
        "report");
    run_detail_header_footer_section_delete_restore(
        temp_root / "detail_header_section_delete.lbx",
        temp_root / "detail_footer_section_delete.lbx",
        temp_root / "detail_header_section_restore.lbx",
        temp_root / "detail_footer_section_restore.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json
