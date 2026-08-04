// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_page_setup_fields <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_updates_report_bottom_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_bottom_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_bottom_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_bottom_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_left_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_left_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_left_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_left_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_right_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_right_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_right_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_right_margin_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_grid_vertical_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_grid_vertical_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_grid_vertical_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_grid_vertical_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_grid_horizontal_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_grid_horizontal_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_grid_horizontal_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_grid_horizontal_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_orientation_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_orientation_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_orientation_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_orientation_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_paper_size_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_paper_size_fields_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_paper_size_fields_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_paper_size_fields_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
