// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_unresolved_memo_placeholders <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_suppresses_unresolved_report_memo_placeholders(argv[1]);
    test_studio_host_json_suppresses_unresolved_report_section_memo_placeholders(argv[1]);
    test_studio_host_json_suppresses_unresolved_deleted_report_object_memo_placeholders(argv[1]);
    test_studio_host_json_suppresses_unresolved_unplaced_report_object_memo_placeholders(argv[1]);
    test_studio_host_json_defaults_unresolved_report_geometry_memo_placeholders(argv[1]);

    return failures == 0 ? 0 : 1;
}
