// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_reorder_visual_object_memo_properties_within_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\nTop = 20\r\nWidth = 80\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\n"},
        {"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#770: property-reorder fixture should be writable");

    const auto memo_property_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto properties = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!properties.ok) {
            return names;
        }
        for (const auto& property : properties.properties) {
            if (!property.direct_field) {
                names.push_back(property.property_name);
            }
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "width",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support UNIQUEID selection and first placement");
    expect(reorder_result.affected_object_count == 1U,
        "#1005: successful property reorder should report one affected object");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}),
        "#770: first placement should move the requested memo property to the start");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .property_name = "WIDTH",
        .placement = "last",
        .relative_property_name = {}
    });
    expect(reorder_result.ok, "#770: property reorder should support object-name selection and last placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: last placement should move the requested memo property to the end");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "Top"
    });
    expect(reorder_result.ok, "#770: property reorder should support record-index selection and after placement");
    expect(order_is(memo_property_order("save-guid"), {"Left", "Top", "Caption", "Width"}),
        "#770: after placement should move the requested memo property after the relative property");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "Left"
    });
    expect(reorder_result.ok, "#770: property reorder should support before placement");
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: before placement should move the requested memo property before the relative property");

    auto caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto left = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Left"
    });
    auto width = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Width"
    });
    expect(caption.ok && caption.exists && caption.value == "\"Save\"" &&
            left.ok && left.exists && left.value == "10" &&
            width.ok && width.exists && width.value == "80",
        "#770: property reorder should preserve memo property names and values");
    expect(order_is(memo_property_order("name-guid"), {"Caption", "Left"}),
        "#770: property reorder should preserve unrelated object PROPERTIES memos");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "HPOS",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed source fields");
    expect(reorder_result.affected_object_count == 0U,
        "#1005: failed property reorder should report zero affected objects");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = "HPOS"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject direct DBF-backed relative fields");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Missing",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing source properties");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "before",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing relative properties for before placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "after",
        .relative_property_name = "caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject self-relative before/after placement");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption",
        .placement = "middle",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject unknown placements");

    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " ",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject empty source names");

    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: failed property reorders should not mutate the PROPERTIES memo");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#770: undo should restore each successful property reorder");
    }
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}),
        "#770: undo should restore original memo property ordering");

    const fs::path duplicate_path = temp_dir / "property_reorder_duplicate.scx";
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        {{"dupObj", "dupName", "dup-guid", "333", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}});
    expect(duplicate_create_result.ok, "#770: duplicate property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "CAPTION",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate source assignments as ambiguous");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .property_name = "Anchor",
        .placement = "before",
        .relative_property_name = "Caption"
    });
    expect(!reorder_result.ok, "#770: property reorder should reject duplicate relative assignments as ambiguous");

    const fs::path no_properties_path = temp_dir / "property_reorder_no_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const auto no_properties_create_result = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        {{"cmdNoProps", "no-props-guid"}});
    expect(no_properties_create_result.ok, "#770: missing-PROPERTIES property-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption",
        .placement = "first",
        .relative_property_name = {}
    });
    expect(!reorder_result.ok, "#770: property reorder should reject missing PROPERTIES fields");

    fs::remove_all(temp_dir, ignored);
}


void test_reorder_visual_object_memo_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_reorder_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_reorder_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\nTop = 20\r\nWidth = 80\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\nTop = 40\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "Caption = \"Status\"\r\nLeft = 50\r\nWidth = 90\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#773: property-reorder-batch fixture should be writable");

    const auto memo_property_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto properties = copperfin::vfp::list_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!properties.ok) {
            return names;
        }
        for (const auto& property : properties.properties) {
            if (!property.direct_field) {
                names.push_back(property.property_name);
            }
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };
    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .property_name = "Caption",
                .placement = "after",
                .relative_property_name = "Left"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .property_name = "Left",
                .placement = "before",
                .relative_property_name = "Caption"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "last",
                .relative_property_name = {}
            }
        }
    });
    expect(batch_result.ok, "#773: batch property reorder should support mixed selectors and all placements");
    expect(batch_result.affected_object_count == 3U,
        "#1005: successful batch property reorder should count only real mutations");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}) &&
            order_is(memo_property_order("name-guid"), {"Left", "Caption", "Top"}) &&
            order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: batch property reorder should persist expected memo assignment ordering");

    const auto caption = property_state("save-guid", "Caption");
    const auto width = property_state("save-guid", "Width");
    expect(caption.ok && caption.exists && caption.value == "\"Save\"" &&
            width.ok && width.exists && width.value == "80",
        "#773: batch property reorder should preserve assignment names and values");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#773: successful batch reorders should leave normal visual undo history available");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "Top",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "HPOS",
                .placement = "first",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject direct DBF-backed fields");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property reorder should report zero affected objects");
    expect(order_is(memo_property_order("save-guid"), {"Width", "Caption", "Left", "Top"}),
        "#773: direct-field failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .property_name = "Top",
                .placement = "before",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject missing relative names");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: missing-relative failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = " ",
                .placement = "last",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject empty property names");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: empty-name failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "first",
                .relative_property_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "Top",
                .placement = "middle",
                .relative_property_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject unknown placements");
    expect(order_is(memo_property_order("status-guid"), {"Left", "Caption", "Width"}),
        "#773: unknown-placement failures should roll back earlier memo reorders");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Width",
                .placement = "after",
                .relative_property_name = "width"
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject self-relative placement");

    const fs::path duplicate_path = temp_dir / "property_reorder_batch_duplicate.scx";
    const auto duplicate_create = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        {{"dupObj", "dupName", "dup-guid", "111", "Caption = \"First\"\r\ncaption = \"Second\"\r\nAnchor = 0\r\n"}});
    expect(duplicate_create.ok, "#773: duplicate property-reorder-batch fixture should be writable");
    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = duplicate_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .property_name = "Anchor",
                .placement = "before",
                .relative_property_name = "Caption"
            }
        }
    });
    expect(!batch_result.ok, "#773: batch property reorder should reject duplicate relative assignments");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#773: failed batch reorder rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::reorder_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#773: empty batch reorder requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property reorder should report zero affected objects");

    for (int index = 0; index < 3; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#773: undo should restore each successful batch property reorder");
    }
    expect(order_is(memo_property_order("save-guid"), {"Caption", "Left", "Top", "Width"}) &&
            order_is(memo_property_order("name-guid"), {"Caption", "Left", "Top"}) &&
            order_is(memo_property_order("status-guid"), {"Caption", "Left", "Width"}),
        "#773: successful batch reorder undo should restore original memo property order");

    fs::remove_all(temp_dir, ignored);
}


}  // namespace cf_test_visual_asset_editor
