// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#ifndef COPPERFIN_TEST_VISUAL_ASSET_EDITOR_SUPPORT_H
#define COPPERFIN_TEST_VISUAL_ASSET_EDITOR_SUPPORT_H

#include "copperfin/vfp/dbf_table.h"
#include "copperfin/vfp/visual_asset_editor.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <cmath>
#include <limits>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#define _getpid getpid
#endif
#include <string>
#include <vector>





namespace cf_test_visual_asset_editor {

inline std::string vfp_free_table_field_name(std::string_view logical_name) {
    constexpr std::size_t max_field_name_bytes = 10U;
    return std::string(logical_name.substr(0U, max_field_name_bytes));
}

struct SyntheticNamedVisualObject;

// ==== Shared test helpers and fixtures ====

extern int failures;
void expect(bool condition, const std::string& message);
double parse_number(const std::string& text);
const copperfin::vfp::DbfRecordValue* find_record_field(
    const copperfin::vfp::DbfRecord& record,
    const std::string& field_name);
void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value);
void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value);
void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value);
std::vector<std::uint8_t> read_file_bytes(const std::filesystem::path& path);
void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length);
void write_synthetic_direct_and_memo_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::string& direct_field_name,
    const std::string& direct_field_value,
    const std::string& memo_field_name,
    const std::string& memo_field_value);
void write_synthetic_named_object_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::vector<SyntheticNamedVisualObject>& objects);
void write_synthetic_named_direct_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path);
void write_synthetic_named_geometry_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path);
std::string vfp_string_literal_for_test(const std::string& value);
void test_visual_asset_editor_errors_resolve_through_localization_catalog();
void test_visual_asset_editor_default_catalog_refreshes_when_locale_changes();
void test_visual_asset_raw_reorder_preserves_frx_frt_and_lbx_lbt_bytes();
void test_visual_asset_raw_duplicate_preserves_frx_frt_and_lbx_lbt_bytes();
void test_visual_asset_raw_create_preserves_frx_frt_and_lbx_lbt_records();
void test_visual_asset_raw_batch_and_subtree_edits_preserve_bytes();
void test_visual_asset_raw_malformed_layouts_fail_without_writes();
void test_visual_asset_raw_create_preserves_binary_memo_payloads();
void test_visual_asset_raw_create_write_faults_are_atomic();
void test_visual_asset_raw_unicode_path_transaction_round_trip();
void test_update_visual_object_property_preserves_equals_for_blank_property_values();
void test_update_visual_object_report_settings_property_preserves_comment_lines();
void test_report_settings_bottom_margin_memo_round_trips();
void test_report_printer_duplex_and_winspool_settings_are_admitted();
void test_report_settings_fallback_root_gridv_round_trips();
void test_report_settings_case_insensitive_expr_field_round_trips();
void test_report_settings_topmargin_and_tag_memo_round_trips();
void test_fractional_report_section_moves_follow_layout_membership();
void test_visual_geometry_parsing_uses_invariant_decimal_text();
void test_report_label_character_field_writes_preserve_leading_spaces();
template <typename AssignTextProperty>
void test_visual_object_text_property_assigns_text(
    const std::string& issue_id,
    const std::string& temp_suffix,
    const std::string& property_name,
    const std::string& field_name,
    const std::string& property_label,
    const std::string& first_value,
    const std::string& second_value,
    const std::string& other_value,
    const std::string& replacement_value,
    const std::string& unique_id_value,
    AssignTextProperty assign_property) {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_" + temp_suffix + "_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto label = issue_id + ": " + property_label;
    const fs::path table_path = temp_dir / (temp_suffix + ".scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = vfp_free_table_field_name(field_name), .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", first_value},
        {"txtTwo", "twoBox", "two-guid", second_value},
        {"txtOther", "otherBox", "other-guid", other_value}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, label + " fixture should be writable");

    const auto property_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, label + " fixture property should be readable");
        return result.value;
    };
    const auto property_value = [&](const std::string& unique_id) {
        return property_for(table_path.string(), unique_id);
    };
    const auto direct_state = [&]() {
        return property_value("one-guid") + "," +
            property_value("two-guid") + "," +
            property_value("other-guid");
    };

    auto edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        replacement_value);
    expect(edit_result.ok, label + " assignment should support object-name and record-index selectors");
    expect(edit_result.affected_object_count == 2U,
        "#1002: " + label + " successful assignment should report affected object count");
    expect(property_value("one-guid") == replacement_value &&
            property_value("two-guid") == replacement_value &&
            property_value("other-guid") == other_value,
        label + " direct assignment should write raw text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " first write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " second write should remain undo-backed");
    expect(direct_state() == first_value + "," + second_value + "," + other_value,
        label + " undo should restore original direct values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        unique_id_value);
    expect(edit_result.ok, label + " assignment should support UNIQUEID selectors");
    expect(property_value("one-guid") == unique_id_value &&
            property_value("two-guid") == unique_id_value,
        label + " direct assignment should store caller text without serialized quoting");

    const std::string committed_state = direct_state();
    edit_result = assign_property(table_path.string(), {}, "Ignored");
    expect(!edit_result.ok, label + " assignment should reject empty selections");
    expect(edit_result.affected_object_count == 0U,
        "#1002: " + label + " failed assignment should report zero affected objects");
    expect(direct_state() == committed_state, label + " empty-selection failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        "Ignored");
    expect(!edit_result.ok, label + " assignment should reject missing selected objects");
    expect(direct_state() == committed_state, label + " missing-object failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        "Ignored");
    expect(!edit_result.ok, label + " assignment should reject duplicate selected objects");
    expect(direct_state() == committed_state, label + " duplicate-selection failures should not mutate values");

    const fs::path blob_path = temp_dir / (temp_suffix + "_blob.scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", property_name + " = " + vfp_string_literal_for_test(first_value) +
            "\r\nCaption = \"Name\"\r\n"},
        {"txtNoValue", "no-value-guid", "Caption = \"No value\"\r\n"},
        {"txtOther", "other-guid", property_name + " = " + vfp_string_literal_for_test(other_value) + "\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, label + " property-blob fixture should be writable");

    const auto blob_property_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    const std::string serialized_value = replacement_value + ".\"quoted\"";
    edit_result = assign_property(
        blob_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoValue", .unique_id = {}}
        },
        serialized_value);
    expect(edit_result.ok, label + " assignment should support existing and absent serialized properties");
    auto blob_value = blob_property_state("blob-guid");
    auto appended_value = blob_property_state("no-value-guid");
    auto other_blob_value = blob_property_state("other-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == vfp_string_literal_for_test(serialized_value) &&
            appended_value.ok && appended_value.exists &&
                appended_value.value == vfp_string_literal_for_test(serialized_value) &&
            other_blob_value.ok && other_blob_value.exists &&
                other_blob_value.value == vfp_string_literal_for_test(other_value),
        label + " serialized assignment should quote text, append missing property, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " appended serialized write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " existing serialized write should remain undo-backed");
    blob_value = blob_property_state("blob-guid");
    appended_value = blob_property_state("no-value-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == vfp_string_literal_for_test(first_value) &&
            appended_value.ok && !appended_value.exists,
        label + " serialized undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / ("missing_" + temp_suffix + ".scx");
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
    expect(incomplete_create.ok, label + " missing-carrier fixture should be writable");

    edit_result = assign_property(
        incomplete_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        "Ignored");
    expect(!edit_result.ok, label + " assignment should reject objects without a writable carrier");

    fs::remove_all(temp_dir, ignored);
}
template <typename AssignNumericProperty>
void test_visual_object_non_negative_numeric_property_assigns_value(
    const std::string& issue_id,
    const std::string& temp_suffix,
    const std::string& property_name,
    const std::string& field_name,
    const std::string& property_label,
    int first_value,
    int second_value,
    int other_value,
    int replacement_value,
    int unique_id_value,
    AssignNumericProperty assign_property) {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_" + temp_suffix + "_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto label = issue_id + ": " + property_label;
    const fs::path table_path = temp_dir / (temp_suffix + ".scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = vfp_free_table_field_name(field_name), .type = 'C', .length = 10U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", std::to_string(first_value)},
        {"txtTwo", "twoBox", "two-guid", std::to_string(second_value)},
        {"txtOther", "otherBox", "other-guid", std::to_string(other_value)}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, label + " fixture should be writable");

    const auto property_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, label + " fixture property should be readable");
        return result.value;
    };
    const auto property_value = [&](const std::string& unique_id) {
        return property_for(table_path.string(), unique_id);
    };
    const auto direct_state = [&]() {
        return property_value("one-guid") + "," +
            property_value("two-guid") + "," +
            property_value("other-guid");
    };

    auto edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        replacement_value);
    expect(edit_result.ok, label + " assignment should support object-name and record-index selectors");
    expect(edit_result.affected_object_count == 2U,
        "#1003: " + label + " successful numeric assignment should report affected object count");
    expect(property_value("one-guid") == std::to_string(replacement_value) &&
            property_value("two-guid") == std::to_string(replacement_value) &&
            property_value("other-guid") == std::to_string(other_value),
        label + " direct assignment should write unquoted numeric text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " first write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " second write should remain undo-backed");
    expect(direct_state() == std::to_string(first_value) + "," +
            std::to_string(second_value) + "," + std::to_string(other_value),
        label + " undo should restore original direct values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        unique_id_value);
    expect(edit_result.ok, label + " assignment should support UNIQUEID selectors");
    expect(property_value("one-guid") == std::to_string(unique_id_value) &&
            property_value("two-guid") == std::to_string(unique_id_value),
        label + " direct assignment should store caller numeric value");

    const std::string committed_state = direct_state();
    edit_result = assign_property(table_path.string(), {}, replacement_value);
    expect(!edit_result.ok, label + " assignment should reject empty selections");
    expect(edit_result.affected_object_count == 0U,
        "#1003: " + label + " failed numeric assignment should report zero affected objects");
    expect(direct_state() == committed_state, label + " empty-selection failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"}
        },
        -1);
    expect(!edit_result.ok, label + " assignment should reject negative values");
    expect(direct_state() == committed_state, label + " negative-value failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        replacement_value);
    expect(!edit_result.ok, label + " assignment should reject missing selected objects");
    expect(direct_state() == committed_state, label + " missing-object failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        replacement_value);
    expect(!edit_result.ok, label + " assignment should reject duplicate selected objects");
    expect(direct_state() == committed_state, label + " duplicate-selection failures should not mutate values");

    const fs::path blob_path = temp_dir / (temp_suffix + "_blob.scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", property_name + " = " + std::to_string(first_value) +
            "\r\nCaption = \"Name\"\r\n"},
        {"txtNoValue", "no-value-guid", "Caption = \"No value\"\r\n"},
        {"txtOther", "other-guid", property_name + " = " + std::to_string(other_value) + "\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, label + " property-blob fixture should be writable");

    const auto blob_property_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    edit_result = assign_property(
        blob_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoValue", .unique_id = {}}
        },
        replacement_value);
    expect(edit_result.ok, label + " assignment should support existing and absent serialized properties");
    auto blob_value = blob_property_state("blob-guid");
    auto appended_value = blob_property_state("no-value-guid");
    auto other_blob_value = blob_property_state("other-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == std::to_string(replacement_value) &&
            appended_value.ok && appended_value.exists && appended_value.value == std::to_string(replacement_value) &&
            other_blob_value.ok && other_blob_value.exists && other_blob_value.value == std::to_string(other_value),
        label + " serialized assignment should write numeric values, append missing property, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " appended serialized write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " existing serialized write should remain undo-backed");
    blob_value = blob_property_state("blob-guid");
    appended_value = blob_property_state("no-value-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == std::to_string(first_value) &&
            appended_value.ok && !appended_value.exists,
        label + " serialized undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / ("missing_" + temp_suffix + ".scx");
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
    expect(incomplete_create.ok, label + " missing-carrier fixture should be writable");

    edit_result = assign_property(
        incomplete_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        replacement_value);
    expect(!edit_result.ok, label + " assignment should reject objects without a writable carrier");

    fs::remove_all(temp_dir, ignored);
}
template <typename AssignLogicalProperty>
void test_visual_object_logical_property_assigns_state(
    const std::string& issue_id,
    const std::string& temp_suffix,
    const std::string& property_name,
    const std::string& field_name,
    const std::string& property_label,
    AssignLogicalProperty assign_property) {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_" + temp_suffix + "_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto label = issue_id + ": " + property_label;
    const fs::path table_path = temp_dir / (temp_suffix + ".scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = vfp_free_table_field_name(field_name), .type = 'C', .length = 8U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtOne", "oneBox", "one-guid", ".F."},
        {"txtTwo", "twoBox", "two-guid", ".F."},
        {"txtOther", "otherBox", "other-guid", ".T."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, label + " fixture should be writable");

    const auto property_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, label + " fixture property should be readable");
        return result.value;
    };
    const auto property_value = [&](const std::string& unique_id) {
        return property_for(table_path.string(), unique_id);
    };
    const auto direct_state = [&]() {
        return property_value("one-guid") + "," +
            property_value("two-guid") + "," +
            property_value("other-guid");
    };

    auto edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        true);
    expect(edit_result.ok, label + " assignment should support object-name and record-index selectors");
    expect(edit_result.affected_object_count == 2U,
        "#1003: " + label + " successful logical assignment should report affected object count");
    expect(property_value("one-guid") == ".T." &&
            property_value("two-guid") == ".T." &&
            property_value("other-guid") == ".T.",
        label + " direct assignment should write FoxPro logical text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " first write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " second write should remain undo-backed");
    expect(direct_state() == ".F.,.F.,.T.", label + " undo should restore original direct values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "two-guid"}
        },
        true);
    expect(edit_result.ok, label + " assignment should support UNIQUEID selectors");
    expect(property_value("one-guid") == ".T." &&
            property_value("two-guid") == ".T.",
        label + " direct assignment should store caller logical state");

    const std::string committed_state = direct_state();
    edit_result = assign_property(table_path.string(), {}, true);
    expect(!edit_result.ok, label + " assignment should reject empty selections");
    expect(edit_result.affected_object_count == 0U,
        "#1003: " + label + " failed logical assignment should report zero affected objects");
    expect(direct_state() == committed_state, label + " empty-selection failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        true);
    expect(!edit_result.ok, label + " assignment should reject missing selected objects");
    expect(direct_state() == committed_state, label + " missing-object failures should not mutate values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "one-guid"},
            {.record_index = 0U, .object_name = "txtOne", .unique_id = {}}
        },
        true);
    expect(!edit_result.ok, label + " assignment should reject duplicate selected objects");
    expect(direct_state() == committed_state, label + " duplicate-selection failures should not mutate values");

    const fs::path blob_path = temp_dir / (temp_suffix + "_blob.scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", property_name + " = .F.\r\nCaption = \"Name\"\r\n"},
        {"txtNoValue", "no-value-guid", "Caption = \"No value\"\r\n"},
        {"txtOther", "other-guid", property_name + " = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, label + " property-blob fixture should be writable");

    const auto blob_property_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    edit_result = assign_property(
        blob_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoValue", .unique_id = {}}
        },
        true);
    expect(edit_result.ok, label + " assignment should support existing and absent serialized properties");
    auto blob_value = blob_property_state("blob-guid");
    auto appended_value = blob_property_state("no-value-guid");
    auto other_blob_value = blob_property_state("other-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == ".T." &&
            appended_value.ok && appended_value.exists && appended_value.value == ".T." &&
            other_blob_value.ok && other_blob_value.exists && other_blob_value.value == ".T.",
        label + " serialized assignment should write logicals, append missing property, and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " appended serialized write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " existing serialized write should remain undo-backed");
    blob_value = blob_property_state("blob-guid");
    appended_value = blob_property_state("no-value-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == ".F." &&
            appended_value.ok && !appended_value.exists,
        label + " serialized undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / ("missing_" + temp_suffix + ".scx");
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
    expect(incomplete_create.ok, label + " missing-carrier fixture should be writable");

    edit_result = assign_property(
        incomplete_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        true);
    expect(!edit_result.ok, label + " assignment should reject objects without a writable carrier");

    fs::remove_all(temp_dir, ignored);
}
template <typename AssignDynamicProperty>
void test_dynamic_raw_scalar_property_assigns_expression_value(
    const std::string& issue_id,
    const std::string& temp_suffix,
    const std::string& property_name,
    const std::string& field_name,
    const std::string& property_label,
    const std::string& first_value,
    const std::string& second_value,
    const std::string& other_value,
    const std::string& dynamic_expression,
    const std::string& constant_expression,
    AssignDynamicProperty assign_property) {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_" + temp_suffix + "_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto label = issue_id + ": dynamic " + property_label;
    const fs::path table_path = temp_dir / (temp_suffix + ".scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = vfp_free_table_field_name(field_name), .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", first_value},
        {"lblOrders", "ordersLabel", "orders-guid", second_value},
        {"txtOther", "otherBox", "other-guid", other_value}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, label + " fixture should be writable");

    const auto property_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, label + " fixture property should be readable");
        return result.value;
    };
    const auto property_value = [&](const std::string& unique_id) {
        return property_for(table_path.string(), unique_id);
    };
    const auto direct_state = [&]() {
        return property_value("customer-guid") + "," +
            property_value("orders-guid") + "," +
            property_value("other-guid");
    };

    auto edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        dynamic_expression);
    expect(edit_result.ok, label + " assignment should support object-name and record-index selectors");
    expect(property_value("customer-guid") == dynamic_expression &&
            property_value("orders-guid") == dynamic_expression &&
            property_value("other-guid") == other_value,
        label + " direct assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " first write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " second write should remain undo-backed");
    expect(direct_state() == first_value + "," + second_value + "," + other_value,
        label + " undo should restore original direct values");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        constant_expression);
    expect(edit_result.ok, label + " assignment should support UNIQUEID selectors");
    expect(property_value("customer-guid") == constant_expression &&
            property_value("orders-guid") == constant_expression,
        label + " direct assignment should store raw constant expressions");

    const std::string committed_state = direct_state();
    edit_result = assign_property(table_path.string(), {}, dynamic_expression);
    expect(!edit_result.ok, label + " assignment should reject empty selections");
    expect(direct_state() == committed_state, label + " empty-selection failures should not mutate expressions");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        dynamic_expression);
    expect(!edit_result.ok, label + " assignment should reject missing selected objects");
    expect(direct_state() == committed_state, label + " missing-object failures should not mutate expressions");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        dynamic_expression);
    expect(!edit_result.ok, label + " assignment should reject duplicate selected objects");
    expect(direct_state() == committed_state, label + " duplicate-selection failures should not mutate expressions");

    const fs::path blob_path = temp_dir / (temp_suffix + "_blob.scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", property_name + " = " + first_value + "\r\nCaption = \"Customer\"\r\n"},
        {"txtNoValue", "no-value-guid", "Caption = \"No value\"\r\n"},
        {"txtOther", "other-guid", property_name + " = " + other_value + "\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, label + " property-blob fixture should be writable");

    const auto blob_property_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    edit_result = assign_property(
        blob_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoValue", .unique_id = {}}
        },
        dynamic_expression);
    expect(edit_result.ok, label + " assignment should support existing and absent serialized properties");
    auto blob_value = blob_property_state("blob-guid");
    auto appended_value = blob_property_state("no-value-guid");
    auto other_blob_value = blob_property_state("other-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == dynamic_expression &&
            appended_value.ok && appended_value.exists && appended_value.value == dynamic_expression &&
            other_blob_value.ok && other_blob_value.exists && other_blob_value.value == other_value,
        label + " serialized assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " appended serialized write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " existing serialized write should remain undo-backed");
    blob_value = blob_property_state("blob-guid");
    appended_value = blob_property_state("no-value-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == first_value &&
            appended_value.ok && !appended_value.exists,
        label + " serialized undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / ("missing_" + temp_suffix + ".scx");
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
    expect(incomplete_create.ok, label + " missing-carrier fixture should be writable");

    edit_result = assign_property(
        incomplete_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        dynamic_expression);
    expect(!edit_result.ok, label + " assignment should reject objects without a writable carrier");

    fs::remove_all(temp_dir, ignored);
}
template <typename AssignDynamicProperty>
void test_dynamic_logical_font_property_assigns_expression_value(
    const std::string& issue_id,
    const std::string& temp_suffix,
    const std::string& property_name,
    const std::string& field_name,
    const std::string& property_label,
    AssignDynamicProperty assign_property) {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() /
        ("copperfin_visual_editor_" + temp_suffix + "_tests_" + std::to_string(_getpid()));
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto label = issue_id + ": dynamic " + property_label;
    const fs::path table_path = temp_dir / (temp_suffix + ".scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "NAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = vfp_free_table_field_name(field_name), .type = 'C', .length = 96U}
    };
    const std::vector<std::vector<std::string>> records{
        {"txtCustomer", "customerBox", "customer-guid", ".F."},
        {"lblOrders", "ordersLabel", "orders-guid", ".T."},
        {"txtOther", "otherBox", "other-guid", ".F."}
    };
    const auto create_result = copperfin::vfp::create_dbf_table_file(table_path.string(), fields, records);
    expect(create_result.ok, label + " fixture should be writable");

    const auto property_for = [&](const std::string& path, const std::string& unique_id) {
        const auto result = copperfin::vfp::query_visual_object_property({
            .path = path,
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
        expect(result.ok && result.exists, label + " fixture property should be readable");
        return result.value;
    };
    const auto property_value = [&](const std::string& unique_id) {
        return property_for(table_path.string(), unique_id);
    };
    const auto direct_state = [&]() {
        return property_value("customer-guid") + "," +
            property_value("orders-guid") + "," +
            property_value("other-guid");
    };

    const std::string dynamic_expression = "IIF(RECNO() % 2 = 0, .T., .F.)";
    auto edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}},
            {.record_index = 1U, .object_name = {}, .unique_id = {}}
        },
        dynamic_expression);
    expect(edit_result.ok, label + " assignment should support object-name and record-index selectors");
    expect(property_value("customer-guid") == dynamic_expression &&
            property_value("orders-guid") == dynamic_expression &&
            property_value("other-guid") == ".F.",
        label + " direct assignment should write raw expression text and preserve unrelated objects");

    auto undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " first write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(table_path.string());
    expect(undo_result.ok, label + " second write should remain undo-backed");
    expect(direct_state() == ".F.,.T.,.F.", label + " undo should restore original direct values");

    const std::string constant_expression = ".T.";
    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "orders-guid"}
        },
        constant_expression);
    expect(edit_result.ok, label + " assignment should support UNIQUEID selectors");
    expect(property_value("customer-guid") == constant_expression &&
            property_value("orders-guid") == constant_expression,
        label + " direct assignment should store raw constant expressions");

    const std::string committed_state = direct_state();
    edit_result = assign_property(table_path.string(), {}, ".F.");
    expect(!edit_result.ok, label + " assignment should reject empty selections");
    expect(direct_state() == committed_state, label + " empty-selection failures should not mutate expressions");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = {}, .unique_id = "missing-guid"}
        },
        ".F.");
    expect(!edit_result.ok, label + " assignment should reject missing selected objects");
    expect(direct_state() == committed_state, label + " missing-object failures should not mutate expressions");

    edit_result = assign_property(
        table_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "customer-guid"},
            {.record_index = 0U, .object_name = "txtCustomer", .unique_id = {}}
        },
        ".F.");
    expect(!edit_result.ok, label + " assignment should reject duplicate selected objects");
    expect(direct_state() == committed_state, label + " duplicate-selection failures should not mutate expressions");

    const fs::path blob_path = temp_dir / (temp_suffix + "_blob.scx");
    const std::vector<copperfin::vfp::DbfFieldDescriptor> blob_fields{
        {.name = "OBJNAME", .type = 'C', .length = 20U},
        {.name = "UNIQUEID", .type = 'C', .length = 20U},
        {.name = "PROPERTIES", .type = 'M', .length = 4U}
    };
    const std::vector<std::vector<std::string>> blob_records{
        {"txtBlob", "blob-guid", property_name + " = .F.\r\nCaption = \"Customer\"\r\n"},
        {"txtNoValue", "no-value-guid", "Caption = \"No value\"\r\n"},
        {"txtOther", "other-guid", property_name + " = .T.\r\n"}
    };
    const auto blob_create = copperfin::vfp::create_dbf_table_file(blob_path.string(), blob_fields, blob_records);
    expect(blob_create.ok, label + " property-blob fixture should be writable");

    const auto blob_property_state = [&](const std::string& unique_id) {
        return copperfin::vfp::query_visual_object_property({
            .path = blob_path.string(),
            .record_index = 0U,
            .object_name = {},
            .unique_id = unique_id,
            .property_name = property_name
        });
    };

    edit_result = assign_property(
        blob_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "blob-guid"},
            {.record_index = 0U, .object_name = "txtNoValue", .unique_id = {}}
        },
        dynamic_expression);
    expect(edit_result.ok, label + " assignment should support existing and absent serialized properties");
    auto blob_value = blob_property_state("blob-guid");
    auto appended_value = blob_property_state("no-value-guid");
    auto other_value = blob_property_state("other-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == dynamic_expression &&
            appended_value.ok && appended_value.exists && appended_value.value == dynamic_expression &&
            other_value.ok && other_value.exists && other_value.value == ".T.",
        label + " serialized assignment should write raw expressions and preserve unrelated objects");

    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " appended serialized write should remain undo-backed");
    undo_result = copperfin::vfp::undo_visual_object_property(blob_path.string());
    expect(undo_result.ok, label + " existing serialized write should remain undo-backed");
    blob_value = blob_property_state("blob-guid");
    appended_value = blob_property_state("no-value-guid");
    expect(blob_value.ok && blob_value.exists && blob_value.value == ".F." &&
            appended_value.ok && !appended_value.exists,
        label + " serialized undo should restore existing values and remove appended properties");

    const fs::path incomplete_path = temp_dir / ("missing_" + temp_suffix + ".scx");
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
    expect(incomplete_create.ok, label + " missing-carrier fixture should be writable");

    edit_result = assign_property(
        incomplete_path.string(),
        {
            {.record_index = 0U, .object_name = {}, .unique_id = "a-guid"}
        },
        ".F.");
    expect(!edit_result.ok, label + " assignment should reject objects without a writable carrier");

    fs::remove_all(temp_dir, ignored);
}
struct SyntheticNamedVisualObject {
    std::string objname;
    std::string name;
    std::string unique_id;
    std::string properties;
};

// ==== Object lifecycle tests (create/duplicate/delete/rename/reorder/move/group/align/distribute/nudge/resize/snap/deleted-state) ====
const copperfin::vfp::VisualObjectPropertySnapshot* find_property_snapshot(
    const std::vector<copperfin::vfp::VisualObjectPropertySnapshot>& properties,
    const std::string& property_name);
const copperfin::vfp::VisualObjectMethodSnapshot* find_method_snapshot(
    const std::vector<copperfin::vfp::VisualObjectMethodSnapshot>& methods,
    const std::string& method_name);
void test_rename_visual_object_memo_property_updates_selected_object();
void test_rename_visual_object_memo_properties_rolls_back_failed_batches();
void test_reorder_visual_object_memo_properties_within_selected_object();
void test_reorder_visual_object_memo_properties_rolls_back_failed_batches();
void test_duplicate_visual_object_appends_identity_safe_copy();
void test_duplicate_visual_objects_rolls_back_failed_batches();
void test_create_visual_object_appends_toolbox_field_values();
void test_create_visual_objects_rolls_back_failed_batches();
void test_reparent_visual_object_updates_container_parent();
void test_reparent_visual_objects_rolls_back_failed_batches();
void test_align_visual_objects_to_anchor_geometry();
void test_resize_visual_objects_to_anchor_geometry();
void test_distribute_visual_objects_evenly_by_axis();
void test_snap_visual_objects_to_grid_by_axis();
void test_nudge_visual_objects_by_delta();
void test_group_visual_objects_creates_container_and_rolls_back_failures();
void test_rename_visual_object_updates_identity_safely();
void test_rename_visual_objects_rolls_back_failed_batches();
void test_reorder_visual_object_updates_z_order();
void test_reorder_visual_objects_rolls_back_failed_batches();
void test_duplicate_visual_object_subtree_rewrites_copied_parents();

// ==== Visual object method tests (copy/move/delete/rename/reorder/list/query/update) ====
void test_list_visual_object_methods_reads_selected_methods();
void test_query_visual_object_method_reads_one_selected_method();
void test_update_visual_object_method_updates_and_appends_methods();
void test_delete_visual_object_method_removes_selected_methods();
void test_delete_visual_object_methods_rolls_back_failed_batches();
void test_rename_visual_object_method_updates_declarations();
void test_rename_visual_object_methods_rolls_back_failed_batches();
void test_copy_visual_object_method_between_selected_objects();
void test_copy_visual_object_methods_rolls_back_failed_batches();
void test_move_visual_object_method_between_selected_objects();
void test_move_visual_object_methods_rolls_back_failed_batches();
void test_reorder_visual_object_methods_within_selected_object();
void test_reorder_visual_object_methods_rolls_back_failed_batches();

// ==== Visual object property tests (copy/move/rename/reorder/list/query/clear) ====
void test_query_visual_object_property_reads_selected_values();
void test_clear_visual_object_property_resets_selected_values();
void test_clear_visual_object_properties_rolls_back_failed_batches();
void test_copy_visual_object_property_between_selected_objects();
void test_copy_visual_object_properties_rolls_back_failed_batches();
void test_move_visual_object_property_between_selected_objects();
void test_move_visual_object_properties_rolls_back_failed_batches();
void test_list_visual_object_properties_reads_selected_surface();

// ==== Visual object hierarchy and listing tests (ancestors/children/descendants) ====
void test_list_visual_objects_reads_selection_outline();
void test_list_visual_objects_reads_hierarchy_metadata();
void test_ungroup_visual_object_reparents_children_and_marks_container_deleted();
void test_list_visual_object_children_filters_immediate_children();
void test_list_visual_object_descendants_walks_container_tree();
void test_set_visual_object_subtree_deleted_state_updates_descendants();
void test_set_visual_object_deleted_state_is_undoable();
void test_list_visual_object_ancestors_walks_parent_chain();

// ==== Property/method update and batch round-trip tests ====
void test_update_visual_object_property_rewrites_properties_memo();
void test_visual_asset_undo_rejects_corrupt_journals_without_mutating_assets();
void test_visual_asset_undo_journal_filenames_are_locale_invariant();
void test_update_visual_object_properties_updates_selected_geometry_fields();
void test_update_visual_object_properties_rolls_back_failed_batches();
void test_update_visual_object_batch_undoes_report_and_label_batches_in_single_step();
void test_update_visual_object_batch_moves_report_and_label_band_contents();
void test_visual_asset_memo_writes_are_failure_atomic();
void test_update_visual_object_batch_rolls_back_failed_alignment();
void test_update_visual_object_property_skips_noop_writes();
void test_update_visual_object_property_skips_noop_writes_for_report_and_label_assets();
void test_update_visual_object_property_preserves_unsupported_report_and_label_metadata();
void test_update_visual_object_property_preserves_report_and_label_sibling_rows();
void test_report_and_label_asset_inspection_is_a_noop_binary_round_trip();
void test_update_visual_object_property_targets_selected_object_name();
void test_update_visual_object_property_targets_selected_unique_id();
void test_update_visual_object_property_matches_property_names_case_insensitively();
void test_update_visual_object_property_targets_selected_object_name_direct_field();
void test_update_visual_object_property_rewrites_direct_fields();
void test_update_visual_object_property_round_trips_added_vcx_property();
void test_update_visual_object_property_round_trips_label_and_menu_assets();
void test_update_visual_object_property_round_trips_project_and_database_assets();

// ==== Property setter tests (behavior properties) ====
void test_set_visual_object_deleted_state_targets_selected_object();
void test_set_visual_object_tab_order_assigns_sequential_indexes();
void test_set_visual_object_tab_stop_assigns_logical_state();
void test_set_visual_object_visibility_assigns_logical_state();
void test_set_visual_object_enabled_assigns_logical_state();
void test_set_visual_object_read_only_assigns_logical_state();
void test_set_visual_object_locked_assigns_logical_state();
void test_set_visual_object_caption_assigns_text();
void test_set_visual_object_tooltip_text_assigns_text();
void test_set_visual_object_status_bar_text_assigns_text();
void test_set_visual_object_control_source_assigns_text();
void test_set_visual_object_current_control_assigns_text();
void test_set_visual_object_sparse_assigns_logical_state();
void test_set_visual_object_closable_assigns_logical_state();
void test_set_visual_object_control_box_assigns_logical_state();
void test_set_visual_object_allow_output_assigns_logical_state();
void test_set_visual_object_auto_center_assigns_logical_state();
void test_set_visual_object_auto_size_assigns_logical_state();
void test_set_visual_object_auto_release_assigns_logical_state();
void test_set_visual_object_auto_verb_menu_assigns_logical_state();
void test_set_visual_object_bind_controls_assigns_logical_state();
void test_set_visual_object_clip_controls_assigns_logical_state();
void test_set_visual_object_dockable_assigns_logical_state();
void test_set_visual_object_continuous_scroll_assigns_logical_state();
void test_set_visual_object_desktop_assigns_logical_state();
void test_set_visual_object_key_preview_assigns_logical_state();
void test_set_visual_object_mac_desktop_assigns_logical_state();
void test_set_visual_object_max_button_assigns_logical_state();
void test_set_visual_object_max_height_assigns_numeric_value();
void test_set_visual_object_max_width_assigns_numeric_value();
void test_set_visual_object_max_left_assigns_numeric_value();
void test_set_visual_object_max_top_assigns_numeric_value();
void test_set_visual_object_min_button_assigns_logical_state();
void test_set_visual_object_min_height_assigns_numeric_value();
void test_set_visual_object_min_width_assigns_numeric_value();
void test_set_visual_object_movable_assigns_logical_state();
void test_set_visual_object_half_height_caption_assigns_logical_state();
void test_set_visual_object_mdi_form_assigns_logical_state();
void test_set_visual_object_whats_this_button_assigns_logical_state();
void test_set_visual_object_whats_this_help_assigns_logical_state();
void test_set_visual_object_whats_this_help_id_assigns_numeric_value();
void test_set_visual_object_help_context_id_assigns_numeric_value();
void test_set_visual_object_display_orientation_assigns_numeric_value();
void test_set_visual_object_tab_orientation_assigns_numeric_value();
void test_set_visual_object_list_item_id_assigns_numeric_value();
void test_set_visual_object_lock_screen_assigns_logical_state();
void test_set_visual_object_hide_selection_assigns_logical_state();
void test_set_visual_object_allow_cell_selection_assigns_logical_state();
void test_set_visual_object_delete_mark_assigns_logical_state();
void test_set_visual_object_record_mark_assigns_logical_state();
void test_set_visual_object_split_bar_assigns_logical_state();
void test_set_visual_object_highlight_row_assigns_logical_state();
void test_set_visual_object_panel_link_assigns_logical_state();
void test_set_visual_object_allow_header_sizing_assigns_logical_state();
void test_set_visual_object_allow_row_sizing_assigns_logical_state();
void test_set_visual_object_resizable_assigns_logical_state();
void test_set_visual_object_add_line_feeds_assigns_logical_state();
void test_set_visual_object_always_on_top_assigns_logical_state();
void test_set_visual_object_always_on_bottom_assigns_logical_state();
void test_set_visual_object_style_assigns_numeric_value();
void test_set_visual_object_deleted_states_rolls_back_batch_failures();

// ==== Property setter tests (appearance properties): colors ====
void test_set_visual_object_border_color_assigns_numeric_value();
void test_set_visual_object_grid_line_color_assigns_numeric_value();
void test_set_visual_object_fill_color_assigns_numeric_value();
void test_set_visual_object_selected_back_color_assigns_numeric_value();
void test_set_visual_object_selected_fore_color_assigns_numeric_value();
void test_set_visual_object_selected_item_back_color_assigns_numeric_value();
void test_set_visual_object_selected_item_fore_color_assigns_numeric_value();
void test_set_visual_object_disabled_item_back_color_assigns_numeric_value();
void test_set_visual_object_disabled_item_fore_color_assigns_numeric_value();
void test_set_visual_object_item_back_color_assigns_numeric_value();
void test_set_visual_object_item_fore_color_assigns_numeric_value();
void test_set_visual_object_highlight_back_color_assigns_numeric_value();
void test_set_visual_object_highlight_fore_color_assigns_numeric_value();
void test_set_visual_object_back_color_assigns_numeric_value();
void test_set_visual_object_fore_color_assigns_numeric_value();
void test_set_visual_object_disabled_back_color_assigns_numeric_value();
void test_set_visual_object_disabled_fore_color_assigns_numeric_value();
void test_set_visual_object_dynamic_back_color_assigns_expression_value();
void test_set_visual_object_dynamic_fore_color_assigns_expression_value();

// ==== Property setter tests (appearance properties): fonts ====
void test_set_visual_object_font_name_assigns_text();
void test_set_visual_object_font_size_assigns_numeric_value();
void test_set_visual_object_font_bold_assigns_logical_state();
void test_set_visual_object_font_italic_assigns_logical_state();
void test_set_visual_object_font_underline_assigns_logical_state();
void test_set_visual_object_font_strikethru_assigns_logical_state();
void test_set_visual_object_font_outline_assigns_logical_state();
void test_set_visual_object_font_shadow_assigns_logical_state();
void test_set_visual_object_dynamic_font_name_assigns_expression_value();
void test_set_visual_object_dynamic_font_size_assigns_expression_value();
void test_set_visual_object_dynamic_line_height_assigns_expression_value();
void test_set_visual_object_dynamic_font_bold_assigns_expression_value();
void test_set_visual_object_dynamic_font_italic_assigns_expression_value();
void test_set_visual_object_dynamic_font_underline_assigns_expression_value();
void test_set_visual_object_dynamic_font_strikethru_assigns_expression_value();
void test_set_visual_object_dynamic_font_outline_assigns_expression_value();
void test_set_visual_object_dynamic_font_shadow_assigns_expression_value();

// ==== Property setter tests (appearance properties): pictures and icons ====
void test_set_visual_object_picture_assigns_text();
void test_set_visual_object_down_picture_assigns_text();
void test_set_visual_object_disabled_picture_assigns_text();
void test_set_visual_object_ole_drag_picture_assigns_text();
void test_set_visual_object_mouse_icon_assigns_text();
void test_set_visual_object_drag_icon_assigns_text();
void test_set_visual_object_picture_margin_assigns_numeric_value();
void test_set_visual_object_picture_position_assigns_numeric_value();
void test_set_visual_object_picture_spacing_assigns_numeric_value();
void test_set_visual_object_picture_selection_display_assigns_numeric_value();

// ==== Property setter tests (appearance properties): borders, fill, and drawing ====
void test_set_visual_object_back_style_assigns_numeric_value();
void test_set_visual_object_border_style_assigns_numeric_value();
void test_set_visual_object_border_width_assigns_numeric_value();
void test_set_visual_object_grid_line_width_assigns_numeric_value();
void test_set_visual_object_grid_lines_assigns_numeric_value();
void test_set_visual_object_special_effect_assigns_numeric_value();
void test_set_visual_object_curvature_assigns_numeric_value();
void test_set_visual_object_draw_mode_assigns_numeric_value();
void test_set_visual_object_draw_style_assigns_numeric_value();
void test_set_visual_object_draw_width_assigns_numeric_value();
void test_set_visual_object_fill_style_assigns_numeric_value();

// ==== Property setter tests (appearance properties): window and display chrome ====
void test_set_visual_object_highlight_row_line_width_assigns_numeric_value();
void test_set_visual_object_highlight_style_assigns_numeric_value();
void test_set_visual_object_header_height_assigns_numeric_value();
void test_set_visual_object_row_height_assigns_numeric_value();
void test_set_visual_object_scale_mode_assigns_numeric_value();
void test_set_visual_object_scroll_bars_assigns_numeric_value();
void test_set_visual_object_window_state_assigns_numeric_value();
void test_set_visual_object_show_window_assigns_numeric_value();
void test_set_visual_object_title_bar_assigns_numeric_value();
void test_set_visual_object_mouse_pointer_assigns_numeric_value();
void test_set_visual_object_input_mask_assigns_text();
void test_set_visual_object_format_assigns_text();
void test_set_visual_object_dynamic_alignment_assigns_expression_value();
void test_set_visual_object_dynamic_current_control_assigns_expression_value();
void test_set_visual_object_dynamic_input_mask_assigns_expression_value();

// ==== Property setter tests (appearance properties): OLE drag/drop and buffering ====
void test_set_visual_object_drag_mode_assigns_numeric_value();
void test_set_visual_object_ole_drag_mode_assigns_numeric_value();
void test_set_visual_object_ole_drop_mode_assigns_numeric_value();
void test_set_visual_object_ole_drop_effects_assigns_numeric_value();
void test_set_visual_object_ole_drop_text_insertion_assigns_numeric_value();
void test_set_visual_object_buffer_mode_assigns_numeric_value();
void test_set_visual_object_buffer_mode_override_assigns_numeric_value();

// ==== Property setter tests (data-binding properties) ====
void test_set_visual_object_lock_columns_assigns_numeric_value();
void test_set_visual_object_lock_columns_left_assigns_numeric_value();
void test_set_visual_object_record_source_assigns_text();
void test_set_visual_object_link_master_assigns_text();
void test_set_visual_object_initial_selected_alias_assigns_text();
void test_set_visual_object_default_file_path_assigns_text();
void test_set_visual_object_form_set_class_assigns_text();
void test_set_visual_object_record_source_type_assigns_numeric_value();
void test_set_visual_object_partition_assigns_numeric_value();
void test_set_visual_object_column_order_assigns_numeric_value();
void test_set_visual_object_child_order_assigns_numeric_value();
void test_set_visual_object_data_session_assigns_numeric_value();
void test_set_visual_object_row_source_assigns_text();
void test_set_visual_object_row_source_type_assigns_numeric_value();
void test_set_visual_object_bound_column_assigns_numeric_value();
void test_set_visual_object_button_count_assigns_numeric_value();
void test_set_visual_object_column_count_assigns_numeric_value();
void test_set_visual_object_column_widths_assigns_text();
void test_set_visual_object_column_lines_assigns_logical_state();
void test_set_visual_object_integral_height_assigns_logical_state();
void test_set_visual_object_incremental_search_assigns_logical_state();
void test_set_visual_object_multi_select_assigns_logical_state();
void test_set_visual_object_list_index_assigns_numeric_value();
void test_set_visual_object_left_column_assigns_numeric_value();
void test_set_visual_object_display_value_assigns_text();

}  // namespace cf_test_visual_asset_editor

#endif
