// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_layout_diagnostics <copperfin_studio_host>\n";
        return 2;
    }

    using namespace cf_test_studio_host_json;

    test_studio_host_json_exposes_report_layout_provenance(argv[1]);
    test_studio_host_json_exposes_extended_report_object_kinds(argv[1]);

    return failures == 0 ? 0 : 1;
}
