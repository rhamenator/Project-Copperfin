void test_query_visual_object_property_reads_selected_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_query_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path memo_table_path = temp_dir / "query.scx";
    const fs::path memo_path = temp_dir / "query.sct";
    write_synthetic_named_object_asset(memo_table_path, memo_path, {
        {
            .objname = "cmdSave",
            .name = "saveButton",
            .unique_id = "save-guid",
            .properties = "Caption = \"Save\"\r\nLeft = 10\r\n"
        },
        {
            .objname = "txtName",
            .name = "nameBox",
            .unique_id = "target-guid",
            .properties = "Caption = \"Name\"\r\nLeft = 30\r\n"
        }
    });

    auto query_result = copperfin::vfp::query_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = " TARGET-GUID ",
        .property_name = "caption"
    });
    expect(query_result.ok, "#736: visual property queries should support UNIQUEID selectors");
    expect(query_result.exists, "#736: visual property queries should report existing memo-backed properties");
    expect(!query_result.direct_field, "#736: visual property queries should identify memo-backed properties");
    expect(query_result.record_index == 1U, "#739: UNIQUEID property queries should report the resolved record index");
    expect(query_result.property_name == "Caption", "#736: visual property queries should return the stored memo property name");
    expect(query_result.value == "\"Name\"", "#736: visual property queries should return the selected memo property value");
    expect(!copperfin::vfp::query_visual_object_undo(memo_table_path.string()).available,
        "#736: visual property queries should not create undo history");

    query_result = copperfin::vfp::query_visual_object_property({
        .path = memo_table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .property_name = "MissingProp"
    });
    expect(query_result.ok, "#736: missing memo-backed property queries should report cleanly");
    expect(!query_result.exists, "#736: missing memo-backed property queries should not be marked existing");
    expect(!query_result.direct_field, "#736: missing memo-backed property queries should not be direct fields");
    expect(query_result.property_name == "MissingProp", "#736: missing property queries should echo the requested property name");
    expect(query_result.value.empty(), "#736: missing property queries should return an empty value");

    const fs::path direct_table_path = temp_dir / "query_geometry.scx";
    const fs::path direct_memo_path = temp_dir / "query_geometry.sct";
    write_synthetic_named_geometry_asset(direct_table_path, direct_memo_path);
    query_result = copperfin::vfp::query_visual_object_property({
        .path = direct_table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "hpos"
    });
    expect(query_result.ok, "#736: visual property queries should support object-name selectors");
    expect(query_result.exists, "#736: visual property queries should report existing direct fields");
    expect(query_result.direct_field, "#736: visual property queries should identify direct fields");
    expect(query_result.record_index == 1U, "#739: object-name property queries should report the resolved record index");
    expect(query_result.property_name == "HPOS", "#736: visual property queries should return the stored direct field name");
    expect(std::abs(parse_number(query_result.value) - 222.0) < 0.001,
        "#736: visual property queries should return the selected direct-field value");
    expect(!copperfin::vfp::query_visual_object_undo(direct_table_path.string()).available,
        "#736: direct-field queries should not create undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_clear_visual_object_property_resets_selected_values() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_clear_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "clear.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nLeft = 30\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#766: property-clear fixture should be writable");

    auto clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "MissingProp"
    });
    expect(clear_result.ok, "#766: clearing missing memo-backed properties should succeed as a no-op");
    expect(clear_result.affected_object_count == 1U,
        "#1005: successful property clear should report one affected object");
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#766: missing memo-backed property clears should not create undo history");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .property_name = "hpos"
    });
    expect(clear_result.ok, "#766: property clear should support record-index direct-field selection");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "caption"
    });
    expect(clear_result.ok, "#766: property clear should support UNIQUEID memo-backed selection");

    auto hpos_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(hpos_query.ok && hpos_query.exists && hpos_query.direct_field && hpos_query.value.empty(),
        "#766: direct-field clears should write an empty value through the direct field path");

    auto caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    expect(caption_query.ok && !caption_query.exists,
        "#766: memo-backed clears should remove the assignment instead of leaving an empty value");

    auto left_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Left"
    });
    auto other_caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "Caption"
    });
    expect(left_query.ok && left_query.exists && left_query.value == "10" &&
            other_caption_query.ok && other_caption_query.exists && other_caption_query.value == "\"Name\"",
        "#766: property clear should preserve unrelated memo assignments and unrelated objects");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = " "
    });
    expect(!clear_result.ok, "#766: property clear should reject empty property names");
    expect(clear_result.affected_object_count == 0U,
        "#1005: failed property clear should report zero affected objects");

    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "missing-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject missing selected objects");

    const fs::path no_properties_path = temp_dir / "clear_no_properties.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_properties_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_properties_records{
        {"cmdNoProps", "no-props-guid"}
    };
    const auto no_properties_create = copperfin::vfp::create_dbf_table_file(
        no_properties_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(no_properties_create.ok, "#766: missing-PROPERTIES fixture should be writable");
    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = no_properties_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject missing PROPERTIES fields for memo-backed clears");

    const fs::path unsupported_path = temp_dir / "clear_unsupported.dbf";
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        no_properties_fields,
        no_properties_records);
    expect(unsupported_create.ok, "#766: unsupported asset fixture should be writable");
    clear_result = copperfin::vfp::clear_visual_object_property({
        .path = unsupported_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-props-guid",
        .property_name = "Caption"
    });
    expect(!clear_result.ok, "#766: property clear should reject unsupported asset paths for memo-backed clears");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#766: undo should restore cleared memo-backed assignments");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#766: undo should restore cleared direct fields");

    caption_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    hpos_query = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    expect(caption_query.ok && caption_query.exists && caption_query.value == "\"Save\"" &&
            hpos_query.ok && hpos_query.exists && hpos_query.value == "222",
        "#766: undo should restore direct and memo-backed cleared property values");

    fs::remove_all(temp_dir, ignored);
}

void test_clear_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_clear_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "clear_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "211", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "322", "Caption = \"Name\"\r\nLeft = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "433", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#771: property-clear-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .property_name = "hpos"
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .property_name = "Caption"
            },
            {
                .record_index = 2U,
                .object_name = {},
                .unique_id = {},
                .property_name = "MissingMemo"
            }
        }
    });
    expect(batch_result.ok, "#771: batch property clears should support mixed selectors and missing memo no-ops");
    expect(batch_result.affected_object_count == 3U,
        "#1005: successful batch property clear should report affected item count");

    auto save_hpos = property_state("save-guid", "HPOS");
    auto name_caption = property_state("name-guid", "Caption");
    auto status_left = property_state("status-guid", "Left");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.direct_field && save_hpos.value.empty() &&
            name_caption.ok && !name_caption.exists &&
            status_left.ok && status_left.exists && status_left.value == "50",
        "#771: batch clears should clear direct fields, remove memo assignments, and preserve unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#771: successful batch clears should leave normal visual undo history available");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "VPOS"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "missing-guid",
                .property_name = "Caption"
            }
        }
    });
    expect(!batch_result.ok, "#771: batch property clears should fail when a later selection is missing");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property clear should report zero affected objects");
    auto status_vpos = property_state("status-guid", "VPOS");
    expect(status_vpos.ok && status_vpos.exists && status_vpos.value == "433",
        "#771: failed batch clears should roll back earlier direct-field clears");
    const auto undo_after_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failure.available == undo_before_failure.available &&
            undo_after_failure.label == undo_before_failure.label,
        "#771: failed batch rollback should clean up undo entries created by the failed batch");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = "Left"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .property_name = " "
            }
        }
    });
    expect(!batch_result.ok, "#771: batch property clears should reject empty property names");
    status_left = property_state("status-guid", "Left");
    expect(status_left.ok && status_left.exists && status_left.value == "50",
        "#771: empty-name batch failures should roll back earlier memo clears");

    batch_result = copperfin::vfp::clear_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#771: empty batch clear requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property clear should report zero affected objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#771: undo should restore memo clears from successful batches");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#771: undo should restore direct-field clears from successful batches");

    save_hpos = property_state("save-guid", "HPOS");
    name_caption = property_state("name-guid", "Caption");
    expect(save_hpos.ok && save_hpos.exists && save_hpos.value == "111" &&
            name_caption.ok && name_caption.exists && name_caption.value == "\"Name\"",
        "#771: successful batch clear undo should restore original direct and memo-backed values");

    fs::remove_all(temp_dir, ignored);
}
