// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_json_support.h"

int main(int argc, char** argv) {
    using namespace cf_test_studio_host_json;

    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_layout_classifications <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_ignores_invalid_report_layout_classifications(argv[1]);
    test_studio_host_json_ignores_dot_leading_report_layout_classifications(argv[1]);
    test_studio_host_json_ignores_negative_report_layout_classifications(argv[1]);
    test_studio_host_json_ignores_unsupported_report_layout_objtype_codes(argv[1]);
    test_studio_host_json_uses_integer_portions_for_fractional_report_layout_classifications(argv[1]);
    test_studio_host_json_trims_report_layout_classifications(argv[1]);
    test_studio_host_json_ignores_missing_report_layout_classification_fields(argv[1]);
    test_studio_host_json_ignores_missing_report_layout_objtype_schema(argv[1]);
    test_studio_host_json_exposes_unknown_report_band_codes(argv[1]);
    test_studio_host_json_ignores_invalid_direct_report_page_setup_fields(argv[1]);

    return failures == 0 ? 0 : 1;
}
