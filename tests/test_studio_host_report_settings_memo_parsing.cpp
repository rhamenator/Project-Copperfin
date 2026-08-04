// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_settings_memo_parsing <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_suppresses_unresolved_report_direct_setting_memo_placeholders(argv[1]);
    test_studio_host_json_preserves_mixed_report_direct_setting_memo_placeholders(argv[1]);
    test_studio_host_json_ignores_invalid_report_setting_memo_values(argv[1]);
    test_studio_host_json_preserves_fractional_report_setting_memo_values(argv[1]);
    test_studio_host_json_ignores_blank_report_setting_memo_values(argv[1]);
    test_studio_host_json_ignores_malformed_report_setting_memo_lines(argv[1]);
    test_studio_host_json_parses_cr_only_report_setting_memo_lines(argv[1]);
    test_studio_host_json_parses_mixed_case_report_setting_memo_names(argv[1]);
    test_studio_host_json_writes_case_insensitive_expr_fields(argv[1]);

    return failures == 0 ? 0 : 1;
}
