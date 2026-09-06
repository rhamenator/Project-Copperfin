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
