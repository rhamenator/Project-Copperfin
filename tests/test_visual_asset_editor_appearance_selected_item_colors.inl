void test_set_visual_object_selected_item_back_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_selected_item_back_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "selected_item_back_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "SELECTEDIT", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "16777215"},
        {"lstOrders", "ordersList", "orders-guid", "12632256"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#817: selected-item back-color fixture should be writable");

    const auto selected_item_back_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "SelectedItemBackColor"
        });
        expect(result.ok && result.exists, "#817: selected-item back-color fixture property should be readable");
        return result.value;
    };
    const auto selected_item_back_color = [&](const std::string& unique_id) {
        return selected_item_back_color_for(table_path.string(), unique_id);
    };
    const auto selected_item_back_color_state = [&]() {
        return selected_item_back_color("customer-guid") + "," +
            selected_item_back_color("orders-guid") + "," +
            selected_item_back_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .selected_item_back_color = 65280
    });
    expect(color_result.ok, "#817: selected-item back-color assignment should support object-name and record-index selectors");
    expect(selected_item_back_color("customer-guid") == "65280" &&
            selected_item_back_color("orders-guid") == "65280" &&
            selected_item_back_color("other-guid") == "255",
        "#817: direct selected-item back-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#817: first selected-item back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#817: second selected-item back-color write should remain undo-backed");
    expect(selected_item_back_color_state() == "16777215,12632256,255",
        "#817: selected-item back-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .selected_item_back_color = 0
    });
    expect(color_result.ok, "#817: selected-item back-color assignment should support UNIQUEID selectors");
    expect(selected_item_back_color("customer-guid") == "0" &&
            selected_item_back_color("orders-guid") == "0",
        "#817: direct selected-item back-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = selected_item_back_color_state();
    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {},
        .selected_item_back_color = 1
    });
    expect(!color_result.ok, "#817: selected-item back-color assignment should reject empty selections");
    expect(selected_item_back_color_state() == committed_state,
        "#817: empty-selection failures should not mutate selected-item back colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .selected_item_back_color = -1
    });
    expect(!color_result.ok, "#817: selected-item back-color assignment should reject negative values");
    expect(selected_item_back_color_state() == committed_state,
        "#817: negative-value failures should not mutate selected-item back colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .selected_item_back_color = 1
    });
    expect(!color_result.ok, "#817: selected-item back-color assignment should reject missing selected objects");
    expect(selected_item_back_color_state() == committed_state,
        "#817: missing-object failures should not mutate selected-item back colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .selected_item_back_color = 1
    });
    expect(!color_result.ok, "#817: selected-item back-color assignment should reject duplicate selected objects");
    expect(selected_item_back_color_state() == committed_state,
        "#817: duplicate-selection failures should not mutate selected-item back colors");

    const fs::path blob_path = temp_dir / "selected_item_back_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "SelectedItemBackColor = 16777215\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "SelectedItemBackColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#817: selected-item back-color property-blob fixture should be writable");

    const auto blob_selected_item_back_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "SelectedItemBackColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .selected_item_back_color = 65280
    });
    expect(color_result.ok, "#817: selected-item back-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_selected_item_back_color_state("blob-guid");
    auto appended_color = blob_selected_item_back_color_state("no-color-guid");
    auto other_color = blob_selected_item_back_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#817: serialized selected-item back-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#817: appended serialized selected-item back-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#817: existing serialized selected-item back-color write should remain undo-backed");
    blob_color = blob_selected_item_back_color_state("blob-guid");
    appended_color = blob_selected_item_back_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "16777215" &&
            appended_color.ok && !appended_color.exists,
        "#817: serialized selected-item back-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_selecteditembackcolor.scx";
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
    expect(incomplete_create.ok, "#817: missing-SelectedItemBackColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_selected_item_back_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .selected_item_back_color = 1
    });
    expect(!color_result.ok,
        "#817: selected-item back-color assignment should reject objects without a writable SelectedItemBackColor carrier");

    fs::remove_all(temp_dir, ignored);
}
void test_set_visual_object_selected_item_fore_color_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_selected_item_fore_color_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "selected_item_fore_color.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "SELECTEDIT", .type = 'C', .length = 12U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cboCustomer", "customerCombo", "customer-guid", "0"},
        {"lstOrders", "ordersList", "orders-guid", "16777215"},
        {"cboOther", "otherCombo", "other-guid", "255"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#818: selected-item fore-color fixture should be writable");

    const auto selected_item_fore_color_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "SelectedItemForeColor"
        });
        expect(result.ok && result.exists, "#818: selected-item fore-color fixture property should be readable");
        return result.value;
    };
    const auto selected_item_fore_color = [&](const std::string& unique_id) {
        return selected_item_fore_color_for(table_path.string(), unique_id);
    };
    const auto selected_item_fore_color_state = [&]() {
        return selected_item_fore_color("customer-guid") + "," +
            selected_item_fore_color("orders-guid") + "," +
            selected_item_fore_color("other-guid");
    };

    auto color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .selected_item_fore_color = 65280
    });
    expect(color_result.ok, "#818: selected-item fore-color assignment should support object-name and record-index selectors");
    expect(selected_item_fore_color("customer-guid") == "65280" &&
            selected_item_fore_color("orders-guid") == "65280" &&
            selected_item_fore_color("other-guid") == "255",
        "#818: direct selected-item fore-color assignment should write raw numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#818: first selected-item fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#818: second selected-item fore-color write should remain undo-backed");
    expect(selected_item_fore_color_state() == "0,16777215,255",
        "#818: selected-item fore-color undo should restore original direct values");

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .selected_item_fore_color = 0
    });
    expect(color_result.ok, "#818: selected-item fore-color assignment should support UNIQUEID selectors");
    expect(selected_item_fore_color("customer-guid") == "0" &&
            selected_item_fore_color("orders-guid") == "0",
        "#818: direct selected-item fore-color assignment should store zero as an unquoted numeric value");

    const std::string committed_state = selected_item_fore_color_state();
    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {},
        .selected_item_fore_color = 1
    });
    expect(!color_result.ok, "#818: selected-item fore-color assignment should reject empty selections");
    expect(selected_item_fore_color_state() == committed_state,
        "#818: empty-selection failures should not mutate selected-item fore colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .selected_item_fore_color = -1
    });
    expect(!color_result.ok, "#818: selected-item fore-color assignment should reject negative values");
    expect(selected_item_fore_color_state() == committed_state,
        "#818: negative-value failures should not mutate selected-item fore colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .selected_item_fore_color = 1
    });
    expect(!color_result.ok, "#818: selected-item fore-color assignment should reject missing selected objects");
    expect(selected_item_fore_color_state() == committed_state,
        "#818: missing-object failures should not mutate selected-item fore colors");

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "cboCustomer", .unique_id = {}}
        },
        .selected_item_fore_color = 1
    });
    expect(!color_result.ok, "#818: selected-item fore-color assignment should reject duplicate selected objects");
    expect(selected_item_fore_color_state() == committed_state,
        "#818: duplicate-selection failures should not mutate selected-item fore colors");

    const fs::path blob_path = temp_dir / "selected_item_fore_color_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"cboBlob", "blob-guid", "SelectedItemForeColor = 0\r\nCaption = \"Customer\"\r\n"},
        {"cboNoColor", "no-color-guid", "Caption = \"No color\"\r\n"},
        {"cboOther", "other-guid", "SelectedItemForeColor = 255\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#818: selected-item fore-color property-blob fixture should be writable");

    const auto blob_selected_item_fore_color_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "SelectedItemForeColor"
        });
    };

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "cboNoColor", .unique_id = {}}
        },
        .selected_item_fore_color = 65280
    });
    expect(color_result.ok, "#818: selected-item fore-color assignment should support existing and absent serialized properties");
    auto blob_color = blob_selected_item_fore_color_state("blob-guid");
    auto appended_color = blob_selected_item_fore_color_state("no-color-guid");
    auto other_color = blob_selected_item_fore_color_state("other-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "65280" &&
            appended_color.ok && appended_color.exists && appended_color.value == "65280" &&
            other_color.ok && other_color.exists && other_color.value == "255",
        "#818: serialized selected-item fore-color assignment should write unquoted numeric values and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#818: appended serialized selected-item fore-color write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#818: existing serialized selected-item fore-color write should remain undo-backed");
    blob_color = blob_selected_item_fore_color_state("blob-guid");
    appended_color = blob_selected_item_fore_color_state("no-color-guid");
    expect(blob_color.ok && blob_color.exists && blob_color.value == "0" &&
            appended_color.ok && !appended_color.exists,
        "#818: serialized selected-item fore-color undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_selecteditemforecolor.scx";
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
    expect(incomplete_create.ok, "#818: missing-SelectedItemForeColor fixture should be writable");

    color_result = copperfin::vfp::set_visual_object_selected_item_fore_color({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .selected_item_fore_color = 1
    });
    expect(!color_result.ok,
        "#818: selected-item fore-color assignment should reject objects without a writable SelectedItemForeColor carrier");

    fs::remove_all(temp_dir, ignored);
}
