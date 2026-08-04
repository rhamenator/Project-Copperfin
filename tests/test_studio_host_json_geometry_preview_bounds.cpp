// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void expect_full_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 8100",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 8100",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1000",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 2200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 2900",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 1200",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}

void expect_empty_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": false",
                    prefix + " should not fabricate live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_fractional_geometry_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 10",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 425",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 1010",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 425",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 1000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 425",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 700",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 575",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 150",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 40",
                    prefix + " should preserve deleted preview heights");
}

void expect_negative_dimension_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 300",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 300",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 700",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 50",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 700",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 50",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve deleted preview heights");
}

void expect_zero_available_report_layout_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve zero live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 0",
                    prefix + " should preserve zero live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve zero live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 0",
                    prefix + " should preserve zero live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve zero live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 0",
                    prefix + " should preserve zero live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

#if !defined(COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_SKIP_UNRESOLVED_AND_MISSING_HELPERS) && \
    (!defined(COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS) || \
     defined(COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_INCLUDE_UNRESOLVED_AND_MISSING_HELPERS))
void expect_unresolved_memo_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2000",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 7000",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 5200",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 5000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_unresolved_section_memo_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 100",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 600",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 500",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 900",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 1200",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 300",
                    prefix + " should preserve deleted preview heights");
}

void expect_unresolved_deleted_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2000",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 7000",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 5000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 1200",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 2600",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 5200",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 3050",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 4000",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 450",
                    prefix + " should preserve deleted preview heights");
}

void expect_unresolved_unplaced_object_memo_preview_bounds(
    const std::string& text,
    const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 1200",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 2600",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 5200",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 3050",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 4000",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 450",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": false",
                    prefix + " should not fabricate deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve zero deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 0",
                    prefix + " should preserve zero deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve zero deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 0",
                    prefix + " should preserve zero deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve zero deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 0",
                    prefix + " should preserve zero deleted preview heights");
}

void expect_missing_section_objcode_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 150",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 0",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 600",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 0",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 450",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 0",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 900",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 0",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 1150",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 0",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 250",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_objcode_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 120",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 300",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 820",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 390",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 700",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 90",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 260",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 620",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 760",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 500",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 120",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_expr_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 200",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 820",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 1200",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 820",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 1000",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 260",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 620",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 760",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 740",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 500",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 120",
                    prefix + " should preserve deleted preview heights");
}

void expect_missing_object_title_preview_bounds(const std::string& text, const std::string& prefix) {
    expect_contains(text, "\"previewBoundsAvailable\": true",
                    prefix + " should preserve live preview availability");
    expect_contains(text, "\"previewBoundsLeft\": 0",
                    prefix + " should preserve live preview left bounds");
    expect_contains(text, "\"previewBoundsTop\": 100",
                    prefix + " should preserve live preview top bounds");
    expect_contains(text, "\"previewBoundsRight\": 560",
                    prefix + " should preserve live preview right bounds");
    expect_contains(text, "\"previewBoundsBottom\": 900",
                    prefix + " should preserve live preview bottom bounds");
    expect_contains(text, "\"previewBoundsWidth\": 560",
                    prefix + " should preserve live preview widths");
    expect_contains(text, "\"previewBoundsHeight\": 800",
                    prefix + " should preserve live preview heights");
    expect_contains(text, "\"deletedPreviewBoundsAvailable\": true",
                    prefix + " should preserve deleted preview availability");
    expect_contains(text, "\"deletedPreviewBoundsLeft\": 360",
                    prefix + " should preserve deleted preview left bounds");
    expect_contains(text, "\"deletedPreviewBoundsTop\": 500",
                    prefix + " should preserve deleted preview top bounds");
    expect_contains(text, "\"deletedPreviewBoundsRight\": 740",
                    prefix + " should preserve deleted preview right bounds");
    expect_contains(text, "\"deletedPreviewBoundsBottom\": 610",
                    prefix + " should preserve deleted preview bottom bounds");
    expect_contains(text, "\"deletedPreviewBoundsWidth\": 380",
                    prefix + " should preserve deleted preview widths");
    expect_contains(text, "\"deletedPreviewBoundsHeight\": 110",
                    prefix + " should preserve deleted preview heights");
}
#endif

#if !defined(COPPERFIN_GEOMETRY_PREVIEW_BOUNDS_ONLY_HELPERS)
#if !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_SKIP_HOST_SMOKE)
void test_studio_host_json_refreshes_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_preview_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_footer_section_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_section_kind_json(asset_path);

            const auto update_footer_height_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "900",
                    "--json"
                },
                temp_root);

            if (update_footer_height_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview height stdout:\n"
                          << update_footer_height_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview height stderr:\n"
                          << update_footer_height_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_height_process.exit_code == 0,
                   "#1819: detail-footer section preview height update by stable selection should exit successfully");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "900",
                   "#1819: detail-footer section preview height update should persist the HEIGHT field");
            expect_contains(update_footer_height_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1819: detail-footer section preview height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_height_process.stdout_text, "\"isLabel\": true",
                                "#1819: detail-footer label section preview height update should retain label identity");
            }
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1819: detail-footer section preview height update should preserve preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsBottom\": 1200",
                            "#1819: detail-footer section preview height update should refresh preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsHeight\": 1200",
                            "#1819: detail-footer section preview height update should refresh preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve deleted preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1819: detail-footer section preview height update should preserve deleted preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1819: detail-footer section preview height update should preserve deleted preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"sectionHeightTotal\": 1200",
                            "#1819: detail-footer section preview height update should refresh live section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1819: detail-footer section preview height update should preserve selected section availability");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1819: detail-footer section preview height update should preserve selection kind");
            expect_contains_in_order(
                update_footer_height_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 1",
                    "\"top\": 300",
                    "\"height\": 900",
                    "\"bottom\": 1200"
                },
                "#1819: detail-footer section preview height update should refresh selected-section geometry");

            const auto update_header_top_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "-150",
                    "--json"
                },
                temp_root);

            if (update_header_top_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview top stdout:\n"
                          << update_header_top_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview top stderr:\n"
                          << update_header_top_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_top_process.exit_code == 0,
                   "#1819: detail-header section preview top update by stable selection should exit successfully");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "-150",
                   "#1819: detail-header section preview top update should persist the VPOS field");
            expect_contains(update_header_top_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1819: detail-header section preview top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_top_process.stdout_text, "\"isLabel\": true",
                                "#1819: detail-header label section preview top update should retain label identity");
            }
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1819: detail-header section preview top update should preserve preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsTop\": -150",
                            "#1819: detail-header section preview top update should refresh preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsBottom\": 1200",
                            "#1819: detail-header section preview top update should preserve expanded preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsHeight\": 1350",
                            "#1819: detail-header section preview top update should refresh preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1819: detail-header section preview top update should preserve deleted preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsTop\": 550",
                            "#1819: detail-header section preview top update should preserve deleted preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsBottom\": 750",
                            "#1819: detail-header section preview top update should preserve deleted preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"sectionHeightTotal\": 1200",
                            "#1819: detail-header section preview top update should preserve live section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1819: detail-header section preview top update should preserve selected section availability");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1819: detail-header section preview top update should preserve selection kind");
            expect_contains_in_order(
                update_header_top_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"top\": -150",
                    "\"height\": 300",
                    "\"bottom\": 150"
                },
                "#1819: detail-header section preview top update should refresh selected-section geometry");
        };

    run_detail_header_footer_section_preview_bounds(
        temp_root / "detail_header_footer_section_preview_bounds_stable.frx",
        "detail_header_footer_section_preview_bounds_stable.frx",
        "report");
    run_detail_header_footer_section_preview_bounds(
        temp_root / "detail_header_footer_section_preview_bounds_stable.lbx",
        "detail_header_footer_section_preview_bounds_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

#endif

#if !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_SKIP_HOST_SMOKE) && \
    !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY)

void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_update = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "WIDTH",
                "--property-value", "6000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1533: report/label layout object width update should exit successfully");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "6000",
               "#1533: report/label layout object width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1533: report/label layout object width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1894: label layout object width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1533: report/label layout object width update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 7200",
                        "#1533: report/label layout object width update should refresh preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 7200",
                        "#1533: report/label layout object width update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1894: report/label layout object width update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1894: report/label layout object width update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1894: report/label layout object width update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1894: report/label layout object width update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1894: report/label layout object width update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1894: report/label layout object width update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1894: report/label layout object width update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1533: report/label layout object width update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1533: report/label layout object width update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 6000",
                "\"right\": 7200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1533: report/label layout object width update should refresh selected object bounds and preserve section membership");
    };

    run_width_update(temp_root / "width_bounds.frx", "width_bounds.frx", "report");
    run_width_update(temp_root / "width_bounds.lbx", "width_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_update = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "WIDTH",
                "--property-value", "6000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout width update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout width update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1636: report/label layout object stable width update should exit successfully");
        const auto width_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "WIDTH"
        });
        expect(width_property.ok && width_property.exists && width_property.value == "6000",
               "#1636: report/label layout object stable width update should persist the WIDTH field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1636: report/label layout object stable width update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1636: label layout object stable width update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1636: report/label layout object stable width update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsRight\": 7200",
                        "#1636: report/label layout object stable width update should refresh preview right bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsWidth\": 7200",
                        "#1636: report/label layout object stable width update should refresh preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1895: report/label layout object stable width update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1895: report/label layout object stable width update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1895: report/label layout object stable width update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1895: report/label layout object stable width update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1895: report/label layout object stable width update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1895: report/label layout object stable width update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1895: report/label layout object stable width update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1636: report/label layout object stable width update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1636: report/label layout object stable width update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 6000",
                "\"right\": 7200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1636: report/label layout object stable width update should refresh selected object bounds and preserve section membership");
    };

    run_width_update(temp_root / "width_bounds_stable.frx",
                     "width_bounds_stable.frx",
                     "report");
    run_width_update(temp_root / "width_bounds_stable.lbx",
                     "width_bounds_stable.lbx",
                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_clear = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1560: report/label layout object width clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1560: report/label layout object width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1896: label layout object width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1560: report/label layout object width clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1560: report/label layout object width clear should refresh preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1560: report/label layout object width clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1896: report/label layout object width clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1896: report/label layout object width clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1896: report/label layout object width clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1896: report/label layout object width clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1896: report/label layout object width clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1896: report/label layout object width clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1896: report/label layout object width clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1560: report/label layout object width clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1560: report/label layout object width clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 0",
                "\"right\": 1200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1560: report/label layout object width clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"width\": 4000",
                            "#1560: report/label layout object width clear should not leak stale selected-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 5200",
                            "#1560: report/label layout object width clear should not leak stale selected-object right bounds");
    };

    run_width_clear(temp_root / "width_clear_bounds.frx", "width_clear_bounds.frx", "report");
    run_width_clear(temp_root / "width_clear_bounds.lbx", "width_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_width_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_width_clear = [&](const fs::path& asset_path,
                                     const std::string& title,
                                     const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "WIDTH",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout width clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout width clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1637: report/label layout object stable width clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1637: report/label layout object stable width clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1637: label layout object stable width clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1637: report/label layout object stable width clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsRight\": 2700",
                        "#1637: report/label layout object stable width clear should refresh preview right bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsWidth\": 2700",
                        "#1637: report/label layout object stable width clear should refresh preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1897: report/label layout object stable width clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1897: report/label layout object stable width clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1897: report/label layout object stable width clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1897: report/label layout object stable width clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1897: report/label layout object stable width clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1897: report/label layout object stable width clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1897: report/label layout object stable width clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1637: report/label layout object stable width clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1637: report/label layout object stable width clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"width\": 0",
                "\"right\": 1200",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 1050",
                "\"sectionObjectCount\": 1",
                "\"objectKind\": \"field\""
            },
            "#1637: report/label layout object stable width clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"width\": 4000",
                            "#1637: report/label layout object stable width clear should not leak stale selected-object widths");
        expect_not_contains(clear_process.stdout_text, "\"right\": 5200",
                            "#1637: report/label layout object stable width clear should not leak stale selected-object right bounds");
    };

    run_width_clear(temp_root / "width_clear_bounds_stable.frx",
                    "width_clear_bounds_stable.frx",
                    "report");
    run_width_clear(temp_root / "width_clear_bounds_stable.lbx",
                    "width_clear_bounds_stable.lbx",
                    "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

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

#if !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_SKIP_HOST_SMOKE) && \
    !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_ONLY)

void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_update = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--record", "3",
                "--property-name", "HEIGHT",
                "--property-value", "7000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1535: report/label layout object height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = {},
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "7000",
               "#1535: report/label layout object height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1535: report/label layout object height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1902: label layout object height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1535: report/label layout object height update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 9600",
                        "#1535: report/label layout object height update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9600",
                        "#1535: report/label layout object height update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1902: report/label layout object height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1902: report/label layout object height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1902: report/label layout object height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1902: report/label layout object height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1902: report/label layout object height update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1902: report/label layout object height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1902: report/label layout object height update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1535: report/label layout object height update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1535: report/label layout object height update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 7000",
                "\"bottom\": 9600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 7600",
                "\"objectKind\": \"field\""
            },
            "#1535: report/label layout object height update should refresh selected object bounds and preserve section membership");
    };

    run_height_update(temp_root / "height_bounds.frx", "height_bounds.frx", "report");
    run_height_update(temp_root / "height_bounds.lbx", "height_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_record_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_clear_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_clear = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--record", "3",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1562: report/label layout object height clear should exit successfully");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1562: report/label layout object height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1904: label layout object height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1562: report/label layout object height clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1562: report/label layout object height clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1562: report/label layout object height clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1904: report/label layout object height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1904: report/label layout object height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1904: report/label layout object height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1904: report/label layout object height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1904: report/label layout object height clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1904: report/label layout object height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1904: report/label layout object height clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1562: report/label layout object height clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1562: report/label layout object height clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 0",
                "\"bottom\": 2600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
                "\"objectKind\": \"field\""
            },
            "#1562: report/label layout object height clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"height\": 450",
                            "#1562: report/label layout object height clear should not leak stale selected-object heights");
        expect_not_contains(clear_process.stdout_text, "\"bottom\": 3050",
                            "#1562: report/label layout object height clear should not leak stale selected-object bottom bounds");
    };

    run_height_clear(temp_root / "height_clear_bounds.frx", "height_clear_bounds.frx", "report");
    run_height_clear(temp_root / "height_clear_bounds.lbx", "height_clear_bounds.lbx", "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_updates_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_update = [&](const fs::path& asset_path,
                                       const std::string& title,
                                       const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto update_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--set-property",
                "--unique-id", "field-guid",
                "--property-name", "HEIGHT",
                "--property-value", "7000",
                "--json"
            },
            temp_root);

        if (update_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout height update stdout:\n"
                      << update_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout height update stderr:\n"
                      << update_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(update_process.exit_code == 0,
               "#1640: report/label layout object stable height update should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.value == "7000",
               "#1640: report/label layout object stable height update should persist the HEIGHT field");
        expect_contains(update_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1640: report/label layout object stable height update should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(update_process.stdout_text, "\"isLabel\": true",
                            "#1640: label layout object stable height update should retain label identity");
        }
        expect_contains(update_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1640: report/label layout object stable height update should preserve preview bounds availability");
        expect_contains(update_process.stdout_text, "\"previewBoundsBottom\": 9600",
                        "#1640: report/label layout object stable height update should refresh preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"previewBoundsHeight\": 9600",
                        "#1640: report/label layout object stable height update should refresh preview heights");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1903: report/label layout object stable height update should preserve deleted preview availability");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1903: report/label layout object stable height update should preserve deleted preview left bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1903: report/label layout object stable height update should preserve deleted preview top bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1903: report/label layout object stable height update should preserve deleted preview right bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1903: report/label layout object stable height update should preserve deleted preview bottom bounds");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1903: report/label layout object stable height update should preserve deleted preview widths");
        expect_contains(update_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1903: report/label layout object stable height update should preserve deleted preview heights");
        expect_contains(update_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1640: report/label layout object stable height update should preserve placed counts");
        expect_contains(update_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1640: report/label layout object stable height update should preserve selected containing-section availability");
        expect_contains_in_order(
            update_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 7000",
                "\"bottom\": 9600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 7600",
                "\"objectKind\": \"field\""
            },
            "#1640: report/label layout object stable height update should refresh selected object bounds and preserve section membership");
    };

    run_height_update(temp_root / "height_bounds_stable.frx",
                      "height_bounds_stable.frx",
                      "report");
    run_height_update(temp_root / "height_bounds_stable.lbx",
                      "height_bounds_stable.lbx",
                      "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_clears_report_layout_object_height_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() / "copperfin_studio_host_report_layout_height_clear_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const auto run_height_clear = [&](const fs::path& asset_path,
                                      const std::string& title,
                                      const std::string& label) {
        write_synthetic_report_table_for_layout_json(asset_path);
        const auto clear_process = run_process_capture(
            studio_host_path,
            {
                "--path", asset_path.string(),
                "--clear-property",
                "--unique-id", "field-guid",
                "--property-name", "HEIGHT",
                "--json"
            },
            temp_root);

        if (clear_process.exit_code != 0) {
            std::cerr << "studio host " << label << " stable layout height clear stdout:\n"
                      << clear_process.stdout_text << "\n";
            std::cerr << "studio host " << label << " stable layout height clear stderr:\n"
                      << clear_process.stderr_text << "\n";
            std::cerr << "fixture root: " << temp_root << "\n";
        }

        expect(clear_process.exit_code == 0,
               "#1641: report/label layout object stable height clear should exit successfully");
        const auto height_property = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 3U,
            .object_name = {},
            .unique_id = "field-guid",
            .property_name = "HEIGHT"
        });
        expect(height_property.ok && height_property.exists && height_property.direct_field &&
                   height_property.value.empty(),
               "#1641: report/label layout object stable height clear should blank the HEIGHT field");
        expect_contains(clear_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                        "#1641: report/label layout object stable height clear should return refreshed report-layout JSON");
        if (asset_path.extension() == ".lbx") {
            expect_contains(clear_process.stdout_text, "\"isLabel\": true",
                            "#1641: label layout object stable height clear should retain label identity");
        }
        expect_contains(clear_process.stdout_text, "\"previewBoundsAvailable\": true",
                        "#1641: report/label layout object stable height clear should preserve preview bounds availability");
        expect_contains(clear_process.stdout_text, "\"previewBoundsBottom\": 8100",
                        "#1641: report/label layout object stable height clear should preserve document preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"previewBoundsHeight\": 8100",
                        "#1641: report/label layout object stable height clear should preserve document preview heights");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                        "#1905: report/label layout object stable height clear should preserve deleted preview availability");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsLeft\": 1000",
                        "#1905: report/label layout object stable height clear should preserve deleted preview left bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsTop\": 2600",
                        "#1905: report/label layout object stable height clear should preserve deleted preview top bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsRight\": 2200",
                        "#1905: report/label layout object stable height clear should preserve deleted preview right bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsBottom\": 2900",
                        "#1905: report/label layout object stable height clear should preserve deleted preview bottom bounds");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1200",
                        "#1905: report/label layout object stable height clear should preserve deleted preview widths");
        expect_contains(clear_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                        "#1905: report/label layout object stable height clear should preserve deleted preview heights");
        expect_contains(clear_process.stdout_text, "\"placedObjectCount\": 2",
                        "#1641: report/label layout object stable height clear should preserve placed counts");
        expect_contains(clear_process.stdout_text, "\"selectedReportObjectSectionAvailable\": true",
                        "#1641: report/label layout object stable height clear should preserve selected containing-section availability");
        expect_contains_in_order(
            clear_process.stdout_text,
            {
                "\"selectedReportObject\": {",
                "\"recordIndex\": 3",
                "\"containingSectionId\": \"detail_2\"",
                "\"top\": 2600",
                "\"height\": 0",
                "\"bottom\": 2600",
                "\"sectionRelativeTop\": 600",
                "\"sectionRelativeBottom\": 600",
                "\"objectKind\": \"field\""
            },
            "#1641: report/label layout object stable height clear should refresh selected object bounds and preserve section membership");
        expect_not_contains(clear_process.stdout_text, "\"height\": 450",
                            "#1641: report/label layout object stable height clear should not leak stale selected-object heights");
        expect_not_contains(clear_process.stdout_text, "\"bottom\": 3050",
                            "#1641: report/label layout object stable height clear should not leak stale selected-object bottom bounds");
    };

    run_height_clear(temp_root / "height_clear_bounds_stable.frx",
                     "height_clear_bounds_stable.frx",
                     "report");
    run_height_clear(temp_root / "height_clear_bounds_stable.lbx",
                     "height_clear_bounds_stable.lbx",
                     "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

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

#if !defined(COPPERFIN_REPORT_LAYOUT_WIDTH_LEFT_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_REPORT_LAYOUT_HEIGHT_TOP_PREVIEW_BOUNDS_ONLY) && \
    !defined(COPPERFIN_DETAIL_HEADER_FOOTER_SECTION_PREVIEW_BOUNDS_SKIP_HOST_SMOKE)

void test_studio_host_json_refreshes_deleted_detail_header_footer_section_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_deleted_detail_header_footer_section_preview_bounds_stable_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_deleted_detail_header_footer_section_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(asset_path);

            const auto update_footer_height_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-footer-guid",
                    "--property-name", "HEIGHT",
                    "--property-value", "700",
                    "--json"
                },
                temp_root);

            if (update_footer_height_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stdout:\n"
                          << update_footer_height_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-footer section preview height stderr:\n"
                          << update_footer_height_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_footer_height_process.exit_code == 0,
                   "#1820: deleted detail-footer section preview height update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1820: deleted detail-footer section preview height update should preserve deleted state");
            const auto footer_height_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 2U,
                .object_name = {},
                .unique_id = "deleted-detail-footer-guid",
                .property_name = "HEIGHT"
            });
            expect(footer_height_property.ok && footer_height_property.exists &&
                       footer_height_property.value == "700",
                   "#1820: deleted detail-footer section preview height update should persist the HEIGHT field");
            expect_contains(update_footer_height_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-footer section preview height update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_footer_height_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-footer label section preview height update should retain label identity");
            }
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve live preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-footer section preview height update should preserve live preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview availability");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsTop\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve deleted preview top bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview bottom bounds");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted preview heights");
            expect_contains(update_footer_height_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-footer section preview height update should preserve live section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-footer section preview height update should refresh deleted section height totals");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-footer section preview height update should preserve selected section availability");
            expect_contains(update_footer_height_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-footer section preview height update should preserve selection kind");
            expect_contains_in_order(
                update_footer_height_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"top\": 800",
                    "\"height\": 700",
                    "\"bottom\": 1500"
                },
                "#1820: deleted detail-footer section preview height update should refresh selected deleted-section geometry");

            const auto update_header_top_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--set-property",
                    "--unique-id", "deleted-detail-header-guid",
                    "--property-name", "VPOS",
                    "--property-value", "350",
                    "--json"
                },
                temp_root);

            if (update_header_top_process.exit_code != 0) {
                std::cerr << "studio host " << label << " deleted detail-header section preview top stdout:\n"
                          << update_header_top_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " deleted detail-header section preview top stderr:\n"
                          << update_header_top_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(update_header_top_process.exit_code == 0,
                   "#1820: deleted detail-header section preview top update by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 1U),
                   "#1820: deleted detail-header section preview top update should preserve deleted state");
            const auto header_top_property = copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 1U,
                .object_name = {},
                .unique_id = "deleted-detail-header-guid",
                .property_name = "VPOS"
            });
            expect(header_top_property.ok && header_top_property.exists &&
                       header_top_property.value == "350",
                   "#1820: deleted detail-header section preview top update should persist the VPOS field");
            expect_contains(update_header_top_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1820: deleted detail-header section preview top update should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(update_header_top_process.stdout_text, "\"isLabel\": true",
                                "#1820: deleted detail-header label section preview top update should retain label identity");
            }
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve live preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1820: deleted detail-header section preview top update should preserve live preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsBottom\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve deleted preview availability");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsTop\": 350",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview top bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsBottom\": 1500",
                            "#1820: deleted detail-header section preview top update should preserve expanded deleted preview bottom bounds");
            expect_contains(update_header_top_process.stdout_text, "\"deletedPreviewBoundsHeight\": 1150",
                            "#1820: deleted detail-header section preview top update should refresh deleted preview heights");
            expect_contains(update_header_top_process.stdout_text, "\"sectionHeightTotal\": 500",
                            "#1820: deleted detail-header section preview top update should preserve live section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"deletedSectionHeightTotal\": 1000",
                            "#1820: deleted detail-header section preview top update should preserve deleted section height totals");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1820: deleted detail-header section preview top update should preserve selected section availability");
            expect_contains(update_header_top_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1820: deleted detail-header section preview top update should preserve selection kind");
            expect_contains_in_order(
                update_header_top_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 1",
                    "\"deleted\": true",
                    "\"top\": 350",
                    "\"height\": 300",
                    "\"bottom\": 650"
                },
                "#1820: deleted detail-header section preview top update should refresh selected deleted-section geometry");
        };

    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "deleted_detail_header_footer_section_preview_bounds_stable.frx",
        "report");
    run_deleted_detail_header_footer_section_preview_bounds(
        temp_root / "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "deleted_detail_header_footer_section_preview_bounds_stable.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_refreshes_detail_header_footer_section_delete_restore_preview_bounds_by_stable_selection(
    const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root =
        fs::temp_directory_path() /
        "copperfin_studio_host_detail_header_footer_section_delete_restore_preview_bounds_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    ScopedDefaultLocaleCatalogEnvironment default_locale_environment;

    const auto run_detail_header_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#2278: detail-header section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#2278: detail-header section preview delete should preserve live preview left bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 50",
                            "#2278: detail-header section preview delete should preserve the retained header-object top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 1040",
                            "#2278: detail-header section preview delete should preserve live preview right bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview delete should preserve sibling live preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 1040",
                            "#2278: detail-header section preview delete should preserve live preview widths");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 500",
                            "#2278: detail-header section preview delete should preserve the retained header-object preview height");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#2278: detail-header section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#2278: detail-header section preview delete should expose deleted preview left bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 0",
                            "#2278: detail-header section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 800",
                            "#2278: detail-header section preview delete should expose deleted preview right bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 800",
                            "#2278: detail-header section preview delete should expose deleted preview widths");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 300",
                            "#2278: detail-header section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#2278: detail-header section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#2278: detail-header section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview delete should not orphan former header objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": true",
                            "#2278: detail-header section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2278: detail-header section preview delete JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300",
                    "\"objectCount\": 1"
                },
                "#2278: detail-header section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Header\"",
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview delete should retain former header object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-header-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-header section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-header section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#2278: detail-header section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 0U),
                   "#2278: detail-header section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#2278: detail-header section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#2278: detail-header label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#2278: detail-header section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#2278: detail-header section preview restore should restore live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#2278: detail-header section preview restore should preserve live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#2278: detail-header section preview restore should restore live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#2278: detail-header section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#2278: detail-header section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#2278: detail-header section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#2278: detail-header section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#2278: detail-header section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#2278: detail-header section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#2278: detail-header section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2278: detail-header section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2278: detail-header section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": true",
                            "#2278: detail-header section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2278: detail-header section preview restore JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Header\"",
                    "\"bandKind\": \"detail_header\"",
                    "\"recordIndex\": 0",
                    "\"deleted\": false",
                    "\"sectionIndex\": 0",
                    "\"sectionCount\": 2",
                    "\"top\": 0",
                    "\"height\": 300",
                    "\"bottom\": 300"
                },
                "#2278: detail-header section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 1",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-header-guid\"",
                    "\"containingSectionRecordIndex\": 0",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#2278: detail-header section preview restore should restore header object containment");
        };

    const auto run_detail_footer_delete_restore_preview_bounds =
        [&](const fs::path& asset_path, const std::string& title, const std::string& label) {
            write_synthetic_report_table_for_detail_header_footer_object_json(asset_path);

            const auto delete_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--delete-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (delete_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview delete stdout:\n"
                          << delete_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview delete stderr:\n"
                          << delete_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(delete_process.exit_code == 0,
                   "#1821: detail-footer section preview delete by stable selection should exit successfully");
            expect(dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview delete should mark the section deleted");
            expect_contains(delete_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview delete should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(delete_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview delete should retain label identity");
            }
            expect_contains(delete_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve live preview availability");
            expect_contains(delete_process.stdout_text, "\"previewBoundsLeft\": 0",
                            "#1821: detail-footer section preview delete should preserve live preview left bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview delete should preserve live preview top bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsRight\": 1040",
                            "#1821: detail-footer section preview delete should preserve live preview right bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsBottom\": 460",
                            "#1821: detail-footer section preview delete should preserve the retained footer-object bottom bounds");
            expect_contains(delete_process.stdout_text, "\"previewBoundsWidth\": 1040",
                            "#1821: detail-footer section preview delete should preserve live preview widths");
            expect_contains(delete_process.stdout_text, "\"previewBoundsHeight\": 460",
                            "#1821: detail-footer section preview delete should preserve the retained footer-object preview height");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsAvailable\": true",
                            "#1821: detail-footer section preview delete should expose deleted preview availability");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsLeft\": 0",
                            "#1821: detail-footer section preview delete should expose deleted preview left bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsTop\": 300",
                            "#1821: detail-footer section preview delete should expose deleted preview top bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsRight\": 1040",
                            "#1821: detail-footer section preview delete should expose deleted preview right bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsBottom\": 550",
                            "#1821: detail-footer section preview delete should expose deleted preview bottom bounds");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsWidth\": 1040",
                            "#1821: detail-footer section preview delete should expose deleted preview widths");
            expect_contains(delete_process.stdout_text, "\"deletedPreviewBoundsHeight\": 250",
                            "#1821: detail-footer section preview delete should expose deleted preview heights");
            expect_contains(delete_process.stdout_text, "\"sectionCount\": 1",
                            "#1821: detail-footer section preview delete should keep the sibling section live");
            expect_contains(delete_process.stdout_text, "\"deletedSectionCount\": 1",
                            "#1821: detail-footer section preview delete should expose one deleted section");
            expect_contains(delete_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview delete should keep sibling objects placed");
            expect_contains(delete_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview delete should not orphan former footer objects");
            expect_contains(delete_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview delete should preserve selected section availability");
            expect_contains(delete_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview delete should preserve selection kind");
            expect_contains(delete_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview delete JSON should expose committed state");
            expect_contains(delete_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview delete JSON should expose mutation state");
            expect_contains(delete_process.stdout_text, "\"undoAvailable\": true",
                            "#2241: detail-footer section preview delete JSON should expose undo availability");
            expect_contains(delete_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2241: detail-footer section preview delete JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": true",
                    "\"sectionIndex\": null",
                    "\"sectionCount\": 0",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550",
                    "\"objectCount\": 1"
                },
                "#1821: detail-footer section preview delete should refresh selected deleted-section geometry");
            expect_contains_in_order(
                delete_process.stdout_text,
                {
                    "\"deletedSections\": [",
                    "\"title\": \"Detail Footer\"",
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview delete should retain former footer object containment inside the deleted section");

            const auto restore_process = run_process_capture(
                studio_host_path,
                {
                    "--path", asset_path.string(),
                    "--restore-object",
                    "--unique-id", "detail-footer-guid",
                    "--json"
                },
                temp_root);

            if (restore_process.exit_code != 0) {
                std::cerr << "studio host " << label << " detail-footer section preview restore stdout:\n"
                          << restore_process.stdout_text << "\n";
                std::cerr << "studio host " << label << " detail-footer section preview restore stderr:\n"
                          << restore_process.stderr_text << "\n";
                std::cerr << "fixture root: " << temp_root << "\n";
            }

            expect(restore_process.exit_code == 0,
                   "#1821: detail-footer section preview restore by stable selection should exit successfully");
            expect(!dbf_record_deleted(asset_path, 2U),
                   "#1821: detail-footer section preview restore should clear the deleted state");
            expect_contains(restore_process.stdout_text, "\"documentTitle\": \"" + title + "\"",
                            "#1821: detail-footer section preview restore should return refreshed layout JSON");
            if (asset_path.extension() == ".lbx") {
                expect_contains(restore_process.stdout_text, "\"isLabel\": true",
                                "#1821: detail-footer label section preview restore should retain label identity");
            }
            expect_contains(restore_process.stdout_text, "\"previewBoundsAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve live preview availability");
            expect_contains(restore_process.stdout_text, "\"previewBoundsTop\": 0",
                            "#1821: detail-footer section preview restore should preserve live preview top bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsBottom\": 550",
                            "#1821: detail-footer section preview restore should expand live preview bottom bounds");
            expect_contains(restore_process.stdout_text, "\"previewBoundsHeight\": 550",
                            "#1821: detail-footer section preview restore should expand live preview heights");
            expect_contains(restore_process.stdout_text, "\"deletedPreviewBoundsAvailable\": false",
                            "#1821: detail-footer section preview restore should clear deleted preview availability");
            expect_contains(restore_process.stdout_text, "\"sectionCount\": 2",
                            "#1821: detail-footer section preview restore should restore live section counts");
            expect_contains(restore_process.stdout_text, "\"deletedSectionCount\": 0",
                            "#1821: detail-footer section preview restore should clear deleted section counts");
            expect_contains(restore_process.stdout_text, "\"placedObjectCount\": 2",
                            "#1821: detail-footer section preview restore should restore placed object counts");
            expect_contains(restore_process.stdout_text, "\"unplacedObjectCount\": 0",
                            "#1821: detail-footer section preview restore should clear unplaced object counts");
            expect_contains(restore_process.stdout_text, "\"selectedReportSectionAvailable\": true",
                            "#1821: detail-footer section preview restore should preserve selected section availability");
            expect_contains(restore_process.stdout_text, "\"selectedReportSelectionKind\": \"section\"",
                            "#1821: detail-footer section preview restore should preserve selection kind");
            expect_contains(restore_process.stdout_text, "\"dryRun\": false",
                            "#2241: detail-footer section preview restore JSON should expose committed state");
            expect_contains(restore_process.stdout_text, "\"mutatesAsset\": true",
                            "#2241: detail-footer section preview restore JSON should expose mutation state");
            expect_contains(restore_process.stdout_text, "\"undoAvailable\": true",
                            "#2241: detail-footer section preview restore JSON should expose undo availability");
            expect_contains(restore_process.stdout_text, "\"undoLabel\": \"Deleted state\"",
                            "#2241: detail-footer section preview restore JSON should expose the deleted-state undo label");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"selectedReportSection\": {",
                    "\"title\": \"Detail Footer\"",
                    "\"bandKind\": \"detail_footer\"",
                    "\"recordIndex\": 2",
                    "\"deleted\": false",
                    "\"sectionIndex\": 1",
                    "\"sectionCount\": 2",
                    "\"top\": 300",
                    "\"height\": 250",
                    "\"bottom\": 550"
                },
                "#1821: detail-footer section preview restore should refresh selected live-section geometry");
            expect_contains_in_order(
                restore_process.stdout_text,
                {
                    "\"objects\": [",
                    "\"recordIndex\": 3",
                    "\"deleted\": false",
                    "\"containingSectionId\": \"detail-footer-guid\"",
                    "\"containingSectionRecordIndex\": 2",
                    "\"sectionObjectIndex\": 0",
                    "\"sectionObjectCount\": 1"
                },
                "#1821: detail-footer section preview restore should restore footer object containment");
        };

    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.frx",
        "detail_header_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_header_delete_restore_preview_bounds(
        temp_root / "detail_header_section_delete_restore_preview_bounds.lbx",
        "detail_header_section_delete_restore_preview_bounds.lbx",
        "label");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.frx",
        "detail_footer_section_delete_restore_preview_bounds.frx",
        "report");
    run_detail_footer_delete_restore_preview_bounds(
        temp_root / "detail_footer_section_delete_restore_preview_bounds.lbx",
        "detail_footer_section_delete_restore_preview_bounds.lbx",
        "label");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}
#endif
#endif

}  // namespace cf_test_studio_host_json
