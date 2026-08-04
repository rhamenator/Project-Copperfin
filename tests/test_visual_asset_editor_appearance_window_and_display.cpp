// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
void test_set_visual_object_highlight_row_line_width_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#928",
        "highlight_row_line_width",
        "HighlightRowLineWidth",
        "HIGHLIGHTROWLINEWIDTH",
        "highlight-row-line-width",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_highlight_row_line_width({
                .path = path,
                .objects = objects,
                .highlight_row_line_width = value
            });
        });
}

void test_set_visual_object_highlight_style_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#937",
        "highlight_style",
        "HighlightStyle",
        "HIGHLIGHTSTYLE",
        "highlight-style",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_highlight_style({
                .path = path,
                .objects = objects,
                .highlight_style = value
            });
        });
}

void test_set_visual_object_header_height_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#918",
        "header_height",
        "HeaderHeight",
        "HEADERHEIGHT",
        "header-height",
        0,
        18,
        24,
        30,
        36,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_header_height({
                .path = path,
                .objects = objects,
                .header_height = value
            });
        });
}

void test_set_visual_object_row_height_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#919",
        "row_height",
        "RowHeight",
        "ROWHEIGHT",
        "row-height",
        0,
        18,
        24,
        30,
        36,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_row_height({
                .path = path,
                .objects = objects,
                .row_height = value
            });
        });
}

void test_set_visual_object_scale_mode_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#892",
        "scale_mode",
        "ScaleMode",
        "SCALEMODE",
        "scale-mode",
        0,
        1,
        3,
        2,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_scale_mode({
                .path = path,
                .objects = objects,
                .scale_mode = value
            });
        });
}

void test_set_visual_object_scroll_bars_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#893",
        "scroll_bars",
        "ScrollBars",
        "SCROLLBARS",
        "scroll-bars",
        0,
        1,
        3,
        2,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_scroll_bars({
                .path = path,
                .objects = objects,
                .scroll_bars = value
            });
        });
}

void test_set_visual_object_window_state_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#894",
        "window_state",
        "WindowState",
        "WINDOWSTATE",
        "window-state",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_window_state({
                .path = path,
                .objects = objects,
                .window_state = value
            });
        });
}

void test_set_visual_object_show_window_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#904",
        "show_window",
        "ShowWindow",
        "SHOWWINDOW",
        "show-window",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_show_window({
                .path = path,
                .objects = objects,
                .show_window = value
            });
        });
}

void test_set_visual_object_title_bar_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#905",
        "title_bar",
        "TitleBar",
        "TITLEBAR",
        "title-bar",
        0,
        1,
        2,
        3,
        4,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_title_bar({
                .path = path,
                .objects = objects,
                .title_bar = value
            });
        });
}

void test_set_visual_object_mouse_pointer_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#865",
        "mouse_pointer",
        "MousePointer",
        "MOUSEPOINTER",
        "mouse-pointer",
        0,
        0,
        15,
        99,
        1,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_mouse_pointer({
                .path = path,
                .objects = objects,
                .mouse_pointer = value
            });
        });
}

void test_set_visual_object_input_mask_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_inputmask_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "inputmask.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "INPUTMASK", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtPhone", "phoneBox", "phone-guid", "(999) 999-9999"},
        {"txtZip", "zipBox", "zip-guid", "99999"},
        {"txtOther", "otherBox", "other-guid", "XXXXXXXX"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#803: input-mask fixture should be writable");

    const auto input_mask_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "InputMask"
        });
        expect(result.ok && result.exists, "#803: input-mask fixture property should be readable");
        return result.value;
    };
    const auto input_mask = [&](const std::string& unique_id) {
        return input_mask_for(table_path.string(), unique_id);
    };
    const auto input_mask_state = [&]() {
        return input_mask("phone-guid") + "," +
            input_mask("zip-guid") + "," +
            input_mask("other-guid");
    };

    auto mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtPhone", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .input_mask = "999-99-9999"
    });
    expect(mask_result.ok, "#803: input-mask assignment should support object-name and record-index selectors");
    expect(input_mask("phone-guid") == "999-99-9999" &&
            input_mask("zip-guid") == "999-99-9999" &&
            input_mask("other-guid") == "XXXXXXXX",
        "#803: direct input-mask assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#803: first input-mask write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#803: second input-mask write should remain undo-backed");
    expect(input_mask_state() == "(999) 999-9999,99999,XXXXXXXX",
        "#803: input-mask undo should restore original direct values");

    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "phone-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "zip-guid"}
        },
        .input_mask = "AA-9999"
    });
    expect(mask_result.ok, "#803: input-mask assignment should support UNIQUEID selectors");
    expect(input_mask("phone-guid") == "AA-9999" &&
            input_mask("zip-guid") == "AA-9999",
        "#803: direct input-mask assignment should store caller text without serialized quoting");

    const std::string committed_state = input_mask_state();
    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = table_path.string(),
        .objects = {},
        .input_mask = "Ignored"
    });
    expect(!mask_result.ok, "#803: input-mask assignment should reject empty selections");
    expect(input_mask_state() == committed_state, "#803: empty-selection failures should not mutate input masks");

    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "phone-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .input_mask = "Ignored"
    });
    expect(!mask_result.ok, "#803: input-mask assignment should reject missing selected objects");
    expect(input_mask_state() == committed_state, "#803: missing-object failures should not mutate input masks");

    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "phone-guid"},
            {.record_index = 0U, .object_name = "txtPhone", .unique_id = {}}
        },
        .input_mask = "Ignored"
    });
    expect(!mask_result.ok, "#803: input-mask assignment should reject duplicate selected objects");
    expect(input_mask_state() == committed_state, "#803: duplicate-selection failures should not mutate input masks");

    const fs::path blob_path = temp_dir / "inputmask_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "InputMask = \"99999\"\r\nCaption = \"Zip\"\r\n"},
        {"txtNoMask", "no-mask-guid", "Caption = \"No mask\"\r\n"},
        {"txtOther", "other-guid", "InputMask = \"XXXXXXXX\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#803: input-mask property-blob fixture should be writable");

    const auto blob_input_mask_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "InputMask"
        });
    };

    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoMask", .unique_id = {}}
        },
        .input_mask = "AA\"99"
    });
    expect(mask_result.ok, "#803: input-mask assignment should support existing and absent serialized properties");
    auto blob_mask = blob_input_mask_state("blob-guid");
    auto appended_mask = blob_input_mask_state("no-mask-guid");
    auto other_mask = blob_input_mask_state("other-guid");
    expect(blob_mask.ok && blob_mask.exists && blob_mask.value == "\"AA\"\"99\"" &&
            appended_mask.ok && appended_mask.exists && appended_mask.value == "\"AA\"\"99\"" &&
            other_mask.ok && other_mask.exists && other_mask.value == "\"XXXXXXXX\"",
        "#803: serialized input-mask assignment should quote text, append missing InputMask, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#803: appended serialized input-mask write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#803: existing serialized input-mask write should remain undo-backed");
    blob_mask = blob_input_mask_state("blob-guid");
    appended_mask = blob_input_mask_state("no-mask-guid");
    expect(blob_mask.ok && blob_mask.exists && blob_mask.value == "\"99999\"" &&
            appended_mask.ok && !appended_mask.exists,
        "#803: serialized input-mask undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_inputmask.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"txtA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#803: missing-InputMask fixture should be writable");

    mask_result = copperfin::vfp::set_visual_object_input_mask({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .input_mask = "Ignored"
    });
    expect(!mask_result.ok, "#803: input-mask assignment should reject objects without a writable InputMask carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_format_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_format_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "format.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FORMAT", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtAmount", "amountBox", "amount-guid", "$999,999.99"},
        {"txtDate", "dateBox", "date-guid", "K"},
        {"txtOther", "otherBox", "other-guid", "!"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#804: format fixture should be writable");

    const auto format_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Format"
        });
        expect(result.ok && result.exists, "#804: format fixture property should be readable");
        return result.value;
    };
    const auto format = [&](const std::string& unique_id) {
        return format_for(table_path.string(), unique_id);
    };
    const auto format_state = [&]() {
        return format("amount-guid") + "," +
            format("date-guid") + "," +
            format("other-guid");
    };

    auto format_result = copperfin::vfp::set_visual_object_format({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtAmount", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .format = "999,999.99"
    });
    expect(format_result.ok, "#804: format assignment should support object-name and record-index selectors");
    expect(format("amount-guid") == "999,999.99" &&
            format("date-guid") == "999,999.99" &&
            format("other-guid") == "!",
        "#804: direct format assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#804: first format write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#804: second format write should remain undo-backed");
    expect(format_state() == "$999,999.99,K,!", "#804: format undo should restore original direct values");

    format_result = copperfin::vfp::set_visual_object_format({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "amount-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "date-guid"}
        },
        .format = "@!"
    });
    expect(format_result.ok, "#804: format assignment should support UNIQUEID selectors");
    expect(format("amount-guid") == "@!" &&
            format("date-guid") == "@!",
        "#804: direct format assignment should store caller text without serialized quoting");

    const std::string committed_state = format_state();
    format_result = copperfin::vfp::set_visual_object_format({
        .path = table_path.string(),
        .objects = {},
        .format = "Ignored"
    });
    expect(!format_result.ok, "#804: format assignment should reject empty selections");
    expect(format_state() == committed_state, "#804: empty-selection failures should not mutate formats");

    format_result = copperfin::vfp::set_visual_object_format({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "amount-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .format = "Ignored"
    });
    expect(!format_result.ok, "#804: format assignment should reject missing selected objects");
    expect(format_state() == committed_state, "#804: missing-object failures should not mutate formats");

    format_result = copperfin::vfp::set_visual_object_format({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "amount-guid"},
            {.record_index = 0U, .object_name = "txtAmount", .unique_id = {}}
        },
        .format = "Ignored"
    });
    expect(!format_result.ok, "#804: format assignment should reject duplicate selected objects");
    expect(format_state() == committed_state, "#804: duplicate-selection failures should not mutate formats");

    const fs::path blob_path = temp_dir / "format_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "Format = \"99999\"\r\nCaption = \"Zip\"\r\n"},
        {"txtNoFormat", "no-format-guid", "Caption = \"No format\"\r\n"},
        {"txtOther", "other-guid", "Format = \"!\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#804: format property-blob fixture should be writable");

    const auto blob_format_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "Format"
        });
    };

    format_result = copperfin::vfp::set_visual_object_format({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoFormat", .unique_id = {}}
        },
        .format = "A\"!"
    });
    expect(format_result.ok, "#804: format assignment should support existing and absent serialized properties");
    auto blob_format = blob_format_state("blob-guid");
    auto appended_format = blob_format_state("no-format-guid");
    auto other_format = blob_format_state("other-guid");
    expect(blob_format.ok && blob_format.exists && blob_format.value == "\"A\"\"!\"" &&
            appended_format.ok && appended_format.exists && appended_format.value == "\"A\"\"!\"" &&
            other_format.ok && other_format.exists && other_format.value == "\"!\"",
        "#804: serialized format assignment should quote text, append missing Format, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#804: appended serialized format write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#804: existing serialized format write should remain undo-backed");
    blob_format = blob_format_state("blob-guid");
    appended_format = blob_format_state("no-format-guid");
    expect(blob_format.ok && blob_format.exists && blob_format.value == "\"99999\"" &&
            appended_format.ok && !appended_format.exists,
        "#804: serialized format undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_format.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"txtA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#804: missing-Format fixture should be writable");

    format_result = copperfin::vfp::set_visual_object_format({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .format = "Ignored"
    });
    expect(!format_result.ok, "#804: format assignment should reject objects without a writable Format carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_alignment_assigns_expression_value() {
    test_dynamic_raw_scalar_property_assigns_expression_value(
        "#847",
        "dynamic_alignment",
        "DynamicAlignment",
        "DYNAMICALIGNMENT",
        "alignment",
        "0",
        "1",
        "2",
        "IIF(RECNO() % 2 = 0, 2, 0)",
        "1",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_alignment({
                .path = path,
                .objects = objects,
                .dynamic_alignment = expression
            });
        });
}

void test_set_visual_object_dynamic_current_control_assigns_expression_value() {
    test_dynamic_raw_scalar_property_assigns_expression_value(
        "#850",
        "dynamic_current_control",
        "DynamicCurrentControl",
        "DYNAMICCURRENTCONTROL",
        "current-control",
        "\"txtCustomer\"",
        "\"txtOrders\"",
        "\"txtOther\"",
        "IIF(RECNO() % 2 = 0, \"txtOrders\", \"txtCustomer\")",
        "\"txtCustomer\"",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_current_control({
                .path = path,
                .objects = objects,
                .dynamic_current_control = expression
            });
        });
}

void test_set_visual_object_dynamic_input_mask_assigns_expression_value() {
    test_dynamic_raw_scalar_property_assigns_expression_value(
        "#848",
        "dynamic_input_mask",
        "DynamicInputMask",
        "DYNAMICINPUTMASK",
        "input-mask",
        "\"999-99-9999\"",
        "\"99999\"",
        "\"!!!\"",
        "IIF(EMPTY(ALLTRIM(customer.type)), \"99999\", \"999-99-9999\")",
        "\"999\"",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_input_mask({
                .path = path,
                .objects = objects,
                .dynamic_input_mask = expression
            });
        });
}

}  // namespace cf_test_visual_asset_editor
