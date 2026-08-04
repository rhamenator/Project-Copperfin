// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_layout_placement <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_record_selection(argv[1]);
    test_studio_host_json_moves_report_layout_objects_from_unplaced_to_sections_by_stable_selection(argv[1]);
    test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_record_selection(argv[1]);
    test_studio_host_json_moves_report_layout_objects_from_sections_to_unplaced_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
