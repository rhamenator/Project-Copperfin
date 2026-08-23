// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
#if !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_SKIP_HOST_SMOKE) && \
    !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY)
void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_update = [&](const fs::path& asset_path,
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
                "--property-value", "-1000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1536: report/label layout object top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "-1000",
               "#1536: report/label layout object top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1536: report/label layout object top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1906: label layout object top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1536: report/label layout object top update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": -1000",
                        "#1536: report/label layout object top update should refresh preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1536: report/label layout object top update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9100",
                        "#1536: report/label layout object top update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1906: report/label layout object top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1906: report/label layout object top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1906: report/label layout object top update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1906: report/label layout object top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1906: report/label layout object top update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1906: report/label layout object top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1906: report/label layout object top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1536: report/label layout object top update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1536: report/label layout object top update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1536: report/label layout object top update should clear selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"top\": -1000",
                "\"height\": 450",
                "\"bottom\": -550"
            },
            "#1536: report/label layout object top update should refresh selected object top bounds and unplaced metadata");
    };

    run_top_update(temp_root / "top_bounds.frx", "top_bounds.frx", "report");
    run_top_update(temp_root / "top_bounds.lbx", "top_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_clear = [&](const fs::path& asset_path,
                                   const std::string& title,
                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1563: report/label layout object top clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1563: report/label layout object top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1908: label layout object top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1563: report/label layout object top clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1563: report/label layout object top clear should preserve document preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1563: report/label layout object top clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1563: report/label layout object top clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1908: report/label layout object top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1908: report/label layout object top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1908: report/label layout object top clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1908: report/label layout object top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1908: report/label layout object top clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1908: report/label layout object top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1908: report/label layout object top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1563: report/label layout object top clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1563: report/label layout object top clear should preserve unplaced counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1563: report/label layout object top clear should expose selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"page_header_1\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"field\"",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1563: report/label layout object top clear should refresh selected object top bounds and section metadata");
    };

    run_top_clear(temp_root / "top_clear_bounds.frx", "top_clear_bounds.frx", "report");
    run_top_clear(temp_root / "top_clear_bounds.lbx", "top_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_update = [&](const fs::path& asset_path,
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
                "--property-value", "-1000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout top update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout top update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1642: report/label layout object stable top update should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.value == "-1000",
               "#1642: report/label layout object stable top update should persist the VPOS field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1642: report/label layout object stable top update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1642: label layout object stable top update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1642: report/label layout object stable top update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsTop\": -1000",
                        "#1642: report/label layout object stable top update should refresh preview top bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1642: report/label layout object stable top update should preserve preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9100",
                        "#1642: report/label layout object stable top update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1907: report/label layout object stable top update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1907: report/label layout object stable top update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1907: report/label layout object stable top update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1907: report/label layout object stable top update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1907: report/label layout object stable top update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1907: report/label layout object stable top update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1907: report/label layout object stable top update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 1",
                        "#1642: report/label layout object stable top update should decrement placed counts");
        expect_contains(update_process.stdout_text, "\"unplacedObjectCount\": 2",
                        "#1642: report/label layout object stable top update should increment unplaced counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": false",
                        "#1642: report/label layout object stable top update should clear selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 0",
                "\"objectKind\": \"field\"",
                "\"top\": -1000",
                "\"height\": 450",
                "\"bottom\": -550"
            },
            "#1642: report/label layout object stable top update should refresh selected object top bounds and unplaced metadata");
    };

    run_top_update(temp_root / "top_bounds_stable.frx",
                   "top_bounds_stable.frx",
                   "report");
    run_top_update(temp_root / "top_bounds_stable.lbx",
                   "top_bounds_stable.lbx",
                   "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_top_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_top_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_top_clear = [&](const fs::path& asset_path,
                                   const std::string& title,
                                   const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "VPOS",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout top clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout top clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1643: report/label layout object stable top clear should exit successfully");
        const auto top_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "VPOS"
        });
        expect(top_property.ok && top_property.exists && top_property.direct_field &&
                   top_property.value.empty(),
               "#1643: report/label layout object stable top clear should blank the VPOS field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1643: report/label layout object stable top clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1643: label layout object stable top clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1643: report/label layout object stable top clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsTop\": 0",
                        "#1643: report/label layout object stable top clear should preserve document preview top bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1643: report/label layout object stable top clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1643: report/label layout object stable top clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1909: report/label layout object stable top clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1909: report/label layout object stable top clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1909: report/label layout object stable top clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1909: report/label layout object stable top clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1909: report/label layout object stable top clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1909: report/label layout object stable top clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1909: report/label layout object stable top clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1643: report/label layout object stable top clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"unplacedObjectCount\": 1",
                        "#1643: report/label layout object stable top clear should preserve unplaced counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1643: report/label layout object stable top clear should expose selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"page_header_1\"",
                "\"sectionRelativeTop\": 0",
                "\"sectionRelativeBottom\": 450",
                "\"objectKind\": \"field\"",
                "\"top\": 0",
                "\"height\": 450",
                "\"bottom\": 450"
            },
            "#1643: report/label layout object stable top clear should refresh selected object top bounds and section metadata");
    };

    run_top_clear(temp_root / "top_clear_bounds_stable.frx",
                  "top_clear_bounds_stable.frx",
                  "report");
    run_top_clear(temp_root / "top_clear_bounds_stable.lbx",
                  "top_clear_bounds_stable.lbx",
                  "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
}  // namespace cf_test_studio_host_json
