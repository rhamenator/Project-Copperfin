// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_set_visual_object_border_color_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#863",
        "border_color",
        "BorderColor",
        "BORDERCOLOR",
        "border-color",
        0,
        0,
        16777215,
        8421504,
        255,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_border_color({
                .path = path,
                .objects = objects,
                .border_color = value
            });
        });
}

void test_set_visual_object_grid_line_color_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#917",
        "grid_line_color",
        "GridLineColor",
        "GRIDLINECOLOR",
        "grid-line-color",
        0,
        0,
        16777215,
        8421504,
        255,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_grid_line_color({
                .path = path,
                .objects = objects,
                .grid_line_color = value
            });
        });
}

void test_set_visual_object_fill_color_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#939",
        "fill_color",
        "FillColor",
        "FILLCOLOR",
        "fill-color",
        0,
        255,
        65280,
        16711680,
        16777215,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_fill_color({
                .path = path,
                .objects = objects,
                .fill_color = value
            });
        });
}

void test_set_visual_object_selected_back_color_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#879",
        "selected_back_color",
        "SelectedBackColor",
        "SELECTEDBACKCOLOR",
        "selected back-color",
        16777215,
        12632256,
        255,
        65280,
        0,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_selected_back_color({
                .path = path,
                .objects = objects,
                .selected_back_color = value
            });
        });
}

void test_set_visual_object_selected_fore_color_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#880",
        "selected_fore_color",
        "SelectedForeColor",
        "SELECTEDFORECOLOR",
        "selected fore-color",
        16777215,
        12632256,
        255,
        65280,
        0,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_selected_fore_color({
                .path = path,
                .objects = objects,
                .selected_fore_color = value
            });
        });
}

#include "test_visual_asset_editor_appearance_selected_item_colors.inl"
#include "test_visual_asset_editor_appearance_disabled_item_colors.inl"
void test_set_visual_object_item_back_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_item_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "item_back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "ITEMBACKCO", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#821: item back-color fixture should be writable");

    const auto item_back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ItemBackColor"
        });
        expect(result.ok && result.exists, "#821: item back-color fixture property should be readable");
        return result.value;
    };
    const auto item_back_color = [&](const std::string& unique_id) {
        return item_back_color_for(table_path.string(), unique_id);
    };
    const auto item_back_color_state = [&]() {
        return item_back_color("customer-guid") + "," +
            item_back_color("orders-guid") + "," +
            item_back_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .item_back_color = 65280
    });
    expect(color_result.ok, "#821: item back-color assignment should support object-name and record-index selectors");
    expect(item_back_color("customer-guid") == "65280" &&
            item_back_color("orders-guid") == "65280" &&
            item_back_color("other-guid") == "255",
        "#821: direct item back-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#821: first item back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#821: second item back-color write should remain undo-backed");
    expect(item_back_color_state() == "16777215,12632256,255",
        "#821: item back-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .item_back_color = 0
    });
    expect(color_result.ok, "#821: item back-color assignment should support UNIQUEID selectors");
    expect(item_back_color("customer-guid") == "0" &&
            item_back_color("orders-guid") == "0",
        "#821: direct item back-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = item_back_color_state();
    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {},
        .item_back_color = 1
    });
    expect(!color_result.ok, "#821: item back-color assignment should reject empty selections");
    expect(item_back_color_state() == committed_state,
        "#821: empty-selection failures should not mutate item back colors");

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .item_back_color = -1
    });
    expect(!color_result.ok, "#821: item back-color assignment should reject negative values");
    expect(item_back_color_state() == committed_state,
        "#821: negative-value failures should not mutate item back colors");

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .item_back_color = 1
    });
    expect(!color_result.ok, "#821: item back-color assignment should reject missing selected objects");
    expect(item_back_color_state() == committed_state,
        "#821: missing-object failures should not mutate item back colors");

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .item_back_color = 1
    });
    expect(!color_result.ok, "#821: item back-color assignment should reject duplicate selected objects");
    expect(item_back_color_state() == committed_state,
        "#821: duplicate-selection failures should not mutate item back colors");

    const fs::path blob_path = temp_dir / "item_back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ItemBackColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "ItemBackColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#821: item back-color property-blob fixture should be writable");

    const auto blob_item_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ItemBackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .item_back_color = 65280
    });
    expect(color_result.ok, "#821: item back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_item_back_color_state("blob-guid");
    auto appended_color = blob_item_back_color_state("no-color-guid");
    auto other_color = blob_item_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#821: serialized item back-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#821: appended serialized item back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#821: existing serialized item back-color write should remain undo-backed");
    blob_color = blob_item_back_color_state("blob-guid");
    appended_color = blob_item_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#821: serialized item back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_itembackcolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#821: missing-ItemBackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_item_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .item_back_color = 1
    });
    expect(!color_result.ok,
        "#821: item back-color assignment should reject objects without a writable ItemBackColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_item_fore_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_item_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "item_fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "ITEMFORECO", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "0"},
        {"lstOrders", "ordersList", "orders-guid", "16777215"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#822: item fore-color fixture should be writable");

    const auto item_fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ItemForeColor"
        });
        expect(result.ok && result.exists, "#822: item fore-color fixture property should be readable");
        return result.value;
    };
    const auto item_fore_color = [&](const std::string& unique_id) {
        return item_fore_color_for(table_path.string(), unique_id);
    };
    const auto item_fore_color_state = [&]() {
        return item_fore_color("customer-guid") + "," +
            item_fore_color("orders-guid") + "," +
            item_fore_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .item_fore_color = 65280
    });
    expect(color_result.ok, "#822: item fore-color assignment should support object-name and record-index selectors");
    expect(item_fore_color("customer-guid") == "65280" &&
            item_fore_color("orders-guid") == "65280" &&
            item_fore_color("other-guid") == "255",
        "#822: direct item fore-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#822: first item fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#822: second item fore-color write should remain undo-backed");
    expect(item_fore_color_state() == "0,16777215,255",
        "#822: item fore-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .item_fore_color = 0
    });
    expect(color_result.ok, "#822: item fore-color assignment should support UNIQUEID selectors");
    expect(item_fore_color("customer-guid") == "0" &&
            item_fore_color("orders-guid") == "0",
        "#822: direct item fore-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = item_fore_color_state();
    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {},
        .item_fore_color = 1
    });
    expect(!color_result.ok, "#822: item fore-color assignment should reject empty selections");
    expect(item_fore_color_state() == committed_state,
        "#822: empty-selection failures should not mutate item fore colors");

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .item_fore_color = -1
    });
    expect(!color_result.ok, "#822: item fore-color assignment should reject negative values");
    expect(item_fore_color_state() == committed_state,
        "#822: negative-value failures should not mutate item fore colors");

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .item_fore_color = 1
    });
    expect(!color_result.ok, "#822: item fore-color assignment should reject missing selected objects");
    expect(item_fore_color_state() == committed_state,
        "#822: missing-object failures should not mutate item fore colors");

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .item_fore_color = 1
    });
    expect(!color_result.ok, "#822: item fore-color assignment should reject duplicate selected objects");
    expect(item_fore_color_state() == committed_state,
        "#822: duplicate-selection failures should not mutate item fore colors");

    const fs::path blob_path = temp_dir / "item_fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ItemForeColor = 0\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "ItemForeColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#822: item fore-color property-blob fixture should be writable");

    const auto blob_item_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ItemForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .item_fore_color = 65280
    });
    expect(color_result.ok, "#822: item fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_item_fore_color_state("blob-guid");
    auto appended_color = blob_item_fore_color_state("no-color-guid");
    auto other_color = blob_item_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#822: serialized item fore-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#822: appended serialized item fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#822: existing serialized item fore-color write should remain undo-backed");
    blob_color = blob_item_fore_color_state("blob-guid");
    appended_color = blob_item_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "0" &&
            appended_color.ok && !appended_color.exists,
        "#822: serialized item fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_itemforecolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#822: missing-ItemForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_item_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .item_fore_color = 1
    });
    expect(!color_result.ok,
        "#822: item fore-color assignment should reject objects without a writable ItemForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_highlight_back_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_highlight_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "highlight_back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HIGHLIGHTB", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#823: highlight back-color fixture should be writable");

    const auto highlight_back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "HighlightBackColor"
        });
        expect(result.ok && result.exists, "#823: highlight back-color fixture property should be readable");
        return result.value;
    };
    const auto highlight_back_color = [&](const std::string& unique_id) {
        return highlight_back_color_for(table_path.string(), unique_id);
    };
    const auto highlight_back_color_state = [&]() {
        return highlight_back_color("customer-guid") + "," +
            highlight_back_color("orders-guid") + "," +
            highlight_back_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .highlight_back_color = 65280
    });
    expect(color_result.ok, "#823: highlight back-color assignment should support object-name and record-index selectors");
    expect(highlight_back_color("customer-guid") == "65280" &&
            highlight_back_color("orders-guid") == "65280" &&
            highlight_back_color("other-guid") == "255",
        "#823: direct highlight back-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#823: first highlight back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#823: second highlight back-color write should remain undo-backed");
    expect(highlight_back_color_state() == "16777215,12632256,255",
        "#823: highlight back-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .highlight_back_color = 0
    });
    expect(color_result.ok, "#823: highlight back-color assignment should support UNIQUEID selectors");
    expect(highlight_back_color("customer-guid") == "0" &&
            highlight_back_color("orders-guid") == "0",
        "#823: direct highlight back-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = highlight_back_color_state();
    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {},
        .highlight_back_color = 1
    });
    expect(!color_result.ok, "#823: highlight back-color assignment should reject empty selections");
    expect(highlight_back_color_state() == committed_state,
        "#823: empty-selection failures should not mutate highlight back colors");

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .highlight_back_color = -1
    });
    expect(!color_result.ok, "#823: highlight back-color assignment should reject negative values");
    expect(highlight_back_color_state() == committed_state,
        "#823: negative-value failures should not mutate highlight back colors");

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .highlight_back_color = 1
    });
    expect(!color_result.ok, "#823: highlight back-color assignment should reject missing selected objects");
    expect(highlight_back_color_state() == committed_state,
        "#823: missing-object failures should not mutate highlight back colors");

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .highlight_back_color = 1
    });
    expect(!color_result.ok, "#823: highlight back-color assignment should reject duplicate selected objects");
    expect(highlight_back_color_state() == committed_state,
        "#823: duplicate-selection failures should not mutate highlight back colors");

    const fs::path blob_path = temp_dir / "highlight_back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "HighlightBackColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "HighlightBackColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#823: highlight back-color property-blob fixture should be writable");

    const auto blob_highlight_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "HighlightBackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .highlight_back_color = 65280
    });
    expect(color_result.ok, "#823: highlight back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_highlight_back_color_state("blob-guid");
    auto appended_color = blob_highlight_back_color_state("no-color-guid");
    auto other_color = blob_highlight_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#823: serialized highlight back-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#823: appended serialized highlight back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#823: existing serialized highlight back-color write should remain undo-backed");
    blob_color = blob_highlight_back_color_state("blob-guid");
    appended_color = blob_highlight_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#823: serialized highlight back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_highlightbackcolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#823: missing-HighlightBackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_highlight_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .highlight_back_color = 1
    });
    expect(!color_result.ok,
        "#823: highlight back-color assignment should reject objects without a writable HighlightBackColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_highlight_fore_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_highlight_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "highlight_fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HIGHLIGHTF", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#824: highlight fore-color fixture should be writable");

    const auto highlight_fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "HighlightForeColor"
        });
        expect(result.ok && result.exists, "#824: highlight fore-color fixture property should be readable");
        return result.value;
    };
    const auto highlight_fore_color = [&](const std::string& unique_id) {
        return highlight_fore_color_for(table_path.string(), unique_id);
    };
    const auto highlight_fore_color_state = [&]() {
        return highlight_fore_color("customer-guid") + "," +
            highlight_fore_color("orders-guid") + "," +
            highlight_fore_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .highlight_fore_color = 65280
    });
    expect(color_result.ok, "#824: highlight fore-color assignment should support object-name and record-index selectors");
    expect(highlight_fore_color("customer-guid") == "65280" &&
            highlight_fore_color("orders-guid") == "65280" &&
            highlight_fore_color("other-guid") == "255",
        "#824: direct highlight fore-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#824: first highlight fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#824: second highlight fore-color write should remain undo-backed");
    expect(highlight_fore_color_state() == "16777215,12632256,255",
        "#824: highlight fore-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .highlight_fore_color = 0
    });
    expect(color_result.ok, "#824: highlight fore-color assignment should support UNIQUEID selectors");
    expect(highlight_fore_color("customer-guid") == "0" &&
            highlight_fore_color("orders-guid") == "0",
        "#824: direct highlight fore-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = highlight_fore_color_state();
    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {},
        .highlight_fore_color = 1
    });
    expect(!color_result.ok, "#824: highlight fore-color assignment should reject empty selections");
    expect(highlight_fore_color_state() == committed_state,
        "#824: empty-selection failures should not mutate highlight fore colors");

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .highlight_fore_color = -1
    });
    expect(!color_result.ok, "#824: highlight fore-color assignment should reject negative values");
    expect(highlight_fore_color_state() == committed_state,
        "#824: negative-value failures should not mutate highlight fore colors");

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .highlight_fore_color = 1
    });
    expect(!color_result.ok, "#824: highlight fore-color assignment should reject missing selected objects");
    expect(highlight_fore_color_state() == committed_state,
        "#824: missing-object failures should not mutate highlight fore colors");

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .highlight_fore_color = 1
    });
    expect(!color_result.ok, "#824: highlight fore-color assignment should reject duplicate selected objects");
    expect(highlight_fore_color_state() == committed_state,
        "#824: duplicate-selection failures should not mutate highlight fore colors");

    const fs::path blob_path = temp_dir / "highlight_fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "HighlightForeColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "HighlightForeColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#824: highlight fore-color property-blob fixture should be writable");

    const auto blob_highlight_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "HighlightForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .highlight_fore_color = 65280
    });
    expect(color_result.ok, "#824: highlight fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_highlight_fore_color_state("blob-guid");
    auto appended_color = blob_highlight_fore_color_state("no-color-guid");
    auto other_color = blob_highlight_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#824: serialized highlight fore-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#824: appended serialized highlight fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#824: existing serialized highlight fore-color write should remain undo-backed");
    blob_color = blob_highlight_fore_color_state("blob-guid");
    appended_color = blob_highlight_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#824: serialized highlight fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_highlightforecolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#824: missing-HighlightForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_highlight_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .highlight_fore_color = 1
    });
    expect(!color_result.ok,
        "#824: highlight fore-color assignment should reject objects without a writable HighlightForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_back_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "BACKCOLOR", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#825: back-color fixture should be writable");

    const auto back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "BackColor"
        });
        expect(result.ok && result.exists, "#825: back-color fixture property should be readable");
        return result.value;
    };
    const auto back_color = [&](const std::string& unique_id) {
        return back_color_for(table_path.string(), unique_id);
    };
    const auto back_color_state = [&]() {
        return back_color("customer-guid") + "," +
            back_color("orders-guid") + "," +
            back_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .back_color = 65280
    });
    expect(color_result.ok, "#825: back-color assignment should support object-name and record-index selectors");
    expect(back_color("customer-guid") == "65280" &&
            back_color("orders-guid") == "65280" &&
            back_color("other-guid") == "255",
        "#825: direct back-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#825: first back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#825: second back-color write should remain undo-backed");
    expect(back_color_state() == "16777215,12632256,255",
        "#825: back-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .back_color = 0
    });
    expect(color_result.ok, "#825: back-color assignment should support UNIQUEID selectors");
    expect(back_color("customer-guid") == "0" &&
            back_color("orders-guid") == "0",
        "#825: direct back-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = back_color_state();
    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {},
        .back_color = 1
    });
    expect(!color_result.ok, "#825: back-color assignment should reject empty selections");
    expect(back_color_state() == committed_state,
        "#825: empty-selection failures should not mutate back colors");

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .back_color = -1
    });
    expect(!color_result.ok, "#825: back-color assignment should reject negative values");
    expect(back_color_state() == committed_state,
        "#825: negative-value failures should not mutate back colors");

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .back_color = 1
    });
    expect(!color_result.ok, "#825: back-color assignment should reject missing selected objects");
    expect(back_color_state() == committed_state,
        "#825: missing-object failures should not mutate back colors");

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .back_color = 1
    });
    expect(!color_result.ok, "#825: back-color assignment should reject duplicate selected objects");
    expect(back_color_state() == committed_state,
        "#825: duplicate-selection failures should not mutate back colors");

    const fs::path blob_path = temp_dir / "back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "BackColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "BackColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#825: back-color property-blob fixture should be writable");

    const auto blob_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "BackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .back_color = 65280
    });
    expect(color_result.ok, "#825: back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_back_color_state("blob-guid");
    auto appended_color = blob_back_color_state("no-color-guid");
    auto other_color = blob_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#825: serialized back-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#825: appended serialized back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#825: existing serialized back-color write should remain undo-backed");
    blob_color = blob_back_color_state("blob-guid");
    appended_color = blob_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#825: serialized back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_backcolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#825: missing-BackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .back_color = 1
    });
    expect(!color_result.ok,
        "#825: back-color assignment should reject objects without a writable BackColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_fore_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FORECOLOR", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#826: fore-color fixture should be writable");

    const auto fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ForeColor"
        });
        expect(result.ok && result.exists, "#826: fore-color fixture property should be readable");
        return result.value;
    };
    const auto fore_color = [&](const std::string& unique_id) {
        return fore_color_for(table_path.string(), unique_id);
    };
    const auto fore_color_state = [&]() {
        return fore_color("customer-guid") + "," +
            fore_color("orders-guid") + "," +
            fore_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .fore_color = 65280
    });
    expect(color_result.ok, "#826: fore-color assignment should support object-name and record-index selectors");
    expect(fore_color("customer-guid") == "65280" &&
            fore_color("orders-guid") == "65280" &&
            fore_color("other-guid") == "255",
        "#826: direct fore-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#826: first fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#826: second fore-color write should remain undo-backed");
    expect(fore_color_state() == "16777215,12632256,255",
        "#826: fore-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .fore_color = 0
    });
    expect(color_result.ok, "#826: fore-color assignment should support UNIQUEID selectors");
    expect(fore_color("customer-guid") == "0" &&
            fore_color("orders-guid") == "0",
        "#826: direct fore-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = fore_color_state();
    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {},
        .fore_color = 1
    });
    expect(!color_result.ok, "#826: fore-color assignment should reject empty selections");
    expect(fore_color_state() == committed_state,
        "#826: empty-selection failures should not mutate fore colors");

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .fore_color = -1
    });
    expect(!color_result.ok, "#826: fore-color assignment should reject negative values");
    expect(fore_color_state() == committed_state,
        "#826: negative-value failures should not mutate fore colors");

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .fore_color = 1
    });
    expect(!color_result.ok, "#826: fore-color assignment should reject missing selected objects");
    expect(fore_color_state() == committed_state,
        "#826: missing-object failures should not mutate fore colors");

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .fore_color = 1
    });
    expect(!color_result.ok, "#826: fore-color assignment should reject duplicate selected objects");
    expect(fore_color_state() == committed_state,
        "#826: duplicate-selection failures should not mutate fore colors");

    const fs::path blob_path = temp_dir / "fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ForeColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "ForeColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#826: fore-color property-blob fixture should be writable");

    const auto blob_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .fore_color = 65280
    });
    expect(color_result.ok, "#826: fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_fore_color_state("blob-guid");
    auto appended_color = blob_fore_color_state("no-color-guid");
    auto other_color = blob_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#826: serialized fore-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#826: appended serialized fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#826: existing serialized fore-color write should remain undo-backed");
    blob_color = blob_fore_color_state("blob-guid");
    appended_color = blob_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#826: serialized fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_forecolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#826: missing-ForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .fore_color = 1
    });
    expect(!color_result.ok,
        "#826: fore-color assignment should reject objects without a writable ForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_disabled_back_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_disabled_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "disabled_back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DISABLEDBA", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#827: disabled back-color fixture should be writable");

    const auto disabled_back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisabledBackColor"
        });
        expect(result.ok && result.exists, "#827: disabled back-color fixture property should be readable");
        return result.value;
    };
    const auto disabled_back_color = [&](const std::string& unique_id) {
        return disabled_back_color_for(table_path.string(), unique_id);
    };
    const auto disabled_back_color_state = [&]() {
        return disabled_back_color("customer-guid") + "," +
            disabled_back_color("orders-guid") + "," +
            disabled_back_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .disabled_back_color = 65280
    });
    expect(color_result.ok, "#827: disabled back-color assignment should support object-name and record-index selectors");
    expect(disabled_back_color("customer-guid") == "65280" &&
            disabled_back_color("orders-guid") == "65280" &&
            disabled_back_color("other-guid") == "255",
        "#827: direct disabled back-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#827: first disabled back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#827: second disabled back-color write should remain undo-backed");
    expect(disabled_back_color_state() == "16777215,12632256,255",
        "#827: disabled back-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .disabled_back_color = 0
    });
    expect(color_result.ok, "#827: disabled back-color assignment should support UNIQUEID selectors");
    expect(disabled_back_color("customer-guid") == "0" &&
            disabled_back_color("orders-guid") == "0",
        "#827: direct disabled back-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = disabled_back_color_state();
    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {},
        .disabled_back_color = 1
    });
    expect(!color_result.ok, "#827: disabled back-color assignment should reject empty selections");
    expect(disabled_back_color_state() == committed_state,
        "#827: empty-selection failures should not mutate disabled back colors");

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .disabled_back_color = -1
    });
    expect(!color_result.ok, "#827: disabled back-color assignment should reject negative values");
    expect(disabled_back_color_state() == committed_state,
        "#827: negative-value failures should not mutate disabled back colors");

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .disabled_back_color = 1
    });
    expect(!color_result.ok, "#827: disabled back-color assignment should reject missing selected objects");
    expect(disabled_back_color_state() == committed_state,
        "#827: missing-object failures should not mutate disabled back colors");

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .disabled_back_color = 1
    });
    expect(!color_result.ok, "#827: disabled back-color assignment should reject duplicate selected objects");
    expect(disabled_back_color_state() == committed_state,
        "#827: duplicate-selection failures should not mutate disabled back colors");

    const fs::path blob_path = temp_dir / "disabled_back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "DisabledBackColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "DisabledBackColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#827: disabled back-color property-blob fixture should be writable");

    const auto blob_disabled_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisabledBackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .disabled_back_color = 65280
    });
    expect(color_result.ok, "#827: disabled back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_disabled_back_color_state("blob-guid");
    auto appended_color = blob_disabled_back_color_state("no-color-guid");
    auto other_color = blob_disabled_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#827: serialized disabled back-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#827: appended serialized disabled back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#827: existing serialized disabled back-color write should remain undo-backed");
    blob_color = blob_disabled_back_color_state("blob-guid");
    appended_color = blob_disabled_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#827: serialized disabled back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_disabledbackcolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#827: missing-DisabledBackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_disabled_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .disabled_back_color = 1
    });
    expect(!color_result.ok,
        "#827: disabled back-color assignment should reject objects without a writable DisabledBackColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_disabled_fore_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_disabled_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "disabled_fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DISABLEDFO", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#828: disabled fore-color fixture should be writable");

    const auto disabled_fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisabledForeColor"
        });
        expect(result.ok && result.exists, "#828: disabled fore-color fixture property should be readable");
        return result.value;
    };
    const auto disabled_fore_color = [&](const std::string& unique_id) {
        return disabled_fore_color_for(table_path.string(), unique_id);
    };
    const auto disabled_fore_color_state = [&]() {
        return disabled_fore_color("customer-guid") + "," +
            disabled_fore_color("orders-guid") + "," +
            disabled_fore_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .disabled_fore_color = 65280
    });
    expect(color_result.ok, "#828: disabled fore-color assignment should support object-name and record-index selectors");
    expect(disabled_fore_color("customer-guid") == "65280" &&
            disabled_fore_color("orders-guid") == "65280" &&
            disabled_fore_color("other-guid") == "255",
        "#828: direct disabled fore-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#828: first disabled fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#828: second disabled fore-color write should remain undo-backed");
    expect(disabled_fore_color_state() == "16777215,12632256,255",
        "#828: disabled fore-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .disabled_fore_color = 0
    });
    expect(color_result.ok, "#828: disabled fore-color assignment should support UNIQUEID selectors");
    expect(disabled_fore_color("customer-guid") == "0" &&
            disabled_fore_color("orders-guid") == "0",
        "#828: direct disabled fore-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = disabled_fore_color_state();
    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {},
        .disabled_fore_color = 1
    });
    expect(!color_result.ok, "#828: disabled fore-color assignment should reject empty selections");
    expect(disabled_fore_color_state() == committed_state,
        "#828: empty-selection failures should not mutate disabled fore colors");

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .disabled_fore_color = -1
    });
    expect(!color_result.ok, "#828: disabled fore-color assignment should reject negative values");
    expect(disabled_fore_color_state() == committed_state,
        "#828: negative-value failures should not mutate disabled fore colors");

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .disabled_fore_color = 1
    });
    expect(!color_result.ok, "#828: disabled fore-color assignment should reject missing selected objects");
    expect(disabled_fore_color_state() == committed_state,
        "#828: missing-object failures should not mutate disabled fore colors");

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .disabled_fore_color = 1
    });
    expect(!color_result.ok, "#828: disabled fore-color assignment should reject duplicate selected objects");
    expect(disabled_fore_color_state() == committed_state,
        "#828: duplicate-selection failures should not mutate disabled fore colors");

    const fs::path blob_path = temp_dir / "disabled_fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "DisabledForeColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "DisabledForeColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#828: disabled fore-color property-blob fixture should be writable");

    const auto blob_disabled_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisabledForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .disabled_fore_color = 65280
    });
    expect(color_result.ok, "#828: disabled fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_disabled_fore_color_state("blob-guid");
    auto appended_color = blob_disabled_fore_color_state("no-color-guid");
    auto other_color = blob_disabled_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#828: serialized disabled fore-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#828: appended serialized disabled fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#828: existing serialized disabled fore-color write should remain undo-backed");
    blob_color = blob_disabled_fore_color_state("blob-guid");
    appended_color = blob_disabled_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#828: serialized disabled fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_disabledforecolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#828: missing-DisabledForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_disabled_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .disabled_fore_color = 1
    });
    expect(!color_result.ok,
        "#828: disabled fore-color assignment should reject objects without a writable DisabledForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_back_color_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICBAC", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "RGB(255,255,255)"},
        {"lstOrders", "ordersList", "orders-guid", "RGB(192,192,192)"},
        {"cboOther", "otherCombo", "other-guid", "RGB(0,0,255)"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#829: dynamic back-color fixture should be writable");

    const auto dynamic_back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicBackColor"
        });
        expect(result.ok && result.exists, "#829: dynamic back-color fixture property should be readable");
        return result.value;
    };
    const auto dynamic_back_color = [&](const std::string& unique_id) {
        return dynamic_back_color_for(table_path.string(), unique_id);
    };
    const auto dynamic_back_color_state = [&]() {
        return dynamic_back_color("customer-guid") + "," +
            dynamic_back_color("orders-guid") + "," +
            dynamic_back_color("other-guid");
    };

    const std::string zebra_expression = "IIF(RECNO() % 2 = 0, RGB(0,255,0), RGB(255,255,255))";
    auto color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_back_color = zebra_expression
    });
    expect(color_result.ok, "#829: dynamic back-color assignment should support object-name and record-index selectors");
    expect(dynamic_back_color("customer-guid") == zebra_expression &&
            dynamic_back_color("orders-guid") == zebra_expression &&
            dynamic_back_color("other-guid") == "RGB(0,0,255)",
        "#829: direct dynamic back-color assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#829: first dynamic back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#829: second dynamic back-color write should remain undo-backed");
    expect(dynamic_back_color_state() == "RGB(255,255,255),RGB(192,192,192),RGB(0,0,255)",
        "#829: dynamic back-color undo should restore original direct values");

    const std::string constant_expression = "RGB(0,0,0)";
    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_back_color = constant_expression
    });
    expect(color_result.ok, "#829: dynamic back-color assignment should support UNIQUEID selectors");
    expect(dynamic_back_color("customer-guid") == constant_expression &&
            dynamic_back_color("orders-guid") == constant_expression,
        "#829: direct dynamic back-color assignment should store raw constant expressions");

    const std::string committed_state = dynamic_back_color_state();
    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = table_path.string(),
        .objects = {},
        .dynamic_back_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#829: dynamic back-color assignment should reject empty selections");
    expect(dynamic_back_color_state() == committed_state,
        "#829: empty-selection failures should not mutate dynamic back colors");

    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_back_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#829: dynamic back-color assignment should reject missing selected objects");
    expect(dynamic_back_color_state() == committed_state,
        "#829: missing-object failures should not mutate dynamic back colors");

    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .dynamic_back_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#829: dynamic back-color assignment should reject duplicate selected objects");
    expect(dynamic_back_color_state() == committed_state,
        "#829: duplicate-selection failures should not mutate dynamic back colors");

    const fs::path blob_path = temp_dir / "dynamic_back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "DynamicBackColor = RGB(255,255,255)\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "DynamicBackColor = RGB(0,0,255)\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#829: dynamic back-color property-blob fixture should be writable");

    const auto blob_dynamic_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicBackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .dynamic_back_color = zebra_expression
    });
    expect(color_result.ok, "#829: dynamic back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_dynamic_back_color_state("blob-guid");
    auto appended_color = blob_dynamic_back_color_state("no-color-guid");
    auto other_color = blob_dynamic_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == zebra_expression &&
            appended_color.ok && appended_color.exists && appended_color.value == zebra_expression &&
            other_color.ok && other_color.exists && other_color.value == "RGB(0,0,255)",
        "#829: serialized dynamic back-color assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#829: appended serialized dynamic back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#829: existing serialized dynamic back-color write should remain undo-backed");
    blob_color = blob_dynamic_back_color_state("blob-guid");
    appended_color = blob_dynamic_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "RGB(255,255,255)" &&
            appended_color.ok && !appended_color.exists,
        "#829: serialized dynamic back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicbackcolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#829: missing-DynamicBackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_dynamic_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_back_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok,
        "#829: dynamic back-color assignment should reject objects without a writable DynamicBackColor carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_fore_color_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFOR", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "RGB(255,255,255)"},
        {"lstOrders", "ordersList", "orders-guid", "RGB(192,192,192)"},
        {"cboOther", "otherCombo", "other-guid", "RGB(0,0,255)"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#830: dynamic fore-color fixture should be writable");

    const auto dynamic_fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicForeColor"
        });
        expect(result.ok && result.exists, "#830: dynamic fore-color fixture property should be readable");
        return result.value;
    };
    const auto dynamic_fore_color = [&](const std::string& unique_id) {
        return dynamic_fore_color_for(table_path.string(), unique_id);
    };
    const auto dynamic_fore_color_state = [&]() {
        return dynamic_fore_color("customer-guid") + "," +
            dynamic_fore_color("orders-guid") + "," +
            dynamic_fore_color("other-guid");
    };

    const std::string zebra_expression = "IIF(RECNO() % 2 = 0, RGB(0,255,0), RGB(255,255,255))";
    auto color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_fore_color = zebra_expression
    });
    expect(color_result.ok, "#830: dynamic fore-color assignment should support object-name and record-index selectors");
    expect(dynamic_fore_color("customer-guid") == zebra_expression &&
            dynamic_fore_color("orders-guid") == zebra_expression &&
            dynamic_fore_color("other-guid") == "RGB(0,0,255)",
        "#830: direct dynamic fore-color assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#830: first dynamic fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#830: second dynamic fore-color write should remain undo-backed");
    expect(dynamic_fore_color_state() == "RGB(255,255,255),RGB(192,192,192),RGB(0,0,255)",
        "#830: dynamic fore-color undo should restore original direct values");

    const std::string constant_expression = "RGB(0,0,0)";
    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_fore_color = constant_expression
    });
    expect(color_result.ok, "#830: dynamic fore-color assignment should support UNIQUEID selectors");
    expect(dynamic_fore_color("customer-guid") == constant_expression &&
            dynamic_fore_color("orders-guid") == constant_expression,
        "#830: direct dynamic fore-color assignment should store raw constant expressions");

    const std::string committed_state = dynamic_fore_color_state();
    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = table_path.string(),
        .objects = {},
        .dynamic_fore_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#830: dynamic fore-color assignment should reject empty selections");
    expect(dynamic_fore_color_state() == committed_state,
        "#830: empty-selection failures should not mutate dynamic fore colors");

    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_fore_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#830: dynamic fore-color assignment should reject missing selected objects");
    expect(dynamic_fore_color_state() == committed_state,
        "#830: missing-object failures should not mutate dynamic fore colors");

    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .dynamic_fore_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok, "#830: dynamic fore-color assignment should reject duplicate selected objects");
    expect(dynamic_fore_color_state() == committed_state,
        "#830: duplicate-selection failures should not mutate dynamic fore colors");

    const fs::path blob_path = temp_dir / "dynamic_fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "DynamicForeColor = RGB(255,255,255)\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "DynamicForeColor = RGB(0,0,255)\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#830: dynamic fore-color property-blob fixture should be writable");

    const auto blob_dynamic_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .dynamic_fore_color = zebra_expression
    });
    expect(color_result.ok, "#830: dynamic fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_dynamic_fore_color_state("blob-guid");
    auto appended_color = blob_dynamic_fore_color_state("no-color-guid");
    auto other_color = blob_dynamic_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == zebra_expression &&
            appended_color.ok && appended_color.exists && appended_color.value == zebra_expression &&
            other_color.ok && other_color.exists && other_color.value == "RGB(0,0,255)",
        "#830: serialized dynamic fore-color assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#830: appended serialized dynamic fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#830: existing serialized dynamic fore-color write should remain undo-backed");
    blob_color = blob_dynamic_fore_color_state("blob-guid");
    appended_color = blob_dynamic_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "RGB(255,255,255)" &&
            appended_color.ok && !appended_color.exists,
        "#830: serialized dynamic fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicforecolor.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"cboA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#830: missing-DynamicForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_dynamic_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_fore_color = "RGB(1,1,1)"
    });
    expect(!color_result.ok,
        "#830: dynamic fore-color assignment should reject objects without a writable DynamicForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
