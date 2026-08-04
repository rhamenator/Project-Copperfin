// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::string localized_missing_value_after_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.MissingValueAfterOption",
        {
            {"option", std::string(option)}
        });
}

std::string localized_selection_context_error(const localization::LocalizedCatalog& catalog) {
    const std::string allowed_values = catalog.translate(
        "StudioHost.LaunchParse.SelectionContextAllowedValues",
        {
            {"visualObject", studio_editor_selection_context_name(StudioEditorSelectionContext::visual_object)},
            {"visualMethod", studio_editor_selection_context_name(StudioEditorSelectionContext::visual_method)},
            {"containerObject", studio_editor_selection_context_name(StudioEditorSelectionContext::container_object)},
            {"classDesigner", studio_editor_selection_context_name(StudioEditorSelectionContext::class_designer)},
            {"reportExpression", studio_editor_selection_context_name(StudioEditorSelectionContext::report_expression)},
            {"labelExpression", studio_editor_selection_context_name(StudioEditorSelectionContext::label_expression)},
            {"menuItem", studio_editor_selection_context_name(StudioEditorSelectionContext::menu_item)},
            {"projectItem", studio_editor_selection_context_name(StudioEditorSelectionContext::project_item)},
            {"dataEnvironment", studio_editor_selection_context_name(StudioEditorSelectionContext::data_environment)}
        });
    return catalog.translate(
        "StudioHost.LaunchParse.Error.SelectionContextValueRequired",
        {
            {"option", "--selection-context"},
            {"allowedValues", allowed_values}
        });
}

std::string localized_unsigned_integer_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.UnsignedIntegerValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_numeric_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.NumericValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_integer_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.IntegerValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_non_negative_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.NonNegativeValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_not_negative_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.NotNegativeValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_true_false_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.TrueFalseValueRequired",
        {
            {"option", std::string(option)},
            {"trueValue", "true"},
            {"falseValue", "false"}
        });
}

std::string localized_logical_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.LogicalValueRequired",
        {
            {"option", std::string(option)}
        });
}

std::string localized_deleted_state_requires_target_selector(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.Error.DeletedStateRequiresTargetSelector");
}

std::string localized_field_value_name_value_syntax_required(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.FieldValueNameValueSyntaxRequired",
        {
            {"syntax", "name=value"}
        });
}

std::string localized_object_assignment_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresOption",
        {
            {"propertyName", std::string(property_name)},
            {"option", std::string(option)}
        });
}

std::string localized_object_assignment_requires_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresTargetSelector",
        {
            {"propertyName", std::string(property_name)}
        });
}

std::string localized_object_assignment_requires_non_negative_value(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view value_name) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectAssignmentRequiresNonNegativeValue",
        {
            {"propertyName", std::string(property_name)},
            {"valueName", std::string(value_name)}
        });
}

std::string localized_object_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectArgumentsRequireMode",
        {
            {"propertyName", std::string(property_name)},
            {"option", std::string(option)}
        });
}

std::string localized_request_requires_selector(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view selector_name) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.RequestRequiresSelector",
        {
            {"requestName", std::string(request_name)},
            {"selectorName", std::string(selector_name)}
        });
}

std::string localized_request_item_requires_option_after_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view item_name,
    std::string_view selector_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.RequestItemRequiresOptionAfterTargetSelector",
        {
            {"itemName", std::string(item_name)},
            {"selectorName", std::string(selector_name)},
            {"option", std::string(option)}
        });
}

std::string localized_request_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.RequestRequiresOption",
        {
            {"requestName", std::string(request_name)},
            {"option", std::string(option)}
        });
}

std::string localized_request_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.RequestArgumentsRequireMode",
        {
            {"requestName", std::string(request_name)},
            {"option", std::string(option)}
        });
}

std::string localized_object_action_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectActionRequiresOption",
        {
            {"actionName", std::string(action_name)},
            {"option", std::string(option)}
        });
}

std::string localized_object_action_requires_either_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view first_option,
    std::string_view second_option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectActionRequiresEitherOption",
        {
            {"actionName", std::string(action_name)},
            {"firstOption", std::string(first_option)},
            {"secondOption", std::string(second_option)}
        });
}

std::string localized_object_action_requires_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectActionRequiresTargetSelector",
        {
            {"actionName", std::string(action_name)}
        });
}

std::string localized_object_action_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectActionArgumentsRequireMode",
        {
            {"actionName", std::string(action_name)},
            {"option", std::string(option)}
        });
}

std::string localized_layout_action_alignment(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.Alignment");
}

std::string localized_layout_action_alignment_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.AlignmentTitle");
}

std::string localized_layout_action_distribution(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.Distribution");
}

std::string localized_layout_action_distribution_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.DistributionTitle");
}

std::string localized_layout_action_nudge(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.Nudge");
}

std::string localized_layout_action_nudge_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.NudgeTitle");
}

std::string localized_layout_action_resize(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.Resize");
}

std::string localized_layout_action_resize_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.ResizeTitle");
}

std::string localized_layout_action_snap(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.Snap");
}

std::string localized_layout_action_snap_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAction.SnapTitle");
}

std::string localized_undo_mode_value_required(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.UndoModeValueRequired",
        {
            {"option", "--undo-mode"},
            {"firstValue", "edit"},
            {"secondValue", "command"}
        });
}

std::string localized_unknown_argument(
    const localization::LocalizedCatalog& catalog,
    std::string_view argument) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.UnknownArgument",
        {
            {"argument", std::string(argument)}
        });
}

std::string localized_unexpected_extra_positional_argument(
    const localization::LocalizedCatalog& catalog,
    std::string_view argument) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.UnexpectedExtraPositionalArgument",
        {
            {"argument", std::string(argument)}
        });
}

std::string localized_property_command_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view command_name,
    std::string_view option) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.PropertyCommandRequiresOption",
        {
            {"commandName", std::string(command_name)},
            {"option", std::string(option)}
        });
}

std::string localized_property_command_clear(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.PropertyCommand.Clear");
}

std::string localized_property_command_rename(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.PropertyCommand.Rename");
}

std::string localized_property_command_update(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.PropertyCommand.Update");
}

std::string localized_no_asset_path_provided(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.Error.NoAssetPathProvided");
}

std::string localized_object_command_rename(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectCommand.Rename");
}

std::string localized_object_command_reparent(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectCommand.Reparent");
}

std::string localized_object_command_reorder(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectCommand.Reorder");
}

std::string localized_object_rename_required_options(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.ObjectCommand.RenameRequiredOptions",
        {
            {"newObjectNameOption", "--new-object-name"},
            {"newNameOption", "--new-name"},
            {"newUniqueIdOption", "--new-unique-id"}
        });
}

std::string localized_object_reparent_required_options(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.ObjectCommand.ReparentRequiredOptions",
        {
            {"parentNameOption", "--parent-name"},
            {"parentUniqueIdOption", "--parent-unique-id"},
            {"clearParentOption", "--clear-parent"}
        });
}

std::string localized_object_assignment_format(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAssignment.Format");
}

std::string localized_object_assignment_format_title(const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FormatTitle");
}

std::string localized_object_command_requires_options(
    const localization::LocalizedCatalog& catalog,
    std::string_view command_name,
    std::string_view options) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectCommandRequiresOptions",
        {
            {"commandName", std::string(command_name)},
            {"options", std::string(options)}
        });
}

std::string localized_object_group_requires_grouped_child_selector(
    const localization::LocalizedCatalog& catalog) {
    return catalog.translate("StudioHost.LaunchParse.Error.ObjectGroupRequiresGroupedChildSelector");
}

std::string localized_object_group_requires_field_value(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.ObjectGroupRequiresFieldValue",
        {
            {"option", "--field-value"}
        });
}

std::string localized_field_value_only_with_group_object(const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.FieldValueOnlyWithGroupObject",
        {
            {"option", "--field-value"},
            {"commandOption", "--group-object"}
        });
}

std::string localized_grouped_child_selectors_only_with_group_object(
    const localization::LocalizedCatalog& catalog) {
    return catalog.translate(
        "StudioHost.LaunchParse.Error.GroupedChildSelectorsOnlyWithGroupObject",
        {
            {"commandOption", "--group-object"}
        });
}

}  // namespace copperfin::studio
