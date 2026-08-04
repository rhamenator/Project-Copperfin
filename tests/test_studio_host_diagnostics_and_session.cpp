// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#include "test_studio_host_support.h"

namespace cf_test_studio_host {
void test_launch_parse_errors_resolve_through_localization_catalog() {
    const auto catalog_root = copperfin::localization::resolve_catalog_root();
    const auto english_catalog = copperfin::localization::load_catalogs(catalog_root, "en-US");
    const auto spanish_catalog = copperfin::localization::load_catalogs(catalog_root, "es-419");
    const auto portuguese_catalog = copperfin::localization::load_catalogs(catalog_root, "pt-BR");
    const auto pseudo_catalog = copperfin::localization::load_catalogs(catalog_root, "qps-ploc");

    const std::vector<std::string_view> launch_parse_error_keys{
        "StudioHost.LaunchParse.Error.AnchorSelectorsRequireAlignOrResize",
        "StudioHost.LaunchParse.Error.FieldValueNameValueSyntaxRequired",
        "StudioHost.LaunchParse.Error.IntegerValueRequired",
        "StudioHost.LaunchParse.Error.LogicalValueRequired",
        "StudioHost.LaunchParse.Error.MissingValueAfterOption",
        "StudioHost.LaunchParse.Error.MixedObjectPropertyCommands",
        "StudioHost.LaunchParse.Error.NonNegativeValueRequired",
        "StudioHost.LaunchParse.Error.NotNegativeValueRequired",
        "StudioHost.LaunchParse.Error.NumericValueRequired",
        "StudioHost.LaunchParse.Error.ObjectArgumentsRequireMode",
        "StudioHost.LaunchParse.Error.ObjectActionArgumentsRequireMode",
        "StudioHost.LaunchParse.Error.ObjectActionRequiresEitherOption",
        "StudioHost.LaunchParse.Error.ObjectActionRequiresOption",
        "StudioHost.LaunchParse.Error.ObjectActionRequiresTargetSelector",
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresOption",
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresNonNegativeValue",
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresTargetSelector",
        "StudioHost.LaunchParse.Error.ObjectCommandRequiresOptions",
        "StudioHost.LaunchParse.Error.ObjectGroupRequiresFieldValue",
        "StudioHost.LaunchParse.Error.ObjectGroupRequiresGroupedChildSelector",
        "StudioHost.LaunchParse.Error.FieldValueOnlyWithGroupObject",
        "StudioHost.LaunchParse.Error.GroupedChildSelectorsOnlyWithGroupObject",
        "StudioHost.LaunchParse.Error.NoAssetPathProvided",
        "StudioHost.LaunchParse.Error.PropertyCommandRequiresOption",
        "StudioHost.LaunchParse.Error.RequestArgumentsRequireMode",
        "StudioHost.LaunchParse.Error.DeletedStateRequiresTargetSelector",
        "StudioHost.LaunchParse.Error.RequestItemRequiresOptionAfterTargetSelector",
        "StudioHost.LaunchParse.Error.RequestRequiresOption",
        "StudioHost.LaunchParse.Error.RequestRequiresSelector",
        "StudioHost.LaunchParse.Error.SelectionContextValueRequired",
        "StudioHost.LaunchParse.Error.SingleObjectCommand",
        "StudioHost.LaunchParse.Error.SinglePropertyCommand",
        "StudioHost.LaunchParse.Error.TrueFalseValueRequired",
        "StudioHost.LaunchParse.Error.UndoModeValueRequired",
        "StudioHost.LaunchParse.Error.UnexpectedExtraPositionalArgument",
        "StudioHost.LaunchParse.Error.UnknownArgument",
        "StudioHost.LaunchParse.Error.UnsignedIntegerValueRequired"
    };

    expect(
        english_catalog.translate("StudioHost.LaunchParse.Error.MissingValueAfterOption") ==
            "Missing value after {option}.",
        "#2653: en-US should keep launch-parse error prose stable");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.Error.NoAssetPathProvided") ==
            "No se proporciono ninguna ruta de asset.",
        "#2653: es-419 should localize no-asset-path launch-parse errors");
    expect(
        spanish_catalog.translate("StudioHost.LaunchParse.Error.RequestRequiresSelector") ==
            "Una solicitud de {requestName} requiere al menos un selector {selectorName}.",
        "#2653: es-419 should localize request-selector launch-parse errors");
    expect(
        portuguese_catalog.translate("StudioHost.LaunchParse.Error.SelectionContextValueRequired") ==
            "O valor de {option} deve ser {allowedValues}.",
        "#2653: pt-BR should localize selection-context launch-parse errors");
    expect(
        portuguese_catalog.translate("StudioHost.LaunchParse.Error.ObjectActionRequiresTargetSelector") ==
            "Uma acao de objeto {actionName} exige pelo menos um seletor de destino.",
        "#2653: pt-BR should localize object-action selector launch-parse errors");
    expect(
        pseudo_catalog.translate("StudioHost.LaunchParse.Error.UnknownArgument") ==
            copperfin::localization::pseudo_localize("Unknown argument: {argument}."),
        "#2653: qps-ploc should pseudo-localize launch-parse error prose");

    expect(
        count_missing_locale_keys(spanish_catalog, "es-419", launch_parse_error_keys) == 0U,
        "#2653: es-419 should define every remaining StudioHost.LaunchParse.Error localization key");
    expect(
        count_missing_locale_keys(portuguese_catalog, "pt-BR", launch_parse_error_keys) == 0U,
        "#2653: pt-BR should define every remaining StudioHost.LaunchParse.Error localization key");
    expect(
        count_missing_locale_keys(pseudo_catalog, "qps-ploc", launch_parse_error_keys) == 0U,
        "#2653: qps-ploc should define every remaining StudioHost.LaunchParse.Error localization key");

    const auto missing_record_result = copperfin::studio::parse_launch_arguments({"--record"});
    expect(!missing_record_result.ok, "#2653: parse_launch_arguments should reject missing record values");
    expect(
        missing_record_result.error == "Missing value after --record.",
        "#2653: parse_launch_arguments should preserve missing-value diagnostics for record selectors");

    const auto selection_context_result =
        copperfin::studio::parse_launch_arguments({"--selection-context"});
    expect(
        !selection_context_result.ok,
        "#2653: parse_launch_arguments should reject missing selection-context values");
    expect(
        selection_context_result.error ==
            "The --selection-context value must be visual_object, visual_method, container_object, "
            "class_designer, report_expression, label_expression, menu_item, project_item, or "
            "data_environment.",
        "#2653: parse_launch_arguments should preserve selection-context diagnostics");

    copperfin::test_support::ScopedEnvironmentValue locale_override("COPPERFIN_LOCALE");
    locale_override.set("en-US");
    const auto english_default_result = copperfin::studio::parse_launch_arguments({"--record"});
    locale_override.set("es-419");
    const auto spanish_default_result = copperfin::studio::parse_launch_arguments({"--record"});
    locale_override.set("qps-ploc");
    const auto pseudo_default_result = copperfin::studio::parse_launch_arguments({"--record"});
    constexpr std::string_view missing_value_key = "StudioHost.LaunchParse.Error.MissingValueAfterOption";
    expect(english_default_result.error == english_catalog.translate(missing_value_key, {{"option", "--record"}}) &&
               spanish_default_result.error == spanish_catalog.translate(missing_value_key, {{"option", "--record"}}) &&
               pseudo_default_result.error == pseudo_catalog.translate(missing_value_key, {{"option", "--record"}}),
           "#4370: default launch-contract diagnostics should refresh across locales");
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
    expect(!result.request.deleted_states, "#1201: launch contract should keep deleted-states off by default");
    expect(!result.request.subtree_deleted_state, "#1202: launch contract should keep subtree-deleted-state off by default");
    expect(!result.request.subtree_deleted_available,
        "#1202: launch contract should keep subtree deleted-state unavailable by default");
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
    expect(!result.request.picture_object,
        "#1098: launch contract should keep picture-object off by default");
    expect(!result.request.picture_available,
        "#1098: launch contract should keep picture unavailable by default");
    expect(!result.request.down_picture_object,
        "#1099: launch contract should keep down-picture-object off by default");
    expect(!result.request.down_picture_available,
        "#1099: launch contract should keep down-picture unavailable by default");
    expect(!result.request.disabled_picture_object,
        "#1100: launch contract should keep disabled-picture-object off by default");
    expect(!result.request.disabled_picture_available,
        "#1100: launch contract should keep disabled-picture unavailable by default");
    expect(!result.request.ole_drag_picture_object,
        "#1101: launch contract should keep OLE drag-picture-object off by default");
    expect(!result.request.ole_drag_picture_available,
        "#1101: launch contract should keep OLE drag-picture unavailable by default");
    expect(!result.request.mouse_icon_object,
        "#1102: launch contract should keep mouse-icon-object off by default");
    expect(!result.request.mouse_icon_available,
        "#1102: launch contract should keep mouse-icon unavailable by default");
    expect(!result.request.drag_icon_object,
        "#1103: launch contract should keep drag-icon-object off by default");
    expect(!result.request.drag_icon_available,
        "#1103: launch contract should keep drag-icon unavailable by default");
    expect(!result.request.drag_mode_object,
        "#1104: launch contract should keep drag-mode-object off by default");
    expect(!result.request.drag_mode_available,
        "#1104: launch contract should keep drag-mode unavailable by default");
    expect(!result.request.ole_drag_mode_object,
        "#1105: launch contract should keep OLE drag-mode-object off by default");
    expect(!result.request.ole_drag_mode_available,
        "#1105: launch contract should keep OLE drag-mode unavailable by default");
    expect(!result.request.ole_drop_mode_object,
        "#1106: launch contract should keep OLE drop-mode-object off by default");
    expect(!result.request.ole_drop_mode_available,
        "#1106: launch contract should keep OLE drop-mode unavailable by default");
    expect(!result.request.ole_drop_effects_object,
        "#1107: launch contract should keep OLE drop-effects-object off by default");
    expect(!result.request.ole_drop_effects_available,
        "#1107: launch contract should keep OLE drop-effects unavailable by default");
    expect(!result.request.ole_drop_text_insertion_object,
        "#1108: launch contract should keep OLE drop text-insertion-object off by default");
    expect(!result.request.ole_drop_text_insertion_available,
        "#1108: launch contract should keep OLE drop text-insertion unavailable by default");
    expect(!result.request.button_count_object,
        "#1110: launch contract should keep button-count-object off by default");
    expect(!result.request.button_count_available,
        "#1110: launch contract should keep button-count unavailable by default");
    expect(!result.request.curvature_object,
        "#1111: launch contract should keep curvature-object off by default");
    expect(!result.request.curvature_available,
        "#1111: launch contract should keep curvature unavailable by default");
    expect(!result.request.draw_mode_object,
        "#1112: launch contract should keep draw-mode-object off by default");
    expect(!result.request.draw_mode_available,
        "#1112: launch contract should keep draw-mode unavailable by default");
    expect(!result.request.draw_style_object,
        "#1113: launch contract should keep draw-style-object off by default");
    expect(!result.request.draw_style_available,
        "#1113: launch contract should keep draw-style unavailable by default");
    expect(!result.request.draw_width_object,
        "#1114: launch contract should keep draw-width-object off by default");
    expect(!result.request.draw_width_available,
        "#1114: launch contract should keep draw-width unavailable by default");
    expect(!result.request.fill_style_object,
        "#1115: launch contract should keep fill-style-object off by default");
    expect(!result.request.fill_style_available,
        "#1115: launch contract should keep fill-style unavailable by default");
    expect(!result.request.scale_mode_object,
        "#1116: launch contract should keep scale-mode-object off by default");
    expect(!result.request.scale_mode_available,
        "#1116: launch contract should keep scale-mode unavailable by default");
    expect(!result.request.buffer_mode_object,
        "#1117: launch contract should keep buffer-mode-object off by default");
    expect(!result.request.buffer_mode_available,
        "#1117: launch contract should keep buffer-mode unavailable by default");
    expect(!result.request.buffer_mode_override_object,
        "#1118: launch contract should keep buffer-mode-override-object off by default");
    expect(!result.request.buffer_mode_override_available,
        "#1118: launch contract should keep buffer-mode-override unavailable by default");
    expect(!result.request.data_session_object,
        "#1119: launch contract should keep data-session-object off by default");
    expect(!result.request.data_session_available,
        "#1119: launch contract should keep data-session unavailable by default");
    expect(!result.request.grid_line_color_object,
        "#1120: launch contract should keep grid-line-color-object off by default");
    expect(!result.request.grid_line_color_available,
        "#1120: launch contract should keep grid-line-color unavailable by default");
    expect(!result.request.header_height_object,
        "#1121: launch contract should keep header-height-object off by default");
    expect(!result.request.header_height_available,
        "#1121: launch contract should keep header-height unavailable by default");
    expect(!result.request.row_height_object,
        "#1122: launch contract should keep row-height-object off by default");
    expect(!result.request.row_height_available,
        "#1122: launch contract should keep row-height unavailable by default");
    expect(!result.request.lock_columns_object,
        "#1123: launch contract should keep lock-columns-object off by default");
    expect(!result.request.lock_columns_available,
        "#1123: launch contract should keep lock-columns unavailable by default");
    expect(!result.request.lock_columns_left_object,
        "#1124: launch contract should keep lock-columns-left-object off by default");
    expect(!result.request.lock_columns_left_available,
        "#1124: launch contract should keep lock-columns-left unavailable by default");
    expect(!result.request.grid_line_width_object,
        "#1125: launch contract should keep grid-line-width-object off by default");
    expect(!result.request.grid_line_width_available,
        "#1125: launch contract should keep grid-line-width unavailable by default");
    expect(!result.request.grid_lines_object,
        "#1126: launch contract should keep grid-lines-object off by default");
    expect(!result.request.grid_lines_available,
        "#1126: launch contract should keep grid-lines unavailable by default");
    expect(!result.request.highlight_row_line_width_object,
        "#1127: launch contract should keep highlight-row-line-width-object off by default");
    expect(!result.request.highlight_row_line_width_available,
        "#1127: launch contract should keep highlight-row-line-width unavailable by default");
    expect(!result.request.partition_object,
        "#1128: launch contract should keep partition-object off by default");
    expect(!result.request.partition_available,
        "#1128: launch contract should keep partition unavailable by default");
    expect(!result.request.record_source_type_object,
        "#1129: launch contract should keep record-source-type-object off by default");
    expect(!result.request.record_source_type_available,
        "#1129: launch contract should keep record-source-type unavailable by default");
    expect(!result.request.column_order_object,
        "#1131: launch contract should keep column-order-object off by default");
    expect(!result.request.column_order_available,
        "#1131: launch contract should keep column-order unavailable by default");
    expect(!result.request.highlight_style_object,
        "#1132: launch contract should keep highlight-style-object off by default");
    expect(!result.request.highlight_style_available,
        "#1132: launch contract should keep highlight-style unavailable by default");
    expect(!result.request.child_order_object,
        "#1133: launch contract should keep child-order-object off by default");
    expect(!result.request.child_order_available,
        "#1133: launch contract should keep child-order unavailable by default");
    expect(!result.request.fill_color_object,
        "#1134: launch contract should keep fill-color-object off by default");
    expect(!result.request.fill_color_available,
        "#1134: launch contract should keep fill-color unavailable by default");
    expect(!result.request.list_item_id_object,
        "#1135: launch contract should keep list-item-id-object off by default");
    expect(!result.request.list_item_id_available,
        "#1135: launch contract should keep list-item-id unavailable by default");
    expect(!result.request.tab_orientation_object,
        "#1139: launch contract should keep tab-orientation-object off by default");
    expect(!result.request.tab_orientation_available,
        "#1139: launch contract should keep tab-orientation unavailable by default");
    expect(!result.request.display_orientation_object,
        "#1140: launch contract should keep display-orientation-object off by default");
    expect(!result.request.display_orientation_available,
        "#1140: launch contract should keep display-orientation unavailable by default");
    expect(!result.request.help_context_id_object,
        "#1141: launch contract should keep help-context-id-object off by default");
    expect(!result.request.help_context_id_available,
        "#1141: launch contract should keep help-context-id unavailable by default");
    expect(!result.request.whats_this_help_id_object,
        "#1142: launch contract should keep whats-this-help-id-object off by default");
    expect(!result.request.whats_this_help_id_available,
        "#1142: launch contract should keep whats-this-help-id unavailable by default");
    expect(!result.request.whats_this_help_object,
        "#1143: launch contract should keep whats-this-help-object off by default");
    expect(!result.request.whats_this_help_available,
        "#1143: launch contract should keep whats-this-help unavailable by default");
    expect(!result.request.whats_this_button_object,
        "#1144: launch contract should keep whats-this-button-object off by default");
    expect(!result.request.whats_this_button_available,
        "#1144: launch contract should keep whats-this-button unavailable by default");
    expect(!result.request.record_source_object, "#1130: launch contract should keep record-source-object off by default");
    expect(!result.request.record_source_available, "#1130: launch contract should keep record source unavailable by default");
    expect(!result.request.form_set_class_object, "#1136: launch contract should keep form-set-class-object off by default");
    expect(!result.request.form_set_class_available, "#1136: launch contract should keep form set class unavailable by default");
    expect(!result.request.default_file_path_object, "#1137: launch contract should keep default-file-path-object off by default");
    expect(!result.request.default_file_path_available, "#1137: launch contract should keep default file path unavailable by default");
    expect(!result.request.initial_selected_alias_object, "#1138: launch contract should keep initial-selected-alias-object off by default");
    expect(!result.request.initial_selected_alias_available, "#1138: launch contract should keep initial selected alias unavailable by default");
    expect(!result.request.tooltip_text_object, "#1043: launch contract should keep tooltip-text-object off by default");
    expect(!result.request.tooltip_text_available, "#1043: launch contract should keep tooltip text unavailable by default");
    expect(!result.request.status_bar_text_object, "#1044: launch contract should keep status-bar-text-object off by default");
    expect(!result.request.status_bar_text_available, "#1044: launch contract should keep status-bar text unavailable by default");
    expect(!result.request.control_source_object, "#1045: launch contract should keep control-source-object off by default");
    expect(!result.request.control_source_available, "#1045: launch contract should keep control source unavailable by default");
    expect(!result.request.current_control_object,
        "#1072: launch contract should keep current-control-object off by default");
    expect(!result.request.current_control_available,
        "#1072: launch contract should keep current control unavailable by default");
    expect(!result.request.input_mask_object, "#1046: launch contract should keep input-mask-object off by default");
    expect(!result.request.input_mask_available, "#1046: launch contract should keep input mask unavailable by default");
    expect(!result.request.format_object, "#1047: launch contract should keep format-object off by default");
    expect(!result.request.format_available, "#1047: launch contract should keep format unavailable by default");
    expect(!result.request.row_source_object, "#1048: launch contract should keep row-source-object off by default");
    expect(!result.request.row_source_available, "#1048: launch contract should keep row source unavailable by default");
    expect(!result.request.column_widths_object, "#1196: launch contract should keep column-widths-object off by default");
    expect(!result.request.column_widths_available, "#1196: launch contract should keep column widths unavailable by default");
    expect(!result.request.column_lines_object, "#1197: launch contract should keep column-lines-object off by default");
    expect(!result.request.column_lines_available, "#1197: launch contract should keep column lines unavailable by default");
    expect(!result.request.integral_height_object, "#1198: launch contract should keep integral-height-object off by default");
    expect(!result.request.integral_height_available, "#1198: launch contract should keep integral height unavailable by default");
    expect(!result.request.incremental_search_object, "#1199: launch contract should keep incremental-search-object off by default");
    expect(!result.request.incremental_search_available, "#1199: launch contract should keep incremental search unavailable by default");
    expect(!result.request.multi_select_object, "#1200: launch contract should keep multi-select-object off by default");
    expect(!result.request.multi_select_available, "#1200: launch contract should keep multi select unavailable by default");
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
    expect(!result.request.fore_color_object,
        "#1067: launch contract should keep fore-color-object off by default");
    expect(!result.request.fore_color_available,
        "#1067: launch contract should keep fore color unavailable by default");
    expect(!result.request.disabled_back_color_object,
        "#1068: launch contract should keep disabled-back-color-object off by default");
    expect(!result.request.disabled_back_color_available,
        "#1068: launch contract should keep disabled back color unavailable by default");
    expect(!result.request.disabled_fore_color_object,
        "#1069: launch contract should keep disabled-fore-color-object off by default");
    expect(!result.request.disabled_fore_color_available,
        "#1069: launch contract should keep disabled fore color unavailable by default");
    expect(!result.request.dynamic_back_color_object,
        "#1070: launch contract should keep dynamic-back-color-object off by default");
    expect(!result.request.dynamic_back_color_available,
        "#1070: launch contract should keep dynamic back color unavailable by default");
    expect(!result.request.dynamic_fore_color_object,
        "#1071: launch contract should keep dynamic-fore-color-object off by default");
    expect(!result.request.dynamic_fore_color_available,
        "#1071: launch contract should keep dynamic fore color unavailable by default");
    expect(!result.request.closable_object,
        "#1073: launch contract should keep closable-object off by default");
    expect(!result.request.closable_available,
        "#1073: launch contract should keep closable unavailable by default");
    expect(!result.request.control_box_object,
        "#1074: launch contract should keep control-box-object off by default");
    expect(!result.request.control_box_available,
        "#1074: launch contract should keep control box unavailable by default");
    expect(!result.request.allow_output_object,
        "#1075: launch contract should keep allow-output-object off by default");
    expect(!result.request.allow_output_available,
        "#1075: launch contract should keep allow output unavailable by default");
    expect(!result.request.bind_controls_object,
        "#1146: launch contract should keep bind-controls-object off by default");
    expect(!result.request.bind_controls_available,
        "#1146: launch contract should keep bind controls unavailable by default");
    expect(!result.request.auto_verb_menu_object,
        "#1145: launch contract should keep auto-verb-menu-object off by default");
    expect(!result.request.auto_verb_menu_available,
        "#1145: launch contract should keep auto verb menu unavailable by default");
    expect(!result.request.desktop_object,
        "#1147: launch contract should keep desktop-object off by default");
    expect(!result.request.desktop_available,
        "#1147: launch contract should keep desktop unavailable by default");
    expect(!result.request.key_preview_object,
        "#1148: launch contract should keep key-preview-object off by default");
    expect(!result.request.key_preview_available,
        "#1148: launch contract should keep key preview unavailable by default");
    expect(!result.request.mac_desktop_object,
        "#1149: launch contract should keep mac-desktop-object off by default");
    expect(!result.request.mac_desktop_available,
        "#1149: launch contract should keep mac desktop unavailable by default");
    expect(!result.request.max_button_object,
        "#1150: launch contract should keep max-button-object off by default");
    expect(!result.request.max_button_available,
        "#1150: launch contract should keep max button unavailable by default");
    expect(!result.request.min_button_object,
        "#1155: launch contract should keep min-button-object off by default");
    expect(!result.request.min_button_available,
        "#1155: launch contract should keep min button unavailable by default");
    expect(!result.request.min_height_object,
        "#1156: launch contract should keep min-height-object off by default");
    expect(!result.request.min_height_available,
        "#1156: launch contract should keep min height unavailable by default");
    expect(!result.request.min_width_object,
        "#1157: launch contract should keep min-width-object off by default");
    expect(!result.request.min_width_available,
        "#1157: launch contract should keep min width unavailable by default");
    expect(!result.request.max_height_object,
        "#1151: launch contract should keep max-height-object off by default");
    expect(!result.request.max_height_available,
        "#1151: launch contract should keep max height unavailable by default");
    expect(!result.request.movable_object,
        "#1158: launch contract should keep movable-object off by default");
    expect(!result.request.movable_available,
        "#1158: launch contract should keep movable unavailable by default");
    expect(!result.request.half_height_caption_object,
        "#1159: launch contract should keep half-height-caption-object off by default");
    expect(!result.request.half_height_caption_available,
        "#1159: launch contract should keep half-height-caption unavailable by default");
    expect(!result.request.mdi_form_object,
        "#1160: launch contract should keep mdi-form-object off by default");
    expect(!result.request.mdi_form_available,
        "#1160: launch contract should keep MDI form unavailable by default");
    expect(!result.request.back_style_object,
        "#1161: launch contract should keep back-style-object off by default");
    expect(!result.request.back_style_available,
        "#1161: launch contract should keep back style unavailable by default");
    expect(!result.request.border_style_object,
        "#1162: launch contract should keep border-style-object off by default");
    expect(!result.request.border_style_available,
        "#1162: launch contract should keep border style unavailable by default");
    expect(!result.request.border_width_object,
        "#1163: launch contract should keep border-width-object off by default");
    expect(!result.request.border_width_available,
        "#1163: launch contract should keep border width unavailable by default");
    expect(!result.request.border_color_object,
        "#1164: launch contract should keep border-color-object off by default");
    expect(!result.request.border_color_available,
        "#1164: launch contract should keep border color unavailable by default");
    expect(!result.request.special_effect_object,
        "#1166: launch contract should keep special-effect-object off by default");
    expect(!result.request.special_effect_available,
        "#1166: launch contract should keep special effect unavailable by default");
    expect(!result.request.scroll_bars_object,
        "#1167: launch contract should keep scroll-bars-object off by default");
    expect(!result.request.scroll_bars_available,
        "#1167: launch contract should keep scroll bars unavailable by default");
    expect(!result.request.window_state_object,
        "#1168: launch contract should keep window-state-object off by default");
    expect(!result.request.window_state_available,
        "#1168: launch contract should keep window state unavailable by default");
    expect(!result.request.show_window_object,
        "#1169: launch contract should keep show-window-object off by default");
    expect(!result.request.show_window_available,
        "#1169: launch contract should keep show window unavailable by default");
    expect(!result.request.title_bar_object,
        "#1170: launch contract should keep title-bar-object off by default");
    expect(!result.request.title_bar_available,
        "#1170: launch contract should keep title bar unavailable by default");
    expect(!result.request.mouse_pointer_object,
        "#1171: launch contract should keep mouse-pointer-object off by default");
    expect(!result.request.mouse_pointer_available,
        "#1171: launch contract should keep mouse pointer unavailable by default");
    expect(!result.request.picture_margin_object,
        "#1172: launch contract should keep picture-margin-object off by default");
    expect(!result.request.picture_margin_available,
        "#1172: launch contract should keep picture margin unavailable by default");
    expect(!result.request.picture_position_object,
        "#1173: launch contract should keep picture-position-object off by default");
    expect(!result.request.picture_position_available,
        "#1173: launch contract should keep picture position unavailable by default");
    expect(!result.request.picture_spacing_object,
        "#1174: launch contract should keep picture-spacing-object off by default");
    expect(!result.request.picture_spacing_available,
        "#1174: launch contract should keep picture spacing unavailable by default");
    expect(!result.request.picture_selection_display_object,
        "#1175: launch contract should keep picture-selection-display-object off by default");
    expect(!result.request.picture_selection_display_available,
        "#1175: launch contract should keep picture selection display unavailable by default");
    expect(!result.request.dynamic_input_mask_object,
        "#1176: launch contract should keep dynamic-input-mask-object off by default");
    expect(!result.request.dynamic_input_mask_available,
        "#1176: launch contract should keep dynamic input mask unavailable by default");
    expect(!result.request.dynamic_line_height_object,
        "#1177: launch contract should keep dynamic-line-height-object off by default");
    expect(!result.request.dynamic_line_height_available,
        "#1177: launch contract should keep dynamic line height unavailable by default");
    expect(!result.request.dynamic_alignment_object,
        "#1186: launch contract should keep dynamic-alignment-object off by default");
    expect(!result.request.dynamic_alignment_available,
        "#1186: launch contract should keep dynamic alignment unavailable by default");
    expect(!result.request.dynamic_current_control_object,
        "#1187: launch contract should keep dynamic-current-control-object off by default");
    expect(!result.request.dynamic_current_control_available,
        "#1187: launch contract should keep dynamic current control unavailable by default");
    expect(!result.request.dynamic_font_name_object,
        "#1188: launch contract should keep dynamic-font-name-object off by default");
    expect(!result.request.dynamic_font_name_available,
        "#1188: launch contract should keep dynamic font name unavailable by default");
    expect(!result.request.dynamic_font_size_object,
        "#1189: launch contract should keep dynamic-font-size-object off by default");
    expect(!result.request.dynamic_font_size_available,
        "#1189: launch contract should keep dynamic font size unavailable by default");
    expect(!result.request.dynamic_font_bold_object,
        "#1190: launch contract should keep dynamic-font-bold-object off by default");
    expect(!result.request.dynamic_font_bold_available,
        "#1190: launch contract should keep dynamic font bold unavailable by default");
    expect(!result.request.dynamic_font_italic_object,
        "#1191: launch contract should keep dynamic-font-italic-object off by default");
    expect(!result.request.dynamic_font_italic_available,
        "#1191: launch contract should keep dynamic font italic unavailable by default");
    expect(!result.request.dynamic_font_underline_object,
        "#1192: launch contract should keep dynamic-font-underline-object off by default");
    expect(!result.request.dynamic_font_underline_available,
        "#1192: launch contract should keep dynamic font underline unavailable by default");
    expect(!result.request.dynamic_font_strikethru_object,
        "#1193: launch contract should keep dynamic-font-strikethru-object off by default");
    expect(!result.request.dynamic_font_strikethru_available,
        "#1193: launch contract should keep dynamic font strikethru unavailable by default");
    expect(!result.request.dynamic_font_outline_object,
        "#1194: launch contract should keep dynamic-font-outline-object off by default");
    expect(!result.request.dynamic_font_outline_available,
        "#1194: launch contract should keep dynamic font outline unavailable by default");
    expect(!result.request.dynamic_font_shadow_object,
        "#1195: launch contract should keep dynamic-font-shadow-object off by default");
    expect(!result.request.dynamic_font_shadow_available,
        "#1195: launch contract should keep dynamic font shadow unavailable by default");
    expect(!result.request.font_name_object,
        "#1178: launch contract should keep font-name-object off by default");
    expect(!result.request.font_name_available,
        "#1178: launch contract should keep font name unavailable by default");
    expect(!result.request.font_size_object,
        "#1179: launch contract should keep font-size-object off by default");
    expect(!result.request.font_size_available,
        "#1179: launch contract should keep font size unavailable by default");
    expect(!result.request.font_bold_object,
        "#1180: launch contract should keep font-bold-object off by default");
    expect(!result.request.font_bold_available,
        "#1180: launch contract should keep font bold unavailable by default");
    expect(!result.request.font_italic_object,
        "#1181: launch contract should keep font-italic-object off by default");
    expect(!result.request.font_italic_available,
        "#1181: launch contract should keep font italic unavailable by default");
    expect(!result.request.font_underline_object,
        "#1182: launch contract should keep font-underline-object off by default");
    expect(!result.request.font_underline_available,
        "#1182: launch contract should keep font underline unavailable by default");
    expect(!result.request.font_strikethru_object,
        "#1183: launch contract should keep font-strikethru-object off by default");
    expect(!result.request.font_strikethru_available,
        "#1183: launch contract should keep font strikethru unavailable by default");
    expect(!result.request.font_outline_object,
        "#1184: launch contract should keep font-outline-object off by default");
    expect(!result.request.font_outline_available,
        "#1184: launch contract should keep font outline unavailable by default");
    expect(!result.request.font_shadow_object,
        "#1185: launch contract should keep font-shadow-object off by default");
    expect(!result.request.font_shadow_available,
        "#1185: launch contract should keep font shadow unavailable by default");
    expect(!result.request.max_width_object,
        "#1152: launch contract should keep max-width-object off by default");
    expect(!result.request.max_width_available,
        "#1152: launch contract should keep max width unavailable by default");
    expect(!result.request.max_left_object,
        "#1153: launch contract should keep max-left-object off by default");
    expect(!result.request.max_left_available,
        "#1153: launch contract should keep max left unavailable by default");
    expect(!result.request.max_top_object,
        "#1154: launch contract should keep max-top-object off by default");
    expect(!result.request.max_top_available,
        "#1154: launch contract should keep max top unavailable by default");
    expect(!result.request.auto_center_object,
        "#1078: launch contract should keep auto-center-object off by default");
    expect(!result.request.auto_center_available,
        "#1078: launch contract should keep auto center unavailable by default");
    expect(!result.request.auto_size_object,
        "#1079: launch contract should keep auto-size-object off by default");
    expect(!result.request.auto_size_available,
        "#1079: launch contract should keep auto size unavailable by default");
    expect(!result.request.auto_release_object,
        "#1080: launch contract should keep auto-release-object off by default");
    expect(!result.request.auto_release_available,
        "#1080: launch contract should keep auto release unavailable by default");
    expect(!result.request.continuous_scroll_object,
        "#1081: launch contract should keep continuous-scroll-object off by default");
    expect(!result.request.continuous_scroll_available,
        "#1081: launch contract should keep continuous scroll unavailable by default");
    expect(!result.request.dockable_object,
        "#1082: launch contract should keep dockable-object off by default");
    expect(!result.request.dockable_available,
        "#1082: launch contract should keep dockable unavailable by default");
    expect(!result.request.clip_controls_object,
        "#1083: launch contract should keep clip-controls-object off by default");
    expect(!result.request.clip_controls_available,
        "#1083: launch contract should keep clip controls unavailable by default");
    expect(!result.request.sparse_object,
        "#1084: launch contract should keep sparse-object off by default");
    expect(!result.request.sparse_available,
        "#1084: launch contract should keep sparse unavailable by default");
    expect(!result.request.lock_screen_object,
        "#1085: launch contract should keep lock-screen-object off by default");
    expect(!result.request.lock_screen_available,
        "#1085: launch contract should keep lock screen unavailable by default");
    expect(!result.request.hide_selection_object,
        "#1109: launch contract should keep hide-selection-object off by default");
    expect(!result.request.hide_selection_available,
        "#1109: launch contract should keep hide selection unavailable by default");
    expect(!result.request.allow_cell_selection_object,
        "#1086: launch contract should keep allow-cell-selection-object off by default");
    expect(!result.request.allow_cell_selection_available,
        "#1086: launch contract should keep allow cell selection unavailable by default");
    expect(!result.request.delete_mark_object,
        "#1087: launch contract should keep delete-mark-object off by default");
    expect(!result.request.delete_mark_available,
        "#1087: launch contract should keep delete mark unavailable by default");
    expect(!result.request.record_mark_object,
        "#1088: launch contract should keep record-mark-object off by default");
    expect(!result.request.record_mark_available,
        "#1088: launch contract should keep record mark unavailable by default");
    expect(!result.request.split_bar_object,
        "#1089: launch contract should keep split-bar-object off by default");
    expect(!result.request.split_bar_available,
        "#1089: launch contract should keep split bar unavailable by default");
    expect(!result.request.highlight_row_object,
        "#1090: launch contract should keep highlight-row-object off by default");
    expect(!result.request.highlight_row_available,
        "#1090: launch contract should keep highlight row unavailable by default");
    expect(!result.request.panel_link_object,
        "#1091: launch contract should keep panel-link-object off by default");
    expect(!result.request.panel_link_available,
        "#1091: launch contract should keep panel link unavailable by default");
    expect(!result.request.allow_header_sizing_object,
        "#1092: launch contract should keep allow-header-sizing-object off by default");
    expect(!result.request.allow_header_sizing_available,
        "#1092: launch contract should keep allow header sizing unavailable by default");
    expect(!result.request.allow_row_sizing_object,
        "#1093: launch contract should keep allow-row-sizing-object off by default");
    expect(!result.request.allow_row_sizing_available,
        "#1093: launch contract should keep allow row sizing unavailable by default");
    expect(!result.request.resizable_object,
        "#1094: launch contract should keep resizable-object off by default");
    expect(!result.request.resizable_available,
        "#1094: launch contract should keep resizable unavailable by default");
    expect(!result.request.add_line_feeds_object,
        "#1095: launch contract should keep add-line-feeds-object off by default");
    expect(!result.request.add_line_feeds_available,
        "#1095: launch contract should keep add line feeds unavailable by default");
    expect(!result.request.always_on_top_object,
        "#1096: launch contract should keep always-on-top-object off by default");
    expect(!result.request.always_on_top_available,
        "#1096: launch contract should keep always on top unavailable by default");
    expect(!result.request.always_on_bottom_object,
        "#1097: launch contract should keep always-on-bottom-object off by default");
    expect(!result.request.always_on_bottom_available,
        "#1097: launch contract should keep always on bottom unavailable by default");
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

}  // namespace cf_test_studio_host
