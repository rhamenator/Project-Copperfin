// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_direct_setting_fields <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_ignores_invalid_direct_report_column_setup_fields(argv[1]);
    test_studio_host_json_ignores_invalid_direct_report_margin_grid_fields(argv[1]);
    test_studio_host_json_skips_blank_report_direct_setting_fields(argv[1]);
    test_studio_host_json_preserves_mixed_invalid_report_direct_setting_fields(argv[1]);
    test_studio_host_json_preserves_trimmed_report_direct_setting_fields(argv[1]);
    test_studio_host_json_preserves_fractional_report_direct_setting_fields(argv[1]);
    test_studio_host_json_ignores_oversized_report_direct_setting_fields(argv[1]);
    test_studio_host_json_ignores_dot_leading_report_direct_setting_fields(argv[1]);

    return failures == 0 ? 0 : 1;
}
