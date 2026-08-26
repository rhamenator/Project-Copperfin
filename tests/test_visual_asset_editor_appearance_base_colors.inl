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
