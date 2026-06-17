#include "copperfin/studio/document_model.h"
#include "copperfin/studio/vs_launch_contract.h"
#include "copperfin/vfp/dbf_table.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << "\n";
        ++failures;
    }
}

template <typename Descriptor>
bool has_descriptor_id(const std::vector<Descriptor>& descriptors, std::string_view id) {
    for (const auto& descriptor : descriptors) {
        if (descriptor.id == id) {
            return true;
        }
    }
    return false;
}

std::vector<std::uint8_t> make_vfp_header() {
    std::vector<std::uint8_t> bytes(32U, 0U);
    bytes[0] = 0x30U;
    bytes[1] = 126U;
    bytes[2] = 4U;
    bytes[3] = 7U;
    bytes[8] = 0xA1U;
    bytes[9] = 0x00U;
    bytes[10] = 0x40U;
    bytes[11] = 0x00U;
    bytes[28] = 0x01U;
    bytes[29] = 0x03U;
    return bytes;
}

void write_synthetic_form_table_with_data_environment(const std::filesystem::path& path) {
    const std::vector<copperfin::vfp::DbfFieldDescriptor> fields{
        {.name = "OBJNAME", .type = 'C', .length = 24U},
        {.name = "BASECLASS", .type = 'C', .length = 24U},
        {.name = "CLASS", .type = 'C', .length = 24U}
    };
    const std::vector<std::vector<std::string>> records{
        {"Dataenvironment", "dataenvironment", ""},
        {"frmCustomer", "form", ""},
        {"pgfCustomer", "pageframe", "pageframe"}
    };

    const auto create_result = copperfin::vfp::create_dbf_table_file(path.string(), fields, records);
    expect(create_result.ok, "#966: synthetic SCX table with DataEnvironment record should be created");
}

void test_parse_launch_arguments() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--from-vs",
        "--read-only",
        "--json",
        "--set-property",
        "--record", "3",
        "--object-name", "cmdSave",
        "--unique-id", "button-guid",
        "--property-name", "Left",
        "--property-value", "25",
        "--line", "25",
        "--column", "7",
        "--symbol", "cmdSave.Click",
        "--selection-context", "visual_method",
        "--selection-context", "container_object",
        "--selection-context", "report_expression",
        "--selection-context", "label_expression",
        "--selection-context", "class_designer",
        "--selection-context", "menu_item",
        "--undo-mode", "command",
        "--undo-label", "Bulk Undo"
    });

    expect(result.ok, "launch contract should parse a complete Visual Studio launch request");
    expect(result.request.path == "E:\\Forms\\customer.scx", "launch path should be captured");
    expect(result.request.launched_from_visual_studio, "launch contract should detect --from-vs");
    expect(result.request.read_only, "launch contract should detect --read-only");
    expect(result.output_json, "launch contract should detect --json");
    expect(result.request.apply_property_update, "launch contract should detect --set-property");
    expect(!result.request.clear_property, "#1021: launch contract should keep clear-property off by default");
    expect(!result.request.rename_property, "#1022: launch contract should keep rename-property off by default");
    expect(!result.request.delete_object, "#1023: launch contract should keep delete-object off by default");
    expect(!result.request.restore_object, "#1024: launch contract should keep restore-object off by default");
    expect(!result.request.duplicate_object, "#1025: launch contract should keep duplicate-object off by default");
    expect(!result.request.rename_object, "#1026: launch contract should keep rename-object off by default");
    expect(!result.request.reparent_object, "#1027: launch contract should keep reparent-object off by default");
    expect(!result.request.reorder_object, "#1028: launch contract should keep reorder-object off by default");
    expect(!result.request.group_object, "#1030: launch contract should keep group-object off by default");
    expect(!result.request.align_object, "#1031: launch contract should keep align-object off by default");
    expect(!result.request.resize_object, "#1032: launch contract should keep resize-object off by default");
    expect(!result.request.distribute_object, "#1033: launch contract should keep distribute-object off by default");
    expect(!result.request.snap_object, "#1034: launch contract should keep snap-object off by default");
    expect(!result.request.nudge_object, "#1035: launch contract should keep nudge-object off by default");
    expect(!result.request.tab_order_object, "#1036: launch contract should keep tab-order-object off by default");
    expect(!result.request.tab_stop_object, "#1037: launch contract should keep tab-stop-object off by default");
    expect(!result.request.tab_stop_available, "#1037: launch contract should keep tab-stop unavailable by default");
    expect(!result.request.visibility_object, "#1038: launch contract should keep visibility-object off by default");
    expect(!result.request.visible_available, "#1038: launch contract should keep visible unavailable by default");
    expect(!result.request.enabled_object, "#1039: launch contract should keep enabled-object off by default");
    expect(!result.request.enabled_available, "#1039: launch contract should keep enabled unavailable by default");
    expect(!result.request.read_only_object, "#1040: launch contract should keep read-only-object off by default");
    expect(!result.request.object_read_only_available, "#1040: launch contract should keep object read-only unavailable by default");
    expect(!result.request.locked_object, "#1041: launch contract should keep locked-object off by default");
    expect(!result.request.locked_available, "#1041: launch contract should keep locked unavailable by default");
    expect(!result.request.caption_object, "#1042: launch contract should keep caption-object off by default");
    expect(!result.request.caption_available, "#1042: launch contract should keep caption unavailable by default");
    expect(!result.request.tooltip_text_object, "#1043: launch contract should keep tooltip-text-object off by default");
    expect(!result.request.tooltip_text_available, "#1043: launch contract should keep tooltip text unavailable by default");
    expect(!result.request.status_bar_text_object, "#1044: launch contract should keep status-bar-text-object off by default");
    expect(!result.request.status_bar_text_available, "#1044: launch contract should keep status-bar text unavailable by default");
    expect(!result.request.control_source_object, "#1045: launch contract should keep control-source-object off by default");
    expect(!result.request.control_source_available, "#1045: launch contract should keep control source unavailable by default");
    expect(!result.request.input_mask_object, "#1046: launch contract should keep input-mask-object off by default");
    expect(!result.request.input_mask_available, "#1046: launch contract should keep input mask unavailable by default");
    expect(!result.request.format_object, "#1047: launch contract should keep format-object off by default");
    expect(!result.request.format_available, "#1047: launch contract should keep format unavailable by default");
    expect(!result.request.row_source_object, "#1048: launch contract should keep row-source-object off by default");
    expect(!result.request.row_source_available, "#1048: launch contract should keep row source unavailable by default");
    expect(!result.request.row_source_type_object, "#1049: launch contract should keep row-source-type-object off by default");
    expect(!result.request.row_source_type_available, "#1049: launch contract should keep row source type unavailable by default");
    expect(!result.request.bound_column_object, "#1050: launch contract should keep bound-column-object off by default");
    expect(!result.request.bound_column_available, "#1050: launch contract should keep bound column unavailable by default");
    expect(!result.request.column_count_object, "#1051: launch contract should keep column-count-object off by default");
    expect(!result.request.column_count_available, "#1051: launch contract should keep column count unavailable by default");
    expect(!result.request.style_object, "#1052: launch contract should keep style-object off by default");
    expect(!result.request.style_available, "#1052: launch contract should keep style unavailable by default");
    expect(!result.request.list_index_object, "#1053: launch contract should keep list-index-object off by default");
    expect(!result.request.list_index_available, "#1053: launch contract should keep list index unavailable by default");
    expect(!result.request.left_column_object, "#1054: launch contract should keep left-column-object off by default");
    expect(!result.request.left_column_available, "#1054: launch contract should keep left column unavailable by default");
    expect(!result.request.display_value_object, "#1055: launch contract should keep display-value-object off by default");
    expect(!result.request.display_value_available, "#1055: launch contract should keep display value unavailable by default");
    expect(!result.request.selected_back_color_object,
        "#1056: launch contract should keep selected-back-color-object off by default");
    expect(!result.request.selected_back_color_available,
        "#1056: launch contract should keep selected back color unavailable by default");
    expect(!result.request.selected_fore_color_object,
        "#1057: launch contract should keep selected-fore-color-object off by default");
    expect(!result.request.selected_fore_color_available,
        "#1057: launch contract should keep selected fore color unavailable by default");
    expect(!result.request.selected_item_back_color_object,
        "#1058: launch contract should keep selected-item-back-color-object off by default");
    expect(!result.request.selected_item_back_color_available,
        "#1058: launch contract should keep selected item back color unavailable by default");
    expect(!result.request.selected_item_fore_color_object,
        "#1059: launch contract should keep selected-item-fore-color-object off by default");
    expect(!result.request.selected_item_fore_color_available,
        "#1059: launch contract should keep selected item fore color unavailable by default");
    expect(!result.request.disabled_item_back_color_object,
        "#1060: launch contract should keep disabled-item-back-color-object off by default");
    expect(!result.request.disabled_item_back_color_available,
        "#1060: launch contract should keep disabled item back color unavailable by default");
    expect(!result.request.disabled_item_fore_color_object,
        "#1061: launch contract should keep disabled-item-fore-color-object off by default");
    expect(!result.request.disabled_item_fore_color_available,
        "#1061: launch contract should keep disabled item fore color unavailable by default");
    expect(!result.request.item_back_color_object,
        "#1062: launch contract should keep item-back-color-object off by default");
    expect(!result.request.item_back_color_available,
        "#1062: launch contract should keep item back color unavailable by default");
    expect(!result.request.item_fore_color_object,
        "#1063: launch contract should keep item-fore-color-object off by default");
    expect(!result.request.item_fore_color_available,
        "#1063: launch contract should keep item fore color unavailable by default");
    expect(!result.request.highlight_back_color_object,
        "#1064: launch contract should keep highlight-back-color-object off by default");
    expect(!result.request.highlight_back_color_available,
        "#1064: launch contract should keep highlight back color unavailable by default");
    expect(!result.request.highlight_fore_color_object,
        "#1065: launch contract should keep highlight-fore-color-object off by default");
    expect(!result.request.highlight_fore_color_available,
        "#1065: launch contract should keep highlight fore color unavailable by default");
    expect(!result.request.back_color_object,
        "#1066: launch contract should keep back-color-object off by default");
    expect(!result.request.back_color_available,
        "#1066: launch contract should keep back color unavailable by default");
    expect(!result.request.ungroup_object, "#1029: launch contract should keep ungroup-object off by default");
    expect(result.request.record_index == 3U, "launch contract should parse the record index");
    expect(result.request.selection_record_available, "launch contract should mark explicit record selection");
    expect(result.request.object_name == "cmdSave", "#1020: launch contract should parse object-name selectors");
    expect(result.request.unique_id == "button-guid", "#1020: launch contract should parse unique-id selectors");
    expect(result.request.property_name == "Left", "launch contract should capture the property name");
    expect(result.request.property_value == "25", "launch contract should capture the property value");
    expect(result.request.line == 25U, "launch contract should parse the line value");
    expect(result.request.column == 7U, "launch contract should parse the column value");
    expect(result.request.symbol == "cmdSave.Click", "launch contract should parse the symbol");
    expect(result.request.designer_selection_contexts.size() == 6U,
           "#962: launch contract should collect explicit selection-context tokens");
    if (result.request.designer_selection_contexts.size() == 6U) {
        expect(result.request.designer_selection_contexts[0] == copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#962: launch contract should parse visual_method selection-context tokens");
        expect(result.request.designer_selection_contexts[1] == copperfin::studio::StudioEditorSelectionContext::container_object,
               "#1014: launch contract should parse container_object selection-context tokens");
        expect(result.request.designer_selection_contexts[2] == copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#962: launch contract should parse report_expression selection-context tokens");
        expect(result.request.designer_selection_contexts[3] == copperfin::studio::StudioEditorSelectionContext::label_expression,
               "#1011: launch contract should parse label_expression selection-context tokens");
        expect(result.request.designer_selection_contexts[4] == copperfin::studio::StudioEditorSelectionContext::class_designer,
               "#1012: launch contract should parse class_designer selection-context tokens");
        expect(result.request.designer_selection_contexts[5] == copperfin::studio::StudioEditorSelectionContext::menu_item,
               "#1013: launch contract should parse menu_item selection-context tokens");
    }
    expect(result.request.undo_mode == copperfin::studio::StudioUndoMode::command, "launch contract should parse the undo mode");
    expect(result.request.undo_label == "Bulk Undo", "launch contract should parse the undo label");
}

void test_parse_launch_arguments_for_clear_property() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--clear-property",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--property-name", "Caption"
    });

    expect(result.ok, "#1021: launch contract should parse clear-property requests");
    expect(result.request.clear_property, "#1021: launch contract should detect --clear-property");
    expect(!result.request.apply_property_update,
        "#1021: launch contract should not treat clear-property as set-property");
    expect(result.request.object_name == "txtName",
        "#1021: clear-property requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1021: clear-property requests should carry unique-id selectors");
    expect(result.request.property_name == "Caption",
        "#1021: clear-property requests should carry property names");
}

void test_parse_launch_arguments_rejects_ambiguous_property_command() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--set-property",
        "--clear-property",
        "--property-name", "Caption",
        "--property-value", "New"
    });

    expect(!result.ok,
        "#1021: launch contract should reject simultaneous set-property and clear-property requests");
}

void test_parse_launch_arguments_for_rename_property() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--rename-property",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--property-name", "ControlSource",
        "--new-property-name", "InputSource"
    });

    expect(result.ok, "#1022: launch contract should parse rename-property requests");
    expect(result.request.rename_property, "#1022: launch contract should detect --rename-property");
    expect(!result.request.apply_property_update,
        "#1022: launch contract should not treat rename-property as set-property");
    expect(!result.request.clear_property,
        "#1022: launch contract should not treat rename-property as clear-property");
    expect(result.request.object_name == "txtName",
        "#1022: rename-property requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1022: rename-property requests should carry unique-id selectors");
    expect(result.request.property_name == "ControlSource",
        "#1022: rename-property requests should carry source property names");
    expect(result.request.new_property_name == "InputSource",
        "#1022: rename-property requests should carry target property names");
}

void test_parse_launch_arguments_rejects_rename_property_missing_target() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-property",
        "--property-name", "ControlSource"
    });

    expect(!result.ok,
        "#1022: launch contract should reject rename-property requests without target property names");
}

void test_parse_launch_arguments_rejects_any_ambiguous_property_commands() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--set-property",
        "--rename-property",
        "--property-name", "Caption",
        "--property-value", "New",
        "--new-property-name", "Title"
    });

    expect(!result.ok,
        "#1022: launch contract should reject simultaneous set-property and rename-property requests");
}

void test_parse_launch_arguments_for_delete_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--delete-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid"
    });

    expect(result.ok, "#1023: launch contract should parse delete-object requests");
    expect(result.request.delete_object, "#1023: launch contract should detect --delete-object");
    expect(result.request.object_name == "txtName",
        "#1023: delete-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1023: delete-object requests should carry unique-id selectors");
}

void test_parse_launch_arguments_rejects_delete_object_property_ambiguity() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-object",
        "--clear-property",
        "--property-name", "Caption"
    });

    expect(!result.ok,
        "#1023: launch contract should reject delete-object combined with property commands");
}

void test_parse_launch_arguments_for_restore_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--restore-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid"
    });

    expect(result.ok, "#1024: launch contract should parse restore-object requests");
    expect(result.request.restore_object, "#1024: launch contract should detect --restore-object");
    expect(result.request.object_name == "txtName",
        "#1024: restore-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1024: restore-object requests should carry unique-id selectors");
}

void test_parse_launch_arguments_rejects_restore_object_ambiguity() {
    const auto delete_restore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--delete-object",
        "--restore-object",
        "--unique-id", "textbox-guid"
    });
    expect(!delete_restore_result.ok,
        "#1024: launch contract should reject simultaneous delete-object and restore-object requests");

    const auto restore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--restore-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!restore_property_result.ok,
        "#1024: launch contract should reject restore-object combined with property commands");
}

void test_parse_launch_arguments_for_duplicate_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--duplicate-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtNameCopy",
        "--new-name", "txtNameCopy",
        "--new-unique-id", "textbox-copy-guid"
    });

    expect(result.ok, "#1025: launch contract should parse duplicate-object requests");
    expect(result.request.duplicate_object, "#1025: launch contract should detect --duplicate-object");
    expect(result.request.object_name == "txtName",
        "#1025: duplicate-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1025: duplicate-object requests should carry unique-id selectors");
    expect(result.request.new_object_name == "txtNameCopy",
        "#1025: duplicate-object requests should carry replacement object names");
    expect(result.request.new_name == "txtNameCopy",
        "#1025: duplicate-object requests should carry replacement NAME values");
    expect(result.request.new_unique_id == "textbox-copy-guid",
        "#1025: duplicate-object requests should carry replacement unique ids");
}

void test_parse_launch_arguments_rejects_duplicate_object_ambiguity() {
    const auto duplicate_delete_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--duplicate-object",
        "--delete-object",
        "--unique-id", "textbox-guid"
    });
    expect(!duplicate_delete_result.ok,
        "#1025: launch contract should reject simultaneous duplicate-object and delete-object requests");

    const auto duplicate_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--duplicate-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!duplicate_property_result.ok,
        "#1025: launch contract should reject duplicate-object combined with property commands");
}

void test_parse_launch_arguments_for_rename_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--rename-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtCustomer",
        "--new-name", "txtCustomer",
        "--new-unique-id", "customer-textbox-guid"
    });

    expect(result.ok, "#1026: launch contract should parse rename-object requests");
    expect(result.request.rename_object, "#1026: launch contract should detect --rename-object");
    expect(result.request.object_name == "txtName",
        "#1026: rename-object requests should carry object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1026: rename-object requests should carry unique-id selectors");
    expect(result.request.new_object_name == "txtCustomer",
        "#1026: rename-object requests should carry replacement object names");
    expect(result.request.new_name == "txtCustomer",
        "#1026: rename-object requests should carry replacement NAME values");
    expect(result.request.new_unique_id == "customer-textbox-guid",
        "#1026: rename-object requests should carry replacement unique ids");
}

void test_parse_launch_arguments_rejects_rename_object_ambiguity_and_empty_identity() {
    const auto empty_identity_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--unique-id", "textbox-guid"
    });
    expect(!empty_identity_result.ok,
        "#1026: launch contract should reject rename-object requests without replacement identity fields");

    const auto rename_duplicate_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--duplicate-object",
        "--unique-id", "textbox-guid",
        "--new-object-name", "txtCustomer"
    });
    expect(!rename_duplicate_result.ok,
        "#1026: launch contract should reject simultaneous rename-object and duplicate-object requests");

    const auto rename_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--rename-object",
        "--clear-property",
        "--property-name", "Caption",
        "--new-object-name", "txtCustomer"
    });
    expect(!rename_property_result.ok,
        "#1026: launch contract should reject rename-object combined with property commands");
}

void test_parse_launch_arguments_for_reparent_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--reparent-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--parent-name", "cntPanel",
        "--parent-unique-id", "panel-guid"
    });

    expect(result.ok, "#1027: launch contract should parse reparent-object requests");
    expect(result.request.reparent_object, "#1027: launch contract should detect --reparent-object");
    expect(result.request.object_name == "txtName",
        "#1027: reparent-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1027: reparent-object requests should carry source unique-id selectors");
    expect(result.request.parent_name == "cntPanel",
        "#1027: reparent-object requests should carry parent object-name selectors");
    expect(result.request.parent_unique_id == "panel-guid",
        "#1027: reparent-object requests should carry parent unique-id selectors");
}

void test_parse_launch_arguments_rejects_reparent_object_ambiguity_and_missing_parent() {
    const auto missing_parent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--unique-id", "textbox-guid"
    });
    expect(!missing_parent_result.ok,
        "#1027: launch contract should reject reparent-object requests without parent selectors or clear-parent");

    const auto clear_parent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--unique-id", "textbox-guid",
        "--clear-parent"
    });
    expect(clear_parent_result.ok && clear_parent_result.request.clear_parent,
        "#1027: launch contract should parse clear-parent reparent requests");

    const auto reparent_rename_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--rename-object",
        "--unique-id", "textbox-guid",
        "--parent-name", "cntPanel",
        "--new-object-name", "txtCustomer"
    });
    expect(!reparent_rename_result.ok,
        "#1027: launch contract should reject simultaneous reparent-object and rename-object requests");

    const auto reparent_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reparent-object",
        "--clear-property",
        "--property-name", "Caption",
        "--parent-name", "cntPanel"
    });
    expect(!reparent_property_result.ok,
        "#1027: launch contract should reject reparent-object combined with property commands");
}

void test_parse_launch_arguments_for_reorder_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--reorder-object",
        "--object-name", "txtName",
        "--unique-id", "textbox-guid",
        "--placement", "before",
        "--target-object-name", "cmdSave",
        "--target-unique-id", "button-guid"
    });

    expect(result.ok, "#1028: launch contract should parse reorder-object requests");
    expect(result.request.reorder_object, "#1028: launch contract should detect --reorder-object");
    expect(result.request.object_name == "txtName",
        "#1028: reorder-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "textbox-guid",
        "#1028: reorder-object requests should carry source unique-id selectors");
    expect(result.request.placement == "before",
        "#1028: reorder-object requests should carry placement");
    expect(result.request.target_object_name == "cmdSave",
        "#1028: reorder-object requests should carry target object-name selectors");
    expect(result.request.target_unique_id == "button-guid",
        "#1028: reorder-object requests should carry target unique-id selectors");
}

void test_parse_launch_arguments_rejects_reorder_object_ambiguity_and_missing_placement() {
    const auto missing_placement_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--unique-id", "textbox-guid"
    });
    expect(!missing_placement_result.ok,
        "#1028: launch contract should reject reorder-object requests without placement");

    const auto reorder_reparent_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--reparent-object",
        "--unique-id", "textbox-guid",
        "--placement", "front",
        "--parent-name", "cntPanel"
    });
    expect(!reorder_reparent_result.ok,
        "#1028: launch contract should reject simultaneous reorder-object and reparent-object requests");

    const auto reorder_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--reorder-object",
        "--clear-property",
        "--property-name", "Caption",
        "--placement", "front"
    });
    expect(!reorder_property_result.ok,
        "#1028: launch contract should reject reorder-object combined with property commands");
}

void test_parse_launch_arguments_for_ungroup_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--ungroup-object",
        "--object-name", "cntGroup",
        "--unique-id", "group-guid"
    });

    expect(result.ok, "#1029: launch contract should parse ungroup-object requests");
    expect(result.request.ungroup_object, "#1029: launch contract should detect --ungroup-object");
    expect(result.request.object_name == "cntGroup",
        "#1029: ungroup-object requests should carry source object-name selectors");
    expect(result.request.unique_id == "group-guid",
        "#1029: ungroup-object requests should carry source unique-id selectors");
}

void test_parse_launch_arguments_rejects_ungroup_object_ambiguity() {
    const auto ungroup_reorder_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ungroup-object",
        "--reorder-object",
        "--unique-id", "group-guid",
        "--placement", "front"
    });
    expect(!ungroup_reorder_result.ok,
        "#1029: launch contract should reject simultaneous ungroup-object and reorder-object requests");

    const auto ungroup_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--ungroup-object",
        "--clear-property",
        "--property-name", "Caption"
    });
    expect(!ungroup_property_result.ok,
        "#1029: launch contract should reject ungroup-object combined with property commands");
}

void test_parse_launch_arguments_for_group_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--group-object",
        "--field-value", "OBJNAME=cntGroup",
        "--field-value", "UNIQUEID=group-guid",
        "--field-value", "PARENT=frmCustomer",
        "--group-child-object-name", "cmdSave",
        "--group-child-unique-id", "name-guid"
    });

    expect(result.ok, "#1030: launch contract should parse group-object requests");
    expect(result.request.group_object, "#1030: launch contract should detect --group-object");
    expect(result.request.field_values.size() == 3U,
        "#1030: group-object requests should collect container field values");
    if (result.request.field_values.size() == 3U) {
        expect(result.request.field_values[0].property_name == "OBJNAME" &&
                result.request.field_values[0].property_value == "cntGroup",
            "#1030: group-object requests should parse first container field assignment");
        expect(result.request.field_values[1].property_name == "UNIQUEID" &&
                result.request.field_values[1].property_value == "group-guid",
            "#1030: group-object requests should parse second container field assignment");
    }
    expect(result.request.group_objects.size() == 2U,
        "#1030: group-object requests should collect grouped child selectors");
    if (result.request.group_objects.size() == 2U) {
        expect(result.request.group_objects[0].object_name == "cmdSave" &&
                result.request.group_objects[0].unique_id.empty(),
            "#1030: group-object requests should parse child object-name selectors");
        expect(result.request.group_objects[1].object_name.empty() &&
                result.request.group_objects[1].unique_id == "name-guid",
            "#1030: group-object requests should parse child unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_group_object_invalid_inputs() {
    const auto missing_field_values_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--group-child-unique-id", "name-guid"
    });
    expect(!missing_field_values_result.ok,
        "#1030: launch contract should reject group-object requests without container field values");

    const auto missing_children_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--field-value", "OBJNAME=cntGroup"
    });
    expect(!missing_children_result.ok,
        "#1030: launch contract should reject group-object requests without child selectors");

    const auto invalid_assignment_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--field-value", "OBJNAME",
        "--group-child-unique-id", "name-guid"
    });
    expect(!invalid_assignment_result.ok,
        "#1030: launch contract should reject group-object field values without assignment syntax");
}

void test_parse_launch_arguments_rejects_group_object_ambiguity() {
    const auto group_ungroup_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--ungroup-object",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!group_ungroup_result.ok,
        "#1030: launch contract should reject simultaneous group-object and ungroup-object requests");

    const auto group_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--group-object",
        "--clear-property",
        "--property-name", "Caption",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!group_property_result.ok,
        "#1030: launch contract should reject group-object combined with property commands");

    const auto stray_field_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--field-value", "OBJNAME=cntGroup"
    });
    expect(!stray_field_value_result.ok,
        "#1030: launch contract should reject stray group field values");
}

void test_parse_launch_arguments_for_align_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-object-name", "txtName",
        "--align-target-unique-id", "status-guid"
    });

    expect(result.ok, "#1031: launch contract should parse align-object requests");
    expect(result.request.align_object, "#1031: launch contract should detect --align-object");
    expect(result.request.alignment_mode == "left",
        "#1031: align-object requests should carry alignment mode");
    expect(result.request.anchor_unique_id == "anchor-guid",
        "#1031: align-object requests should carry anchor unique-id selectors");
    expect(result.request.align_objects.size() == 2U,
        "#1031: align-object requests should collect alignment target selectors");
    if (result.request.align_objects.size() == 2U) {
        expect(result.request.align_objects[0].object_name == "txtName" &&
                result.request.align_objects[0].unique_id.empty(),
            "#1031: align-object requests should parse target object-name selectors");
        expect(result.request.align_objects[1].object_name.empty() &&
                result.request.align_objects[1].unique_id == "status-guid",
            "#1031: align-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_align_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!missing_mode_result.ok,
        "#1031: launch contract should reject align-object requests without alignment mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--align-target-unique-id", "name-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1031: launch contract should reject align-object requests without anchor selectors");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--alignment-mode", "left",
        "--anchor-object-name", "cmdAnchor"
    });
    expect(!missing_targets_result.ok,
        "#1031: launch contract should reject align-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_align_object_ambiguity() {
    const auto align_group_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--group-object",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid",
        "--field-value", "OBJNAME=cntGroup",
        "--group-child-unique-id", "name-guid"
    });
    expect(!align_group_result.ok,
        "#1031: launch contract should reject simultaneous align-object and group-object requests");

    const auto align_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--align-object",
        "--clear-property",
        "--property-name", "Caption",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!align_property_result.ok,
        "#1031: launch contract should reject align-object combined with property commands");

    const auto stray_alignment_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--alignment-mode", "left"
    });
    expect(!stray_alignment_result.ok,
        "#1031: launch contract should reject stray alignment arguments");
}

void test_parse_launch_arguments_for_resize_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-object-name", "txtName",
        "--resize-target-unique-id", "status-guid"
    });

    expect(result.ok, "#1032: launch contract should parse resize-object requests");
    expect(result.request.resize_object, "#1032: launch contract should detect --resize-object");
    expect(result.request.resize_mode == "width",
        "#1032: resize-object requests should carry resize mode");
    expect(result.request.anchor_unique_id == "anchor-guid",
        "#1032: resize-object requests should carry anchor unique-id selectors");
    expect(result.request.resize_objects.size() == 2U,
        "#1032: resize-object requests should collect resize target selectors");
    if (result.request.resize_objects.size() == 2U) {
        expect(result.request.resize_objects[0].object_name == "txtName" &&
                result.request.resize_objects[0].unique_id.empty(),
            "#1032: resize-object requests should parse target object-name selectors");
        expect(result.request.resize_objects[1].object_name.empty() &&
                result.request.resize_objects[1].unique_id == "status-guid",
            "#1032: resize-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_resize_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!missing_mode_result.ok,
        "#1032: launch contract should reject resize-object requests without resize mode");

    const auto missing_anchor_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!missing_anchor_result.ok,
        "#1032: launch contract should reject resize-object requests without anchor selectors");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--resize-mode", "width",
        "--anchor-object-name", "cmdAnchor"
    });
    expect(!missing_targets_result.ok,
        "#1032: launch contract should reject resize-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_resize_object_ambiguity() {
    const auto resize_align_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--align-object",
        "--resize-mode", "width",
        "--alignment-mode", "left",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid",
        "--align-target-unique-id", "name-guid"
    });
    expect(!resize_align_result.ok,
        "#1032: launch contract should reject simultaneous resize-object and align-object requests");

    const auto resize_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-object",
        "--clear-property",
        "--property-name", "Caption",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--resize-target-unique-id", "name-guid"
    });
    expect(!resize_property_result.ok,
        "#1032: launch contract should reject resize-object combined with property commands");

    const auto stray_resize_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--resize-mode", "width"
    });
    expect(!stray_resize_result.ok,
        "#1032: launch contract should reject stray resize arguments");
}

void test_parse_launch_arguments_for_distribute_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--distribute-object",
        "--distribution-mode", "horizontal",
        "--distribute-target-object-name", "cmdLeft",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-object-name", "cmdRight"
    });

    expect(result.ok, "#1033: launch contract should parse distribute-object requests");
    expect(result.request.distribute_object, "#1033: launch contract should detect --distribute-object");
    expect(result.request.distribution_mode == "horizontal",
        "#1033: distribute-object requests should carry distribution mode");
    expect(result.request.distribute_objects.size() == 3U,
        "#1033: distribute-object requests should collect distribution target selectors");
    if (result.request.distribute_objects.size() == 3U) {
        expect(result.request.distribute_objects[0].object_name == "cmdLeft" &&
                result.request.distribute_objects[0].unique_id.empty(),
            "#1033: distribute-object requests should parse target object-name selectors");
        expect(result.request.distribute_objects[1].object_name.empty() &&
                result.request.distribute_objects[1].unique_id == "middle-guid",
            "#1033: distribute-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_distribute_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid"
    });
    expect(!missing_mode_result.ok,
        "#1033: launch contract should reject distribute-object requests without distribution mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--distribution-mode", "horizontal"
    });
    expect(!missing_targets_result.ok,
        "#1033: launch contract should reject distribute-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_distribute_object_ambiguity() {
    const auto distribute_resize_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--resize-object",
        "--distribution-mode", "horizontal",
        "--resize-mode", "width",
        "--anchor-unique-id", "anchor-guid",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid",
        "--resize-target-unique-id", "middle-guid"
    });
    expect(!distribute_resize_result.ok,
        "#1033: launch contract should reject simultaneous distribute-object and resize-object requests");

    const auto distribute_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribute-object",
        "--clear-property",
        "--property-name", "Caption",
        "--distribution-mode", "horizontal",
        "--distribute-target-unique-id", "left-guid",
        "--distribute-target-unique-id", "middle-guid",
        "--distribute-target-unique-id", "right-guid"
    });
    expect(!distribute_property_result.ok,
        "#1033: launch contract should reject distribute-object combined with property commands");

    const auto stray_distribution_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--distribution-mode", "horizontal"
    });
    expect(!stray_distribution_result.ok,
        "#1033: launch contract should reject stray distribution arguments");
}

void test_parse_launch_arguments_for_snap_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--snap-object",
        "--snap-mode", "both",
        "--grid-width", "10.5",
        "--grid-height", "25",
        "--snap-target-object-name", "cmdOne",
        "--snap-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1034: launch contract should parse snap-object requests");
    expect(result.request.snap_object, "#1034: launch contract should detect --snap-object");
    expect(result.request.snap_mode == "both",
        "#1034: snap-object requests should carry snap mode");
    expect(result.request.grid_width == 10.5 && result.request.grid_height == 25.0,
        "#1034: snap-object requests should carry numeric grid dimensions");
    expect(result.request.snap_objects.size() == 2U,
        "#1034: snap-object requests should collect snap target selectors");
    if (result.request.snap_objects.size() == 2U) {
        expect(result.request.snap_objects[0].object_name == "cmdOne" &&
                result.request.snap_objects[0].unique_id.empty(),
            "#1034: snap-object requests should parse target object-name selectors");
        expect(result.request.snap_objects[1].object_name.empty() &&
                result.request.snap_objects[1].unique_id == "two-guid",
            "#1034: snap-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_snap_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1034: launch contract should reject snap-object requests without snap mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "horizontal",
        "--grid-width", "10"
    });
    expect(!missing_targets_result.ok,
        "#1034: launch contract should reject snap-object requests without target selectors");

    const auto invalid_grid_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--snap-mode", "horizontal",
        "--grid-width", "wide",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!invalid_grid_result.ok,
        "#1034: launch contract should reject non-numeric grid widths");
}

void test_parse_launch_arguments_rejects_snap_object_ambiguity() {
    const auto snap_distribute_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--distribute-object",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--distribution-mode", "horizontal",
        "--snap-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "one-guid",
        "--distribute-target-unique-id", "two-guid",
        "--distribute-target-unique-id", "three-guid"
    });
    expect(!snap_distribute_result.ok,
        "#1034: launch contract should reject simultaneous snap-object and distribute-object requests");

    const auto snap_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-object",
        "--clear-property",
        "--property-name", "Caption",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!snap_property_result.ok,
        "#1034: launch contract should reject snap-object combined with property commands");

    const auto stray_snap_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--snap-mode", "horizontal"
    });
    expect(!stray_snap_result.ok,
        "#1034: launch contract should reject stray snap arguments");
}

void test_parse_launch_arguments_for_nudge_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--nudge-object",
        "--nudge-mode", "both",
        "--delta-hpos", "5.5",
        "--delta-vpos", "-2",
        "--nudge-target-object-name", "cmdOne",
        "--nudge-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1035: launch contract should parse nudge-object requests");
    expect(result.request.nudge_object, "#1035: launch contract should detect --nudge-object");
    expect(result.request.nudge_mode == "both",
        "#1035: nudge-object requests should carry nudge mode");
    expect(result.request.delta_hpos == 5.5 && result.request.delta_vpos == -2.0,
        "#1035: nudge-object requests should carry numeric deltas");
    expect(result.request.nudge_objects.size() == 2U,
        "#1035: nudge-object requests should collect nudge target selectors");
    if (result.request.nudge_objects.size() == 2U) {
        expect(result.request.nudge_objects[0].object_name == "cmdOne" &&
                result.request.nudge_objects[0].unique_id.empty(),
            "#1035: nudge-object requests should parse target object-name selectors");
        expect(result.request.nudge_objects[1].object_name.empty() &&
                result.request.nudge_objects[1].unique_id == "two-guid",
            "#1035: nudge-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_nudge_object_invalid_inputs() {
    const auto missing_mode_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!missing_mode_result.ok,
        "#1035: launch contract should reject nudge-object requests without nudge mode");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1"
    });
    expect(!missing_targets_result.ok,
        "#1035: launch contract should reject nudge-object requests without target selectors");

    const auto invalid_delta_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "right",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!invalid_delta_result.ok,
        "#1035: launch contract should reject non-numeric horizontal deltas");
}

void test_parse_launch_arguments_rejects_nudge_object_ambiguity() {
    const auto nudge_snap_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--snap-object",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--snap-mode", "horizontal",
        "--grid-width", "10",
        "--nudge-target-unique-id", "one-guid",
        "--snap-target-unique-id", "one-guid"
    });
    expect(!nudge_snap_result.ok,
        "#1035: launch contract should reject simultaneous nudge-object and snap-object requests");

    const auto nudge_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-object",
        "--clear-property",
        "--property-name", "Caption",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!nudge_property_result.ok,
        "#1035: launch contract should reject nudge-object combined with property commands");

    const auto stray_nudge_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--nudge-mode", "horizontal"
    });
    expect(!stray_nudge_result.ok,
        "#1035: launch contract should reject stray nudge arguments");
}

void test_parse_launch_arguments_for_tab_order_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tab-order-object",
        "--starting-tab-index", "5",
        "--tab-order-target-object-name", "cmdOne",
        "--tab-order-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1036: launch contract should parse tab-order-object requests");
    expect(result.request.tab_order_object, "#1036: launch contract should detect --tab-order-object");
    expect(result.request.starting_tab_index == 5 && result.request.starting_tab_index_available,
        "#1036: tab-order-object requests should carry starting tab index");
    expect(result.request.tab_order_objects.size() == 2U,
        "#1036: tab-order-object requests should collect tab-order target selectors");
    if (result.request.tab_order_objects.size() == 2U) {
        expect(result.request.tab_order_objects[0].object_name == "cmdOne" &&
                result.request.tab_order_objects[0].unique_id.empty(),
            "#1036: tab-order-object requests should parse target object-name selectors");
        expect(result.request.tab_order_objects[1].object_name.empty() &&
                result.request.tab_order_objects[1].unique_id == "two-guid",
            "#1036: tab-order-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tab_order_object_invalid_inputs() {
    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "0"
    });
    expect(!missing_targets_result.ok,
        "#1036: launch contract should reject tab-order-object requests without target selectors");

    const auto negative_start_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "-1",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!negative_start_result.ok,
        "#1036: launch contract should reject negative tab-order starting indexes");

    const auto invalid_start_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--starting-tab-index", "first",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!invalid_start_result.ok,
        "#1036: launch contract should reject non-integer tab-order starting indexes");
}

void test_parse_launch_arguments_rejects_tab_order_object_ambiguity() {
    const auto tab_order_nudge_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--nudge-object",
        "--starting-tab-index", "1",
        "--tab-order-target-unique-id", "one-guid",
        "--nudge-mode", "horizontal",
        "--delta-hpos", "1",
        "--nudge-target-unique-id", "one-guid"
    });
    expect(!tab_order_nudge_result.ok,
        "#1036: launch contract should reject simultaneous tab-order-object and nudge-object requests");

    const auto tab_order_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-order-object",
        "--clear-property",
        "--property-name", "Caption",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!tab_order_property_result.ok,
        "#1036: launch contract should reject tab-order-object combined with property commands");

    const auto stray_tab_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--starting-tab-index", "0"
    });
    expect(!stray_tab_order_result.ok,
        "#1036: launch contract should reject stray tab-order arguments");
}

void test_parse_launch_arguments_for_tab_stop_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tab-stop-object",
        "--tab-stop", "false",
        "--tab-stop-target-object-name", "cmdOne",
        "--tab-stop-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1037: launch contract should parse tab-stop-object requests");
    expect(result.request.tab_stop_object, "#1037: launch contract should detect --tab-stop-object");
    expect(result.request.tab_stop_available && !result.request.tab_stop,
        "#1037: tab-stop-object requests should carry false tab-stop state");
    expect(result.request.tab_stop_objects.size() == 2U,
        "#1037: tab-stop-object requests should collect tab-stop target selectors");
    if (result.request.tab_stop_objects.size() == 2U) {
        expect(result.request.tab_stop_objects[0].object_name == "cmdOne" &&
                result.request.tab_stop_objects[0].unique_id.empty(),
            "#1037: tab-stop-object requests should parse target object-name selectors");
        expect(result.request.tab_stop_objects[1].object_name.empty() &&
                result.request.tab_stop_objects[1].unique_id == "two-guid",
            "#1037: tab-stop-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tab_stop_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1037: launch contract should reject tab-stop-object requests without tab-stop state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop", "true"
    });
    expect(!missing_targets_result.ok,
        "#1037: launch contract should reject tab-stop-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-stop", "maybe",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1037: launch contract should reject unsupported tab-stop values");
}

void test_parse_launch_arguments_rejects_tab_stop_object_ambiguity() {
    const auto tab_stop_tab_order_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--tab-order-object",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid",
        "--starting-tab-index", "1",
        "--tab-order-target-unique-id", "one-guid"
    });
    expect(!tab_stop_tab_order_result.ok,
        "#1037: launch contract should reject simultaneous tab-stop-object and tab-order-object requests");

    const auto tab_stop_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop-object",
        "--clear-property",
        "--property-name", "Caption",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!tab_stop_property_result.ok,
        "#1037: launch contract should reject tab-stop-object combined with property commands");

    const auto stray_tab_stop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tab-stop", "true"
    });
    expect(!stray_tab_stop_result.ok,
        "#1037: launch contract should reject stray tab-stop arguments");
}

void test_parse_launch_arguments_for_visibility_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--visibility-object",
        "--visible", "false",
        "--visibility-target-object-name", "cmdOne",
        "--visibility-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1038: launch contract should parse visibility-object requests");
    expect(result.request.visibility_object, "#1038: launch contract should detect --visibility-object");
    expect(result.request.visible_available && !result.request.visible,
        "#1038: visibility-object requests should carry false visible state");
    expect(result.request.visibility_objects.size() == 2U,
        "#1038: visibility-object requests should collect visibility target selectors");
    if (result.request.visibility_objects.size() == 2U) {
        expect(result.request.visibility_objects[0].object_name == "cmdOne" &&
                result.request.visibility_objects[0].unique_id.empty(),
            "#1038: visibility-object requests should parse target object-name selectors");
        expect(result.request.visibility_objects[1].object_name.empty() &&
                result.request.visibility_objects[1].unique_id == "two-guid",
            "#1038: visibility-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_visibility_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1038: launch contract should reject visibility-object requests without visible state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visible", "true"
    });
    expect(!missing_targets_result.ok,
        "#1038: launch contract should reject visibility-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--visible", "maybe",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1038: launch contract should reject unsupported visible values");
}

void test_parse_launch_arguments_rejects_visibility_object_ambiguity() {
    const auto visibility_tab_stop_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--tab-stop-object",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid",
        "--tab-stop", "true",
        "--tab-stop-target-unique-id", "one-guid"
    });
    expect(!visibility_tab_stop_result.ok,
        "#1038: launch contract should reject simultaneous visibility-object and tab-stop-object requests");

    const auto visibility_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visibility-object",
        "--clear-property",
        "--property-name", "Caption",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!visibility_property_result.ok,
        "#1038: launch contract should reject visibility-object combined with property commands");

    const auto stray_visible_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--visible", "true"
    });
    expect(!stray_visible_result.ok,
        "#1038: launch contract should reject stray visible arguments");
}

void test_parse_launch_arguments_for_enabled_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--enabled-object",
        "--enabled", "false",
        "--enabled-target-object-name", "cmdOne",
        "--enabled-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1039: launch contract should parse enabled-object requests");
    expect(result.request.enabled_object, "#1039: launch contract should detect --enabled-object");
    expect(result.request.enabled_available && !result.request.enabled,
        "#1039: enabled-object requests should carry false enabled state");
    expect(result.request.enabled_objects.size() == 2U,
        "#1039: enabled-object requests should collect enabled target selectors");
    if (result.request.enabled_objects.size() == 2U) {
        expect(result.request.enabled_objects[0].object_name == "cmdOne" &&
                result.request.enabled_objects[0].unique_id.empty(),
            "#1039: enabled-object requests should parse target object-name selectors");
        expect(result.request.enabled_objects[1].object_name.empty() &&
                result.request.enabled_objects[1].unique_id == "two-guid",
            "#1039: enabled-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_enabled_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1039: launch contract should reject enabled-object requests without enabled state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled", "true"
    });
    expect(!missing_targets_result.ok,
        "#1039: launch contract should reject enabled-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--enabled", "maybe",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1039: launch contract should reject unsupported enabled values");
}

void test_parse_launch_arguments_rejects_enabled_object_ambiguity() {
    const auto enabled_visibility_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--visibility-object",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid",
        "--visible", "true",
        "--visibility-target-unique-id", "one-guid"
    });
    expect(!enabled_visibility_result.ok,
        "#1039: launch contract should reject simultaneous enabled-object and visibility-object requests");

    const auto enabled_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled-object",
        "--clear-property",
        "--property-name", "Caption",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!enabled_property_result.ok,
        "#1039: launch contract should reject enabled-object combined with property commands");

    const auto stray_enabled_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--enabled", "true"
    });
    expect(!stray_enabled_result.ok,
        "#1039: launch contract should reject stray enabled arguments");
}

void test_parse_launch_arguments_for_read_only_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--read-only-object",
        "--object-read-only", "true",
        "--read-only-target-object-name", "txtOne",
        "--read-only-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1040: launch contract should parse read-only-object requests");
    expect(result.request.read_only_object, "#1040: launch contract should detect --read-only-object");
    expect(result.request.object_read_only_available && result.request.object_read_only,
        "#1040: read-only-object requests should carry true read-only state");
    expect(result.request.read_only_objects.size() == 2U,
        "#1040: read-only-object requests should collect read-only target selectors");
    if (result.request.read_only_objects.size() == 2U) {
        expect(result.request.read_only_objects[0].object_name == "txtOne" &&
                result.request.read_only_objects[0].unique_id.empty(),
            "#1040: read-only-object requests should parse target object-name selectors");
        expect(result.request.read_only_objects[1].object_name.empty() &&
                result.request.read_only_objects[1].unique_id == "two-guid",
            "#1040: read-only-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_read_only_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1040: launch contract should reject read-only-object requests without object read-only state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--object-read-only", "true"
    });
    expect(!missing_targets_result.ok,
        "#1040: launch contract should reject read-only-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--object-read-only", "maybe",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1040: launch contract should reject unsupported object read-only values");
}

void test_parse_launch_arguments_rejects_read_only_object_ambiguity() {
    const auto read_only_enabled_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--enabled-object",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid",
        "--enabled", "true",
        "--enabled-target-unique-id", "one-guid"
    });
    expect(!read_only_enabled_result.ok,
        "#1040: launch contract should reject simultaneous read-only-object and enabled-object requests");

    const auto read_only_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--read-only-object",
        "--clear-property",
        "--property-name", "Caption",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!read_only_property_result.ok,
        "#1040: launch contract should reject read-only-object combined with property commands");

    const auto stray_read_only_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--object-read-only", "true"
    });
    expect(!stray_read_only_result.ok,
        "#1040: launch contract should reject stray object read-only arguments");
}

void test_parse_launch_arguments_for_locked_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--locked-object",
        "--locked", "true",
        "--locked-target-object-name", "txtOne",
        "--locked-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1041: launch contract should parse locked-object requests");
    expect(result.request.locked_object, "#1041: launch contract should detect --locked-object");
    expect(result.request.locked_available && result.request.locked,
        "#1041: locked-object requests should carry true locked state");
    expect(result.request.locked_objects.size() == 2U,
        "#1041: locked-object requests should collect locked target selectors");
    if (result.request.locked_objects.size() == 2U) {
        expect(result.request.locked_objects[0].object_name == "txtOne" &&
                result.request.locked_objects[0].unique_id.empty(),
            "#1041: locked-object requests should parse target object-name selectors");
        expect(result.request.locked_objects[1].object_name.empty() &&
                result.request.locked_objects[1].unique_id == "two-guid",
            "#1041: locked-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_locked_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1041: launch contract should reject locked-object requests without locked state");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked", "true"
    });
    expect(!missing_targets_result.ok,
        "#1041: launch contract should reject locked-object requests without target selectors");

    const auto invalid_bool_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--locked", "maybe",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!invalid_bool_result.ok,
        "#1041: launch contract should reject unsupported locked values");
}

void test_parse_launch_arguments_rejects_locked_object_ambiguity() {
    const auto locked_read_only_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--read-only-object",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid",
        "--object-read-only", "true",
        "--read-only-target-unique-id", "one-guid"
    });
    expect(!locked_read_only_result.ok,
        "#1041: launch contract should reject simultaneous locked-object and read-only-object requests");

    const auto locked_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked-object",
        "--clear-property",
        "--property-name", "Caption",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!locked_property_result.ok,
        "#1041: launch contract should reject locked-object combined with property commands");

    const auto stray_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--locked", "true"
    });
    expect(!stray_locked_result.ok,
        "#1041: launch contract should reject stray locked arguments");
}

void test_parse_launch_arguments_for_caption_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--caption-object",
        "--caption", "Save Customer",
        "--caption-target-object-name", "cmdSave",
        "--caption-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1042: launch contract should parse caption-object requests");
    expect(result.request.caption_object, "#1042: launch contract should detect --caption-object");
    expect(result.request.caption_available && result.request.caption == "Save Customer",
        "#1042: caption-object requests should carry caption text");
    expect(result.request.caption_objects.size() == 2U,
        "#1042: caption-object requests should collect caption target selectors");
    if (result.request.caption_objects.size() == 2U) {
        expect(result.request.caption_objects[0].object_name == "cmdSave" &&
                result.request.caption_objects[0].unique_id.empty(),
            "#1042: caption-object requests should parse target object-name selectors");
        expect(result.request.caption_objects[1].object_name.empty() &&
                result.request.caption_objects[1].unique_id == "two-guid",
            "#1042: caption-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_caption_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1042: launch contract should reject caption-object requests without caption text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--caption", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1042: launch contract should reject caption-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_caption_object_ambiguity() {
    const auto caption_locked_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--locked-object",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid",
        "--locked", "true",
        "--locked-target-unique-id", "one-guid"
    });
    expect(!caption_locked_result.ok,
        "#1042: launch contract should reject simultaneous caption-object and locked-object requests");

    const auto caption_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption-object",
        "--clear-property",
        "--property-name", "Caption",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!caption_property_result.ok,
        "#1042: launch contract should reject caption-object combined with property commands");

    const auto stray_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--caption", "Save"
    });
    expect(!stray_caption_result.ok,
        "#1042: launch contract should reject stray caption arguments");
}

void test_parse_launch_arguments_for_tooltip_text_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--tooltip-text-object",
        "--tooltip-text", "Save this customer",
        "--tooltip-text-target-object-name", "cmdSave",
        "--tooltip-text-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1043: launch contract should parse tooltip-text-object requests");
    expect(result.request.tooltip_text_object, "#1043: launch contract should detect --tooltip-text-object");
    expect(result.request.tooltip_text_available && result.request.tooltip_text == "Save this customer",
        "#1043: tooltip-text-object requests should carry tooltip text");
    expect(result.request.tooltip_text_objects.size() == 2U,
        "#1043: tooltip-text-object requests should collect tooltip text target selectors");
    if (result.request.tooltip_text_objects.size() == 2U) {
        expect(result.request.tooltip_text_objects[0].object_name == "cmdSave" &&
                result.request.tooltip_text_objects[0].unique_id.empty(),
            "#1043: tooltip-text-object requests should parse target object-name selectors");
        expect(result.request.tooltip_text_objects[1].object_name.empty() &&
                result.request.tooltip_text_objects[1].unique_id == "two-guid",
            "#1043: tooltip-text-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_tooltip_text_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1043: launch contract should reject tooltip-text-object requests without tooltip text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--tooltip-text", "Save"
    });
    expect(!missing_targets_result.ok,
        "#1043: launch contract should reject tooltip-text-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_tooltip_text_object_ambiguity() {
    const auto tooltip_caption_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--caption-object",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid",
        "--caption", "Save",
        "--caption-target-unique-id", "one-guid"
    });
    expect(!tooltip_caption_result.ok,
        "#1043: launch contract should reject simultaneous tooltip-text-object and caption-object requests");

    const auto tooltip_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text-object",
        "--clear-property",
        "--property-name", "ToolTipText",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!tooltip_property_result.ok,
        "#1043: launch contract should reject tooltip-text-object combined with property commands");

    const auto stray_tooltip_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--tooltip-text", "Save"
    });
    expect(!stray_tooltip_result.ok,
        "#1043: launch contract should reject stray tooltip text arguments");
}

void test_parse_launch_arguments_for_status_bar_text_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--status-bar-text-object",
        "--status-bar-text", "Ready to save",
        "--status-bar-text-target-object-name", "cmdSave",
        "--status-bar-text-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1044: launch contract should parse status-bar-text-object requests");
    expect(result.request.status_bar_text_object, "#1044: launch contract should detect --status-bar-text-object");
    expect(result.request.status_bar_text_available && result.request.status_bar_text == "Ready to save",
        "#1044: status-bar-text-object requests should carry status-bar text");
    expect(result.request.status_bar_text_objects.size() == 2U,
        "#1044: status-bar-text-object requests should collect status-bar text target selectors");
    if (result.request.status_bar_text_objects.size() == 2U) {
        expect(result.request.status_bar_text_objects[0].object_name == "cmdSave" &&
                result.request.status_bar_text_objects[0].unique_id.empty(),
            "#1044: status-bar-text-object requests should parse target object-name selectors");
        expect(result.request.status_bar_text_objects[1].object_name.empty() &&
                result.request.status_bar_text_objects[1].unique_id == "two-guid",
            "#1044: status-bar-text-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_status_bar_text_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without status-bar text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--status-bar-text", "Ready"
    });
    expect(!missing_targets_result.ok,
        "#1044: launch contract should reject status-bar-text-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_status_bar_text_object_ambiguity() {
    const auto status_tooltip_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--tooltip-text-object",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid",
        "--tooltip-text", "Save",
        "--tooltip-text-target-unique-id", "one-guid"
    });
    expect(!status_tooltip_result.ok,
        "#1044: launch contract should reject simultaneous status-bar-text-object and tooltip-text-object requests");

    const auto status_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text-object",
        "--clear-property",
        "--property-name", "StatusBarText",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!status_property_result.ok,
        "#1044: launch contract should reject status-bar-text-object combined with property commands");

    const auto stray_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--status-bar-text", "Ready"
    });
    expect(!stray_status_result.ok,
        "#1044: launch contract should reject stray status-bar text arguments");
}

void test_parse_launch_arguments_for_control_source_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--control-source-object",
        "--control-source", "customers.name",
        "--control-source-target-object-name", "txtName",
        "--control-source-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1045: launch contract should parse control-source-object requests");
    expect(result.request.control_source_object, "#1045: launch contract should detect --control-source-object");
    expect(result.request.control_source_available && result.request.control_source == "customers.name",
        "#1045: control-source-object requests should carry control source text");
    expect(result.request.control_source_objects.size() == 2U,
        "#1045: control-source-object requests should collect control source target selectors");
    if (result.request.control_source_objects.size() == 2U) {
        expect(result.request.control_source_objects[0].object_name == "txtName" &&
                result.request.control_source_objects[0].unique_id.empty(),
            "#1045: control-source-object requests should parse target object-name selectors");
        expect(result.request.control_source_objects[1].object_name.empty() &&
                result.request.control_source_objects[1].unique_id == "two-guid",
            "#1045: control-source-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_control_source_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1045: launch contract should reject control-source-object requests without control source text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--control-source", "customers.name"
    });
    expect(!missing_targets_result.ok,
        "#1045: launch contract should reject control-source-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_control_source_object_ambiguity() {
    const auto control_status_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--status-bar-text-object",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid",
        "--status-bar-text", "Ready",
        "--status-bar-text-target-unique-id", "one-guid"
    });
    expect(!control_status_result.ok,
        "#1045: launch contract should reject simultaneous control-source-object and status-bar-text-object requests");

    const auto control_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source-object",
        "--clear-property",
        "--property-name", "ControlSource",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!control_property_result.ok,
        "#1045: launch contract should reject control-source-object combined with property commands");

    const auto stray_control_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--control-source", "customers.name"
    });
    expect(!stray_control_result.ok,
        "#1045: launch contract should reject stray control-source arguments");
}

void test_parse_launch_arguments_for_input_mask_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--input-mask-object",
        "--input-mask", "999-99-9999",
        "--input-mask-target-object-name", "txtPhone",
        "--input-mask-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1046: launch contract should parse input-mask-object requests");
    expect(result.request.input_mask_object, "#1046: launch contract should detect --input-mask-object");
    expect(result.request.input_mask_available && result.request.input_mask == "999-99-9999",
        "#1046: input-mask-object requests should carry input mask text");
    expect(result.request.input_mask_objects.size() == 2U,
        "#1046: input-mask-object requests should collect input mask target selectors");
    if (result.request.input_mask_objects.size() == 2U) {
        expect(result.request.input_mask_objects[0].object_name == "txtPhone" &&
                result.request.input_mask_objects[0].unique_id.empty(),
            "#1046: input-mask-object requests should parse target object-name selectors");
        expect(result.request.input_mask_objects[1].object_name.empty() &&
                result.request.input_mask_objects[1].unique_id == "two-guid",
            "#1046: input-mask-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_input_mask_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1046: launch contract should reject input-mask-object requests without input mask text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--input-mask", "99999"
    });
    expect(!missing_targets_result.ok,
        "#1046: launch contract should reject input-mask-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_input_mask_object_ambiguity() {
    const auto input_control_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--control-source-object",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid",
        "--control-source", "customers.name",
        "--control-source-target-unique-id", "one-guid"
    });
    expect(!input_control_result.ok,
        "#1046: launch contract should reject simultaneous input-mask-object and control-source-object requests");

    const auto input_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask-object",
        "--clear-property",
        "--property-name", "InputMask",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!input_property_result.ok,
        "#1046: launch contract should reject input-mask-object combined with property commands");

    const auto stray_input_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--input-mask", "99999"
    });
    expect(!stray_input_result.ok,
        "#1046: launch contract should reject stray input-mask arguments");
}

void test_parse_launch_arguments_for_format_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--format-object",
        "--format", "999,999.99",
        "--format-target-object-name", "txtAmount",
        "--format-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1047: launch contract should parse format-object requests");
    expect(result.request.format_object, "#1047: launch contract should detect --format-object");
    expect(result.request.format_available && result.request.format == "999,999.99",
        "#1047: format-object requests should carry format text");
    expect(result.request.format_objects.size() == 2U,
        "#1047: format-object requests should collect format target selectors");
    if (result.request.format_objects.size() == 2U) {
        expect(result.request.format_objects[0].object_name == "txtAmount" &&
                result.request.format_objects[0].unique_id.empty(),
            "#1047: format-object requests should parse target object-name selectors");
        expect(result.request.format_objects[1].object_name.empty() &&
                result.request.format_objects[1].unique_id == "two-guid",
            "#1047: format-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_format_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--format-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1047: launch contract should reject format-object requests without format text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--format", "99999"
    });
    expect(!missing_targets_result.ok,
        "#1047: launch contract should reject format-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_format_object_ambiguity() {
    const auto format_input_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--input-mask-object",
        "--format", "99999",
        "--format-target-unique-id", "one-guid",
        "--input-mask", "99999",
        "--input-mask-target-unique-id", "one-guid"
    });
    expect(!format_input_result.ok,
        "#1047: launch contract should reject simultaneous format-object and input-mask-object requests");

    const auto format_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format-object",
        "--clear-property",
        "--property-name", "Format",
        "--format", "99999",
        "--format-target-unique-id", "one-guid"
    });
    expect(!format_property_result.ok,
        "#1047: launch contract should reject format-object combined with property commands");

    const auto stray_format_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--format", "99999"
    });
    expect(!stray_format_result.ok,
        "#1047: launch contract should reject stray format arguments");
}

void test_parse_launch_arguments_for_row_source_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--row-source-object",
        "--row-source", "customers.name,customer_id",
        "--row-source-target-object-name", "cboCustomer",
        "--row-source-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1048: launch contract should parse row-source-object requests");
    expect(result.request.row_source_object, "#1048: launch contract should detect --row-source-object");
    expect(result.request.row_source_available && result.request.row_source == "customers.name,customer_id",
        "#1048: row-source-object requests should carry row source text");
    expect(result.request.row_source_objects.size() == 2U,
        "#1048: row-source-object requests should collect row source target selectors");
    if (result.request.row_source_objects.size() == 2U) {
        expect(result.request.row_source_objects[0].object_name == "cboCustomer" &&
                result.request.row_source_objects[0].unique_id.empty(),
            "#1048: row-source-object requests should parse target object-name selectors");
        expect(result.request.row_source_objects[1].object_name.empty() &&
                result.request.row_source_objects[1].unique_id == "two-guid",
            "#1048: row-source-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_row_source_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1048: launch contract should reject row-source-object requests without row source text");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--row-source", "customers.name"
    });
    expect(!missing_targets_result.ok,
        "#1048: launch contract should reject row-source-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_row_source_object_ambiguity() {
    const auto row_format_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--format-object",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid",
        "--format", "!",
        "--format-target-unique-id", "one-guid"
    });
    expect(!row_format_result.ok,
        "#1048: launch contract should reject simultaneous row-source-object and format-object requests");

    const auto row_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-object",
        "--clear-property",
        "--property-name", "RowSource",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!row_property_result.ok,
        "#1048: launch contract should reject row-source-object combined with property commands");

    const auto stray_row_source_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source", "customers.name"
    });
    expect(!stray_row_source_result.ok,
        "#1048: launch contract should reject stray row-source arguments");
}

void test_parse_launch_arguments_for_row_source_type_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--row-source-type-object",
        "--row-source-type", "6",
        "--row-source-type-target-object-name", "cboCustomer",
        "--row-source-type-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1049: launch contract should parse row-source-type-object requests");
    expect(result.request.row_source_type_object, "#1049: launch contract should detect --row-source-type-object");
    expect(result.request.row_source_type_available && result.request.row_source_type == 6,
        "#1049: row-source-type-object requests should carry row source type values");
    expect(result.request.row_source_type_objects.size() == 2U,
        "#1049: row-source-type-object requests should collect row source type target selectors");
    if (result.request.row_source_type_objects.size() == 2U) {
        expect(result.request.row_source_type_objects[0].object_name == "cboCustomer" &&
                result.request.row_source_type_objects[0].unique_id.empty(),
            "#1049: row-source-type-object requests should parse target object-name selectors");
        expect(result.request.row_source_type_objects[1].object_name.empty() &&
                result.request.row_source_type_objects[1].unique_id == "two-guid",
            "#1049: row-source-type-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_row_source_type_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1049: launch contract should reject row-source-type-object requests without row source type");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "6"
    });
    expect(!missing_targets_result.ok,
        "#1049: launch contract should reject row-source-type-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "fields",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1049: launch contract should reject non-integer row-source-type values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-type", "-1",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1049: launch contract should reject negative row-source-type values before mutation");
}

void test_parse_launch_arguments_rejects_row_source_type_object_ambiguity() {
    const auto type_row_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--row-source-object",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid",
        "--row-source", "customers.name",
        "--row-source-target-unique-id", "one-guid"
    });
    expect(!type_row_result.ok,
        "#1049: launch contract should reject simultaneous row-source-type-object and row-source-object requests");

    const auto type_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type-object",
        "--clear-property",
        "--property-name", "RowSourceType",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!type_property_result.ok,
        "#1049: launch contract should reject row-source-type-object combined with property commands");

    const auto stray_row_source_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--row-source-type", "6"
    });
    expect(!stray_row_source_type_result.ok,
        "#1049: launch contract should reject stray row-source-type arguments");
}

void test_parse_launch_arguments_for_bound_column_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--bound-column-object",
        "--bound-column", "4",
        "--bound-column-target-object-name", "cboCustomer",
        "--bound-column-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1050: launch contract should parse bound-column-object requests");
    expect(result.request.bound_column_object, "#1050: launch contract should detect --bound-column-object");
    expect(result.request.bound_column_available && result.request.bound_column == 4,
        "#1050: bound-column-object requests should carry bound column values");
    expect(result.request.bound_column_objects.size() == 2U,
        "#1050: bound-column-object requests should collect bound column target selectors");
    if (result.request.bound_column_objects.size() == 2U) {
        expect(result.request.bound_column_objects[0].object_name == "cboCustomer" &&
                result.request.bound_column_objects[0].unique_id.empty(),
            "#1050: bound-column-object requests should parse target object-name selectors");
        expect(result.request.bound_column_objects[1].object_name.empty() &&
                result.request.bound_column_objects[1].unique_id == "two-guid",
            "#1050: bound-column-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_bound_column_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1050: launch contract should reject bound-column-object requests without bound column");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "4"
    });
    expect(!missing_targets_result.ok,
        "#1050: launch contract should reject bound-column-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "first",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1050: launch contract should reject non-integer bound-column values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--bound-column", "-1",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1050: launch contract should reject negative bound-column values before mutation");
}

void test_parse_launch_arguments_rejects_bound_column_object_ambiguity() {
    const auto column_type_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--row-source-type-object",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid",
        "--row-source-type", "6",
        "--row-source-type-target-unique-id", "one-guid"
    });
    expect(!column_type_result.ok,
        "#1050: launch contract should reject simultaneous bound-column-object and row-source-type-object requests");

    const auto column_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column-object",
        "--clear-property",
        "--property-name", "BoundColumn",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!column_property_result.ok,
        "#1050: launch contract should reject bound-column-object combined with property commands");

    const auto stray_bound_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--bound-column", "4"
    });
    expect(!stray_bound_column_result.ok,
        "#1050: launch contract should reject stray bound-column arguments");
}

void test_parse_launch_arguments_for_column_count_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--column-count-object",
        "--column-count", "5",
        "--column-count-target-object-name", "cboCustomer",
        "--column-count-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1051: launch contract should parse column-count-object requests");
    expect(result.request.column_count_object, "#1051: launch contract should detect --column-count-object");
    expect(result.request.column_count_available && result.request.column_count == 5,
        "#1051: column-count-object requests should carry column count values");
    expect(result.request.column_count_objects.size() == 2U,
        "#1051: column-count-object requests should collect column count target selectors");
    if (result.request.column_count_objects.size() == 2U) {
        expect(result.request.column_count_objects[0].object_name == "cboCustomer" &&
                result.request.column_count_objects[0].unique_id.empty(),
            "#1051: column-count-object requests should parse target object-name selectors");
        expect(result.request.column_count_objects[1].object_name.empty() &&
                result.request.column_count_objects[1].unique_id == "two-guid",
            "#1051: column-count-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_column_count_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1051: launch contract should reject column-count-object requests without column count");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "5"
    });
    expect(!missing_targets_result.ok,
        "#1051: launch contract should reject column-count-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "many",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1051: launch contract should reject non-integer column-count values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--column-count", "-1",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1051: launch contract should reject negative column-count values before mutation");
}

void test_parse_launch_arguments_rejects_column_count_object_ambiguity() {
    const auto count_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--bound-column-object",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid",
        "--bound-column", "4",
        "--bound-column-target-unique-id", "one-guid"
    });
    expect(!count_column_result.ok,
        "#1051: launch contract should reject simultaneous column-count-object and bound-column-object requests");

    const auto count_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count-object",
        "--clear-property",
        "--property-name", "ColumnCount",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!count_property_result.ok,
        "#1051: launch contract should reject column-count-object combined with property commands");

    const auto stray_column_count_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--column-count", "5"
    });
    expect(!stray_column_count_result.ok,
        "#1051: launch contract should reject stray column-count arguments");
}

void test_parse_launch_arguments_for_style_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--style-object",
        "--style", "2",
        "--style-target-object-name", "cboCustomer",
        "--style-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1052: launch contract should parse style-object requests");
    expect(result.request.style_object, "#1052: launch contract should detect --style-object");
    expect(result.request.style_available && result.request.style == 2,
        "#1052: style-object requests should carry style values");
    expect(result.request.style_objects.size() == 2U,
        "#1052: style-object requests should collect style target selectors");
    if (result.request.style_objects.size() == 2U) {
        expect(result.request.style_objects[0].object_name == "cboCustomer" &&
                result.request.style_objects[0].unique_id.empty(),
            "#1052: style-object requests should parse target object-name selectors");
        expect(result.request.style_objects[1].object_name.empty() &&
                result.request.style_objects[1].unique_id == "two-guid",
            "#1052: style-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_style_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1052: launch contract should reject style-object requests without style");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "2"
    });
    expect(!missing_targets_result.ok,
        "#1052: launch contract should reject style-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "combo",
        "--style-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1052: launch contract should reject non-integer style values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--style", "-1",
        "--style-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1052: launch contract should reject negative style values before mutation");
}

void test_parse_launch_arguments_rejects_style_object_ambiguity() {
    const auto style_count_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--column-count-object",
        "--style", "2",
        "--style-target-unique-id", "one-guid",
        "--column-count", "5",
        "--column-count-target-unique-id", "one-guid"
    });
    expect(!style_count_result.ok,
        "#1052: launch contract should reject simultaneous style-object and column-count-object requests");

    const auto style_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style-object",
        "--clear-property",
        "--property-name", "Style",
        "--style", "2",
        "--style-target-unique-id", "one-guid"
    });
    expect(!style_property_result.ok,
        "#1052: launch contract should reject style-object combined with property commands");

    const auto stray_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--style", "2"
    });
    expect(!stray_style_result.ok,
        "#1052: launch contract should reject stray style arguments");
}

void test_parse_launch_arguments_for_list_index_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--list-index-object",
        "--list-index", "3",
        "--list-index-target-object-name", "cboCustomer",
        "--list-index-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1053: launch contract should parse list-index-object requests");
    expect(result.request.list_index_object, "#1053: launch contract should detect --list-index-object");
    expect(result.request.list_index_available && result.request.list_index == 3,
        "#1053: list-index-object requests should carry list index values");
    expect(result.request.list_index_objects.size() == 2U,
        "#1053: list-index-object requests should collect list index target selectors");
    if (result.request.list_index_objects.size() == 2U) {
        expect(result.request.list_index_objects[0].object_name == "cboCustomer" &&
                result.request.list_index_objects[0].unique_id.empty(),
            "#1053: list-index-object requests should parse target object-name selectors");
        expect(result.request.list_index_objects[1].object_name.empty() &&
                result.request.list_index_objects[1].unique_id == "two-guid",
            "#1053: list-index-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_list_index_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1053: launch contract should reject list-index-object requests without list index");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "3"
    });
    expect(!missing_targets_result.ok,
        "#1053: launch contract should reject list-index-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "selected",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1053: launch contract should reject non-integer list-index values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--list-index", "-1",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1053: launch contract should reject negative list-index values before mutation");
}

void test_parse_launch_arguments_rejects_list_index_object_ambiguity() {
    const auto index_style_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--style-object",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid",
        "--style", "2",
        "--style-target-unique-id", "one-guid"
    });
    expect(!index_style_result.ok,
        "#1053: launch contract should reject simultaneous list-index-object and style-object requests");

    const auto index_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index-object",
        "--clear-property",
        "--property-name", "ListIndex",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!index_property_result.ok,
        "#1053: launch contract should reject list-index-object combined with property commands");

    const auto stray_list_index_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--list-index", "3"
    });
    expect(!stray_list_index_result.ok,
        "#1053: launch contract should reject stray list-index arguments");
}

void test_parse_launch_arguments_for_left_column_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--left-column-object",
        "--left-column", "7",
        "--left-column-target-object-name", "cboCustomer",
        "--left-column-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1054: launch contract should parse left-column-object requests");
    expect(result.request.left_column_object, "#1054: launch contract should detect --left-column-object");
    expect(result.request.left_column_available && result.request.left_column == 7,
        "#1054: left-column-object requests should carry left column values");
    expect(result.request.left_column_objects.size() == 2U,
        "#1054: left-column-object requests should collect left column target selectors");
    if (result.request.left_column_objects.size() == 2U) {
        expect(result.request.left_column_objects[0].object_name == "cboCustomer" &&
                result.request.left_column_objects[0].unique_id.empty(),
            "#1054: left-column-object requests should parse target object-name selectors");
        expect(result.request.left_column_objects[1].object_name.empty() &&
                result.request.left_column_objects[1].unique_id == "two-guid",
            "#1054: left-column-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_left_column_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1054: launch contract should reject left-column-object requests without left column");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "7"
    });
    expect(!missing_targets_result.ok,
        "#1054: launch contract should reject left-column-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "first",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1054: launch contract should reject non-integer left-column values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--left-column", "-1",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1054: launch contract should reject negative left-column values before mutation");
}

void test_parse_launch_arguments_rejects_left_column_object_ambiguity() {
    const auto left_index_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--list-index-object",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid",
        "--list-index", "3",
        "--list-index-target-unique-id", "one-guid"
    });
    expect(!left_index_result.ok,
        "#1054: launch contract should reject simultaneous left-column-object and list-index-object requests");

    const auto left_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column-object",
        "--clear-property",
        "--property-name", "LeftColumn",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!left_property_result.ok,
        "#1054: launch contract should reject left-column-object combined with property commands");

    const auto stray_left_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--left-column", "7"
    });
    expect(!stray_left_column_result.ok,
        "#1054: launch contract should reject stray left-column arguments");
}

void test_parse_launch_arguments_for_display_value_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--display-value-object",
        "--display-value", "Bob \"B\"",
        "--display-value-target-object-name", "cboCustomer",
        "--display-value-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1055: launch contract should parse display-value-object requests");
    expect(result.request.display_value_object, "#1055: launch contract should detect --display-value-object");
    expect(result.request.display_value_available && result.request.display_value == "Bob \"B\"",
        "#1055: display-value-object requests should carry display values");
    expect(result.request.display_value_objects.size() == 2U,
        "#1055: display-value-object requests should collect display-value target selectors");
    if (result.request.display_value_objects.size() == 2U) {
        expect(result.request.display_value_objects[0].object_name == "cboCustomer" &&
                result.request.display_value_objects[0].unique_id.empty(),
            "#1055: display-value-object requests should parse target object-name selectors");
        expect(result.request.display_value_objects[1].object_name.empty() &&
                result.request.display_value_objects[1].unique_id == "two-guid",
            "#1055: display-value-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_display_value_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1055: launch contract should reject display-value-object requests without display value");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--display-value", "Bob"
    });
    expect(!missing_targets_result.ok,
        "#1055: launch contract should reject display-value-object requests without target selectors");
}

void test_parse_launch_arguments_rejects_display_value_object_ambiguity() {
    const auto display_left_column_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--left-column-object",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid",
        "--left-column", "7",
        "--left-column-target-unique-id", "one-guid"
    });
    expect(!display_left_column_result.ok,
        "#1055: launch contract should reject simultaneous display-value-object and left-column-object requests");

    const auto display_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value-object",
        "--clear-property",
        "--property-name", "DisplayValue",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!display_property_result.ok,
        "#1055: launch contract should reject display-value-object combined with property commands");

    const auto stray_display_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--display-value", "Bob"
    });
    expect(!stray_display_value_result.ok,
        "#1055: launch contract should reject stray display-value arguments");
}

void test_parse_launch_arguments_for_selected_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-back-color-object",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-object-name", "lstOrders",
        "--selected-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1056: launch contract should parse selected-back-color-object requests");
    expect(result.request.selected_back_color_object,
        "#1056: launch contract should detect --selected-back-color-object");
    expect(result.request.selected_back_color_available && result.request.selected_back_color == 16777215,
        "#1056: selected-back-color-object requests should carry selected back color values");
    expect(result.request.selected_back_color_objects.size() == 2U,
        "#1056: selected-back-color-object requests should collect selected-back-color target selectors");
    if (result.request.selected_back_color_objects.size() == 2U) {
        expect(result.request.selected_back_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_back_color_objects[0].unique_id.empty(),
            "#1056: selected-back-color-object requests should parse target object-name selectors");
        expect(result.request.selected_back_color_objects[1].object_name.empty() &&
                result.request.selected_back_color_objects[1].unique_id == "two-guid",
            "#1056: selected-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1056: launch contract should reject selected-back-color-object requests without selected back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "16777215"
    });
    expect(!missing_targets_result.ok,
        "#1056: launch contract should reject selected-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "white",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1056: launch contract should reject non-integer selected-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--selected-back-color", "-1",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1056: launch contract should reject negative selected-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_back_color_object_ambiguity() {
    const auto selected_back_display_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--display-value-object",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid",
        "--display-value", "Bob",
        "--display-value-target-unique-id", "one-guid"
    });
    expect(!selected_back_display_value_result.ok,
        "#1056: launch contract should reject simultaneous selected-back-color-object and display-value-object requests");

    const auto selected_back_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color-object",
        "--clear-property",
        "--property-name", "SelectedBackColor",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_back_property_result.ok,
        "#1056: launch contract should reject selected-back-color-object combined with property commands");

    const auto stray_selected_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-back-color", "16777215"
    });
    expect(!stray_selected_back_color_result.ok,
        "#1056: launch contract should reject stray selected-back-color arguments");
}

void test_parse_launch_arguments_for_selected_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-fore-color-object",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-object-name", "lstOrders",
        "--selected-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1057: launch contract should parse selected-fore-color-object requests");
    expect(result.request.selected_fore_color_object,
        "#1057: launch contract should detect --selected-fore-color-object");
    expect(result.request.selected_fore_color_available && result.request.selected_fore_color == 255,
        "#1057: selected-fore-color-object requests should carry selected fore color values");
    expect(result.request.selected_fore_color_objects.size() == 2U,
        "#1057: selected-fore-color-object requests should collect selected-fore-color target selectors");
    if (result.request.selected_fore_color_objects.size() == 2U) {
        expect(result.request.selected_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_fore_color_objects[0].unique_id.empty(),
            "#1057: selected-fore-color-object requests should parse target object-name selectors");
        expect(result.request.selected_fore_color_objects[1].object_name.empty() &&
                result.request.selected_fore_color_objects[1].unique_id == "two-guid",
            "#1057: selected-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1057: launch contract should reject selected-fore-color-object requests without selected fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "255"
    });
    expect(!missing_targets_result.ok,
        "#1057: launch contract should reject selected-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "blue",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1057: launch contract should reject non-integer selected-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-fore-color", "-1",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1057: launch contract should reject negative selected-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_fore_color_object_ambiguity() {
    const auto selected_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--selected-back-color-object",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid",
        "--selected-back-color", "16777215",
        "--selected-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_fore_back_result.ok,
        "#1057: launch contract should reject simultaneous selected-fore-color-object and selected-back-color-object requests");

    const auto selected_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color-object",
        "--clear-property",
        "--property-name", "SelectedForeColor",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_fore_property_result.ok,
        "#1057: launch contract should reject selected-fore-color-object combined with property commands");

    const auto stray_selected_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-fore-color", "255"
    });
    expect(!stray_selected_fore_color_result.ok,
        "#1057: launch contract should reject stray selected-fore-color arguments");
}

void test_parse_launch_arguments_for_selected_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-object-name", "lstOrders",
        "--selected-item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1058: launch contract should parse selected-item-back-color-object requests");
    expect(result.request.selected_item_back_color_object,
        "#1058: launch contract should detect --selected-item-back-color-object");
    expect(result.request.selected_item_back_color_available && result.request.selected_item_back_color == 65280,
        "#1058: selected-item-back-color-object requests should carry selected item back color values");
    expect(result.request.selected_item_back_color_objects.size() == 2U,
        "#1058: selected-item-back-color-object requests should collect selected-item-back-color target selectors");
    if (result.request.selected_item_back_color_objects.size() == 2U) {
        expect(result.request.selected_item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_item_back_color_objects[0].unique_id.empty(),
            "#1058: selected-item-back-color-object requests should parse target object-name selectors");
        expect(result.request.selected_item_back_color_objects[1].object_name.empty() &&
                result.request.selected_item_back_color_objects[1].unique_id == "two-guid",
            "#1058: selected-item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object requests without selected item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "green",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1058: launch contract should reject non-integer selected-item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-item-back-color", "-1",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1058: launch contract should reject negative selected-item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_item_back_color_object_ambiguity() {
    const auto selected_item_selected_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--selected-fore-color-object",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid",
        "--selected-fore-color", "255",
        "--selected-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_selected_fore_result.ok,
        "#1058: launch contract should reject simultaneous selected-item-back-color-object and selected-fore-color-object requests");

    const auto selected_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color-object",
        "--clear-property",
        "--property-name", "SelectedItemBackColor",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_property_result.ok,
        "#1058: launch contract should reject selected-item-back-color-object combined with property commands");

    const auto stray_selected_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-back-color", "65280"
    });
    expect(!stray_selected_item_back_color_result.ok,
        "#1058: launch contract should reject stray selected-item-back-color arguments");
}

void test_parse_launch_arguments_for_selected_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-object-name", "lstOrders",
        "--selected-item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1059: launch contract should parse selected-item-fore-color-object requests");
    expect(result.request.selected_item_fore_color_object,
        "#1059: launch contract should detect --selected-item-fore-color-object");
    expect(result.request.selected_item_fore_color_available && result.request.selected_item_fore_color == 65280,
        "#1059: selected-item-fore-color-object requests should carry selected item fore color values");
    expect(result.request.selected_item_fore_color_objects.size() == 2U,
        "#1059: selected-item-fore-color-object requests should collect selected-item-fore-color target selectors");
    if (result.request.selected_item_fore_color_objects.size() == 2U) {
        expect(result.request.selected_item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.selected_item_fore_color_objects[0].unique_id.empty(),
            "#1059: selected-item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.selected_item_fore_color_objects[1].object_name.empty() &&
                result.request.selected_item_fore_color_objects[1].unique_id == "two-guid",
            "#1059: selected-item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_selected_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object requests without selected item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "green",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1059: launch contract should reject non-integer selected-item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-fore-color", "-1",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1059: launch contract should reject negative selected-item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_selected_item_fore_color_object_ambiguity() {
    const auto selected_item_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--selected-item-back-color-object",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid",
        "--selected-item-back-color", "65280",
        "--selected-item-back-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_fore_back_result.ok,
        "#1059: launch contract should reject simultaneous selected-item-fore-color-object and selected-item-back-color-object requests");

    const auto selected_item_fore_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color-object",
        "--clear-property",
        "--property-name", "SelectedItemForeColor",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!selected_item_fore_property_result.ok,
        "#1059: launch contract should reject selected-item-fore-color-object combined with property commands");

    const auto stray_selected_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selected-item-fore-color", "65280"
    });
    expect(!stray_selected_item_fore_color_result.ok,
        "#1059: launch contract should reject stray selected-item-fore-color arguments");
}

void test_parse_launch_arguments_for_disabled_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-object-name", "lstOrders",
        "--disabled-item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1060: launch contract should parse disabled-item-back-color-object requests");
    expect(result.request.disabled_item_back_color_object,
        "#1060: launch contract should detect --disabled-item-back-color-object");
    expect(result.request.disabled_item_back_color_available && result.request.disabled_item_back_color == 65280,
        "#1060: disabled-item-back-color-object requests should carry disabled item back color values");
    expect(result.request.disabled_item_back_color_objects.size() == 2U,
        "#1060: disabled-item-back-color-object requests should collect disabled-item-back-color target selectors");
    if (result.request.disabled_item_back_color_objects.size() == 2U) {
        expect(result.request.disabled_item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_item_back_color_objects[0].unique_id.empty(),
            "#1060: disabled-item-back-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_item_back_color_objects[1].object_name.empty() &&
                result.request.disabled_item_back_color_objects[1].unique_id == "two-guid",
            "#1060: disabled-item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object requests without disabled item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "green",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1060: launch contract should reject non-integer disabled-item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--disabled-item-back-color", "-1",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1060: launch contract should reject negative disabled-item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_item_back_color_object_ambiguity() {
    const auto disabled_selected_item_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--selected-item-fore-color-object",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid",
        "--selected-item-fore-color", "65280",
        "--selected-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_selected_item_fore_result.ok,
        "#1060: launch contract should reject simultaneous disabled-item-back-color-object and selected-item-fore-color-object requests");

    const auto disabled_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color-object",
        "--clear-property",
        "--property-name", "DisabledItemBackColor",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_item_property_result.ok,
        "#1060: launch contract should reject disabled-item-back-color-object combined with property commands");

    const auto stray_disabled_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-back-color", "65280"
    });
    expect(!stray_disabled_item_back_color_result.ok,
        "#1060: launch contract should reject stray disabled-item-back-color arguments");
}

void test_parse_launch_arguments_for_disabled_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-object-name", "lstOrders",
        "--disabled-item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1061: launch contract should parse disabled-item-fore-color-object requests");
    expect(result.request.disabled_item_fore_color_object,
        "#1061: launch contract should detect --disabled-item-fore-color-object");
    expect(result.request.disabled_item_fore_color_available && result.request.disabled_item_fore_color == 65280,
        "#1061: disabled-item-fore-color-object requests should carry disabled item fore color values");
    expect(result.request.disabled_item_fore_color_objects.size() == 2U,
        "#1061: disabled-item-fore-color-object requests should collect disabled-item-fore-color target selectors");
    if (result.request.disabled_item_fore_color_objects.size() == 2U) {
        expect(result.request.disabled_item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.disabled_item_fore_color_objects[0].unique_id.empty(),
            "#1061: disabled-item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.disabled_item_fore_color_objects[1].object_name.empty() &&
                result.request.disabled_item_fore_color_objects[1].unique_id == "two-guid",
            "#1061: disabled-item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_disabled_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object requests without disabled item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "green",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1061: launch contract should reject non-integer disabled-item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-fore-color", "-1",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1061: launch contract should reject negative disabled-item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_disabled_item_fore_color_object_ambiguity() {
    const auto disabled_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--disabled-item-back-color-object",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid",
        "--disabled-item-back-color", "65280",
        "--disabled-item-back-color-target-unique-id", "one-guid"
    });
    expect(!disabled_fore_back_result.ok,
        "#1061: launch contract should reject simultaneous disabled-item-fore-color-object and disabled-item-back-color-object requests");

    const auto disabled_item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color-object",
        "--clear-property",
        "--property-name", "DisabledItemForeColor",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!disabled_item_property_result.ok,
        "#1061: launch contract should reject disabled-item-fore-color-object combined with property commands");

    const auto stray_disabled_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--disabled-item-fore-color", "65280"
    });
    expect(!stray_disabled_item_fore_color_result.ok,
        "#1061: launch contract should reject stray disabled-item-fore-color arguments");
}

void test_parse_launch_arguments_for_item_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--item-back-color-object",
        "--item-back-color", "65280",
        "--item-back-color-target-object-name", "lstOrders",
        "--item-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1062: launch contract should parse item-back-color-object requests");
    expect(result.request.item_back_color_object,
        "#1062: launch contract should detect --item-back-color-object");
    expect(result.request.item_back_color_available && result.request.item_back_color == 65280,
        "#1062: item-back-color-object requests should carry item back color values");
    expect(result.request.item_back_color_objects.size() == 2U,
        "#1062: item-back-color-object requests should collect item-back-color target selectors");
    if (result.request.item_back_color_objects.size() == 2U) {
        expect(result.request.item_back_color_objects[0].object_name == "lstOrders" &&
                result.request.item_back_color_objects[0].unique_id.empty(),
            "#1062: item-back-color-object requests should parse target object-name selectors");
        expect(result.request.item_back_color_objects[1].object_name.empty() &&
                result.request.item_back_color_objects[1].unique_id == "two-guid",
            "#1062: item-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_item_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1062: launch contract should reject item-back-color-object requests without item back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1062: launch contract should reject item-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "green",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1062: launch contract should reject non-integer item-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--item-back-color", "-1",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1062: launch contract should reject negative item-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_item_back_color_object_ambiguity() {
    const auto item_disabled_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--disabled-item-fore-color-object",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid",
        "--disabled-item-fore-color", "65280",
        "--disabled-item-fore-color-target-unique-id", "one-guid"
    });
    expect(!item_disabled_fore_result.ok,
        "#1062: launch contract should reject simultaneous item-back-color-object and disabled-item-fore-color-object requests");

    const auto item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color-object",
        "--clear-property",
        "--property-name", "ItemBackColor",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!item_property_result.ok,
        "#1062: launch contract should reject item-back-color-object combined with property commands");

    const auto stray_item_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-back-color", "65280"
    });
    expect(!stray_item_back_color_result.ok,
        "#1062: launch contract should reject stray item-back-color arguments");
}

void test_parse_launch_arguments_for_item_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--item-fore-color-object",
        "--item-fore-color", "65280",
        "--item-fore-color-target-object-name", "lstOrders",
        "--item-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1063: launch contract should parse item-fore-color-object requests");
    expect(result.request.item_fore_color_object,
        "#1063: launch contract should detect --item-fore-color-object");
    expect(result.request.item_fore_color_available && result.request.item_fore_color == 65280,
        "#1063: item-fore-color-object requests should carry item fore color values");
    expect(result.request.item_fore_color_objects.size() == 2U,
        "#1063: item-fore-color-object requests should collect item-fore-color target selectors");
    if (result.request.item_fore_color_objects.size() == 2U) {
        expect(result.request.item_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.item_fore_color_objects[0].unique_id.empty(),
            "#1063: item-fore-color-object requests should parse target object-name selectors");
        expect(result.request.item_fore_color_objects[1].object_name.empty() &&
                result.request.item_fore_color_objects[1].unique_id == "two-guid",
            "#1063: item-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_item_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1063: launch contract should reject item-fore-color-object requests without item fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1063: launch contract should reject item-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "green",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1063: launch contract should reject non-integer item-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-fore-color", "-1",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1063: launch contract should reject negative item-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_item_fore_color_object_ambiguity() {
    const auto item_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--item-back-color-object",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid",
        "--item-back-color", "65280",
        "--item-back-color-target-unique-id", "one-guid"
    });
    expect(!item_fore_back_result.ok,
        "#1063: launch contract should reject simultaneous item-fore-color-object and item-back-color-object requests");

    const auto item_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color-object",
        "--clear-property",
        "--property-name", "ItemForeColor",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!item_property_result.ok,
        "#1063: launch contract should reject item-fore-color-object combined with property commands");

    const auto stray_item_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--item-fore-color", "65280"
    });
    expect(!stray_item_fore_color_result.ok,
        "#1063: launch contract should reject stray item-fore-color arguments");
}

void test_parse_launch_arguments_for_highlight_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-back-color-object",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-object-name", "lstOrders",
        "--highlight-back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1064: launch contract should parse highlight-back-color-object requests");
    expect(result.request.highlight_back_color_object,
        "#1064: launch contract should detect --highlight-back-color-object");
    expect(result.request.highlight_back_color_available && result.request.highlight_back_color == 65280,
        "#1064: highlight-back-color-object requests should carry highlight back color values");
    expect(result.request.highlight_back_color_objects.size() == 2U,
        "#1064: highlight-back-color-object requests should collect highlight-back-color target selectors");
    if (result.request.highlight_back_color_objects.size() == 2U) {
        expect(result.request.highlight_back_color_objects[0].object_name == "lstOrders" &&
                result.request.highlight_back_color_objects[0].unique_id.empty(),
            "#1064: highlight-back-color-object requests should parse target object-name selectors");
        expect(result.request.highlight_back_color_objects[1].object_name.empty() &&
                result.request.highlight_back_color_objects[1].unique_id == "two-guid",
            "#1064: highlight-back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1064: launch contract should reject highlight-back-color-object requests without highlight back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1064: launch contract should reject highlight-back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "green",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1064: launch contract should reject non-integer highlight-back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--highlight-back-color", "-1",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1064: launch contract should reject negative highlight-back-color values before mutation");
}

void test_parse_launch_arguments_rejects_highlight_back_color_object_ambiguity() {
    const auto highlight_item_fore_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--item-fore-color-object",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid",
        "--item-fore-color", "65280",
        "--item-fore-color-target-unique-id", "one-guid"
    });
    expect(!highlight_item_fore_result.ok,
        "#1064: launch contract should reject simultaneous highlight-back-color-object and item-fore-color-object requests");

    const auto highlight_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color-object",
        "--clear-property",
        "--property-name", "HighlightBackColor",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!highlight_property_result.ok,
        "#1064: launch contract should reject highlight-back-color-object combined with property commands");

    const auto stray_highlight_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-back-color", "65280"
    });
    expect(!stray_highlight_back_color_result.ok,
        "#1064: launch contract should reject stray highlight-back-color arguments");
}

void test_parse_launch_arguments_for_highlight_fore_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-object-name", "lstOrders",
        "--highlight-fore-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1065: launch contract should parse highlight-fore-color-object requests");
    expect(result.request.highlight_fore_color_object,
        "#1065: launch contract should detect --highlight-fore-color-object");
    expect(result.request.highlight_fore_color_available && result.request.highlight_fore_color == 65280,
        "#1065: highlight-fore-color-object requests should carry highlight fore color values");
    expect(result.request.highlight_fore_color_objects.size() == 2U,
        "#1065: highlight-fore-color-object requests should collect highlight-fore-color target selectors");
    if (result.request.highlight_fore_color_objects.size() == 2U) {
        expect(result.request.highlight_fore_color_objects[0].object_name == "lstOrders" &&
                result.request.highlight_fore_color_objects[0].unique_id.empty(),
            "#1065: highlight-fore-color-object requests should parse target object-name selectors");
        expect(result.request.highlight_fore_color_objects[1].object_name.empty() &&
                result.request.highlight_fore_color_objects[1].unique_id == "two-guid",
            "#1065: highlight-fore-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_highlight_fore_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object requests without highlight fore color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "green",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1065: launch contract should reject non-integer highlight-fore-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-fore-color", "-1",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1065: launch contract should reject negative highlight-fore-color values before mutation");
}

void test_parse_launch_arguments_rejects_highlight_fore_color_object_ambiguity() {
    const auto highlight_fore_back_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--highlight-back-color-object",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid",
        "--highlight-back-color", "65280",
        "--highlight-back-color-target-unique-id", "one-guid"
    });
    expect(!highlight_fore_back_result.ok,
        "#1065: launch contract should reject simultaneous highlight-fore-color-object and highlight-back-color-object requests");

    const auto highlight_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color-object",
        "--clear-property",
        "--property-name", "HighlightForeColor",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!highlight_property_result.ok,
        "#1065: launch contract should reject highlight-fore-color-object combined with property commands");

    const auto stray_highlight_fore_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--highlight-fore-color", "65280"
    });
    expect(!stray_highlight_fore_color_result.ok,
        "#1065: launch contract should reject stray highlight-fore-color arguments");
}

void test_parse_launch_arguments_for_back_color_object() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--json",
        "--back-color-object",
        "--back-color", "65280",
        "--back-color-target-object-name", "lstOrders",
        "--back-color-target-unique-id", "two-guid"
    });

    expect(result.ok, "#1066: launch contract should parse back-color-object requests");
    expect(result.request.back_color_object,
        "#1066: launch contract should detect --back-color-object");
    expect(result.request.back_color_available && result.request.back_color == 65280,
        "#1066: back-color-object requests should carry back color values");
    expect(result.request.back_color_objects.size() == 2U,
        "#1066: back-color-object requests should collect back-color target selectors");
    if (result.request.back_color_objects.size() == 2U) {
        expect(result.request.back_color_objects[0].object_name == "lstOrders" &&
                result.request.back_color_objects[0].unique_id.empty(),
            "#1066: back-color-object requests should parse target object-name selectors");
        expect(result.request.back_color_objects[1].object_name.empty() &&
                result.request.back_color_objects[1].unique_id == "two-guid",
            "#1066: back-color-object requests should parse target unique-id selectors");
    }
}

void test_parse_launch_arguments_rejects_back_color_object_invalid_inputs() {
    const auto missing_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!missing_value_result.ok,
        "#1066: launch contract should reject back-color-object requests without back color");

    const auto missing_targets_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "65280"
    });
    expect(!missing_targets_result.ok,
        "#1066: launch contract should reject back-color-object requests without target selectors");

    const auto invalid_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "green",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!invalid_value_result.ok,
        "#1066: launch contract should reject non-integer back-color values");

    const auto negative_value_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--back-color", "-1",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!negative_value_result.ok,
        "#1066: launch contract should reject negative back-color values before mutation");
}

void test_parse_launch_arguments_rejects_back_color_object_ambiguity() {
    const auto back_highlight_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--highlight-fore-color-object",
        "--back-color", "65280",
        "--back-color-target-unique-id", "one-guid",
        "--highlight-fore-color", "65280",
        "--highlight-fore-color-target-unique-id", "one-guid"
    });
    expect(!back_highlight_result.ok,
        "#1066: launch contract should reject simultaneous back-color-object and highlight-fore-color-object requests");

    const auto back_property_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color-object",
        "--clear-property",
        "--property-name", "BackColor",
        "--back-color", "65280",
        "--back-color-target-unique-id", "one-guid"
    });
    expect(!back_property_result.ok,
        "#1066: launch contract should reject back-color-object combined with property commands");

    const auto stray_back_color_result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--back-color", "65280"
    });
    expect(!stray_back_color_result.ok,
        "#1066: launch contract should reject stray back-color arguments");
}

void test_parse_launch_arguments_rejects_unknown_switch() {
    const auto result = copperfin::studio::parse_launch_arguments({"--mystery"});
    expect(!result.ok, "launch contract should reject unknown switches");
}

void test_parse_launch_arguments_rejects_unknown_undo_mode() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--undo-mode", "mystery"
    });
    expect(!result.ok, "launch contract should reject unknown undo modes");
}

void test_parse_launch_arguments_rejects_unknown_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context", "mystery"
    });
    expect(!result.ok, "#962: launch contract should reject unknown selection-context tokens");
}

void test_parse_launch_arguments_rejects_missing_selection_context() {
    const auto result = copperfin::studio::parse_launch_arguments({
        "--path", "E:\\Forms\\customer.scx",
        "--selection-context"
    });
    expect(!result.ok, "#962: launch contract should reject missing selection-context values");
}

void test_open_document_infers_form_sidecar() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const fs::path sidecar_path = temp_dir / "customer.sct";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .symbol = "form1",
        .line = 10U,
        .column = 2U,
        .launched_from_visual_studio = true,
        .read_only = false
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a valid synthetic SCX file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::form, "SCX should map to a form document");
    expect(result.document.display_name == "customer.scx", "document display name should use the file name");
    expect(result.document.has_sidecar, "open_document should detect the SCT sidecar");
    expect(result.document.sidecar_path == sidecar_path.string(), "open_document should infer the SCT sidecar path");
    expect(result.document.launched_from_visual_studio, "launch metadata should flow into the Studio document");
    expect(result.document.selection_symbol == "form1", "#964: launch selection symbol should flow into the Studio document");
    expect(result.document.selection_line == 10U, "#964: launch selection line should flow into the Studio document");
    expect(result.document.selection_column == 2U, "#964: launch selection column should flow into the Studio document");
    expect(result.document.selection_record_index == 0U,
           "#964: launch selection record index should keep the default when none is supplied");
    expect(!result.document.selection_record_available,
           "#967: launch selection record availability should be false when no record is supplied");
    expect(result.document.inspection.header_available, "inspection metadata should be attached to the document");

    const auto objects = copperfin::studio::build_object_snapshot(result.document);
    expect(objects.empty(), "header-only synthetic SCX should not produce object snapshots without parsed records");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_uses_vfp_filename_for_display_name() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_vfp_filename_tests";
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / R"(E:\Forms\customer.scx)";
    const fs::path sidecar_path = temp_dir / R"(E:\Forms\customer.sct)";

    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    {
        std::ofstream output(sidecar_path, std::ios::binary);
        output << "memo-sidecar";
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .launched_from_visual_studio = true
    });

    expect(result.ok, "#702: open_document should accept synthetic Windows-style VFP path names on the host filesystem");
    expect(result.document.display_name == "customer.scx",
           "#702: Studio display names should use VFP-aware filename parsing for backslash paths");
    expect(result.document.has_sidecar, "#702: sidecar inference should remain compatible with VFP-style path text");
    expect(result.document.sidecar_path == sidecar_path.string(),
           "#702: inferred sidecar path should still replace the extension in the host path");

    std::error_code ignored;
    fs::remove(form_path, ignored);
    fs::remove(sidecar_path, ignored);
    fs::remove(temp_dir, ignored);
}

void test_open_document_attaches_default_designer_contexts() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_designer_context_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const auto write_synthetic_asset = [&](const std::string& filename) {
        const fs::path path = temp_dir / filename;
        const auto bytes = make_vfp_header();
        std::ofstream output(path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        return path;
    };

    const auto form_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("customer.scx").string()
    });
    expect(form_result.ok, "#960: synthetic form should open for designer-context checks");
    expect(form_result.document.designer_contexts.size() == 1U,
           "#960: form documents should expose one default designer context");
    if (!form_result.document.designer_contexts.empty()) {
        const auto& context = form_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#960: form documents should expose the visual-object designer context");
        expect(context.editor_action_count == context.editor_actions.size(),
               "#1009: form designer context should report editor-action count metadata");
        expect(context.builder_count == context.builders.size(),
               "#1009: form designer context should report builder count metadata");
        expect(context.builder_count == 3U,
               "#1010: form designer context should expose form plus control builders");
        expect(context.toolbox_item_count == context.toolbox_items.size(),
               "#1009: form designer context should report toolbox-item count metadata");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#960: form designer context should include property-grid actions");
        expect(has_descriptor_id(context.builders, "form-builder"),
               "#1010: form designer context should include form builder");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#960: form designer context should include control builders");
        expect(has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: form designer context should include form toolbox items");
    }

    const auto container_override_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::container_object
        }
    });
    expect(container_override_result.ok, "#1014: synthetic form should open for explicit container context checks");
    expect(container_override_result.document.designer_contexts.size() == 1U,
           "#1014: explicit container contexts should override the form default context list");
    if (!container_override_result.document.designer_contexts.empty()) {
        const auto& context = container_override_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::container_object,
               "#1014: explicit container contexts should be preserved");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#1014: explicit container contexts should include method-editor actions");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#1014: explicit container contexts should include control builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1014: explicit container contexts should not expose form builders");
        expect(has_descriptor_id(context.toolbox_items, "checkbox"),
               "#1014: explicit container contexts should include container-safe toolbox items");
    }

    const auto class_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("customer.vcx").string()
    });
    expect(class_result.ok, "#1012: synthetic class library should open for designer-context checks");
    expect(class_result.document.designer_contexts.size() == 1U,
           "#1012: class-library documents should expose one default designer context");
    if (!class_result.document.designer_contexts.empty()) {
        const auto& context = class_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::class_designer,
               "#1012: class-library documents should expose the class-designer context");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#1012: class designer context should include property-grid actions");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#1012: class designer context should include method-editor actions");
        expect(has_descriptor_id(context.builders, "class-builder"),
               "#1012: class designer context should include class builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1012: class designer context should not expose form builders");
        expect(!has_descriptor_id(context.builders, "control-builder"),
               "#1012: class designer context should not expose control builders");
        expect(has_descriptor_id(context.toolbox_items, "textbox"),
               "#1012: class designer context should include class-safe toolbox items");
    }

    const auto method_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "cmdSave.Click"
    });
    expect(method_symbol_result.ok, "#963: synthetic form should open for method-symbol context checks");
    expect(method_symbol_result.document.designer_contexts.size() == 1U,
           "#963: method-symbol form documents should expose one inferred designer context");
    if (!method_symbol_result.document.designer_contexts.empty()) {
        const auto& context = method_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#963: method-like symbols should infer the visual-method designer context for forms");
        expect(has_descriptor_id(context.editor_actions, "edit-visual-method"),
               "#963: inferred visual-method contexts should include method-editor actions");
    }

    const auto data_environment_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .symbol = "Dataenvironment.OpenTables"
    });
    expect(data_environment_symbol_result.ok, "#965: synthetic form should open for data-environment symbol checks");
    expect(data_environment_symbol_result.document.designer_contexts.size() == 1U,
           "#965: data-environment symbols should expose one inferred designer context");
    if (!data_environment_symbol_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#965: DataEnvironment method symbols should infer the data-environment designer context for forms");
        expect(context.editor_action_count == context.editor_actions.size() &&
                   context.builder_count == context.builders.size() &&
                   context.toolbox_item_count == 0U,
               "#1009: inferred data-environment contexts should report filtered descriptor counts");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#965: inferred data-environment contexts should include data-environment editor actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#965: inferred data-environment contexts should include data-environment builders");
    }

    const fs::path selected_record_path = temp_dir / "selected_record.scx";
    write_synthetic_form_table_with_data_environment(selected_record_path);
    const auto data_environment_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(data_environment_record_result.ok, "#966: synthetic form should open for selected-record context checks");
    expect(data_environment_record_result.document.designer_contexts.size() == 1U,
           "#966: selected DataEnvironment records should expose one inferred designer context");
    if (!data_environment_record_result.document.designer_contexts.empty()) {
        const auto& context = data_environment_record_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#966: selected DataEnvironment records should infer the data-environment designer context");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#966: selected DataEnvironment records should include data-environment builders");
    }

    const auto visual_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 1U,
        .selection_record_available = true
    });
    expect(visual_record_result.ok, "#966: synthetic form should open for visual selected-record context checks");
    expect(visual_record_result.document.designer_contexts.size() == 1U,
           "#966: visual selected records should preserve the generic form default context count");
    if (!visual_record_result.document.designer_contexts.empty()) {
        expect(visual_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_object,
               "#966: non-DataEnvironment selected records should preserve visual-object defaults");
    }

    const auto container_record_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 2U,
        .selection_record_available = true
    });
    expect(container_record_result.ok, "#1015: synthetic form should open for selected container context checks");
    expect(container_record_result.document.designer_contexts.size() == 1U,
           "#1015: selected container records should expose one inferred designer context");
    if (!container_record_result.document.designer_contexts.empty()) {
        const auto& context = container_record_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::container_object,
               "#1015: selected container records should infer the container-object designer context");
        expect(has_descriptor_id(context.builders, "control-builder"),
               "#1015: selected container records should include control builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1015: selected container records should not include form builders");
        expect(has_descriptor_id(context.toolbox_items, "checkbox"),
               "#1015: selected container records should include container-safe toolbox items");
    }

    const auto multi_override_result = copperfin::studio::open_document({
        .path = (temp_dir / "customer.scx").string(),
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::visual_method,
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(multi_override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(multi_override_result.document.designer_contexts.size() == 2U,
           "#962: explicit selection contexts should override the form default context list");
    if (multi_override_result.document.designer_contexts.size() == 2U) {
        expect(multi_override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::visual_method,
               "#962: explicit visual_method contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[0].editor_actions, "edit-visual-method"),
               "#962: explicit visual_method contexts should include method-editor actions");
        expect(multi_override_result.document.designer_contexts[1].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#962: explicit report_expression contexts should be preserved in request order");
        expect(has_descriptor_id(multi_override_result.document.designer_contexts[1].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto override_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 0U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(override_result.ok, "#962: synthetic form should open for explicit designer-context checks");
    expect(override_result.document.designer_contexts.size() == 1U,
           "#966: explicit selection contexts should override selected-record context defaults");
    if (override_result.document.designer_contexts.size() == 1U) {
        expect(override_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#966: explicit report_expression contexts should win over selected-record data-environment contexts");
        expect(has_descriptor_id(override_result.document.designer_contexts[0].editor_actions, "edit-report-expression"),
               "#962: explicit report_expression contexts should include expression-editor actions");
    }

    const auto container_override_precedence_result = copperfin::studio::open_document({
        .path = selected_record_path.string(),
        .record_index = 2U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::report_expression
        }
    });
    expect(container_override_precedence_result.ok,
           "#1015: synthetic form should open for explicit-over-container checks");
    expect(container_override_precedence_result.document.designer_contexts.size() == 1U,
           "#1015: explicit selection contexts should override selected-record container defaults");
    if (container_override_precedence_result.document.designer_contexts.size() == 1U) {
        expect(container_override_precedence_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#1015: explicit report_expression contexts should win over selected-record container contexts");
    }

    const auto report_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("summary.frx").string()
    });
    expect(report_result.ok, "#960: synthetic report should open for designer-context checks");
    expect(report_result.document.designer_contexts.size() == 1U,
           "#960: report documents should expose one default designer context");
    if (!report_result.document.designer_contexts.empty()) {
        const auto& context = report_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::report_expression,
               "#960: report documents should expose the report-expression designer context");
        expect(context.editor_action_count == context.editor_actions.size() &&
                   context.builder_count == context.builders.size() &&
                   context.toolbox_item_count == context.toolbox_items.size(),
               "#1009: report designer context should report descriptor counts");
        expect(has_descriptor_id(context.editor_actions, "edit-report-expression"),
               "#960: report designer context should include expression editor actions");
        expect(has_descriptor_id(context.builders, "report-builder"),
               "#960: report designer context should include report builders");
        expect(has_descriptor_id(context.toolbox_items, "label"),
               "#960: report designer context should include report-safe toolbox items");
        expect(!has_descriptor_id(context.toolbox_items, "textbox"),
               "#960: report designer context should exclude form-only toolbox items");
    }

    const auto report_data_environment_symbol_result = copperfin::studio::open_document({
        .path = (temp_dir / "summary.frx").string(),
        .symbol = "Dataenvironment.OpenTables"
    });
    expect(report_data_environment_symbol_result.ok,
           "#1016: synthetic report should open for data-environment symbol checks");
    expect(report_data_environment_symbol_result.document.designer_contexts.size() == 1U,
           "#1016: report DataEnvironment symbols should expose one inferred designer context");
    if (!report_data_environment_symbol_result.document.designer_contexts.empty()) {
        const auto& context = report_data_environment_symbol_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: report DataEnvironment symbols should infer the data-environment designer context");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#1016: report DataEnvironment symbols should include data-environment builders");
    }

    const fs::path report_data_environment_path = temp_dir / "report_data_environment.frx";
    write_synthetic_form_table_with_data_environment(report_data_environment_path);
    const auto report_data_environment_record_result = copperfin::studio::open_document({
        .path = report_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(report_data_environment_record_result.ok,
           "#1016: synthetic report should open for selected DataEnvironment record checks");
    expect(report_data_environment_record_result.document.designer_contexts.size() == 1U,
           "#1016: selected report DataEnvironment records should expose one inferred designer context");
    if (!report_data_environment_record_result.document.designer_contexts.empty()) {
        expect(report_data_environment_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: selected report DataEnvironment records should infer the data-environment designer context");
    }

    const auto label_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("mailing.lbx").string()
    });
    expect(label_result.ok, "#1011: synthetic label should open for designer-context checks");
    expect(label_result.document.designer_contexts.size() == 1U,
           "#1011: label documents should expose one default designer context");
    if (!label_result.document.designer_contexts.empty()) {
        const auto& context = label_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::label_expression,
               "#1011: label documents should expose the label-expression designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-report-expression"),
               "#1011: label designer context should include expression editor actions");
        expect(has_descriptor_id(context.builders, "label-wizard"),
               "#1011: label designer context should include label wizard builders");
        expect(!has_descriptor_id(context.builders, "report-builder"),
               "#1011: label designer context should not reuse report builders");
        expect(has_descriptor_id(context.toolbox_items, "label"),
               "#1011: label designer context should include report-safe toolbox items");
    }

    const fs::path label_data_environment_path = temp_dir / "label_data_environment.lbx";
    write_synthetic_form_table_with_data_environment(label_data_environment_path);
    const auto label_data_environment_record_result = copperfin::studio::open_document({
        .path = label_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true
    });
    expect(label_data_environment_record_result.ok,
           "#1016: synthetic label should open for selected DataEnvironment record checks");
    expect(label_data_environment_record_result.document.designer_contexts.size() == 1U,
           "#1016: selected label DataEnvironment records should expose one inferred designer context");
    if (!label_data_environment_record_result.document.designer_contexts.empty()) {
        expect(label_data_environment_record_result.document.designer_contexts.front().selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#1016: selected label DataEnvironment records should infer the data-environment designer context");
    }

    const auto label_override_precedence_result = copperfin::studio::open_document({
        .path = label_data_environment_path.string(),
        .record_index = 0U,
        .selection_record_available = true,
        .designer_selection_contexts = {
            copperfin::studio::StudioEditorSelectionContext::label_expression
        }
    });
    expect(label_override_precedence_result.ok,
           "#1016: synthetic label should open for explicit-over-DataEnvironment checks");
    expect(label_override_precedence_result.document.designer_contexts.size() == 1U,
           "#1016: explicit selection contexts should override selected label DataEnvironment defaults");
    if (label_override_precedence_result.document.designer_contexts.size() == 1U) {
        expect(label_override_precedence_result.document.designer_contexts[0].selection_context ==
                   copperfin::studio::StudioEditorSelectionContext::label_expression,
               "#1016: explicit label_expression contexts should win over selected-record data-environment contexts");
    }

    const auto menu_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("mainmenu.mnx").string()
    });
    expect(menu_result.ok, "#1013: synthetic menu should open for designer-context checks");
    expect(menu_result.document.designer_contexts.size() == 1U,
           "#1013: menu documents should expose one default designer context");
    if (!menu_result.document.designer_contexts.empty()) {
        const auto& context = menu_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::menu_item,
               "#1013: menu documents should expose the menu-item designer context");
        expect(has_descriptor_id(context.editor_actions, "show-property-grid"),
               "#1013: menu designer context should include property-grid actions");
        expect(has_descriptor_id(context.editor_actions, "open-builder"),
               "#1013: menu designer context should include builder actions");
        expect(has_descriptor_id(context.builders, "menu-designer"),
               "#1013: menu designer context should include menu designer builders");
        expect(!has_descriptor_id(context.builders, "form-builder"),
               "#1013: menu designer context should not expose form builders");
        expect(context.toolbox_items.empty(),
               "#1013: menu designer context should not expose toolbox items");
    }

    const auto project_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("demo.pjx").string()
    });
    expect(project_result.ok, "#960: synthetic project should open for designer-context checks");
    expect(project_result.document.designer_contexts.size() == 1U,
           "#960: project documents should expose one default designer context");
    if (!project_result.document.designer_contexts.empty()) {
        const auto& context = project_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::project_item,
               "#960: project documents should expose the project-item designer context");
        expect(has_descriptor_id(context.editor_actions, "navigate-project-item"),
               "#960: project designer context should include project navigation actions");
        expect(has_descriptor_id(context.builders, "application-wizard"),
               "#960: project designer context should include application wizard builders");
        expect(context.toolbox_items.empty(), "#960: project designer context should not expose toolbox items");
    }

    const auto database_result = copperfin::studio::open_document({
        .path = write_synthetic_asset("data.dbc").string()
    });
    expect(database_result.ok, "#960: synthetic database container should open for designer-context checks");
    expect(database_result.document.designer_contexts.size() == 1U,
           "#960: database documents should expose one default designer context");
    if (!database_result.document.designer_contexts.empty()) {
        const auto& context = database_result.document.designer_contexts.front();
        expect(context.selection_context == copperfin::studio::StudioEditorSelectionContext::data_environment,
               "#960: database documents should expose the data-environment designer context");
        expect(has_descriptor_id(context.editor_actions, "edit-data-environment"),
               "#960: data designer context should include data-environment actions");
        expect(has_descriptor_id(context.builders, "data-environment-builder"),
               "#960: data designer context should include data-environment builders");
        expect(context.toolbox_items.empty(), "#960: data designer context should not expose toolbox items");
    }

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_launch_selection_record_metadata() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_selection_metadata_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "customer.scx";
    const auto bytes = make_vfp_header();
    {
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const auto result = copperfin::studio::open_document({
        .path = form_path.string(),
        .symbol = "cmdSave.Click",
        .line = 42U,
        .column = 7U,
        .record_index = 5U,
        .selection_record_available = true
    });

    expect(result.ok, "#964: synthetic form should open for launch selection metadata checks");
    expect(result.document.selection_symbol == "cmdSave.Click",
           "#964: open_document should preserve launch selection symbols");
    expect(result.document.selection_line == 42U,
           "#964: open_document should preserve launch selection lines");
    expect(result.document.selection_column == 7U,
           "#964: open_document should preserve launch selection columns");
    expect(result.document.selection_record_index == 5U,
           "#964: open_document should preserve launch selection record indexes");
    expect(result.document.selection_record_available,
           "#967: open_document should preserve explicit launch selection record availability");

    fs::remove_all(temp_dir, ignored);
}

void test_object_snapshot_preserves_empty_and_null_design_fields() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\customer.scx)";
    document.kind = copperfin::studio::StudioAssetKind::form;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 7U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "cmdSave", .memo_block_number = 101U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "8.000", .memo_block_number = 102U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "1.000", .memo_block_number = 103U},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "WINDOWS", .memo_block_number = 104U},
                {.field_name = "PARENT", .field_type = 'M', .is_null = false, .display_value = "frmCustomer", .memo_block_number = 105U},
                {.field_name = "HELP", .field_type = 'M', .is_null = true, .display_value = "", .memo_block_number = 0U},
                {.field_name = "TAG", .field_type = 'M', .is_null = false, .display_value = ""},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "Caption = Save\r\nEnabled = .T.", .memo_block_number = 7U},
                {.field_name = "UNIQUEID", .field_type = 'M', .is_null = false, .display_value = "cmd-save-1", .memo_block_number = 108U},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 109U},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "commandbutton", .memo_block_number = 110U}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 1U, "#658: form design snapshot should include the parsed record");
    if (!objects.empty()) {
        expect(objects[0].objtype_code == 8, "#667: object snapshots should expose raw OBJTYPE metadata");
        expect(objects[0].objtype_field_index == 1U, "#671: raw OBJTYPE metadata should retain DBF field provenance");
        expect(objects[0].objtype_memo_block_number == 102U, "#724: raw OBJTYPE metadata should retain memo block provenance");
        expect(objects[0].objcode_code == 1, "#667: object snapshots should expose raw OBJCODE metadata");
        expect(objects[0].objcode_field_index == 2U, "#671: raw OBJCODE metadata should retain DBF field provenance");
        expect(objects[0].objcode_memo_block_number == 103U, "#724: raw OBJCODE metadata should retain memo block provenance");
        expect(objects[0].platform == "WINDOWS", "#667: object snapshots should expose raw PLATFORM metadata");
        expect(objects[0].platform_field_index == 3U, "#671: raw PLATFORM metadata should retain DBF field provenance");
        expect(objects[0].platform_memo_block_number == 104U, "#724: raw PLATFORM metadata should retain memo block provenance");
        expect(objects[0].object_name == "cmdSave", "#660: object snapshots should expose the design object name");
        expect(objects[0].object_name_field_index == 0U, "#672: object name metadata should retain DBF field provenance");
        expect(objects[0].object_name_memo_block_number == 101U, "#717: object names should retain selected memo block provenance");
        expect(objects[0].unique_id == "cmd-save-1", "#660: object snapshots should expose stable UNIQUEID metadata");
        expect(objects[0].unique_id_field_index == 8U, "#672: UNIQUEID metadata should retain DBF field provenance");
        expect(objects[0].unique_id_memo_block_number == 108U, "#717: unique IDs should retain selected memo block provenance");
        expect(objects[0].parent_name == "frmCustomer", "#660: object snapshots should expose parent hierarchy metadata");
        expect(objects[0].parent_name_field_index == 4U, "#672: parent hierarchy metadata should retain DBF field provenance");
        expect(objects[0].parent_name_memo_block_number == 105U, "#717: parent names should retain selected memo block provenance");
        expect(objects[0].class_name == "commandbutton", "#660: object snapshots should expose CLASS metadata");
        expect(objects[0].class_name_field_index == 9U, "#672: CLASS metadata should retain DBF field provenance");
        expect(objects[0].class_name_memo_block_number == 109U, "#717: class names should retain selected memo block provenance");
        expect(objects[0].baseclass_name == "commandbutton", "#660: object snapshots should expose BASECLASS metadata");
        expect(objects[0].baseclass_name_field_index == 10U, "#672: BASECLASS metadata should retain DBF field provenance");
        expect(objects[0].baseclass_name_memo_block_number == 110U, "#717: baseclass names should retain selected memo block provenance");
        expect(objects[0].title == "cmdSave", "#673: friendly titles should keep existing form selection priority");
        expect(objects[0].title_field_index == 0U, "#673: friendly title metadata should retain selected DBF field provenance");
        expect(objects[0].title_memo_block_number == 101U, "#717: friendly titles should inherit selected field memo block provenance");
        expect(objects[0].subtitle == "commandbutton", "#673: friendly subtitles should keep existing form selection priority");
        expect(objects[0].subtitle_field_index == 10U, "#673: friendly subtitle metadata should retain selected DBF field provenance");
        expect(objects[0].subtitle_memo_block_number == 110U, "#717: friendly subtitles should inherit selected field memo block provenance");
        const auto parent = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "PARENT";
        });
        const auto tag = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "TAG";
        });
        const auto help = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "HELP";
        });
        const auto caption = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        const auto enabled = std::find_if(objects[0].properties.begin(), objects[0].properties.end(), [](const auto& property) {
            return property.name == "Enabled";
        });

        expect(parent != objects[0].properties.end(), "#660: parent design field should stay in object snapshots");
        if (parent != objects[0].properties.end()) {
            expect(parent->value == "frmCustomer", "#660: parent field should remain available as direct property metadata");
            expect(parent->field_index == 4U, "#659: direct design fields should preserve their DBF field ordinal");
            expect(!parent->derived_from_property_blob, "#659: direct DBF fields should not be marked blob-derived");
            expect(parent->source_line_index == copperfin::studio::StudioObjectMissingLineIndex,
                "#684: direct DBF fields should not masquerade as property-blob line metadata");
            expect(parent->memo_block_number == 105U, "#717: memo-backed direct identity properties should retain memo block provenance");
        }
        expect(help != objects[0].properties.end(), "#658: null design fields should stay in object snapshots");
        if (help != objects[0].properties.end()) {
            expect(help->is_null, "#658: null design field metadata should stay attached");
            expect(help->field_index == 5U, "#659: null direct fields should preserve their DBF field ordinal");
            expect(help->memo_block_number == 0U, "#712: null block-zero memo properties should expose memo block zero");
        }
        expect(tag != objects[0].properties.end(), "#658: empty memo-backed design fields should stay in object snapshots");
        if (tag != objects[0].properties.end()) {
            expect(tag->value.empty(), "#658: empty design fields should preserve their empty value");
            expect(tag->field_index == 6U, "#659: empty direct fields should preserve their DBF field ordinal");
            expect(tag->memo_block_number == 0U, "#712: empty direct memo properties should expose memo block zero");
        }
        expect(caption != objects[0].properties.end(), "#658: visual property blob expansion should still work");
        if (caption != objects[0].properties.end()) {
            expect(caption->field_index == 7U, "#659: blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(caption->derived_from_property_blob, "#659: blob-derived properties should expose their provenance");
            expect(caption->source_line_index == 0U, "#684: first blob-derived property should retain its source memo line");
            expect(caption->memo_block_number == 7U, "#712: blob-derived properties should inherit the source PROPERTIES memo block");
        }
        expect(enabled != objects[0].properties.end(), "#684: second visual property blob line should expand into snapshots");
        if (enabled != objects[0].properties.end()) {
            expect(enabled->field_index == 7U, "#684: later blob-derived properties should retain the source PROPERTIES field ordinal");
            expect(enabled->source_line_index == 1U, "#684: later blob-derived properties should retain their source memo line");
            expect(enabled->memo_block_number == 7U, "#712: later blob-derived properties should inherit the source PROPERTIES memo block");
        }
    }
}

void test_object_snapshot_suppresses_unresolved_memo_placeholders() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\placeholder.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 10U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 40>", .memo_block_number = 40U},
                {.field_name = "NAME", .field_type = 'M', .is_null = false, .display_value = "<memo block 41>"},
                {.field_name = "TITLE", .field_type = 'M', .is_null = false, .display_value = "<memo block 42>"},
                {.field_name = "CLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 43>"},
                {.field_name = "BASECLASS", .field_type = 'M', .is_null = false, .display_value = "<memo block 44>"},
                {.field_name = "PROPERTIES", .field_type = 'M', .is_null = false, .display_value = "<memo block 45>", .memo_block_number = 45U}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#696: unresolved memo placeholders should not prevent object capture");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name.empty(), "#696: unresolved memo object names should not become active names");
        expect(form_objects[0].object_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: active object-name provenance should be missing when usable text is absent");
        expect(form_objects[0].object_name_memo_block_number == 0U,
            "#717: suppressed object-name metadata should expose memo block zero");
        expect(form_objects[0].title == "Record 10", "#696: unresolved memo title sources should use synthetic fallback");
        expect(form_objects[0].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#696: synthetic titles should not masquerade as unresolved memo provenance");
        expect(form_objects[0].title_memo_block_number == 0U,
            "#717: synthetic object titles should expose memo block zero");
        expect(form_objects[0].subtitle.empty(), "#696: unresolved memo subtitle sources should remain absent");
        expect(form_objects[0].subtitle_memo_block_number == 0U,
            "#717: suppressed subtitles should expose memo block zero");
        const auto objname = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "OBJNAME";
        });
        expect(objname != form_objects[0].properties.end(), "#696: direct unresolved memo fields should remain visible as properties");
        if (objname != form_objects[0].properties.end()) {
            expect(objname->field_index == 0U, "#696: direct unresolved memo fields should retain field provenance");
            expect(objname->value.empty(), "#696: direct unresolved memo placeholders should not become property values");
            expect(objname->memo_block_number == 40U, "#712: unresolved direct memo fields should retain memo block provenance");
        }
        const auto caption = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "Caption";
        });
        expect(caption == form_objects[0].properties.end(), "#696: unresolved PROPERTIES memo placeholders should not expand blob properties");
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\placeholder.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 11U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "<memo block 46>", .memo_block_number = 46U},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "<memo block 47>", .memo_block_number = 47U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "<memo block 48>", .memo_block_number = 48U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "fallback_menu"}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#696: unresolved menu memo placeholders should not prevent object capture");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt.empty(), "#696: unresolved menu PROMPT placeholders should not become prompt text");
        expect(menu_objects[0].menu_prompt_field_index == 0U, "#696: menu PROMPT provenance should remain available");
        expect(menu_objects[0].menu_prompt_memo_block_number == 46U,
            "#718: unresolved menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_command.empty(), "#696: unresolved menu COMMAND placeholders should not become command text");
        expect(menu_objects[0].menu_command_field_index == 1U, "#696: menu COMMAND provenance should remain available");
        expect(menu_objects[0].menu_command_memo_block_number == 47U,
            "#718: unresolved menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message.empty(), "#696: unresolved menu MESSAGE placeholders should not become message text");
        expect(menu_objects[0].menu_message_field_index == 2U, "#696: menu MESSAGE provenance should remain available");
        expect(menu_objects[0].menu_message_memo_block_number == 48U,
            "#718: unresolved menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].title == "fallback_menu", "#696: menu titles should skip unresolved PROMPT and use the next usable source");
        expect(menu_objects[0].title_field_index == 3U, "#696: menu title provenance should point at the selected usable source");
    }
}

void test_object_snapshot_trims_normalized_display_metadata() {
    copperfin::studio::StudioDocumentModel form_document;
    form_document.path = R"(E:\Project-Copperfin\samples\trimmed.scx)";
    form_document.kind = copperfin::studio::StudioAssetKind::form;
    form_document.table_preview_available = true;
    form_document.table_preview.records = {
        {
            .record_index = 12U,
            .deleted = false,
            .values = {
                {.field_name = "OBJNAME", .field_type = 'C', .is_null = false, .display_value = "   "},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  cmdSave  "},
                {.field_name = "BASECLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "},
                {.field_name = "PLATFORM", .field_type = 'C', .is_null = false, .display_value = "  WINDOWS  ", .memo_block_number = 134U},
                {.field_name = "CLASS", .field_type = 'C', .is_null = false, .display_value = "  commandbutton  "}
            }
        }
    };

    const auto form_objects = copperfin::studio::build_object_snapshot(form_document);
    expect(form_objects.size() == 1U, "#705: trimmed form metadata should still produce an object snapshot");
    if (!form_objects.empty()) {
        expect(form_objects[0].object_name == "cmdSave",
            "#705: whitespace-only OBJNAME should be ignored and fallback NAME should be trimmed");
        expect(form_objects[0].object_name_field_index == 1U,
            "#705: object-name provenance should point at the selected fallback field");
        expect(form_objects[0].title == "cmdSave", "#705: form titles should use trimmed display metadata");
        expect(form_objects[0].title_field_index == 1U, "#705: form title provenance should point at trimmed NAME");
        expect(form_objects[0].subtitle == "commandbutton", "#705: form subtitles should use trimmed display metadata");
        expect(form_objects[0].subtitle_field_index == 2U, "#705: form subtitle provenance should point at BASECLASS");
        expect(form_objects[0].platform == "WINDOWS", "#705: platform metadata should be trimmed");
        expect(form_objects[0].platform_field_index == 3U, "#705: platform provenance should still point at PLATFORM");
        expect(form_objects[0].platform_memo_block_number == 134U, "#724: trimmed platform metadata should retain source memo block provenance");
        const auto name = std::find_if(form_objects[0].properties.begin(), form_objects[0].properties.end(), [](const auto& property) {
            return property.name == "NAME";
        });
        expect(name != form_objects[0].properties.end(), "#705: direct NAME property should remain visible");
        if (name != form_objects[0].properties.end()) {
            expect(name->value == "  cmdSave  ", "#705: direct DBF property values should remain source-faithful");
            expect(name->field_index == 1U, "#705: direct DBF property provenance should remain unchanged");
        }
    }

    copperfin::studio::StudioDocumentModel menu_document;
    menu_document.path = R"(E:\Project-Copperfin\samples\trimmed.mnx)";
    menu_document.kind = copperfin::studio::StudioAssetKind::menu;
    menu_document.table_preview_available = true;
    menu_document.table_preview.records = {
        {
            .record_index = 13U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "  Customer  ", .memo_block_number = 130U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "  MAIN  "},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "  DO FORM customer  ", .memo_block_number = 132U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "  Open customer  ", .memo_block_number = 133U},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "  customer_menu  "}
            }
        }
    };

    const auto menu_objects = copperfin::studio::build_object_snapshot(menu_document);
    expect(menu_objects.size() == 1U, "#705: trimmed menu metadata should still produce an object snapshot");
    if (!menu_objects.empty()) {
        expect(menu_objects[0].menu_prompt == "Customer", "#705: menu PROMPT metadata should be trimmed");
        expect(menu_objects[0].menu_prompt_memo_block_number == 130U, "#718: decoded menu PROMPT metadata should retain memo block provenance");
        expect(menu_objects[0].menu_level_name == "MAIN", "#705: menu LEVELNAME metadata should be trimmed");
        expect(menu_objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(menu_objects[0].menu_command == "DO FORM customer", "#705: menu COMMAND metadata should be trimmed");
        expect(menu_objects[0].menu_command_memo_block_number == 132U, "#718: decoded menu COMMAND metadata should retain memo block provenance");
        expect(menu_objects[0].menu_message == "Open customer", "#705: menu MESSAGE metadata should be trimmed");
        expect(menu_objects[0].menu_message_memo_block_number == 133U, "#718: decoded menu MESSAGE metadata should retain memo block provenance");
        expect(menu_objects[0].object_name == "customer_menu", "#705: menu object-name fallback should be trimmed");
        expect(menu_objects[0].object_name_field_index == 4U, "#705: menu object-name provenance should stay on NAME");
        expect(menu_objects[0].title == "Customer", "#705: menu title metadata should be trimmed");
        expect(menu_objects[0].title_field_index == 0U, "#705: menu title provenance should stay on PROMPT");
        expect(menu_objects[0].subtitle == "MAIN", "#705: menu subtitle metadata should be trimmed");
        expect(menu_objects[0].subtitle_field_index == 1U, "#705: menu subtitle provenance should stay on LEVELNAME");
        const auto prompt = std::find_if(menu_objects[0].properties.begin(), menu_objects[0].properties.end(), [](const auto& property) {
            return property.name == "PROMPT";
        });
        expect(prompt != menu_objects[0].properties.end(), "#705: direct PROMPT property should remain visible");
        if (prompt != menu_objects[0].properties.end()) {
            expect(prompt->value == "  Customer  ", "#705: direct menu property values should remain source-faithful");
            expect(prompt->field_index == 0U, "#705: direct menu property provenance should remain unchanged");
        }
    }
}

void test_menu_object_snapshot_preserves_normalized_menu_metadata() {
    copperfin::studio::StudioDocumentModel document;
    document.path = R"(E:\Project-Copperfin\samples\mainmenu.mnx)";
    document.kind = copperfin::studio::StudioAssetKind::menu;
    document.table_preview_available = true;
    document.table_preview.records = {
        {
            .record_index = 2U,
            .deleted = false,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Customer", .memo_block_number = 201U},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "MAIN"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO FORM customer", .memo_block_number = 203U},
                {.field_name = "MESSAGE", .field_type = 'M', .is_null = false, .display_value = "Open customer maintenance", .memo_block_number = 204U},
                {.field_name = "OBJTYPE", .field_type = 'N', .is_null = false, .display_value = "3.000", .memo_block_number = 205U},
                {.field_name = "OBJCODE", .field_type = 'N', .is_null = false, .display_value = "7.000", .memo_block_number = 206U}
            }
        },
        {
            .record_index = 3U,
            .deleted = false,
            .values = {
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "TOOLS"},
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Tools"},
                {.field_name = "COMMAND", .field_type = 'M', .is_null = false, .display_value = "DO tools"},
                {.field_name = "NAME", .field_type = 'C', .is_null = false, .display_value = "tools_menu"},
                {.field_name = "PARENTID", .field_type = 'C', .is_null = false, .display_value = "main_menu"}
            }
        },
        {
            .record_index = 4U,
            .deleted = false,
            .values = {
                {.field_name = "COMMENT", .field_type = 'M', .is_null = false, .display_value = "No display fields"}
            }
        },
        {
            .record_index = 5U,
            .deleted = true,
            .values = {
                {.field_name = "PROMPT", .field_type = 'M', .is_null = false, .display_value = "Obsolete"},
                {.field_name = "LEVELNAME", .field_type = 'C', .is_null = false, .display_value = "OLD"}
            }
        }
    };

    const auto objects = copperfin::studio::build_object_snapshot(document);
    expect(objects.size() == 4U, "#668: menu snapshot should include parsed menu records");
    if (objects.size() >= 1U) {
        expect(objects[0].menu_prompt == "Customer", "#668: menu snapshots should expose PROMPT metadata");
        expect(objects[0].menu_prompt_field_index == 0U, "#669: menu PROMPT metadata should retain DBF field provenance");
        expect(objects[0].menu_prompt_memo_block_number == 201U, "#718: menu PROMPT metadata should retain source memo block provenance");
        expect(objects[0].menu_level_name == "MAIN", "#668: menu snapshots should expose LEVELNAME metadata");
        expect(objects[0].menu_level_name_field_index == 1U, "#669: menu LEVELNAME metadata should retain DBF field provenance");
        expect(objects[0].menu_level_name_memo_block_number == 0U, "#718: non-memo menu LEVELNAME metadata should expose memo block zero");
        expect(objects[0].menu_command == "DO FORM customer", "#668: menu snapshots should expose COMMAND metadata");
        expect(objects[0].menu_command_field_index == 2U, "#669: menu COMMAND metadata should retain DBF field provenance");
        expect(objects[0].menu_command_memo_block_number == 203U, "#718: menu COMMAND metadata should retain source memo block provenance");
        expect(objects[0].menu_message == "Open customer maintenance", "#668: menu snapshots should expose MESSAGE metadata");
        expect(objects[0].menu_message_field_index == 3U, "#669: menu MESSAGE metadata should retain DBF field provenance");
        expect(objects[0].menu_message_memo_block_number == 204U, "#718: menu MESSAGE metadata should retain source memo block provenance");
        expect(objects[0].title == "Customer", "#668: menu prompt should continue to drive friendly title fallback");
        expect(objects[0].title_field_index == 0U, "#673: menu title metadata should retain selected PROMPT provenance");
        expect(objects[0].subtitle == "MAIN", "#668: menu level name should continue to drive friendly subtitle fallback");
        expect(objects[0].subtitle_field_index == 1U, "#673: menu subtitle metadata should retain selected LEVELNAME provenance");
        expect(objects[0].objtype_code == 3, "#668: menu snapshots should retain raw OBJTYPE metadata");
        expect(objects[0].objtype_memo_block_number == 205U, "#724: menu OBJTYPE metadata should retain source memo block provenance");
        expect(objects[0].objcode_code == 7, "#668: menu snapshots should retain raw OBJCODE metadata");
        expect(objects[0].objcode_memo_block_number == 206U, "#724: menu OBJCODE metadata should retain source memo block provenance");
    }
    if (objects.size() >= 2U) {
        expect(objects[1].menu_prompt == "Tools", "#668: menu snapshots should expose PROMPT metadata when it is not field zero");
        expect(objects[1].menu_prompt_field_index == 1U, "#670: present menu fields should keep their actual DBF ordinal");
        expect(objects[1].object_name == "tools_menu", "#672: object name metadata should fall back to NAME");
        expect(objects[1].object_name_field_index == 3U, "#672: object name fallback should retain selected NAME field provenance");
        expect(objects[1].parent_name == "main_menu", "#672: parent metadata should fall back to PARENTID");
        expect(objects[1].parent_name_field_index == 4U, "#672: parent fallback should retain selected PARENTID field provenance");
        expect(objects[1].title == "Tools", "#673: menu title metadata should preserve existing PROMPT priority");
        expect(objects[1].title_field_index == 1U, "#673: menu title fallback should retain selected PROMPT provenance");
        expect(objects[1].subtitle == "TOOLS", "#673: menu subtitle metadata should preserve existing LEVELNAME priority");
        expect(objects[1].subtitle_field_index == 0U, "#673: menu subtitle fallback should retain selected LEVELNAME provenance");
        expect(objects[1].unique_id_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing UNIQUEID provenance should use the object missing-field sentinel");
        expect(objects[1].class_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing CLASS provenance should use the object missing-field sentinel");
        expect(objects[1].baseclass_name_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#672: missing BASECLASS provenance should use the object missing-field sentinel");
        expect(objects[1].menu_message.empty(), "#670: missing menu MESSAGE values should remain empty");
        expect(objects[1].menu_message_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#670: missing menu MESSAGE provenance should not masquerade as field zero");
        expect(objects[1].menu_message_memo_block_number == 0U,
            "#718: missing menu MESSAGE metadata should expose memo block zero");
        expect(objects[1].objtype_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJTYPE provenance should use the object missing-field sentinel");
        expect(objects[1].objtype_memo_block_number == 0U,
            "#724: missing OBJTYPE metadata should expose memo block zero");
        expect(objects[1].objcode_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing OBJCODE provenance should use the object missing-field sentinel");
        expect(objects[1].objcode_memo_block_number == 0U,
            "#724: missing OBJCODE metadata should expose memo block zero");
        expect(objects[1].platform_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#671: missing PLATFORM provenance should use the object missing-field sentinel");
        expect(objects[1].platform_memo_block_number == 0U,
            "#724: missing PLATFORM metadata should expose memo block zero");
    }
    if (objects.size() >= 3U) {
        expect(objects[2].title == "Record 4", "#673: snapshots without display fields should keep synthetic title fallback");
        expect(objects[2].title_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: synthesized titles should use the object missing-field sentinel");
        expect(objects[2].subtitle.empty(), "#673: snapshots without subtitle fields should keep empty subtitle fallback");
        expect(objects[2].subtitle_field_index == copperfin::studio::StudioObjectMissingFieldIndex,
            "#673: missing subtitles should use the object missing-field sentinel");
    }
    if (objects.size() >= 4U) {
        expect(objects[3].deleted, "#688: deleted menu records should stay visible on object snapshots");
        expect(objects[3].title == "Obsolete", "#688: deleted menu records should keep normalized title metadata");
        expect(objects[3].title_field_index == 0U, "#688: deleted menu records should keep title provenance");
    }
}

void test_open_document_preserves_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "missing_sidecar.scx";
    {
        const auto bytes = make_vfp_header();
        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .launched_from_visual_studio = false,
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for readable assets that carry validation findings");
    expect(
        result.document.inspection.has_validation_issues(),
        "Studio documents should retain validation findings from asset inspection");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.sidecar_missing";
            }),
        "Studio documents should expose the missing-sidecar validation finding");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_memo_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_memo_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "payload_truncated.scx";
    const fs::path sidecar_path = temp_dir / "payload_truncated.sct";

    {
        std::vector<std::uint8_t> table_bytes(115U, 0U);
        table_bytes[0] = 0x30U;
        table_bytes[1] = 126U;
        table_bytes[2] = 4U;
        table_bytes[3] = 11U;
        table_bytes[4] = 0x01U;
        table_bytes[8] = 97U;
        table_bytes[10] = 18U;
        table_bytes[11] = 0U;
        table_bytes[28] = 0x00U;
        table_bytes[29] = 0x03U;
        table_bytes[32] = 'O';
        table_bytes[33] = 'B';
        table_bytes[34] = 'J';
        table_bytes[35] = 'N';
        table_bytes[36] = 'A';
        table_bytes[37] = 'M';
        table_bytes[38] = 'E';
        table_bytes[43] = 'C';
        table_bytes[44] = 1U;
        table_bytes[48] = 12U;
        table_bytes[64] = 'P';
        table_bytes[65] = 'R';
        table_bytes[66] = 'O';
        table_bytes[67] = 'P';
        table_bytes[68] = 'E';
        table_bytes[69] = 'R';
        table_bytes[70] = 'T';
        table_bytes[71] = 'I';
        table_bytes[72] = 'E';
        table_bytes[73] = 'S';
        table_bytes[75] = 'M';
        table_bytes[76] = 13U;
        table_bytes[80] = 4U;
        table_bytes[96] = 0x0DU;
        table_bytes[97] = 0x20U;
        table_bytes[98] = 't';
        table_bytes[99] = 'x';
        table_bytes[100] = 't';
        table_bytes[101] = 'T';
        table_bytes[102] = 'i';
        table_bytes[103] = 't';
        table_bytes[104] = 'l';
        table_bytes[105] = 'e';
        table_bytes[110] = 0x01U;
        table_bytes[114] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(table_bytes.data()), static_cast<std::streamsize>(table_bytes.size()));
    }

    {
        std::vector<std::uint8_t> memo_bytes(1024U, 0U);
        memo_bytes[3] = 2U;
        memo_bytes[6] = 0x02U;
        memo_bytes[7] = 0x00U;
        memo_bytes[512U + 3U] = 1U;
        memo_bytes[512U + 4U] = 0x00U;
        memo_bytes[512U + 5U] = 0x00U;
        memo_bytes[512U + 6U] = 0x03U;
        memo_bytes[512U + 7U] = 0x84U;
        std::ofstream output(sidecar_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(memo_bytes.data()), static_cast<std::streamsize>(memo_bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for forms with truncated memo payloads");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "memo.payload_truncated";
            }),
        "Studio documents should preserve memo payload validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_preserves_dbf_descriptor_validation_findings() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_descriptor_validation_tests";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path form_path = temp_dir / "bad_fields.scx";
    {
        std::vector<std::uint8_t> bytes(129U, 0U);
        bytes[0] = 0x30U;
        bytes[1] = 126U;
        bytes[2] = 4U;
        bytes[3] = 11U;
        bytes[4] = 0x01U;
        bytes[8] = 97U;
        bytes[10] = 17U;
        bytes[11] = 0U;
        bytes[28] = 0x00U;
        bytes[29] = 0x03U;
        bytes[32] = '1';
        bytes[33] = '2';
        bytes[34] = '3';
        bytes[35] = 'B';
        bytes[36] = 'A';
        bytes[37] = 'D';
        bytes[38] = 'N';
        bytes[39] = 'A';
        bytes[40] = 'M';
        bytes[41] = 'E';
        bytes[43] = 'C';
        bytes[44] = 1U;
        bytes[48] = 8U;
        bytes[64] = '1';
        bytes[65] = '2';
        bytes[66] = '3';
        bytes[67] = 'B';
        bytes[68] = 'A';
        bytes[69] = 'D';
        bytes[70] = 'N';
        bytes[71] = 'A';
        bytes[72] = 'M';
        bytes[73] = 'E';
        bytes[75] = 'C';
        bytes[76] = 9U;
        bytes[80] = 8U;
        bytes[96] = 0x0DU;
        bytes[97] = 0x20U;
        bytes[128] = 0x1AU;

        std::ofstream output(form_path, std::ios::binary);
        output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = form_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should still succeed for assets with DBF descriptor validation findings");
    expect(
        std::any_of(
            result.document.inspection.validation_issues.begin(),
            result.document.inspection.validation_issues.end(),
            [](const copperfin::vfp::AssetValidationIssue& issue) {
                return issue.code == "dbf.field_name_duplicate";
            }),
        "Studio documents should preserve DBF descriptor validation findings");

    fs::remove_all(temp_dir, ignored);
}

void test_open_document_includes_prg_static_diagnostics() {
    namespace fs = std::filesystem;
    const fs::path temp_dir = fs::temp_directory_path() / "copperfin_studio_host_prg_diagnostics";
    std::error_code ignored;
    fs::remove_all(temp_dir, ignored);
    fs::create_directories(temp_dir);

    const fs::path program_path = temp_dir / "flagged.prg";
    {
        std::ofstream output(program_path, std::ios::binary);
        output << "DO WHILE .T.\n";
        output << "x = 1\n";
        output << "ENDDO\n";
    }

    const copperfin::studio::StudioOpenRequest request{
        .path = program_path.string(),
        .read_only = true
    };

    const auto result = copperfin::studio::open_document(request);
    expect(result.ok, "open_document should succeed for a PRG file");
    expect(result.document.kind == copperfin::studio::StudioAssetKind::program, "PRG should map to a program document");
    expect(!result.document.static_diagnostics.empty(), "Studio documents should include PRG static diagnostics");
    expect(
        std::any_of(
            result.document.static_diagnostics.begin(),
            result.document.static_diagnostics.end(),
            [](const copperfin::runtime::PrgStaticDiagnostic& diagnostic) {
                return diagnostic.code == "PRG1001";
            }),
        "Studio documents should surface analyzer diagnostics for PRG files");

    fs::remove_all(temp_dir, ignored);
}

}  // namespace

int main() {
    test_parse_launch_arguments();
    test_parse_launch_arguments_for_clear_property();
    test_parse_launch_arguments_rejects_ambiguous_property_command();
    test_parse_launch_arguments_for_rename_property();
    test_parse_launch_arguments_rejects_rename_property_missing_target();
    test_parse_launch_arguments_rejects_any_ambiguous_property_commands();
    test_parse_launch_arguments_for_delete_object();
    test_parse_launch_arguments_rejects_delete_object_property_ambiguity();
    test_parse_launch_arguments_for_restore_object();
    test_parse_launch_arguments_rejects_restore_object_ambiguity();
    test_parse_launch_arguments_for_duplicate_object();
    test_parse_launch_arguments_rejects_duplicate_object_ambiguity();
    test_parse_launch_arguments_for_rename_object();
    test_parse_launch_arguments_rejects_rename_object_ambiguity_and_empty_identity();
    test_parse_launch_arguments_for_reparent_object();
    test_parse_launch_arguments_rejects_reparent_object_ambiguity_and_missing_parent();
    test_parse_launch_arguments_for_reorder_object();
    test_parse_launch_arguments_rejects_reorder_object_ambiguity_and_missing_placement();
    test_parse_launch_arguments_for_ungroup_object();
    test_parse_launch_arguments_rejects_ungroup_object_ambiguity();
    test_parse_launch_arguments_for_group_object();
    test_parse_launch_arguments_rejects_group_object_invalid_inputs();
    test_parse_launch_arguments_rejects_group_object_ambiguity();
    test_parse_launch_arguments_for_align_object();
    test_parse_launch_arguments_rejects_align_object_invalid_inputs();
    test_parse_launch_arguments_rejects_align_object_ambiguity();
    test_parse_launch_arguments_for_resize_object();
    test_parse_launch_arguments_rejects_resize_object_invalid_inputs();
    test_parse_launch_arguments_rejects_resize_object_ambiguity();
    test_parse_launch_arguments_for_distribute_object();
    test_parse_launch_arguments_rejects_distribute_object_invalid_inputs();
    test_parse_launch_arguments_rejects_distribute_object_ambiguity();
    test_parse_launch_arguments_for_snap_object();
    test_parse_launch_arguments_rejects_snap_object_invalid_inputs();
    test_parse_launch_arguments_rejects_snap_object_ambiguity();
    test_parse_launch_arguments_for_nudge_object();
    test_parse_launch_arguments_rejects_nudge_object_invalid_inputs();
    test_parse_launch_arguments_rejects_nudge_object_ambiguity();
    test_parse_launch_arguments_for_tab_order_object();
    test_parse_launch_arguments_rejects_tab_order_object_invalid_inputs();
    test_parse_launch_arguments_rejects_tab_order_object_ambiguity();
    test_parse_launch_arguments_for_tab_stop_object();
    test_parse_launch_arguments_rejects_tab_stop_object_invalid_inputs();
    test_parse_launch_arguments_rejects_tab_stop_object_ambiguity();
    test_parse_launch_arguments_for_visibility_object();
    test_parse_launch_arguments_rejects_visibility_object_invalid_inputs();
    test_parse_launch_arguments_rejects_visibility_object_ambiguity();
    test_parse_launch_arguments_for_enabled_object();
    test_parse_launch_arguments_rejects_enabled_object_invalid_inputs();
    test_parse_launch_arguments_rejects_enabled_object_ambiguity();
    test_parse_launch_arguments_for_read_only_object();
    test_parse_launch_arguments_rejects_read_only_object_invalid_inputs();
    test_parse_launch_arguments_rejects_read_only_object_ambiguity();
    test_parse_launch_arguments_for_locked_object();
    test_parse_launch_arguments_rejects_locked_object_invalid_inputs();
    test_parse_launch_arguments_rejects_locked_object_ambiguity();
    test_parse_launch_arguments_for_caption_object();
    test_parse_launch_arguments_rejects_caption_object_invalid_inputs();
    test_parse_launch_arguments_rejects_caption_object_ambiguity();
    test_parse_launch_arguments_for_tooltip_text_object();
    test_parse_launch_arguments_rejects_tooltip_text_object_invalid_inputs();
    test_parse_launch_arguments_rejects_tooltip_text_object_ambiguity();
    test_parse_launch_arguments_for_status_bar_text_object();
    test_parse_launch_arguments_rejects_status_bar_text_object_invalid_inputs();
    test_parse_launch_arguments_rejects_status_bar_text_object_ambiguity();
    test_parse_launch_arguments_for_control_source_object();
    test_parse_launch_arguments_rejects_control_source_object_invalid_inputs();
    test_parse_launch_arguments_rejects_control_source_object_ambiguity();
    test_parse_launch_arguments_for_input_mask_object();
    test_parse_launch_arguments_rejects_input_mask_object_invalid_inputs();
    test_parse_launch_arguments_rejects_input_mask_object_ambiguity();
    test_parse_launch_arguments_for_format_object();
    test_parse_launch_arguments_rejects_format_object_invalid_inputs();
    test_parse_launch_arguments_rejects_format_object_ambiguity();
    test_parse_launch_arguments_for_row_source_object();
    test_parse_launch_arguments_rejects_row_source_object_invalid_inputs();
    test_parse_launch_arguments_rejects_row_source_object_ambiguity();
    test_parse_launch_arguments_for_row_source_type_object();
    test_parse_launch_arguments_rejects_row_source_type_object_invalid_inputs();
    test_parse_launch_arguments_rejects_row_source_type_object_ambiguity();
    test_parse_launch_arguments_for_bound_column_object();
    test_parse_launch_arguments_rejects_bound_column_object_invalid_inputs();
    test_parse_launch_arguments_rejects_bound_column_object_ambiguity();
    test_parse_launch_arguments_for_column_count_object();
    test_parse_launch_arguments_rejects_column_count_object_invalid_inputs();
    test_parse_launch_arguments_rejects_column_count_object_ambiguity();
    test_parse_launch_arguments_for_style_object();
    test_parse_launch_arguments_rejects_style_object_invalid_inputs();
    test_parse_launch_arguments_rejects_style_object_ambiguity();
    test_parse_launch_arguments_for_list_index_object();
    test_parse_launch_arguments_rejects_list_index_object_invalid_inputs();
    test_parse_launch_arguments_rejects_list_index_object_ambiguity();
    test_parse_launch_arguments_for_left_column_object();
    test_parse_launch_arguments_rejects_left_column_object_invalid_inputs();
    test_parse_launch_arguments_rejects_left_column_object_ambiguity();
    test_parse_launch_arguments_for_display_value_object();
    test_parse_launch_arguments_rejects_display_value_object_invalid_inputs();
    test_parse_launch_arguments_rejects_display_value_object_ambiguity();
    test_parse_launch_arguments_for_selected_back_color_object();
    test_parse_launch_arguments_rejects_selected_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_selected_back_color_object_ambiguity();
    test_parse_launch_arguments_for_selected_fore_color_object();
    test_parse_launch_arguments_rejects_selected_fore_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_selected_fore_color_object_ambiguity();
    test_parse_launch_arguments_for_selected_item_back_color_object();
    test_parse_launch_arguments_rejects_selected_item_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_selected_item_back_color_object_ambiguity();
    test_parse_launch_arguments_for_selected_item_fore_color_object();
    test_parse_launch_arguments_rejects_selected_item_fore_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_selected_item_fore_color_object_ambiguity();
    test_parse_launch_arguments_for_disabled_item_back_color_object();
    test_parse_launch_arguments_rejects_disabled_item_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_disabled_item_back_color_object_ambiguity();
    test_parse_launch_arguments_for_disabled_item_fore_color_object();
    test_parse_launch_arguments_rejects_disabled_item_fore_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_disabled_item_fore_color_object_ambiguity();
    test_parse_launch_arguments_for_item_back_color_object();
    test_parse_launch_arguments_rejects_item_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_item_back_color_object_ambiguity();
    test_parse_launch_arguments_for_item_fore_color_object();
    test_parse_launch_arguments_rejects_item_fore_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_item_fore_color_object_ambiguity();
    test_parse_launch_arguments_for_highlight_back_color_object();
    test_parse_launch_arguments_rejects_highlight_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_highlight_back_color_object_ambiguity();
    test_parse_launch_arguments_for_highlight_fore_color_object();
    test_parse_launch_arguments_rejects_highlight_fore_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_highlight_fore_color_object_ambiguity();
    test_parse_launch_arguments_for_back_color_object();
    test_parse_launch_arguments_rejects_back_color_object_invalid_inputs();
    test_parse_launch_arguments_rejects_back_color_object_ambiguity();
    test_parse_launch_arguments_rejects_unknown_switch();
    test_parse_launch_arguments_rejects_unknown_undo_mode();
    test_parse_launch_arguments_rejects_unknown_selection_context();
    test_parse_launch_arguments_rejects_missing_selection_context();
    test_open_document_infers_form_sidecar();
    test_open_document_uses_vfp_filename_for_display_name();
    test_open_document_attaches_default_designer_contexts();
    test_open_document_preserves_launch_selection_record_metadata();
    test_object_snapshot_preserves_empty_and_null_design_fields();
    test_object_snapshot_suppresses_unresolved_memo_placeholders();
    test_object_snapshot_trims_normalized_display_metadata();
    test_menu_object_snapshot_preserves_normalized_menu_metadata();
    test_open_document_preserves_validation_findings();
    test_open_document_preserves_memo_validation_findings();
    test_open_document_preserves_dbf_descriptor_validation_findings();
    test_open_document_includes_prg_static_diagnostics();

    if (failures != 0) {
        std::cerr << failures << " test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All tests passed.\n";
    return EXIT_SUCCESS;
}
