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
