void test_set_visual_object_font_name_assigns_text() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_name_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_name.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTNAME", .type = 'C', .length = 40U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", "Arial"},
        {"lblOrders", "ordersLabel", "orders-guid", "Tahoma"},
        {"txtOther", "otherBox", "other-guid", "Courier New"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#831: font-name fixture should be writable");

    const auto font_name_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontName"
        });
        expect(result.ok && result.exists, "#831: font-name fixture property should be readable");
        return result.value;
    };
    const auto font_name = [&](const std::string& unique_id) {
        return font_name_for(table_path.string(), unique_id);
    };
    const auto font_name_state = [&]() {
        return font_name("customer-guid") + "," +
            font_name("orders-guid") + "," +
            font_name("other-guid");
    };

    auto font_result = copperfin::vfp::set_visual_object_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_name = "Segoe UI"
    });
    expect(font_result.ok, "#831: font-name assignment should support object-name and record-index selectors");
    expect(font_name("customer-guid") == "Segoe UI" &&
            font_name("orders-guid") == "Segoe UI" &&
            font_name("other-guid") == "Courier New",
        "#831: direct font-name assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#831: first font-name write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#831: second font-name write should remain undo-backed");
    expect(font_name_state() == "Arial,Tahoma,Courier New",
        "#831: font-name undo should restore original direct values");

    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_name = "Consolas"
    });
    expect(font_result.ok, "#831: font-name assignment should support UNIQUEID selectors");
    expect(font_name("customer-guid") == "Consolas" &&
            font_name("orders-guid") == "Consolas",
        "#831: direct font-name assignment should store caller text without serialized quoting");

    const std::string committed_state = font_name_state();
    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = table_path.string(),
        .objects = {},
        .font_name = "Ignored"
    });
    expect(!font_result.ok, "#831: font-name assignment should reject empty selections");
    expect(font_name_state() == committed_state, "#831: empty-selection failures should not mutate font names");

    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_name = "Ignored"
    });
    expect(!font_result.ok, "#831: font-name assignment should reject missing selected objects");
    expect(font_name_state() == committed_state, "#831: missing-object failures should not mutate font names");

    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_name = "Ignored"
    });
    expect(!font_result.ok, "#831: font-name assignment should reject duplicate selected objects");
    expect(font_name_state() == committed_state, "#831: duplicate-selection failures should not mutate font names");

    const fs::path blob_path = temp_dir / "font_name_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontName = \"Arial\"\r\nCaption = \"Customer\"\r\n"},
        {"txtNoFont", "no-font-guid", "Caption = \"No font\"\r\n"},
        {"txtOther", "other-guid", "FontName = \"Courier New\"\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#831: font-name property-blob fixture should be writable");

    const auto blob_font_name_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontName"
        });
    };

    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoFont", .unique_id = {}}
        },
        .font_name = "Aptos \"Display\""
    });
    expect(font_result.ok, "#831: font-name assignment should support existing and absent serialized properties");
    auto blob_font = blob_font_name_state("blob-guid");
    auto appended_font = blob_font_name_state("no-font-guid");
    auto other_font = blob_font_name_state("other-guid");
    expect(blob_font.ok && blob_font.exists && blob_font.value == "\"Aptos \"\"Display\"\"\"" &&
            appended_font.ok && appended_font.exists && appended_font.value == "\"Aptos \"\"Display\"\"\"" &&
            other_font.ok && other_font.exists && other_font.value == "\"Courier New\"",
        "#831: serialized font-name assignment should quote text, append missing FontName, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#831: appended serialized font-name write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#831: existing serialized font-name write should remain undo-backed");
    blob_font = blob_font_name_state("blob-guid");
    appended_font = blob_font_name_state("no-font-guid");
    expect(blob_font.ok && blob_font.exists && blob_font.value == "\"Arial\"" &&
            appended_font.ok && !appended_font.exists,
        "#831: serialized font-name undo should restore existing values and remove appended properties");

    const fs::path casefold_blob_path = temp_dir / "font_name_casefold.scx";
    const auto casefold_blob_create = copperfin::vfp::create_dbf_table_file(casefold_blob_path.string(), blob_fields, blob_records);
    expect(casefold_blob_create.ok, "#2983: casefold font-name property-blob fixture should be writable");

    fs::path lowercase_sidecar = casefold_blob_path;
    lowercase_sidecar.replace_extension(".sct");
    fs::path uppercase_sidecar = casefold_blob_path;
    uppercase_sidecar.replace_extension(".SCT");
    fs::rename(lowercase_sidecar, uppercase_sidecar, ignored);
    expect(!ignored, "#2983: casefold font-name fixture should rename the memo sidecar");

    auto casefold_font_result = copperfin::vfp::set_visual_object_font_name({
        .path = casefold_blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"}
        },
        .font_name = "Consolas"
    });
    expect(casefold_font_result.ok,
        "#2983: font-name assignment should resolve uppercase memo sidecars for property-blob assets");
    auto casefold_font = copperfin::vfp::query_visual_object_property({
        .path = casefold_blob_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "blob-guid",
        .property_name = "FontName"
    });
    expect(casefold_font.ok && casefold_font.exists && casefold_font.value == "\"Consolas\"",
        "#2983: font-name assignment should persist serialized text through uppercase memo sidecars");
    undo_result = copperfin::vfp::undo_visual_object_property(casefold_blob_path.string());
    expect(undo_result.ok, "#2983: uppercase-sidecar font-name assignment should remain undo-backed");
    casefold_font = copperfin::vfp::query_visual_object_property({
        .path = casefold_blob_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "blob-guid",
        .property_name = "FontName"
    });
    expect(casefold_font.ok && casefold_font.exists && casefold_font.value == "\"Arial\"",
        "#2983: uppercase-sidecar font-name undo should restore the original serialized text");

    const fs::path incomplete_path = temp_dir / "missing_fontname.scx";
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
    expect(incomplete_create.ok, "#831: missing-FontName fixture should be writable");

    font_result = copperfin::vfp::set_visual_object_font_name({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_name = "Ignored"
    });
    expect(!font_result.ok, "#831: font-name assignment should reject objects without a writable FontName carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_font_size_assigns_numeric_value() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_size_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_size.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTSIZE", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", "10"},
        {"lblOrders", "ordersLabel", "orders-guid", "9"},
        {"txtOther", "otherBox", "other-guid", "12"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#832: font-size fixture should be writable");

    const auto font_size_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontSize"
        });
        expect(result.ok && result.exists, "#832: font-size fixture property should be readable");
        return result.value;
    };
    const auto font_size = [&](const std::string& unique_id) {
        return font_size_for(table_path.string(), unique_id);
    };
    const auto font_size_state = [&]() {
        return font_size("customer-guid") + "," +
            font_size("orders-guid") + "," +
            font_size("other-guid");
    };

    auto font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_size = 13.5
    });
    expect(font_result.ok, "#832: font-size assignment should support object-name and record-index selectors");
    expect(font_size("customer-guid") == "13.5" &&
            font_size("orders-guid") == "13.5" &&
            font_size("other-guid") == "12",
        "#832: direct font-size assignment should write compact numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#832: first font-size write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#832: second font-size write should remain undo-backed");
    expect(font_size_state() == "10,9,12", "#832: font-size undo should restore original direct values");

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_size = 11.25
    });
    expect(font_result.ok, "#832: font-size assignment should support UNIQUEID selectors");
    expect(font_size("customer-guid") == "11.25" &&
            font_size("orders-guid") == "11.25",
        "#832: direct font-size assignment should store compact fractional values");

    const std::string committed_state = font_size_state();
    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {},
        .font_size = 8.0
    });
    expect(!font_result.ok, "#832: font-size assignment should reject empty selections");
    expect(font_size_state() == committed_state, "#832: empty-selection failures should not mutate font sizes");

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"}
        },
        .font_size = -1.0
    });
    expect(!font_result.ok, "#832: font-size assignment should reject negative sizes");
    expect(font_size_state() == committed_state, "#832: negative-size failures should not mutate font sizes");

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_size = 8.0
    });
    expect(!font_result.ok, "#832: font-size assignment should reject missing selected objects");
    expect(font_size_state() == committed_state, "#832: missing-object failures should not mutate font sizes");

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_size = 8.0
    });
    expect(!font_result.ok, "#832: font-size assignment should reject duplicate selected objects");
    expect(font_size_state() == committed_state, "#832: duplicate-selection failures should not mutate font sizes");

    const fs::path blob_path = temp_dir / "font_size_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontSize = 10\r\nCaption = \"Customer\"\r\n"},
        {"txtNoSize", "no-size-guid", "Caption = \"No size\"\r\n"},
        {"txtOther", "other-guid", "FontSize = 12\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#832: font-size property-blob fixture should be writable");

    const auto blob_font_size_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontSize"
        });
    };

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoSize", .unique_id = {}}
        },
        .font_size = 9.75
    });
    expect(font_result.ok, "#832: font-size assignment should support existing and absent serialized properties");
    auto blob_size = blob_font_size_state("blob-guid");
    auto appended_size = blob_font_size_state("no-size-guid");
    auto other_size = blob_font_size_state("other-guid");
    expect(blob_size.ok && blob_size.exists && blob_size.value == "9.75" &&
            appended_size.ok && appended_size.exists && appended_size.value == "9.75" &&
            other_size.ok && other_size.exists && other_size.value == "12",
        "#832: serialized font-size assignment should store unquoted numerics, append missing FontSize, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#832: appended serialized font-size write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#832: existing serialized font-size write should remain undo-backed");
    blob_size = blob_font_size_state("blob-guid");
    appended_size = blob_font_size_state("no-size-guid");
    expect(blob_size.ok && blob_size.exists && blob_size.value == "10" &&
            appended_size.ok && !appended_size.exists,
        "#832: serialized font-size undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontsize.scx";
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
    expect(incomplete_create.ok, "#832: missing-FontSize fixture should be writable");

    font_result = copperfin::vfp::set_visual_object_font_size({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_size = 8.0
    });
    expect(!font_result.ok, "#832: font-size assignment should reject objects without a writable FontSize carrier");

    fs::remove_all(temp_dir, ignored);
}
