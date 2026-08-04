// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_width_left_preview_bounds <copperfin_studio_host>\n";
        return 2;
    }

    using namespace cf_test_studio_host_json;

    test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_width_preview_bounds_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_width_preview_bounds_by_stable_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_left_preview_bounds_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_left_preview_bounds_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
