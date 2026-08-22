// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "test_studio_host_json_support.h"

namespace cf_test_studio_host_json {
void test_studio_host_json_assigns_row_source_type_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_row_source_type_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path row_source_type_path = temp_root / "row_source_type.scx";
    write_synthetic_form_table_for_object_row_source_type(row_source_type_path);
    const auto row_source_type_process = run_process_capture(
        studio_host_path,
        {
            "--path", row_source_type_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-object-name", "cboCustomer",
            "--row-source-type-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(row_source_type_process.exit_code == 0,
        "#1049: host object row-source-type assignment should exit successfully");
    expect(visual_object_property(row_source_type_path, "one-guid", "ROWSOURCETYPE") == "6" &&
            visual_object_property(row_source_type_path, "two-guid", "ROWSOURCETYPE") == "6" &&
            visual_object_property(row_source_type_path, "three-guid", "ROWSOURCETYPE") == "0" &&
            visual_object_property(row_source_type_path, "other-guid", "ROWSOURCETYPE") == "5",
        "#1049: host object row-source-type assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source-type-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1049: missing-target host object row-source-type assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "ROWSOURCETYPE") == "2" &&
            visual_object_property(missing_target_path, "two-guid", "ROWSOURCETYPE") == "3",
        "#1049: missing-target host object row-source-type assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1049: row-source-type-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_row_source_type(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--row-source-type-object",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1049: row-source-type-object without row-source-type value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object without row-source-type value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_row_source_type(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--row-source-type-object",
            "--row-source-type", "-1",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1049: negative row-source-type values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: negative row-source-type values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_row_source_type(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--row-source-type-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source-type-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1049: duplicate-target host object row-source-type assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: duplicate-target host object row-source-type assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_row_source_type(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--row-source-type-object",
            "--row-source-object",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--row-source", "products.name",
            "--row-source-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1049: row-source-type-object plus row-source-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "ROWSOURCETYPE") == "2",
        "#1049: row-source-type-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_bound_column_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_bound_column_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path bound_column_path = temp_root / "bound_column.scx";
    write_synthetic_form_table_for_object_bound_column(bound_column_path);
    const auto bound_column_process = run_process_capture(
        studio_host_path,
        {
            "--path", bound_column_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-object-name", "cboCustomer",
            "--bound-column-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(bound_column_process.exit_code == 0,
        "#1050: host object bound-column assignment should exit successfully");
    expect(visual_object_property(bound_column_path, "one-guid", "BOUNDCOLUMN") == "4" &&
            visual_object_property(bound_column_path, "two-guid", "BOUNDCOLUMN") == "4" &&
            visual_object_property(bound_column_path, "three-guid", "BOUNDCOLUMN") == "0" &&
            visual_object_property(bound_column_path, "other-guid", "BOUNDCOLUMN") == "3",
        "#1050: host object bound-column assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_bound_column(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--bound-column-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1050: missing-target host object bound-column assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "BOUNDCOLUMN") == "1" &&
            visual_object_property(missing_target_path, "two-guid", "BOUNDCOLUMN") == "2",
        "#1050: missing-target host object bound-column assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_bound_column(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1050: bound-column-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_bound_column(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--bound-column-object",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1050: bound-column-object without bound-column value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object without bound-column value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_bound_column(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--bound-column-object",
            "--bound-column", "-1",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1050: negative bound-column values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: negative bound-column values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_bound_column(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--bound-column-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--bound-column-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1050: duplicate-target host object bound-column assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: duplicate-target host object bound-column assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_bound_column(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--bound-column-object",
            "--row-source-type-object",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--row-source-type", "6",
            "--row-source-type-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1050: bound-column-object plus row-source-type-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "BOUNDCOLUMN") == "1",
        "#1050: bound-column-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_column_count_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_column_count_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path column_count_path = temp_root / "column_count.scx";
    write_synthetic_form_table_for_object_column_count(column_count_path);
    const auto column_count_process = run_process_capture(
        studio_host_path,
        {
            "--path", column_count_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-object-name", "cboCustomer",
            "--column-count-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(column_count_process.exit_code == 0,
        "#1051: host object column-count assignment should exit successfully");
    expect(visual_object_property(column_count_path, "one-guid", "COLUMNCOUNT") == "5" &&
            visual_object_property(column_count_path, "two-guid", "COLUMNCOUNT") == "5" &&
            visual_object_property(column_count_path, "three-guid", "COLUMNCOUNT") == "0" &&
            visual_object_property(column_count_path, "other-guid", "COLUMNCOUNT") == "4",
        "#1051: host object column-count assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_column_count(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--column-count-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1051: missing-target host object column-count assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "COLUMNCOUNT") == "2" &&
            visual_object_property(missing_target_path, "two-guid", "COLUMNCOUNT") == "3",
        "#1051: missing-target host object column-count assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_column_count(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1051: column-count-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_column_count(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--column-count-object",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1051: column-count-object without column-count value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object without column-count value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_column_count(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--column-count-object",
            "--column-count", "-1",
            "--column-count-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1051: negative column-count values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: negative column-count values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_column_count(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--column-count-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--column-count-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1051: duplicate-target host object column-count assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: duplicate-target host object column-count assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_column_count(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--column-count-object",
            "--bound-column-object",
            "--column-count", "5",
            "--column-count-target-unique-id", "one-guid",
            "--bound-column", "4",
            "--bound-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1051: column-count-object plus bound-column-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "COLUMNCOUNT") == "2",
        "#1051: column-count-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_list_index_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_list_index_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path list_index_path = temp_root / "list_index.scx";
    write_synthetic_form_table_for_object_list_index(list_index_path);
    const auto list_index_process = run_process_capture(
        studio_host_path,
        {
            "--path", list_index_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-object-name", "cboCustomer",
            "--list-index-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(list_index_process.exit_code == 0,
        "#1053: host object list-index assignment should exit successfully");
    expect(visual_object_property(list_index_path, "one-guid", "LISTINDEX") == "3" &&
            visual_object_property(list_index_path, "two-guid", "LISTINDEX") == "3" &&
            visual_object_property(list_index_path, "three-guid", "LISTINDEX") == "0" &&
            visual_object_property(list_index_path, "other-guid", "LISTINDEX") == "2",
        "#1053: host object list-index assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_list_index(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--list-index-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1053: missing-target host object list-index assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LISTINDEX") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LISTINDEX") == "1",
        "#1053: missing-target host object list-index assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_list_index(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1053: list-index-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_list_index(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--list-index-object",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1053: list-index-object without list-index value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object without list-index value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_list_index(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--list-index-object",
            "--list-index", "-1",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1053: negative list-index values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "LISTINDEX") == "0",
        "#1053: negative list-index values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_list_index(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--list-index-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--list-index-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1053: duplicate-target host object list-index assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LISTINDEX") == "0",
        "#1053: duplicate-target host object list-index assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_list_index(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--list-index-object",
            "--style-object",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--style", "2",
            "--style-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1053: list-index-object plus style-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LISTINDEX") == "0",
        "#1053: list-index-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_left_column_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_left_column_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path left_column_path = temp_root / "left_column.scx";
    write_synthetic_form_table_for_object_left_column(left_column_path);
    const auto left_column_process = run_process_capture(
        studio_host_path,
        {
            "--path", left_column_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-object-name", "grdCustomer",
            "--left-column-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(left_column_process.exit_code == 0,
        "#1054: host object left-column assignment should exit successfully");
    expect(visual_object_property(left_column_path, "one-guid", "LEFTCOLUMN") == "7" &&
            visual_object_property(left_column_path, "two-guid", "LEFTCOLUMN") == "7" &&
            visual_object_property(left_column_path, "three-guid", "LEFTCOLUMN") == "0" &&
            visual_object_property(left_column_path, "other-guid", "LEFTCOLUMN") == "2",
        "#1054: host object left-column assignment should assign selected numeric values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_left_column(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--left-column-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1054: missing-target host object left-column assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "LEFTCOLUMN") == "0" &&
            visual_object_property(missing_target_path, "two-guid", "LEFTCOLUMN") == "1",
        "#1054: missing-target host object left-column assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_left_column(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1054: left-column-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_left_column(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--left-column-object",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1054: left-column-object without left-column value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object without left-column value should not mutate the asset");

    const fs::path negative_path = temp_root / "negative.scx";
    write_synthetic_form_table_for_object_left_column(negative_path);
    const auto negative_process = run_process_capture(
        studio_host_path,
        {
            "--path", negative_path.string(),
            "--left-column-object",
            "--left-column", "-1",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(negative_process.exit_code == 2,
        "#1054: negative left-column values should fail during launch parsing");
    expect(visual_object_property(negative_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: negative left-column values should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_left_column(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--left-column-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--left-column-target-object-name", "grdCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1054: duplicate-target host object left-column assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: duplicate-target host object left-column assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_left_column(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--left-column-object",
            "--list-index-object",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--list-index", "3",
            "--list-index-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1054: left-column-object plus list-index-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "LEFTCOLUMN") == "0",
        "#1054: left-column-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

void test_studio_host_json_assigns_display_value_by_stable_selectors(const std::string& studio_host_path) {
    namespace fs = std::filesystem;

    const fs::path temp_root = fs::temp_directory_path() / "copperfin_studio_host_display_value_object_json_tests";
    std::error_code ignored;
    fs::remove_all(temp_root, ignored);
    fs::create_directories(temp_root);

    const fs::path display_value_path = temp_root / "display_value.scx";
    write_synthetic_form_table_for_object_display_value(display_value_path);
    const auto display_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", display_value_path.string(),
            "--display-value-object",
            "--display-value", "Bob \"B\"",
            "--display-value-target-object-name", "cboCustomer",
            "--display-value-target-unique-id", "two-guid",
            "--json"
        },
        temp_root);
    expect(display_value_process.exit_code == 0,
        "#1055: host object display-value assignment should exit successfully");
    expect(visual_object_property(display_value_path, "one-guid", "DISPLAYVALUE") == "Bob \"B\"" &&
            visual_object_property(display_value_path, "two-guid", "DISPLAYVALUE") == "Bob \"B\"" &&
            visual_object_property(display_value_path, "three-guid", "DISPLAYVALUE") == "Ready" &&
            visual_object_property(display_value_path, "other-guid", "DISPLAYVALUE") == "Other",
        "#1055: host object display-value assignment should assign selected text values and preserve unrelated objects");

    const fs::path missing_target_path = temp_root / "missing_target.scx";
    write_synthetic_form_table_for_object_display_value(missing_target_path);
    const auto missing_target_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_target_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--display-value-target-unique-id", "missing-guid",
            "--json"
        },
        temp_root);
    expect(missing_target_process.exit_code == 4,
        "#1055: missing-target host object display-value assignment should return command failure");
    expect(visual_object_property(missing_target_path, "one-guid", "DISPLAYVALUE") == "Alice" &&
            visual_object_property(missing_target_path, "two-guid", "DISPLAYVALUE") == "Order 100",
        "#1055: missing-target host object display-value assignment should not mutate the asset");

    const fs::path missing_selector_path = temp_root / "missing_selector.scx";
    write_synthetic_form_table_for_object_display_value(missing_selector_path);
    const auto missing_selector_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_selector_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--json"
        },
        temp_root);
    expect(missing_selector_process.exit_code == 2,
        "#1055: display-value-object without target selectors should fail during launch parsing");
    expect(visual_object_property(missing_selector_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object without target selectors should not mutate the asset");

    const fs::path missing_value_path = temp_root / "missing_value.scx";
    write_synthetic_form_table_for_object_display_value(missing_value_path);
    const auto missing_value_process = run_process_capture(
        studio_host_path,
        {
            "--path", missing_value_path.string(),
            "--display-value-object",
            "--display-value-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(missing_value_process.exit_code == 2,
        "#1055: display-value-object without display-value should fail during launch parsing");
    expect(visual_object_property(missing_value_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object without display-value should not mutate the asset");

    const fs::path duplicate_path = temp_root / "duplicate.scx";
    write_synthetic_form_table_for_object_display_value(duplicate_path);
    const auto duplicate_process = run_process_capture(
        studio_host_path,
        {
            "--path", duplicate_path.string(),
            "--display-value-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--display-value-target-object-name", "cboCustomer",
            "--json"
        },
        temp_root);
    expect(duplicate_process.exit_code == 4,
        "#1055: duplicate-target host object display-value assignment should return command failure");
    expect(visual_object_property(duplicate_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: duplicate-target host object display-value assignment should not mutate the asset");

    const fs::path ambiguous_path = temp_root / "ambiguous.scx";
    write_synthetic_form_table_for_object_display_value(ambiguous_path);
    const auto ambiguous_process = run_process_capture(
        studio_host_path,
        {
            "--path", ambiguous_path.string(),
            "--display-value-object",
            "--left-column-object",
            "--display-value", "Bob",
            "--display-value-target-unique-id", "one-guid",
            "--left-column", "7",
            "--left-column-target-unique-id", "one-guid",
            "--json"
        },
        temp_root);
    expect(ambiguous_process.exit_code == 2,
        "#1055: display-value-object plus left-column-object requests should fail during launch parsing");
    expect(visual_object_property(ambiguous_path, "one-guid", "DISPLAYVALUE") == "Alice",
        "#1055: display-value-object ambiguity should not mutate the asset");

    if (failures == 0) {
        fs::remove_all(temp_root, ignored);
    }
}

}  // namespace cf_test_studio_host_json

