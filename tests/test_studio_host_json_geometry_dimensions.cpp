#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_negative_dimension_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", "", "", "", "", ""},
        {"9", "4", "", "", "0", "", "-500", ""},
        {"5", "", "\"Negative live\"", "300", "0", "-100", "-200", "negative-live-guid"},
        {"5", "", "\"Negative deleted\"", "700", "50", "-400", "-300", "negative-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1715: synthetic report table for negative layout dimensions should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1715: synthetic report table should mark the negative-dimension object deleted");
}

void write_synthetic_report_table_for_missing_geometry_layout_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "ORIENTATION=0", ""},
        {"9", "4", "", ""},
        {"5", "", "\"Missing geometry live\"", "missing-geometry-live-guid"},
        {"5", "", "\"Missing geometry deleted\"", "missing-geometry-deleted-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1720: synthetic report table without geometry fields should be created");
    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 3U, true);
    expect(delete_result.ok, "#1720: synthetic report table should mark the missing-geometry object deleted");
}

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

void test_studio_host_json_deletes_edited_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_edited_geometry_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1615: report/label layout object geometry pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1615: report/label layout object geometry pre-delete update should preserve live state");
        };

        set_live_geometry("HPOS", "1400");
        set_live_geometry("WIDTH", "2400");
        set_live_geometry("HEIGHT", "900");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1615: edited report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1615: edited report/label layout object delete should mark the DBF record deleted");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   width_property.ok && width_property.exists && width_property.value == "2400" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1615: edited report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1615: edited report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1887: edited label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1887: edited report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1887: edited report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1887: edited report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1887: edited report/label layout object delete should preserve deleted preview availability");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1887: edited report/label layout object delete should refresh deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1887: edited report/label layout object delete should refresh deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3800",
                        "#1887: edited report/label layout object delete should refresh deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 3500",
                        "#1887: edited report/label layout object delete should refresh deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 2800",
                        "#1887: edited report/label layout object delete should refresh deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 900",
                        "#1887: edited report/label layout object delete should refresh deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1615: edited report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1615: edited report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1615: edited report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1615: edited report/label layout object delete should not fabricate containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1615: edited report/label layout object delete should serialize null containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": 1400",
                "\"top\": 2600",
                "\"width\": 2400",
                "\"right\": 3800",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1615: edited report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_geometry_delete(temp_root / "delete_edited_geometry.frx",
                               "delete_edited_geometry.frx",
                               "report");
    run_edited_geometry_delete(temp_root / "delete_edited_geometry.lbx",
                               "delete_edited_geometry.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_deletes_edited_unplaced_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_delete_edited_unplaced_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_edited_unplaced_delete = [&](const fs::path& asset_path,
                                                const std::string& title,
                                                const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(!visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete fixture should start live");

        const auto set_live_geometry = [&](const std::string& property_name,
                                           const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "3",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " live unplaced " << property_name
                          << " pre-delete update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1616: report/label layout object unplaced pre-delete update should exit successfully");
            expect(!visual_object_deleted(asset_path, "field-guid"),
                   "#1616: report/label layout object unplaced pre-delete update should preserve live state");
        };

        set_live_geometry("HPOS", "-300");
        set_live_geometry("VPOS", "9000");
        set_live_geometry("HEIGHT", "700");

        const auto delete_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "3",
                "--delete-object",
                "--unique-id", "field-guid",
                "--json"
            },
            temp_root);

        if (delete_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited unplaced layout delete stdout:\n"
                      << delete_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited unplaced layout delete stderr:\n"
                      << delete_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(delete_process.exit_code == 0,
               "#1616: edited unplaced report/label layout object delete should exit successfully");
        expect(visual_object_deleted(asset_path, "field-guid"),
               "#1616: edited unplaced report/label layout object delete should mark the DBF record deleted");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-300" &&
                   top_property.ok && top_property.exists && top_property.value == "9000" &&
                   height_property.ok && height_property.exists && height_property.value == "700",
               "#1616: edited unplaced report/label layout object delete should preserve edited geometry fields");
        expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1616: edited unplaced report/label layout object delete should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                            "#1889: edited unplaced label layout object delete should retain label identity");
        }
        expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1889: edited unplaced report/label layout object delete should preserve live preview availability");
        expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview left bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview top bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview right bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview widths");
        expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1889: edited unplaced report/label layout object delete should refresh live preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should keep deleted preview bounds available");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": -300",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview left bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1889: edited unplaced report/label layout object delete should preserve deleted preview top bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 3700",
                        "#1889: edited unplaced report/label layout object delete should expand deleted preview right bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 9700",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview bottom bounds");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 4000",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview widths");
        expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 7100",
                        "#1616: edited unplaced report/label layout object delete should expand deleted preview heights");
        expect_contains(delete_process.stdout_text, "\"deletedObjectCount\": 2",
                        "#1616: edited unplaced report/label layout object delete should add the edited object to deleted-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedPlacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should preserve deleted placed-object counts");
        expect_contains(delete_process.stdout_text, "\"deletedUnplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should count the edited deleted object as unplaced");
        expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1616: edited unplaced report/label layout object delete should remove the edited object from live unplaced counts");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1616: edited unplaced report/label layout object delete should preserve object selection kind");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1616: edited unplaced report/label layout object delete should not fabricate containing-section availability");
        expect_contains(delete_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1616: edited unplaced report/label layout object delete should serialize null containing-section metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve edited deleted-object geometry metadata");
        expect_contains_in_order(
            delete_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"left\": -300",
                "\"top\": 9000",
                "\"width\": 4000",
                "\"right\": 3700",
                "\"height\": 700",
                "\"bottom\": 9700"
            },
            "#1616: edited unplaced report/label layout object delete should preserve selected deleted-object geometry metadata");
    };

    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.frx",
                               "delete_edited_unplaced.frx",
                               "report");
    run_edited_unplaced_delete(temp_root / "delete_edited_unplaced.lbx",
                               "delete_edited_unplaced.lbx",
                               "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1605: deleted report/label layout object width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1605: deleted report/label layout object width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "2400",
               "#1605: deleted report/label layout object width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1605: deleted report/label layout object width update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1605: deleted report/label layout object width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1605: deleted report/label layout object width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1605: deleted report/label layout object width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1605: deleted report/label layout object width update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1605: deleted report/label layout object width update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1605: deleted report/label layout object width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1605: deleted report/label layout object width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update.frx",
                             "deleted_width_update.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update.lbx",
                             "deleted_width_update.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1606: deleted report/label layout object width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1606: deleted report/label layout object width clear should preserve deleted state");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1606: deleted report/label layout object width clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1606: deleted report/label layout object width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1606: deleted report/label layout object width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1606: deleted report/label layout object width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1606: deleted report/label layout object width clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1606: deleted report/label layout object width clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1606: deleted report/label layout object width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1606: deleted report/label layout object width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear.frx",
                            "deleted_width_clear.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear.lbx",
                            "deleted_width_clear.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_update = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--property-value", "2400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1644: deleted report/label layout object stable width update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1644: deleted report/label layout object stable width update should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.value == "2400",
               "#1644: deleted report/label layout object stable width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1644: deleted report/label layout object stable width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1644: label deleted layout object stable width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1644: deleted report/label layout object stable width update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1644: deleted report/label layout object stable width update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1644: deleted report/label layout object stable width update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1644: deleted report/label layout object stable width update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1644: deleted report/label layout object stable width update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 2400",
                "\"right\": 3400"
            },
            "#1644: deleted report/label layout object stable width update should refresh selected deleted-object geometry metadata");
        expect_not_contains(update_process.stdout_text, "\"width\": 1200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object widths");
        expect_not_contains(update_process.stdout_text, "\"right\": 2200",
                            "#1644: deleted report/label layout object stable width update should not leak stale deleted-object right bounds");
    };

    run_deleted_width_update(temp_root / "deleted_width_update_stable.frx",
                             "deleted_width_update_stable.frx",
                             "report");
    run_deleted_width_update(temp_root / "deleted_width_update_stable.lbx",
                             "deleted_width_update_stable.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_width_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_width_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_width_clear = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1645: deleted report/label layout object stable width clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1645: deleted report/label layout object stable width clear should preserve deleted state");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.record_deleted &&
                   width_property.direct_field && width_property.value.empty(),
               "#1645: deleted report/label layout object stable width clear should blank the WIDTH field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1645: deleted report/label layout object stable width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1645: label deleted layout object stable width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1645: deleted report/label layout object stable width clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1645: deleted report/label layout object stable width clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1645: deleted report/label layout object stable width clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1645: deleted report/label layout object stable width clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1645: deleted report/label layout object stable width clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"width\": 0",
                "\"right\": 1000"
            },
            "#1645: deleted report/label layout object stable width clear should refresh selected deleted-object geometry metadata");
        expect_not_contains(clear_process.stdout_text, "\"width\": 1200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2200",
                            "#1645: deleted report/label layout object stable width clear should not leak stale deleted-object right bounds");
    };

    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.frx",
                            "deleted_width_clear_stable.frx",
                            "report");
    run_deleted_width_clear(temp_root / "deleted_width_clear_stable.lbx",
                            "deleted_width_clear_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_left_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1607: deleted report/label layout object left fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "HPOS",
                "--property-value", "1400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1607: deleted report/label layout object left update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1607: deleted report/label layout object left update should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400",
               "#1607: deleted report/label layout object left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1607: deleted report/label layout object left update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1607: deleted report/label layout object left update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1607: deleted report/label layout object left update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1607: deleted report/label layout object left update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1607: deleted report/label layout object left update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1607: deleted report/label layout object left update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1607: deleted report/label layout object left update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1607: deleted report/label layout object left update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_update(temp_root / "deleted_left_update.frx",
                            "deleted_left_update.frx",
                            "report");
    run_deleted_left_update(temp_root / "deleted_left_update.lbx",
                            "deleted_left_update.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_left_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_left_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1608: deleted report/label layout object left clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1608: deleted report/label layout object left clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1608: deleted report/label layout object left clear should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value.empty(),
               "#1608: deleted report/label layout object left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1608: deleted report/label layout object left clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1608: deleted report/label layout object left clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1608: deleted report/label layout object left clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1608: deleted report/label layout object left clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1608: deleted report/label layout object left clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1608: deleted report/label layout object left clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1608: deleted report/label layout object left clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1608: deleted report/label layout object left clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_clear(temp_root / "deleted_left_clear.frx",
                           "deleted_left_clear.frx",
                           "report");
    run_deleted_left_clear(temp_root / "deleted_left_clear.lbx",
                           "deleted_left_clear.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_left_update = [&](const fs::path& asset_path,
                                             const std::string& title,
                                             const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1646: deleted report/label layout object stable left fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HPOS",
                "--property-value", "1400",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1646: deleted report/label layout object stable left update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1646: deleted report/label layout object stable left update should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.record_deleted &&
                   left_property.value == "1400",
               "#1646: deleted report/label layout object stable left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1646: deleted report/label layout object stable left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1646: label deleted layout object stable left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1646: deleted report/label layout object stable left update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1646: deleted report/label layout object stable left update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1646: deleted report/label layout object stable left update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1646: deleted report/label layout object stable left update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1646: deleted report/label layout object stable left update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1646: deleted report/label layout object stable left update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"width\": 1200",
                "\"right\": 2600"
            },
            "#1646: deleted report/label layout object stable left update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_update(temp_root / "deleted_left_update_stable.frx",
                            "deleted_left_update_stable.frx",
                            "report");
    run_deleted_left_update(temp_root / "deleted_left_update_stable.lbx",
                            "deleted_left_update_stable.lbx",
                            "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_left_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_left_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_left_clear = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1647: deleted report/label layout object stable left clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1647: deleted report/label layout object stable left clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1647: deleted report/label layout object stable left clear should preserve deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.record_deleted &&
                   left_property.direct_field && left_property.value.empty(),
               "#1647: deleted report/label layout object stable left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1647: deleted report/label layout object stable left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1647: label deleted layout object stable left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1647: deleted report/label layout object stable left clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1647: deleted report/label layout object stable left clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1647: deleted report/label layout object stable left clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1647: deleted report/label layout object stable left clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1647: deleted report/label layout object stable left clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1647: deleted report/label layout object stable left clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"left\": 0",
                "\"width\": 1200",
                "\"right\": 1200"
            },
            "#1647: deleted report/label layout object stable left clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_left_clear(temp_root / "deleted_left_clear_stable.frx",
                           "deleted_left_clear_stable.frx",
                           "report");
    run_deleted_left_clear(temp_root / "deleted_left_clear_stable.lbx",
                           "deleted_left_clear_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1609: deleted report/label layout object height fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "HEIGHT",
                "--property-value", "900",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1609: deleted report/label layout object height update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1609: deleted report/label layout object height update should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "900",
               "#1609: deleted report/label layout object height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1609: deleted report/label layout object height update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1609: deleted report/label layout object height update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1609: deleted report/label layout object height update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1609: deleted report/label layout object height update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1609: deleted report/label layout object height update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1609: deleted report/label layout object height update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1609: deleted report/label layout object height update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1609: deleted report/label layout object height update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_update(temp_root / "deleted_height_update.frx",
                              "deleted_height_update.frx",
                              "report");
    run_deleted_height_update(temp_root / "deleted_height_update.lbx",
                              "deleted_height_update.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_height_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1610: deleted report/label layout object height clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1610: deleted report/label layout object height clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1610: deleted report/label layout object height clear should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value.empty(),
               "#1610: deleted report/label layout object height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1610: deleted report/label layout object height clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1610: deleted report/label layout object height clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1610: deleted report/label layout object height clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1610: deleted report/label layout object height clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1610: deleted report/label layout object height clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1610: deleted report/label layout object height clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1610: deleted report/label layout object height clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1610: deleted report/label layout object height clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_clear(temp_root / "deleted_height_clear.frx",
                             "deleted_height_clear.frx",
                             "report");
    run_deleted_height_clear(temp_root / "deleted_height_clear.lbx",
                             "deleted_height_clear.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_height_update = [&](const fs::path& asset_path,
                                               const std::string& title,
                                               const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1648: deleted report/label layout object stable height fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HEIGHT",
                "--property-value", "900",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1648: deleted report/label layout object stable height update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1648: deleted report/label layout object stable height update should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.record_deleted &&
                   height_property.value == "900",
               "#1648: deleted report/label layout object stable height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1648: deleted report/label layout object stable height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1648: label deleted layout object stable height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1648: deleted report/label layout object stable height update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1648: deleted report/label layout object stable height update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1648: deleted report/label layout object stable height update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1648: deleted report/label layout object stable height update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1648: deleted report/label layout object stable height update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1648: deleted report/label layout object stable height update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 900",
                "\"bottom\": 3500"
            },
            "#1648: deleted report/label layout object stable height update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_update(temp_root / "deleted_height_update_stable.frx",
                              "deleted_height_update_stable.frx",
                              "report");
    run_deleted_height_update(temp_root / "deleted_height_update_stable.lbx",
                              "deleted_height_update_stable.lbx",
                              "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_height_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_height_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_height_clear = [&](const fs::path& asset_path,
                                              const std::string& title,
                                              const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1649: deleted report/label layout object stable height clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1649: deleted report/label layout object stable height clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1649: deleted report/label layout object stable height clear should preserve deleted state");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.record_deleted &&
                   height_property.direct_field && height_property.value.empty(),
               "#1649: deleted report/label layout object stable height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1649: deleted report/label layout object stable height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1649: label deleted layout object stable height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1649: deleted report/label layout object stable height clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1649: deleted report/label layout object stable height clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1649: deleted report/label layout object stable height clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1649: deleted report/label layout object stable height clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1649: deleted report/label layout object stable height clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1649: deleted report/label layout object stable height clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 2600",
                "\"width\": 1200",
                "\"height\": 0",
                "\"bottom\": 2600"
            },
            "#1649: deleted report/label layout object stable height clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_height_clear(temp_root / "deleted_height_clear_stable.frx",
                             "deleted_height_clear_stable.frx",
                             "report");
    run_deleted_height_clear(temp_root / "deleted_height_clear_stable.lbx",
                             "deleted_height_clear_stable.lbx",
                             "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1611: deleted report/label layout object top fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "6",
                "--property-name", "VPOS",
                "--property-value", "3100",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1611: deleted report/label layout object top update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1611: deleted report/label layout object top update should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "3100",
               "#1611: deleted report/label layout object top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1611: deleted report/label layout object top update should return refreshed report-layout JSON");
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1611: deleted report/label layout object top update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1611: deleted report/label layout object top update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1611: deleted report/label layout object top update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1611: deleted report/label layout object top update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1611: deleted report/label layout object top update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1611: deleted report/label layout object top update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1611: deleted report/label layout object top update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_update(temp_root / "deleted_top_update.frx",
                           "deleted_top_update.frx",
                           "report");
    run_deleted_top_update(temp_root / "deleted_top_update.lbx",
                           "deleted_top_update.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_top_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_clear_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1612: deleted report/label layout object top clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "6",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " deleted layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " deleted layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1612: deleted report/label layout object top clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1612: deleted report/label layout object top clear should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value.empty(),
               "#1612: deleted report/label layout object top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1612: deleted report/label layout object top clear should return refreshed report-layout JSON");
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1612: deleted report/label layout object top clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1612: deleted report/label layout object top clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1612: deleted report/label layout object top clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1612: deleted report/label layout object top clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1612: deleted report/label layout object top clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1612: deleted report/label layout object top clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1612: deleted report/label layout object top clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_clear(temp_root / "deleted_top_clear.frx",
                          "deleted_top_clear.frx",
                          "report");
    run_deleted_top_clear(temp_root / "deleted_top_clear.lbx",
                          "deleted_top_clear.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_top_update = [&](const fs::path& asset_path,
                                            const std::string& title,
                                            const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1650: deleted report/label layout object stable top fixture should start deleted");
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "VPOS",
                "--property-value", "3100",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1650: deleted report/label layout object stable top update should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1650: deleted report/label layout object stable top update should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.record_deleted &&
                   top_property.value == "3100",
               "#1650: deleted report/label layout object stable top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1650: deleted report/label layout object stable top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1650: label deleted layout object stable top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1650: deleted report/label layout object stable top update should preserve deleted object counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1650: deleted report/label layout object stable top update should preserve selected deleted-object availability");
        expect_contains(update_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1650: deleted report/label layout object stable top update should preserve object selection kind");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1650: deleted report/label layout object stable top update should not fabricate containing-section availability");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1650: deleted report/label layout object stable top update should serialize null containing-section metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1650: deleted report/label layout object stable top update should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 3100",
                "\"height\": 300",
                "\"bottom\": 3400"
            },
            "#1650: deleted report/label layout object stable top update should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_update(temp_root / "deleted_top_update_stable.frx",
                           "deleted_top_update_stable.frx",
                           "report");
    run_deleted_top_update(temp_root / "deleted_top_update_stable.lbx",
                           "deleted_top_update_stable.lbx",
                           "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_deleted_report_layout_object_top_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_top_clear_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_top_clear = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1651: deleted report/label layout object stable top clear fixture should start deleted");
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "deleted-label-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable deleted layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable deleted layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1651: deleted report/label layout object stable top clear should exit successfully");
        expect(dbf_record_deleted(asset_path, 6U),
               "#1651: deleted report/label layout object stable top clear should preserve deleted state");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.record_deleted &&
                   top_property.direct_field && top_property.value.empty(),
               "#1651: deleted report/label layout object stable top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1651: deleted report/label layout object stable top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1651: label deleted layout object stable top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1651: deleted report/label layout object stable top clear should preserve deleted object counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1651: deleted report/label layout object stable top clear should preserve selected deleted-object availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1651: deleted report/label layout object stable top clear should preserve object selection kind");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1651: deleted report/label layout object stable top clear should not fabricate containing-section availability");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSection\": null",
                        "#1651: deleted report/label layout object stable top clear should serialize null containing-section metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"deletedObjects\": [",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1651: deleted report/label layout object stable top clear should refresh deleted-object geometry metadata");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": true",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"label\"",
                "\"top\": 0",
                "\"height\": 300",
                "\"bottom\": 300"
            },
            "#1651: deleted report/label layout object stable top clear should refresh selected deleted-object geometry metadata");
    };

    run_deleted_top_clear(temp_root / "deleted_top_clear_stable.frx",
                          "deleted_top_clear_stable.frx",
                          "report");
    run_deleted_top_clear(temp_root / "deleted_top_clear_stable.lbx",
                          "deleted_top_clear_stable.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1613: restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--record", "6",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1613: deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1613: deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--record", "6",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1613: edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1613: edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "1400" &&
                   top_property.ok && top_property.exists && top_property.value == "3100" &&
                   height_property.ok && height_property.exists && height_property.value == "900",
               "#1613: edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1613: edited deleted report/label layout object restore should return refreshed report-layout JSON");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1613: edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1613: edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1613: edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1613: edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1613: edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.frx",
                                 "deleted_restore_edited_geometry.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry.lbx",
                                 "deleted_restore_edited_geometry.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_restores_edited_deleted_report_layout_object_geometry_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_deleted_report_layout_restore_edited_geometry_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_geometry_restore = [&](const fs::path& asset_path,
                                                  const std::string& title,
                                                  const std::string& label) {
        write_synthetic_report_table_for_stable_deleted_layout_json(asset_path);
        expect(dbf_record_deleted(asset_path, 6U),
               "#1652: stable restore edited deleted layout object fixture should start deleted");

        const auto set_deleted_geometry = [&](const std::string& property_name,
                                              const std::string& property_value) {
            const auto update_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-label-guid",
                    "--property-name", property_name,
                    "--property-value", property_value,
                    "--json"
                },
                temp_root);

            if (update_process.exit_code != 0) {
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stdout:\n"
                          << update_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " stable deleted layout " << property_name
                          << " pre-restore update stderr:\n"
                          << update_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_process.exit_code == 0,
                   "#1652: stable deleted report/label layout object geometry pre-restore update should exit successfully");
            expect(dbf_record_deleted(asset_path, 6U),
                   "#1652: stable deleted report/label layout object geometry pre-restore update should preserve deleted state");
        };

        set_deleted_geometry("HPOS", "1400");
        set_deleted_geometry("VPOS", "3100");
        set_deleted_geometry("HEIGHT", "900");

        const auto restore_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--unique-id", "deleted-label-guid",
                "--restore-object",
                "--json"
            },
            temp_root);

        if (restore_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable edited deleted layout restore stdout:\n"
                      << restore_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable edited deleted layout restore stderr:\n"
                      << restore_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(restore_process.exit_code == 0,
               "#1652: stable edited deleted report/label layout object restore should exit successfully");
        expect(!dbf_record_deleted(asset_path, 6U),
               "#1652: stable edited deleted report/label layout object restore should clear deleted state");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HPOS"
        });
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "VPOS"
        });
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 6U,
            .object_name = {},
            .unique_id = "deleted-label-guid",
            .property_name = "HEIGHT"
        });
        expect(left_property.ok && left_property.exists && !left_property.record_deleted &&
                   left_property.value == "1400" &&
                   top_property.ok && top_property.exists && !top_property.record_deleted &&
                   top_property.value == "3100" &&
                   height_property.ok && height_property.exists && !height_property.record_deleted &&
                   height_property.value == "900",
               "#1652: stable edited deleted report/label layout object restore should preserve edited geometry fields");
        expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1652: stable edited deleted report/label layout object restore should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                            "#1652: label stable edited deleted layout object restore should retain label identity");
        }
        expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1891: stable edited deleted report/label layout object restore should preserve live preview availability");
        expect_contains(restore_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview left bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview top bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview right bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview bottom bounds");
        expect_contains(restore_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview widths");
        expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1891: stable edited deleted report/label layout object restore should refresh live preview heights");
        expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                        "#1891: stable edited deleted report/label layout object restore should clear deleted preview availability");
        expect_contains(restore_process.stdout_text, "\"deletedObjectCount\": 0",
                        "#1652: stable edited deleted report/label layout object restore should remove restored objects from deleted-object counts");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should preserve selected-object availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"object\"",
                        "#1652: stable edited deleted report/label layout object restore should preserve object selection kind");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1652: stable edited deleted report/label layout object restore should rehydrate containing-section availability");
        expect_contains(restore_process.stdout_text, "\"selectedReportObjectSection\": {",
                        "#1652: stable edited deleted report/label layout object restore should serialize containing-section metadata");
        expect_contains_in_order(
            restore_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 6",
                "\"deleted\": false",
                "\"containingSectionId\": \"detail_2\"",
                "\"containingSectionRecordIndex\": 2",
                "\"sectionRelativeTop\": 1100",
                "\"sectionRelativeBottom\": 2000",
                "\"sectionObjectIndex\": 1",
                "\"sectionObjectCount\": 2",
                "\"objectKind\": \"label\"",
                "\"left\": 1400",
                "\"top\": 3100",
                "\"width\": 1200",
                "\"height\": 900",
                "\"right\": 2600",
                "\"bottom\": 4000"
            },
            "#1652: stable edited deleted report/label layout object restore should refresh selected live geometry and section metadata");
    };

    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.frx",
                                 "deleted_restore_edited_geometry_stable.frx",
                                 "report");
    run_deleted_geometry_restore(temp_root / "deleted_restore_edited_geometry_stable.lbx",
                                 "deleted_restore_edited_geometry_stable.lbx",
                                 "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
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

void test_studio_host_json_clamps_negative_report_layout_dimensions(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_negative_report_layout_dimension_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_negative_dimension_layout = [&](const fs::path& asset_path,
                                                   const std::string& title,
                                                   const std::string& label) {
        write_synthetic_report_table_for_negative_dimension_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " negative layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " negative layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1715: negative-dimension report/label layout JSON should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1715: negative-dimension layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1715: negative-dimension label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1715: negative-dimension live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1715: negative-dimension live layouts should keep section-origin left bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1715: negative-dimension live layouts should keep section-origin top bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 300",
                        "#1715: negative-dimension live layouts should clamp object right bounds to left plus zero width");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1715: negative-dimension live layouts should not invert bottom bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 300",
                        "#1715: negative-dimension live layouts should compute non-negative preview widths");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1715: negative-dimension live layouts should compute zero preview height");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1715: negative-dimension deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 700",
                        "#1715: negative-dimension deleted layouts should preserve deleted left bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 50",
                        "#1715: negative-dimension deleted layouts should preserve deleted top bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 700",
                        "#1715: negative-dimension deleted layouts should clamp deleted right bounds to left plus zero width");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 50",
                        "#1715: negative-dimension deleted layouts should clamp deleted bottom bounds to top plus zero height");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1715: negative-dimension deleted layouts should compute zero deleted preview width");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1715: negative-dimension deleted layouts should compute zero deleted preview height");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1715: negative-dimension layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1715: negative-dimension layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1715: negative section heights should be clamped to zero in summaries");
        expect_negative_dimension_preview_bounds(
            summary_process.stdout_text,
            "#2353: negative-dimension summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1715: negative-dimension live object selection should keep inspection non-failing");
        expect_negative_dimension_preview_bounds(
            live_process.stdout_text,
            "#2353: selected negative-dimension live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 300",
                "\"top\": 0",
                "\"width\": 0",
                "\"right\": 300",
                "\"height\": 0",
                "\"bottom\": 0"
            },
            "#1715: negative live object dimensions should clamp selected geometry to zero width and height");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1715: negative-dimension deleted object selection should keep inspection non-failing");
        expect_negative_dimension_preview_bounds(
            deleted_process.stdout_text,
            "#2353: selected negative-dimension deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 700",
                "\"top\": 50",
                "\"width\": 0",
                "\"right\": 700",
                "\"height\": 0",
                "\"bottom\": 50"
            },
            "#1715: negative deleted object dimensions should clamp selected geometry to zero width and height");
    };

    run_negative_dimension_layout(temp_root / "negative_dimensions.frx",
                                  "negative_dimensions.frx",
                                  "report");
    run_negative_dimension_layout(temp_root / "negative_dimensions.lbx",
                                  "negative_dimensions.lbx",
                                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_uses_integer_portions_for_fractional_report_layout_geometry(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_fractional_report_layout_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_fractional_layout = [&](const fs::path& asset_path,
                                           const std::string& title,
                                           const std::string& label) {
        write_synthetic_report_table_for_fractional_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " fractional layout summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " fractional layout summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1719: fractional report/label layout numerics should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1719: fractional layout numerics should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1719: fractional numeric label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1719: fractional live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1719: fractional live layout left bounds should include the section origin");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 10",
                        "#1719: fractional section top should use the integer portion");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 425",
                        "#1719: fractional live layout right bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 1010",
                        "#1719: fractional section bottom should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 425",
                        "#1719: fractional live layout width should use integer portions");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 1000",
                        "#1719: fractional live layout height should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1719: fractional deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 425",
                        "#1719: fractional deleted layout left bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 700",
                        "#1719: fractional deleted layout top bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 575",
                        "#1719: fractional deleted layout right bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 740",
                        "#1719: fractional deleted layout bottom bounds should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 150",
                        "#1719: fractional deleted layout width should use integer portions");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 40",
                        "#1719: fractional deleted layout height should use integer portions");
        expect_fractional_geometry_preview_bounds(
            summary_process.stdout_text,
            "#2347: fractional geometry summary JSON");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1719: fractional layout numerics should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1719: fractional layout numerics should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 1000",
                        "#1719: fractional section heights should use integer portions in summaries");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1719: fractional live object selection should keep inspection non-failing");
        expect_fractional_geometry_preview_bounds(
            live_process.stdout_text,
            "#2347: selected fractional live object JSON");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 125",
                "\"top\": 200",
                "\"width\": 300",
                "\"right\": 425",
                "\"height\": 80",
                "\"bottom\": 280"
            },
            "#1719: fractional live object geometry should use integer portions");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1719: fractional deleted object selection should keep inspection non-failing");
        expect_fractional_geometry_preview_bounds(
            deleted_process.stdout_text,
            "#2347: selected fractional deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 425",
                "\"top\": 700",
                "\"width\": 150",
                "\"right\": 575",
                "\"height\": 40",
                "\"bottom\": 740"
            },
            "#1719: fractional deleted object geometry should use integer portions");
    };

    run_fractional_layout(temp_root / "fractional_geometry.frx",
                          "fractional_geometry.frx",
                          "report");
    run_fractional_layout(temp_root / "fractional_geometry.lbx",
                          "fractional_geometry.lbx",
                          "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_defaults_missing_report_layout_geometry_fields(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_missing_report_layout_geometry_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);
    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_missing_geometry_layout = [&](const fs::path& asset_path,
                                                 const std::string& title,
                                                 const std::string& label) {
        write_synthetic_report_table_for_missing_geometry_layout_json(asset_path);

        const auto summary_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--json"},
            temp_root);

        if (summary_process.exit_code != 0) {
            std::cerr << "studio host " << label << " missing geometry summary stdout:\n"
                      << summary_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " missing geometry summary stderr:\n"
                      << summary_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(summary_process.exit_code == 0,
               "#1720: missing report/label layout geometry fields should keep inspection non-failing");
        expect_contains(summary_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1720: missing geometry layouts should preserve document titles");
        if (asset_path.extension() == ".lbx") {
            expect_contains(summary_process.stdout_text, "\"isLabel\": true",
                            "#1720: missing geometry label layouts should retain label identity");
        }
        expect_contains(summary_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1720: missing geometry live layouts should expose preview bounds");
        expect_contains(summary_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1720: missing geometry live layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1720: missing geometry live layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsRight\": 0",
                        "#1720: missing geometry live layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsBottom\": 0",
                        "#1720: missing geometry live layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"previewBoundsWidth\": 0",
                        "#1720: missing geometry live layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"previewBoundsHeight\": 0",
                        "#1720: missing geometry live layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1720: missing geometry deleted layouts should expose deleted preview bounds");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                        "#1720: missing geometry deleted layout left bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                        "#1720: missing geometry deleted layout top bounds should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsRight\": 0",
                        "#1720: missing geometry deleted layout right bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsBottom\": 0",
                        "#1720: missing geometry deleted layout bottom bounds should stay non-inverted");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsWidth\": 0",
                        "#1720: missing geometry deleted layout width should default to zero");
        expect_contains(summary_process.stdout_text, "\"deletedPreviewBoundsHeight\": 0",
                        "#1720: missing geometry deleted layout height should default to zero");
        expect_contains(summary_process.stdout_text, "\"liveObjectCount\": 1",
                        "#1720: missing geometry layouts should preserve live object counts");
        expect_contains(summary_process.stdout_text, "\"deletedObjectCount\": 1",
                        "#1720: missing geometry layouts should preserve deleted object counts");
        expect_contains(summary_process.stdout_text, "\"sectionCount\": 1",
                        "#1720: missing geometry layouts should preserve section rows");
        expect_contains(summary_process.stdout_text, "\"sectionHeightTotal\": 0",
                        "#1720: missing section geometry should default to zero in summaries");
        expect_zero_available_report_layout_preview_bounds(
            summary_process.stdout_text,
            "#2360: missing-geometry summary JSON");

        const auto live_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "2", "--json"},
            temp_root);

        expect(live_process.exit_code == 0,
               "#1720: missing geometry live object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            live_process.stdout_text,
            "#2313: selected missing-geometry live object JSON");
        expect_contains(live_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1720: missing geometry live object should still resolve the zero-height section");
        expect_contains_in_order(
            live_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 2",
                "\"deleted\": false",
                "\"left\": 0",
                "\"leftFieldIndex\": null",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"width\": 0",
                "\"widthFieldIndex\": null",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1720: missing live object geometry should default to zero with null field provenance");

        const auto deleted_process = run_process_capture(
            studio_host_path,
            {"--path", asset_path.string(), "--record", "3", "--json"},
            temp_root);

        expect(deleted_process.exit_code == 0,
               "#1720: missing geometry deleted object selection should keep inspection non-failing");
        expect_zero_available_report_layout_preview_bounds(
            deleted_process.stdout_text,
            "#2313: selected missing-geometry deleted object JSON");
        expect_contains_in_order(
            deleted_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"deleted\": true",
                "\"left\": 0",
                "\"leftFieldIndex\": null",
                "\"top\": 0",
                "\"topFieldIndex\": null",
                "\"width\": 0",
                "\"widthFieldIndex\": null",
                "\"right\": 0",
                "\"height\": 0",
                "\"heightFieldIndex\": null",
                "\"bottom\": 0"
            },
            "#1720: missing deleted object geometry should default to zero with null field provenance");
    };

    run_missing_geometry_layout(temp_root / "missing_geometry.frx",
                                "missing_geometry.frx",
                                "report");
    run_missing_geometry_layout(temp_root / "missing_geometry.lbx",
                                "missing_geometry.lbx",
                                "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

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
                "\"id\": \"page_header_0\"",
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
                "\"id\": \"summary_1\"",
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
                "\"id\": \"page_header_0\"",
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
                "\"id\": \"summary_1\"",
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

}  // namespace cf_test_studio_host_json
