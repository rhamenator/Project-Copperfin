void test_update_visual_object_properties_updates_selected_geometry_fields() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_multi_property_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "geometry.scx";
    const fs::path memo_path = temp_dir / "geometry.sct";
    write_synthetic_named_geometry_asset(table_path, memo_path);

    const auto update_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {
            {.property_name = "hpos", .property_value = "333.000"},
            {.property_name = "VPOS", .property_value = "444.000"}
        }
    });
    expect(update_result.ok, "#735: multi-property edits should update selected geometry fields");
    expect(update_result.affected_object_count == 1U,
        "#1007: successful multi-property edit should report one affected object");

    auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#735: multi-property geometry fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* first_hpos = find_record_field(parse_result.table.records[0], "HPOS");
        const auto* first_vpos = find_record_field(parse_result.table.records[0], "VPOS");
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(first_hpos != nullptr && std::abs(parse_number(first_hpos->display_value) - 111.0) < 0.001,
            "#735: multi-property edits should preserve unrelated HPOS values");
        expect(first_vpos != nullptr && std::abs(parse_number(first_vpos->display_value) - 211.0) < 0.001,
            "#735: multi-property edits should preserve unrelated VPOS values");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 333.0) < 0.001,
            "#735: multi-property edits should update selected HPOS values");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 444.0) < 0.001,
            "#735: multi-property edits should update selected VPOS values");
    }

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#735: multi-property edits should keep existing per-property undo compatibility");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#735: multi-property edits should make each changed property undoable");
    parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#735: multi-property geometry fixture should remain readable after undo");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#735: multi-property undo should restore selected HPOS values");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 322.0) < 0.001,
            "#735: multi-property undo should restore selected VPOS values");
    }
    const auto empty_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {}
    });
    expect(!empty_result.ok, "#735: empty multi-property edit requests should fail explicitly");
    expect(empty_result.affected_object_count == 0U,
        "#1007: empty multi-property edit should report zero affected objects");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_properties_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_multi_property_rollback_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "geometry_rollback.scx";
    const fs::path memo_path = temp_dir / "geometry_rollback.sct";
    write_synthetic_named_geometry_asset(table_path, memo_path);

    const auto update_result = copperfin::vfp::update_visual_object_properties({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "target-guid",
        .properties = {
            {.property_name = "HPOS", .property_value = "333.000"},
            {.property_name = "NOT_A_FIELD", .property_value = "444.000"}
        }
    });
    expect(!update_result.ok, "#740: failing multi-property edits should report the failed property change");
    expect(update_result.affected_object_count == 0U,
        "#1007: failed multi-property edit should report zero affected objects");

    const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 2U);
    expect(parse_result.ok, "#740: rollback geometry fixture should remain readable");
    if (parse_result.ok && parse_result.table.records.size() == 2U) {
        const auto* second_hpos = find_record_field(parse_result.table.records[1], "HPOS");
        const auto* second_vpos = find_record_field(parse_result.table.records[1], "VPOS");
        expect(second_hpos != nullptr && std::abs(parse_number(second_hpos->display_value) - 222.0) < 0.001,
            "#740: failed multi-property edits should restore earlier successful field changes");
        expect(second_vpos != nullptr && std::abs(parse_number(second_vpos->display_value) - 322.0) < 0.001,
            "#740: failed multi-property edits should leave later untouched fields unchanged");
    }
    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#740: failed multi-property rollback should not leave extra undo history");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_batch_rolls_back_failed_alignment() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "HPOS", .type = 'C', .length = 10U},
        {.name = "VPOS", .type = 'C', .length = 10U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {"cmdSave", "saveButton", "save-guid", "10", "20", "Caption = \"Save\"\r\n"},
        {"txtName", "nameBox", "name-guid", "30", "40", "Caption = \"Name\"\r\n"},
        {"lblStatus", "statusLabel", "status-guid", "50", "60", "Caption = \"Status\"\r\n"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#752: batch-edit fixture should be writable");

    const auto property_value = [&](const std::string& unique_id, const std::string& property_name) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, "#752: batch-edit fixture property should be readable");
        return result.value;
    };

    auto batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "save-guid",
                .properties = {
                    {.property_name = "HPOS", .property_value = "100"},
                    {.property_name = "VPOS", .property_value = "200"}
                }
            },
            {
                .record_index = 0U,
                .object_name = "txtName",
                .unique_id = {},
                .properties = {
                    {.property_name = "HPOS", .property_value = "100"},
                    {.property_name = "VPOS", .property_value = "300"}
                }
            }
        }
    });
    expect(batch_result.ok, "#752: batch edits should apply multi-object geometry changes");
    expect(batch_result.affected_object_count == 2U,
        "#998: successful batch edits should report affected object count");
    expect(property_value("save-guid", "HPOS") == "100" &&
            property_value("save-guid", "VPOS") == "200",
        "#752: batch edits should update UNIQUEID-selected geometry");
    expect(property_value("name-guid", "HPOS") == "100" &&
            property_value("name-guid", "VPOS") == "300",
        "#752: batch edits should update object-name-selected geometry");
    expect(property_value("status-guid", "HPOS") == "50" &&
            property_value("status-guid", "VPOS") == "60",
        "#752: batch edits should preserve unrelated records");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#752: successful batch edits should leave normal visual undo history available");

    batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {
            {
                .record_index = 0U,
                .object_name = "cmdSave",
                .unique_id = {},
                .properties = {
                    {.property_name = "HPOS", .property_value = "400"}
                }
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "name-guid",
                .properties = {}
            }
        }
    });
    expect(!batch_result.ok, "#752: batch edits should fail explicitly on an empty item property list");
    expect(batch_result.affected_object_count == 0U,
        "#998: failed batch edits should report zero affected objects after rollback");
    expect(property_value("save-guid", "HPOS") == "100",
        "#752: failed batch edits should roll back earlier successful object edits");
    const auto undo_after_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failure.available == undo_before_failure.available &&
            undo_after_failure.label == undo_before_failure.label,
        "#752: failed batch rollback should clean up undo entries created by the failed batch");

    batch_result = copperfin::vfp::update_visual_object_batch({
        .path = table_path.string(),
        .objects = {}
    });
    expect(!batch_result.ok, "#752: empty batch edit requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#998: empty batch edit requests should report zero affected objects");
    expect(property_value("save-guid", "HPOS") == "100" &&
            property_value("name-guid", "VPOS") == "300",
        "#752: empty batch edit requests should not mutate existing geometry");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_batch_undoes_report_and_label_batches_in_single_step() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_report_batch_undo_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("batch" + table_extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "WIDTH", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 4U}
        };
        const std::vector<std::vector<std::string>> records{
            {"8", "1200", "2400", "customer.company"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, asset_label + " batch-undo fixture should be writable");

        const auto batch_result = copperfin::vfp::update_visual_object_batch({
            .path = table_path.string(),
            .objects = {
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = {},
                    .properties = {
                        {.property_name = "HPOS", .property_value = "1800"},
                        {.property_name = "WIDTH", .property_value = "3200"},
                        {.property_name = "EXPR", .property_value = "\"updated.expr\""}
                    }
                }
            }
        });
        expect(batch_result.ok, asset_label + " batch property edits should succeed");
        expect(batch_result.affected_object_count == 1U,
            asset_label + " batch property edits should report one affected object");

        auto undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(undo_status.available, asset_label + " batch property edits should expose undo");
        expect(undo_status.label.find("EXPR") != std::string::npos,
            asset_label + " batch property edits should keep the latest-property undo label");

        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, asset_label + " batch property edits should undo in a single command");

        const auto parse_result = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), 1U);
        expect(parse_result.ok, asset_label + " fixture should remain readable after the command undo");
        if (parse_result.ok && parse_result.table.records.size() == 1U) {
            const auto& record = parse_result.table.records[0];
            for (const auto& value : record.values) {
                if (value.field_name == "HPOS") {
                    expect(value.display_value == "1200", asset_label + " command undo should restore HPOS");
                }
                if (value.field_name == "WIDTH") {
                    expect(value.display_value == "2400", asset_label + " command undo should restore WIDTH");
                }
                if (value.field_name == "EXPR") {
                    expect(value.display_value == "customer.company", asset_label + " command undo should restore EXPR");
                }
            }
        }

        undo_status = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(!undo_status.available, asset_label + " command undo should consume the only report batch history entry");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", "Report");
    exercise_asset("lbx", ".lbx", "Label");
}

void test_update_visual_object_batch_moves_report_and_label_band_contents() {
    namespace fs = std::filesystem;
    const auto exercise_asset = [&](const std::string& stem,
                                    const std::string& table_extension,
                                    const std::string& memo_extension,
                                    const std::string& asset_label) {
        const fs::path temp_dir = fs::temp_directory_path() /
            ("copperfin_visual_editor_band_batch_" + stem + "_tests_" + std::to_string(_getpid()));
        std::error_code ignored;
        fs::remove_all(temp_dir, ignored);
        fs::create_directories(temp_dir);

        const fs::path table_path = temp_dir / ("band_batch" + table_extension);
        const fs::path memo_path = temp_dir / ("band_batch" + memo_extension);
        const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
            {.name = "OBJTYPE", .type = 'N', .length = 8U},
            {.name = "OBJCODE", .type = 'N', .length = 8U},
            {.name = "VPOS", .type = 'N', .length = 10U},
            {.name = "HEIGHT", .type = 'N', .length = 10U},
            {.name = "HPOS", .type = 'N', .length = 10U},
            {.name = "WIDTH", .type = 'N', .length = 10U},
            {.name = "EXPR", .type = 'M', .length = 4U},
            {.name = "UNIQUEID", .type = 'C', .length = 24U},
            {.name = "USERFLAG", .type = 'C', .length = 24U}
        };
        const std::vector<std::vector<std::string>> records{
            {"9", "0", "100", "100", "0", "0", "band.one", "band-one", "band-one-meta"},
            {"8", "0", "120", "10", "10", "20", "object.one", "object-one", "object-one-meta"},
            {"9", "0", "300", "100", "0", "0", "band.two", "band-two", "band-two-meta"},
            {"8", "0", "320", "10", "30", "20", "object.two", "object-two", "object-two-meta"},
            {"8", "0", "600", "10", "50", "20", "unplaced.object", "unplaced", "unplaced-meta"},
            {"8", "0", "800", "10", "70", "20", "deleted.object", "deleted", "deleted-meta"}
        };
        const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
        expect(create_result.ok, "#3919: " + asset_label + " band-batch fixture should be writable");
        expect(fs::exists(memo_path), "#3919: " + asset_label + " band-batch fixture should create its memo sidecar");
        const auto delete_result = copperfin::vfp::set_record_deleted_flag(table_path.string(), 5U, true);
        expect(delete_result.ok, "#3919: " + asset_label + " band-batch fixture should retain a deleted row");

        const auto initial_memo_bytes = read_file_bytes(memo_path);
        const copperfin::vfp::VisualObjectBatchEditRequest request{
            .path = table_path.string(),
            .objects = {
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "band-one",
                    .properties = {
                        {.property_name = "VPOS", .property_value = "150"},
                        {.property_name = "HPOS", .property_value = "5"}
                    }
                },
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "band-two",
                    .properties = {
                        {.property_name = "VPOS", .property_value = "260"}
                    }
                },
                {
                    .record_index = 0U,
                    .object_name = {},
                    .unique_id = "object-one",
                    .properties = {
                        {.property_name = "HPOS", .property_value = "25"}
                    }
                }
            }
        };

        const auto batch_result = copperfin::vfp::update_visual_object_batch(request);
        expect(batch_result.ok, "#3919: " + asset_label + " mixed band batch should succeed");
        expect(batch_result.affected_object_count == request.objects.size(),
               "#3919: " + asset_label + " mixed band batch should retain public selection count");

        const auto expect_geometry = [&](std::size_t record_index,
                                         const std::string& expected_vpos,
                                         const std::string& expected_hpos,
                                         const std::string& context) {
            const auto parsed = copperfin::vfp::parse_dbf_table_from_file(
                table_path.string(),
                std::numeric_limits<std::size_t>::max());
            expect(parsed.ok && record_index < parsed.table.records.size(),
                   "#3919: " + asset_label + " " + context + " should reopen");
            if (!parsed.ok || record_index >= parsed.table.records.size()) {
                return;
            }
            const auto* vpos = find_record_field(parsed.table.records[record_index], "VPOS");
            const auto* hpos = find_record_field(parsed.table.records[record_index], "HPOS");
            expect(vpos != nullptr && vpos->display_value == expected_vpos &&
                       hpos != nullptr && hpos->display_value == expected_hpos,
                   "#3919: " + asset_label + " " + context + " geometry should match");
        };

        expect_geometry(0U, "150", "5", "positive-delta band");
        expect_geometry(1U, "170", "25", "positive-delta contained object");
        expect_geometry(2U, "260", "0", "negative-delta band");
        expect_geometry(3U, "280", "30", "negative-delta contained object");
        expect_geometry(4U, "600", "50", "unplaced sibling");
        expect_geometry(5U, "800", "70", "deleted sibling");

        auto parsed = copperfin::vfp::parse_dbf_table_from_file(
            table_path.string(),
            std::numeric_limits<std::size_t>::max());
        expect(parsed.ok && parsed.table.records.size() == records.size(),
               "#3919: " + asset_label + " mixed band batch should preserve record ordering");
        if (parsed.ok && parsed.table.records.size() == records.size()) {
            for (std::size_t index = 0U; index < records.size(); ++index) {
                const auto* unique_id = find_record_field(parsed.table.records[index], "UNIQUEID");
                const auto* user_flag = find_record_field(parsed.table.records[index], "USERFLAG");
                expect(unique_id != nullptr && unique_id->display_value == records[index][7],
                       "#3919: " + asset_label + " mixed band batch should preserve row order");
                expect(user_flag != nullptr && user_flag->display_value == records[index][8],
                       "#3919: " + asset_label + " mixed band batch should preserve unsupported fields");
            }
            expect(parsed.table.records[5].deleted,
                   "#3919: " + asset_label + " mixed band batch should preserve deleted rows");
        }
        expect(read_file_bytes(memo_path) == initial_memo_bytes,
               "#3919: " + asset_label + " direct geometry batch should preserve memo bytes");

        const auto updated_table_bytes = read_file_bytes(table_path);
        const auto updated_memo_bytes = read_file_bytes(memo_path);
        const auto undo_before_noop = copperfin::vfp::query_visual_object_undo(table_path.string());
        const auto repeat_result = copperfin::vfp::update_visual_object_batch(request);
        expect(repeat_result.ok, "#3919: " + asset_label + " repeated mixed band batch should succeed");
        expect(read_file_bytes(table_path) == updated_table_bytes &&
                   read_file_bytes(memo_path) == updated_memo_bytes,
               "#3919: " + asset_label + " repeated mixed band batch should be a byte no-op");
        const auto undo_after_noop = copperfin::vfp::query_visual_object_undo(table_path.string());
        expect(undo_after_noop.available == undo_before_noop.available &&
                   undo_after_noop.label == undo_before_noop.label,
               "#3919: " + asset_label + " repeated mixed band batch should not replace undo state");

        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#3919: " + asset_label + " mixed band batch should undo in one command");
        expect_geometry(0U, "100", "0", "undone first band");
        expect_geometry(1U, "120", "10", "undone first contained object");
        expect_geometry(2U, "300", "0", "undone second band");
        expect_geometry(3U, "320", "30", "undone second contained object");
        expect_geometry(4U, "600", "50", "undo-preserved unplaced sibling");
        expect_geometry(5U, "800", "70", "undo-preserved deleted sibling");
        expect(read_file_bytes(memo_path) == initial_memo_bytes,
               "#3919: " + asset_label + " command undo should preserve memo bytes");
        expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
               "#3919: " + asset_label + " command undo should consume the grouped history");

        const auto multi_result = copperfin::vfp::update_visual_object_properties({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = "band-one",
            .properties = {
                {.property_name = "VPOS", .property_value = "90"},
                {.property_name = "VPOS", .property_value = "75"},
                {.property_name = "WIDTH", .property_value = "15"}
            }
        });
        expect(multi_result.ok && multi_result.affected_object_count == 1U,
               "#3919: " + asset_label + " grouped multi-property band edit should succeed");
        expect_geometry(0U, "75", "0", "multi-property final band top");
        expect_geometry(1U, "95", "10", "multi-property contained-object delta");
        parsed = copperfin::vfp::parse_dbf_table_from_file(table_path.string(), records.size());
        if (parsed.ok && parsed.table.records.size() == records.size()) {
            const auto* width = find_record_field(parsed.table.records[0], "WIDTH");
            expect(width != nullptr && width->display_value == "15",
                   "#3919: " + asset_label + " grouped multi-property edit should retain sibling properties");
        }
        const auto multi_undo = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(multi_undo.ok,
               "#3919: " + asset_label + " grouped multi-property band edit should undo in one command");
        expect_geometry(0U, "100", "0", "multi-property undone band");
        expect_geometry(1U, "120", "10", "multi-property undone contained object");
        expect(read_file_bytes(memo_path) == initial_memo_bytes,
               "#3919: " + asset_label + " grouped multi-property path should preserve memo bytes");
        expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
               "#3919: " + asset_label + " grouped multi-property undo should consume history");

        fs::remove_all(temp_dir, ignored);
    };

    exercise_asset("frx", ".frx", ".frt", "Report");
    exercise_asset("lbx", ".lbx", ".lbt", "Label");
}
