// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_update_visual_object_report_settings_property_preserves_comment_lines() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_settings_comment_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path report_path = temp_dir / "settings_comment.frx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJTYPE", .type = 'N', .length = 8U},
        {.name = "OBJCODE", .type = 'N', .length = 8U},
        {.name = "EXPR", .type = 'M', .length = 4U},
        {.name = "UNIQUEID", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"1", "53", "* Header = comment text\r\nORIENTATION=1", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(report_path.string(), fields, records);
    expect(create_result.ok, "report-settings comment-line fixture should be writable");

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = report_path.string(), .record_index = 0U, .object_name = {}, .unique_id = {},
        .property_name = "ORIENTATION", .property_value = "2"
    });
    expect(update_result.ok, "updating the real ORIENTATION setting should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(report_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
        "updated report-settings comment-line fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto* expr = find_record_field(parse_result.table.records[0], "EXPR");
        expect(expr != nullptr, "updated record should retain the EXPR field");
        if (expr != nullptr) {
            expect(expr->display_value.find("* Header = comment text") != std::string::npos,
                "a settings comment line containing '=' with non-empty trailing text must survive re-serialization "
                "verbatim, not be misclassified as a real property assignment (its name starts with '*', the VFP "
                "comment marker) and reformatted as \"* Header=comment text\"");
            expect(expr->display_value.find("ORIENTATION=2") != std::string::npos,
                "the real ORIENTATION assignment should be updated to the new value");
        }
    }

    fs::remove_all(temp_dir, ignored);
}
}  // namespace cf_test_visual_asset_editor
