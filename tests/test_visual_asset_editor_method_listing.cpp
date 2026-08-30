// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {

void test_list_visual_object_methods_reads_selected_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_list_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#746: method-list fixture should be writable");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#746: visual object method lists should support UNIQUEID selection");
    expect(method_result.record_index == 0U && !method_result.record_deleted,
        "#746: visual object method lists should expose resolved record metadata");
    expect(method_result.methods.size() == 2U,
        "#746: visual object method lists should parse PROCEDURE and FUNCTION declarations");
    if (method_result.methods.size() == 2U) {
        expect(method_result.methods[0].method_name == "Click" &&
                method_result.methods[0].kind == "procedure" &&
                method_result.methods[0].source_text == "THISFORM.Save()",
            "#746: visual object method lists should parse procedure names and source bodies");
        expect(method_result.methods[0].source_line_index == 0U &&
                method_result.methods[0].source_memo_block_number != 0U,
            "#746: visual object method lists should expose procedure source-line and memo-block metadata");
        expect(method_result.methods[1].method_name == "GetCaption" &&
                method_result.methods[1].kind == "function" &&
                method_result.methods[1].source_text == "RETURN THIS.Caption",
            "#746: visual object method lists should parse function names and source bodies");
        expect(method_result.methods[1].source_line_index == 3U &&
                method_result.methods[1].source_memo_block_number == method_result.methods[0].source_memo_block_number,
            "#746: later methods should retain declaration line indexes and source memo block metadata");
    }
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#746: visual object method listing should not create undo history");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok, "#746: visual object method lists should support object-name selection");
    expect(method_result.record_index == 1U && method_result.methods.empty(),
        "#746: missing or empty METHODS fields should return an empty method list successfully");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
