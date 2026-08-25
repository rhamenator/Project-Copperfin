void test_reorder_visual_object_methods_within_selected_object() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_reorder_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_reorder.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Alpha\r\nTHISFORM.Alpha()\r\nENDPROC\r\nFUNCTION Bravo\r\nRETURN THIS.Caption\r\nENDFUNC\r\nPROCEDURE Charlie\r\nTHISFORM.Charlie()\r\nENDPROC"
        },
        {
            "txtOther",
            "otherBox",
            "other-guid",
            "PROCEDURE Other\r\nTHISFORM.Other()\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#765: method-reorder fixture should be writable");

    const auto method_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto methods = copperfin::vfp::list_visual_object_methods({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!methods.ok) {
            return names;
        }
        for (const auto& method : methods.methods) {
            names.push_back(method.method_name);
        }
        return names;
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "charlie",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(reorder_result.ok, "#765: method reorder should support UNIQUEID selection and first placement");
    expect(reorder_result.affected_object_count == 1U,
        "#1004: successful method reorder should report one affected object");
    expect(order_is(method_order("source-guid"), {"Charlie", "Alpha", "Bravo"}),
        "#765: first placement should move the requested method to the start");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = "cmdSource",
        .unique_id = {},
        .method_name = "CHARLIE",
        .placement = "last",
        .relative_method_name = {}
    });
    expect(reorder_result.ok, "#765: method reorder should support object-name selection and last placement");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: last placement should move the requested method to the end");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = {},
        .method_name = "Alpha",
        .placement = "after",
        .relative_method_name = "Charlie"
    });
    expect(reorder_result.ok, "#765: method reorder should support record-index selection and after placement");
    expect(order_is(method_order("source-guid"), {"Bravo", "Charlie", "Alpha"}),
        "#765: after placement should move the requested method after the relative method");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "before",
        .relative_method_name = "Bravo"
    });
    expect(reorder_result.ok, "#765: method reorder should support before placement");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: before placement should move the requested method before the relative method");

    const auto methods = copperfin::vfp::list_visual_object_methods({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid"
    });
    const auto* alpha = methods.ok ? find_method_snapshot(methods.methods, "Alpha") : nullptr;
    const auto* bravo = methods.ok ? find_method_snapshot(methods.methods, "Bravo") : nullptr;
    const auto* charlie = methods.ok ? find_method_snapshot(methods.methods, "Charlie") : nullptr;
    expect(alpha != nullptr &&
            alpha->kind == "procedure" &&
            alpha->source_text == "THISFORM.Alpha()" &&
            bravo != nullptr &&
            bravo->kind == "function" &&
            bravo->source_text == "RETURN THIS.Caption" &&
            charlie != nullptr &&
            charlie->kind == "procedure" &&
            charlie->source_text == "THISFORM.Charlie()",
        "#765: method reorder should preserve method names, kinds, and source bodies");

    expect(order_is(method_order("other-guid"), {"Other"}),
        "#765: method reorder should preserve unrelated object METHODS memos");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Missing",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing methods");
    expect(reorder_result.affected_object_count == 0U,
        "#1004: failed method reorder should report zero affected objects");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "before",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing relative methods for before placement");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "after",
        .relative_method_name = "alpha"
    });
    expect(!reorder_result.ok, "#765: method reorder should reject self-relative before/after placement");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = "Alpha",
        .placement = "middle",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject unknown placements");

    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = table_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "source-guid",
        .method_name = " ",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject empty method names");

    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: failed method reorders should not mutate the METHODS memo");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#765: undo should restore each successful method reorder");
    }
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie"}),
        "#765: undo should restore original method ordering");

    const fs::path duplicate_path = temp_dir / "method_reorder_duplicate.scx";
    const std::vector<std::vector<std::string>> duplicate_records{
        {
            "cmdDup",
            "dupButton",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\nPROCEDURE click\r\nRETURN 2\r\nENDPROC\r\nPROCEDURE Anchor\r\nRETURN 3\r\nENDPROC"
        }
    };
    const auto duplicate_create_result = copperfin::vfp::create_dbf_table_file(
        duplicate_path.string(),
        fields,
        duplicate_records);
    expect(duplicate_create_result.ok, "#765: duplicate method-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "CLICK",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject duplicate source declarations as ambiguous");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = duplicate_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "dup-guid",
        .method_name = "Anchor",
        .placement = "before",
        .relative_method_name = "Click"
    });
    expect(!reorder_result.ok, "#765: method reorder should reject duplicate relative declarations as ambiguous");

    const fs::path no_methods_path = temp_dir / "method_reorder_no_methods.scx";
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
    expect(no_methods_create_result.ok, "#765: missing-METHODS method-reorder fixture should be writable");
    reorder_result = copperfin::vfp::reorder_visual_object_method({
        .path = no_methods_path.string(),
        .record_index = 0U,
        .object_name = {},
        .unique_id = "no-methods-guid",
        .method_name = "Click",
        .placement = "first",
        .relative_method_name = {}
    });
    expect(!reorder_result.ok, "#765: method reorder should reject missing METHODS fields");

    fs::remove_all(temp_dir, ignored);
}

void test_reorder_visual_object_methods_rolls_back_failed_batches() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_method_reorder_batch_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "method_reorder_batch.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "NAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U},
        {.name = "METHODS", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> records{
        {
            "cmdSource",
            "sourceButton",
            "source-guid",
            "PROCEDURE Alpha\r\nTHISFORM.Alpha()\r\nENDPROC\r\n"
            "FUNCTION Bravo\r\nRETURN THIS.Caption\r\nENDFUNC\r\n"
            "PROCEDURE Charlie\r\nTHISFORM.Charlie()\r\nENDPROC\r\n"
            "PROCEDURE Delta\r\nTHISFORM.Delta()\r\nENDPROC"
        },
        {
            "txtName",
            "nameBox",
            "name-guid",
            "PROCEDURE One\r\nTHISFORM.One()\r\nENDPROC\r\n"
            "PROCEDURE Two\r\nTHISFORM.Two()\r\nENDPROC\r\n"
            "FUNCTION Three\r\nRETURN 3\r\nENDFUNC"
        },
        {
            "lblStatus",
            "statusLabel",
            "status-guid",
            "PROCEDURE Red\r\nTHISFORM.Red()\r\nENDPROC\r\n"
            "FUNCTION Green\r\nRETURN .T.\r\nENDFUNC\r\n"
            "PROCEDURE Blue\r\nTHISFORM.Blue()\r\nENDPROC"
        },
        {
            "dupObj",
            "dupName",
            "dup-guid",
            "PROCEDURE Click\r\nRETURN 1\r\nENDPROC\r\n"
            "PROCEDURE click\r\nRETURN 2\r\nENDPROC\r\n"
            "PROCEDURE Anchor\r\nRETURN 3\r\nENDPROC"
        }
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#780: method-reorder-batch fixture should be writable");

    const auto method_order = [&](const std::string& unique_id) {
        std::vector<std::string> names;
        const auto methods = copperfin::vfp::list_visual_object_methods({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id
        });
        if (!methods.ok) {
            return names;
        }
        for (const auto& method : methods.methods) {
            names.push_back(method.method_name);
        }
        return names;
    };
    const auto method_state = [&](const std::string& unique_id, const std::string& method_name) {
        return copperfin::vfp::query_visual_object_method({
            .path = table_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .method_name = method_name
        });
    };
    const auto order_is = [](const std::vector<std::string>& names, std::initializer_list<const char*> expected) {
        if (names.size() != expected.size()) {
            return false;
        }
        auto name_it = names.begin();
        auto expected_it = expected.begin();
        for (; name_it != names.end(); ++name_it, ++expected_it) {
            if (*name_it != *expected_it) {
                return false;
            }
        }
        return true;
    };

    auto batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "charlie",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = "cmdSource",
                .unique_id = {},
                .method_name = "CHARLIE",
                .placement = "last",
                .relative_method_name = {}
            },
            {
                .record_index = 1U,
                .object_name = {},
                .unique_id = {},
                .method_name = "One",
                .placement = "after",
                .relative_method_name = "Three"
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "status-guid",
                .method_name = "Blue",
                .placement = "before",
                .relative_method_name = "Green"
            }
        }
    });
    expect(batch_result.ok, "#780: batch method reorder should support mixed selectors and all placements");
    expect(batch_result.affected_object_count == 4U,
        "#1004: successful batch method reorder should report affected item count");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}) &&
            order_is(method_order("name-guid"), {"Two", "Three", "One"}) &&
            order_is(method_order("status-guid"), {"Red", "Blue", "Green"}),
        "#780: batch method reorder should persist expected method ordering");

    const auto alpha = method_state("source-guid", "Alpha");
    const auto bravo = method_state("source-guid", "Bravo");
    const auto blue = method_state("status-guid", "Blue");
    expect(alpha.ok && alpha.exists && alpha.method.source_text == "THISFORM.Alpha()" &&
            bravo.ok && bravo.exists && bravo.method.kind == "function" &&
            blue.ok && blue.exists && blue.method.source_text == "THISFORM.Blue()",
        "#780: batch method reorder should preserve method names, kinds, and source bodies");

    const auto undo_before_failure = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_before_failure.available,
        "#780: successful batch reorders should leave normal visual undo history available");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Missing",
                .placement = "first",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject missing methods");
    expect(batch_result.affected_object_count == 0U,
        "#1004: failed batch method reorder should report zero affected objects");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}),
        "#780: missing-method failures should roll back earlier method reorders");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Alpha",
                .placement = "before",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject missing relative names");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}),
        "#780: missing-relative failures should roll back earlier method reorders");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = " ",
                .placement = "last",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject empty method names");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}),
        "#780: empty-name failures should roll back earlier method reorders");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Alpha",
                .placement = "middle",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject unknown placements");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}),
        "#780: unknown-placement failures should roll back earlier method reorders");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Alpha",
                .placement = "after",
                .relative_method_name = "alpha"
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject self-relative placement");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}),
        "#780: self-relative failures should roll back earlier method reorders");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .method_name = "Click",
                .placement = "first",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject duplicate source methods");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}) &&
            order_is(method_order("dup-guid"), {"Click", "click", "Anchor"}),
        "#780: duplicate-source failures should roll back earlier method reorders and report ambiguity");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "source-guid",
                .method_name = "Delta",
                .placement = "first",
                .relative_method_name = {}
            },
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "dup-guid",
                .method_name = "Anchor",
                .placement = "before",
                .relative_method_name = "Click"
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject duplicate relative methods");
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Delta", "Charlie"}) &&
            order_is(method_order("dup-guid"), {"Click", "click", "Anchor"}),
        "#780: duplicate-relative failures should roll back earlier method reorders and report ambiguity");

    const auto undo_after_failures = copperfin::vfp::query_visual_object_undo(table_path.string());
    expect(undo_after_failures.available == undo_before_failure.available &&
            undo_after_failures.label == undo_before_failure.label,
        "#780: failed batch reorder rollbacks should preserve prior undo history");

    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = table_path.string(),
        .methods = {}
    });
    expect(!batch_result.ok, "#780: empty batch method reorder requests should fail explicitly");
    expect(batch_result.affected_object_count == 0U,
        "#1004: empty batch method reorder should report zero affected objects");

    const fs::path unsupported_path = temp_dir / "method_reorder_batch_unsupported.scx";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> unsupported_fields{
        {.name = "OBJNAME", .type = 'C', .length = 16U},
        {.name = "UNIQUEID", .type = 'C', .length = 16U}
    };
    const std::vector<std::vector<std::string>> unsupported_records{
        {"cmdUnsupported", "unsupported-guid"}
    };
    const auto unsupported_create = copperfin::vfp::create_dbf_table_file(
        unsupported_path.string(),
        unsupported_fields,
        unsupported_records);
    expect(unsupported_create.ok, "#780: unsupported method-reorder-batch fixture should be writable");
    batch_result = copperfin::vfp::reorder_visual_object_methods({
        .path = unsupported_path.string(),
        .methods = {
            {
                .record_index = 0U,
                .object_name = {},
                .unique_id = "unsupported-guid",
                .method_name = "Click",
                .placement = "first",
                .relative_method_name = {}
            }
        }
    });
    expect(!batch_result.ok, "#780: batch method reorder should reject objects without METHODS carriers");

    for (int index = 0; index < 4; ++index) {
        const auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
        expect(undo_result.ok, "#780: undo should restore each successful batch method reorder");
    }
    expect(order_is(method_order("source-guid"), {"Alpha", "Bravo", "Charlie", "Delta"}) &&
            order_is(method_order("name-guid"), {"One", "Two", "Three"}) &&
            order_is(method_order("status-guid"), {"Red", "Green", "Blue"}),
        "#780: successful batch reorder undo should restore original method ordering");

    fs::remove_all(temp_dir, ignored);
}
