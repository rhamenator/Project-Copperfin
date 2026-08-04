// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_deleted_states <copperfin_studio_host>\n";
        return 2;
    }

    using namespace cf_test_studio_host_json;

    test_studio_host_json_applies_report_deleted_states_by_stable_selection(argv[1]);
    test_studio_host_json_applies_report_object_deleted_states_by_stable_selection(argv[1]);
    test_studio_host_json_applies_report_object_subtree_deleted_state_by_stable_selection(argv[1]);
    test_studio_host_json_applies_mixed_report_deleted_states_by_stable_selection(argv[1]);

    return failures == 0 ? 0 : 1;
}
