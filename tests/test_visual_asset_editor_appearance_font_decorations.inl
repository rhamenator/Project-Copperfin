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
