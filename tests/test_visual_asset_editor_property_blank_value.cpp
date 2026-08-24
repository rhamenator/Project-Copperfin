// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {

void test_update_visual_object_property_preserves_equals_for_blank_property_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_blank_equals_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "blank_equals.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtBox", "blank-guid", "Caption = \"Hello\"\r\nFormat = \r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "blank-value property fixture should be writable");

    const auto update_result = copperfin::vfp::update_visual_object_property({
        .path = table_path.string(), .record_index = 0U, .object_name = {}, .unique_id = "blank-guid",
        .property_name = "Caption", .property_value = "\"World\""
    });
    expect(update_result.ok, "updating an unrelated property should succeed");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
    expect(parse_result.ok && parse_result.table.records.size() == 1U,
        "updated blank-equals fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 1U) {
        const auto* properties = find_record_field(parse_result.table.records[0], "PROPERTIES");
        expect(properties != nullptr, "updated record should retain the PROPERTIES field");
        if (properties != nullptr) {
            expect(properties->display_value.find("Format =") != std::string::npos,
                "re-serializing the PROPERTIES blob should not drop the '=' for properties with a blank value "
                "(matching the sibling report-settings serializer, which always preserves it)");
        }
    }

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
