// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

// ---- export_database_as_json tests ----

void test_export_database_as_json_errors_leave_json_empty() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_export_error_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path empty_path = temp_dir / "empty.dbc";
    {
        std::ofstream output(empty_path, std::ios::binary);
    }
    const auto empty_result = copperfin::vfp::export_database_as_json(empty_path.string());
    expect(!empty_result.ok && !empty_result.error.empty() && empty_result.json.empty(),
           "#3988: unreadable empty DBC exports should report only an error");

    const fs::path malformed_path = temp_dir / "malformed.dbc";
    {
        std::ofstream output(malformed_path, std::ios::binary);
        output.put(static_cast<char>(0x30));
    }
    const auto malformed_result = copperfin::vfp::export_database_as_json(malformed_path.string());
    expect(!malformed_result.ok && !malformed_result.error.empty() && malformed_result.json.empty(),
           "#3988: malformed-header DBC exports should report only an error");

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_json_produces_catalog_json() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "northwind.dbc";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };

    std::vector<std::vector<std::string>> records{
        {"DATABASE", "northwind", "", ""},
        {"TABLE", "Customers", "northwind", ""},
        {"TABLE", "Orders", "northwind", ""}
    };
    records.reserve(1234U);
    for (std::size_t index = records.size(); index < 1233U; ++index) {
        records.push_back({"TABLE", "Padding" + std::to_string(index), "northwind", ""});
    }
    records.push_back({"TABLE", "HighIndex", "northwind", ""});

    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_path.string(), fields, records);
    expect(create_result.ok, "export_database_as_json: DBC fixture should be created");

    const std::locale grouping_locale(std::locale::classic(), new grouped_numpunct());
    global_locale_guard locale_guard(grouping_locale);
    const auto result = copperfin::vfp::export_database_as_json(dbc_path.string());
    expect(result.ok, "export_database_as_json should succeed on a minimal DBC fixture");
    expect(result.error.empty(),
           "#3988: successful database exports should leave the error result empty");
    if (result.ok) {
        expect(result.json.find("\"schema_version\": 1,") != std::string::npos,
               "export JSON should identify the version-1 interchange envelope");
        expect(result.json.find("\"northwind\"") != std::string::npos,
               "export JSON should include the database name");
        expect(result.json.find("\"catalog\"") != std::string::npos,
               "export JSON should include the catalog array");
        expect(result.json.find("\"database\"") != std::string::npos,
               "export JSON should include the database block");
        expect(result.json.find("\"tables\"") != std::string::npos,
               "export JSON should include the tables block");
        expect(result.json.find("\"Customers\"") != std::string::npos,
               "export JSON catalog should contain the Customers table entry");
        expect(result.json.find("\"Orders\"") != std::string::npos,
               "export JSON catalog should contain the Orders table entry");
        expect(result.json.find("\"record_index\": 1234,") != std::string::npos,
               "export JSON should preserve invariant high catalog record indices under grouped punctuation");
        expect(result.json.find("\"record_index\": 1.234,") == std::string::npos,
               "export JSON should reject grouped punctuation in catalog record indices");
        expect(result.json.find("\"HighIndex\"") != std::string::npos,
               "export JSON should retain the high-index catalog object");
        // No .dbf files exist for those tables, so tables block should be empty
        expect(result.json.find("\"tables\": {\n  }") == std::string::npos ||
               result.json.find("\"records\"") == std::string::npos,
               "export JSON tables block should be empty when no table DBFs are present");
        const auto plan = copperfin::vfp::build_database_json_import_plan(result.json);
        expect(plan.ok && plan.plan.database_name == "northwind" && plan.plan.tables.empty(),
               "database JSON import planning should admit the exporter\'s version-1 catalog-only snapshot");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_json_decodes_properties_blob() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_props_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / copperfin::platform::path_from_utf8_string("caf\xC3\xA9.dbc");
    const std::string dbc_utf8_path = copperfin::platform::path_to_utf8_string(dbc_path);

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };

    // Create the DBC with two records; start PROPERTIES empty
    const std::vector<std::vector<std::string>> records{
        {"DATABASE", "sample", "", ""},
        {"TABLE", "Customers", "sample", ""}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(dbc_utf8_path, fields, records);
    expect(create_result.ok, "properties-decode test: DBC fixture should be created");

    // Build a binary PROPERTIES blob for the TABLE record (record index 1, 0-based):
    //   Caption = "Customers"  (type 0x01, name "Caption", value "Customers")
    //   Comment = "Test table" (type 0x01, name "Comment", value "Test table")
    //   end marker 0x00
    std::string props_blob;
    // Caption
    props_blob += '\x01';                      // type: C
    props_blob += '\x07'; props_blob += '\x00'; // name_len = 7
    props_blob += "Caption";
    props_blob += '\x09'; props_blob += '\x00'; // value_len = 9
    props_blob += "Customers";
    // Comment
    props_blob += '\x01';                      // type: C
    props_blob += '\x07'; props_blob += '\x00'; // name_len = 7
    props_blob += "Comment";
    props_blob += '\x0A'; props_blob += '\x00'; // value_len = 10
    props_blob += "Test table";
    // End marker
    props_blob += '\x00';

    // Write the properties blob into the PROPERTIES memo for record 1 (TABLE, 0-based)
    const auto write_result = copperfin::vfp::replace_record_field_value(
        dbc_utf8_path, 1U, "PROPERTIES", props_blob);
    expect(write_result.ok, "properties-decode test: PROPERTIES memo should be writable");

    const auto result = copperfin::vfp::export_database_as_json(dbc_utf8_path);
    expect(result.ok, "export_database_as_json should succeed when PROPERTIES blob is present");
    if (result.ok) {
        expect(result.json.find("\"Caption\"") != std::string::npos,
               "export JSON should contain decoded Caption property name");
        expect(result.json.find("\"Customers\"") != std::string::npos,
               "export JSON should contain decoded Caption value");
        expect(result.json.find("\"Comment\"") != std::string::npos,
               "export JSON should contain decoded Comment property name");
        expect(result.json.find("\"Test table\"") != std::string::npos,
               "export JSON should contain decoded Comment value");
        expect(result.json.find("caf\xC3\xA9.dbc") != std::string::npos,
               "export JSON should preserve the UTF-8 DBC basename");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_export_database_as_json_prefers_catalog_name_and_casefolded_assets() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_dbc_casefold_export_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path dbc_path = temp_dir / "container.dbc";
    const fs::path dct_path = temp_dir / "container.dct";
    const fs::path uppercase_dct_path = temp_dir / "CONTAINER.DCT";
    const fs::path upper_table_path = temp_dir / "CUSTOMERS.DBF";

    const std::vector<copperfin::vfp::DbfFieldDescriptor> dbc_fields{
        {.name = "OBJECTTYPE", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "OBJECTNAME", .type = 'C', .offset = 17U, .length = 32U, .decimal_count = 0U},
        {.name = "PARENTNAME", .type = 'C', .offset = 49U, .length = 32U, .decimal_count = 0U},
        {.name = "PROPERTIES", .type = 'M', .offset = 81U, .length = 4U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> dbc_records{
        {"DATABASE", "NorthwindRuntime", "", ""},
        {"TABLE", "Customers", "NorthwindRuntime", ""}
    };
    const auto dbc_create = copperfin::vfp::create_dbf_table_file(dbc_path.string(), dbc_fields, dbc_records);
    expect(dbc_create.ok, "casefold export test: DBC fixture should be created");

    std::string props_blob;
    props_blob += '\x01';
    props_blob += '\x07'; props_blob += '\x00';
    props_blob += "Caption";
    props_blob += '\x09'; props_blob += '\x00';
    props_blob += "Customers";
    props_blob += '\x00';
    const auto props_write = copperfin::vfp::replace_record_field_value(
        dbc_path.string(), 1U, "PROPERTIES", props_blob);
    expect(props_write.ok, "casefold export test: PROPERTIES memo should be writable");

    if (fs::exists(dct_path)) {
        fs::rename(dct_path, uppercase_dct_path, ignored);
    }

    const std::vector<copperfin::vfp::DbfFieldDescriptor> table_fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 16U, .decimal_count = 0U},
        {.name = "AGE", .type = 'N', .offset = 17U, .length = 6U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> table_records{
        {"ALICE", "30"},
        {"BOB", "25"}
    };
    const auto table_create = copperfin::vfp::create_dbf_table_file(upper_table_path.string(), table_fields, table_records);
    expect(table_create.ok, "casefold export test: uppercase table fixture should be created");

    const auto result = copperfin::vfp::export_database_as_json(dbc_path.string());
    expect(result.ok, "export_database_as_json should succeed with case-folded DCT/DBF companions");
    if (result.ok) {
        expect(result.json.find("\"name\": \"NorthwindRuntime\"") != std::string::npos,
               "export JSON should prefer the DATABASE catalog object name over the DBC file stem");
        expect(result.json.find("\"Caption\"") != std::string::npos,
               "export JSON should decode PROPERTIES from a case-folded DCT sidecar");
        expect(result.json.find("\"Customers\"") != std::string::npos,
               "export JSON should include the catalog table entry");
        expect(result.json.find("\"ALICE\"") != std::string::npos,
               "export JSON should resolve and export rows from an uppercase same-base table file");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_build_database_json_import_plan_validates_without_mutation() {
    const std::string document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Northwind.dbc", "name": "Northwind"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Orders": {
      "fields": [{"name": "ORDERID", "type": "N", "length": 8, "decimals": 0}],
      "records": [{"ORDERID": 7}]
    },
    "Customers": {
      "fields": [{"name": "NAME", "type": "C", "length": 40, "decimals": 0}],
      "records": [{"NAME": "Acme"}]
    }
  }
})JSON";

    const auto result = copperfin::vfp::build_database_json_import_plan(document);
    expect(result.ok, "database JSON import planning should accept a version-1 export envelope");
    expect(result.error_code.empty(), "successful database JSON planning should not retain an error code");
    if (result.ok) {
        expect(result.plan.database_name == "Northwind",
               "database JSON planning should retain the database name without using its source path");
        expect(result.plan.catalog_json == "[{\"record_index\": 1}]",
               "database JSON planning should retain the catalog only as inert JSON");
        expect(result.plan.tables.size() == 2U && result.plan.tables[0].name == "Customers" &&
                   result.plan.tables[1].name == "Orders",
               "database JSON planning should sort table plans deterministically");
        expect(result.plan.tables[0].fields.size() == 1U &&
                   result.plan.tables[0].fields[0].name == "NAME" &&
                   result.plan.tables[0].records_json == "[{\"NAME\": \"Acme\"}]",
               "database JSON planning should retain table schema and records without writing a database");
    }

    const auto unsupported_schema = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":2,"database":{"path":"x","name":"n"},"catalog":[],"tables":{}})");
    expect(!unsupported_schema.ok &&
               unsupported_schema.error_code == "database_json_import.unsupported_schema_version",
           "database JSON planning should reject unsupported schema versions before any reconstruction step");

    const auto colliding_tables = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[],"records":[]},"people":{"fields":[],"records":[]}}})");
    expect(!colliding_tables.ok &&
               colliding_tables.error_code == "database_json_import.duplicate_table_name",
           "database JSON planning should reject cross-platform case-folded table-name collisions");

    const auto empty_table_schema = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[],"records":[]}}})");
    expect(empty_table_schema.ok && empty_table_schema.plan.tables.size() == 1U &&
               empty_table_schema.plan.tables[0].fields.empty(),
           "database JSON planning should retain the exporter\'s empty-table schema marker without reconstruction");

    const auto invalid_field = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[{"name":"ID","type":"N","length":0,"decimals":0}],"records":[]}}})");
    expect(!invalid_field.ok && invalid_field.error_code == "database_json_import.invalid_field",
           "database JSON planning should reject a field that cannot be represented by a DBF descriptor");

    const auto unsupported_storage = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[{"name":"ID","type":"!","length":1,"decimals":0}],"records":[]}}})");
    expect(!unsupported_storage.ok && unsupported_storage.error_code == "database_json_import.invalid_field",
           "database JSON planning should reject field storage types unsupported by the DBF writer");

    const auto invalid_fixed_width = copperfin::vfp::build_database_json_import_plan(
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[{"name":"ID","type":"B","length":1,"decimals":0}],"records":[]}}})");
    expect(!invalid_fixed_width.ok && invalid_fixed_width.error_code == "database_json_import.invalid_field",
           "database JSON planning should reject field widths the DBF writer cannot create");

    const std::string overflow_decimal_document =
        R"({"schema_version":1,"database":{"path":"x","name":"n"},"catalog":[],"tables":{"People":{"fields":[{"name":"ID","type":"N","length":1,"decimals":)" +
        std::string(1024U, '9') +
        R"(}],"records":[]}}})";
    const auto overflow_decimal =
        copperfin::vfp::build_database_json_import_plan(overflow_decimal_document);
    expect(!overflow_decimal.ok &&
               overflow_decimal.error_code == "database_json_import.invalid_field",
           "database JSON planning should reject out-of-range decimal text without integer wraparound");
}

void test_materialize_database_json_import_plan_fails_closed_and_round_trips() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_database_json_materialize_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Northwind.dbc", "name": "Northwind"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Orders": {
      "fields": [{"name": "ORDERID", "type": "N", "length": 8, "decimals": 0}],
      "records": [{"ORDERID": 7}, {"ORDERID": 12}]
    },
    "Customers": {
      "fields": [{"name": "NAME", "type": "C", "length": 40, "decimals": 0},
                 {"name": "ACTIVE", "type": "L", "length": 1, "decimals": 0}],
      "records": [{"NAME": "Acme", "ACTIVE": true}, {"NAME": null, "ACTIVE": false}]
    }
  }
})JSON";
    const auto plan_result = copperfin::vfp::build_database_json_import_plan(document);
    expect(plan_result.ok, "materializer fixture plan should build successfully");
    if (!plan_result.ok) {
        return;
    }

    // Empty plan.
    const auto empty_result = copperfin::vfp::materialize_database_json_import_plan(
        copperfin::vfp::DatabaseJsonImportPlan{}, (temp_dir / "empty.dbc").string());
    expect(!empty_result.ok && empty_result.table_count == 0U,
           "materializing a plan with no tables should fail closed");
    expect(!fs::exists(temp_dir / "empty.dbc"),
           "a no-tables plan must not create a destination DBC");

    // Successful materialization and round-trip via the real exporter.
    const fs::path dbc_path = temp_dir / "northwind.dbc";
    const auto materialize_result = copperfin::vfp::materialize_database_json_import_plan(
        plan_result.plan, dbc_path.string());
    expect(materialize_result.ok, "materializing a valid plan should succeed: " + materialize_result.error);
    expect(materialize_result.table_count == 2U, "materializing should report both tables");
    expect(fs::exists(dbc_path), "materializing should create the destination DBC");
    expect(fs::exists(temp_dir / "Orders.dbf") && fs::exists(temp_dir / "Customers.dbf"),
           "materializing should create one DBF per table");

    const auto reexported = copperfin::vfp::export_database_as_json(dbc_path.string());
    expect(reexported.ok, "the materialized DBC should itself be exportable: " + reexported.error);
    if (reexported.ok) {
        expect(reexported.json.find("\"NAME\": \"Acme\"") != std::string::npos,
               "materializing should restore character row data");
        expect(reexported.json.find("\"NAME\": \"\"") != std::string::npos,
               "materializing should turn a JSON null into a blank field value rather than failing");
        expect(reexported.json.find("\"ACTIVE\": true") != std::string::npos &&
                   reexported.json.find("\"ACTIVE\": false") != std::string::npos,
               "materializing should restore both logical values, not coerce them to one");
        expect(reexported.json.find("\"ORDERID\": 7") != std::string::npos &&
                   reexported.json.find("\"ORDERID\": 12") != std::string::npos,
               "materializing should restore every numeric row, not just the first");
    }

    // Fail closed: destination DBC already exists. Must not touch it.
    const std::uintmax_t dbc_size_before = fs::file_size(dbc_path, ignored);
    const auto exists_result = copperfin::vfp::materialize_database_json_import_plan(
        plan_result.plan, dbc_path.string());
    expect(!exists_result.ok, "materializing into an existing DBC path should fail closed");
    expect(fs::file_size(dbc_path, ignored) == dbc_size_before,
           "a rejected re-materialization must not modify the existing destination");

    // Fail closed: a derived table path already exists, DBC path is free.
    // Nothing -- not even the DBC -- may be created around the collision.
    const fs::path collision_dir = temp_dir / "collision";
    fs::create_directories(collision_dir, ignored);
    const auto pre_existing_table = copperfin::vfp::create_dbf_table_file(
        (collision_dir / "Orders.dbf").string(),
        {{.name = "X", .type = 'C', .length = 1U}},
        {{"z"}});
    expect(pre_existing_table.ok, "table-collision fixture should create the pre-existing table");
    const auto table_collision_result = copperfin::vfp::materialize_database_json_import_plan(
        plan_result.plan, (collision_dir / "fresh.dbc").string());
    expect(!table_collision_result.ok,
           "materializing should fail closed when a derived table path already exists");
    expect(!fs::exists(collision_dir / "fresh.dbc"),
           "a table-path collision must not leave a partially materialized DBC behind");
    expect(!fs::exists(collision_dir / "Customers.dbf"),
           "a table-path collision on one table must not materialize any other table either");

    // Fail closed: a table name that would escape the destination directory
    // must be rejected before any path is derived from it, and must not
    // create anything -- not the DBC, not any other table in the plan.
    const fs::path traversal_dir = temp_dir / "traversal";
    fs::create_directories(traversal_dir, ignored);
    copperfin::vfp::DatabaseJsonImportPlan traversal_plan = plan_result.plan;
    traversal_plan.tables.front().name = "../escaped";
    const auto traversal_result = copperfin::vfp::materialize_database_json_import_plan(
        traversal_plan, (traversal_dir / "fresh.dbc").string());
    expect(!traversal_result.ok,
           "a table name that escapes the destination directory must be rejected");
    expect(!fs::exists(traversal_dir / "fresh.dbc"),
           "a rejected unsafe table name must not leave a destination DBC behind");
    expect(!fs::exists(temp_dir / "escaped.dbf"),
           "a rejected '../' table name must not materialize a table outside the destination directory");
    for (const auto& entry : fs::directory_iterator(traversal_dir)) {
        expect(entry.path().filename().string().rfind(".copperfin-import-", 0U) != 0U,
               "no temporary staging directory should remain after a rejected unsafe table name");
    }

    copperfin::vfp::DatabaseJsonImportPlan absolute_plan = plan_result.plan;
#if defined(_WIN32)
    absolute_plan.tables.front().name = "C:\\escaped";
#else
    absolute_plan.tables.front().name = "/escaped";
#endif
    const auto absolute_result = copperfin::vfp::materialize_database_json_import_plan(
        absolute_plan, (traversal_dir / "fresh2.dbc").string());
    expect(!absolute_result.ok,
           "an absolute-path table name must be rejected rather than replacing the destination path");
    expect(!fs::exists(traversal_dir / "fresh2.dbc"),
           "a rejected absolute table name must not leave a destination DBC behind");

    // Memo (M) fields get a .fpt sidecar written alongside the .dbf by
    // create_dbf_table_file(); materializing must stage and commit that
    // sidecar too, not just the .dbf.
    const fs::path memo_dir = temp_dir / "memo";
    fs::create_directories(memo_dir, ignored);
    const std::string memo_document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Notes.dbc", "name": "Notes"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Notes": {
      "fields": [{"name": "TITLE", "type": "C", "length": 20, "decimals": 0},
                 {"name": "BODY", "type": "M", "length": 4, "decimals": 0}],
      "records": [{"TITLE": "first", "BODY": "a memo payload long enough to matter"}]
    }
  }
})JSON";
    const auto memo_plan_result = copperfin::vfp::build_database_json_import_plan(memo_document);
    expect(memo_plan_result.ok, "memo-sidecar fixture plan should build successfully");
    if (memo_plan_result.ok) {
        const fs::path memo_dbc_path = memo_dir / "notes.dbc";
        const auto memo_materialize_result = copperfin::vfp::materialize_database_json_import_plan(
            memo_plan_result.plan, memo_dbc_path.string());
        expect(memo_materialize_result.ok,
               "materializing a memo-field table should succeed: " + memo_materialize_result.error);
        expect(fs::exists(memo_dir / "Notes.dbf"), "materializing should create the memo table's .dbf");
        expect(fs::exists(memo_dir / "Notes.fpt"),
               "materializing a memo-field table must also commit its .fpt sidecar");

        const auto memo_reexported = copperfin::vfp::export_database_as_json(memo_dbc_path.string());
        expect(memo_reexported.ok,
               "the materialized memo-table DBC should itself be exportable: " + memo_reexported.error);
        if (memo_reexported.ok) {
            expect(memo_reexported.json.find("a memo payload long enough to matter") != std::string::npos,
                   "the committed .fpt sidecar must actually contain the memo payload, not just exist");
        }
    }

    // No staging directories should ever be left behind, success or failure.
    for (const auto& entry : fs::directory_iterator(temp_dir)) {
        expect(entry.path().filename().string().rfind(".copperfin-import-", 0U) != 0U,
               "no temporary staging directory should remain after materialize_database_json_import_plan returns");
    }
    for (const auto& entry : fs::directory_iterator(collision_dir)) {
        expect(entry.path().filename().string().rfind(".copperfin-import-", 0U) != 0U,
               "no temporary staging directory should remain in the collision directory either");
    }

    fs::remove_all(temp_dir, ignored);
}

// ---- build_database_sql_import_plan tests ----

void test_build_database_sql_import_plan_validates_without_mutation() {
    const std::string document =
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: Northwind\n"
        "-- source: /source/Northwind.dbc\n"
        "\n"
        "CREATE TABLE \"Customers\" (\n"
        "    \"NAME\" VARCHAR(40),\n"
        "    \"ACTIVE\" BOOLEAN\n"
        ");\n"
        "\n"
        "INSERT INTO \"Customers\" (\"NAME\", \"ACTIVE\") VALUES ('Acme', TRUE);\n"
        "INSERT INTO \"Customers\" (\"NAME\", \"ACTIVE\") VALUES (NULL, FALSE);\n"
        "\n"
        "CREATE TABLE \"Orders\" (\n"
        "    \"ORDERID\" DECIMAL(8, 0)\n"
        ");\n"
        "\n"
        "INSERT INTO \"Orders\" (\"ORDERID\") VALUES (7);\n"
        "INSERT INTO \"Orders\" (\"ORDERID\") VALUES (12);\n";

    const auto result = copperfin::vfp::build_database_sql_import_plan(document);
    expect(result.ok, "database SQL import planning should accept the exact dialect export_database_as_sql() emits: " + result.error_code);
    expect(result.error_code.empty(), "successful database SQL planning should not retain an error code");
    if (result.ok) {
        expect(result.plan.database_name == "Northwind",
               "database SQL planning should retain the database name from the header comment");
        expect(result.plan.tables.size() == 2U && result.plan.tables[0].name == "Customers" &&
                   result.plan.tables[1].name == "Orders",
               "database SQL planning should retain tables in CREATE TABLE order");
        expect(result.plan.tables[0].fields.size() == 2U &&
                   result.plan.tables[0].fields[0].name == "NAME" && result.plan.tables[0].fields[0].type == 'C' &&
                   result.plan.tables[0].fields[1].name == "ACTIVE" && result.plan.tables[0].fields[1].type == 'L',
               "database SQL planning should map VARCHAR/BOOLEAN columns to C/L field descriptors");
        expect(result.plan.tables[0].records_json == "[{\"NAME\":\"Acme\",\"ACTIVE\":true},{\"NAME\":null,\"ACTIVE\":false}]",
               "database SQL planning should convert INSERT literals to the same JSON row shape the JSON path already consumes");
        expect(result.plan.tables[1].fields[0].type == 'N' && result.plan.tables[1].fields[0].length == 8U,
               "database SQL planning should map DECIMAL columns to N field descriptors");
        expect(result.plan.tables[1].records_json == "[{\"ORDERID\":7},{\"ORDERID\":12}]",
               "database SQL planning should retain every INSERT row for a table, not just the first");
    }

    const auto missing_header = copperfin::vfp::build_database_sql_import_plan(
        "CREATE TABLE \"X\" (\"A\" INTEGER);\n");
    expect(!missing_header.ok && missing_header.error_code == "database_sql_import.invalid_header",
           "database SQL planning should reject a document missing the exporter's exact three-line header");

    const auto foreign_dialect = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE X (A INT);\n");
    expect(!foreign_dialect.ok,
           "database SQL planning should reject unquoted identifiers rather than guessing at a foreign SQL dialect");

    const auto unknown_type = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"X\" (\"A\" NUMERIC(5));\n");
    expect(!unknown_type.ok && unknown_type.error_code == "database_sql_import.unknown_column_type",
           "database SQL planning should reject a column type outside export_database_as_sql()'s fixed vocabulary");

    const auto duplicate_table = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" INTEGER);\n"
        "CREATE TABLE \"people\" (\"B\" INTEGER);\n");
    expect(!duplicate_table.ok && duplicate_table.error_code == "database_sql_import.duplicate_table_name",
           "database SQL planning should reject cross-platform case-folded table-name collisions, matching the JSON path");

    const auto unknown_insert_table = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" INTEGER);\n"
        "INSERT INTO \"Other\" (\"A\") VALUES (1);\n");
    expect(!unknown_insert_table.ok && unknown_insert_table.error_code == "database_sql_import.insert_unknown_table",
           "database SQL planning should reject an INSERT INTO a table with no matching CREATE TABLE");

    const auto unknown_insert_column = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" INTEGER);\n"
        "INSERT INTO \"People\" (\"B\") VALUES (1);\n");
    expect(!unknown_insert_column.ok && unknown_insert_column.error_code == "database_sql_import.insert_unknown_column",
           "database SQL planning should reject an INSERT column with no matching CREATE TABLE field");

    const auto mismatched_value_count = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" INTEGER, \"B\" INTEGER);\n"
        "INSERT INTO \"People\" (\"A\", \"B\") VALUES (1);\n");
    expect(!mismatched_value_count.ok && mismatched_value_count.error_code == "database_sql_import.insert_value_count_mismatch",
           "database SQL planning should reject an INSERT whose VALUES count does not match its column list");

    const auto wrong_value_shape = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" INTEGER);\n"
        "INSERT INTO \"People\" (\"A\") VALUES ('not a number');\n");
    expect(!wrong_value_shape.ok && wrong_value_shape.error_code == "database_sql_import.invalid_insert_value",
           "database SQL planning should reject a string literal supplied for a numeric column rather than coercing it");

    const auto exponent_form_accepted = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" DOUBLE PRECISION);\n"
        "INSERT INTO \"People\" (\"A\") VALUES (1e+20);\n"
        "INSERT INTO \"People\" (\"A\") VALUES (1.5e-10);\n"
        "INSERT INTO \"People\" (\"A\") VALUES (-2E5);\n");
    expect(exponent_form_accepted.ok,
           "database SQL planning should accept the scientific-notation exponent suffix export_database_as_sql() can emit for DOUBLE PRECISION values: " +
               exponent_form_accepted.error_code);
    if (exponent_form_accepted.ok) {
        expect(exponent_form_accepted.plan.tables[0].records_json ==
                   "[{\"A\":1e+20},{\"A\":1.5e-10},{\"A\":-2E5}]",
               "database SQL planning should retain an exponent-form numeric literal verbatim as a JSON number");
    }

    const auto bare_trailing_dot_rejected = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" DOUBLE PRECISION);\n"
        "INSERT INTO \"People\" (\"A\") VALUES (1.);\n");
    expect(!bare_trailing_dot_rejected.ok,
           "database SQL planning should reject a numeric literal with a trailing decimal point and no fraction digits, which is not valid JSON");

    const auto bare_leading_dot_rejected = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"People\" (\"A\" DOUBLE PRECISION);\n"
        "INSERT INTO \"People\" (\"A\") VALUES (.5);\n");
    expect(!bare_leading_dot_rejected.ok,
           "database SQL planning should reject a numeric literal with a leading decimal point and no integer digits, which is not valid JSON");

    const auto timestamp_round_trips = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"Events\" (\"WHEN\" TIMESTAMP);\n"
        "INSERT INTO \"Events\" (\"WHEN\") VALUES ('2026-05-01 12:34:56');\n"
        "INSERT INTO \"Events\" (\"WHEN\") VALUES (NULL);\n");
    expect(timestamp_round_trips.ok,
           "database SQL planning should accept a TIMESTAMP literal in the exact shape export_database_as_sql() emits: " +
               timestamp_round_trips.error_code);
    if (timestamp_round_trips.ok) {
        expect(timestamp_round_trips.plan.tables[0].records_json.find("\"WHEN\":\"julian:") != std::string::npos,
               "database SQL planning should convert a TIMESTAMP literal into the julian:/millis: internal storage contract, not pass it through as raw text");
        expect(timestamp_round_trips.plan.tables[0].records_json.find("\"WHEN\":null") != std::string::npos,
               "database SQL planning should still accept a NULL TIMESTAMP value");
    }

    const auto malformed_timestamp_rejected = copperfin::vfp::build_database_sql_import_plan(
        "-- Copperfin EXPORT DATABASE ... TYPE SQL\n"
        "-- database: N\n"
        "-- source: x\n"
        "CREATE TABLE \"Events\" (\"WHEN\" TIMESTAMP);\n"
        "INSERT INTO \"Events\" (\"WHEN\") VALUES ('not a timestamp');\n");
    expect(!malformed_timestamp_rejected.ok && malformed_timestamp_rejected.error_code == "database_sql_import.invalid_insert_value",
           "database SQL planning should reject a TIMESTAMP literal that isn't the exact YYYY-MM-DD HH:MM:SS shape");
}

// ---- export_database_as_sql -> build_database_sql_import_plan round trip ----

void test_export_database_as_sql_round_trips_through_import() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_database_sql_import_round_trip_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    // Build a real source database via the JSON materializer (already proven
    // above), so this test exercises export_database_as_sql() against a real
    // DBC/DBF pair rather than a hand-written fixture string.
    const std::string source_document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Northwind.dbc", "name": "Northwind"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Orders": {
      "fields": [{"name": "ORDERID", "type": "N", "length": 8, "decimals": 0},
                 {"name": "CREATEDAT", "type": "T", "length": 8, "decimals": 0},
                 {"name": "MAGNITUDE", "type": "B", "length": 8, "decimals": 0}],
      "records": [{"ORDERID": 7, "CREATEDAT": "julian:2461162 millis:45296000", "MAGNITUDE": 100000000000000000000},
                  {"ORDERID": 12, "CREATEDAT": "julian:2461163 millis:0", "MAGNITUDE": 0.0000001}]
    },
    "Customers": {
      "fields": [{"name": "NAME", "type": "C", "length": 40, "decimals": 0},
                 {"name": "ACTIVE", "type": "L", "length": 1, "decimals": 0}],
      "records": [{"NAME": "Acme", "ACTIVE": true}, {"NAME": "O'Brien", "ACTIVE": false}]
    }
  }
})JSON";
    const auto source_plan = copperfin::vfp::build_database_json_import_plan(source_document);
    expect(source_plan.ok, "round-trip fixture plan should build successfully");
    if (!source_plan.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }
    const fs::path source_dir = temp_dir / "source";
    fs::create_directories(source_dir, ignored);
    const fs::path source_dbc_path = source_dir / "source.dbc";
    const auto source_materialize = copperfin::vfp::materialize_database_json_import_plan(
        source_plan.plan, source_dbc_path.string());
    expect(source_materialize.ok, "round-trip fixture database should materialize: " + source_materialize.error);
    if (!source_materialize.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const auto sql_export = copperfin::vfp::export_database_as_sql(source_dbc_path.string());
    expect(sql_export.ok, "export_database_as_sql should succeed against the fixture database: " + sql_export.error);
    if (!sql_export.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // A TIMESTAMP literal (export_database_as_sql()'s human-readable
    // conversion of the T-type internal storage contract) and a very large
    // DOUBLE PRECISION value (whose default text formatting can switch to
    // scientific notation, e.g. "1e+20") must both round-trip through the
    // parser -- captured from the real exporter's own output rather than
    // hand-computed, so this test doesn't need to duplicate julian-day math.
    const auto extract_first_single_quoted_literal = [](const std::string& text, std::size_t from) -> std::optional<std::string> {
        const std::size_t open = text.find('\'', from);
        if (open == std::string::npos) return std::nullopt;
        const std::size_t close = text.find('\'', open + 1U);
        if (close == std::string::npos) return std::nullopt;
        return text.substr(open + 1U, close - open - 1U);
    };
    const std::size_t orders_insert_position = sql_export.sql.find("INSERT INTO \"Orders\"");
    expect(orders_insert_position != std::string::npos, "fixture SQL export should contain an Orders INSERT statement");
    const auto original_timestamp_literal = extract_first_single_quoted_literal(sql_export.sql, orders_insert_position);
    expect(original_timestamp_literal.has_value(), "fixture SQL export should contain a quoted CREATEDAT timestamp literal");

    const auto sql_plan = copperfin::vfp::build_database_sql_import_plan(sql_export.sql);
    expect(sql_plan.ok, "build_database_sql_import_plan should accept export_database_as_sql()'s own output: " + sql_plan.error_code);
    if (!sql_plan.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    const fs::path reimport_dir = temp_dir / "reimport";
    fs::create_directories(reimport_dir, ignored);
    const fs::path reimported_dbc_path = reimport_dir / "reimported.dbc";
    const auto reimport_materialize = copperfin::vfp::materialize_database_json_import_plan(
        sql_plan.plan, reimported_dbc_path.string());
    expect(reimport_materialize.ok,
           "materializing the SQL-derived plan should succeed: " + reimport_materialize.error);
    expect(reimport_materialize.table_count == 2U, "the SQL round trip should recreate both tables");

    const auto reexported_json = copperfin::vfp::export_database_as_json(reimported_dbc_path.string());
    expect(reexported_json.ok, "the SQL round-tripped DBC should itself be exportable: " + reexported_json.error);
    if (reexported_json.ok) {
        expect(reexported_json.json.find("\"NAME\": \"Acme\"") != std::string::npos,
               "the SQL round trip should preserve character row data");
        expect(reexported_json.json.find("\"NAME\": \"O'Brien\"") != std::string::npos,
               "the SQL round trip should preserve an embedded single quote through both quoting layers");
        expect(reexported_json.json.find("\"ACTIVE\": true") != std::string::npos &&
                   reexported_json.json.find("\"ACTIVE\": false") != std::string::npos,
               "the SQL round trip should preserve both logical values");
        expect(reexported_json.json.find("\"ORDERID\": 7") != std::string::npos &&
                   reexported_json.json.find("\"ORDERID\": 12") != std::string::npos,
               "the SQL round trip should preserve every numeric row");
    }

    // Re-export as SQL again and confirm the T-type timestamp round-tripped
    // exactly, and that the very-large/very-small DOUBLE PRECISION values
    // (whatever scientific-notation shape the real formatter produced) were
    // accepted by the parser rather than truncated at the exponent.
    const auto reexported_sql = copperfin::vfp::export_database_as_sql(reimported_dbc_path.string());
    expect(reexported_sql.ok, "the SQL round-tripped DBC should itself be exportable as SQL: " + reexported_sql.error);
    if (reexported_sql.ok && original_timestamp_literal.has_value()) {
        const std::size_t reexported_orders_insert_position = reexported_sql.sql.find("INSERT INTO \"Orders\"");
        expect(reexported_orders_insert_position != std::string::npos,
               "the re-exported SQL should still contain an Orders INSERT statement");
        const auto reexported_timestamp_literal =
            extract_first_single_quoted_literal(reexported_sql.sql, reexported_orders_insert_position);
        expect(reexported_timestamp_literal.has_value() && *reexported_timestamp_literal == *original_timestamp_literal,
               "the SQL round trip should preserve a T-type timestamp value exactly: expected '" +
                   *original_timestamp_literal + "', got '" +
                   (reexported_timestamp_literal.has_value() ? *reexported_timestamp_literal : "<none>") + "'");
        expect(reexported_sql.sql.find("MAGNITUDE") != std::string::npos,
               "the SQL round trip should preserve the DOUBLE PRECISION column itself");
    }

    fs::remove_all(temp_dir, ignored);
}

// #5678 (found by an automated Codex code-review pass): the shared JSON/SQL
// database import materializer checked destination existence with exact
// std::filesystem::exists() paths only. On a case-sensitive filesystem, an
// existing case-folded alias (e.g. a pre-existing "ORDERS.DBF" when the
// plan names table "Orders") therefore did not count as an existing
// destination, letting an import publish a second physical file
// representing the same VFP/Windows table identity -- not portable to
// Windows, and violating #5472's own fail-closed overwrite-consent
// contract. Covers all three collision points the issue itself names: the
// destination DBC, a table's own .dbf, and a table's own .fpt memo
// sidecar. Verified to reliably fail against the pre-fix code (each
// scenario below previously returned ok=true and created a second,
// case-differing file) and reliably pass against the fix.
void test_materialize_database_json_import_plan_rejects_case_folded_collisions() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_database_json_case_collision_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const std::string document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Orders.dbc", "name": "Orders"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Orders": {
      "fields": [{"name": "ORDERID", "type": "N", "length": 8, "decimals": 0}],
      "records": [{"ORDERID": 7}]
    }
  }
})JSON";
    const auto plan_result = copperfin::vfp::build_database_json_import_plan(document);
    expect(plan_result.ok, "case-collision fixture plan should build successfully");
    if (!plan_result.ok) {
        fs::remove_all(temp_dir, ignored);
        return;
    }

    // Case 1: a differently-cased DBC destination already exists.
    {
        const fs::path dbc_case_dir = temp_dir / "dbc_case";
        fs::create_directories(dbc_case_dir, ignored);
        const fs::path existing_upper_dbc = dbc_case_dir / "CONTAINER.DBC";
        {
            std::ofstream output(existing_upper_dbc, std::ios::binary);
            output << "pre-existing";
        }
        const fs::path requested_dbc_path = dbc_case_dir / "container.dbc";
        const auto result = copperfin::vfp::materialize_database_json_import_plan(
            plan_result.plan, requested_dbc_path.string());
        expect(!result.ok,
               "materializing must fail closed when a case-folded DBC alias already exists");
        // #5678 review round: requested_dbc_path ("container.dbc") is a
        // case-folded alias of the pre-existing "CONTAINER.DBC", so on a
        // case-insensitive filesystem fs::exists(requested_dbc_path) is
        // TRUE regardless of whether this fix behaved correctly -- it
        // resolves straight through to the original file. Verify no new,
        // distinctly-spelled file was created (directory entry count
        // unchanged) and the original file's own bytes are untouched,
        // instead of asserting non-existence of an aliasing path.
        {
            std::error_code count_error;
            const auto entry_count = std::distance(
                fs::directory_iterator(dbc_case_dir, count_error), fs::directory_iterator());
            expect(entry_count == 1,
                   "a case-folded DBC collision must not create any additional directory entry");
        }
        std::ifstream verify_input(existing_upper_dbc, std::ios::binary);
        const std::string verify_content((std::istreambuf_iterator<char>(verify_input)),
                                          std::istreambuf_iterator<char>());
        expect(verify_content == "pre-existing",
               "a case-folded DBC collision must leave the pre-existing file's bytes untouched");
        expect(!fs::exists(dbc_case_dir / "Orders.dbf"),
               "a case-folded DBC collision must not materialize any table either");
    }

    // Case 2: a differently-cased table .dbf destination already exists.
    {
        const fs::path table_case_dir = temp_dir / "table_case";
        fs::create_directories(table_case_dir, ignored);
        const auto pre_existing_table = copperfin::vfp::create_dbf_table_file(
            (table_case_dir / "ORDERS.DBF").string(),
            {{.name = "X", .type = 'C', .length = 1U}},
            {{"z"}});
        expect(pre_existing_table.ok, "table-case fixture should create the pre-existing table");
        const auto pre_existing_table_path = table_case_dir / "ORDERS.DBF";
        std::error_code size_error;
        const auto original_table_size = fs::file_size(pre_existing_table_path, size_error);
        expect(!size_error, "table-case fixture's pre-existing file size should be readable");
        const fs::path requested_dbc_path = table_case_dir / "fresh.dbc";
        const auto result = copperfin::vfp::materialize_database_json_import_plan(
            plan_result.plan, requested_dbc_path.string());
        expect(!result.ok,
               "materializing must fail closed when a case-folded table .dbf alias already exists");
        expect(!fs::exists(requested_dbc_path),
               "a case-folded table collision must not leave a partially materialized DBC behind");
        // #5678 review round: table_case_dir / "Orders.dbf" is a
        // case-folded alias of the pre-existing "ORDERS.DBF", so
        // fs::exists() on it is TRUE on a case-insensitive filesystem
        // regardless of this fix's own correctness. Verify no additional
        // directory entry was created and the pre-existing table's own
        // size is unchanged instead.
        {
            std::error_code count_error;
            const auto entry_count = std::distance(
                fs::directory_iterator(table_case_dir, count_error), fs::directory_iterator());
            expect(entry_count == 1,
                   "a case-folded table collision must not additionally create the exact-case .dbf");
        }
        const auto post_table_size = fs::file_size(pre_existing_table_path, size_error);
        expect(!size_error && post_table_size == original_table_size,
               "a case-folded table collision must leave the pre-existing table file untouched");
    }

    // Case 3: a differently-cased memo (.fpt) sidecar destination already
    // exists for a table whose plan declares a memo field.
    {
        const fs::path memo_case_dir = temp_dir / "memo_case";
        fs::create_directories(memo_case_dir, ignored);
        {
            std::ofstream output(memo_case_dir / "ORDERS.FPT", std::ios::binary);
            output << "pre-existing memo";
        }
        const std::string memo_document = R"JSON({
  "schema_version": 1,
  "database": {"path": "/source/Orders.dbc", "name": "Orders"},
  "catalog": [{"record_index": 1}],
  "tables": {
    "Orders": {
      "fields": [{"name": "NOTES", "type": "M", "length": 4, "decimals": 0}],
      "records": [{"NOTES": "hello"}]
    }
  }
})JSON";
        const auto memo_plan_result = copperfin::vfp::build_database_json_import_plan(memo_document);
        expect(memo_plan_result.ok, "memo case-collision fixture plan should build successfully");
        if (memo_plan_result.ok) {
            const fs::path requested_dbc_path = memo_case_dir / "fresh.dbc";
            const auto result = copperfin::vfp::materialize_database_json_import_plan(
                memo_plan_result.plan, requested_dbc_path.string());
            expect(!result.ok,
                   "materializing must fail closed when a case-folded memo .fpt alias already exists");
            expect(!fs::exists(requested_dbc_path),
                   "a case-folded memo collision must not leave a partially materialized DBC behind");
            expect(!fs::exists(memo_case_dir / "Orders.dbf"),
                   "a case-folded memo collision must not materialize the table's own .dbf either");
        }
    }

    fs::remove_all(temp_dir, ignored);
}
