// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {

void write_synthetic_report_table_for_detail_header_footer_section_kind_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "9", "detail header expression", "0", "300", "detail-header-guid"},
        {"9", "10", "detail footer expression", "300", "250", "detail-footer-guid"},
        {"9", "10", "deleted detail footer expression", "550", "200", "deleted-detail-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1763: synthetic report table for detail header/footer section JSON should be created");

    const auto delete_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_result.ok, "#1763: synthetic report table should mark deleted detail footer section");
}

void write_synthetic_report_table_for_deleted_detail_header_footer_section_expression_json(
    const std::filesystem::path& report_path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "VPOS", .type = 'N', .length = 10U},
        {.name = "HEIGHT", .type = 'N', .length = 10U},
        {.name = "UNIQUEID", .type = 'C', .length = 32U}
    };
    const std::vector<std::vector<std::string>> records{
        {"9", "4", "live detail expression", "0", "500", "live-detail-guid"},
        {"9", "9", "deleted detail header expression", "500", "300", "deleted-detail-header-guid"},
        {"9", "10", "deleted detail footer expression", "800", "250", "deleted-detail-footer-guid"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok,
           "#1766: synthetic report table for deleted detail header/footer expression JSON should be created");

    const auto delete_header_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(delete_header_result.ok, "#1766: synthetic report table should mark deleted detail header section");
    const auto delete_footer_result = copperfin::vfp::set_record_deleted_flag(report_path.string(), 2U, true);
    expect(delete_footer_result.ok, "#1766: synthetic report table should mark deleted detail footer section");
}

}  // namespace cf_test_studio_host_json
