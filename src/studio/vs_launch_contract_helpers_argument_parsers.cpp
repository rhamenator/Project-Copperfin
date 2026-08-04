// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

#include "vs_launch_contract_internal.h"

namespace copperfin::studio {

bool parse_form_set_class_argument(const std::string& argument,
                                   const localization::LocalizedCatalog& catalog,
                                   const std::vector<std::string>& args,
                                   std::size_t& index,
                                   LaunchParseResult& result,
                                   std::string& error) {
    if (argument == "--form-set-class-object") {
        result.request.form_set_class_object = true;
        return true;
    }

    if (argument == "--form-set-class") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--form-set-class");
            return true;
        }
        result.request.form_set_class = args[++index];
        result.request.form_set_class_available = true;
        return true;
    }

    if (argument == "--form-set-class-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--form-set-class-target-object-name");
            return true;
        }
        result.request.form_set_class_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--form-set-class-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--form-set-class-target-unique-id");
            return true;
        }
        result.request.form_set_class_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_default_file_path_argument(const std::string& argument,
                                      const localization::LocalizedCatalog& catalog,
                                      const std::vector<std::string>& args,
                                      std::size_t& index,
                                      LaunchParseResult& result,
                                      std::string& error) {
    if (argument == "--default-file-path-object") {
        result.request.default_file_path_object = true;
        return true;
    }

    if (argument == "--default-file-path") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--default-file-path");
            return true;
        }
        result.request.default_file_path = args[++index];
        result.request.default_file_path_available = true;
        return true;
    }

    if (argument == "--default-file-path-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--default-file-path-target-object-name");
            return true;
        }
        result.request.default_file_path_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--default-file-path-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--default-file-path-target-unique-id");
            return true;
        }
        result.request.default_file_path_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_initial_selected_alias_argument(const std::string& argument,
                                           const localization::LocalizedCatalog& catalog,
                                           const std::vector<std::string>& args,
                                           std::size_t& index,
                                           LaunchParseResult& result,
                                           std::string& error) {
    if (argument == "--initial-selected-alias-object") {
        result.request.initial_selected_alias_object = true;
        return true;
    }

    if (argument == "--initial-selected-alias") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--initial-selected-alias");
            return true;
        }
        result.request.initial_selected_alias = args[++index];
        result.request.initial_selected_alias_available = true;
        return true;
    }

    if (argument == "--initial-selected-alias-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--initial-selected-alias-target-object-name");
            return true;
        }
        result.request.initial_selected_alias_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--initial-selected-alias-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--initial-selected-alias-target-unique-id");
            return true;
        }
        result.request.initial_selected_alias_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_tab_orientation_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error) {
    if (argument == "--tab-orientation-object") {
        result.request.tab_orientation_object = true;
        return true;
    }

    if (argument == "--tab-orientation") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--tab-orientation");
            return true;
        }
        int tab_orientation = 0;
        if (!parse_int_value(args[++index], tab_orientation)) {
            error = localized_integer_value_required(catalog, "--tab-orientation");
            return true;
        }
        if (tab_orientation < 0) {
            error = localized_not_negative_value_required(catalog, "--tab-orientation");
            return true;
        }
        result.request.tab_orientation = tab_orientation;
        result.request.tab_orientation_available = true;
        return true;
    }

    if (argument == "--tab-orientation-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--tab-orientation-target-object-name");
            return true;
        }
        result.request.tab_orientation_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--tab-orientation-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--tab-orientation-target-unique-id");
            return true;
        }
        result.request.tab_orientation_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_display_orientation_argument(const std::string& argument,
                                        const localization::LocalizedCatalog& catalog,
                                        const std::vector<std::string>& args,
                                        std::size_t& index,
                                        LaunchParseResult& result,
                                        std::string& error) {
    if (argument == "--display-orientation-object") {
        result.request.display_orientation_object = true;
        return true;
    }

    if (argument == "--display-orientation") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--display-orientation");
            return true;
        }
        int display_orientation = 0;
        if (!parse_int_value(args[++index], display_orientation)) {
            error = localized_integer_value_required(catalog, "--display-orientation");
            return true;
        }
        if (display_orientation < 0) {
            error = localized_not_negative_value_required(catalog, "--display-orientation");
            return true;
        }
        result.request.display_orientation = display_orientation;
        result.request.display_orientation_available = true;
        return true;
    }

    if (argument == "--display-orientation-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--display-orientation-target-object-name");
            return true;
        }
        result.request.display_orientation_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--display-orientation-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--display-orientation-target-unique-id");
            return true;
        }
        result.request.display_orientation_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_help_context_id_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error) {
    if (argument == "--help-context-id-object") {
        result.request.help_context_id_object = true;
        return true;
    }

    if (argument == "--help-context-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--help-context-id");
            return true;
        }
        int help_context_id = 0;
        if (!parse_int_value(args[++index], help_context_id)) {
            error = localized_integer_value_required(catalog, "--help-context-id");
            return true;
        }
        if (help_context_id < 0) {
            error = localized_not_negative_value_required(catalog, "--help-context-id");
            return true;
        }
        result.request.help_context_id = help_context_id;
        result.request.help_context_id_available = true;
        return true;
    }

    if (argument == "--help-context-id-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--help-context-id-target-object-name");
            return true;
        }
        result.request.help_context_id_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--help-context-id-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--help-context-id-target-unique-id");
            return true;
        }
        result.request.help_context_id_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_whats_this_help_id_argument(const std::string& argument,
                                       const localization::LocalizedCatalog& catalog,
                                       const std::vector<std::string>& args,
                                       std::size_t& index,
                                       LaunchParseResult& result,
                                       std::string& error) {
    if (argument == "--whats-this-help-id-object") {
        result.request.whats_this_help_id_object = true;
        return true;
    }

    if (argument == "--whats-this-help-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help-id");
            return true;
        }
        int whats_this_help_id = 0;
        if (!parse_int_value(args[++index], whats_this_help_id)) {
            error = localized_integer_value_required(catalog, "--whats-this-help-id");
            return true;
        }
        if (whats_this_help_id < 0) {
            error = localized_not_negative_value_required(catalog, "--whats-this-help-id");
            return true;
        }
        result.request.whats_this_help_id = whats_this_help_id;
        result.request.whats_this_help_id_available = true;
        return true;
    }

    if (argument == "--whats-this-help-id-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help-id-target-object-name");
            return true;
        }
        result.request.whats_this_help_id_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--whats-this-help-id-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help-id-target-unique-id");
            return true;
        }
        result.request.whats_this_help_id_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_whats_this_help_argument(const std::string& argument,
                                    const localization::LocalizedCatalog& catalog,
                                    const std::vector<std::string>& args,
                                    std::size_t& index,
                                    LaunchParseResult& result,
                                    std::string& error) {
    if (argument == "--whats-this-help-object") {
        result.request.whats_this_help_object = true;
        return true;
    }

    if (argument == "--whats-this-help") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help");
            return true;
        }
        const auto value = parse_bool_value(args[++index]);
        if (!value.has_value()) {
            error = localized_logical_value_required(catalog, "--whats-this-help");
            return true;
        }
        result.request.whats_this_help = *value;
        result.request.whats_this_help_available = true;
        return true;
    }

    if (argument == "--whats-this-help-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help-target-object-name");
            return true;
        }
        result.request.whats_this_help_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--whats-this-help-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-help-target-unique-id");
            return true;
        }
        result.request.whats_this_help_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

bool parse_whats_this_button_argument(const std::string& argument,
                                      const localization::LocalizedCatalog& catalog,
                                      const std::vector<std::string>& args,
                                      std::size_t& index,
                                      LaunchParseResult& result,
                                      std::string& error) {
    if (argument == "--whats-this-button-object") {
        result.request.whats_this_button_object = true;
        return true;
    }

    if (argument == "--whats-this-button") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-button");
            return true;
        }
        const auto value = parse_bool_value(args[++index]);
        if (!value.has_value()) {
            error = localized_logical_value_required(catalog, "--whats-this-button");
            return true;
        }
        result.request.whats_this_button = *value;
        result.request.whats_this_button_available = true;
        return true;
    }

    if (argument == "--whats-this-button-target-object-name") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-button-target-object-name");
            return true;
        }
        result.request.whats_this_button_objects.push_back({
            .record_index = 0U,
            .object_name = args[++index],
            .unique_id = {}
        });
        return true;
    }

    if (argument == "--whats-this-button-target-unique-id") {
        if ((index + 1U) >= args.size()) {
            error = localized_missing_value_after_option(catalog, "--whats-this-button-target-unique-id");
            return true;
        }
        result.request.whats_this_button_objects.push_back({
            .record_index = 0U,
            .object_name = {},
            .unique_id = args[++index]
        });
        return true;
    }

    return false;
}

}  // namespace copperfin::studio
