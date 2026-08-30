void test_set_visual_object_font_bold_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_bold_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_bold.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTBOLD", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#833: font-bold fixture should be writable");

    const auto font_bold_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontBold"
        });
        expect(result.ok && result.exists, "#833: font-bold fixture property should be readable");
        return result.value;
    };
    const auto font_bold = [&](const std::string& unique_id) {
        return font_bold_for(table_path.string(), unique_id);
    };
    const auto font_bold_state = [&]() {
        return font_bold("customer-guid") + "," +
            font_bold("orders-guid") + "," +
            font_bold("other-guid");
    };

    auto bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_bold = true
    });
    expect(bold_result.ok, "#833: font-bold assignment should support object-name and record-index selectors");
    expect(font_bold("customer-guid") == ".T." &&
            font_bold("orders-guid") == ".T." &&
            font_bold("other-guid") == ".T.",
        "#833: direct font-bold assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#833: first font-bold write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#833: second font-bold write should remain undo-backed");
    expect(font_bold_state() == ".F.,.F.,.T.", "#833: font-bold undo should restore original direct values");

    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_bold = true
    });
    expect(bold_result.ok, "#833: font-bold assignment should support UNIQUEID selectors");
    expect(font_bold("customer-guid") == ".T." &&
            font_bold("orders-guid") == ".T.",
        "#833: direct font-bold assignment should store caller logical state");

    const std::string committed_state = font_bold_state();
    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = table_path.string(),
        .objects = {},
        .font_bold = false
    });
    expect(!bold_result.ok, "#833: font-bold assignment should reject empty selections");
    expect(font_bold_state() == committed_state, "#833: empty-selection failures should not mutate font-bold values");

    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_bold = false
    });
    expect(!bold_result.ok, "#833: font-bold assignment should reject missing selected objects");
    expect(font_bold_state() == committed_state, "#833: missing-object failures should not mutate font-bold values");

    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_bold = false
    });
    expect(!bold_result.ok, "#833: font-bold assignment should reject duplicate selected objects");
    expect(font_bold_state() == committed_state, "#833: duplicate-selection failures should not mutate font-bold values");

    const fs::path blob_path = temp_dir / "font_bold_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontBold = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoBold", "no-bold-guid", "Caption = \"No bold\"\r\n"},
        {"txtOther", "other-guid", "FontBold = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#833: font-bold property-blob fixture should be writable");

    const auto blob_font_bold_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontBold"
        });
    };

    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoBold", .unique_id = {}}
        },
        .font_bold = true
    });
    expect(bold_result.ok, "#833: font-bold assignment should support existing and absent serialized properties");
    auto blob_bold = blob_font_bold_state("blob-guid");
    auto appended_bold = blob_font_bold_state("no-bold-guid");
    auto other_bold = blob_font_bold_state("other-guid");
    expect(blob_bold.ok && blob_bold.exists && blob_bold.value == ".T." &&
            appended_bold.ok && appended_bold.exists && appended_bold.value == ".T." &&
            other_bold.ok && other_bold.exists && other_bold.value == ".T.",
        "#833: serialized font-bold assignment should store logical values, append missing FontBold, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#833: appended serialized font-bold write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#833: existing serialized font-bold write should remain undo-backed");
    blob_bold = blob_font_bold_state("blob-guid");
    appended_bold = blob_font_bold_state("no-bold-guid");
    expect(blob_bold.ok && blob_bold.exists && blob_bold.value == ".F." &&
            appended_bold.ok && !appended_bold.exists,
        "#833: serialized font-bold undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontbold.scx";
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
    expect(incomplete_create.ok, "#833: missing-FontBold fixture should be writable");

    bold_result = copperfin::vfp::set_visual_object_font_bold({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_bold = true
    });
    expect(!bold_result.ok, "#833: font-bold assignment should reject objects without a writable FontBold carrier");

    fs::remove_all(temp_dir, ignored);
}

void test_set_visual_object_font_italic_assigns_logical_state() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_font_italic_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "font_italic.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "FONTITALIC", .type = 'C', .length = 3U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#834: font-italic fixture should be writable");

    const auto font_italic_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontItalic"
        });
        expect(result.ok && result.exists, "#834: font-italic fixture property should be readable");
        return result.value;
    };
    const auto font_italic = [&](const std::string& unique_id) {
        return font_italic_for(table_path.string(), unique_id);
    };
    const auto font_italic_state = [&]() {
        return font_italic("customer-guid") + "," +
            font_italic("orders-guid") + "," +
            font_italic("other-guid");
    };

    auto italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        .font_italic = true
    });
    expect(italic_result.ok, "#834: font-italic assignment should support object-name and record-index selectors");
    expect(font_italic("customer-guid") == ".T." &&
            font_italic("orders-guid") == ".T." &&
            font_italic("other-guid") == ".T.",
        "#834: direct font-italic assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#834: first font-italic write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#834: second font-italic write should remain undo-backed");
    expect(font_italic_state() == ".F.,.F.,.T.", "#834: font-italic undo should restore original direct values");

    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        .font_italic = true
    });
    expect(italic_result.ok, "#834: font-italic assignment should support UNIQUEID selectors");
    expect(font_italic("customer-guid") == ".T." &&
            font_italic("orders-guid") == ".T.",
        "#834: direct font-italic assignment should store caller logical state");

    const std::string committed_state = font_italic_state();
    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = table_path.string(),
        .objects = {},
        .font_italic = false
    });
    expect(!italic_result.ok, "#834: font-italic assignment should reject empty selections");
    expect(font_italic_state() == committed_state, "#834: empty-selection failures should not mutate font-italic values");

    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        .font_italic = false
    });
    expect(!italic_result.ok, "#834: font-italic assignment should reject missing selected objects");
    expect(font_italic_state() == committed_state, "#834: missing-object failures should not mutate font-italic values");

    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = table_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        .font_italic = false
    });
    expect(!italic_result.ok, "#834: font-italic assignment should reject duplicate selected objects");
    expect(font_italic_state() == committed_state, "#834: duplicate-selection failures should not mutate font-italic values");

    const fs::path blob_path = temp_dir / "font_italic_blob.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", "FontItalic = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoItalic", "no-italic-guid", "Caption = \"No italic\"\r\n"},
        {"txtOther", "other-guid", "FontItalic = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, "#834: font-italic property-blob fixture should be writable");

    const auto blob_font_italic_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = "FontItalic"
        });
    };

    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = blob_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoItalic", .unique_id = {}}
        },
        .font_italic = true
    });
    expect(italic_result.ok, "#834: font-italic assignment should support existing and absent serialized properties");
    auto blob_italic = blob_font_italic_state("blob-guid");
    auto appended_italic = blob_font_italic_state("no-italic-guid");
    auto other_italic = blob_font_italic_state("other-guid");
    expect(blob_italic.ok && blob_italic.exists && blob_italic.value == ".T." &&
            appended_italic.ok && appended_italic.exists && appended_italic.value == ".T." &&
            other_italic.ok && other_italic.exists && other_italic.value == ".T.",
        "#834: serialized font-italic assignment should store logical values, append missing FontItalic, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#834: appended serialized font-italic write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, "#834: existing serialized font-italic write should remain undo-backed");
    blob_italic = blob_font_italic_state("blob-guid");
    appended_italic = blob_font_italic_state("no-italic-guid");
    expect(blob_italic.ok && blob_italic.exists && blob_italic.value == ".F." &&
            appended_italic.ok && !appended_italic.exists,
        "#834: serialized font-italic undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / "missing_fontitalic.scx";
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
    expect(incomplete_create.ok, "#834: missing-FontItalic fixture should be writable");

    italic_result = copperfin::vfp::set_visual_object_font_italic({
        .path = incomplete_path.string(),
        .objects = {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        .font_italic = true
    });
    expect(!italic_result.ok, "#834: font-italic assignment should reject objects without a writable FontItalic carrier");

    fs::remove_all(temp_dir, ignored);
}
