void test_copy_visual_object_property_between_selected_objects() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_copy_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_copy.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "111", "Caption = \"Save\"\r\nLeft = 10\r\n"},
        {"txtName", "nameBox", "name-guid", "222", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblOther", "otherLabel", "other-guid", "333", "Caption = \"Other\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#767: property-copy fixture should be writable");

    auto copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "hpos",
        .target_record_index = 0U,
        .target_object_name = "txtName",
        .target_unique_id = {},
        .target_property_name = {},
        .replace_existing = true
    });
    expect(copy_result.ok, "#767: property copy should support UNIQUEID source, object-name target, and direct-field replacement");
    expect(copy_result.affected_object_count == 1U,
        "#1005: successful property copy should report one affected object");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = "cmdSave",
        .source_unique_id = {},
        .source_property_name = "caption",
        .target_record_index = 1U,
        .target_object_name = {},
        .target_unique_id = {},
        .target_property_name = "CopiedCaption",
        .replace_existing = false
    });
    expect(copy_result.ok, "#767: property copy should support object-name source, record-index target, and target renames");

    auto target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    auto target_copied_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "CopiedCaption"
    });
    expect(target_hpos.ok && target_hpos.exists && target_hpos.value == "111" &&
            target_copied_caption.ok && target_copied_caption.exists && target_copied_caption.value == "\"Save\"",
        "#767: property copy should persist direct-field and memo-backed target values");

    auto source_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .property_name = "Caption"
    });
    auto target_top = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "Top"
    });
    auto other_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "other-guid",
        .property_name = "Caption"
    });
    expect(source_caption.ok && source_caption.exists && source_caption.value == "\"Save\"" &&
            target_top.ok && target_top.exists && target_top.value == "30" &&
            other_caption.ok && other_caption.exists && other_caption.value == "\"Other\"",
        "#767: property copy should preserve source values, unrelated target assignments, and unrelated objects");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject target collisions without replacement");
    expect(copy_result.affected_object_count == 0U,
        "#1005: failed property copy should report zero affected objects");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "MissingProp",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "MissingCopy",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject missing source properties");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = " ",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = "EmptySource",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject empty source property names");

    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = table_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "save-guid",
        .source_property_name = "Caption",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "name-guid",
        .target_property_name = " ",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject empty requested target property names");

    const fs::path unsupported_path = temp_dir / "property_copy_unsupported.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "HPOS", .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdSource", "source-guid", "111"},
        {"txtTarget", "target-guid", "222"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#767: unsupported property-copy fixture should be writable");
    copy_result = copperfin::vfp::copy_visual_object_property({
        .path = unsupported_path.string(),
        .source_record_index = 0U,
        .source_object_name = {},
        .source_unique_id = "source-guid",
        .source_property_name = "HPOS",
        .target_record_index = 0U,
        .target_object_name = {},
        .target_unique_id = "target-guid",
        .target_property_name = "Caption",
        .replace_existing = false
    });
    expect(!copy_result.ok, "#767: property copy should reject unsupported target property paths");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#767: undo should restore copied memo-backed properties");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#767: undo should restore copied direct fields");

    target_hpos = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {},
        .property_name = "HPOS"
    });
    target_copied_caption = copperfin::vfp::query_visual_object_property({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "name-guid",
        .property_name = "CopiedCaption"
    });
    expect(target_hpos.ok && target_hpos.exists && target_hpos.value == "222" &&
            target_copied_caption.ok && !target_copied_caption.exists,
        "#767: undo should restore direct fields and remove copied memo-backed assignments");

    fs::remove_all(temp_dir, ignored);
}

void test_copy_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_property_copy_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "property_copy_batch.scx";
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
        {"txtName", "nameBox", "name-guid", "222", "322", "Caption = \"Name\"\r\nTop = 30\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "333", "433", "Caption = \"Status\"\r\nLeft = 50\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#774: property-copy-batch fixture should be writable");

    const auto property_state = [&](const std::string& unique_id, const std::string& property_name) {
        return copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    auto batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "hpos",
                .target_record_index = 0U,
                .target_object_name = "txtName",
                .target_unique_id = {},
                .target_property_name = {},
                .replace_existing = true
            },
            {
                .source_record_index = 0U,
                .source_object_name = "cmdSave",
                .source_unique_id = {},
                .source_property_name = "Caption",
                .target_record_index = 2U,
                .target_object_name = {},
                .target_unique_id = {},
                .target_property_name = "CopiedCaption",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "status-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "StatusLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "VPOS",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "VPOS",
                .replace_existing = true
            }
        }
    });
    expect(batch_result.ok, "#774: batch property copy should support mixed selectors, direct fields, memo properties, target renames, and replacement");
    expect(batch_result.affected_object_count == 4U,
        "#1005: successful batch property copy should report affected item count");

    auto name_hpos = property_state("name-guid", "HPOS");
    auto status_copied_caption = property_state("status-guid", "CopiedCaption");
    auto name_status_left = property_state("name-guid", "StatusLeft");
    auto status_vpos = property_state("status-guid", "VPOS");
    auto save_caption = property_state("save-guid", "Caption");
    expect(name_hpos.ok && name_hpos.exists && name_hpos.value == "111" &&
            status_copied_caption.ok && status_copied_caption.exists && status_copied_caption.value == "\"Save\"" &&
            name_status_left.ok && name_status_left.exists && name_status_left.value == "50" &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "211" &&
            save_caption.ok && save_caption.exists && save_caption.value == "\"Save\"",
        "#774: batch property copy should persist target values while preserving sources and unrelated assignments");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#774: successful batch copies should leave normal visual undo history available");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "Caption",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject target collisions");
    expect(batch_result.affected_object_count == 0U,
        "#1005: failed batch property copy should report zero affected objects");
    auto status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: target-collision failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Missing",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "MissingCopy",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject missing source properties");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: missing-source failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = " ",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = "EmptySource",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject empty source names");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: empty-source failures should roll back earlier memo copy targets");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Left",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "status-guid",
                .target_property_name = "TempLeft",
                .replace_existing = false
            },
            {
                .source_record_index = 0U,
                .source_object_name = {},
                .source_unique_id = "save-guid",
                .source_property_name = "Caption",
                .target_record_index = 0U,
                .target_object_name = {},
                .target_unique_id = "name-guid",
                .target_property_name = " ",
                .replace_existing = false
            }
        }
    });
    expect(!batch_result.ok, "#774: batch property copy should reject empty target names");
    status_temp_left = property_state("status-guid", "TempLeft");
    expect(status_temp_left.ok && !status_temp_left.exists,
        "#774: empty-target failures should roll back earlier memo copy targets");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#774: failed batch copy rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::copy_visual_object_properties({
        .path = table_path.string(),
        .properties = {}
    });
    expect(!batch_result.ok, "#774: empty batch copy requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1005: empty batch property copy should report zero affected objects");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#774: undo should restore each successful batch copy target");
    }

    name_hpos = property_state("name-guid", "HPOS");
    status_copied_caption = property_state("status-guid", "CopiedCaption");
    name_status_left = property_state("name-guid", "StatusLeft");
    status_vpos = property_state("status-guid", "VPOS");
    expect(name_hpos.ok && name_hpos.exists && name_hpos.value == "222" &&
            status_copied_caption.ok && !status_copied_caption.exists &&
            name_status_left.ok && !name_status_left.exists &&
            status_vpos.ok && status_vpos.exists && status_vpos.value == "433",
        "#774: successful batch copy undo should restore original target state");

    fs::remove_all(temp_dir, ignored);
}
