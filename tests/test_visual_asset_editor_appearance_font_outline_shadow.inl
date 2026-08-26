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
