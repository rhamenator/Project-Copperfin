// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

using namespace cf_test_studio_host_json;

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: test_studio_host_report_schema_fallbacks <copperfin_studio_host>\n";
        return 2;
    }

    test_studio_host_json_defaults_missing_report_section_objcode_schema(argv[1]);
    test_studio_host_json_defaults_missing_report_object_objcode_schema(argv[1]);
    test_studio_host_json_preserves_report_sections_without_expr_schema(argv[1]);
    test_studio_host_json_defaults_report_sections_without_geometry_schema(argv[1]);
    test_studio_host_json_preserves_report_objects_without_expr_schema(argv[1]);
    test_studio_host_json_synthesizes_report_object_titles_without_title_schema(argv[1]);

    return failures == 0 ? 0 : 1;
}
