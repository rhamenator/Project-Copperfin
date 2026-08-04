// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_detail_header_footer_object_layout_actions <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_aligns_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_aligns_deleted_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_resizes_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_resizes_deleted_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_snaps_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_snaps_deleted_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_nudges_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_nudges_deleted_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_distributes_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_distributes_deleted_detail_header_footer_objects_by_stable_selection(argv[1]);
    test_studio_host_json_distributes_detail_header_footer_objects_vertically_by_stable_selection(argv[1]);
    test_studio_host_json_distributes_deleted_detail_header_footer_objects_vertically_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
