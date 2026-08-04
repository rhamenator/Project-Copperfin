// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_detail_header_footer_object_geometry <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_updates_detail_header_footer_object_geometry_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_detail_header_footer_object_geometry_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
