// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_label_layout_actions <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_exposes_label_layout_parity(argv[1]);
    test_studio_host_json_nudges_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_aligns_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_resizes_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_snaps_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_deletes_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_restores_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_distributes_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_reorders_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_duplicates_label_layout_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_label_layout_object_identity_by_stable_selectors(argv[1]);
    test_studio_host_json_updates_label_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_updates_label_layout_object_expression_by_stable_selection(argv[1]);
    test_studio_host_json_updates_deleted_label_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_clears_label_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_clears_label_layout_object_expression_by_stable_selection(argv[1]);
    test_studio_host_json_clears_deleted_label_layout_object_expressions_by_record_selection(argv[1]);
    test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_record_selection(argv[1]);
    test_studio_host_json_restores_edited_deleted_label_layout_object_as_unplaced_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
