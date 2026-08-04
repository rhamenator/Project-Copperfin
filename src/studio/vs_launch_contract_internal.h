// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COPPERFIN_VS_LAUNCH_CONTRACT_INTERNAL_H
#define COPPERFIN_VS_LAUNCH_CONTRACT_INTERNAL_H

#include "copperfin/studio/vs_launch_contract.h"

#include "copperfin/localization/localization.h"

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <optional>
#include <string_view>

namespace copperfin::studio {

// ==== Value parsing primitives ====
bool parse_size_value(const std::string& text, std::size_t& value);
bool parse_int_value(const std::string& text, int& value);
bool parse_double_value(const std::string& text, double& value);
std::string lowercase_copy(std::string text);
std::optional<bool> parse_bool_value(std::string text);
std::optional<StudioEditorSelectionContext> parse_selection_context_token(std::string token);
const localization::LocalizedCatalog& default_launch_catalog();

// ==== Localized error-message builders ====
std::string localized_missing_value_after_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_selection_context_error(const localization::LocalizedCatalog& catalog);
std::string localized_unsigned_integer_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_numeric_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_integer_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_non_negative_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_not_negative_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_true_false_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_logical_value_required(
    const localization::LocalizedCatalog& catalog,
    std::string_view option);
std::string localized_deleted_state_requires_target_selector(const localization::LocalizedCatalog& catalog);
std::string localized_field_value_name_value_syntax_required(const localization::LocalizedCatalog& catalog);
std::string localized_object_assignment_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view option);
std::string localized_object_assignment_requires_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name);
std::string localized_object_assignment_requires_non_negative_value(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view value_name);
std::string localized_object_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view property_name,
    std::string_view option);
std::string localized_request_requires_selector(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view selector_name);
std::string localized_request_item_requires_option_after_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view item_name,
    std::string_view selector_name,
    std::string_view option);
std::string localized_request_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view option);
std::string localized_request_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view request_name,
    std::string_view option);
std::string localized_object_action_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view option);
std::string localized_object_action_requires_either_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view first_option,
    std::string_view second_option);
std::string localized_object_action_requires_target(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name);
std::string localized_object_action_arguments_require_mode(
    const localization::LocalizedCatalog& catalog,
    std::string_view action_name,
    std::string_view option);
std::string localized_layout_action_alignment(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_alignment_title(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_distribution(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_distribution_title(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_nudge(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_nudge_title(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_resize(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_resize_title(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_snap(const localization::LocalizedCatalog& catalog);
std::string localized_layout_action_snap_title(const localization::LocalizedCatalog& catalog);
std::string localized_undo_mode_value_required(const localization::LocalizedCatalog& catalog);
std::string localized_unknown_argument(
    const localization::LocalizedCatalog& catalog,
    std::string_view argument);
std::string localized_unexpected_extra_positional_argument(
    const localization::LocalizedCatalog& catalog,
    std::string_view argument);
std::string localized_property_command_requires_option(
    const localization::LocalizedCatalog& catalog,
    std::string_view command_name,
    std::string_view option);
std::string localized_property_command_clear(const localization::LocalizedCatalog& catalog);
std::string localized_property_command_rename(const localization::LocalizedCatalog& catalog);
std::string localized_property_command_update(const localization::LocalizedCatalog& catalog);
std::string localized_no_asset_path_provided(const localization::LocalizedCatalog& catalog);
std::string localized_object_command_rename(const localization::LocalizedCatalog& catalog);
std::string localized_object_command_reparent(const localization::LocalizedCatalog& catalog);
std::string localized_object_command_reorder(const localization::LocalizedCatalog& catalog);
std::string localized_object_rename_required_options(const localization::LocalizedCatalog& catalog);
std::string localized_object_reparent_required_options(const localization::LocalizedCatalog& catalog);
std::string localized_object_assignment_format(const localization::LocalizedCatalog& catalog);
std::string localized_object_assignment_format_title(const localization::LocalizedCatalog& catalog);
std::string localized_object_command_requires_options(
    const localization::LocalizedCatalog& catalog,
    std::string_view command_name,
    std::string_view options);
std::string localized_object_group_requires_grouped_child_selector(
    const localization::LocalizedCatalog& catalog);
std::string localized_object_group_requires_field_value(const localization::LocalizedCatalog& catalog);
std::string localized_field_value_only_with_group_object(const localization::LocalizedCatalog& catalog);
std::string localized_grouped_child_selectors_only_with_group_object(
    const localization::LocalizedCatalog& catalog);

// ==== Per-flag argument parsers (multi-token CLI options) ====
bool parse_form_set_class_argument(const std::string& argument,
                                   const localization::LocalizedCatalog& catalog,
                                   const std::vector<std::string>& args,
                                   std::size_t& index,
                                   LaunchParseResult& result,
                                   std::string& error);
bool parse_default_file_path_argument(const std::string& argument,
                                      const localization::LocalizedCatalog& catalog,
                                      const std::vector<std::string>& args,
                                      std::size_t& index,
                                      LaunchParseResult& result,
                                      std::string& error);
bool parse_initial_selected_alias_argument(const std::string& argument,
                                           const localization::LocalizedCatalog& catalog,
                                           const std::vector<std::string>& args,
                                           std::size_t& index,
                                           LaunchParseResult& result,
                                           std::string& error);
bool parse_tab_orientation_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error);
bool parse_display_orientation_argument(const std::string& argument,
                                        const localization::LocalizedCatalog& catalog,
                                        const std::vector<std::string>& args,
                                        std::size_t& index,
                                        LaunchParseResult& result,
                                        std::string& error);
bool parse_help_context_id_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error);
bool parse_whats_this_help_id_argument(const std::string& argument,
                                       const localization::LocalizedCatalog& catalog,
                                       const std::vector<std::string>& args,
                                       std::size_t& index,
                                       LaunchParseResult& result,
                                       std::string& error);
bool parse_whats_this_help_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error);
bool parse_whats_this_button_argument(const std::string& argument,
                                      const localization::LocalizedCatalog& catalog,
                                      const std::vector<std::string>& args,
                                      std::size_t& index,
                                      LaunchParseResult& result,
                                      std::string& error);

// ==== Per-flag post-parse request validators ====
std::optional<std::string> validate_form_set_class_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_default_file_path_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_initial_selected_alias_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_tab_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_display_orientation_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_help_context_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_whats_this_help_id_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_whats_this_help_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);
std::optional<std::string> validate_whats_this_button_request(
    const StudioOpenRequest& request,
    const localization::LocalizedCatalog& catalog);

struct LaunchArgumentDispatchOutcome {
    bool handled = false;
    bool should_return = false;
};

// ==== Argument dispatch, grouped by CLI-flag theme ====
LaunchArgumentDispatchOutcome try_parse_diagnostics_and_session(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_selection_context(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_property_commands(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_object_lifecycle(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_layout_actions(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_setters_behavior(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_setters_appearance(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_setters_data(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);
LaunchArgumentDispatchOutcome try_parse_positional_and_fallback(
    const std::string& argument,
    const localization::LocalizedCatalog& catalog,
    const std::vector<std::string>& args,
    std::size_t& index,
    LaunchParseResult& result,
    std::string& parsed_argument_error);

// ==== Post-parse validation, grouped by CLI-flag theme ====
std::optional<LaunchParseResult> validate_diagnostics_and_session(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_property_commands(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_object_lifecycle(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_layout_actions(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_setters_behavior(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_setters_appearance(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);
std::optional<LaunchParseResult> validate_setters_data(
    const LaunchParseResult& result,
    const localization::LocalizedCatalog& catalog);

}  // namespace copperfin::studio

#endif
