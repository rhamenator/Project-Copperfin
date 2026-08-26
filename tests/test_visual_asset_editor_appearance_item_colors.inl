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
