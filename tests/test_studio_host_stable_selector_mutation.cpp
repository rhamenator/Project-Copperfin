// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_stable_selector_mutation <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_sets_properties_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_properties_by_stable_selectors(argv[1]);
    test_studio_host_json_applies_deleted_states_by_stable_selectors(argv[1]);
    test_studio_host_json_applies_subtree_deleted_state_by_stable_selectors(argv[1]);

    return failures == 0 ? 0 : 1;
}
