// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

std::optional<std::string> validate_form_set_class_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.form_set_class_object && !request.form_set_class_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FormSetClass"),
            "--form-set-class");
    }
    if (request.form_set_class_object && request.form_set_class_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FormSetClass"));
    }
    if (!request.form_set_class_object &&
        (request.form_set_class_available ||
         !request.form_set_class_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.FormSetClassTitle"),
            "--form-set-class-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_default_file_path_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.default_file_path_object && !request.default_file_path_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DefaultFilePath"),
            "--default-file-path");
    }
    if (request.default_file_path_object && request.default_file_path_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DefaultFilePath"));
    }
    if (!request.default_file_path_object &&
        (request.default_file_path_available ||
         !request.default_file_path_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DefaultFilePathTitle"),
            "--default-file-path-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_initial_selected_alias_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.initial_selected_alias_object && !request.initial_selected_alias_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InitialSelectedAlias"),
            "--initial-selected-alias");
    }
    if (request.initial_selected_alias_object && request.initial_selected_alias_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InitialSelectedAlias"));
    }
    if (!request.initial_selected_alias_object &&
        (request.initial_selected_alias_available ||
         !request.initial_selected_alias_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.InitialSelectedAliasTitle"),
            "--initial-selected-alias-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_tab_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.tab_orientation_object && !request.tab_orientation_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrientation"),
            "--tab-orientation");
    }
    if (request.tab_orientation_object && request.tab_orientation_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrientation"));
    }
    if (!request.tab_orientation_object &&
        (request.tab_orientation_available ||
         !request.tab_orientation_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.TabOrientationTitle"),
            "--tab-orientation-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_display_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.display_orientation_object && !request.display_orientation_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayOrientation"),
            "--display-orientation");
    }
    if (request.display_orientation_object && request.display_orientation_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayOrientation"));
    }
    if (!request.display_orientation_object &&
        (request.display_orientation_available ||
         !request.display_orientation_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.DisplayOrientationTitle"),
            "--display-orientation-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_help_context_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.help_context_id_object && !request.help_context_id_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HelpContextId"),
            "--help-context-id");
    }
    if (request.help_context_id_object && request.help_context_id_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HelpContextId"));
    }
    if (!request.help_context_id_object &&
        (request.help_context_id_available ||
         !request.help_context_id_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.HelpContextIdTitle"),
            "--help-context-id-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_help_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_help_id_object && !request.whats_this_help_id_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpId"),
            "--whats-this-help-id");
    }
    if (request.whats_this_help_id_object && request.whats_this_help_id_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpId"));
    }
    if (!request.whats_this_help_id_object &&
        (request.whats_this_help_id_available ||
         !request.whats_this_help_id_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpIdTitle"),
            "--whats-this-help-id-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_help_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_help_object && !request.whats_this_help_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelp"),
            "--whats-this-help");
    }
    if (request.whats_this_help_object && request.whats_this_help_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelp"));
    }
    if (!request.whats_this_help_object &&
        (request.whats_this_help_available ||
         !request.whats_this_help_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisHelpTitle"),
            "--whats-this-help-object");
    }
    return std::nullopt;
}

std::optional<std::string> validate_whats_this_button_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog) {
    if (request.whats_this_button_object && !request.whats_this_button_available) {
        return localized_object_assignment_requires_option(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisButton"),
            "--whats-this-button");
    }
    if (request.whats_this_button_object && request.whats_this_button_objects.empty()) {
        return localized_object_assignment_requires_target(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisButton"));
    }
    if (!request.whats_this_button_object &&
        (request.whats_this_button_available ||
         !request.whats_this_button_objects.empty())) {
        return localized_object_arguments_require_mode(
            catalog,
            catalog.translate("StudioHost.LaunchParse.ObjectAssignment.WhatsThisButtonTitle"),
            "--whats-this-button-object");
    }
    return std::nullopt;
}

}  // namespace copperfin::studio
