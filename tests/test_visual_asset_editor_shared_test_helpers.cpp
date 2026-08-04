// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_visual_asset_editor_support.h"
#include "test_environment_support.h"
#include "copperfin/platform/invariant_numeric.h"

namespace cf_test_visual_asset_editor {
int failures = 0;

const copperfin::vfp::VisualObjectPropertySnapshot* find_property_snapshot(
    const std::vector<copperfin::vfp::VisualObjectPropertySnapshot>& properties,
    const std::string& property_name) {
    const auto value = std::find_if(properties.begin(), properties.end(), [&](const auto& candidate) {
        return candidate.property_name == property_name;
    });
    return value == properties.end() ? nullptr : &(*value);
}

const copperfin::vfp::VisualObjectMethodSnapshot* find_method_snapshot(
    const std::vector<copperfin::vfp::VisualObjectMethodSnapshot>& methods,
    const std::string& method_name) {
    const auto value = std::find_if(methods.begin(), methods.end(), [&](const auto& candidate) {
        return candidate.method_name == method_name;
    });
    return value == methods.end() ? nullptr : &(*value);
}

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

double parse_number(const std::string& text) {
    return copperfin::platform::try_parse_invariant_double(text).value_or(
        std::numeric_limits<double>::quiet_NaN());
}

const copperfin::vfp::DbfRecordValue* find_record_field(
    const copperfin::vfp::DbfRecord& record,
    const std::string& field_name) {
    const auto value = std::find_if(record.values.begin(), record.values.end(), [&](const auto& candidate) {
        return candidate.field_name == field_name;
    });
    return value == record.values.end() ? nullptr : &(*value);
}

void write_le_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void write_le_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void write_be_u16(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_be_u32(std::vector<std::uint8_t>& bytes, std::size_t offset, std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>(value & 0xFFU);
}

void write_ascii(std::vector<std::uint8_t>& bytes, std::size_t offset, const std::string& value) {
    for (std::size_t index = 0; index < value.size(); ++index) {
        bytes[offset + index] = static_cast<std::uint8_t>(value[index]);
    }
}

std::vector<std::uint8_t> read_file_bytes(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

void write_field_descriptor(
    std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    const std::string& name,
    char type,
    std::uint32_t field_offset,
    std::uint8_t field_length) {
    write_ascii(bytes, offset, name);
    bytes[offset + 11U] = static_cast<std::uint8_t>(type);
    write_le_u32(bytes, offset + 12U, field_offset);
    bytes[offset + 16U] = field_length;
}

void write_synthetic_direct_and_memo_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::string& direct_field_name,
    const std::string& direct_field_value,
    const std::string& memo_field_name,
    const std::string& memo_field_value) {
    std::vector<std::uint8_t> table_bytes(122U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, 1U);
    write_le_u16(table_bytes, 8U, 97U);
    write_le_u16(table_bytes, 10U, 25U);
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, direct_field_name, 'C', 1U, 20U);
    write_field_descriptor(table_bytes, 64U, memo_field_name, 'M', 21U, 4U);
    table_bytes[96] = 0x0DU;

    table_bytes[97] = 0x20U;
    std::fill_n(table_bytes.begin() + 98U, 20U, static_cast<std::uint8_t>(' '));
    write_ascii(table_bytes, 98U, direct_field_value);
    write_le_u32(table_bytes, 118U, 1U);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    std::vector<std::uint8_t> memo_bytes(1024U, 0U);
    write_be_u32(memo_bytes, 0U, 2U);
    write_be_u16(memo_bytes, 6U, 512U);
    memo_bytes[512 + 3] = 1U;
    write_be_u32(memo_bytes, 512 + 4, static_cast<std::uint32_t>(memo_field_value.size()));
    write_ascii(memo_bytes, 520U, memo_field_value);

    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

void write_synthetic_named_object_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path,
    const std::vector<SyntheticNamedVisualObject>& objects) {
    constexpr std::size_t header_length = 161U;
    constexpr std::size_t record_length = 37U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * objects.size()) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(objects.size()));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "UNIQUEID", 'M', 29U, 4U);
    write_field_descriptor(table_bytes, 128U, "PROPERTIES", 'M', 33U, 4U);
    table_bytes[160] = 0x0DU;
    table_bytes[header_length + (record_length * objects.size())] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(512U * (objects.size() * 3U + 2U), 0U);
    write_be_u16(memo_bytes, 6U, 512U);
    std::uint32_t next_block = 1U;
    auto write_memo = [&](const std::string& value) {
        const std::uint32_t block = next_block++;
        const std::size_t offset = static_cast<std::size_t>(block) * 512U;
        memo_bytes[offset + 3U] = 1U;
        write_be_u32(memo_bytes, offset + 4U, static_cast<std::uint32_t>(value.size()));
        write_ascii(memo_bytes, offset + 8U, value);
        return block;
    };

    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& object = objects[index];
        const std::size_t record_offset = header_length + (record_length * index);
        table_bytes[record_offset] = 0x20U;
        if (!object.objname.empty()) {
            write_le_u32(table_bytes, record_offset + 1U, write_memo(object.objname));
        }
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, object.name);
        if (!object.unique_id.empty()) {
            write_le_u32(table_bytes, record_offset + 29U, write_memo(object.unique_id));
        }
        if (!object.properties.empty()) {
            write_le_u32(table_bytes, record_offset + 33U, write_memo(object.properties));
        }
    }
    write_be_u32(memo_bytes, 0U, next_block);

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

void write_synthetic_named_direct_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path) {
    constexpr std::size_t header_length = 129U;
    constexpr std::size_t record_length = 39U;
    constexpr std::size_t record_count = 2U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * record_count) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(record_count));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "HPOS", 'N', 29U, 10U);
    table_bytes[128] = 0x0DU;
    table_bytes[header_length + (record_length * record_count)] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(1536U, 0U);
    write_be_u32(memo_bytes, 0U, 3U);
    write_be_u16(memo_bytes, 6U, 512U);
    memo_bytes[512U + 3U] = 1U;
    write_be_u32(memo_bytes, 512U + 4U, 7U);
    write_ascii(memo_bytes, 520U, "cmdSave");
    memo_bytes[1024U + 3U] = 1U;
    write_be_u32(memo_bytes, 1024U + 4U, 7U);
    write_ascii(memo_bytes, 1032U, "txtName");

    const auto write_record = [&](std::size_t record_index, std::uint32_t objname_block, const std::string& name, const std::string& hpos) {
        const std::size_t record_offset = header_length + (record_length * record_index);
        table_bytes[record_offset] = 0x20U;
        write_le_u32(table_bytes, record_offset + 1U, objname_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, name);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 29U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 29U + (10U - hpos.size()), hpos);
    };
    write_record(0U, 1U, "saveButton", "111.000");
    write_record(1U, 2U, "nameBox", "222.000");

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

void write_synthetic_named_geometry_asset(
    const std::filesystem::path& table_path,
    const std::filesystem::path& memo_path) {
    constexpr std::size_t header_length = 193U;
    constexpr std::size_t record_length = 53U;
    constexpr std::size_t record_count = 2U;
    std::vector<std::uint8_t> table_bytes(header_length + (record_length * record_count) + 1U, 0U);
    table_bytes[0] = 0x30U;
    table_bytes[1] = 126U;
    table_bytes[2] = 4U;
    table_bytes[3] = 7U;
    write_le_u32(table_bytes, 4U, static_cast<std::uint32_t>(record_count));
    write_le_u16(table_bytes, 8U, static_cast<std::uint16_t>(header_length));
    write_le_u16(table_bytes, 10U, static_cast<std::uint16_t>(record_length));
    table_bytes[28] = 0x00U;
    table_bytes[29] = 0x03U;

    write_field_descriptor(table_bytes, 32U, "OBJNAME", 'M', 1U, 4U);
    write_field_descriptor(table_bytes, 64U, "NAME", 'C', 5U, 24U);
    write_field_descriptor(table_bytes, 96U, "UNIQUEID", 'M', 29U, 4U);
    write_field_descriptor(table_bytes, 128U, "HPOS", 'N', 33U, 10U);
    write_field_descriptor(table_bytes, 160U, "VPOS", 'N', 43U, 10U);
    table_bytes[192] = 0x0DU;
    table_bytes[header_length + (record_length * record_count)] = 0x1AU;

    std::vector<std::uint8_t> memo_bytes(2560U, 0U);
    write_be_u32(memo_bytes, 0U, 5U);
    write_be_u16(memo_bytes, 6U, 512U);
    const auto write_memo = [&](std::uint32_t block, const std::string& value) {
        const std::size_t offset = static_cast<std::size_t>(block) * 512U;
        memo_bytes[offset + 3U] = 1U;
        write_be_u32(memo_bytes, offset + 4U, static_cast<std::uint32_t>(value.size()));
        write_ascii(memo_bytes, offset + 8U, value);
    };
    write_memo(1U, "cmdSave");
    write_memo(2U, "first-guid");
    write_memo(3U, "txtName");
    write_memo(4U, "target-guid");

    const auto write_record = [&](
        std::size_t record_index,
        std::uint32_t objname_block,
        const std::string& name,
        std::uint32_t unique_id_block,
        const std::string& hpos,
        const std::string& vpos) {
        const std::size_t record_offset = header_length + (record_length * record_index);
        table_bytes[record_offset] = 0x20U;
        write_le_u32(table_bytes, record_offset + 1U, objname_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 5U), 24U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 5U, name);
        write_le_u32(table_bytes, record_offset + 29U, unique_id_block);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 33U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 33U + (10U - hpos.size()), hpos);
        std::fill_n(table_bytes.begin() + static_cast<std::ptrdiff_t>(record_offset + 43U), 10U, static_cast<std::uint8_t>(' '));
        write_ascii(table_bytes, record_offset + 43U + (10U - vpos.size()), vpos);
    };
    write_record(0U, 1U, "saveButton", 2U, "111.000", "211.000");
    write_record(1U, 3U, "nameBox", 4U, "222.000", "322.000");

    {
        std::ofstream output(table_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }
    {
        std::ofstream output(memo_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }
}

std::string vfp_string_literal_for_test(const std::string& value) {
    std::string literal = "\"";
    for (char ch : value) {
        if (ch == '"') {
            literal += "\"\"";
        } else {
            literal += ch;
        }
    }
    literal += "\"";
    return literal;
}

void test_visual_asset_editor_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    expect(english_catalog.translate("VisualAssetEditor.Object.RecordUnavailable") ==
               "The requested object record is not currently available." &&
               english_catalog.translate("VisualAssetEditor.Object.UniqueIdNotFound") ==
                   "No visual object with the requested unique id was found." &&
               english_catalog.translate("VisualAssetEditor.Method.NotFound") ==
                   "The requested method was not found." &&
               english_catalog.translate("VisualAssetEditor.Property.TargetObjectAlreadyHasProperty") ==
                   "The target object already has the requested property." &&
               english_catalog.translate("VisualAssetEditor.Identity.ReplacementFieldMissing") ==
                   "The requested replacement identity field is not present in the asset." &&
               english_catalog.translate("VisualAssetEditor.Field.NotFound") ==
                   "The requested field was not found in the asset." &&
               english_catalog.translate("VisualAssetEditor.Geometry.GridWidthPositiveRequired") ==
                   "Grid width must be positive for horizontal snapping." &&
               english_catalog.translate(
                   "VisualAssetEditor.Property.NonNegativeRequired",
                   {{"propertyName", "BackColor"}}) == "BackColor must not be negative." &&
               english_catalog.translate("VisualAssetEditor.Operation.AssetPathRequired") ==
                   "No asset path was provided." &&
               english_catalog.translate("VisualAssetEditor.Object.AlignmentTargetsRequired") ==
                   "No visual object alignment targets were provided." &&
               english_catalog.translate("VisualAssetEditor.Object.GridSnappingModeUnsupported") ==
                   "Unsupported visual object grid snapping mode." &&
               english_catalog.translate("VisualAssetEditor.Method.PlacementUnsupported") ==
                   "Unknown method placement was requested." &&
               english_catalog.translate("VisualAssetEditor.Property.ChangeBatchRequired") ==
                   "No property changes were provided." &&
               english_catalog.translate(
                   "VisualAssetEditor.Operation.RollbackFailed",
                   {{"error", "No property name was provided."}, {"rollbackError", "undo failed"}}) ==
                   "No property name was provided. Rollback failed: undo failed" &&
               english_catalog.translate("VisualAssetEditor.PropertyLabel.ToolTipText") ==
                   "tooltip text" &&
               english_catalog.translate(
                   "VisualAssetEditor.Object.PropertyAssignmentSelectionRequired",
                   {{"propertyLabel", english_catalog.translate("VisualAssetEditor.PropertyLabel.ToolTipText")}}) ==
                   "No visual objects were selected for tooltip text assignment." &&
               english_catalog.translate(
                   "VisualAssetEditor.Object.PropertyAssignmentDuplicate",
                   {{"propertyLabel", english_catalog.translate("VisualAssetEditor.PropertyLabel.ToolTipText")}}) ==
                   "The same visual object was selected more than once for tooltip text assignment." &&
               english_catalog.translate("VisualAssetEditor.Object.TabStopSelectionDuplicate") ==
                   "The same visual object was selected more than once for tab-stop assignment." &&
               english_catalog.translate("VisualAssetEditor.Storage.TableOpenFailed") ==
                   "Unable to open the visual asset table." &&
               english_catalog.translate("VisualAssetEditor.Undo.PropertyLookupMismatch") ==
                   "Property lookup mismatch while recording undo." &&
               english_catalog.translate(
                   "VisualAssetEditor.Object.SelectedFieldOrPropertyMissing",
                   {{"fieldName", "TABSTOP"}}) ==
                   "The selected object does not expose a TABSTOP field or property." &&
               english_catalog.translate(
                   "VisualAssetEditor.Undo.PropertyLabel",
                   {{"propertyName", "Caption"}}) ==
                   "Property Caption" &&
               english_catalog.translate(
                   "VisualAssetEditor.Identity.CopiedRowFieldRequired",
                   {{"fieldName", "UNIQUEID"}}) ==
                   "Every copied row must expose a UNIQUEID." &&
               english_catalog.translate("VisualAssetEditor.Object.SelectedContainerChildrenRequired") ==
                   "The selected container has no child objects to ungroup." &&
               english_catalog.translate("VisualAssetEditor.Object.GroupContainerUnavailable") ==
                   "The created group container is not available." &&
               pseudo_catalog.translate("VisualAssetEditor.Object.RecordUnavailable").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Identity.ReplacementExists").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Geometry.RequiredFieldsMissing").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Object.EditBatchRequired").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.PropertyLabel.ToolTipText").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Storage.MemoSidecarOpenFailed").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Object.GroupContainerNameMissing").starts_with("[!! ") &&
               pseudo_catalog.translate("VisualAssetEditor.Object.GroupContainerUnavailable").starts_with("[!! "),
           "#2373/#2374/#2375/#2376/#2377/#2378: visual asset editor prose should resolve through localizable catalog keys");
}

void test_visual_asset_editor_default_catalog_refreshes_when_locale_changes() {
    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");
    const auto english_result = copperfin::vfp::list_visual_objects({});
    locale_override.set("es-419");
    const auto spanish_result = copperfin::vfp::list_visual_objects({});
    locale_override.set("qps-ploc");
    const auto pseudo_result = copperfin::vfp::list_visual_objects({});

    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");
    constexpr std::string_view key = "VisualAssetEditor.Operation.AssetPathRequired";
    expect(!english_result.ok && english_result.error == english_catalog.translate(key),
           "#4366: visual-asset diagnostics should begin in en-US");
    expect(!spanish_result.ok && spanish_result.error == spanish_catalog.translate(key),
           "#4366: visual-asset diagnostics should refresh to es-419");
    expect(!pseudo_result.ok && pseudo_result.error == pseudo_catalog.translate(key),
           "#4366: visual-asset diagnostics should refresh to qps-ploc");
}

}  // namespace cf_test_visual_asset_editor
