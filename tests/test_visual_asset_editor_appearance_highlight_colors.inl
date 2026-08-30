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
