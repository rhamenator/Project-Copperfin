// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void write_synthetic_report_table_for_detail_header_footer_object_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header label\"", "100", "50", "700", "120", "detail-header-label-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.total", "140", "360", "900", "100", "detail-footer-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1764: synthetic report table for detail header/footer object JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_distribution_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header left\"", "100", "50", "50", "100", "detail-header-left-guid"},
        {"5", "", "\"Header middle\"", "175", "60", "50", "100", "detail-header-middle-guid"},
        {"5", "", "\"Header right\"", "700", "70", "50", "100", "detail-header-right-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.left", "140", "360", "50", "100", "detail-footer-left-guid"},
        {"8", "", "footer.middle", "200", "370", "50", "100", "detail-footer-middle-guid"},
        {"8", "", "footer.right", "740", "380", "50", "100", "detail-footer-right-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1797: synthetic report table for detail header/footer object distribution JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_vertical_distribution_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "detail-header-guid"},
        {"5", "", "\"Header left\"", "100", "40", "50", "100", "detail-header-left-guid"},
        {"5", "", "\"Header middle\"", "175", "42", "50", "100", "detail-header-middle-guid"},
        {"5", "", "\"Header right\"", "700", "90", "50", "100", "detail-header-right-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "detail-footer-guid"},
        {"8", "", "footer.left", "140", "330", "50", "100", "detail-footer-left-guid"},
        {"8", "", "footer.middle", "200", "335", "50", "100", "detail-footer-middle-guid"},
        {"8", "", "footer.right", "740", "390", "50", "100", "detail-footer-right-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1799: synthetic report table for detail header/footer object vertical distribution JSON should be created");
}

void write_synthetic_report_table_for_detail_header_footer_object_font_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "HPOS", .type = 'N', .length = 10U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "WIDTH", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "FONTFACE", .type = 'M', .length = 4U},
        {.name = "FONTSIZE", .type = 'C', .length = 16U},
        {.name = "MODE", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "", "0", "", "300", "", "", "", "detail-header-guid"},
        {"5", "", "\"Header label\"", "100", "50", "700", "120", "Courier New", "12", "1",
         "detail-header-label-guid"},
        {"9", "10", "detail footer expression", "", "300", "", "250", "", "", "", "detail-footer-guid"},
        {"8", "", "footer.total", "140", "360", "900", "100", "Segoe UI", "10", "2",
         "detail-footer-field-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1773: synthetic report table for detail header/footer object font JSON should be created");
}

}  // namespace cf_test_studio_host_json
