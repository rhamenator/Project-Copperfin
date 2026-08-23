// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

#if !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_SKIP_HOST_SMOKE) && \
    !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY)

void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "4",
                "--property-name", "HPOS",
                "--property-value", "-200",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1534: report/label layout object left update should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-200",
               "#1534: report/label layout object left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1534: report/label layout object left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1898: label layout object left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1534: report/label layout object left update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": -200",
                        "#1534: report/label layout object left update should refresh preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1534: report/label layout object left update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 5400",
                        "#1534: report/label layout object left update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1898: report/label layout object left update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1898: report/label layout object left update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1898: report/label layout object left update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1898: report/label layout object left update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1898: report/label layout object left update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1898: report/label layout object left update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1898: report/label layout object left update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1534: report/label layout object left update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1534: report/label layout object left update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": -200",
                "\"width\": 1800",
                "\"right\": 1600",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1534: report/label layout object left update should refresh selected object bounds and preserve section membership");
    };

    run_left_update(temp_root / "left_bounds.frx", "left_bounds.frx", "report");
    run_left_update(temp_root / "left_bounds.lbx", "left_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_update = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "label-guid",
                "--property-name", "HPOS",
                "--property-value", "-200",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout left update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout left update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1638: report/label layout object stable left update should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = "label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.value == "-200",
               "#1638: report/label layout object stable left update should persist the HPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1638: report/label layout object stable left update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1638: label layout object stable left update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1638: report/label layout object stable left update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsLeft\": -200",
                        "#1638: report/label layout object stable left update should refresh preview left bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1638: report/label layout object stable left update should preserve preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 5400",
                        "#1638: report/label layout object stable left update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1899: report/label layout object stable left update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1899: report/label layout object stable left update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1899: report/label layout object stable left update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1899: report/label layout object stable left update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1899: report/label layout object stable left update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1899: report/label layout object stable left update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1899: report/label layout object stable left update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1638: report/label layout object stable left update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1638: report/label layout object stable left update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": -200",
                "\"width\": 1800",
                "\"right\": 1600",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1638: report/label layout object stable left update should refresh selected object bounds and preserve section membership");
    };

    run_left_update(temp_root / "left_bounds_stable.frx",
                    "left_bounds_stable.frx",
                    "report");
    run_left_update(temp_root / "left_bounds_stable.lbx",
                    "left_bounds_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "4",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1561: report/label layout object left clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1561: report/label layout object left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1900: label layout object left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1561: report/label layout object left clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1561: report/label layout object left clear should refresh preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1561: report/label layout object left clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1561: report/label layout object left clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1900: report/label layout object left clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1900: report/label layout object left clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1900: report/label layout object left clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1900: report/label layout object left clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1900: report/label layout object left clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1900: report/label layout object left clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1900: report/label layout object left clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1561: report/label layout object left clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1561: report/label layout object left clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": 0",
                "\"width\": 1800",
                "\"right\": 1800",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1561: report/label layout object left clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"left\": 900",
                            "#1561: report/label layout object left clear should not leak stale selected-object left positions");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2700",
                            "#1561: report/label layout object left clear should not leak stale selected-object right bounds");
    };

    run_left_clear(temp_root / "left_clear_bounds.frx", "left_clear_bounds.frx", "report");
    run_left_clear(temp_root / "left_clear_bounds.lbx", "left_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_left_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_left_clear = [&](const fs::path& asset_path,
                                    const std::string& title,
                                    const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "label-guid",
                "--property-name", "HPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout left clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout left clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1639: report/label layout object stable left clear should exit successfully");
        const auto left_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 4U,
            .object_name = {},
            .unique_id = "label-guid",
            .property_name = "HPOS"
        });
        expect(left_property.ok && left_property.exists && left_property.direct_field &&
                   left_property.value.empty(),
               "#1639: report/label layout object stable left clear should blank the HPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1639: report/label layout object stable left clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1639: label layout object stable left clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1639: report/label layout object stable left clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsLeft\": 0",
                        "#1639: report/label layout object stable left clear should refresh preview left bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 5200",
                        "#1639: report/label layout object stable left clear should preserve preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 5200",
                        "#1639: report/label layout object stable left clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1901: report/label layout object stable left clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1901: report/label layout object stable left clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1901: report/label layout object stable left clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1901: report/label layout object stable left clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1901: report/label layout object stable left clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1901: report/label layout object stable left clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1901: report/label layout object stable left clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1639: report/label layout object stable left clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1639: report/label layout object stable left clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 4",
                "\"containingSectionId\": \"page_header_1\"",
                "\"left\": 0",
                "\"width\": 1800",
                "\"right\": 1800",
                "\"sectionRelativeTop\": 100",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"label\""
            },
            "#1639: report/label layout object stable left clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"left\": 900",
                            "#1639: report/label layout object stable left clear should not leak stale selected-object left positions");
        expect_not_contains(clear_process.stdout_text, "\"right\": 2700",
                            "#1639: report/label layout object stable left clear should not leak stale selected-object right bounds");
    };

    run_left_clear(temp_root / "left_clear_bounds_stable.frx",
                   "left_clear_bounds_stable.frx",
                   "report");
    run_left_clear(temp_root / "left_clear_bounds_stable.lbx",
                   "left_clear_bounds_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

}  // namespace cf_test_studio_host_json
