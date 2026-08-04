// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_deleted_report_visual_property_rejections <copperfin_studio_host>\n";
        return 2;
    }

    using namespace cf_test_studio_host_json;

    test_studio_host_json_rejects_deleted_report_visual_property_rename_by_stable_selection(argv[1]);
    test_studio_host_json_rejects_deleted_report_visual_property_rename_batches_by_stable_selection(argv[1]);
    test_studio_host_json_rejects_deleted_report_visual_property_reorder_by_stable_selection(argv[1]);
    test_studio_host_json_rejects_deleted_report_visual_property_reorder_batches_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
