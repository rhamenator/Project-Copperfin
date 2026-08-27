// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_visual_asset_editor_support.h"

namespace cf_test_visual_asset_editor {
#include "test_visual_asset_editor_setters_data_binding.inl"
#include "test_visual_asset_editor_setters_row_source.inl"
void test_set_visual_object_bound_column_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_bound_column_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "bound_column.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "BOUNDCOLUM", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "1"},
        {"lstOrders", "ordersList", "orders-guid", "2"},
        {"cboOther", "otherCombo", "other-guid", "3"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#807: bound-column fixture should be writable");

    const auto bound_column_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "BoundColumn"
        });
        expect(result.ok && result.exists, "#807: bound-column fixture property should be readable");
        return result.value;
    };
    const auto bound_column = [&](const std::string& unique_id) {
        return bound_column_for(table_path.string(), unique_id);
    };
    const auto bound_column_state = [&]() {
        return bound_column("customer-guid") + "," +
            bound_column("orders-guid") + "," +
            bound_column("other-guid");
    };

    auto column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .bound_column = 4
    });
    expect(column_result.ok, "#807: bound-column assignment should support object-name and record-index selectors");
    expect(bound_column("customer-guid") == "4" &&
            bound_column("orders-guid") == "4" &&
            bound_column("other-guid") == "3",
        "#807: direct bound-column assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#807: first bound-column write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#807: second bound-column write should remain undo-backed");
    expect(bound_column_state() == "1,2,3", "#807: bound-column undo should restore original direct values");

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .bound_column = 0
    });
    expect(column_result.ok, "#807: bound-column assignment should support UNIQUEID selectors");
    expect(bound_column("customer-guid") == "0" &&
            bound_column("orders-guid") == "0",
        "#807: direct bound-column assignment should store unquoted numeric values");

    const std::string committed_state = bound_column_state();
    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {},
        .bound_column = 4
    });
    expect(!column_result.ok, "#807: bound-column assignment should reject empty selections");
    expect(bound_column_state() == committed_state, "#807: empty-selection failures should not mutate bound columns");

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .bound_column = -1
    });
    expect(!column_result.ok, "#807: bound-column assignment should reject negative values");
    expect(bound_column_state() == committed_state, "#807: negative-value failures should not mutate bound columns");

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .bound_column = 4
    });
    expect(!column_result.ok, "#807: bound-column assignment should reject missing selected objects");
    expect(bound_column_state() == committed_state, "#807: missing-object failures should not mutate bound columns");

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .bound_column = 4
    });
    expect(!column_result.ok, "#807: bound-column assignment should reject duplicate selected objects");
    expect(bound_column_state() == committed_state, "#807: duplicate-selection failures should not mutate bound columns");

    const fs::path blob_path = temp_dir / "bound_column_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "BoundColumn = 1\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColumn", "no-column-guid", "Caption = \"No column\"\r\n"},
        {"cboOther", "other-guid", "BoundColumn = 3\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#807: bound-column property-blob fixture should be writable");

    const auto blob_bound_column_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "BoundColumn"
        });
    };

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColumn", .unique_id = {}}
        },
        .bound_column = 2
    });
    expect(column_result.ok, "#807: bound-column assignment should support existing and absent serialized properties");
    auto blob_column = blob_bound_column_state("blob-guid");
    auto appended_column = blob_bound_column_state("no-column-guid");
    auto other_column = blob_bound_column_state("other-guid");
    expect(blob_column.ok && blob_column.exists && blob_column.value == "2" &&
            appended_column.ok && appended_column.exists && appended_column.value == "2" &&
            other_column.ok && other_column.exists && other_column.value == "3",
        "#807: serialized bound-column assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#807: appended serialized bound-column write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#807: existing serialized bound-column write should remain undo-backed");
    blob_column = blob_bound_column_state("blob-guid");
    appended_column = blob_bound_column_state("no-column-guid");
    expect(blob_column.ok && blob_column.exists && blob_column.value == "1" &&
            appended_column.ok && !appended_column.exists,
        "#807: serialized bound-column undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_boundcolumn.scx";
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
    expect(incomplete_create.ok, "#807: missing-BoundColumn fixture should be writable");

    column_result = copperfin::vfp::set_visual_object_bound_column({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .bound_column = 2
    });
    expect(!column_result.ok, "#807: bound-column assignment should reject objects without a writable BoundColumn carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_button_count_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#881",
        "button_count",
        "ButtonCount",
        "BUTTONCOUNT",
        "button-count",
        2,
        3,
        1,
        4,
        0,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_button_count({
                .path = path,
                .objects = objects,
                .button_count = value
            });
        });
}

void test_set_visual_object_column_count_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_column_count_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "column_count.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "COLUMNCOUN", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "2"},
        {"lstOrders", "ordersList", "orders-guid", "3"},
        {"cboOther", "otherCombo", "other-guid", "4"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#808: column-count fixture should be writable");

    const auto column_count_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnCount"
        });
        expect(result.ok && result.exists, "#808: column-count fixture property should be readable");
        return result.value;
    };
    const auto column_count = [&](const std::string& unique_id) {
        return column_count_for(table_path.string(), unique_id);
    };
    const auto column_count_state = [&]() {
        return column_count("customer-guid") + "," +
            column_count("orders-guid") + "," +
            column_count("other-guid");
    };

    auto count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .column_count = 5
    });
    expect(count_result.ok, "#808: column-count assignment should support object-name and record-index selectors");
    expect(column_count("customer-guid") == "5" &&
            column_count("orders-guid") == "5" &&
            column_count("other-guid") == "4",
        "#808: direct column-count assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#808: first column-count write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#808: second column-count write should remain undo-backed");
    expect(column_count_state() == "2,3,4", "#808: column-count undo should restore original direct values");

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .column_count = 1
    });
    expect(count_result.ok, "#808: column-count assignment should support UNIQUEID selectors");
    expect(column_count("customer-guid") == "1" &&
            column_count("orders-guid") == "1",
        "#808: direct column-count assignment should store unquoted numeric values");

    const std::string committed_state = column_count_state();
    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {},
        .column_count = 4
    });
    expect(!count_result.ok, "#808: column-count assignment should reject empty selections");
    expect(column_count_state() == committed_state, "#808: empty-selection failures should not mutate column counts");

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .column_count = -1
    });
    expect(!count_result.ok, "#808: column-count assignment should reject negative values");
    expect(column_count_state() == committed_state, "#808: negative-value failures should not mutate column counts");

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .column_count = 4
    });
    expect(!count_result.ok, "#808: column-count assignment should reject missing selected objects");
    expect(column_count_state() == committed_state, "#808: missing-object failures should not mutate column counts");

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .column_count = 4
    });
    expect(!count_result.ok, "#808: column-count assignment should reject duplicate selected objects");
    expect(column_count_state() == committed_state, "#808: duplicate-selection failures should not mutate column counts");

    const fs::path blob_path = temp_dir / "column_count_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ColumnCount = 2\r\nCaption = \"Customer\"\r\n"},
        {"cboNoCount", "no-count-guid", "Caption = \"No count\"\r\n"},
        {"cboOther", "other-guid", "ColumnCount = 4\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#808: column-count property-blob fixture should be writable");

    const auto blob_column_count_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnCount"
        });
    };

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoCount", .unique_id = {}}
        },
        .column_count = 3
    });
    expect(count_result.ok, "#808: column-count assignment should support existing and absent serialized properties");
    auto blob_count = blob_column_count_state("blob-guid");
    auto appended_count = blob_column_count_state("no-count-guid");
    auto other_count = blob_column_count_state("other-guid");
    expect(blob_count.ok && blob_count.exists && blob_count.value == "3" &&
            appended_count.ok && appended_count.exists && appended_count.value == "3" &&
            other_count.ok && other_count.exists && other_count.value == "4",
        "#808: serialized column-count assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#808: appended serialized column-count write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#808: existing serialized column-count write should remain undo-backed");
    blob_count = blob_column_count_state("blob-guid");
    appended_count = blob_column_count_state("no-count-guid");
    expect(blob_count.ok && blob_count.exists && blob_count.value == "2" &&
            appended_count.ok && !appended_count.exists,
        "#808: serialized column-count undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_columncount.scx";
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
    expect(incomplete_create.ok, "#808: missing-ColumnCount fixture should be writable");

    count_result = copperfin::vfp::set_visual_object_column_count({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .column_count = 2
    });
    expect(!count_result.ok, "#808: column-count assignment should reject objects without a writable ColumnCount carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_column_widths_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_column_widths_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "column_widths.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "COLUMNWIDT", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "75,125"},
        {"lstOrders", "ordersList", "orders-guid", "60,80,100"},
        {"cboOther", "otherCombo", "other-guid", "90,90"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#809: column-widths fixture should be writable");

    const auto column_widths_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnWidths"
        });
        expect(result.ok && result.exists, "#809: column-widths fixture property should be readable");
        return result.value;
    };
    const auto column_widths = [&](const std::string& unique_id) {
        return column_widths_for(table_path.string(), unique_id);
    };
    const auto column_widths_state = [&]() {
        return column_widths("customer-guid") + ";" +
            column_widths("orders-guid") + ";" +
            column_widths("other-guid");
    };

    auto widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .column_widths = "40,90,120"
    });
    expect(widths_result.ok, "#809: column-widths assignment should support object-name and record-index selectors");
    expect(column_widths("customer-guid") == "40,90,120" &&
            column_widths("orders-guid") == "40,90,120" &&
            column_widths("other-guid") == "90,90",
        "#809: direct column-widths assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#809: first column-widths write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#809: second column-widths write should remain undo-backed");
    expect(column_widths_state() == "75,125;60,80,100;90,90",
        "#809: column-widths undo should restore original direct values");

    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .column_widths = "10,20"
    });
    expect(widths_result.ok, "#809: column-widths assignment should support UNIQUEID selectors");
    expect(column_widths("customer-guid") == "10,20" &&
            column_widths("orders-guid") == "10,20",
        "#809: direct column-widths assignment should store caller text without serialized quoting");

    const std::string committed_state = column_widths_state();
    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = table_path.string(),
        .objects = {},
        .column_widths = "Ignored"
    });
    expect(!widths_result.ok, "#809: column-widths assignment should reject empty selections");
    expect(column_widths_state() == committed_state, "#809: empty-selection failures should not mutate column widths");

    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .column_widths = "Ignored"
    });
    expect(!widths_result.ok, "#809: column-widths assignment should reject missing selected objects");
    expect(column_widths_state() == committed_state, "#809: missing-object failures should not mutate column widths");

    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .column_widths = "Ignored"
    });
    expect(!widths_result.ok, "#809: column-widths assignment should reject duplicate selected objects");
    expect(column_widths_state() == committed_state, "#809: duplicate-selection failures should not mutate column widths");

    const fs::path blob_path = temp_dir / "column_widths_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ColumnWidths = \"75,125\"\r\nCaption = \"Customer\"\r\n"},
        {"cboNoWidths", "no-widths-guid", "Caption = \"No widths\"\r\n"},
        {"cboOther", "other-guid", "ColumnWidths = \"90,90\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#809: column-widths property-blob fixture should be writable");

    const auto blob_column_widths_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnWidths"
        });
    };

    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoWidths", .unique_id = {}}
        },
        .column_widths = "40,\"Auto\",120"
    });
    expect(widths_result.ok, "#809: column-widths assignment should support existing and absent serialized properties");
    auto blob_widths = blob_column_widths_state("blob-guid");
    auto appended_widths = blob_column_widths_state("no-widths-guid");
    auto other_widths = blob_column_widths_state("other-guid");
    expect(blob_widths.ok && blob_widths.exists && blob_widths.value == "\"40,\"\"Auto\"\",120\"" &&
            appended_widths.ok && appended_widths.exists && appended_widths.value == "\"40,\"\"Auto\"\",120\"" &&
            other_widths.ok && other_widths.exists && other_widths.value == "\"90,90\"",
        "#809: serialized column-widths assignment should quote text, append missing ColumnWidths, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#809: appended serialized column-widths write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#809: existing serialized column-widths write should remain undo-backed");
    blob_widths = blob_column_widths_state("blob-guid");
    appended_widths = blob_column_widths_state("no-widths-guid");
    expect(blob_widths.ok && blob_widths.exists && blob_widths.value == "\"75,125\"" &&
            appended_widths.ok && !appended_widths.exists,
        "#809: serialized column-widths undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_columnwidths.scx";
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
    expect(incomplete_create.ok, "#809: missing-ColumnWidths fixture should be writable");

    widths_result = copperfin::vfp::set_visual_object_column_widths({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .column_widths = "Ignored"
    });
    expect(!widths_result.ok, "#809: column-widths assignment should reject objects without a writable ColumnWidths carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_column_lines_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_column_lines_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "column_lines.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "COLUMNLINE", .type = 'C', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", ".F."},
        {"lstOrders", "ordersList", "orders-guid", ".F."},
        {"cboOther", "otherCombo", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#810: column-lines fixture should be writable");

    const auto column_lines_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnLines"
        });
        expect(result.ok && result.exists, "#810: column-lines fixture property should be readable");
        return result.value;
    };
    const auto column_lines = [&](const std::string& unique_id) {
        return column_lines_for(table_path.string(), unique_id);
    };
    const auto column_lines_state = [&]() {
        return column_lines("customer-guid") + "," +
            column_lines("orders-guid") + "," +
            column_lines("other-guid");
    };

    auto lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .column_lines = true
    });
    expect(lines_result.ok, "#810: column-lines assignment should support object-name and record-index selectors");
    expect(column_lines("customer-guid") == ".T." &&
            column_lines("orders-guid") == ".T." &&
            column_lines("other-guid") == ".T.",
        "#810: direct column-lines assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#810: first column-lines write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#810: second column-lines write should remain undo-backed");
    expect(column_lines_state() == ".F.,.F.,.T.", "#810: column-lines undo should restore original direct values");

    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .column_lines = false
    });
    expect(lines_result.ok, "#810: column-lines assignment should support UNIQUEID selectors");
    expect(column_lines("customer-guid") == ".F." &&
            column_lines("orders-guid") == ".F.",
        "#810: direct column-lines assignment should store false FoxPro logical values");

    const std::string committed_state = column_lines_state();
    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = table_path.string(),
        .objects = {},
        .column_lines = true
    });
    expect(!lines_result.ok, "#810: column-lines assignment should reject empty selections");
    expect(column_lines_state() == committed_state, "#810: empty-selection failures should not mutate column lines");

    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .column_lines = true
    });
    expect(!lines_result.ok, "#810: column-lines assignment should reject missing selected objects");
    expect(column_lines_state() == committed_state, "#810: missing-object failures should not mutate column lines");

    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .column_lines = true
    });
    expect(!lines_result.ok, "#810: column-lines assignment should reject duplicate selected objects");
    expect(column_lines_state() == committed_state, "#810: duplicate-selection failures should not mutate column lines");

    const fs::path blob_path = temp_dir / "column_lines_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ColumnLines = .F.\r\nCaption = \"Customer\"\r\n"},
        {"cboNoLines", "no-lines-guid", "Caption = \"No lines\"\r\n"},
        {"cboOther", "other-guid", "ColumnLines = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#810: column-lines property-blob fixture should be writable");

    const auto blob_column_lines_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ColumnLines"
        });
    };

    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoLines", .unique_id = {}}
        },
        .column_lines = true
    });
    expect(lines_result.ok, "#810: column-lines assignment should support existing and absent serialized properties");
    auto blob_lines = blob_column_lines_state("blob-guid");
    auto appended_lines = blob_column_lines_state("no-lines-guid");
    auto other_lines = blob_column_lines_state("other-guid");
    expect(blob_lines.ok && blob_lines.exists && blob_lines.value == ".T." &&
            appended_lines.ok && appended_lines.exists && appended_lines.value == ".T." &&
            other_lines.ok && other_lines.exists && other_lines.value == ".T.",
        "#810: serialized column-lines assignment should write FoxPro logical values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#810: appended serialized column-lines write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#810: existing serialized column-lines write should remain undo-backed");
    blob_lines = blob_column_lines_state("blob-guid");
    appended_lines = blob_column_lines_state("no-lines-guid");
    expect(blob_lines.ok && blob_lines.exists && blob_lines.value == ".F." &&
            appended_lines.ok && !appended_lines.exists,
        "#810: serialized column-lines undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_columnlines.scx";
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
    expect(incomplete_create.ok, "#810: missing-ColumnLines fixture should be writable");

    lines_result = copperfin::vfp::set_visual_object_column_lines({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .column_lines = true
    });
    expect(!lines_result.ok, "#810: column-lines assignment should reject objects without a writable ColumnLines carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_integral_height_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_integral_height_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "integral_height.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "INTEGRALHE", .type = 'C', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", ".F."},
        {"lstOrders", "ordersList", "orders-guid", ".F."},
        {"cboOther", "otherCombo", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#811: integral-height fixture should be writable");

    const auto integral_height_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "IntegralHeight"
        });
        expect(result.ok && result.exists, "#811: integral-height fixture property should be readable");
        return result.value;
    };
    const auto integral_height = [&](const std::string& unique_id) {
        return integral_height_for(table_path.string(), unique_id);
    };
    const auto integral_height_state = [&]() {
        return integral_height("customer-guid") + "," +
            integral_height("orders-guid") + "," +
            integral_height("other-guid");
    };

    auto height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .integral_height = true
    });
    expect(height_result.ok, "#811: integral-height assignment should support object-name and record-index selectors");
    expect(integral_height("customer-guid") == ".T." &&
            integral_height("orders-guid") == ".T." &&
            integral_height("other-guid") == ".T.",
        "#811: direct integral-height assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#811: first integral-height write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#811: second integral-height write should remain undo-backed");
    expect(integral_height_state() == ".F.,.F.,.T.", "#811: integral-height undo should restore original direct values");

    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .integral_height = false
    });
    expect(height_result.ok, "#811: integral-height assignment should support UNIQUEID selectors");
    expect(integral_height("customer-guid") == ".F." &&
            integral_height("orders-guid") == ".F.",
        "#811: direct integral-height assignment should store false FoxPro logical values");

    const std::string committed_state = integral_height_state();
    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = table_path.string(),
        .objects = {},
        .integral_height = true
    });
    expect(!height_result.ok, "#811: integral-height assignment should reject empty selections");
    expect(integral_height_state() == committed_state, "#811: empty-selection failures should not mutate integral-height values");

    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .integral_height = true
    });
    expect(!height_result.ok, "#811: integral-height assignment should reject missing selected objects");
    expect(integral_height_state() == committed_state, "#811: missing-object failures should not mutate integral-height values");

    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .integral_height = true
    });
    expect(!height_result.ok, "#811: integral-height assignment should reject duplicate selected objects");
    expect(integral_height_state() == committed_state, "#811: duplicate-selection failures should not mutate integral-height values");

    const fs::path blob_path = temp_dir / "integral_height_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "IntegralHeight = .F.\r\nCaption = \"Customer\"\r\n"},
        {"cboNoHeight", "no-height-guid", "Caption = \"No height\"\r\n"},
        {"cboOther", "other-guid", "IntegralHeight = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#811: integral-height property-blob fixture should be writable");

    const auto blob_integral_height_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "IntegralHeight"
        });
    };

    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoHeight", .unique_id = {}}
        },
        .integral_height = true
    });
    expect(height_result.ok, "#811: integral-height assignment should support existing and absent serialized properties");
    auto blob_height = blob_integral_height_state("blob-guid");
    auto appended_height = blob_integral_height_state("no-height-guid");
    auto other_height = blob_integral_height_state("other-guid");
    expect(blob_height.ok && blob_height.exists && blob_height.value == ".T." &&
            appended_height.ok && appended_height.exists && appended_height.value == ".T." &&
            other_height.ok && other_height.exists && other_height.value == ".T.",
        "#811: serialized integral-height assignment should write FoxPro logical values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#811: appended serialized integral-height write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#811: existing serialized integral-height write should remain undo-backed");
    blob_height = blob_integral_height_state("blob-guid");
    appended_height = blob_integral_height_state("no-height-guid");
    expect(blob_height.ok && blob_height.exists && blob_height.value == ".F." &&
            appended_height.ok && !appended_height.exists,
        "#811: serialized integral-height undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_integralheight.scx";
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
    expect(incomplete_create.ok, "#811: missing-IntegralHeight fixture should be writable");

    height_result = copperfin::vfp::set_visual_object_integral_height({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .integral_height = true
    });
    expect(!height_result.ok, "#811: integral-height assignment should reject objects without a writable IntegralHeight carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_incremental_search_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_incremental_search_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "incremental_search.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "INCREMENTA", .type = 'C', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", ".F."},
        {"lstOrders", "ordersList", "orders-guid", ".F."},
        {"cboOther", "otherCombo", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#812: incremental-search fixture should be writable");

    const auto incremental_search_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "IncrementalSearch"
        });
        expect(result.ok && result.exists, "#812: incremental-search fixture property should be readable");
        return result.value;
    };
    const auto incremental_search = [&](const std::string& unique_id) {
        return incremental_search_for(table_path.string(), unique_id);
    };
    const auto incremental_search_state = [&]() {
        return incremental_search("customer-guid") + "," +
            incremental_search("orders-guid") + "," +
            incremental_search("other-guid");
    };

    auto search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .incremental_search = true
    });
    expect(search_result.ok, "#812: incremental-search assignment should support object-name and record-index selectors");
    expect(incremental_search("customer-guid") == ".T." &&
            incremental_search("orders-guid") == ".T." &&
            incremental_search("other-guid") == ".T.",
        "#812: direct incremental-search assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#812: first incremental-search write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#812: second incremental-search write should remain undo-backed");
    expect(incremental_search_state() == ".F.,.F.,.T.", "#812: incremental-search undo should restore original direct values");

    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .incremental_search = false
    });
    expect(search_result.ok, "#812: incremental-search assignment should support UNIQUEID selectors");
    expect(incremental_search("customer-guid") == ".F." &&
            incremental_search("orders-guid") == ".F.",
        "#812: direct incremental-search assignment should store false FoxPro logical values");

    const std::string committed_state = incremental_search_state();
    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = table_path.string(),
        .objects = {},
        .incremental_search = true
    });
    expect(!search_result.ok, "#812: incremental-search assignment should reject empty selections");
    expect(incremental_search_state() == committed_state, "#812: empty-selection failures should not mutate incremental-search values");

    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .incremental_search = true
    });
    expect(!search_result.ok, "#812: incremental-search assignment should reject missing selected objects");
    expect(incremental_search_state() == committed_state, "#812: missing-object failures should not mutate incremental-search values");

    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .incremental_search = true
    });
    expect(!search_result.ok, "#812: incremental-search assignment should reject duplicate selected objects");
    expect(incremental_search_state() == committed_state, "#812: duplicate-selection failures should not mutate incremental-search values");

    const fs::path blob_path = temp_dir / "incremental_search_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "IncrementalSearch = .F.\r\nCaption = \"Customer\"\r\n"},
        {"cboNoSearch", "no-search-guid", "Caption = \"No search\"\r\n"},
        {"cboOther", "other-guid", "IncrementalSearch = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#812: incremental-search property-blob fixture should be writable");

    const auto blob_incremental_search_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "IncrementalSearch"
        });
    };

    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoSearch", .unique_id = {}}
        },
        .incremental_search = true
    });
    expect(search_result.ok, "#812: incremental-search assignment should support existing and absent serialized properties");
    auto blob_search = blob_incremental_search_state("blob-guid");
    auto appended_search = blob_incremental_search_state("no-search-guid");
    auto other_search = blob_incremental_search_state("other-guid");
    expect(blob_search.ok && blob_search.exists && blob_search.value == ".T." &&
            appended_search.ok && appended_search.exists && appended_search.value == ".T." &&
            other_search.ok && other_search.exists && other_search.value == ".T.",
        "#812: serialized incremental-search assignment should write FoxPro logical values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#812: appended serialized incremental-search write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#812: existing serialized incremental-search write should remain undo-backed");
    blob_search = blob_incremental_search_state("blob-guid");
    appended_search = blob_incremental_search_state("no-search-guid");
    expect(blob_search.ok && blob_search.exists && blob_search.value == ".F." &&
            appended_search.ok && !appended_search.exists,
        "#812: serialized incremental-search undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_incrementalsearch.scx";
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
    expect(incomplete_create.ok, "#812: missing-IncrementalSearch fixture should be writable");

    search_result = copperfin::vfp::set_visual_object_incremental_search({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .incremental_search = true
    });
    expect(!search_result.ok, "#812: incremental-search assignment should reject objects without a writable IncrementalSearch carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_multi_select_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_multi_select_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "multi_select.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "MULTISELEC", .type = 'C', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", ".F."},
        {"lstOrders", "ordersList", "orders-guid", ".F."},
        {"lstOther", "otherList", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#813: multi-select fixture should be writable");

    const auto multi_select_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "MultiSelect"
        });
        expect(result.ok && result.exists, "#813: multi-select fixture property should be readable");
        return result.value;
    };
    const auto multi_select = [&](const std::string& unique_id) {
        return multi_select_for(table_path.string(), unique_id);
    };
    const auto multi_select_state = [&]() {
        return multi_select("customer-guid") + "," +
            multi_select("orders-guid") + "," +
            multi_select("other-guid");
    };

    auto select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .multi_select = true
    });
    expect(select_result.ok, "#813: multi-select assignment should support object-name and record-index selectors");
    expect(multi_select("customer-guid") == ".T." &&
            multi_select("orders-guid") == ".T." &&
            multi_select("other-guid") == ".T.",
        "#813: direct multi-select assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#813: first multi-select write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#813: second multi-select write should remain undo-backed");
    expect(multi_select_state() == ".F.,.F.,.T.", "#813: multi-select undo should restore original direct values");

    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .multi_select = false
    });
    expect(select_result.ok, "#813: multi-select assignment should support UNIQUEID selectors");
    expect(multi_select("customer-guid") == ".F." &&
            multi_select("orders-guid") == ".F.",
        "#813: direct multi-select assignment should store false FoxPro logical values");

    const std::string committed_state = multi_select_state();
    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = table_path.string(),
        .objects = {},
        .multi_select = true
    });
    expect(!select_result.ok, "#813: multi-select assignment should reject empty selections");
    expect(multi_select_state() == committed_state, "#813: empty-selection failures should not mutate multi-select values");

    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .multi_select = true
    });
    expect(!select_result.ok, "#813: multi-select assignment should reject missing selected objects");
    expect(multi_select_state() == committed_state, "#813: missing-object failures should not mutate multi-select values");

    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .multi_select = true
    });
    expect(!select_result.ok, "#813: multi-select assignment should reject duplicate selected objects");
    expect(multi_select_state() == committed_state, "#813: duplicate-selection failures should not mutate multi-select values");

    const fs::path blob_path = temp_dir / "multi_select_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"lstBlob", "blob-guid", "MultiSelect = .F.\r\nCaption = \"Customer\"\r\n"},
        {"lstNoSelect", "no-select-guid", "Caption = \"No select\"\r\n"},
        {"lstOther", "other-guid", "MultiSelect = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#813: multi-select property-blob fixture should be writable");

    const auto blob_multi_select_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "MultiSelect"
        });
    };

    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "lstNoSelect", .unique_id = {}}
        },
        .multi_select = true
    });
    expect(select_result.ok, "#813: multi-select assignment should support existing and absent serialized properties");
    auto blob_select = blob_multi_select_state("blob-guid");
    auto appended_select = blob_multi_select_state("no-select-guid");
    auto other_select = blob_multi_select_state("other-guid");
    expect(blob_select.ok && blob_select.exists && blob_select.value == ".T." &&
            appended_select.ok && appended_select.exists && appended_select.value == ".T." &&
            other_select.ok && other_select.exists && other_select.value == ".T.",
        "#813: serialized multi-select assignment should write FoxPro logical values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#813: appended serialized multi-select write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#813: existing serialized multi-select write should remain undo-backed");
    blob_select = blob_multi_select_state("blob-guid");
    appended_select = blob_multi_select_state("no-select-guid");
    expect(blob_select.ok && blob_select.exists && blob_select.value == ".F." &&
            appended_select.ok && !appended_select.exists,
        "#813: serialized multi-select undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_multiselect.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> incomplete_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U}
    };
    const std::vector<std::vector<std::string>> incomplete_records{
        {"lstA", "a-guid"}
    };
    const auto incomplete_create = copperfin::vfp::create_dbf_table_file(
        incomplete_path.string(),
        incomplete_fields,
        incomplete_records);
    expect(incomplete_create.ok, "#813: missing-MultiSelect fixture should be writable");

    select_result = copperfin::vfp::set_visual_object_multi_select({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .multi_select = true
    });
    expect(!select_result.ok, "#813: multi-select assignment should reject objects without a writable MultiSelect carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_list_index_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_list_index_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "list_index.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "LISTINDEX", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "0"},
        {"lstOrders", "ordersList", "orders-guid", "1"},
        {"cboOther", "otherCombo", "other-guid", "2"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#815: list-index fixture should be writable");

    const auto list_index_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ListIndex"
        });
        expect(result.ok && result.exists, "#815: list-index fixture property should be readable");
        return result.value;
    };
    const auto list_index = [&](const std::string& unique_id) {
        return list_index_for(table_path.string(), unique_id);
    };
    const auto list_index_state = [&]() {
        return list_index("customer-guid") + "," +
            list_index("orders-guid") + "," +
            list_index("other-guid");
    };

    auto index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .list_index = 3
    });
    expect(index_result.ok, "#815: list-index assignment should support object-name and record-index selectors");
    expect(list_index("customer-guid") == "3" &&
            list_index("orders-guid") == "3" &&
            list_index("other-guid") == "2",
        "#815: direct list-index assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#815: first list-index write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#815: second list-index write should remain undo-backed");
    expect(list_index_state() == "0,1,2", "#815: list-index undo should restore original direct values");

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .list_index = 0
    });
    expect(index_result.ok, "#815: list-index assignment should support UNIQUEID selectors");
    expect(list_index("customer-guid") == "0" &&
            list_index("orders-guid") == "0",
        "#815: direct list-index assignment should store zero as an unquoted numeric value");

    const std::string committed_state = list_index_state();
    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {},
        .list_index = 1
    });
    expect(!index_result.ok, "#815: list-index assignment should reject empty selections");
    expect(list_index_state() == committed_state, "#815: empty-selection failures should not mutate list indexes");

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .list_index = -1
    });
    expect(!index_result.ok, "#815: list-index assignment should reject negative values");
    expect(list_index_state() == committed_state, "#815: negative-value failures should not mutate list indexes");

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .list_index = 1
    });
    expect(!index_result.ok, "#815: list-index assignment should reject missing selected objects");
    expect(list_index_state() == committed_state, "#815: missing-object failures should not mutate list indexes");

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .list_index = 1
    });
    expect(!index_result.ok, "#815: list-index assignment should reject duplicate selected objects");
    expect(list_index_state() == committed_state, "#815: duplicate-selection failures should not mutate list indexes");

    const fs::path blob_path = temp_dir / "list_index_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "ListIndex = 0\r\nCaption = \"Customer\"\r\n"},
        {"cboNoIndex", "no-index-guid", "Caption = \"No index\"\r\n"},
        {"cboOther", "other-guid", "ListIndex = 2\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#815: list-index property-blob fixture should be writable");

    const auto blob_list_index_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "ListIndex"
        });
    };

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoIndex", .unique_id = {}}
        },
        .list_index = 4
    });
    expect(index_result.ok, "#815: list-index assignment should support existing and absent serialized properties");
    auto blob_index = blob_list_index_state("blob-guid");
    auto appended_index = blob_list_index_state("no-index-guid");
    auto other_index = blob_list_index_state("other-guid");
    expect(blob_index.ok && blob_index.exists && blob_index.value == "4" &&
            appended_index.ok && appended_index.exists && appended_index.value == "4" &&
            other_index.ok && other_index.exists && other_index.value == "2",
        "#815: serialized list-index assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#815: appended serialized list-index write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#815: existing serialized list-index write should remain undo-backed");
    blob_index = blob_list_index_state("blob-guid");
    appended_index = blob_list_index_state("no-index-guid");
    expect(blob_index.ok && blob_index.exists && blob_index.value == "0" &&
            appended_index.ok && !appended_index.exists,
        "#815: serialized list-index undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_listindex.scx";
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
    expect(incomplete_create.ok, "#815: missing-ListIndex fixture should be writable");

    index_result = copperfin::vfp::set_visual_object_list_index({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .list_index = 1
    });
    expect(!index_result.ok, "#815: list-index assignment should reject objects without a writable ListIndex carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_left_column_assigns_numeric_value() {
    test_visual_object_non_negative_numeric_property_assigns_value(
        "#948",
        "left_column",
        "LeftColumn",
        "LEFTCOLUMN",
        "left-column",
        0,
        1,
        42,
        7,
        99,
        [](const std::string& path,
            const std::vector<copperfin::vfp::VisualObjectAlignmentTarget>& objects,
            int value) {
            return copperfin::vfp::set_visual_object_left_column({
                .path = path,
                .objects = objects,
                .left_column = value
            });
        });
}

void test_set_visual_object_display_value_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_display_value_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "display_value.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "DISPLAYVAL", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "Alice"},
        {"lstOrders", "ordersList", "orders-guid", "Order 100"},
        {"cboOther", "otherCombo", "other-guid", "Other"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#816: display-value fixture should be writable");

    const auto display_value_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisplayValue"
        });
        expect(result.ok && result.exists, "#816: display-value fixture property should be readable");
        return result.value;
    };
    const auto display_value = [&](const std::string& unique_id) {
        return display_value_for(table_path.string(), unique_id);
    };
    const auto display_value_state = [&]() {
        return display_value("customer-guid") + "," +
            display_value("orders-guid") + "," +
            display_value("other-guid");
    };

    auto display_result = copperfin::vfp::set_visual_object_display_value({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .display_value = "Bob"
    });
    expect(display_result.ok, "#816: display-value assignment should support object-name and record-index selectors");
    expect(display_value("customer-guid") == "Bob" &&
            display_value("orders-guid") == "Bob" &&
            display_value("other-guid") == "Other",
        "#816: direct display-value assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#816: first display-value write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#816: second display-value write should remain undo-backed");
    expect(display_value_state() == "Alice,Order 100,Other",
        "#816: display-value undo should restore original direct values");

    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .display_value = "Selected"
    });
    expect(display_result.ok, "#816: display-value assignment should support UNIQUEID selectors");
    expect(display_value("customer-guid") == "Selected" &&
            display_value("orders-guid") == "Selected",
        "#816: direct display-value assignment should store caller text without serialized quoting");

    const std::string committed_state = display_value_state();
    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = table_path.string(),
        .objects = {},
        .display_value = "Ignored"
    });
    expect(!display_result.ok, "#816: display-value assignment should reject empty selections");
    expect(display_value_state() == committed_state, "#816: empty-selection failures should not mutate display values");

    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .display_value = "Ignored"
    });
    expect(!display_result.ok, "#816: display-value assignment should reject missing selected objects");
    expect(display_value_state() == committed_state, "#816: missing-object failures should not mutate display values");

    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .display_value = "Ignored"
    });
    expect(!display_result.ok, "#816: display-value assignment should reject duplicate selected objects");
    expect(display_value_state() == committed_state, "#816: duplicate-selection failures should not mutate display values");

    const fs::path blob_path = temp_dir / "display_value_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "DisplayValue = \"Alice\"\r\nCaption = \"Customer\"\r\n"},
        {"cboNoDisplay", "no-display-guid", "Caption = \"No display\"\r\n"},
        {"cboOther", "other-guid", "DisplayValue = \"Other\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#816: display-value property-blob fixture should be writable");

    const auto blob_display_value_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "DisplayValue"
        });
    };

    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoDisplay", .unique_id = {}}
        },
        .display_value = "Bob \"B\""
    });
    expect(display_result.ok, "#816: display-value assignment should support existing and absent serialized properties");
    auto blob_display = blob_display_value_state("blob-guid");
    auto appended_display = blob_display_value_state("no-display-guid");
    auto other_display = blob_display_value_state("other-guid");
    expect(blob_display.ok && blob_display.exists && blob_display.value == "\"Bob \"\"B\"\"\"" &&
            appended_display.ok && appended_display.exists && appended_display.value == "\"Bob \"\"B\"\"\"" &&
            other_display.ok && other_display.exists && other_display.value == "\"Other\"",
        "#816: serialized display-value assignment should quote text, append missing DisplayValue, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#816: appended serialized display-value write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#816: existing serialized display-value write should remain undo-backed");
    blob_display = blob_display_value_state("blob-guid");
    appended_display = blob_display_value_state("no-display-guid");
    expect(blob_display.ok && blob_display.exists && blob_display.value == "\"Alice\"" &&
            appended_display.ok && !appended_display.exists,
        "#816: serialized display-value undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_displayvalue.scx";
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
    expect(incomplete_create.ok, "#816: missing-DisplayValue fixture should be writable");

    display_result = copperfin::vfp::set_visual_object_display_value({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .display_value = "Ignored"
    });
    expect(!display_result.ok, "#816: display-value assignment should reject objects without a writable DisplayValue carrier");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace cf_test_visual_asset_editor
