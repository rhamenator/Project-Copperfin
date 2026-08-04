// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_font_metadata <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_updates_report_layout_object_font_metadata_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_layout_object_font_metadata_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_font_metadata_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_layout_object_font_metadata_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_font_options_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_font_options_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_layout_object_font_options_by_record_selection(argv[1]);
    test_studio_host_json_exposes_detail_header_footer_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_exposes_deleted_detail_header_footer_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_updates_detail_header_footer_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_detail_header_footer_object_font_metadata_by_stable_selection(argv[1]);
    test_studio_host_json_updates_detail_header_footer_object_font_options_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_detail_header_footer_object_font_options_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
