// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_remaining_object_lifecycle <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_reparents_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_reorders_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_groups_objects_by_stable_child_selectors(argv[1]);
    test_studio_host_json_assigns_delete_mark_by_stable_selectors(argv[1]);
    test_studio_host_json_ungroups_objects_by_stable_selectors(argv[1]);

    return failures == 0 ? 0 : 1;
}
