// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_report_layout_support.h"

#include <cstdlib>
#include <iostream>

namespace cf_test_report_layout {
void test_build_report_layout_groups_band_objects();
void test_report_image_picture_provenance();
void test_build_report_layout_localizes_section_titles_without_localizing_band_kinds();
void test_report_layout_default_catalog_refreshes_when_locale_changes();
void test_report_layout_section_catalog_entries_cover_placeholder_locales();
void test_build_report_layout_suppresses_unresolved_memo_placeholders();
void test_build_report_layout_carries_group_section_expressions();
void test_build_report_layout_carries_nested_group_section_ordering();
void test_build_report_layout_summarizes_groupings();
void test_build_report_layout_resolves_grouping_expression_for_blank_footer();
void test_build_report_layout_resolves_grouping_expression_for_deleted_blank_footer();
void test_build_report_layout_preserves_group_pairing_when_group_header_moves_below_footer();
void test_build_report_layout_counts_deleted_objects_per_section();
void test_build_report_layout_preserves_live_objects_in_deleted_sections();
void test_build_report_layout_keeps_tall_objects_in_the_section_where_they_begin();
void test_build_report_layout_summarizes_nested_mixed_state_groupings();
void test_build_report_layout_reports_missing_title_provenance_when_unavailable();
void test_build_report_layout_includes_direct_orientation_settings();
void test_build_report_layout_includes_direct_paper_size_settings();
void test_build_report_layout_includes_direct_side_margin_settings();
void test_build_report_layout_summarizes_paper_dimensions();
void test_build_report_layout_summarizes_color_and_copies();
void test_build_report_layout_summarizes_auxiliary_print_settings();
void test_build_report_layout_falls_back_to_deleted_root_summary_settings();
void test_build_report_layout_prefers_live_root_summary_settings_over_deleted_fallback();
void test_build_report_layout_summarizes_root_sort_expression();
void test_build_report_layout_preserves_band_unique_ids_for_section_identity();
}
int main() {
    using namespace cf_test_report_layout;
    test_build_report_layout_groups_band_objects();
    test_report_image_picture_provenance();
    test_build_report_layout_localizes_section_titles_without_localizing_band_kinds();
    test_report_layout_default_catalog_refreshes_when_locale_changes();
    test_report_layout_section_catalog_entries_cover_placeholder_locales();
    test_build_report_layout_suppresses_unresolved_memo_placeholders();
    test_build_report_layout_carries_group_section_expressions();
    test_build_report_layout_carries_nested_group_section_ordering();
    test_build_report_layout_summarizes_groupings();
    test_build_report_layout_resolves_grouping_expression_for_blank_footer();
    test_build_report_layout_resolves_grouping_expression_for_deleted_blank_footer();
    test_build_report_layout_preserves_group_pairing_when_group_header_moves_below_footer();
    test_build_report_layout_counts_deleted_objects_per_section();
    test_build_report_layout_preserves_live_objects_in_deleted_sections();
    test_build_report_layout_keeps_tall_objects_in_the_section_where_they_begin();
    test_build_report_layout_summarizes_nested_mixed_state_groupings();
    test_build_report_layout_reports_missing_title_provenance_when_unavailable();
    test_build_report_layout_includes_direct_orientation_settings();
    test_build_report_layout_includes_direct_paper_size_settings();
    test_build_report_layout_includes_direct_side_margin_settings();
    test_build_report_layout_summarizes_paper_dimensions();
    test_build_report_layout_summarizes_color_and_copies();
    test_build_report_layout_summarizes_auxiliary_print_settings();
    test_build_report_layout_falls_back_to_deleted_root_summary_settings();
    test_build_report_layout_prefers_live_root_summary_settings_over_deleted_fallback();
    test_build_report_layout_summarizes_root_sort_expression();
    test_build_report_layout_preserves_band_unique_ids_for_section_identity();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
