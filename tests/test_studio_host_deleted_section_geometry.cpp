#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void write_synthetic_report_table_for_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_reorder_json(report_path);
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_result.ok, "#1474: synthetic FRX table should mark report section deleted");
}

void write_synthetic_report_table_for_stable_deleted_section_json(const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_deleted_section_json(report_path);
    const auto unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "deleted-section-guid"
    });
    expect(unique_id_result.ok, "#1654: stable deleted section fixture should seed a deleted section unique id");
    expect(dbf_record_deleted(report_path, 1U),
           "#1654: stable deleted section fixture should preserve the deleted section state");
}

void test_studio_host_json_updates_deleted_report_section_heights_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1919: deleted report/label section height update should preserve empty live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1919: deleted report/label section height update should preserve empty live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1919: deleted report/label section height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1919: deleted report/label section height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1919: deleted report/label section height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1919: deleted report/label section height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4400",
                        "#1919: deleted report/label section height update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
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
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1575: deleted report/label section height update should preserve section-owned object accounting");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                        "#1575: deleted report/label section height update should preserve containing sections");
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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

void test_studio_host_json_updates_deleted_report_section_tops_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_top_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1917: deleted report/label section top update should preserve empty live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1917: deleted report/label section top update should preserve empty live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1917: deleted report/label section top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1917: deleted report/label section top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2500",
                        "#1917: deleted report/label section top update should refresh deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1917: deleted report/label section top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7500",
                        "#1917: deleted report/label section top update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
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
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1577: deleted report/label section top update should preserve section-owned object accounting");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                        "#1577: deleted report/label section top update should preserve containing sections");
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1918: deleted report/label section top clear should preserve empty live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1918: deleted report/label section top clear should preserve empty live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1918: deleted report/label section top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1918: deleted report/label section top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1918: deleted report/label section top clear should refresh deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1918: deleted report/label section top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 5000",
                        "#1918: deleted report/label section top clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
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
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1578: deleted report/label section top clear should preserve section-owned object accounting");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"detail_1\"",
                        "#1578: deleted report/label section top clear should preserve containing sections");
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

void test_studio_host_json_updates_deleted_report_section_heights_and_tops_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_section_stable_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1921: report/label stable deleted section height update should preserve empty live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1921: report/label stable deleted section height update should preserve empty live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1921: report/label stable deleted section height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1921: report/label stable deleted section height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2000",
                        "#1921: report/label stable deleted section height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1921: report/label stable deleted section height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 4400",
                        "#1921: report/label stable deleted section height update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#1921: report/label stable deleted section height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 2400",
                        "#1921: report/label stable deleted section height update should refresh deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section height update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section height update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1825: report/label stable deleted section height update should preserve section-owned object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section height update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                        "#1825: report/label stable deleted section height update should preserve section selection kind");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"deleted-section-guid\"",
                        "#1825: report/label stable deleted section height update should preserve containing sections");
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
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1923: report/label stable deleted section top update should preserve empty live preview availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview widths");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1923: report/label stable deleted section top update should preserve empty live preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1923: report/label stable deleted section top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1923: report/label stable deleted section top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2500",
                        "#1923: report/label stable deleted section top update should refresh deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1923: report/label stable deleted section top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 7500",
                        "#1923: report/label stable deleted section top update should refresh deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#1923: report/label stable deleted section top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1923: report/label stable deleted section top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section top update should not fabricate live sections");
        expect_contains(update_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section top update should preserve deleted section counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1825: report/label stable deleted section top update should preserve section-owned object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section top update should preserve selected-section availability");
        expect_contains(update_process.stdout_text, "\"containingSectionId\": \"deleted-section-guid\"",
                        "#1825: report/label stable deleted section top update should preserve containing sections");
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
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

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
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": false",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview widths");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1924: report/label stable deleted section top clear should preserve empty live preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1924: report/label stable deleted section top clear should refresh deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 150",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 5000",
                        "#1924: report/label stable deleted section top clear should refresh deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 5000",
                        "#1924: report/label stable deleted section top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"sectionCount\": 0",
                        "#1825: report/label stable deleted section top clear should not fabricate live sections");
        expect_contains(clear_process.stdout_text, "\"deletedSectionCount\": 1",
                        "#1825: report/label stable deleted section top clear should preserve deleted section counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 0",
                        "#1825: report/label stable deleted section top clear should preserve section-owned object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                        "#1825: report/label stable deleted section top clear should preserve selected-section availability");
        expect_contains(clear_process.stdout_text, "\"containingSectionId\": \"deleted-section-guid\"",
                        "#1825: report/label stable deleted section top clear should preserve containing sections");
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

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_deleted_section_geometry <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_section_heights_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_section_heights_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_section_tops_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_section_tops_by_record_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_updates_deleted_report_section_heights_and_tops_by_stable_selection(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_clears_deleted_report_section_heights_and_tops_by_stable_selection(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
