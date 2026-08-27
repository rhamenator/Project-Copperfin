void test_set_visual_object_row_source_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_rowsource_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rowsource.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "ROWSOURCE", .type = 'C', .length = 80U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "customers.name,customer_id"},
        {"lstOrders", "ordersList", "orders-guid", "orders.order_id,total"},
        {"cboOther", "otherCombo", "other-guid", "states.name"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#805: row-source fixture should be writable");

    const auto row_source_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "RowSource"
        });
        expect(result.ok && result.exists, "#805: row-source fixture property should be readable");
        return result.value;
    };
    const auto row_source = [&](const std::string& unique_id) {
        return row_source_for(table_path.string(), unique_id);
    };
    const auto row_source_state = [&]() {
        return row_source("customer-guid") + "," +
            row_source("orders-guid") + "," +
            row_source("other-guid");
    };

    auto row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .row_source = "products.name,product_id"
    });
    expect(row_source_result.ok, "#805: row-source assignment should support object-name and record-index selectors");
    expect(row_source("customer-guid") == "products.name,product_id" &&
            row_source("orders-guid") == "products.name,product_id" &&
            row_source("other-guid") == "states.name",
        "#805: direct row-source assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#805: first row-source write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#805: second row-source write should remain undo-backed");
    expect(row_source_state() == "customers.name,customer_id,orders.order_id,total,states.name",
        "#805: row-source undo should restore original direct values");

    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .row_source = "ThisForm.aRows"
    });
    expect(row_source_result.ok, "#805: row-source assignment should support UNIQUEID selectors");
    expect(row_source("customer-guid") == "ThisForm.aRows" &&
            row_source("orders-guid") == "ThisForm.aRows",
        "#805: direct row-source assignment should store caller text without serialized quoting");

    const std::string committed_state = row_source_state();
    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = table_path.string(),
        .objects = {},
        .row_source = "Ignored"
    });
    expect(!row_source_result.ok, "#805: row-source assignment should reject empty selections");
    expect(row_source_state() == committed_state, "#805: empty-selection failures should not mutate row sources");

    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .row_source = "Ignored"
    });
    expect(!row_source_result.ok, "#805: row-source assignment should reject missing selected objects");
    expect(row_source_state() == committed_state, "#805: missing-object failures should not mutate row sources");

    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .row_source = "Ignored"
    });
    expect(!row_source_result.ok, "#805: row-source assignment should reject duplicate selected objects");
    expect(row_source_state() == committed_state, "#805: duplicate-selection failures should not mutate row sources");

    const fs::path blob_path = temp_dir / "rowsource_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "RowSource = \"customers.name\"\r\nCaption = \"Customer\"\r\n"},
        {"cboNoSource", "no-source-guid", "Caption = \"No source\"\r\n"},
        {"cboOther", "other-guid", "RowSource = \"states.name\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#805: row-source property-blob fixture should be writable");

    const auto blob_row_source_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "RowSource"
        });
    };

    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoSource", .unique_id = {}}
        },
        .row_source = "orders.\"total\""
    });
    expect(row_source_result.ok, "#805: row-source assignment should support existing and absent serialized properties");
    auto blob_source = blob_row_source_state("blob-guid");
    auto appended_source = blob_row_source_state("no-source-guid");
    auto other_source = blob_row_source_state("other-guid");
    expect(blob_source.ok && blob_source.exists && blob_source.value == "\"orders.\"\"total\"\"\"" &&
            appended_source.ok && appended_source.exists && appended_source.value == "\"orders.\"\"total\"\"\"" &&
            other_source.ok && other_source.exists && other_source.value == "\"states.name\"",
        "#805: serialized row-source assignment should quote text, append missing RowSource, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#805: appended serialized row-source write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#805: existing serialized row-source write should remain undo-backed");
    blob_source = blob_row_source_state("blob-guid");
    appended_source = blob_row_source_state("no-source-guid");
    expect(blob_source.ok && blob_source.exists && blob_source.value == "\"customers.name\"" &&
            appended_source.ok && !appended_source.exists,
        "#805: serialized row-source undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_rowsource.scx";
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
    expect(incomplete_create.ok, "#805: missing-RowSource fixture should be writable");

    row_source_result = copperfin::vfp::set_visual_object_row_source({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .row_source = "Ignored"
    });
    expect(!row_source_result.ok, "#805: row-source assignment should reject objects without a writable RowSource carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_row_source_type_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_rowsource_type_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "rowsource_type.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "ROWSOURCET", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "2"},
        {"lstOrders", "ordersList", "orders-guid", "3"},
        {"cboOther", "otherCombo", "other-guid", "5"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#806: row-source-type fixture should be writable");

    const auto row_source_type_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "RowSourceType"
        });
        expect(result.ok && result.exists, "#806: row-source-type fixture property should be readable");
        return result.value;
    };
    const auto row_source_type = [&](const std::string& unique_id) {
        return row_source_type_for(table_path.string(), unique_id);
    };
    const auto row_source_type_state = [&]() {
        return row_source_type("customer-guid") + "," +
            row_source_type("orders-guid") + "," +
            row_source_type("other-guid");
    };

    auto type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .row_source_type = 6
    });
    expect(type_result.ok, "#806: row-source-type assignment should support object-name and record-index selectors");
    expect(row_source_type("customer-guid") == "6" &&
            row_source_type("orders-guid") == "6" &&
            row_source_type("other-guid") == "5",
        "#806: direct row-source-type assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#806: first row-source-type write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#806: second row-source-type write should remain undo-backed");
    expect(row_source_type_state() == "2,3,5", "#806: row-source-type undo should restore original direct values");

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .row_source_type = 5
    });
    expect(type_result.ok, "#806: row-source-type assignment should support UNIQUEID selectors");
    expect(row_source_type("customer-guid") == "5" &&
            row_source_type("orders-guid") == "5",
        "#806: direct row-source-type assignment should store unquoted numeric values");

    const std::string committed_state = row_source_type_state();
    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {},
        .row_source_type = 4
    });
    expect(!type_result.ok, "#806: row-source-type assignment should reject empty selections");
    expect(row_source_type_state() == committed_state, "#806: empty-selection failures should not mutate row-source types");

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .row_source_type = -1
    });
    expect(!type_result.ok, "#806: row-source-type assignment should reject negative values");
    expect(row_source_type_state() == committed_state, "#806: negative-value failures should not mutate row-source types");

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .row_source_type = 4
    });
    expect(!type_result.ok, "#806: row-source-type assignment should reject missing selected objects");
    expect(row_source_type_state() == committed_state, "#806: missing-object failures should not mutate row-source types");

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .row_source_type = 4
    });
    expect(!type_result.ok, "#806: row-source-type assignment should reject duplicate selected objects");
    expect(row_source_type_state() == committed_state, "#806: duplicate-selection failures should not mutate row-source types");

    const fs::path blob_path = temp_dir / "rowsource_type_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "RowSourceType = 2\r\nCaption = \"Customer\"\r\n"},
        {"cboNoType", "no-type-guid", "Caption = \"No type\"\r\n"},
        {"cboOther", "other-guid", "RowSourceType = 5\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#806: row-source-type property-blob fixture should be writable");

    const auto blob_row_source_type_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "RowSourceType"
        });
    };

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoType", .unique_id = {}}
        },
        .row_source_type = 3
    });
    expect(type_result.ok, "#806: row-source-type assignment should support existing and absent serialized properties");
    auto blob_type = blob_row_source_type_state("blob-guid");
    auto appended_type = blob_row_source_type_state("no-type-guid");
    auto other_type = blob_row_source_type_state("other-guid");
    expect(blob_type.ok && blob_type.exists && blob_type.value == "3" &&
            appended_type.ok && appended_type.exists && appended_type.value == "3" &&
            other_type.ok && other_type.exists && other_type.value == "5",
        "#806: serialized row-source-type assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#806: appended serialized row-source-type write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#806: existing serialized row-source-type write should remain undo-backed");
    blob_type = blob_row_source_type_state("blob-guid");
    appended_type = blob_row_source_type_state("no-type-guid");
    expect(blob_type.ok && blob_type.exists && blob_type.value == "2" &&
            appended_type.ok && !appended_type.exists,
        "#806: serialized row-source-type undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_rowsourcetype.scx";
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
    expect(incomplete_create.ok, "#806: missing-RowSourceType fixture should be writable");

    type_result = copperfin::vfp::set_visual_object_row_source_type({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .row_source_type = 4
    });
    expect(!type_result.ok, "#806: row-source-type assignment should reject objects without a writable RowSourceType carrier");

    fs::remove_all(temp_dir, ignored);
}
