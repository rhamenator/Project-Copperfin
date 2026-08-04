// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {

void test_report_settings_case_insensitive_expr_field_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_expr_case_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const std::string& extension, const std::string& expr_field_name) {
        const fs::path asset_path = temp_dir / ("case_" + expr_field_name + extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = expr_field_name, .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(),
            fields,
            {{"1", "53", "CUSTOMSETTING=preserve\r\nGRIDV=4", "case-settings-guid"}});
        expect(create_result.ok, "case-insensitive EXPR fixture should be writable");

        const auto before = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "case-settings-guid",
            .property_name = "GRIDV"
        });
        expect(before.ok && before.exists && before.value == "4",
               "case-insensitive EXPR setting should be readable before mutation");

        const auto update_result = copperfin::vfp::update_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "case-settings-guid",
            .property_name = "GRIDV",
            .property_value = "9"
        });
        expect(update_result.ok,
               "case-insensitive EXPR setting should write through its physical descriptor");

        const auto after = copperfin::vfp::query_visual_object_property({
            .path = asset_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "case-settings-guid",
            .property_name = "GRIDV"
        });
        expect(after.ok && after.exists && after.value == "9",
               "case-insensitive EXPR setting should survive reopen");

        const auto parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        expect(parsed.ok && parsed.table.records.size() == 1U,
               "case-insensitive EXPR asset should remain readable after mutation");
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], expr_field_name);
            expect(expr != nullptr, "EXPR write should preserve the physical field spelling");
            if (expr != nullptr) {
                expect(expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos,
                       "EXPR write should preserve unsupported sibling settings");
                expect(expr->display_value.find("GRIDV=9") != std::string::npos,
                       "EXPR write should update the requested setting");
            }
        }
    };

    run_round_trip(".frx", "expr");
    run_round_trip(".frx", "eXpR");
    run_round_trip(".lbx", "expr");
    run_round_trip(".lbx", "eXpR");

    if (failures == 0) {
        fs::remove_all(temp_dir, ignored);
    }
}

}  // namespace cf_test_visual_asset_editor
