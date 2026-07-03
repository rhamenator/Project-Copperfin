// Copyright © 2026 Richard M. Hamilton. All rights reserved.
// Licensed under the Project Copperfin Source-Available License or
// Commercial License. See LICENSE.md in the repository root.

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_object_action_lifecycle <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_duplicates_visual_object_subtrees(argv[1]);
    test_studio_host_json_deletes_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_restores_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_duplicates_objects_by_stable_selectors(argv[1]);
    test_studio_host_json_renames_objects_by_stable_selectors(argv[1]);

    return failures == 0 ? 0 : 1;
}
