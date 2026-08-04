// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_detail_header_footer_section_lifecycle <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_deletes_and_restores_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_duplicates_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_duplicates_deleted_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_renames_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_renames_deleted_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_reorders_detail_header_footer_sections_by_stable_selection(argv[1]);
    test_studio_host_json_reorders_deleted_detail_header_footer_sections_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
