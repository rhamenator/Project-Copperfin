// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

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

}  // namespace cf_test_studio_host_json

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_geometry_defaults <copperfin_studio_host>\n";
        return 2;
    }

    cf_test_studio_host_json::test_studio_host_json_clamps_negative_report_layout_dimensions(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_uses_integer_portions_for_fractional_report_layout_geometry(argv[1]);
    cf_test_studio_host_json::test_studio_host_json_defaults_missing_report_layout_geometry_fields(argv[1]);

    return cf_test_studio_host_json::failures == 0 ? 0 : 1;
}
