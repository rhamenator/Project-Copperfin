// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_settings_diagnostics <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_deletes_report_settings_by_record_selection(argv[1]);
    test_studio_host_json_preserves_report_settings_without_root_objcode_schema(argv[1]);
    test_studio_host_json_preserves_duplicate_report_setting_precedence(argv[1]);
    test_studio_host_json_preserves_invalid_first_duplicate_report_setting_precedence(argv[1]);
    test_studio_host_json_preserves_report_settings_without_root_expr_schema(argv[1]);
    test_studio_host_json_exposes_printer_identity_report_settings_summary(argv[1]);
    test_studio_host_json_exposes_color_and_copies_report_settings_summary(argv[1]);

    return failures == 0 ? 0 : 1;
}
