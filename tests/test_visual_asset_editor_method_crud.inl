void test_query_visual_object_method_reads_one_selected_method() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_query_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_query.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nfUnCtIoN GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", "PROCEDURE LostFocus\r\nTHISFORM.ValidateName()\r\nENDPROC"}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#762: method-query fixture should be writable");

    auto query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 0U &&
            !query_result.record_deleted &&
            query_result.method.method_name == "Click" &&
            query_result.method.kind == "procedure" &&
            query_result.method.source_text == "THISFORM.Save()" &&
            query_result.method.source_line_index == 0U &&
            query_result.method.source_memo_block_number != 0U,
        "#762: method query should return one procedure with resolved record and source metadata");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSave",
        .unique_id = {},
        .method_name = "GetCaption"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.method.method_name == "GetCaption" &&
            query_result.method.kind == "function" &&
            query_result.method.source_text == "RETURN THIS.Caption",
        "#762: method query should support object-name selection and function declarations");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 1U,
        .object_name = {},
        .unique_id = {},
        .method_name = "LostFocus"
    });
    expect(query_result.ok &&
            query_result.exists &&
            query_result.record_index == 1U &&
            query_result.method.method_name == "LostFocus" &&
            query_result.method.source_text == "THISFORM.ValidateName()",
        "#762: method query should support direct record-index selection");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "Missing"
    });
    expect(query_result.ok && !query_result.exists && query_result.record_index == 0U,
        "#762: missing methods should return a successful not-found result");

    expect(!copperfin::vfp::query_visual_object_undo(table_path.string()).available,
        "#762: method query should not create undo history");

    const fs::path duplicate_path = temp_dir / "method_query_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC"
        }
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#762: duplicate method-query fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "CLICK"
    });
    expect(!query_result.ok, "#762: duplicate matching method names should fail as ambiguous");

    query_result = copperfin::vfp::query_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "   "
    });
    expect(!query_result.ok, "#762: empty method names should fail explicitly");

    const fs::path no_methods_path = temp_dir / "method_query_no_methods.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> no_methods_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> no_methods_records{
        {"cmdNoMethods", "no-methods-guid"}
    };
    const auto no_methods_create_result = copperfin::vfp::create_dbf_table_file(
        no_methods_path.string(),
        no_methods_fields,
        no_methods_records);
    expect(no_methods_create_result.ok, "#762: missing-METHODS fixture should be writable");
    query_result = copperfin::vfp::query_visual_object_method({
        .path = no_methods_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-methods-guid",
        .method_name = "Click"
    });
    expect(!query_result.ok, "#762: missing METHODS fields should fail explicitly");

    fs::remove_all(temp_dir, ignored);
}

void test_update_visual_object_method_updates_and_appends_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_edit_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_edit.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#747: method-edit fixture should be writable");

    auto update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click",
        .method_kind = "procedure",
        .source_text = "THISFORM.Save(.T.)"
    });
    expect(update_result.ok, "#747: method edits should update existing selected-object methods case-insensitively");
    expect(update_result.affected_object_count == 1U,
        "#1004: successful method update should report one affected object");

    update_result = copperfin::vfp::update_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "GetCaption",
        .method_kind = "function",
        .source_text = "RETURN THIS.Caption"
    });
    expect(update_result.ok, "#747: method edits should append missing selected-object methods");
    expect(update_result.affected_object_count == 1U,
        "#1004: successful method append should report one affected object");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: updated method fixture should remain readable");
    const auto* click = find_method_snapshot(method_result.methods, "Click");
    const auto* get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save(.T.)",
        "#747: method edits should replace existing method bodies while preserving declaration names");
    expect(get_caption != nullptr && get_caption->kind == "function" && get_caption->source_text == "RETURN THIS.Caption",
        "#747: method edits should append missing methods with requested kind and source");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#747: selected-object method edits should not mutate unrelated object records");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the appended method edit");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#747: undo should restore the replaced method edit");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#747: method fixture should remain readable after undo");
    click = find_method_snapshot(method_result.methods, "Click");
    get_caption = find_method_snapshot(method_result.methods, "GetCaption");
    expect(click != nullptr && click->source_text == "THISFORM.Save()",
        "#747: undo should restore original method source text");
    expect(get_caption == nullptr,
        "#747: undo should remove methods appended by the edit API");

    fs::remove_all(temp_dir, ignored);
}

void test_delete_visual_object_method_removes_selected_methods() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_delete_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_delete.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSave",
            "saveButton",
            "save-guid",
            "PROCEDURE Click\r\nTHISFORM.Save()\r\nENDPROC\r\nFUNCTION GetCaption\r\nRETURN THIS.Caption\r\nENDFUNC"
        },
        {"txtName", "nameBox", "name-guid", ""}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#748: method-delete fixture should be writable");

    auto delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "click"
    });
    expect(delete_result.ok, "#748: method deletes should remove existing selected-object methods case-insensitively");
    expect(delete_result.affected_object_count == 1U,
        "#1004: successful method delete should report one affected object");

    auto method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok, "#748: method-delete fixture should remain readable after delete");
    expect(find_method_snapshot(method_result.methods, "Click") == nullptr,
        "#748: method deletes should remove the full selected method block");
    expect(find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: method deletes should preserve unrelated methods in the same METHODS memo");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "txtName",
        .unique_id = {}
    });
    expect(method_result.ok && method_result.methods.empty(),
        "#748: selected-object method deletes should not mutate unrelated object records");

    delete_result = copperfin::vfp::delete_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid",
        .method_name = "DoesNotExist"
    });
    expect(!delete_result.ok, "#748: missing method deletes should fail explicitly");
    expect(delete_result.affected_object_count == 0U,
        "#1004: failed method delete should report zero affected objects");

    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") == nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: missing method deletes should not mutate the METHODS memo");

    const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, "#748: undo should restore the deleted method block");
    method_result = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "save-guid"
    });
    expect(method_result.ok &&
            find_method_snapshot(method_result.methods, "Click") != nullptr &&
            find_method_snapshot(method_result.methods, "GetCaption") != nullptr,
        "#748: undo should restore deleted methods while preserving unrelated methods");

    fs::remove_all(temp_dir, ignored);
}
