// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
#include "test_visual_asset_editor_appearance_base_fonts.inl"
#include "test_visual_asset_editor_appearance_font_styles.inl"
void test_set_visual_object_font_underline_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_underline_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_underline.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTUNDERL", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#835: font-underline fixture should be writable");

    const auto font_underline_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontUnderline"
        });
        expect(result.ok && result.exists, "#835: font-underline fixture property should be readable");
        return result.value;
    };
    const auto font_underline = [&](const std::string& unique_id) {
        return font_underline_for(table_path.string(), unique_id);
    };
    const auto font_underline_state = [&]() {
        return font_underline("customer-guid") + "," +
            font_underline("orders-guid") + "," +
            font_underline("other-guid");
    };

    auto underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_underline = true
    });
    expect(underline_result.ok, "#835: font-underline assignment should support object-name and record-index selectors");
    expect(font_underline("customer-guid") == ".T." &&
            font_underline("orders-guid") == ".T." &&
            font_underline("other-guid") == ".T.",
        "#835: direct font-underline assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#835: first font-underline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#835: second font-underline write should remain undo-backed");
    expect(font_underline_state() == ".F.,.F.,.T.", "#835: font-underline undo should restore original direct values");

    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_underline = true
    });
    expect(underline_result.ok, "#835: font-underline assignment should support UNIQUEID selectors");
    expect(font_underline("customer-guid") == ".T." &&
            font_underline("orders-guid") == ".T.",
        "#835: direct font-underline assignment should store caller logical state");

    const std::string committed_state = font_underline_state();
    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = table_path.string(),
        .objects = {},
        .font_underline = false
    });
    expect(!underline_result.ok, "#835: font-underline assignment should reject empty selections");
    expect(font_underline_state() == committed_state, "#835: empty-selection failures should not mutate font-underline values");

    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_underline = false
    });
    expect(!underline_result.ok, "#835: font-underline assignment should reject missing selected objects");
    expect(font_underline_state() == committed_state, "#835: missing-object failures should not mutate font-underline values");

    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_underline = false
    });
    expect(!underline_result.ok, "#835: font-underline assignment should reject duplicate selected objects");
    expect(font_underline_state() == committed_state, "#835: duplicate-selection failures should not mutate font-underline values");

    const fs::path blob_path = temp_dir / "font_underline_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontUnderline = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoUnderline", "no-underline-guid", "Caption = \"No underline\"\r\n"},
        {"txtOther", "other-guid", "FontUnderline = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#835: font-underline property-blob fixture should be writable");

    const auto blob_font_underline_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontUnderline"
        });
    };

    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoUnderline", .unique_id = {}}
        },
        .font_underline = true
    });
    expect(underline_result.ok, "#835: font-underline assignment should support existing and absent serialized properties");
    auto blob_underline = blob_font_underline_state("blob-guid");
    auto appended_underline = blob_font_underline_state("no-underline-guid");
    auto other_underline = blob_font_underline_state("other-guid");
    expect(blob_underline.ok && blob_underline.exists && blob_underline.value == ".T." &&
            appended_underline.ok && appended_underline.exists && appended_underline.value == ".T." &&
            other_underline.ok && other_underline.exists && other_underline.value == ".T.",
        "#835: serialized font-underline assignment should store logical values, append missing FontUnderline, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#835: appended serialized font-underline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#835: existing serialized font-underline write should remain undo-backed");
    blob_underline = blob_font_underline_state("blob-guid");
    appended_underline = blob_font_underline_state("no-underline-guid");
    expect(blob_underline.ok && blob_underline.exists && blob_underline.value == ".F." &&
            appended_underline.ok && !appended_underline.exists,
        "#835: serialized font-underline undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontunderline.scx";
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
    expect(incomplete_create.ok, "#835: missing-FontUnderline fixture should be writable");

    underline_result = copperfin::vfp::set_visual_object_font_underline({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_underline = true
    });
    expect(!underline_result.ok,
        "#835: font-underline assignment should reject objects without a writable FontUnderline carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_font_strikethru_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_strikethru_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_strikethru.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTSTRIKE", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#836: font-strikethru fixture should be writable");

    const auto font_strikethru_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontStrikethru"
        });
        expect(result.ok && result.exists, "#836: font-strikethru fixture property should be readable");
        return result.value;
    };
    const auto font_strikethru = [&](const std::string& unique_id) {
        return font_strikethru_for(table_path.string(), unique_id);
    };
    const auto font_strikethru_state = [&]() {
        return font_strikethru("customer-guid") + "," +
            font_strikethru("orders-guid") + "," +
            font_strikethru("other-guid");
    };

    auto strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_strikethru = true
    });
    expect(strikethru_result.ok, "#836: font-strikethru assignment should support object-name and record-index selectors");
    expect(font_strikethru("customer-guid") == ".T." &&
            font_strikethru("orders-guid") == ".T." &&
            font_strikethru("other-guid") == ".T.",
        "#836: direct font-strikethru assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#836: first font-strikethru write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#836: second font-strikethru write should remain undo-backed");
    expect(font_strikethru_state() == ".F.,.F.,.T.",
        "#836: font-strikethru undo should restore original direct values");

    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_strikethru = true
    });
    expect(strikethru_result.ok, "#836: font-strikethru assignment should support UNIQUEID selectors");
    expect(font_strikethru("customer-guid") == ".T." &&
            font_strikethru("orders-guid") == ".T.",
        "#836: direct font-strikethru assignment should store caller logical state");

    const std::string committed_state = font_strikethru_state();
    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = table_path.string(),
        .objects = {},
        .font_strikethru = false
    });
    expect(!strikethru_result.ok, "#836: font-strikethru assignment should reject empty selections");
    expect(font_strikethru_state() == committed_state,
        "#836: empty-selection failures should not mutate font-strikethru values");

    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_strikethru = false
    });
    expect(!strikethru_result.ok, "#836: font-strikethru assignment should reject missing selected objects");
    expect(font_strikethru_state() == committed_state,
        "#836: missing-object failures should not mutate font-strikethru values");

    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_strikethru = false
    });
    expect(!strikethru_result.ok, "#836: font-strikethru assignment should reject duplicate selected objects");
    expect(font_strikethru_state() == committed_state,
        "#836: duplicate-selection failures should not mutate font-strikethru values");

    const fs::path blob_path = temp_dir / "font_strikethru_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontStrikethru = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoStrikethru", "no-strikethru-guid", "Caption = \"No strikethru\"\r\n"},
        {"txtOther", "other-guid", "FontStrikethru = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#836: font-strikethru property-blob fixture should be writable");

    const auto blob_font_strikethru_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontStrikethru"
        });
    };

    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoStrikethru", .unique_id = {}}
        },
        .font_strikethru = true
    });
    expect(strikethru_result.ok, "#836: font-strikethru assignment should support existing and absent serialized properties");
    auto blob_strikethru = blob_font_strikethru_state("blob-guid");
    auto appended_strikethru = blob_font_strikethru_state("no-strikethru-guid");
    auto other_strikethru = blob_font_strikethru_state("other-guid");
    expect(blob_strikethru.ok && blob_strikethru.exists && blob_strikethru.value == ".T." &&
            appended_strikethru.ok && appended_strikethru.exists && appended_strikethru.value == ".T." &&
            other_strikethru.ok && other_strikethru.exists && other_strikethru.value == ".T.",
        "#836: serialized font-strikethru assignment should store logical values, append missing FontStrikethru, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#836: appended serialized font-strikethru write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#836: existing serialized font-strikethru write should remain undo-backed");
    blob_strikethru = blob_font_strikethru_state("blob-guid");
    appended_strikethru = blob_font_strikethru_state("no-strikethru-guid");
    expect(blob_strikethru.ok && blob_strikethru.exists && blob_strikethru.value == ".F." &&
            appended_strikethru.ok && !appended_strikethru.exists,
        "#836: serialized font-strikethru undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontstrikethru.scx";
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
    expect(incomplete_create.ok, "#836: missing-FontStrikethru fixture should be writable");

    strikethru_result = copperfin::vfp::set_visual_object_font_strikethru({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_strikethru = true
    });
    expect(!strikethru_result.ok,
        "#836: font-strikethru assignment should reject objects without a writable FontStrikethru carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_font_outline_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_outline_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_outline.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTOUTLIN", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#837: font-outline fixture should be writable");

    const auto font_outline_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontOutline"
        });
        expect(result.ok && result.exists, "#837: font-outline fixture property should be readable");
        return result.value;
    };
    const auto font_outline = [&](const std::string& unique_id) {
        return font_outline_for(table_path.string(), unique_id);
    };
    const auto font_outline_state = [&]() {
        return font_outline("customer-guid") + "," +
            font_outline("orders-guid") + "," +
            font_outline("other-guid");
    };

    auto outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_outline = true
    });
    expect(outline_result.ok, "#837: font-outline assignment should support object-name and record-index selectors");
    expect(font_outline("customer-guid") == ".T." &&
            font_outline("orders-guid") == ".T." &&
            font_outline("other-guid") == ".T.",
        "#837: direct font-outline assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#837: first font-outline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#837: second font-outline write should remain undo-backed");
    expect(font_outline_state() == ".F.,.F.,.T.",
        "#837: font-outline undo should restore original direct values");

    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_outline = true
    });
    expect(outline_result.ok, "#837: font-outline assignment should support UNIQUEID selectors");
    expect(font_outline("customer-guid") == ".T." &&
            font_outline("orders-guid") == ".T.",
        "#837: direct font-outline assignment should store caller logical state");

    const std::string committed_state = font_outline_state();
    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = table_path.string(),
        .objects = {},
        .font_outline = false
    });
    expect(!outline_result.ok, "#837: font-outline assignment should reject empty selections");
    expect(font_outline_state() == committed_state,
        "#837: empty-selection failures should not mutate font-outline values");

    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_outline = false
    });
    expect(!outline_result.ok, "#837: font-outline assignment should reject missing selected objects");
    expect(font_outline_state() == committed_state,
        "#837: missing-object failures should not mutate font-outline values");

    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_outline = false
    });
    expect(!outline_result.ok, "#837: font-outline assignment should reject duplicate selected objects");
    expect(font_outline_state() == committed_state,
        "#837: duplicate-selection failures should not mutate font-outline values");

    const fs::path blob_path = temp_dir / "font_outline_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontOutline = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoOutline", "no-outline-guid", "Caption = \"No outline\"\r\n"},
        {"txtOther", "other-guid", "FontOutline = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#837: font-outline property-blob fixture should be writable");

    const auto blob_font_outline_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontOutline"
        });
    };

    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoOutline", .unique_id = {}}
        },
        .font_outline = true
    });
    expect(outline_result.ok, "#837: font-outline assignment should support existing and absent serialized properties");
    auto blob_outline = blob_font_outline_state("blob-guid");
    auto appended_outline = blob_font_outline_state("no-outline-guid");
    auto other_outline = blob_font_outline_state("other-guid");
    expect(blob_outline.ok && blob_outline.exists && blob_outline.value == ".T." &&
            appended_outline.ok && appended_outline.exists && appended_outline.value == ".T." &&
            other_outline.ok && other_outline.exists && other_outline.value == ".T.",
        "#837: serialized font-outline assignment should store logical values, append missing FontOutline, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#837: appended serialized font-outline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#837: existing serialized font-outline write should remain undo-backed");
    blob_outline = blob_font_outline_state("blob-guid");
    appended_outline = blob_font_outline_state("no-outline-guid");
    expect(blob_outline.ok && blob_outline.exists && blob_outline.value == ".F." &&
            appended_outline.ok && !appended_outline.exists,
        "#837: serialized font-outline undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontoutline.scx";
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
    expect(incomplete_create.ok, "#837: missing-FontOutline fixture should be writable");

    outline_result = copperfin::vfp::set_visual_object_font_outline({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_outline = true
    });
    expect(!outline_result.ok,
        "#837: font-outline assignment should reject objects without a writable FontOutline carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_font_shadow_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_shadow_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_shadow.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTSHADOW", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#838: font-shadow fixture should be writable");

    const auto font_shadow_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontShadow"
        });
        expect(result.ok && result.exists, "#838: font-shadow fixture property should be readable");
        return result.value;
    };
    const auto font_shadow = [&](const std::string& unique_id) {
        return font_shadow_for(table_path.string(), unique_id);
    };
    const auto font_shadow_state = [&]() {
        return font_shadow("customer-guid") + "," +
            font_shadow("orders-guid") + "," +
            font_shadow("other-guid");
    };

    auto shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_shadow = true
    });
    expect(shadow_result.ok, "#838: font-shadow assignment should support object-name and record-index selectors");
    expect(font_shadow("customer-guid") == ".T." &&
            font_shadow("orders-guid") == ".T." &&
            font_shadow("other-guid") == ".T.",
        "#838: direct font-shadow assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#838: first font-shadow write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#838: second font-shadow write should remain undo-backed");
    expect(font_shadow_state() == ".F.,.F.,.T.",
        "#838: font-shadow undo should restore original direct values");

    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_shadow = true
    });
    expect(shadow_result.ok, "#838: font-shadow assignment should support UNIQUEID selectors");
    expect(font_shadow("customer-guid") == ".T." &&
            font_shadow("orders-guid") == ".T.",
        "#838: direct font-shadow assignment should store caller logical state");

    const std::string committed_state = font_shadow_state();
    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = table_path.string(),
        .objects = {},
        .font_shadow = false
    });
    expect(!shadow_result.ok, "#838: font-shadow assignment should reject empty selections");
    expect(font_shadow_state() == committed_state,
        "#838: empty-selection failures should not mutate font-shadow values");

    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_shadow = false
    });
    expect(!shadow_result.ok, "#838: font-shadow assignment should reject missing selected objects");
    expect(font_shadow_state() == committed_state,
        "#838: missing-object failures should not mutate font-shadow values");

    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_shadow = false
    });
    expect(!shadow_result.ok, "#838: font-shadow assignment should reject duplicate selected objects");
    expect(font_shadow_state() == committed_state,
        "#838: duplicate-selection failures should not mutate font-shadow values");

    const fs::path blob_path = temp_dir / "font_shadow_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontShadow = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoShadow", "no-shadow-guid", "Caption = \"No shadow\"\r\n"},
        {"txtOther", "other-guid", "FontShadow = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#838: font-shadow property-blob fixture should be writable");

    const auto blob_font_shadow_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontShadow"
        });
    };

    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoShadow", .unique_id = {}}
        },
        .font_shadow = true
    });
    expect(shadow_result.ok, "#838: font-shadow assignment should support existing and absent serialized properties");
    auto blob_shadow = blob_font_shadow_state("blob-guid");
    auto appended_shadow = blob_font_shadow_state("no-shadow-guid");
    auto other_shadow = blob_font_shadow_state("other-guid");
    expect(blob_shadow.ok && blob_shadow.exists && blob_shadow.value == ".T." &&
            appended_shadow.ok && appended_shadow.exists && appended_shadow.value == ".T." &&
            other_shadow.ok && other_shadow.exists && other_shadow.value == ".T.",
        "#838: serialized font-shadow assignment should store logical values, append missing FontShadow, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#838: appended serialized font-shadow write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#838: existing serialized font-shadow write should remain undo-backed");
    blob_shadow = blob_font_shadow_state("blob-guid");
    appended_shadow = blob_font_shadow_state("no-shadow-guid");
    expect(blob_shadow.ok && blob_shadow.exists && blob_shadow.value == ".F." &&
            appended_shadow.ok && !appended_shadow.exists,
        "#838: serialized font-shadow undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontshadow.scx";
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
    expect(incomplete_create.ok, "#838: missing-FontShadow fixture should be writable");

    shadow_result = copperfin::vfp::set_visual_object_font_shadow({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_shadow = true
    });
    expect(!shadow_result.ok,
        "#838: font-shadow assignment should reject objects without a writable FontShadow carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_font_name_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_font_name_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_font_name.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", "\"Arial\""},
        {"lblOrders", "ordersLabel", "orders-guid", "\"Tahoma\""},
        {"txtOther", "otherBox", "other-guid", "\"Courier New\""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#839: dynamic font-name fixture should be writable");

    const auto dynamic_font_name_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontName"
        });
        expect(result.ok && result.exists, "#839: dynamic font-name fixture property should be readable");
        return result.value;
    };
    const auto dynamic_font_name = [&](const std::string& unique_id) {
        return dynamic_font_name_for(table_path.string(), unique_id);
    };
    const auto dynamic_font_name_state = [&]() {
        return dynamic_font_name("customer-guid") + "," +
            dynamic_font_name("orders-guid") + "," +
            dynamic_font_name("other-guid");
    };

    const std::string font_expression = "IIF(RECNO() % 2 = 0, \"Segoe UI\", \"Consolas\")";
    auto font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_font_name = font_expression
    });
    expect(font_result.ok, "#839: dynamic font-name assignment should support object-name and record-index selectors");
    expect(dynamic_font_name("customer-guid") == font_expression &&
            dynamic_font_name("orders-guid") == font_expression &&
            dynamic_font_name("other-guid") == "\"Courier New\"",
        "#839: direct dynamic font-name assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#839: first dynamic font-name write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#839: second dynamic font-name write should remain undo-backed");
    expect(dynamic_font_name_state() == "\"Arial\",\"Tahoma\",\"Courier New\"",
        "#839: dynamic font-name undo should restore original direct values");

    const std::string constant_expression = "\"Verdana\"";
    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_font_name = constant_expression
    });
    expect(font_result.ok, "#839: dynamic font-name assignment should support UNIQUEID selectors");
    expect(dynamic_font_name("customer-guid") == constant_expression &&
            dynamic_font_name("orders-guid") == constant_expression,
        "#839: direct dynamic font-name assignment should store raw constant expressions");

    const std::string committed_state = dynamic_font_name_state();
    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = table_path.string(),
        .objects = {},
        .dynamic_font_name = "\"Ignored\""
    });
    expect(!font_result.ok, "#839: dynamic font-name assignment should reject empty selections");
    expect(dynamic_font_name_state() == committed_state,
        "#839: empty-selection failures should not mutate dynamic font names");

    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_font_name = "\"Ignored\""
    });
    expect(!font_result.ok, "#839: dynamic font-name assignment should reject missing selected objects");
    expect(dynamic_font_name_state() == committed_state,
        "#839: missing-object failures should not mutate dynamic font names");

    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .dynamic_font_name = "\"Ignored\""
    });
    expect(!font_result.ok, "#839: dynamic font-name assignment should reject duplicate selected objects");
    expect(dynamic_font_name_state() == committed_state,
        "#839: duplicate-selection failures should not mutate dynamic font names");

    const fs::path blob_path = temp_dir / "dynamic_font_name_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "DynamicFontName = \"Arial\"\r\nCaption = \"Customer\"\r\n"},
        {"txtNoFont", "no-font-guid", "Caption = \"No font\"\r\n"},
        {"txtOther", "other-guid", "DynamicFontName = \"Courier New\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#839: dynamic font-name property-blob fixture should be writable");

    const auto blob_dynamic_font_name_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontName"
        });
    };

    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoFont", .unique_id = {}}
        },
        .dynamic_font_name = font_expression
    });
    expect(font_result.ok, "#839: dynamic font-name assignment should support existing and absent serialized properties");
    auto blob_font = blob_dynamic_font_name_state("blob-guid");
    auto appended_font = blob_dynamic_font_name_state("no-font-guid");
    auto other_font = blob_dynamic_font_name_state("other-guid");
    expect(blob_font.ok && blob_font.exists && blob_font.value == font_expression &&
            appended_font.ok && appended_font.exists && appended_font.value == font_expression &&
            other_font.ok && other_font.exists && other_font.value == "\"Courier New\"",
        "#839: serialized dynamic font-name assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#839: appended serialized dynamic font-name write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#839: existing serialized dynamic font-name write should remain undo-backed");
    blob_font = blob_dynamic_font_name_state("blob-guid");
    appended_font = blob_dynamic_font_name_state("no-font-guid");
    expect(blob_font.ok && blob_font.exists && blob_font.value == "\"Arial\"" &&
            appended_font.ok && !appended_font.exists,
        "#839: serialized dynamic font-name undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicfontname.scx";
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
    expect(incomplete_create.ok, "#839: missing-DynamicFontName fixture should be writable");

    font_result = copperfin::vfp::set_visual_object_dynamic_font_name({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_font_name = "\"Ignored\""
    });
    expect(!font_result.ok,
        "#839: dynamic font-name assignment should reject objects without a writable DynamicFontName carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_font_size_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_font_size_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_font_size.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", "10"},
        {"lblOrders", "ordersLabel", "orders-guid", "9"},
        {"txtOther", "otherBox", "other-guid", "12"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#840: dynamic font-size fixture should be writable");

    const auto dynamic_font_size_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontSize"
        });
        expect(result.ok && result.exists, "#840: dynamic font-size fixture property should be readable");
        return result.value;
    };
    const auto dynamic_font_size = [&](const std::string& unique_id) {
        return dynamic_font_size_for(table_path.string(), unique_id);
    };
    const auto dynamic_font_size_state = [&]() {
        return dynamic_font_size("customer-guid") + "," +
            dynamic_font_size("orders-guid") + "," +
            dynamic_font_size("other-guid");
    };

    const std::string size_expression = "IIF(RECNO() % 2 = 0, 12, 10)";
    auto size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_font_size = size_expression
    });
    expect(size_result.ok, "#840: dynamic font-size assignment should support object-name and record-index selectors");
    expect(dynamic_font_size("customer-guid") == size_expression &&
            dynamic_font_size("orders-guid") == size_expression &&
            dynamic_font_size("other-guid") == "12",
        "#840: direct dynamic font-size assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#840: first dynamic font-size write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#840: second dynamic font-size write should remain undo-backed");
    expect(dynamic_font_size_state() == "10,9,12",
        "#840: dynamic font-size undo should restore original direct values");

    const std::string constant_expression = "11";
    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_font_size = constant_expression
    });
    expect(size_result.ok, "#840: dynamic font-size assignment should support UNIQUEID selectors");
    expect(dynamic_font_size("customer-guid") == constant_expression &&
            dynamic_font_size("orders-guid") == constant_expression,
        "#840: direct dynamic font-size assignment should store raw constant expressions");

    const std::string committed_state = dynamic_font_size_state();
    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = table_path.string(),
        .objects = {},
        .dynamic_font_size = "99"
    });
    expect(!size_result.ok, "#840: dynamic font-size assignment should reject empty selections");
    expect(dynamic_font_size_state() == committed_state,
        "#840: empty-selection failures should not mutate dynamic font sizes");

    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_font_size = "99"
    });
    expect(!size_result.ok, "#840: dynamic font-size assignment should reject missing selected objects");
    expect(dynamic_font_size_state() == committed_state,
        "#840: missing-object failures should not mutate dynamic font sizes");

    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .dynamic_font_size = "99"
    });
    expect(!size_result.ok, "#840: dynamic font-size assignment should reject duplicate selected objects");
    expect(dynamic_font_size_state() == committed_state,
        "#840: duplicate-selection failures should not mutate dynamic font sizes");

    const fs::path blob_path = temp_dir / "dynamic_font_size_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "DynamicFontSize = 10\r\nCaption = \"Customer\"\r\n"},
        {"txtNoSize", "no-size-guid", "Caption = \"No size\"\r\n"},
        {"txtOther", "other-guid", "DynamicFontSize = 12\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#840: dynamic font-size property-blob fixture should be writable");

    const auto blob_dynamic_font_size_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontSize"
        });
    };

    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoSize", .unique_id = {}}
        },
        .dynamic_font_size = size_expression
    });
    expect(size_result.ok, "#840: dynamic font-size assignment should support existing and absent serialized properties");
    auto blob_size = blob_dynamic_font_size_state("blob-guid");
    auto appended_size = blob_dynamic_font_size_state("no-size-guid");
    auto other_size = blob_dynamic_font_size_state("other-guid");
    expect(blob_size.ok && blob_size.exists && blob_size.value == size_expression &&
            appended_size.ok && appended_size.exists && appended_size.value == size_expression &&
            other_size.ok && other_size.exists && other_size.value == "12",
        "#840: serialized dynamic font-size assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#840: appended serialized dynamic font-size write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#840: existing serialized dynamic font-size write should remain undo-backed");
    blob_size = blob_dynamic_font_size_state("blob-guid");
    appended_size = blob_dynamic_font_size_state("no-size-guid");
    expect(blob_size.ok && blob_size.exists && blob_size.value == "10" &&
            appended_size.ok && !appended_size.exists,
        "#840: serialized dynamic font-size undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicfontsize.scx";
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
    expect(incomplete_create.ok, "#840: missing-DynamicFontSize fixture should be writable");

    size_result = copperfin::vfp::set_visual_object_dynamic_font_size({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_font_size = "99"
    });
    expect(!size_result.ok,
        "#840: dynamic font-size assignment should reject objects without a writable DynamicFontSize carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_line_height_assigns_expression_value() {
    test_dynamic_raw_scalar_property_assigns_expression_value(
        "#849",
        "dynamic_line_height",
        "DynamicLineHeight",
        "DYNAMICLINEHEIGHT",
        "line-height",
        "12",
        "14",
        "16",
        "IIF(RECNO() % 2 = 0, 18, 12)",
        "15",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_line_height({
                .path = path,
                .objects = objects,
                .dynamic_line_height = expression
            });
        });
}

void test_set_visual_object_dynamic_font_bold_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_font_bold_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_font_bold.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#841: dynamic font-bold fixture should be writable");

    const auto dynamic_font_bold_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontBold"
        });
        expect(result.ok && result.exists, "#841: dynamic font-bold fixture property should be readable");
        return result.value;
    };
    const auto dynamic_font_bold = [&](const std::string& unique_id) {
        return dynamic_font_bold_for(table_path.string(), unique_id);
    };
    const auto dynamic_font_bold_state = [&]() {
        return dynamic_font_bold("customer-guid") + "," +
            dynamic_font_bold("orders-guid") + "," +
            dynamic_font_bold("other-guid");
    };

    const std::string bold_expression = "IIF(RECNO() % 2 = 0, .T., .F.)";
    auto bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_font_bold = bold_expression
    });
    expect(bold_result.ok, "#841: dynamic font-bold assignment should support object-name and record-index selectors");
    expect(dynamic_font_bold("customer-guid") == bold_expression &&
            dynamic_font_bold("orders-guid") == bold_expression &&
            dynamic_font_bold("other-guid") == ".F.",
        "#841: direct dynamic font-bold assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#841: first dynamic font-bold write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#841: second dynamic font-bold write should remain undo-backed");
    expect(dynamic_font_bold_state() == ".F.,.T.,.F.",
        "#841: dynamic font-bold undo should restore original direct values");

    const std::string constant_expression = ".T.";
    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_font_bold = constant_expression
    });
    expect(bold_result.ok, "#841: dynamic font-bold assignment should support UNIQUEID selectors");
    expect(dynamic_font_bold("customer-guid") == constant_expression &&
            dynamic_font_bold("orders-guid") == constant_expression,
        "#841: direct dynamic font-bold assignment should store raw constant expressions");

    const std::string committed_state = dynamic_font_bold_state();
    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = table_path.string(),
        .objects = {},
        .dynamic_font_bold = ".F."
    });
    expect(!bold_result.ok, "#841: dynamic font-bold assignment should reject empty selections");
    expect(dynamic_font_bold_state() == committed_state,
        "#841: empty-selection failures should not mutate dynamic font-bold expressions");

    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_font_bold = ".F."
    });
    expect(!bold_result.ok, "#841: dynamic font-bold assignment should reject missing selected objects");
    expect(dynamic_font_bold_state() == committed_state,
        "#841: missing-object failures should not mutate dynamic font-bold expressions");

    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .dynamic_font_bold = ".F."
    });
    expect(!bold_result.ok, "#841: dynamic font-bold assignment should reject duplicate selected objects");
    expect(dynamic_font_bold_state() == committed_state,
        "#841: duplicate-selection failures should not mutate dynamic font-bold expressions");

    const fs::path blob_path = temp_dir / "dynamic_font_bold_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "DynamicFontBold = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoBold", "no-bold-guid", "Caption = \"No bold\"\r\n"},
        {"txtOther", "other-guid", "DynamicFontBold = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#841: dynamic font-bold property-blob fixture should be writable");

    const auto blob_dynamic_font_bold_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontBold"
        });
    };

    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoBold", .unique_id = {}}
        },
        .dynamic_font_bold = bold_expression
    });
    expect(bold_result.ok, "#841: dynamic font-bold assignment should support existing and absent serialized properties");
    auto blob_bold = blob_dynamic_font_bold_state("blob-guid");
    auto appended_bold = blob_dynamic_font_bold_state("no-bold-guid");
    auto other_bold = blob_dynamic_font_bold_state("other-guid");
    expect(blob_bold.ok && blob_bold.exists && blob_bold.value == bold_expression &&
            appended_bold.ok && appended_bold.exists && appended_bold.value == bold_expression &&
            other_bold.ok && other_bold.exists && other_bold.value == ".T.",
        "#841: serialized dynamic font-bold assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#841: appended serialized dynamic font-bold write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#841: existing serialized dynamic font-bold write should remain undo-backed");
    blob_bold = blob_dynamic_font_bold_state("blob-guid");
    appended_bold = blob_dynamic_font_bold_state("no-bold-guid");
    expect(blob_bold.ok && blob_bold.exists && blob_bold.value == ".F." &&
            appended_bold.ok && !appended_bold.exists,
        "#841: serialized dynamic font-bold undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicfontbold.scx";
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
    expect(incomplete_create.ok, "#841: missing-DynamicFontBold fixture should be writable");

    bold_result = copperfin::vfp::set_visual_object_dynamic_font_bold({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_font_bold = ".F."
    });
    expect(!bold_result.ok,
        "#841: dynamic font-bold assignment should reject objects without a writable DynamicFontBold carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_font_italic_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_font_italic_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_font_italic.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#842: dynamic font-italic fixture should be writable");

    const auto dynamic_font_italic_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontItalic"
        });
        expect(result.ok && result.exists, "#842: dynamic font-italic fixture property should be readable");
        return result.value;
    };
    const auto dynamic_font_italic = [&](const std::string& unique_id) {
        return dynamic_font_italic_for(table_path.string(), unique_id);
    };
    const auto dynamic_font_italic_state = [&]() {
        return dynamic_font_italic("customer-guid") + "," +
            dynamic_font_italic("orders-guid") + "," +
            dynamic_font_italic("other-guid");
    };

    const std::string italic_expression = "IIF(RECNO() % 2 = 0, .T., .F.)";
    auto italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_font_italic = italic_expression
    });
    expect(italic_result.ok, "#842: dynamic font-italic assignment should support object-name and record-index selectors");
    expect(dynamic_font_italic("customer-guid") == italic_expression &&
            dynamic_font_italic("orders-guid") == italic_expression &&
            dynamic_font_italic("other-guid") == ".F.",
        "#842: direct dynamic font-italic assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#842: first dynamic font-italic write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#842: second dynamic font-italic write should remain undo-backed");
    expect(dynamic_font_italic_state() == ".F.,.T.,.F.",
        "#842: dynamic font-italic undo should restore original direct values");

    const std::string constant_expression = ".T.";
    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_font_italic = constant_expression
    });
    expect(italic_result.ok, "#842: dynamic font-italic assignment should support UNIQUEID selectors");
    expect(dynamic_font_italic("customer-guid") == constant_expression &&
            dynamic_font_italic("orders-guid") == constant_expression,
        "#842: direct dynamic font-italic assignment should store raw constant expressions");

    const std::string committed_state = dynamic_font_italic_state();
    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = table_path.string(),
        .objects = {},
        .dynamic_font_italic = ".F."
    });
    expect(!italic_result.ok, "#842: dynamic font-italic assignment should reject empty selections");
    expect(dynamic_font_italic_state() == committed_state,
        "#842: empty-selection failures should not mutate dynamic font-italic expressions");

    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_font_italic = ".F."
    });
    expect(!italic_result.ok, "#842: dynamic font-italic assignment should reject missing selected objects");
    expect(dynamic_font_italic_state() == committed_state,
        "#842: missing-object failures should not mutate dynamic font-italic expressions");

    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .dynamic_font_italic = ".F."
    });
    expect(!italic_result.ok, "#842: dynamic font-italic assignment should reject duplicate selected objects");
    expect(dynamic_font_italic_state() == committed_state,
        "#842: duplicate-selection failures should not mutate dynamic font-italic expressions");

    const fs::path blob_path = temp_dir / "dynamic_font_italic_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "DynamicFontItalic = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoItalic", "no-italic-guid", "Caption = \"No italic\"\r\n"},
        {"txtOther", "other-guid", "DynamicFontItalic = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#842: dynamic font-italic property-blob fixture should be writable");

    const auto blob_dynamic_font_italic_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontItalic"
        });
    };

    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoItalic", .unique_id = {}}
        },
        .dynamic_font_italic = italic_expression
    });
    expect(italic_result.ok, "#842: dynamic font-italic assignment should support existing and absent serialized properties");
    auto blob_italic = blob_dynamic_font_italic_state("blob-guid");
    auto appended_italic = blob_dynamic_font_italic_state("no-italic-guid");
    auto other_italic = blob_dynamic_font_italic_state("other-guid");
    expect(blob_italic.ok && blob_italic.exists && blob_italic.value == italic_expression &&
            appended_italic.ok && appended_italic.exists && appended_italic.value == italic_expression &&
            other_italic.ok && other_italic.exists && other_italic.value == ".T.",
        "#842: serialized dynamic font-italic assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#842: appended serialized dynamic font-italic write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#842: existing serialized dynamic font-italic write should remain undo-backed");
    blob_italic = blob_dynamic_font_italic_state("blob-guid");
    appended_italic = blob_dynamic_font_italic_state("no-italic-guid");
    expect(blob_italic.ok && blob_italic.exists && blob_italic.value == ".F." &&
            appended_italic.ok && !appended_italic.exists,
        "#842: serialized dynamic font-italic undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicfontitalic.scx";
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
    expect(incomplete_create.ok, "#842: missing-DynamicFontItalic fixture should be writable");

    italic_result = copperfin::vfp::set_visual_object_dynamic_font_italic({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_font_italic = ".F."
    });
    expect(!italic_result.ok,
        "#842: dynamic font-italic assignment should reject objects without a writable DynamicFontItalic carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_font_underline_assigns_expression_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_dynamic_font_underline_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "dynamic_font_underline.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DYNAMICFON", .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#843: dynamic font-underline fixture should be writable");

    const auto dynamic_font_underline_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontUnderline"
        });
        expect(result.ok && result.exists, "#843: dynamic font-underline fixture property should be readable");
        return result.value;
    };
    const auto dynamic_font_underline = [&](const std::string& unique_id) {
        return dynamic_font_underline_for(table_path.string(), unique_id);
    };
    const auto dynamic_font_underline_state = [&]() {
        return dynamic_font_underline("customer-guid") + "," +
            dynamic_font_underline("orders-guid") + "," +
            dynamic_font_underline("other-guid");
    };

    const std::string underline_expression = "IIF(RECNO() % 2 = 0, .T., .F.)";
    auto underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .dynamic_font_underline = underline_expression
    });
    expect(underline_result.ok,
        "#843: dynamic font-underline assignment should support object-name and record-index selectors");
    expect(dynamic_font_underline("customer-guid") == underline_expression &&
            dynamic_font_underline("orders-guid") == underline_expression &&
            dynamic_font_underline("other-guid") == ".F.",
        "#843: direct dynamic font-underline assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#843: first dynamic font-underline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#843: second dynamic font-underline write should remain undo-backed");
    expect(dynamic_font_underline_state() == ".F.,.T.,.F.",
        "#843: dynamic font-underline undo should restore original direct values");

    const std::string constant_expression = ".T.";
    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .dynamic_font_underline = constant_expression
    });
    expect(underline_result.ok, "#843: dynamic font-underline assignment should support UNIQUEID selectors");
    expect(dynamic_font_underline("customer-guid") == constant_expression &&
            dynamic_font_underline("orders-guid") == constant_expression,
        "#843: direct dynamic font-underline assignment should store raw constant expressions");

    const std::string committed_state = dynamic_font_underline_state();
    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = table_path.string(),
        .objects = {},
        .dynamic_font_underline = ".F."
    });
    expect(!underline_result.ok, "#843: dynamic font-underline assignment should reject empty selections");
    expect(dynamic_font_underline_state() == committed_state,
        "#843: empty-selection failures should not mutate dynamic font-underline expressions");

    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .dynamic_font_underline = ".F."
    });
    expect(!underline_result.ok, "#843: dynamic font-underline assignment should reject missing selected objects");
    expect(dynamic_font_underline_state() == committed_state,
        "#843: missing-object failures should not mutate dynamic font-underline expressions");

    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .dynamic_font_underline = ".F."
    });
    expect(!underline_result.ok, "#843: dynamic font-underline assignment should reject duplicate selected objects");
    expect(dynamic_font_underline_state() == committed_state,
        "#843: duplicate-selection failures should not mutate dynamic font-underline expressions");

    const fs::path blob_path = temp_dir / "dynamic_font_underline_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "DynamicFontUnderline = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoUnderline", "no-underline-guid", "Caption = \"No underline\"\r\n"},
        {"txtOther", "other-guid", "DynamicFontUnderline = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#843: dynamic font-underline property-blob fixture should be writable");

    const auto blob_dynamic_font_underline_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DynamicFontUnderline"
        });
    };

    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoUnderline", .unique_id = {}}
        },
        .dynamic_font_underline = underline_expression
    });
    expect(underline_result.ok,
        "#843: dynamic font-underline assignment should support existing and absent serialized properties");
    auto blob_underline = blob_dynamic_font_underline_state("blob-guid");
    auto appended_underline = blob_dynamic_font_underline_state("no-underline-guid");
    auto other_underline = blob_dynamic_font_underline_state("other-guid");
    expect(blob_underline.ok && blob_underline.exists && blob_underline.value == underline_expression &&
            appended_underline.ok && appended_underline.exists && appended_underline.value == underline_expression &&
            other_underline.ok && other_underline.exists && other_underline.value == ".T.",
        "#843: serialized dynamic font-underline assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#843: appended serialized dynamic font-underline write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#843: existing serialized dynamic font-underline write should remain undo-backed");
    blob_underline = blob_dynamic_font_underline_state("blob-guid");
    appended_underline = blob_dynamic_font_underline_state("no-underline-guid");
    expect(blob_underline.ok && blob_underline.exists && blob_underline.value == ".F." &&
            appended_underline.ok && !appended_underline.exists,
        "#843: serialized dynamic font-underline undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_dynamicfontunderline.scx";
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
    expect(incomplete_create.ok, "#843: missing-DynamicFontUnderline fixture should be writable");

    underline_result = copperfin::vfp::set_visual_object_dynamic_font_underline({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .dynamic_font_underline = ".F."
    });
    expect(!underline_result.ok,
        "#843: dynamic font-underline assignment should reject objects without a writable DynamicFontUnderline carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_dynamic_font_strikethru_assigns_expression_value() {
    test_dynamic_logical_font_property_assigns_expression_value(
        "#844",
        "dynamic_font_strikethru",
        "DynamicFontStrikethru",
        "DYNAMICFONTSTRIKETHRU",
        "font-strikethru",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_font_strikethru({
                .path = path,
                .objects = objects,
                .dynamic_font_strikethru = expression
            });
        });
}

void test_set_visual_object_dynamic_font_outline_assigns_expression_value() {
    test_dynamic_logical_font_property_assigns_expression_value(
        "#845",
        "dynamic_font_outline",
        "DynamicFontOutline",
        "DYNAMICFONTOUTLINE",
        "font-outline",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_font_outline({
                .path = path,
                .objects = objects,
                .dynamic_font_outline = expression
            });
        });
}

void test_set_visual_object_dynamic_font_shadow_assigns_expression_value() {
    test_dynamic_logical_font_property_assigns_expression_value(
        "#846",
        "dynamic_font_shadow",
        "DynamicFontShadow",
        "DYNAMICFONTSHADOW",
        "font-shadow",
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            const std::string& expression) {
            return copperfin::vfp::set_visual_object_dynamic_font_shadow({
                .path = path,
                .objects = objects,
                .dynamic_font_shadow = expression
            });
        });
}

}  // namespace cf_test_visual_asset_editor
