void test_dbf_table_record_value_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            "Character value is too large for the target field.",
        "#2381: DBF table character overflow error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.DateTimeValueInvalid") ==
            "DateTime fields currently accept values formatted as 'julian:<day> millis:<milliseconds>'.",
        "#2381: DBF table datetime validation error should resolve through the en-US catalog");
    expect(
        spanish_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            "El valor de caracteres es demasiado grande para el campo destino.",
        "#2602: DBF table character overflow error should resolve through the es-419 catalog");
    expect(
        portuguese_catalog.translate("Vfp.DbfTable.Error.DateTimeValueInvalid") ==
            "Campos DateTime atualmente aceitam valores formatados como 'julian:<day> millis:<milliseconds>'.",
        "#2602: DBF table datetime validation error should resolve through the pt-BR catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") !=
            english_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge"),
        "#2381: DBF table record/value errors should be pseudo-localizable");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CharacterValueTooLarge") ==
            copperfin::localization::pseudo_localize(
                "Character value is too large for the target field."),
        "#2602: DBF table qps-ploc record/value errors should use the pseudo-localization transform");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_record_value_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_record_value_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 5U, .decimal_count = 0U}
    };
    const std::vector<std::vector<std::string>> records{{"ALPHA"}};
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, "#2381: localized DBF table validation fixture should be created");

    const auto replace_result =
        copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "NAME", "TOO-LONG");
    expect(!replace_result.ok, "#2381: oversized character field writes should fail");
    expect(
        replace_result.error == "Character value is too large for the target field.",
        "#2381: oversized character field writes should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_creation_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    const copperfin::localization::PlaceholderMap y_field{{"fieldType", "Y"}};

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired") ==
            "At least one field is required to create a DBF table.",
        "#2382: DBF table creation field-required error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.EightByteFieldWidthInvalid", y_field) ==
            "Y fields require a width of exactly 8 bytes.",
        "#2382: DBF table field-type width errors should preserve named placeholders");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired") !=
            english_catalog.translate("Vfp.DbfTable.Error.CreateFieldRequired"),
        "#2382: DBF table creation/open/write errors should be pseudo-localizable");

    const auto no_fields_result = copperfin::vfp::create_dbf_table_file("unused.dbf", {}, {});
    expect(!no_fields_result.ok, "#2382: DBF table creation should reject empty field lists");
    expect(
        no_fields_result.error == "At least one field is required to create a DBF table.",
        "#2382: empty field list should preserve the default localized error");

    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "BALANCE", .type = 'Y', .offset = 1U, .length = 4U, .decimal_count = 0U}
    };
    const auto invalid_width_result = copperfin::vfp::create_dbf_table_file("unused.dbf", fields, {{"0"}});
    expect(!invalid_width_result.ok, "#2382: DBF table creation should reject invalid Y field widths");
    expect(
        invalid_width_result.error == "Y fields require a width of exactly 8 bytes.",
        "#2382: invalid Y field width should preserve the default localized placeholder output");
}

void test_dbf_table_schema_mutation_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists") ==
            "The target field already exists.",
        "#2383: DBF table duplicate-field schema error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.DropLastField") ==
            "Cannot drop the last field from a DBF table.",
        "#2383: DBF table last-field drop error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists") !=
            english_catalog.translate("Vfp.DbfTable.Error.TargetFieldExists"),
        "#2383: DBF table schema mutation errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_schema_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_schema_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2383: localized schema validation fixture should be created");

    const auto duplicate_result = copperfin::vfp::add_dbf_table_field(
        table_path.string(),
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U});
    expect(!duplicate_result.ok, "#2383: duplicate DBF fields should be rejected");
    expect(
        duplicate_result.error == "The target field already exists.",
        "#2383: duplicate DBF fields should preserve the default localized error");

    const auto drop_result = copperfin::vfp::drop_dbf_table_field(table_path.string(), "NAME");
    expect(!drop_result.ok, "#2383: dropping the last DBF field should be rejected");
    expect(
        drop_result.error == "Cannot drop the last field from a DBF table.",
        "#2383: last-field drop should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_record_replacement_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable") ==
            "The target field was not found in the table.",
        "#2384: DBF table replacement missing-field error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.MemoSidecarPathMissing") ==
            "No memo sidecar path could be inferred for the table.",
        "#2384: DBF table replacement memo-sidecar-path error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable") !=
            english_catalog.translate("Vfp.DbfTable.Error.TargetFieldNotFoundInTable"),
        "#2384: DBF table append/replacement errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_replacement_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_replacement_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2384: localized replacement validation fixture should be created");

    const auto missing_field_result =
        copperfin::vfp::replace_record_field_value(table_path.string(), 0U, "MISSING", "BRAVO");
    expect(!missing_field_result.ok, "#2384: missing replacement fields should be rejected");
    expect(
        missing_field_result.error == "The target field was not found in the table.",
        "#2384: missing replacement fields should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}

void test_dbf_table_row_header_errors_resolve_through_localization_catalog() {
    namespace fs = std::filesystem;
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(
        english_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge") ==
            "Requested record count exceeds current table size.",
        "#2385: DBF table requested-count error should resolve through the en-US catalog");
    expect(
        english_catalog.translate("Vfp.DbfTable.Error.TableHeaderTruncated") ==
            "Table header is truncated.",
        "#2385: DBF table header truncation error should resolve through the en-US catalog");
    expect(
        pseudo_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge") !=
            english_catalog.translate("Vfp.DbfTable.Error.RequestedRecordCountTooLarge"),
        "#2385: DBF table row/header mutation errors should be pseudo-localizable");

    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_dbf_localized_row_header_error_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path table_path = temp_dir / "localized_row_header_error.dbf";
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "NAME", .type = 'C', .offset = 1U, .length = 10U, .decimal_count = 0U}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, {{"ALPHA"}});
    expect(create_result.ok, "#2385: localized row/header validation fixture should be created");

    const auto truncate_result = copperfin::vfp::truncate_dbf_table_file(table_path.string(), 2U);
    expect(!truncate_result.ok, "#2385: truncation should reject record counts above current table size");
    expect(
        truncate_result.error == "Requested record count exceeds current table size.",
        "#2385: too-large truncate count should preserve the default localized error");

    fs::remove_all(temp_dir, ignored);
}
