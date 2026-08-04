// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void write_synthetic_report_table_for_ambiguous_settings_json(
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
        {"1", "53", "ORIENTATION=0", "", "", "duplicate-settings-guid"},
        {"1", "53", "PAPERSIZE=1", "", "", "DUPLICATE-SETTINGS-GUID"},
        {"9", "4", "", "0", "3200", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "#1701: synthetic report table for ambiguous stable settings JSON should be created");
}

void write_synthetic_report_table_for_stable_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_layout_json(report_path);
    const auto settings_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "settings-guid"
    });
    expect(settings_unique_id_result.ok,
           "#1839: stable deleted-state batch fixture should seed a settings unique id");
    const auto section_unique_id_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "UNIQUEID",
        .property_value = "section-guid"
    });
    expect(section_unique_id_result.ok,
           "#1839: stable deleted-state batch fixture should seed a section unique id");
    expect(!dbf_record_deleted(report_path, 0U) && !dbf_record_deleted(report_path, 1U),
           "#1839: stable deleted-state batch fixture should preserve live settings and section rows");
}

void write_synthetic_report_table_for_stable_deleted_settings_and_section_json(
    const std::filesystem::path& report_path) {
    write_synthetic_report_table_for_stable_settings_and_section_json(report_path);
    const auto settings_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 0U, true);
    expect(settings_delete_result.ok,
           "#1839: stable deleted-state restore fixture should mark settings deleted");
    const auto section_delete_result =
        copperfin::vfp::set_record_deleted_flag(report_path.string(), 1U, true);
    expect(section_delete_result.ok,
           "#1839: stable deleted-state restore fixture should mark section deleted");
    expect(dbf_record_deleted(report_path, 0U) && dbf_record_deleted(report_path, 1U),
           "#1839: stable deleted-state restore fixture should preserve deleted settings and section rows");
}
}  // namespace cf_test_studio_host_json
