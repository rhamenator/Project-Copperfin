// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_live_section_geometry <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_updates_report_section_heights_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_section_heights_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_section_tops_by_record_selection(argv[1]);
    test_studio_host_json_preserves_realistic_zero_top_section_object_membership_on_top_update(argv[1]);
    test_studio_host_json_preserves_tall_object_membership_on_section_top_update(argv[1]);
    test_studio_host_json_clears_report_section_tops_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_section_heights_and_tops_by_stable_selection(argv[1]);
    test_studio_host_json_clears_report_section_heights_and_tops_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
