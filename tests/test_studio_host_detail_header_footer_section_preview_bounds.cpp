// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_detail_header_footer_section_preview_bounds <copperfin_studio_host>\n";
        return 2;
    }

    using namespace cf_test_studio_host_json;

    test_studio_host_json_refreshes_detail_header_footer_section_preview_bounds_by_stable_selection(argv[1]);
    test_studio_host_json_refreshes_deleted_detail_header_footer_section_preview_bounds_by_stable_selection(argv[1]);
    test_studio_host_json_refreshes_detail_header_footer_section_delete_restore_preview_bounds_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
