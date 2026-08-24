// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_report_settings_bottom_margin_memo_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_botmargin_memo_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto run_round_trip = [&](const fs::path& asset_path, const fs::path& memo_path) {
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U}, {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "EXPR", .type = 'M', .length = 4U}, {.name = "UNIQUEID", .type = 'C', .length = 40U}
        };
        const std::vector<std::vector<std::string>> records{
            {"1", "53", "* retain this comment\r\nCUSTOMSETTING=preserve\r\nBOTMARGIN=20\r\nGRIDV=4", "settings-guid"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(asset_path.string(), fields, records);
        expect(create_result.ok, "#3920: memo-backed BOTMARGIN fixture should be writable");
        expect(fs::exists(memo_path), "#3920: report/label fixture should create its memo sidecar");

        auto query = copperfin::vfp::query_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN"});
        expect(query.ok && query.exists && !query.direct_field && query.value == "20", "#3920: EXPR-only BOTMARGIN should be readable before mutation");
        const auto clear_result = copperfin::vfp::clear_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN"});
        expect(clear_result.ok, "#3920: EXPR-only BOTMARGIN should clear successfully");
        query = copperfin::vfp::query_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN"});
        expect(query.ok && !query.exists && !query.direct_field, "#3920: cleared BOTMARGIN should remain a writable known memo setting after reopen");

        auto parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        expect(parsed.ok && parsed.table.records.size() == 1U, "#3920: cleared report/label settings should reopen");
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], "EXPR");
            expect(expr != nullptr, "#3920: cleared settings should retain EXPR");
            if (expr != nullptr) {
                expect(expr->display_value.find("BOTMARGIN") == std::string::npos, "#3920: clear should remove only the BOTMARGIN assignment");
                expect(expr->display_value.find("* retain this comment") != std::string::npos && expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos && expr->display_value.find("GRIDV=4") != std::string::npos, "#3920: clear should preserve comments, unsupported settings, and sibling settings");
            }
        }

        const auto cleared_table_bytes = read_file_bytes(asset_path);
        const auto cleared_memo_bytes = read_file_bytes(memo_path);
        const auto repeat_clear = copperfin::vfp::clear_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN"});
        expect(repeat_clear.ok, "#3920: clearing an absent known BOTMARGIN should be a no-op");
        expect(read_file_bytes(asset_path) == cleared_table_bytes && read_file_bytes(memo_path) == cleared_memo_bytes, "#3920: repeated clear should not rewrite primary or memo bytes");

        const auto update_result = copperfin::vfp::update_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN", .property_value = "35"});
        expect(update_result.ok, "#3920: cleared BOTMARGIN should re-materialize in EXPR");
        query = copperfin::vfp::query_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN"});
        expect(query.ok && query.exists && !query.direct_field && query.value == "35", "#3920: re-materialized BOTMARGIN should survive reopen");

        parsed = copperfin::vfp::parse_dbf_table_from_file(asset_path.string(), 1U);
        if (parsed.ok && parsed.table.records.size() == 1U) {
            const auto* expr = find_record_field(parsed.table.records[0], "EXPR");
            expect(expr != nullptr && expr->display_value.find("BOTMARGIN=35") != std::string::npos && expr->display_value.find("CUSTOMSETTING=preserve") != std::string::npos, "#3920: re-set should preserve unsupported memo content");
        }

        const auto updated_table_bytes = read_file_bytes(asset_path);
        const auto updated_memo_bytes = read_file_bytes(memo_path);
        const auto repeat_update = copperfin::vfp::update_visual_object_property({.path = asset_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "settings-guid", .property_name = "BOTMARGIN", .property_value = "35"});
        expect(repeat_update.ok, "#3920: assigning the current BOTMARGIN should be a no-op");
        expect(read_file_bytes(asset_path) == updated_table_bytes && read_file_bytes(memo_path) == updated_memo_bytes, "#3920: repeated update should not rewrite primary or memo bytes");
    };

    run_round_trip(temp_dir / "botmargin.frx", temp_dir / "botmargin.frt");
    run_round_trip(temp_dir / "botmargin.lbx", temp_dir / "botmargin.lbt");
    fs::remove_all(temp_dir, ignored);
}
}  // namespace cf_test_visual_asset_editor
