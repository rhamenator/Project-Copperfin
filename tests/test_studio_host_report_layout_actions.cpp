// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_layout_actions <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_nudges_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_aligns_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_resizes_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_snaps_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_deletes_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_restores_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_distributes_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_reorders_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_duplicates_report_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_report_layout_object_identity_by_stable_selectors(argv[1]);
    test_studio_host_json_updates_report_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_updates_report_layout_object_expression_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_report_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_clears_report_layout_object_expression_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_report_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_record_selection(argv[1]);
    test_studio_host_json_restores_edited_deleted_report_layout_object_as_unplaced_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
