// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {

void test_report_printer_duplex_and_winspool_settings_are_admitted() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_printer_settings_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const fs::path& asset_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const std::vector<std::vector<std::string>> records{
            {"1", "53", "* preserve this comment\r\nCUSTOMSETTING=preserve", "printer-settings-guid"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(
            asset_path.string(), fields, records);
        expect(create_result.ok, "printer-settings fixture should be writable");

        const auto query = [&](const std::string& property_name) {
            return copperfin::vfp::query_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "printer-settings-guid",
                .property_name = property_name
            });
        };
        const auto update = [&](const std::string& property_name, const std::string& value) {
            return copperfin::vfp::update_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "printer-settings-guid",
                .property_name = property_name,
                .property_value = value
            });
        };
        const auto clear = [&](const std::string& property_name) {
            return copperfin::vfp::clear_visual_object_property({
                .path = asset_path.string(),
                .record_index = 0U,
                .object_name = {},
                .unique_id = "printer-settings-guid",
                .property_name = property_name
            });
        };

        auto duplex = query("DUPLEX");
        auto winspool = query("WINSPOOL");
        expect(duplex.ok && !duplex.exists && !duplex.direct_field,
               "absent DUPLEX should be an admitted memo setting");
        expect(winspool.ok && !winspool.exists && !winspool.direct_field,
               "absent WINSPOOL should be an admitted memo setting");

        expect(update("DUPLEX", "2").ok,
               "DUPLEX should be addable through the shared editor");
        expect(update("DUPLEX", "3").ok,
               "DUPLEX should be updateable after materialization");
        expect(update("WINSPOOL", "1").ok,
               "WINSPOOL should be addable through the shared editor");
        duplex = query("DUPLEX");
        winspool = query("WINSPOOL");
        expect(duplex.ok && duplex.exists && duplex.value == "3",
               "updated DUPLEX should survive query");
        expect(winspool.ok && winspool.exists && winspool.value == "1",
               "added WINSPOOL should survive query");

        expect(!update("UNSUPPORTEDSETTING", "1").ok,
               "unknown absent printer settings must remain rejected");
        expect(clear("DUPLEX").ok,
               "DUPLEX should be clearable through the shared editor");
        duplex = query("DUPLEX");
        expect(duplex.ok && !duplex.exists,
               "cleared DUPLEX should be absent after reopen state refresh");

        const auto parsed_after_clear = copperfin::vfp::parse_dbf_table_from_file(
            asset_path.string(), 1U);
        expect(parsed_after_clear.ok && parsed_after_clear.table.records.size() == 1U,
               "cleared printer settings fixture should remain readable");
        if (parsed_after_clear.ok && parsed_after_clear.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed_after_clear.table.records[0], "EXPR");
            expect(expr != nullptr, "printer settings fixture should retain EXPR");
            if (expr != nullptr) {
                expect(expr->display_value.find("* preserve this comment") != std::string::npos &&
                           expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos &&
                           expr->display_value.find("WINSPOOL=1") != std::string::npos &&
                           expr->display_value.find("DUPLEX") == std::string::npos,
                       "printer setting edits should preserve comments and unsupported sibling assignments");
            }
        }

        expect(copperfin::vfp::undo_visual_object_property(asset_path.string()).ok,
               "clearing DUPLEX should be undoable");
        duplex = query("DUPLEX");
        expect(duplex.ok && duplex.exists && duplex.value == "3",
               "undo should restore the prior DUPLEX value");

        expect(clear("WINSPOOL").ok,
               "WINSPOOL should be clearable through the shared editor");
        expect(copperfin::vfp::undo_visual_object_property(asset_path.string()).ok,
               "clearing WINSPOOL should be undoable");
        winspool = query("WINSPOOL");
        expect(winspool.ok && winspool.exists && winspool.value == "1",
               "undo should restore the prior WINSPOOL value");
    };

    run_round_trip(temp_dir / "printer_settings.frx");
    run_round_trip(temp_dir / "printer_settings.lbx");
    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
